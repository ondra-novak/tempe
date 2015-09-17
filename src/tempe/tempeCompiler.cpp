/*
 * tempeCompiler.cpp
 *
 *  Created on: 28. 4. 2015
 *      Author: ondra
 */

#include "tempeCompiler.h"

#include <lightspeed/base/containers/convertString.tcc>
#include <lightspeed/base/namedEnum.tcc>
#include <lightspeed/base/streams/memfile.h>
#include <lightspeed/base/streams/utf.h>
#include <lightspeed/utils/base64.h>
#include <lightspeed/utils/json/jsonfast.tcc>
#include <lightspeed/utils/urlencode.h>

#include "basicOps.h"
#include "exceptions.h"


namespace Tempe {


static NamedEnumDef<TempeCompiler::ContentType> strContextTypeDef[] = {
		{TempeCompiler::ctnPlain,"plain"},
		{TempeCompiler::ctnPlain,""},
		{TempeCompiler::ctnHtml,"html"},
		{TempeCompiler::ctnXml,"xml"},
		{TempeCompiler::ctnJS,"js"},
		{TempeCompiler::ctnC,"c"},
		{TempeCompiler::ctnURI,"uri"},
		{TempeCompiler::ctnBase64,"base64"},
		{TempeCompiler::ctnHex,"hex"},
		{TempeCompiler::ctnVoid,"void"}
};

NamedEnum<TempeCompiler::ContentType> TempeCompiler::strContentType(strContextTypeDef);

static NamedEnumDef<TempeCompiler::ContentType> strMimeCtDef[] = {
		{TempeCompiler::ctnPlain,"text/plain"},
		{TempeCompiler::ctnHtml,"text/html"},
		{TempeCompiler::ctnXml,"text/xml"},
		{TempeCompiler::ctnJS,"application/json"},
		{TempeCompiler::ctnJS,"text/javascript"},
};

NamedEnum<TempeCompiler::ContentType> TempeCompiler::strMimeCt(strMimeCtDef);

static String escapeXml(String v, bool nlbr) {
	AutoArray<wchar_t, SmallAlloc<256> > buffer;
	for (String::Iterator iter = v.getFwIter(); iter.hasItems();) {
		wchar_t c= iter.getNext();
		switch (c) {
		case '<': buffer.append(ConstStrW(L"&lt;"));break;
		case '>': buffer.append(ConstStrW(L"&gt;"));break;
		case '"': buffer.append(ConstStrW(L"&quot;"));break;
		case '&': buffer.append(ConstStrW(L"&amp;"));break;
		case '\n': if (nlbr) buffer.append(ConstStrW(L"<br />"));else buffer.add(c);break;
		case '\r': if (!nlbr) buffer.add(c);break;
		default: buffer.add(c);break;
		}
	}
	return buffer;
}

static String escapeJs(String v) {
	AutoArray<char, SmallAlloc<256> > buffer;
	AutoArray<char, SmallAlloc<256> >::WriteIter iter = buffer.getWriteIterator();
	VtWriteIterator<AutoArray<char, SmallAlloc<256> >::WriteIter &> wrt(iter);
	JSON::serializeString(v,wrt);
	return buffer.crop(1,1);
}

static String escapeC(String v) {
	//TODO: make it better
	return escapeJs(v);
}

static String escapeURI(String v) {
	StringA vc = v.getUtf8();
	StringA encoded = convertString(UrlEncoder(),ConstStrA(vc));
	return encoded;
}

static String escapeBase64(String v) {
	StringA vc = v.getUtf8();
	StringA encoded = convertString(Base64EncoderT<char,char>(),ConstStrA(vc));
	return encoded;
}

static String escapeHex(String v) {
	StringA vc = v.getUtf8();
	StringA encoded = convertString(HexEncoder<char,char>(),ConstStrA(vc));
	return encoded;
}


class EscapeResult: public AbstractNode {
public:

	EscapeResult(const ExprLocation &loc,
			TempeCompiler::ContentType ctx, PExprNode what):AbstractNode(loc),ctx(ctx),what(what) {}
	virtual Value calculate(IExprEnvironment &env) const {
		Value v = what->calculate(env);
		String s = v->getString();
		switch (ctx) {
		case TempeCompiler::ctnPlain: return v;
		case TempeCompiler::ctnHtml: s = escapeXml(s,true);  break;
		case TempeCompiler::ctnXml: s = escapeXml(s,false); break;
		case TempeCompiler::ctnJS: s = escapeJs(s);break;
		case TempeCompiler::ctnC: s = escapeC(s);break;
		case TempeCompiler::ctnURI: s = escapeURI(s);break;
		case TempeCompiler::ctnBase64: s = escapeBase64(s);break;
		case TempeCompiler::ctnHex: s = escapeHex(s);break;
		case TempeCompiler::ctnVoid: s = String();break;
		default:return v;
		}
		return env.getFactory().newValue(s);
	}


protected:

	TempeCompiler::ContentType ctx;
	PExprNode what;
};

class LangString: public AbstractNode {
public:
	LangString(const ExprLocation &loc, StringA id):AbstractNode(loc),id(id) {}

	virtual Value calculate(IExprEnvironment &env) const {
		const IExprEnvironment &genv = env.getGlobalEnv();
		if (genv.varExists(id)) {
			return genv.getVar(id);
		} else {
			return env.getFactory().newValue(id);
		}
	}

protected:
	StringA id;
};

class ConcatParts: public NaryNode<3> {
public:
	ConcatParts(const ExprLocation &loc):NaryNode<3>(loc) {}

	virtual Value calculate(IExprEnvironment &env, const Value *subResults) const  {
		ConstStrW a = subResults[0]->getString();
		ConstStrW b = subResults[1]->getString();
		ConstStrW c = subResults[2]->getString();
		String res = a+b+c;
		return env.getFactory().newValue(res);
	}


};

PExprNode TempeCompiler::compileUNAR(SourceReader& reader) {

	if (reader(" {$%")) {
		LevelControl levControl(reader,level);
		PExprNode nd = compileTemplate(reader);
		return nd;
	} else if (reader(" frag\b%")) {
		LevelControl levControl(reader,level);
		PExprNode expr = compileUNAR(reader);
		PExprNode frag = compileUNAR(reader);
		frag = new(alloc) Oper_WithDoJoin(reader.getLocation(),frag);
		return (new(alloc) Oper_WithDo(reader.getLocation()))->setBranch(0,expr)
				->setBranch(1,frag);
	} else return Compiler::compileUNAR(reader);
}

PExprNode TempeCompiler::compileTemplate(SourceReader& reader) {

	ContentType ctx = defaultContentType;
	if (reader("%(*)[a-z](*)[0-9]1\\%")) {
		ConstStrA strctx = reader[1].str();
		if (strctx == "default") ctx = defaultContentType;
		else ctx = strContentType[strctx];
	}
	return compileTemplateExpression(reader,ctx,false);

}


PExprNode TempeCompiler::compileTemplateExpression(SourceReader& reader, ContentType ctx, bool noEnding) {

	AutoArrayStream<wchar_t> str;
	Utf8ToWideWriter<AutoArrayStream<wchar_t> &> wr(str);


	while (reader.hasItems()) {
		char c = reader.getNext();
		if (c == '$') {
			char d = reader.getNext();
			if (d == '$') wr.write(d);
			else if (d == '}') {
				if (noEnding) {
					throw ParseError(THISLOCATION,reader.getLocation(),"Template end sequence is not allowed here");
				} else {
					ExprLocation loc = reader.getLocation();
					return createStringNode(loc,str.getArray());
				}
			} else if (d == '{') {
				ExprLocation loc = reader.getLocation();
				PExprNode n1 = createStringNode(loc,str.getArray());
				PExprNode n2 = compilePlaceholder(reader,ctx);
				char e = reader.getNext();
				while (isspace(e)) e = reader.getNext();
				if (e != '}') throw ParseError(THISLOCATION,reader.getLocation(),"Expecting '}'");
				PExprNode n3 = compileTemplateExpression(reader,ctx,noEnding);
				return (new(alloc) ConcatParts(loc))->setBranch(0,n1)
						->setBranch(1,n2)->setBranch(2,n3);

			} else if (isalpha(d) || d == '_') {
				ExprLocation loc = reader.getLocation();
				PExprNode n1 = createStringNode(loc,str.getArray());
				PExprNode n2 = compileTextID(reader,ctx,d);
				PExprNode n3 = compileTemplateExpression(reader,ctx,noEnding);
				return (new(alloc) ConcatParts(loc))->setBranch(0,n1)
						->setBranch(1,n2)->setBranch(2,n3);

			} else {
				throw ParseError(THISLOCATION,reader.getLocation(),"Invalid escape character. Must be one of '$$', '$}' or '${'");
			}
		} else {
			wr.write(c);
		}
	}
	if (noEnding) {
		ExprLocation loc = reader.getLocation();
		return createStringNode(loc,str.getArray());
	} else {
		throw ParseError(THISLOCATION,reader.getLocation(),"Unexpected end of file");
	}

}

PExprNode TempeCompiler::compilePlaceholder(SourceReader& reader, ContentType ctx) {

	ContentType forcedCtx = ctx;

	if (reader("%(*)[a-z](*)[0-9]1\\%")) {
		ConstStrA strctx = reader[1].str();
		if (strctx != "default")  {
			forcedCtx = strContentType[strctx];
		}
	}
	ExprLocation loc = reader.getLocation();

	ContentType saveCtx = defaultContentType;
	try {
		defaultContentType = ctx;
		PExprNode nd = compile(reader);
		defaultContentType = saveCtx;

		if (forcedCtx == ctnPlain) return nd;
		else {
			return new(alloc)EscapeResult(loc,forcedCtx,nd);
		}
	} catch (...) {
		defaultContentType = saveCtx;
		throw;
	}
}

PExprNode TempeCompiler::compileTemplate(ConstStrA text, ContentType ctype) {
	MemFileStr memfile(text);memfile.setStaticObj();
	SeqFileInput infile(&memfile);
	SourceReader treader(infile,FilePath());
	TempeCompiler compiler;
	return compiler.compileTemplateExpression(treader,ctype,true);

}

TempeCompiler::ContentType TempeCompiler::getCtFromMime(ConstStrA mime) {
	return strMimeCt[mime];
}

PExprNode TempeCompiler::compileTextID(SourceReader& reader, ContentType ctx, char firstChar) {
	ContentType forcedCtx = ctx;

	if (reader("%(*)[a-z](*)[0-9]1\\%")) {
		ConstStrA strctx = reader[1].str();
		if (strctx != "default")  {
			forcedCtx = strContentType[strctx];
		}
	}
	ExprLocation loc = reader.getLocation();
	AutoArray<char, SmallAlloc<256> > ident;
	ident.add(firstChar);
	while (reader.hasItems()) {
		char c = reader.getNext();
		if (c == '$') break;
		ident.add(c);
	}
	PExprNode nd = new(alloc) LangString(loc, StringA(ident));
	if (forcedCtx == ctnPlain) return nd;
	else {
		return new(alloc)EscapeResult(loc,forcedCtx,nd);
	}



}

PExprNode TempeCompiler::createStringNode(const ExprLocation &loc, const String& str) {
	return new(alloc) Constant(loc, valueFactory->newValue(str));
}

} /* namespace Tempe */
