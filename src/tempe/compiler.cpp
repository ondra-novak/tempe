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
		{ TokenReader::kwVar, "varname" },
		{ TokenReader::kwNew, "new" },
		{ TokenReader::kwIsNull, "isnull" },
		{ TokenReader::kwLoop, "loop" },
		{ TokenReader::kwTry, "try" },
		{ TokenReader::kwCatch, "catch" },
		{ TokenReader::kwWith, "with" },
		{ TokenReader::kwScope, "scope" },
		{ TokenReader::kwWhile, "while" },
		{ TokenReader::kwFunction, "function" },
		{ TokenReader::kwBreak, "break" },
		{ TokenReader::kwThrow, "throw" },
		{ TokenReader::kwTrue, "true" },
		{ TokenReader::kwFalse, "false" },
		{ TokenReader::kwNull, "null" },
		{ TokenReader::kwLink, "->" },
		{ TokenReader::kwImport, "import" },
		{ TokenReader::kwDo, "do" },
		{ TokenReader::kwOptional, "optional" },
		{ TokenReader::kwOptional, "opt" },
		{ TokenReader::kwTemplate, "template" }, //< template <encoding> {$ template text $} end
		{ TokenReader::kwForeach, "foreach" }, 
		{ TokenReader::kwConst, "const" },  //< const <expr> - executes <expr> during compilation
		{ TokenReader::kwEcho, "echo" },
		{ TokenReader::kwRepeat, "repeat" },
		{ TokenReader::kwUntil, "until" },
		{ TokenReader::kwVarname, "getvarname" },
		{ TokenReader::kwInline, "inline" },
		{ TokenReader::begin, "<begin>" },
		{ TokenReader::eof, "<eof>" },
		{ TokenReader::symbUnknown, "<unknown-symbol>" },
		{ TokenReader::sValue, "<constant>" },
		{ TokenReader::sVarname, "<identifier>" },
		{ TokenReader::sLabel, "<label>" },
		{ TokenReader::symbSemicolon, ";" },
		{ TokenReader::symbAsssign, "=" },
		{ TokenReader::symbEqual, "==" },
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

	NamedEnum<EscapeMode> strMimeCt(strMimeCtDef);


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

	Compiler::Compiler(IRuntimeAlloc &alloc)
		:alloc(alloc)
		, curOutputConfig(new(alloc) OutputConfig)
		, curConstScope(0)

	{

	}

	Compiler::~Compiler()
	{

	}

	class CompilerContext {
	public:
		VarTable globalConstContext;
		FakeGlobalScope constContext;
		IExprEnvironment * &curContext;
		JSON::PFactory &curFactory;
		IExprEnvironment * prevContext;;
		JSON::PFactory prevFactory;

		CompilerContext(IExprEnvironment * &context, JSON::PFactory &factory)
			:constContext(globalConstContext)
			, curContext(context)
			, curFactory(factory)
		{
			prevContext = context;
			prevFactory = factory;
			context = &constContext;
			factory = &constContext.getFactory();
		}

		~CompilerContext() {
			curFactory = prevFactory;
			curContext = prevContext;
		}
			


	};

	PExprNode Compiler::compile(TokenReader &reader)
	{
		CompilerContext ctx(curConstScope, valueFactory);

		reader.resetLevel(false);
		if (reader.getNext() == TokenReader::begin) 
			reader.accept();
		PExprNode nd = compileExprSeq(reader);
		if (reader.getNext() != TokenReader::eof)
			throwUnexpectedError(reader.getLocation(), reader.getNext());
		reader.accept();
		return nd;
	}

	PExprNode Compiler::compileInteractive(TokenReader &reader)
	{
		CompilerContext ctx(curConstScope, valueFactory);


		reader.resetLevel(true);
		if (reader.getNext() == TokenReader::begin)
			reader.accept();
		
		PExprNode r = compileExprSeq(reader);
		if (reader.getNext() != TokenReader::eof)
			throwUnexpectedError(reader.getLocation(), reader.getNext());
		reader.accept();
		return r;
		
	}

	PExprNode Compiler::compileExprSeq(TokenReader& reader)
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
				|| s == TokenReader::kwUntil
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

	PExprNode Compiler::compileAssign(TokenReader& reader)
	{
		PExprNode a = compileOR(reader);
		if (reader.getNext() == TokenReader::symbAsssign) {
			ExprLocation loc = reader.getLocation();
			reader.accept();
			PExprNode b = compileAssign(reader);
			Value v;
			if (b->getIfcPtr<Constant>() == 0 && b->tryToEvalConst(*curConstScope,v)) 
				b =  new(alloc)Constant(loc, v);
			
			return (new(alloc)Oper_Assign(loc))
				->setBranch(0, a)->setBranch(1, b);
		}
		else {
			Value v;
			if (a->getIfcPtr<Constant>() == 0 && a->tryToEvalConst(*curConstScope, v)) {
				return new(alloc)Constant(a->getSourceLocation(), v);
			}
			else {
				return a;
			}
		}
	}

	PExprNode Compiler::compileOR(TokenReader& reader)
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

	PExprNode Compiler::compileAND(TokenReader& reader)
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

	PExprNode Compiler::compileRELAT(TokenReader& reader)
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

	PExprNode Compiler::compilePLUSMINUS(TokenReader& reader)
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

	PExprNode Compiler::compileMULTDIV(TokenReader& reader)
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

PExprNode Compiler::compileNEW(ExprLocation , TokenReader& reader) {
	PExprNode nd = compileUNARSuffix(reader);
	Oper_FunctionCall *fncall = nd->getIfcPtr<Oper_FunctionCall>();
	if (fncall==0) {
		throwExpectedError(reader.getLocation(),TokenReader::symbOBracket);
	}
	return new (alloc) Oper_New(*fncall);
	}


PExprNode Compiler::compileOpVar(ExprLocation loc, TokenReader& reader)
{
	if (reader.getNext() == TokenReader::sValue) {
		reader.accept();
		return new(alloc)VariableRef(loc, VarName(reader.value->getStringUtf8()));
	}
	else {
		return (new(alloc)Oper_Var(loc))
			->setBranch(0, compileUNARSuffix(reader));
	}
}

PExprNode Compiler::compileUNAR(TokenReader& reader)
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
				return compileOpVar(loc, reader);

			case TokenReader::kwNew:
				reader.accept();
				return compileNEW(loc, reader);
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
			case TokenReader::kwWhile:
				reader.accept();
				return compileOpWhile(loc, reader);
			case TokenReader::kwFunction:
				reader.accept();
				return compileOpFunction(loc, reader);
			case TokenReader::kwEcho:
				reader.accept();
				return (new(alloc)OutputResult(loc, curOutputConfig, curOutputConfig->escMode))->setBranch(0,compileAssign(reader));
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
			case TokenReader::kwImport:
				reader.accept();
				return compileInclude(loc,reader);
			case TokenReader::kwTemplate:
				reader.accept();
				return compileTemplateCmd(loc, reader);
			case TokenReader::kwRepeat:
				reader.accept();
				return compileOpRepeatUntil(loc, reader);
			case TokenReader::kwVarname:
				reader.accept();
				return (new(alloc)Oper_Varname(loc))
					->setBranch(0, compileUNAR(reader));
			case TokenReader::symbMinus:
				reader.accept();
				return (new(alloc)Oper_Fn1(loc,&operUnarMinus))
					->setBranch(0, compileUNARSuffix(reader));
			case TokenReader::symbPlus:
				reader.accept();
				return  compileUNARSuffix(reader);
			case TokenReader::symbMult:
				reader.accept();
				return  (new(alloc)Oper_Dereference(loc))
					->setBranch(0, compileUNARSuffix(reader));
			case TokenReader::symbAmp:
				reader.accept();
				return (new(alloc)Oper_ReferenceOper(loc))
						->setBranch(0,compileUNARSuffix(reader));
			case TokenReader::symbOBrace:
				reader.accept();
				return compileJSONObject(loc, reader);
			case TokenReader::symbOSqBracket:
				reader.accept();
				return compileJSONArray(loc, reader);
			case TokenReader::symbOBracket:
				reader.accept();
				return compileSubExpr(loc, reader);
			case TokenReader::sValue:
				reader.accept();
				return new(alloc) Constant(loc, reader.value->clone(valueFactory));
			case TokenReader::kwTrue:
				reader.accept();
				return new(alloc)Constant(loc, valueFactory->newValue(true));
			case TokenReader::kwFalse:
				reader.accept();
				return new(alloc)Constant(loc, valueFactory->newValue(false));
			case TokenReader::kwNull:
				reader.accept();
				return new(alloc)Constant(loc, valueFactory->newValue(null));
			case TokenReader::sVarname:
				reader.accept();
				return new (alloc)VariableRef(loc, reader.varname);
			case TokenReader::symbBeginTemplate:
				reader.accept();
				return compileTemplateText(loc, reader);
			case TokenReader::kwConst:
				reader.accept();
				return compileConst(loc, reader);
			case TokenReader::kwInline:
				reader.accept();
				return compileInline(loc, reader);
			default: {
				TextFormatBuff<char> msg;
				msg("Unexpected '%1'") << kwList[reader.getNext()];
				throw ParseError(THISLOCATION, loc, msg.write());
			}
				


		}
	}

	PExprNode Compiler::compileUNARSuffix(TokenReader& reader)
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

	PExprNode Compiler::compileFunctOp(PExprNode nd, TokenReader& reader)
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

	PExprNode Compiler::compileArrayOp(PExprNode nd, TokenReader& reader)
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
				return (new(alloc)Oper_Fn2(loc,&fnArrIndex))->setBranch(0, nd)->setBranch(1, index);
			}
			else {
				throw ParseError(THISLOCATION, loc, "Expected ']'");
			}
		}
	}

	PExprNode Compiler::compileTemplate(TokenReader& reader, POutputConfig outputCfg) {
		CompilerContext ctx(curConstScope, valueFactory);

		POutputConfig md = curOutputConfig;
		curOutputConfig = outputCfg;
		try {
			return compileTemplateText(reader.getLocation(), reader);
			curOutputConfig = md;
		} catch (...) {
			curOutputConfig = md;
			throw;
		}
	}

	PExprNode Compiler::compileMemberAccessOp(PExprNode nd, TokenReader& reader)
	{
		ExprLocation loc = reader.getLocation();
		reader.accept();
		PExprNode vname = compileUNAR(reader);
		if (vname->getIfcPtr<LocalVarRef>() == 0) {
			throw ParseError(THISLOCATION, reader.getLocation(), "Right side of '.' must be an identifier");
		}
		return new(alloc)Oper_MemberAccess(reader.getLocation(), nd, vname);
	}

	Tempe::PExprNode Compiler::compileOpFirstDefined(ExprLocation loc, TokenReader &reader)
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

	Tempe::PExprNode Compiler::compileOpUnset(ExprLocation loc, TokenReader& reader)
	{
		PExprNode nd = compileUNARSuffix(reader);
		if (nd->getIfcPtr<IGetVarName>() == 0)
			throw ParseError(THISLOCATION, loc, "Expecting variable");
		return (new(alloc)Oper_Unset(loc))->setBranch(0, nd);
	}

	Tempe::PExprNode Compiler::compileIF(ExprLocation loc, TokenReader& reader)
	{
		reader.enterLevel();
		PExprNode cond = compileCondition(loc, reader);
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

	Tempe::PExprNode Compiler::compileSubExpr(ExprLocation loc, TokenReader &reader)
	{
		reader.enterLevel();
		PExprNode nd = compileExprSeq(reader);
		if (reader.getNext() != TokenReader::symbCBracket)
			throwExpectedError(loc, TokenReader::symbCBracket);
		reader.accept();
		reader.leaveLevel();
		return nd;
	}

	Tempe::PExprNode Compiler::compileOpTryCatch(ExprLocation , TokenReader& reader)
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

	Tempe::PExprNode Compiler::compileOpWith(ExprLocation , TokenReader& reader)
	{
		if (reader.getNext() == TokenReader::kwConst) {
			reader.accept();
			reader.enterLevel();
			PExprNode expr = compileUNARSuffix(reader);
			VariableRef *vref = expr->getIfcPtr<VariableRef>();
			if (vref && !curConstScope->varExists(vref->getName())) {
				curConstScope->setVar(vref->getName(), valueFactory->object());
			}
			Value constVar = expr->calculate(*curConstScope);
			IExprEnvironment *prevScope = curConstScope;
			LocalScope scope(*curConstScope, constVar);
			try {
				curConstScope = &scope;
				PExprNode nx = compileExprSeq(reader);
				curConstScope = prevScope;
				if (reader.getNext() == TokenReader::kwEnd) {
					reader.accept();
				}
				reader.leaveLevel();
				return nx;
			}
			catch (...) {
				reader.leaveLevel();
				curConstScope = prevScope;
				throw;
			}

		}
		else {
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
			}
			reader.leaveLevel();
			return (new(alloc)Oper_WithDo(reader.getLocation(), isol))
					->setBranch(0, expr)
					->setBranch(1, cmdlist);			
		}
	}

	Tempe::PExprNode Compiler::compileOpScope(ExprLocation, TokenReader& reader)
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


	Tempe::PExprNode Compiler::compileOpObject(ExprLocation , TokenReader& reader)
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

	Tempe::PExprNode Compiler::compileOpWhile(ExprLocation loc, TokenReader& reader)
	{
		reader.enterLevel();
		PExprNode cond = compileCondition(loc, reader);
		if (reader.getNext() == TokenReader::kwDo) {
			reader.accept();
			PExprNode body = compileExprSeq(reader);
			if (reader.getNext() == TokenReader::kwEnd) {
				reader.accept();
				reader.leaveLevel();
				ExprLocation loc = reader.getLocation();
				Value v1, v2;
				if (cond->tryToEvalConst(*curConstScope, v1)) {
					if (v1->getBool() == false) return new (alloc)Constant(loc, valueFactory->newValue(false));
					else if (body->tryToEvalConst(*curConstScope, v2))
						throw ParseError(THISLOCATION, loc, "Infinite cycle");
				}
				if (body->tryToEvalConst(*curConstScope, v2)) {
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

	Tempe::PExprNode Compiler::compileOpFunction(ExprLocation loc, TokenReader& reader)
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


	void Compiler::throwExpectedError(const ExprLocation &loc, ConstStringT<TokenReader::Symbol> symbols)
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

	void Compiler::throwUnexpectedError(const ExprLocation &loc, ConstStringT<TokenReader::Symbol> symbols)
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

	Tempe::PExprNode Compiler::compileTemplateText(ExprLocation loc, TokenReader& reader)
	{
	
		Value dict;
		if (curConstScope->varExists("dictionary")) {
			dict = curConstScope->getVar("dictionary");
		}
		buffer.clear();
		SourceReader &srcrd = reader;
		while (srcrd.hasItems()) {
			char c = srcrd.getNext();
			if (c == '$') {
				if (!srcrd.hasItems()) break;
				c = srcrd.peek();
				if (c == '$') buffer.add(srcrd.getNext());
				else if (c == '}') {
					srcrd.skip();
					return new(alloc)OutputText(loc, buffer);
				}
				else if (c == '{') {
					srcrd.skip();
					loc = reader.getLocation();
					PExprNode curTemplate = new(alloc)OutputText(loc, buffer);
					PExprNode expr = compileTemplateSubExpr(loc, reader);
					if (reader.getNext() == TokenReader::symbCBrace) {
						reader.accept();
						PExprNode subcmd = (new(alloc)Oper_Comma(loc))
							->setBranch(0, curTemplate)
							->setBranch(1, expr);
						loc = reader.getLocation();
						PExprNode nxtTemplate = compileTemplateText(loc, reader);
						PExprNode subcmd2 = (new(alloc)Oper_Comma(loc))
							->setBranch(0, subcmd)
							->setBranch(1, nxtTemplate);
						return subcmd2;
					}
					else {
						throwExpectedError(loc, TokenReader::symbCBrace);
					}
				}
				else {
					TokenReader::Symbol s = reader.getNext();
					ConstStrA var;
					if (s == TokenReader::sVarname) {
						var = reader.varname;
					}
					else {
						var = kwList[s];
					}
					reader.accept();
					
					JSON::INode *nd;
					if (dict != nil && (nd = dict->getPtr(var)) != 0) {
						buffer.append(nd->getStringUtf8());					
					}
					else {
						buffer.append(var);
					}
				}
			}
			else {
				buffer.add(c);
			}
		} 
		return new(alloc) OutputText(loc, buffer);
	}



	Tempe::PExprNode Compiler::compileTemplateSubExpr(ExprLocation loc, TokenReader& reader)
	{
		TokenReader::Symbol s = reader.getNext();
		EscapeMode escMode;
		if (s == TokenReader::sVarname && reader.peek()==':') {
			 escMode = strEscapeMode[reader.varname];
			reader.accept();
			if (reader.getNext() != TokenReader::symbDblColon) {
				throwExpectedError(reader.getLocation(), TokenReader::symbDblColon);
			}
			reader.accept();			
		}
		else {
			escMode = curOutputConfig->escMode;
		}
		PExprNode expr = compileExprSeq(reader);
		return (new(alloc)OutputResult(loc, curOutputConfig, escMode))->setBranch(0, expr);

		
		
	}

	//foreach x print(this.neco) end

	Tempe::PExprNode Compiler::compileForEach(ExprLocation loc, TokenReader& reader)
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


	template<typename K>
	void glDetachDetectObjCycle(K &container) {
		natural l = container.length();
		JSON::INode *i = container[l-1];

		GCReg *rg = i->getIfcPtr<GCReg>();
		if (rg && rg->isRegistered()) {

			for (JSON::Iterator iter = i->getFwIter(); iter.hasItems();) {
				JSON::INode *n = iter.getNext().node;
				if (n->isObject() || n->isArray()) {
					for (natural z = 0; z < l; z++)
						if (n == container[z]) throw ErrorMessageException(THISLOCATION, "Cycle detected in const object. Cyclic references between const objects are not supported when they are inserted into the code");

					container.add(n);
					glDetachDetectObjCycle(container);
					container.trunc(1);
				}
			}

			rg->unregisterFromGC();
		}

		
	}

	static Value gcDetach(Value param1)
	{
		AutoArray<JSON::INode *, SmallAlloc<32> >container;
		container.add(param1);
		glDetachDetectObjCycle(container);
		return param1;
	}


	Tempe::PExprNode Compiler::compileConst(ExprLocation loc, TokenReader& reader)
	{
		PExprNode expr = compileAssign(reader);

		try {
			LocalScope scope(static_cast<IExprEnvironment &>(*curConstScope));
			return new(alloc)Constant(loc, gcDetach(expr->calculate(*curConstScope)));
		}
		catch (LightSpeed::Exception &e) {
			throw ParseError(THISLOCATION, loc, "Cannot insert constant") << e;
		}
	}

	Tempe::PExprNode Compiler::compileTemplateCmd(ExprLocation loc, TokenReader& reader)
	{
		PExprNode jsonCmd = compileUNARSuffix(reader);
		Value obj = jsonCmd->calculate(*curConstScope);

		if (!obj->isObject()) {
			throw ParseError(THISLOCATION, loc, "Template configuration must be object");
		}

		POutputConfig newcfg = new OutputConfig(*curOutputConfig);
		POutputConfig oldcfg = curOutputConfig;
		try {
			newcfg->load(obj);
		}
		catch (LightSpeed::Exception &e) {
			throw ParseError(THISLOCATION, loc, "Failed to parse template configuration") << e;
		}

		try {
			curOutputConfig = newcfg;
			PExprNode expr = compileExprSeq(reader);
			curOutputConfig = oldcfg;
			if (reader.getNext() == TokenReader::kwEnd)
				reader.accept();
			return expr;
		}
		catch (...) {
			curOutputConfig = oldcfg;
			throw;
		}
	}

PExprNode Compiler::compileInclude(ExprLocation loc, TokenReader& reader) {
		PExprNode expr = compileAssign(reader);

		LocalScope scope(static_cast<IExprEnvironment &>(*curConstScope));
		Value v = expr->calculate(scope);
		ConstStrA name = v->getStringUtf8();
		try {
			std::pair<PExprNode,FilePath> nx = loadCode(loc,name);
			return new(alloc) Oper_IncludeTrace(loc,nx.second,nx.first);
		} catch (LightSpeed::Exception &e) {
			throw ParseError(THISLOCATION, loc, ConstStrA("Unable to open file: ") + name)
					<< e;
		}
}

EscapeMode Compiler::getCtFromMime(ConstStrA contentType) {
	return strMimeCt[contentType];
}

std::pair<PExprNode,FilePath> Compiler::loadCode(ExprLocation , ConstStrA ) {
	throw ErrorMessageException(THISLOCATION, "No source repository is available");
}

PExprNode Compiler::compileCondition(ExprLocation loc, TokenReader& reader) {
	PExprNode cond = compileOR(reader);
	if (reader.getNext() == TokenReader::symbAsssign) {
		throwUnexpectedError(loc,reader.getNext());
	}
	Oper_Assign *assign = cond->getIfcPtr<Oper_Assign>();
	if (assign != 0) {
		throwUnexpectedError(loc,TokenReader::symbAsssign);
	}
	return cond;
}

PExprNode Compiler::compileOpRepeatUntil(ExprLocation loc, TokenReader& reader) {
	reader.enterLevel();
	PExprNode body = compileExprSeq(reader);
	reader.leaveLevel();
	if (reader.getNext() != TokenReader::kwUntil) {
		throwExpectedError(loc,TokenReader::kwUntil);
	}
	reader.accept();

	PExprNode cond = compileCondition(reader.getLocation(), reader);
	PExprNode ncond = (new (alloc) Oper_Fn1(reader.getLocation(),&operUnarNot))
			->setBranch(0,cond);
	PExprNode semicol = (new (alloc) Oper_Comma(reader.getLocation()))
			->setBranch(0,body)->setBranch(1,ncond);
	return (new (alloc) Oper_Cycle(loc))->setBranch(0,semicol);

}

Tempe::PExprNode Compiler::compileJSONObject(ExprLocation loc, TokenReader& reader)
{
	Value constObj = valueFactory->object();
	PExprNode root = nil;
	reader.enterLevel();
	if (reader.getNext() != TokenReader::symbCBrace) {
		bool willcontinue;
		do {
			loc = reader.getLocation();
			StringA varname;
			if (reader.getNext() == TokenReader::sValue) {
				reader.accept();
				varname = reader.value->getStringUtf8();
			}
			else if (reader.getNext() == TokenReader::sVarname) {
				reader.accept();
				varname = reader.varname;
			}
			else if (reader.getNext() == TokenReader::kwConst) {
				reader.accept();
				PExprNode expr = compileAssign(reader);

				LocalScope scope(static_cast<IExprEnvironment &>(*curConstScope));
				varname = expr->calculate(scope)->getStringUtf8();
			}
			else {
				TokenReader::Symbol expected[] = { TokenReader::sValue,
					TokenReader::sVarname, TokenReader::kwConst};
				throwExpectedError(loc, ConstStringT<TokenReader::Symbol>(expected, 3));
			}
			if (reader.getNext() != TokenReader::symbDblColon) {
				throwExpectedError(loc, TokenReader::symbDblColon);
			}
			reader.accept();
			PExprNode expr = compileOR(reader);

			PExprNode assign = (new(alloc)Oper_Assign(loc))
				->setBranch(0, new (alloc)VariableRef(loc, varname))
				->setBranch(1, expr);
			if (root != nil) {
				root = (new(alloc)Oper_Comma(loc))->setBranch(0, root)->setBranch(1, assign);
			}
			else {
				root = assign;
			}

			Value right;
			if (constObj != nil) {
				if (expr->tryToEvalConst(*curConstScope, right)) {
					constObj->add(varname, right);
				}
				else {
					constObj = nil;
				}
			}
				

			loc = reader.getLocation();
			if (reader.getNext() != TokenReader::symbComma && reader.getNext() != TokenReader::symbCBrace) {
				TokenReader::Symbol expected[] = { TokenReader::symbComma,
					TokenReader::symbCBrace };
				throwExpectedError(loc, ConstStringT<TokenReader::Symbol>(expected, 2));
			}
			willcontinue = (reader.getNext() == TokenReader::symbComma);
			reader.accept();
		} while (willcontinue);
	}
	else {
		reader.accept();
	}
	reader.leaveLevel();
	if (constObj != nil) return new (alloc)Constant(loc, constObj);
	else return (new(alloc)Oper_Object(loc))->setBranch(0, root);
}

Tempe::PExprNode Compiler::compileJSONArray(ExprLocation loc, TokenReader& reader)
{
	Value constObj = valueFactory->array();
	AutoArray<PExprNode> args;
	reader.enterLevel();
	if (reader.getNext() != TokenReader::symbCSqBracket) {
		bool willcontinue;
		do {
			loc = reader.getLocation();
			PExprNode expr = compileOR(reader);

			args.add(expr);

			Value right;
			if (constObj != nil) {
				if (expr->tryToEvalConst(*curConstScope, right)) {
					constObj->add(right);
				}
				else {
					constObj = nil;
				}
			}


			loc = reader.getLocation();
			if (reader.getNext() != TokenReader::symbComma && reader.getNext() != TokenReader::symbCSqBracket) {
				TokenReader::Symbol expected[] = { TokenReader::symbComma,
					TokenReader::symbCSqBracket };
				throwExpectedError(loc, ConstStringT<TokenReader::Symbol>(expected, 2));
			}
			willcontinue = (reader.getNext() == TokenReader::symbComma);
			reader.accept();
		} while (willcontinue);
	}
	else {
		reader.accept();
	}
	reader.leaveLevel();
	if (constObj != nil) return new (alloc)Constant(loc, constObj);
	else  {
		RefCntPtr<Oper_ArrayCreate> out = new(alloc)Oper_ArrayCreate(loc);
		out->setBranches(args);
		return out.get();
	}
	

}

Tempe::PExprNode Compiler::compileSingleExpression(TokenReader &reader)
{
	return compileOR(reader);
}

Tempe::PExprNode Compiler::compileInline(ExprLocation loc, TokenReader& reader)
{
	PExprNode nd = compileSingleExpression(reader);

	Value v = nd->calculate(*curConstScope);
	FunctionVar *fv = v->getIfcPtr<FunctionVar>();
	if (fv) {
		return fv->getCode();
	}
	else {
		throw ParseError(THISLOCATION, loc, "You cannot inline such object. Only function can be subject of inline");
	}
}

}
