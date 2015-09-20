/*
 * parser.cpp
 *
 *  Created on: 4.7.2012
 *      Author: ondra
 */

#include "compiler.h"
#include "basicOps.h"
#include "functions.h"
#include "exceptions.h"
#include "lightspeed/base/interface.tcc"
#include <lightspeed/base/text/textstream.tcc>
#include <lightspeed/utils/json/jsonfast.tcc>

#include "functionVar.h"
namespace Tempe {


Compiler::Compiler():legacyFunctions(false),alloc(StdAlloc::getInstance()),valueFactory(JSON::create(StdAlloc::getInstance())),level(0) {}

Compiler::Compiler(IRuntimeAlloc& alloc):legacyFunctions(false),alloc(alloc),valueFactory(JSON::create(alloc)),level(0) {}

Compiler::LevelControl::LevelControl(SourceReader& reader, natural &level):reader(reader),level(level) {
	level++;
	if (level == 1) {
		reader.setWS(" \r\n\t");
	}
}
Compiler::LevelControl::~LevelControl() {
	if (level == 1) {
		reader.setWS(" \t");
	}
	level--;
}

Compiler::NegLevelControl::NegLevelControl(SourceReader& reader, natural &level):reader(reader),level(level) {
	if (level == 1) {
		reader.setWS(" \t");
	}
	level--;
}
Compiler::NegLevelControl::~NegLevelControl() {
	level++;
	if (level == 1) {
		reader.setWS(" \r\n\t");
	}
}


PExprNode Compiler::compile(SourceReader& reader) {


	PExprNode nd = compileAssign(reader);
	if (reader(" ;%")) {
		reader("%(*)[ \t\n\r\b]%%");
		//solves ; before endif and any ; after endif
		if (reader.peek(" endif %") || reader.peek(" done %") || reader.peek(" end %") || reader.peek(" else %") || reader.peek(" catch %"))
			return nd;
		ExprLocation loc = reader.getLocation();
		PExprNode nd2 = compile(reader);
		return (new(alloc) Oper_Comma(loc))
			->setBranch(0,nd)->setBranch(1,nd2);

	} else {
		return nd;
	}
}

PExprNode Compiler::compileAssign(SourceReader& reader) {
	PExprNode nd = compileOR(reader);
	if (reader(" :=%")) {
		ExprLocation loc = reader.getLocation();
		if (nd->getIfcPtr<IGetVarName>() == 0)
			throw ParseError(THISLOCATION,loc,"There must be a variable before ':='");
		PExprNode nd2 = compileAssign(reader);
		return (new(alloc) Oper_Assign(loc))
				->setBranch(0,nd)->setBranch(1,nd2);
	} else {
		return nd;
	}
}

PExprNode Compiler::compileOR(SourceReader& reader) {
	PExprNode nd = compileAND(reader);
	if (reader(" or\b%")) {
		ExprLocation loc = reader.getLocation();
		PExprNode nd2 = compileOR(reader);
		return (new(alloc) Oper_Or(loc))
			->setBranch(0,nd)->setBranch(1,nd2);
	} else {
		return nd;
	}
}

PExprNode Compiler::compileAND(SourceReader& reader) {
	PExprNode nd = compileRELAT(reader);
	if (reader(" and\b%")) {
		ExprLocation loc = reader.getLocation();
		PExprNode nd2 = compileAND(reader);
		return (new(alloc) Oper_And(loc))
			->setBranch(0,nd)->setBranch(1,nd2);
	} else {
		return nd;
	}
}

PExprNode Compiler::compileRELAT(SourceReader& reader) {
	PExprNode nd = compilePLUSMINUS(reader);
	Oper_Fn2::Fn fnpick;
	if (reader(" >=%")) fnpick = &operGreatEqual;
	else if (reader(" <=%")) fnpick = &operLessEqual;
	else if (reader(" !=%")) fnpick = &operNotEqual;
	else if (reader(" <>%")) fnpick = &operNotEqual;
	else if (reader(" ==%")) fnpick = &operEqual;
	else if (reader(" =%")) fnpick = &operEqual;
	else if (reader(" >%")) fnpick = &operGreater;
	else if (reader(" <%")) fnpick = &operLess;
	else return nd;

	ExprLocation loc = reader.getLocation();
	PExprNode nd2 = compileRELAT(reader);

	return (new(alloc) Oper_Fn2(loc,fnpick)) ->setBranch(0,nd)->setBranch(1,nd2);
}

PExprNode Compiler::compilePLUSMINUS(SourceReader& reader) {
	PExprNode nd = compileMULTDIV(reader);
	Oper_Fn2::Fn fnpick;
	if (reader(" +%")) fnpick = &operPlus;
	else if (reader(" &%")) fnpick = &operStringAppend;
	else if (reader(" -%")) fnpick = &operMinus;
	else return nd;

	ExprLocation loc = reader.getLocation();
	PExprNode nd2 = compilePLUSMINUS(reader);

	return (new(alloc) Oper_Fn2(loc,fnpick)) ->setBranch(0,nd)->setBranch(1,nd2);
}

PExprNode Compiler::compileMULTDIV(SourceReader& reader) {
	PExprNode nd = compileUNARSuffix(reader);
	Oper_Fn2::Fn fnpick;
	if (reader(" //%")) fnpick = &operIntegerDiv;
	else if (reader(" *%")) fnpick = &operMul;
	else if (reader(" /%")) fnpick = &operDiv;
	else if (reader(" %%%")) fnpick = &operMod;
	else return nd;

	ExprLocation loc = reader.getLocation();
	PExprNode nd2 = compileMULTDIV(reader);

	return (new(alloc) Oper_Fn2(loc,fnpick)) ->setBranch(0,nd)->setBranch(1,nd2);
}

PExprNode Compiler::compileUNARSuffix(SourceReader& reader) {
	PExprNode nd = compileUNAR(reader);
	do {
		if (reader(" (%")) {

			VariableRef *vname = nd->getIfcPtr<VariableRef >();

			LevelControl levControl(reader,level);
			ExprLocation loc = reader.getLocation();
			PNaryNode fn = vname?createFunction(reader.getLocation(),vname->getName()):new(alloc) Oper_FunctionCall(loc,nd);
			natural cnt = fn->getN();
			if (cnt == naturalNull) {
				natural i = 0;
				if (!reader(" )%")) do {
					PExprNode subnd = compile(reader);
					fn->setBranch(i,subnd);
					i++;
					if (reader(" )%")) break;
					else if (!reader(" ,%"))
						throw ParseError(THISLOCATION,reader.getLocation(),"Expecting ',' or ')'");
				} while (true);
			} else {
				for (natural i = 0, cnt = fn->getN(); i < cnt; i++) {
					PExprNode subnd = compile(reader);
					fn->setBranch(i,subnd);
					if (i + 1 == cnt) {
						if (!reader(" )%"))
							throw ParseError(THISLOCATION,reader.getLocation(),"Expecting ')'");
					} else {
						if (!reader(" ,%"))
							throw ParseError(THISLOCATION,reader.getLocation(),"Expecting ','");
					}
				}
			}
			nd = fn.get();

		} else if (reader(" []%")) {
			nd = (new(alloc) Oper_ArrayAppend(reader.getLocation()))->setBranch(0,nd);
		} else if (reader(" [%")) {
			LevelControl levControl(reader,level);
			PExprNode index = compile(reader);
			if (reader(" ]%")) {
				nd = (new(alloc) Oper_ArrayIndex(reader.getLocation()))->setBranch(0,nd)->setBranch(1,index);
			} else {
				throw ParseError(THISLOCATION,reader.getLocation(),"Expected ']'");
			}
		} else if (reader(" .%")) {

			LevelControl levControl(reader,level);
			PExprNode vname = compileUNAR(reader);
			if (vname->getIfcPtr<LocalVarRef>() == 0) {
				throw ParseError(THISLOCATION,reader.getLocation(),"Right side of '.' must be an identifier");
			}
			Oper_MemberAccess *ma;
			PExprNode pma = ma = new(alloc) Oper_MemberAccess(reader.getLocation(), nd, vname);
			nd = pma;

		} else {
			return nd;
		}
	} while (true);

}


PExprNode Compiler::compileIF(SourceReader& reader) {
	LevelControl levControl(reader,level);
	PExprNode cond = compile(reader);
	if (cond == nil) throw ParseError(THISLOCATION,reader.getLocation(),"Unexcpected end of code after if");
	if (reader(" then %")) {
		PExprNode thenPart = compile(reader);
		if (thenPart == nil) throw ParseError(THISLOCATION,reader.getLocation(),"Unexcpected end of code after then");
		if (reader(" elseif %")) {
			NegLevelControl levControl(reader,level);
			PExprNode elsePart = compileIF(reader);
			return (new(alloc) Oper_If(reader.getLocation()))->setBranch(0,cond)->setBranch(1,thenPart)->setBranch(2,elsePart);
		} else if (reader(" else %")) {
			PExprNode elsePart = compile(reader);
			if (elsePart == nil) throw ParseError(THISLOCATION,reader.getLocation(),"Unexcpected end of code after else");
			NegLevelControl levControl(reader,level);
			if (!reader("%(*)[\r\n\t ]%end %")) throw ParseError(THISLOCATION,reader.getLocation(),"Expecting endif");
			return (new(alloc) Oper_If(reader.getLocation()))->setBranch(0,cond)->setBranch(1,thenPart)->setBranch(2,elsePart);
		} else {
			NegLevelControl levControl(reader,level);
			if (!reader("%(*)[\r\n\t ]%end %")) throw ParseError(THISLOCATION,reader.getLocation(),"Expecting endif");
			return (new(alloc) Oper_And(reader.getLocation()))->setBranch(0,cond)->setBranch(1,thenPart);
		}
	} else {
		throw ParseError(THISLOCATION,reader.getLocation(),"'If' without 'then'");
	}

}
PExprNode Compiler::compileUNAR(SourceReader& reader) {

	if (reader(" not\b%") || reader(" !%")) {
		PExprNode nd = compileUNARSuffix(reader);
		return (new(alloc) Oper_Fn1(reader.getLocation(),&operUnarNot))->setBranch(0,nd);
	} else if (reader(" defined\b%")) {
		PExprNode nd = compileUNARSuffix(reader);
		return (new(alloc) Oper_Exist(reader.getLocation()))->setBranch(0,nd);
	} else if (reader(" firstDefined\b%")) {
		Oper_FirstDefined *fd;
		PExprNode fn = fd = new(alloc) Oper_FirstDefined(reader.getLocation());

		natural i = 0;
		do {
			PExprNode nd = compileUNARSuffix(reader);
			fd->setBranch(i,nd);
		} while (reader(" ,%"));
		return fn;
	} else if (reader(" unset\b%")) {
		PExprNode nd = compileUNARSuffix(reader);
		if (nd->getIfcPtr<IGetVarName>() == 0)
			throw ParseError(THISLOCATION,reader.getLocation(),"Expecting variable");
		return (new(alloc) Oper_Unset(reader.getLocation()))->setBranch(0,nd);
	} else if (reader(" &%")) {
		PExprNode nd = compileUNARSuffix(reader);
		return (new(alloc) Oper_Link(reader.getLocation()))->setBranch(0,nd);
	} else if (reader(" var\b%")) {
		PExprNode nd = compileUNARSuffix(reader);
		return (new(alloc) Oper_Var(reader.getLocation()))->setBranch(0,nd);
	} else if (reader(" new\b%")) {
		PExprNode nd = compileUNARSuffix(reader);
		return (new(alloc) Oper_New(reader.getLocation()))->setBranch(0,nd);
	} else if (reader(" isnull\b%")) {
		PExprNode nd = compileUNARSuffix(reader);
		return (new(alloc) Oper_IsNull(reader.getLocation()))->setBranch(0,nd);
	} else if (reader(" loop\b%")) {
		PExprNode nd = compileUNARSuffix(reader);
		return (new(alloc) Oper_Cycle(reader.getLocation()))->setBranch(0,nd);
	} else if (reader(" if\b%")) {
		return compileIF(reader);
	} else if (reader(" try\b%")) {
		LevelControl levControl(reader,level);
		PExprNode cond = compile(reader);
		if (cond == nil) throw ParseError(THISLOCATION,reader.getLocation(),"Unexcpected end of code after try");
		if (reader(" catch %")) {
			PExprNode var = compileUNARSuffix(reader);
			PExprNode catchPart = compile(reader);
			NegLevelControl levControl(reader,level);
			if (!reader("%(*)[\r\n\t ]%end %")) throw ParseError(THISLOCATION,reader.getLocation(),"Expecting end");
			return (new(alloc) Oper_TryCatch(reader.getLocation()))->setBranch(0,cond)
					->setBranch(1,var)->setBranch(2,catchPart);
		} else {
			throw ParseError(THISLOCATION,reader.getLocation(),"Expecting catch");
		}
	} else if (reader(" with\b%")) {
		LevelControl levControl(reader,level);
		PExprNode expr = compileUNARSuffix(reader);
		PExprNode cmdlist;
		if (reader(" join\b%")) {
			cmdlist = new(alloc) Oper_WithDoJoin(reader.getLocation(),compile(reader));
		} else if (reader(" map\b%")) {
				cmdlist = new(alloc) Oper_WithDoMap(reader.getLocation(),compile(reader));
		} else {
			cmdlist = compile(reader);
		}
		NegLevelControl _levControl(reader,level);
		if (!reader("%(*)[\r\n\t ]%end %")) throw ParseError(THISLOCATION,reader.getLocation(),"Expecting end");
		return (new(alloc) Oper_WithDo(reader.getLocation()))->setBranch(0,expr)
				->setBranch(1,cmdlist);
	} else if (reader(" json%")) {
		LevelControl levControl(reader,level);
		JSON::PNode nd = JSON::parseFast(reader.nxChain());
		return new(alloc) Constant(reader.getLocation(),nd);
	} else if (reader(" scope%")) {
		LevelControl levControl(reader,level);
 		PExprNode body = compile(reader);
		NegLevelControl _levControl(reader,level);
		if (!reader("%(*)[\r\n\t ]%end %")) throw ParseError(THISLOCATION,reader.getLocation(),"Expecting end");
		return (new(alloc) Oper_Scope(reader.getLocation()))->setBranch(0,body);
	} else if (reader(" while\b%")) {
		LevelControl levControl(reader,level);
		PExprNode cond = compile(reader);
		if (cond == nil) throw ParseError(THISLOCATION,reader.getLocation(),"Unexcpected end of code after while");
		if (reader(" do %")) {
			PExprNode body = compile(reader);
			if (body == nil) throw ParseError(THISLOCATION,reader.getLocation(),"Unexcpected end of code after do");
			NegLevelControl levControl(reader,level);
			if (!reader("%(*)[\r\n\t ]%done %")) throw ParseError(THISLOCATION,reader.getLocation(),"Expecting done");
			ExprLocation loc = reader.getLocation();
			PExprNode semcol = (new(alloc) Oper_Comma(loc))->setBranch(0,body)->setBranch(1,cond);
			PExprNode loopCmd = (new(alloc) Oper_Cycle(loc))->setBranch(0,semcol);
			PExprNode andCmd =  (new(alloc) Oper_And(loc))->setBranch(0,cond)->setBranch(1,loopCmd);
			return andCmd;
		} else {
			throw ParseError(THISLOCATION,reader.getLocation(),"Expecting do");
		}
	} else if (reader(" function (%")) {
		AutoArray<FunctionVar::VarName_OutMode> varlist;
		LevelControl levControl(reader,level);
		ExprLocation startloc = reader.getLocation();
		bool opt = false;
		if (!reader(" )%")) do {
			ExprLocation loc = startloc;
			if (reader(" in\b%i1 ... )%") || reader(" in\b%q``1 ... )%")) {
				varlist.add(FunctionVar::VarName_OutMode(reader[1].str(),FunctionVar::variadic));
				break;
			} else if (reader(" in\b%i1%%") || reader(" in\b%q``1%%"))
				varlist.add(FunctionVar::VarName_OutMode(reader[1].str(),opt?FunctionVar::optional:FunctionVar::byValue));
			else if (reader(" out\b%i1%%") || reader(" out\b%q``1%%"))
				varlist.add(FunctionVar::VarName_OutMode(reader[1].str(),opt?FunctionVar::optionalReference:FunctionVar::byReference));
			else if (reader(" optional\b%")) {
				opt = true; continue;
			} else
				throw ParseError(THISLOCATION,loc,"Expecting keyword 'in' or 'out' with identifier");

			loc = reader.getLocation();
			if (reader(" )%")) break;
			if (!reader(" ,%")) throw ParseError(THISLOCATION,loc,"Expecting ',' or ')'");
		} while (true);
		PExprNode nd = compile(reader);
		NegLevelControl levControl2(reader,level);
		if (!reader("%(*)[\r\n\t ]%end%")) throw ParseError(THISLOCATION,reader.getLocation(),"Expecting keyword 'end'");
		return new(alloc) Constant(startloc,Value(new(alloc) FunctionVar(varlist,nd)));
	} else if (reader(" break\b%")) {
		return (new(alloc) Oper_Break(reader.getLocation()));
	} else if (reader(" throw\b%")) {
		PExprNode cond = compileUNARSuffix(reader);
		return (new(alloc) Oper_Throw(reader.getLocation()))->setBranch(0,cond);
	} else if (reader(" -%")) {
		PExprNode nd = compileUNARSuffix(reader);
		return (new(alloc) Oper_Fn1(reader.getLocation(),&operUnarMinus))->setBranch(0,nd);
	} else if (reader(" +%")) {
		return compileUNARSuffix(reader);
//	} else if (reader(" %(1,1)[a-zA-Z_](*)[a-zA-Z0-9._]1% (%")) {
	} else if (reader(" %(1,1)[a-zA-Z_](*)[a-zA-Z0-9_]1%%")) {
		ConstStrA idf = reader[1].str();
		ExprLocation loc = reader.getLocation();
		return createConstant(loc,idf);
	} else if (reader(" %(*)q``1%%")) {
		ConstStrA idf = reader[1].str();
		ExprLocation loc = reader.getLocation();
		return createConstant(loc,idf);
	} else if (reader(" %(*)q\"\"1%%")) {
		return new(alloc) Constant(reader.getLocation(),adjustStringValue(reader[1].str()));
	} else if (reader(" %f1%%")) {
		TextParser<char,SmallAlloc<256> > p2;
		ConstStrA nr = reader[1];
		if (p2("%u1",nr)) return new(alloc) Constant(reader.getLocation(),valueFactory->newValue((natural)reader[1]));
		else return new(alloc) Constant(reader.getLocation(),valueFactory->newValue((double)reader[1]));
	} else if (reader(" (%")) {
		LevelControl levControl(reader,level);
		PExprNode nd = compile(reader);
		if (reader(" )%")) return nd;
		throw ParseError(THISLOCATION,reader.getLocation(),"Missing ')'");
	}
	throw ParseError(THISLOCATION,reader.getLocation(),"Unexpected token\n");
}

Value Compiler::adjustStringValue(ConstStrA text) {
	return valueFactory->newValue(String(text));
}


Compiler::PNaryNode Compiler::createFunction(ExprLocation loc, ConstStrA name) {

	if (legacyFunctions) {

		if (name == ConstStrA("contain")) return new(alloc) Oper_Fn2(loc,&fnContain);
		if (name == ConstStrA("containWord")) return new(alloc) Oper_Fn2(loc,&fnContainWord);
		if (name == ConstStrA("containExact")) return new(alloc) Oper_Fn2(loc,&fnContainExact);
		if (name == ConstStrA("containWordExact")) return new(alloc) Oper_Fn2(loc,&fnContainWordExact);
		if (name == ConstStrA("head")) return new(alloc) Oper_Fn2(loc,&fnHead);
		if (name == ConstStrA("tail")) return new(alloc) Oper_Fn2(loc,&fnTail);
		if (name == ConstStrA("offset")) return new(alloc) Oper_Fn2(loc,&fnOffset);
		if (name == ConstStrA("roffset")) return new(alloc) Oper_Fn2(loc,&fnRoffset);
		if (name == ConstStrA("splitAt")) return new(alloc) Oper_Fn3(loc,&fnSplitAt);
		if (name == ConstStrA("rsplitAt")) return new(alloc) Oper_Fn3(loc,&fnRsplitAt);
		if (name == ConstStrA("toString")) return new(alloc) Oper_Fn1(loc,&fnToString);
		if (name == ConstStrA("toInt")) return new(alloc) Oper_Fn1(loc,&fnToInt);
		if (name == ConstStrA("toReal")) return new(alloc) Oper_Fn1(loc,&fnToReal);
		if (name == ConstStrA("round")) return new(alloc) Oper_Fn1(loc,&fnRound);
		if (name == ConstStrA("floor")) return new(alloc) Oper_Fn1(loc,&fnFloor);
		if (name == ConstStrA("ceil")) return new(alloc) Oper_Fn1(loc,&fnCeil);
		if (name == ConstStrA("exp")) return new(alloc) Oper_Fn1(loc,&fnExp);
		if (name == ConstStrA("pow")) return new(alloc) Oper_Fn2(loc,&fnPow);
		if (name == ConstStrA("sqrt")) return new(alloc) Oper_Fn1(loc,&fnSqrt);
		if (name == ConstStrA("sin")) return new(alloc) Oper_Fn1(loc,&fnSin);
		if (name == ConstStrA("cos")) return new(alloc) Oper_Fn1(loc,&fnCos);
		if (name == ConstStrA("tan")) return new(alloc) Oper_Fn1(loc,&fnTan);
		if (name == ConstStrA("asin")) return new(alloc) Oper_Fn1(loc,&fnASin);
		if (name == ConstStrA("acos")) return new(alloc) Oper_Fn1(loc,&fnACos);
		if (name == ConstStrA("atan")) return new(alloc) Oper_Fn1(loc,&fnATan);
		if (name == ConstStrA("atan2")) return new(alloc) Oper_Fn2(loc,&fnATan2);
		if (name == ConstStrA("log")) return new(alloc) Oper_Fn1(loc,&fnLog);
		if (name == ConstStrA("log10")) return new(alloc) Oper_Fn1(loc,&fnLog10);
		if (name == ConstStrA("typeOf")) return new(alloc) Oper_Fn1(loc,&fnTypeOf);
		if (name == ConstStrA("typeof")) return new(alloc) Oper_Fn1(loc,&fnTypeOf);
		if (name == ConstStrA("charAt")) return new(alloc) Oper_Fn2(loc,&fnCharAt);
		if (name == ConstStrA("code")) return new(alloc) Oper_Fn1(loc,&fnCode);
		if (name == ConstStrA("length")) return new(alloc) Oper_Fn1(loc,&fnLength);
		if (name == ConstStrA("replace")) return new(alloc) Oper_Fn4(loc,&fnReplace);
		if (name == ConstStrA("exec")) return new(alloc) Oper_Exec(loc);
		if (name == ConstStrA("debug")) return new(alloc) Oper_LogOut(loc);
		if (name == ConstStrA("print")) return new(alloc) Oper_PrintOut(loc);
		if (name == ConstStrA("scan")) return new(alloc) Oper_FnParse(loc);
		if (name == ConstStrA("chr")) return new(alloc) Oper_FnChr(loc);
	}
	return new(alloc) Oper_FunctionCall(loc,new (alloc) VariableRef(loc,name));
/*
	throw ParseError(THISLOCATION,loc,String(ConstStrA("Function is not known: ")+name));
	*/
}

PExprNode Compiler::createConstant(ExprLocation loc, ConstStrA name) {
	if (name == ConstStrA("true"))
		return new (alloc) Constant(loc, valueFactory->newValue(true));

	if (name == ConstStrA("false"))
		return new (alloc) Constant(loc, valueFactory->newValue(false));

	if (name == ConstStrA("pi"))
		return new (alloc) Constant(loc, valueFactory->newValue(3.1415926535));

	if (name == ConstStrA("null"))
		return new (alloc) Constant(loc, valueFactory->newNullNode());

	return new (alloc) VariableRef(loc, name);
}/* namespace LightSpeedTest */

}

