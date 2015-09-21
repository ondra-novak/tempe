#include "compiler.h"
#include <lightspeed/base/namedEnum.tcc>
#include <lightspeed/utils/json/jsonfast.tcc>
#include "basicOps.h"
#include "functions.h"
#include "interfaces.h"
#include "exceptions.h"
#include "functionVar.h"
#include <lightspeed/base/text/textParser.tcc>
#include <lightspeed/base/containers/autoArray.tcc>
#include "tempeOps.h"


namespace Tempe {

	static NamedEnumDef<TokenReader::Symbol> kwDef[] = {
		{TokenReader:: kwOr, "or" },
		{ TokenReader::kwAnd, "and" },
		{ TokenReader::kwEnd, "end" },
		{ TokenReader::kwIf, "if" },
		{ TokenReader::kwThen, "then" },
		{ TokenReader::kwElse, "else" },
		{ TokenReader::kwElseIf, "elseif" },
		{ TokenReader::kwNot, "not" },
		{ TokenReader::kwDefined, "defined" },
		{ TokenReader::kwFirstDefined, "firstDefined" },
		{ TokenReader::kwUnset, "unset" },
		{ TokenReader::kwVar, "var" },
		{ TokenReader::kwNew, "new" },
		{ TokenReader::kwIsNull, "isnull" },
		{ TokenReader::kwLoop, "loop" },
		{ TokenReader::kwTry, "try" },
		{ TokenReader::kwCatch, "catch" },
		{ TokenReader::kwWith, "with" },
		{ TokenReader::kwScope, "scope" },
		{ TokenReader::kwObject, "object" },
		{ TokenReader::kwWhile, "while" },
		{ TokenReader::kwFunction, "function" },
		{ TokenReader::kwBreak, "break" },
		{ TokenReader::kwThrow, "throw" },
		{ TokenReader::kwTrue, "true" },
		{ TokenReader::kwFalse, "false" },
		{ TokenReader::kwNull, "null" },
		{ TokenReader::kwLink, "link" },
		{ TokenReader::kwJoin, "join" },
		{ TokenReader::kwMap, "map" },
		{ TokenReader::kwInclude, "include" },
		{ TokenReader::kwImport, "import" },
		{ TokenReader::kwDo, "do" },
		{ TokenReader::kwOptional, "optional" },
		{ TokenReader::kwOptional, "opt" },
		{ TokenReader::kwTemplate, "template" }, //< template <encoding> {$ template text $} end
		{ TokenReader::kwForeach, "foreach" }, 
		{ TokenReader::kwConst, "const" },  //< const <expr> - executes <expr> during compilation
		{ TokenReader::kwEcho, "echo" },
		{ TokenReader::begin, "<begin>" },
		{ TokenReader::eof, "<eof>" },
		{ TokenReader::symbUnknown, "<unknown-symbol>" },
		{ TokenReader::sValue, "<constant>" },
		{ TokenReader::sVarname, "<identifier>" },
		{ TokenReader::sLabel, "<label>" },
		{ TokenReader::symbSemicolon, ";" },
		{ TokenReader::symbAsssign, ":=" },
		{ TokenReader::symbEqual, "=" },
		{ TokenReader::symbNotEqual, "!=" },
		{ TokenReader::symbNotEqual, "<>" },
		{ TokenReader::symbAbove, ">" },
		{ TokenReader::symbBelow, "<" },
		{ TokenReader::symbAboveEqual, ">=" },
		{ TokenReader::symbBelowEqual, "<=" },
		{ TokenReader::symbPlus, "+" },
		{ TokenReader::symbMinus, "-" },
		{ TokenReader::symbAmp, "&" },
		{ TokenReader::symbIntDiv, "//" },
		{ TokenReader::symbDiv, "/" },
		{ TokenReader::symbMult, "*" },
		{ TokenReader::symbMod, "%" },
		{ TokenReader::symbOBracket, "(" },
		{ TokenReader::symbCBracket, ")" },
		{ TokenReader::symbComma, "," },
		{ TokenReader::symbOSqBracket, "[" },
		{ TokenReader::symbCSqBracket, "]" },
		{ TokenReader::symbDot, "." },
		{ TokenReader::symbOBrace, "{" },
		{ TokenReader::symbCBrace, "}" },
		{ TokenReader::symbMoney, "$" },
		{ TokenReader::symbExl, "!" },
		{ TokenReader::symbDblColon, ":" },
		{ TokenReader::symbHash, "#" },
		{ TokenReader::symbQColon, "::" },
		{ TokenReader::symbBeginTemplate, "{$" },
		{ TokenReader::symbEndTemplate, "$}" },
		{ TokenReader::symbThreeDots, "..." },

	};

	static NamedEnum<TokenReader::Symbol> kwList(kwDef);

	static NamedEnumDef<EscapeMode> strEscapeModeDef[] = {
		{ emPlain, "plain" },
		{ emHtml, "html" },
		{ emXml, "xml" },
		{ emJS, "js" },
		{ emJSON, "json" },
		{ emC, "c" },
		{ emURI, "uri" },
		{ emBase64, "base64" },
		{ emHex, "hex" },
		{ emVoid, "void" }
	};

	NamedEnum<EscapeMode> strEscapeMode(strEscapeModeDef);


	static NamedEnumDef<EscapeMode> strMimeCtDef[] = {
			{emPlain,"text/plain"},
			{emHtml,"text/html"},
			{emXml,"text/xml"},
			{emJS,"application/json"},
			{emJS,"text/javascript"},
	};

	static NamedEnum<EscapeMode> strMimeCt(strMimeCtDef);


	TokenReader::Symbol TokenReader::getNext()
	{
		if (accepted) {
			readNext();
			accepted = false;			
		}
		return type;
	}

	void TokenReader::accept() {
		accepted = true;
	}

	void TokenReader::readNext()
	{
		buffer.clear();
		eatWhite();
		//symbol accepted, read next
		if (!hasItems()) {
			type = eof;
		}
		else {
			char c = SourceReader::getNext();
			if (c < 0) {
				type = symbUnknown;
			}
			else if (c == '\r') {
				if (hasItems() && peek() == '\n') {
					skip();
				}
				type = eof;
			}
			else if (c == '\n') {
				type = eof;
			}
			else if (isalpha(c) || c == '_') {
				buffer.add(c);
				if (c > 0 && (isalpha(c) || c == '_')) {
					while (hasItems()) {
						c = peek();
						if (c > 0 && (c == '_' || isalnum(c))) {
							buffer.add(c);
							skip();
						}
						else if (expLabel && c == ':') {
							skip();
							type = sLabel;
							varname = buffer;
							expLabel = false;
							return;
						}
						else {
							break;
						}
					}
					type = sVarname;
					if (!kwList.find(buffer, type))
						varname = buffer;
					
				}
			}
			else if (isdigit(c)) {
				buffer.add(c);
				bool wasDot = false;
				bool wasE = false;
				bool wasSign = false;
				while (hasItems()) {
					c = peek();
					if (c > 0 && isdigit(c)) {
						buffer.add(c);
						skip();
						wasSign = wasE;
					}
					else if (c == '.' && !wasDot) {
						buffer.add(c);
						skip();
						wasDot = true;
					}
					else if ((c == 'e' || c == 'E') && !wasE) {
						buffer.add(c);
						skip();
						wasDot = true;
						wasE = true;
					}
					else if ((c == '+' || c == '-') && !wasSign && wasE) {
						buffer.add(c);
						skip();
						wasSign = true;
					} else 
						break;
				}
				if (wasDot || wasE) {
					buffer.add('\0');
					value = factory->newValue(strtod(buffer.data(), NULL));
				}
				else {
					natural val;
					parseUnsignedNumber(buffer.getFwIter(), val, 10);
					value = factory->newValue(val);
				}
				type = sValue;
			}
			else if (c == '"') {
				buffer.add(c);
				c = SourceReader::getNext();
				while (c != '"') {
					buffer.add(c);
					if (c == '\\') buffer.add(SourceReader::getNext());
					c = SourceReader::getNext();
				}
				buffer.add(c);
				value = JSON::parseFast(buffer.getFwIter());
				type = sValue;
			}
			else if (c == '`') {
				c = SourceReader::getNext();
				while (c != '`') {
					if (c == '\\') buffer.add(SourceReader::getNext());
					else buffer.add(c);
					c = SourceReader::getNext();
				}
				varname = buffer;
				type = sVarname;
			}
			else {
				bool nx = hasItems();
				buffer.add(c);
				if (kwList.find(buffer, type)) {
					while (nx) {
						char c = peek();
						if (c < 0 || isdigit(c) || isalnum(c)) {
							break;
						}
						buffer.add(c);
						if (kwList.find(buffer, type))  {
							skip();
							nx = hasItems();
						}
						else {
							nx = false;
						}
					}
				}
				else {
					type = symbUnknown;
				}
			}
		}


	}


	TokenReader::TokenReader(IRuntimeAlloc &alloc, SeqFileInput srcStream, FilePath fname, natural offset /*= 0*/)
		:SourceReader(srcStream, fname, offset)
		, type(begin)
		, accepted(false)
		, factory(JSON::create(alloc))
		, level(0)
		, expLabel(false)

	{

	}

	void TokenReader::enterLevel()
	{
		level++;
	}

	void TokenReader::leaveLevel()
	{
		level--;
	}

	void TokenReader::resetLevel(bool interactive)
	{
		level = interactive ? 0 : 1;
	}

	void TokenReader::expectLabel()
	{
		expLabel = true;
	}

	void TokenReader::eatWhite()
	{
		if (level == 0) {
			while (hasItems()) {
				char c = peek();
				if (c == ' ' || c == '\t') skip();
				else break;
			}
		}
		else  {
			while (hasItems()) {
				char c = peek();
				if (c >= 0 && isspace(c)) skip();
				else break;
			}
		}
	}

	Compiler2::Compiler2(IRuntimeAlloc &alloc)
		:alloc(alloc), valueFactory(JSON::create(alloc))
		, curEscapeMode(emPlain), constContext(globalConstContext)

	{

	}

	PExprNode Compiler2::compile(TokenReader &reader)
	{
		reader.resetLevel(false);
		if (reader.getNext() == TokenReader::begin) 
			reader.accept();
		PExprNode nd = compileExprSeq(reader);
		if (reader.getNext() != TokenReader::eof)
			throwUnexpectedError(reader.getLocation(), reader.getNext());
		reader.accept();
		return nd;
	}

	PExprNode Compiler2::compileInteractive(TokenReader &reader)
	{
		reader.resetLevel(true);
		if (reader.getNext() == TokenReader::begin)
			reader.accept();
		
		PExprNode r = compileExprSeq(reader);
		if (reader.getNext() != TokenReader::eof)
			throwUnexpectedError(reader.getLocation(), reader.getNext());
		reader.accept();
		return r;
		
	}

	PExprNode Compiler2::compileExprSeq(TokenReader& reader)
	{
		PExprNode a = compileAssign(reader);
		if (reader.getNext() == TokenReader::symbSemicolon) {
			///ignore empty commands
			ExprLocation loc = reader.getLocation();
			reader.accept();
			while (reader.getNext() == TokenReader::symbSemicolon) {
				reader.accept();
			}			

			TokenReader::Symbol s = reader.getNext();
			//ignore empty command before end, and braces
			if (s == TokenReader::kwEnd
				|| s == TokenReader::symbCBracket
				|| s == TokenReader::symbCBrace
				|| s == TokenReader::kwElse
				|| s == TokenReader::kwElseIf
				|| s == TokenReader::kwCatch
				|| s == TokenReader::eof ) {
				return a;
			}
			if (a->getIfcPtr<Constant>() != 0) 
				return compileExprSeq(reader);

			PExprNode b = compileExprSeq(reader);
			if (b->getIfcPtr<ConstStrA>() != 0) 
				return a;
			return (new(alloc)Oper_Comma(loc))
					->setBranch(0, a)->setBranch(1, b);			
		}
		else {
			return a;
		}
	}

	PExprNode Compiler2::compileAssign(TokenReader& reader)
	{
		PExprNode a = compileOR(reader);
		if (reader.getNext() == TokenReader::symbAsssign) {
			ExprLocation loc = reader.getLocation();
			reader.accept();
			PExprNode b = compileAssign(reader);
			Value v;
			if (b->getIfcPtr<Constant>() == 0 && b->tryToEvalConst(constContext,v)) 
				b =  new(alloc)Constant(loc, v);
			
			return (new(alloc)Oper_Assign(loc))
				->setBranch(0, a)->setBranch(1, b);
		}
		else {
			Value v;
			if (a->getIfcPtr<Constant>() == 0 && a->tryToEvalConst(constContext, v)) {
				return new(alloc)Constant(a->getSourceLocation(), v);
			}
			else {
				return a;
			}
		}
	}

	PExprNode Compiler2::compileOR(TokenReader& reader)
	{
		PExprNode a = compileAND(reader);
		if (reader.getNext() == TokenReader::kwOr) {
			ExprLocation loc = reader.getLocation();
			reader.accept();
			PExprNode b = compileOR(reader);
			return (new(alloc)Oper_Or(loc))
				->setBranch(0, a)->setBranch(1, b);
		}
		else {
			return a;
		}
	}

	PExprNode Compiler2::compileAND(TokenReader& reader)
	{
		PExprNode a = compileRELAT(reader);
		if (reader.getNext() == TokenReader::kwAnd) {
			ExprLocation loc = reader.getLocation();
			reader.accept();
			PExprNode b = compileAND(reader);
			return (new(alloc)Oper_And(loc))
				->setBranch(0, a)->setBranch(1, b);
		}
		else {
			return a;
		}

	}

	PExprNode Compiler2::compileRELAT(TokenReader& reader)
	{
		PExprNode nd = compilePLUSMINUS(reader);
		Oper_Fn2::Fn fnpick;
		switch (reader.getNext()) {
			case TokenReader::symbAboveEqual:fnpick = &operGreatEqual;  break;
			case TokenReader::symbBelowEqual: fnpick = &operLessEqual; break;
			case TokenReader::symbNotEqual: fnpick = &operNotEqual; break;
			case TokenReader::symbEqual: fnpick = &operEqual; break;
			case TokenReader::symbBelow: fnpick = &operLess; break;
			case TokenReader::symbAbove:fnpick = &operGreater; break;
			default: return nd;
		}
		reader.accept();

		ExprLocation loc = reader.getLocation();
		PExprNode nd2 = compileRELAT(reader);

		return (new(alloc)Oper_Fn2(loc, fnpick))->setBranch(0, nd)->setBranch(1, nd2);
	}

	PExprNode Compiler2::compilePLUSMINUS(TokenReader& reader)
	{
		PExprNode nd = compileMULTDIV(reader);
		Oper_Fn2::Fn fnpick;
		switch (reader.getNext()) {
			case TokenReader::symbPlus: fnpick = &operPlus; break;
			case TokenReader::symbAmp: fnpick = &operStringAppend; break;
			case TokenReader::symbMinus: fnpick = &operMinus; break;
			default: return nd;
		}
		reader.accept();

		ExprLocation loc = reader.getLocation();
		PExprNode nd2 = compilePLUSMINUS(reader);

		return (new(alloc)Oper_Fn2(loc, fnpick))->setBranch(0, nd)->setBranch(1, nd2);

	}

	PExprNode Compiler2::compileMULTDIV(TokenReader& reader)
	{
		PExprNode nd = compileUNARSuffix(reader);
		Oper_Fn2::Fn fnpick;
		switch (reader.getNext()) {
			case TokenReader::symbIntDiv: fnpick = &operIntegerDiv; break;
			case TokenReader::symbMult: fnpick = &operMul; break;
			case TokenReader::symbDiv: fnpick = &operDiv; break;
			case TokenReader::symbMod: fnpick = &operMod; break;
			default:return nd;
		}
		reader.accept();

		ExprLocation loc = reader.getLocation();
		PExprNode nd2 = compileMULTDIV(reader);

		return (new(alloc)Oper_Fn2(loc, fnpick))->setBranch(0, nd)->setBranch(1, nd2);

	}

	PExprNode Compiler2::compileUNAR(TokenReader& reader)
	{
		ExprLocation loc = reader.getLocation();
		switch (reader.getNext()) {
			case TokenReader::kwNot: 
				reader.accept();
				return (new(alloc)Oper_Fn1(loc, &operUnarNot))
					->setBranch(0, compileUNARSuffix(reader));
			case TokenReader::kwDefined:
				reader.accept();
				return (new(alloc)Oper_Exist(loc))
					->setBranch(0, compileUNARSuffix(reader));
			case TokenReader::kwFirstDefined: 
				reader.accept();
				return compileOpFirstDefined(loc, reader);			
			case TokenReader::kwUnset: 
				reader.accept();
				return compileOpUnset(loc, reader);
			case TokenReader::kwLink: 
				reader.accept();
				return (new(alloc)Oper_Link(loc))
					->setBranch(0, compileUNARSuffix(reader));
			case TokenReader::kwVar:
				reader.accept();
				return (new(alloc)Oper_Var(loc))
					->setBranch(0, compileUNARSuffix(reader));
			case TokenReader::kwNew:
				reader.accept();
				return (new(alloc)Oper_New(loc))
					->setBranch(0, compileUNARSuffix(reader));
			case TokenReader::kwIsNull:
				reader.accept();
				return (new(alloc)Oper_IsNull(loc))
					->setBranch(0, compileUNARSuffix(reader));
			case TokenReader::kwLoop:
				reader.accept();
				return (new(alloc)Oper_Cycle(loc))
					->setBranch(0, compileUNARSuffix(reader));
			case TokenReader::kwIf:
				reader.accept();
				return compileIF(loc, reader);
			case TokenReader::kwTry:
				reader.accept();
				return compileOpTryCatch(loc, reader);
			case TokenReader::kwWith:
				reader.accept();
				return compileOpWith(loc, reader);
			case TokenReader::kwScope:
				reader.accept();
				return compileOpScope(loc, reader);
			case TokenReader::kwObject:
				reader.accept();
				return compileOpObject(loc, reader);
			case TokenReader::kwWhile:
				reader.accept();
				return compileOpWhile(loc, reader);
			case TokenReader::kwFunction:
				reader.accept();
				return compileOpFunction(loc, reader);
			case TokenReader::kwEcho:
				reader.accept();
				return (new(alloc)OutputResult(loc, curEscapeMode))->setBranch(0,compileAssign(reader));
			case TokenReader::kwBreak:
				reader.accept();
				return (new(alloc)Oper_Break(loc));
			case TokenReader::kwThrow:
				reader.accept();
				return (new(alloc)Oper_Throw(loc))
					->setBranch(0, compileUNARSuffix(reader));
			case TokenReader::kwForeach:
				reader.accept();
				return compileForEach(loc, reader);
			case TokenReader::kwTemplate:
				reader.accept();
				return compileTemplateCmd(loc, reader);
			case TokenReader::symbMinus:
				reader.accept();
				return (new(alloc)Oper_Fn1(loc,&operUnarMinus))
					->setBranch(0, compileUNARSuffix(reader));
			case TokenReader::symbPlus:
				reader.accept();
				return  compileUNARSuffix(reader);
			case TokenReader::symbOBracket: 
				reader.accept();
				return compileSubExpr(loc, reader);
			case TokenReader::sValue:
				reader.accept();
				return new(alloc) Constant(loc, reader.value);
			case TokenReader::kwTrue:
				reader.accept();
				return new(alloc)Constant(loc, valueFactory->newValue(true));
			case TokenReader::kwFalse:
				reader.accept();
				return new(alloc)Constant(loc, valueFactory->newValue(false));
			case TokenReader::kwNull:
				reader.accept();
				return new(alloc)Constant(loc, valueFactory->newValue(nil));
			case TokenReader::sVarname:
				reader.accept();
				return new (alloc)VariableRef(loc, reader.varname);
			case TokenReader::symbBeginTemplate:
				reader.accept();
				return compileTemplateText(loc, reader);
			case TokenReader::kwConst:
				reader.accept();
				return compileConst(loc, reader);
			default: {
				TextFormatBuff<char> msg;
				msg("Unexpected '%1'") << kwList[reader.getNext()];
				throw ParseError(THISLOCATION, loc, msg.write());
			}
				


		}
	}

	PExprNode Compiler2::compileUNARSuffix(TokenReader& reader)
	{
		PExprNode nd = compileUNAR(reader);
		do {
			switch (reader.getNext()) {
			case TokenReader::symbOBracket:
				nd = compileFunctOp(nd, reader);
				break;
			case TokenReader::symbOSqBracket:
				nd = compileArrayOp(nd, reader);
				break;
			case TokenReader::symbDot:
				nd = compileMemberAccessOp(nd, reader);
				break;
			default:return nd;
			}
		}
		while (true);
			
	
	}

	PExprNode Compiler2::compileFunctOp(PExprNode nd, TokenReader& reader)
	{
		ExprLocation loc = reader.getLocation();
		reader.accept();
		RefCntPtr<Oper_FunctionCall> fn = new(alloc)Oper_FunctionCall(loc, nd);
		AutoArray<PExprNode, SmallAlloc<32> > branches;
		if (reader.getNext() != TokenReader::symbCBracket) {
			branches.add(compileExprSeq(reader));			
			while (reader.getNext() == TokenReader::symbComma) {
				reader.accept();
				branches.add(compileExprSeq(reader));
			}
			if (reader.getNext() != TokenReader::symbCBracket) {
				TokenReader::Symbol smb[] = { TokenReader::symbComma, TokenReader::symbCBracket };
				throwExpectedError(reader.getLocation(), ConstStringT<TokenReader::Symbol>(smb, 2));
			}
		}
		reader.accept();
		fn->setBranches(ConstStringT<PExprNode>(branches));
		nd = fn.get();
		return nd;

	}

	PExprNode Compiler2::compileArrayOp(PExprNode nd, TokenReader& reader)
	{
		ExprLocation loc = reader.getLocation();
		reader.accept();
		if (reader.getNext() == TokenReader::symbCSqBracket) {
			reader.accept();
			return (new(alloc)Oper_ArrayAppend(loc))->setBranch(0, nd);
		}
		else {
			PExprNode index = compileExprSeq(reader);
			if (reader.getNext() == TokenReader::symbCSqBracket) {
				reader.accept();
				return (new(alloc)Oper_ArrayIndex(loc))->setBranch(0, nd)->setBranch(1, index);
			}
			else {
				throw ParseError(THISLOCATION, loc, "Expected ']'");
			}
		}
	}

	PExprNode Compiler2::compileTemplate(TokenReader& reader, EscapeMode em) {
		EscapeMode md = curEscapeMode;
		curEscapeMode = em;
		try {
			return compileTemplateText(reader.getLocation(), reader);
			curEscapeMode=md;
		} catch (...) {
			curEscapeMode=md;
			throw;
		}
	}

	PExprNode Compiler2::compileMemberAccessOp(PExprNode nd, TokenReader& reader)
	{
		ExprLocation loc = reader.getLocation();
		reader.accept();
		PExprNode vname = compileUNAR(reader);
		if (vname->getIfcPtr<LocalVarRef>() == 0) {
			throw ParseError(THISLOCATION, reader.getLocation(), "Right side of '.' must be an identifier");
		}
		return new(alloc)Oper_MemberAccess(reader.getLocation(), nd, vname);
	}

	Tempe::PExprNode Compiler2::compileOpFirstDefined(ExprLocation loc, TokenReader &reader)
	{
		RefCntPtr<VariadicNode> nd = new(alloc)Oper_FirstDefined(loc);
		AutoArray<PExprNode, SmallAlloc<32> > branches;
		branches.add(compileUNARSuffix(reader));
		while (reader.getNext() == TokenReader::symbComma) {
			reader.accept();
			branches.add(compileUNARSuffix(reader));

		}
		nd->setBranches(branches);
		return nd.get();
	}

	Tempe::PExprNode Compiler2::compileOpUnset(ExprLocation loc, TokenReader& reader)
	{
		PExprNode nd = compileUNARSuffix(reader);
		if (nd->getIfcPtr<IGetVarName>() == 0)
			throw ParseError(THISLOCATION, loc, "Expecting variable");
		return (new(alloc)Oper_Unset(loc))->setBranch(0, nd);
	}

	Tempe::PExprNode Compiler2::compileIF(ExprLocation loc, TokenReader& reader)
	{
		reader.enterLevel();
		PExprNode cond = compileAssign(reader);
		if (reader.getNext() == TokenReader::kwThen) {			
			reader.accept();
			PExprNode thenPart = compileExprSeq(reader);
			PExprNode elsePart;
			if (reader.getNext() == TokenReader::kwElseIf) {
				reader.accept();
				elsePart = compileIF(reader.getLocation(), reader);
			}
			else if (reader.getNext() == TokenReader::kwElse){
				reader.accept();
				elsePart = compileExprSeq(reader);
			} 
			if (reader.getNext() != TokenReader::kwEnd) {
				static const TokenReader::Symbol symbs[] = {
					TokenReader::kwElse,
					TokenReader::kwElseIf,
					TokenReader::kwEnd
				};
				throwExpectedError(reader.getLocation(), ConstStringT<TokenReader::Symbol>(symbs,3));
			}
			reader.accept();
			reader.leaveLevel();
			if (elsePart == nil) {
				return (new(alloc)Oper_And(loc))
					->setBranch(0, cond)
					->setBranch(1, thenPart);
			}
			else {
				return (new(alloc)Oper_If(loc))
					->setBranch(0, cond)
					->setBranch(1, thenPart)
					->setBranch(2, elsePart);
			}
		}
		else {
			throwExpectedError(reader.getLocation(), ConstStringT<TokenReader::Symbol>(TokenReader::kwThen));
			throw;
		}
	}

	Tempe::PExprNode Compiler2::compileSubExpr(ExprLocation loc, TokenReader &reader)
	{
		reader.enterLevel();
		PExprNode nd = compileExprSeq(reader);
		if (reader.getNext() != TokenReader::symbCBracket)
			throwExpectedError(loc, TokenReader::symbCBracket);
		reader.accept();
		reader.leaveLevel();
		return nd;
	}

	Tempe::PExprNode Compiler2::compileOpTryCatch(ExprLocation loc, TokenReader& reader)
	{
		reader.enterLevel();
		PExprNode tryPart = compileExprSeq(reader);
		if (reader.getNext() != TokenReader::kwCatch)
			throwExpectedError(reader.getLocation(), TokenReader::kwCatch);
		reader.accept();
		PExprNode var = compileUNARSuffix(reader);
		PExprNode catchPart = compileExprSeq(reader);
		reader.leaveLevel();
		if (reader.getNext() == TokenReader::kwEnd) {
			reader.accept();
		}
		return (new(alloc)Oper_TryCatch(reader.getLocation()))
			->setBranch(0, tryPart)
			->setBranch(1, var)
			->setBranch(2, catchPart);

	}

	Tempe::PExprNode Compiler2::compileOpWith(ExprLocation loc, TokenReader& reader)
	{
		reader.enterLevel();
		PExprNode expr = compileUNARSuffix(reader);
		PExprNode cmdlist;
		Oper_WithDo::Isolation isol = Oper_WithDo::isoDefault;
		if (reader.getNext() == TokenReader::sVarname && reader.varname == "isolation") {
			reader.accept();
			if (reader.getNext() == TokenReader::sVarname) {
				if (reader.varname == "none" || reader.varname == "rw") {
					isol = Oper_WithDo::isoDefault;
				}
				else if (reader.varname == "readonly" || reader.varname == "ro") {
					isol = Oper_WithDo::isoReadonly;
				}
				else if (reader.varname == "full") {
					isol = Oper_WithDo::isoFull;
				}
				else {
					throw ParseError(THISLOCATION, reader.getLocation(), "Unknown isolation level");
				}
				reader.accept();
			}
		}
		cmdlist = compileExprSeq(reader);
	
		if (reader.getNext() == TokenReader::kwEnd) {
			reader.accept();
			reader.leaveLevel();
			return (new(alloc)Oper_WithDo(reader.getLocation(), isol))
				->setBranch(0, expr)
				->setBranch(1, cmdlist);
		}
		else {
			throwExpectedError(reader.getLocation(), TokenReader::kwEnd);
			throw;
		}
	}

	Tempe::PExprNode Compiler2::compileOpScope(ExprLocation loc, TokenReader& reader)
	{
		reader.enterLevel();
		PExprNode body = compileExprSeq(reader);
		if (reader.getNext() == TokenReader::kwEnd) {
			reader.accept();
			reader.leaveLevel();			
			return (new(alloc)Oper_Scope(reader.getLocation()))->setBranch(0, body);
		} 
		else {
			throwExpectedError(reader.getLocation(), TokenReader::kwEnd);
			throw;
		}

	}


	Tempe::PExprNode Compiler2::compileOpObject(ExprLocation loc, TokenReader& reader)
	{
		reader.enterLevel();
		PExprNode body = compileExprSeq(reader);
		if (reader.getNext() == TokenReader::kwEnd) {
			reader.accept();
			reader.leaveLevel();
			return (new(alloc)Oper_Object(reader.getLocation()))->setBranch(0, body);
		}
		else {
			throwExpectedError(reader.getLocation(), TokenReader::kwEnd);
			throw;
		}

	}

	Tempe::PExprNode Compiler2::compileOpWhile(ExprLocation loc, TokenReader& reader)
	{
		reader.enterLevel();
		PExprNode cond = compileAssign(reader);
		if (reader.getNext() == TokenReader::kwDo) {
			reader.accept();
			PExprNode body = compileExprSeq(reader);
			if (reader.getNext() == TokenReader::kwEnd) {
				reader.accept();
				reader.leaveLevel();
				ExprLocation loc = reader.getLocation();
				Value v1, v2;
				if (cond->tryToEvalConst(constContext, v1)) {
					if (v1->getBool() == false) return new (alloc)Constant(loc, valueFactory->newValue(false));
					else if (body->tryToEvalConst(constContext, v2))
						throw ParseError(THISLOCATION, loc, "Infinite cycle");
				}
				if (body->tryToEvalConst(constContext,v2)) {
					return (new(alloc)Oper_Cycle(loc))->setBranch(0, cond);
				}
				else {
					PExprNode semcol = (new(alloc)Oper_Comma(loc))->setBranch(0, body)->setBranch(1, cond);
					PExprNode loopCmd = (new(alloc)Oper_Cycle(loc))->setBranch(0, semcol);
					PExprNode andCmd = (new(alloc)Oper_And(loc))->setBranch(0, cond)->setBranch(1, loopCmd);
					return andCmd;
				}
			}
			else {
				throwExpectedError(reader.getLocation(), TokenReader::kwEnd);
			}
		}
		else {
			throwExpectedError(reader.getLocation(), TokenReader::kwDo);
		}
		throw;
		
	}

	Tempe::PExprNode Compiler2::compileOpFunction(ExprLocation loc, TokenReader& reader)
	{
		if (reader.getNext() == TokenReader::symbOBracket) {
			reader.accept();
			reader.enterLevel();

			AutoArray<FunctionVar::VarName_OutMode> varlist;
			ExprLocation loc = reader.getLocation();
			bool opt = false;
			bool rep = false;
			if (reader.getNext() != TokenReader::symbCBracket) do {
				bool byref = false;
				StringA varname;
				if (reader.getNext() == TokenReader::kwOptional) {
					reader.accept();
					opt = true;
				}
				FunctionVar::VarType outmode = opt ? FunctionVar::optional : FunctionVar::byValue;
				if (reader.getNext() == TokenReader::symbAmp) {
					reader.accept();
					byref = true;
					outmode = opt ? FunctionVar::optionalReference : FunctionVar::byReference;
				}

				if (reader.getNext() == TokenReader::sVarname) {
					varname = reader.varname;
					reader.accept();
					if (reader.getNext() == TokenReader::symbThreeDots) {
						if (byref) throwUnexpectedError(loc, TokenReader::symbThreeDots);
						reader.accept();
						varlist.add(FunctionVar::VarName_OutMode(varname, FunctionVar::variadic));
					}
					else {
						varlist.add(FunctionVar::VarName_OutMode(varname, outmode));
					}
				}
				else {
					throwExpectedError(loc, TokenReader::sVarname);
				}

				loc = reader.getLocation();
				rep = reader.getNext() == TokenReader::symbComma;
				if (rep) reader.accept();
			} while (rep);
			if (reader.getNext() == TokenReader::symbCBracket) {
				reader.accept();
				PExprNode nd = compileExprSeq(reader);
				if (reader.getNext() == TokenReader::kwEnd) {
					reader.accept();
					reader.leaveLevel();
					return new(alloc)Constant(loc, Value(new(alloc)FunctionVar(varlist, nd)));
				}
				else {
					throwExpectedError(loc, TokenReader::kwEnd);
				}
			}
			else {
				throwExpectedError(loc, TokenReader::symbCBracket);
			}
		}
		else {
			throwExpectedError(loc, TokenReader::symbOBracket);
		}
		throw;
	}


	void Compiler2::throwExpectedError(const ExprLocation &loc, ConstStringT<TokenReader::Symbol> symbols)
	{
		AutoArray<char, SmallAlloc<256> > msg;
		msg.append(ConstStrA("Expected: "));
		for (natural i = 0; i < symbols.length(); i++) {
			if (i) msg.append(ConstStrA(" or "));
			msg.add('\'');
			msg.append(kwList[symbols[i]]);
			msg.add('\'');
		}
		msg.add('.');
		throw ParseError(THISLOCATION, loc, msg);
	}

	void Compiler2::throwUnexpectedError(const ExprLocation &loc, ConstStringT<TokenReader::Symbol> symbols)
	{
		AutoArray<char, SmallAlloc<256> > msg;
		msg.append(ConstStrA("Unexpected: "));
		for (natural i = 0; i < symbols.length(); i++) {
			if (i) msg.append(ConstStrA(" or "));
			msg.add('\'');
			msg.append(kwList[symbols[i]]);
			msg.add('\'');
		}
		msg.add('.');
		throw ParseError(THISLOCATION, loc, msg);
	}

	Tempe::PExprNode Compiler2::compileTemplateText(ExprLocation loc, TokenReader& reader)
	{
		buffer.clear();
		SourceReader &srcrd = reader;
		while (srcrd.hasItems()) {
			char c = srcrd.getNext();
			if (c == '$') {
				if (!srcrd.hasItems()) break;
				c = srcrd.getNext();
				if (c == '$') buffer.add(c);
				else if (c == '}') {
					return new(alloc) OutputText(loc, buffer);
				}
				else {
					loc = reader.getLocation();
					if (c == '{') {
						PExprNode curTemplate = new(alloc)OutputText(loc, buffer);
						PExprNode expr = compileTemplateSubExpr(loc, reader);
						if (reader.getNext() == TokenReader::symbCBrace) {
							reader.accept();
							PExprNode subcmd = (new(alloc) Oper_Comma(loc))
								->setBranch(0, curTemplate)
								->setBranch(1, expr);
							loc = reader.getLocation();
							PExprNode nxtTemplate = compileTemplateText(loc, reader);
							PExprNode subcmd2 = (new(alloc) Oper_Comma(loc))
								->setBranch(0, subcmd)
								->setBranch(1, nxtTemplate);
							return subcmd2;
						}
						else {
							throwExpectedError(loc, TokenReader::symbCBrace);
						}
					}
					else {
						throwExpectedError(loc, TokenReader::symbCBrace);
					}
				}
			}
			else {
				buffer.add(c);
			}
		} 
		return new(alloc) OutputText(loc, buffer);
	}



	Tempe::PExprNode Compiler2::compileTemplateSubExpr(ExprLocation loc, TokenReader& reader)
	{
		reader.expectLabel();
		TokenReader::Symbol s = reader.getNext();
		EscapeMode escMode;
		if (s == TokenReader::sLabel) {
			 escMode = strEscapeMode[reader.varname];
			reader.accept();
		}
		else {
			escMode = curEscapeMode;
		}

		PExprNode expr = compileExprSeq(reader);
		return (new(alloc)OutputResult(loc, escMode))->setBranch(0, expr);
	}

	//foreach x print(this.neco) end

	Tempe::PExprNode Compiler2::compileForEach(ExprLocation loc, TokenReader& reader)
	{
		PExprNode expr = compileUNARSuffix(reader);
		PExprNode body = compileExprSeq(reader);
		if (reader.getNext() == TokenReader::kwEnd) {
			reader.accept();
		}
		return (new(alloc)Oper_ForEach(loc))
			->setBranch(0, expr)
			->setBranch(1, body);
	}

	Tempe::PExprNode Compiler2::compileConst(ExprLocation loc, TokenReader& reader)
	{
		PExprNode expr = compileAssign(reader);

		LocalScope scope(static_cast<IExprEnvironment &>(constContext));
		return new(alloc)Constant(loc, expr->calculate(constContext));
	}

	Tempe::PExprNode Compiler2::compileTemplateCmd(ExprLocation loc, TokenReader& reader)
	{
		reader.expectLabel();
		if (reader.getNext() != TokenReader::sVarname || reader.getNext() != TokenReader::sLabel) {
			reader.accept();
			EscapeMode escMode = strEscapeMode[reader.varname];
			EscapeMode prev = curEscapeMode;
			curEscapeMode = escMode;
			PExprNode expr = compileAssign(reader);
			curEscapeMode = prev;
			if (reader.getNext() == TokenReader::kwEnd) {
				reader.accept();
			}
			return expr;
		}
		else
			throwExpectedError(loc, TokenReader::sLabel);
		throw;
	}

EscapeMode Compiler2::getCtFromMime(ConstStrA contentType) {
	return strMimeCt[contentType];
}

}
