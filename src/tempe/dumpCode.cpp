/*
 * dumpCode.cpp
 *
 *  Created on: Sep 23, 2015
 *      Author: ondra
 */

#include "dumpCode.h"

#include <lightspeed/base/interface.tcc>
#include "interfaces.h"
#include "basicOps.h"
#include "functionVar.h"
#include "lightspeed/base/text/toString.h"


#include "tempeOps.h"
#include "functions.h"
#include <lightspeed/base/namedEnum.tcc>
namespace Tempe {

	using namespace LightSpeed;

	AbstractDumpCode::AbstractDumpCode() :labelCounter(0), jsonf(JSON::create())
	{

	}


#define internalFnName(op) if (addr == &op) return "BUILDIN "#op;

	static ConstStrA getFnName(Value (* addr)(IExprEnvironment&, const Value&) ) {

		internalFnName(operUnarMinus);
		internalFnName(operUnarNot);
		internalFnName(fnToString);
		internalFnName(fnToInt);
		internalFnName(fnToReal);
		internalFnName(fnRound);
		internalFnName(fnFloor);
		internalFnName(fnCeil);
		internalFnName(fnExp);
		internalFnName(fnSqrt);
		internalFnName(fnSin);
		internalFnName(fnCos);
		internalFnName(fnTan);
		internalFnName(fnASin);
		internalFnName(fnACos);
		internalFnName(fnATan);
		internalFnName(fnLog);
		internalFnName(fnLog10);
		internalFnName(fnTypeOf);
		internalFnName(fnRand);
		internalFnName(fnCode);
		internalFnName(fnLength);
		return "<unknown>";

	}

	static ConstStrA getFnName(Value (* addr)(IExprEnvironment&, const Value&, const Value&) ) {

		internalFnName(operEqual);
		internalFnName(operGreater);
		internalFnName(operLess);
		internalFnName(operGreatEqual);
		internalFnName(operLessEqual);
		internalFnName(operNotEqual);
		internalFnName(operPlus);
		internalFnName(operMinus);
		internalFnName(operMul);
		internalFnName(operDiv);
		internalFnName(operMod);
		internalFnName(operIntegerDiv);
		internalFnName(operStringAppend);
		internalFnName(fnContain);
		internalFnName(fnContainWord);
		internalFnName(fnContainExact);
		internalFnName(fnContainWordExact);
		internalFnName(fnHead);
		internalFnName(fnTail);
		internalFnName(fnOffset);
		internalFnName(fnRoffset);
		internalFnName(fnPow);
		internalFnName(fnATan2);
		internalFnName(fnCharAt);
		internalFnName(fnScan);
		internalFnName(fnArrIndex);
		return "<unknown>";

	}

	static ConstStrA getFnName(Value (* addr)(IExprEnvironment&, const Value&, const Value&, const Value&) ) {

		internalFnName(fnSplitAt);
		internalFnName(fnRsplitAt);
		return "<unknown>";

	}

	static ConstStrA getFnName(Value (* addr)(IExprEnvironment&, const Value&, const Value&, const Value&, const Value&) ) {

		internalFnName(fnReplace);
		return "<unknown>";

	}


	static NamedEnumDef<Oper_WithDo::Isolation> isolationStrDef[] = {
		{ Oper_WithDo::isoDefault, "SCOPE_ENTER_WITH_OBJ" },
		{ Oper_WithDo::isoReadonly, "SCOPE_ENTER_WITH_OBJ_ISOL_RO" },
		{ Oper_WithDo::isoFull, "SCOPE_ENTER_WITH_OBJ_ISOL_FULL" }
	};

	static NamedEnum<Oper_WithDo::Isolation> isolationStr(isolationStrDef);


	void AbstractDumpCode::dump(const IExprNode* nd) {
	const Oper_Object *ndobj;
	const Oper_Scope *ndscp;
	const Oper_WithDo *ndwth;
	const Constant *ndc;
	const VariableRef *vrf;
	const Oper_Assign *asnd;
	const Oper_FunctionCall *fnnd;
	const Oper_Fn1 *fnnd1;
	const Oper_Fn2 *fnnd2;
	const Oper_Fn3 *fnnd3;
	const Oper_Fn4 *fnnd4;	

	Value v1 = jsonf->newValue(1);
	Value v2 = jsonf->newValue(2);
	Value v3 = jsonf->newValue(3);
	Value v4 = jsonf->newValue(4);

	if ((ndobj = nd->getIfcPtr<Oper_Object>()) != 0) {
		dumpInstruction(nd->getSourceLocation(), "SCOPE_ENTER", null, nd);
		dump(ndobj->getBranch(0));
		dumpInstruction(nd->getSourceLocation(), "POP", null, nd);
		dumpInstruction(nd->getSourceLocation(), "SCOPE_LEAVE_PUSH_OBJECT", null, nd);
	}
	else if ((ndscp = nd->getIfcPtr<Oper_Scope>()) != 0) {
		dumpInstruction(nd->getSourceLocation(), "SCOPE_ENTER", null, nd);
		dump(ndscp->getBranch(0));
		dumpInstruction(nd->getSourceLocation(), "SCOPE_LEAVE", null, nd);
	}
	else if (nd->getIfcPtr<Oper_Break>()!= 0) {
		dumpInstruction(nd->getSourceLocation(), "BREAK",null,nd);
		return;
	}
	else if (nd->getIfcPtr<OutputText>()!= 0) {
		dumpInstruction(nd->getSourceLocation(), "{$ ... $}",null,nd);
		return;
	}
	else if ((vrf = nd->getIfcPtr<VariableRef>()) != 0) {
		Value v = jsonf->newValue(vrf->getName());
		dumpInstruction(vrf->getSourceLocation(), "PUSH var", v, vrf);

	} else if ((ndc = nd->getIfcPtr < Constant >()) != 0) {
		Value v = ndc->getValue();
		FunctionVar *fv = v->getIfcPtr<FunctionVar>();
		if (fv) {
			natural label = allocLabel();
			natural label2 = allocLabel();
			dumpInstruction(ndc->getSourceLocation(), genInstWLabel("JUMP", label), nil, nd);
			dumpLabel(genLabelName(label2));
			dump(fv->getCode());
			dumpInstruction(ndc->getSourceLocation(), "RET", nil, nd);
			dumpLabel(genLabelName(label));
			dumpInstruction(ndc->getSourceLocation(), genInstWLabel("PUSH addr", label2), nil, nd);
		}
		else {
			dumpInstruction(ndc->getSourceLocation(), "PUSH value", v, ndc);
		}
	}
	else {
		const AbstrNaryNode *nrnd = nd->getIfcPtr<AbstrNaryNode>();
		const Oper_IncludeTrace *itnr;
		if (nrnd) {
			if (nd->getIfcPtr<Oper_TryCatch>()!= 0) {
				natural label1 = allocLabel();
				natural label2 = allocLabel();
				dumpInstruction(nd->getSourceLocation(), genInstWLabel("ENTER_TRY",label1),null,nd);
				dump(nrnd->getBranch(0));
				dumpInstruction(nd->getSourceLocation(), genInstWLabel("JUMP",label2),null,nd);
				dumpLabel(genLabelName(label1));
				dump(nrnd->getBranch(1));
				dumpInstruction(nd->getSourceLocation(), "STORE_EXCEPTION", v1, nd);
				dump(nrnd->getBranch(2));
				dumpLabel(genLabelName(label2));
				dumpInstruction(nd->getSourceLocation(), "EXIT_TRY",null,nd);
				return;
			}
			else if (nd->getIfcPtr<Oper_And>()!= 0) {
				natural label1 = allocLabel();
				dump(nrnd->getBranch(0));
				dumpInstruction(nd->getSourceLocation(), genInstWLabel("JUMP_IF_FALSE",label1),v1,nd);
				dump(nrnd->getBranch(1));
				dumpLabel(genLabelName(label1));
				return;
			}
			else if (nd->getIfcPtr<Oper_Or>()!= 0) {
				natural label1 = allocLabel();
				dump(nrnd->getBranch(0));
				dumpInstruction(nd->getSourceLocation(), genInstWLabel("JUMP_IF_TRUE",label1),v1,nd);
				dump(nrnd->getBranch(1));
				dumpLabel(genLabelName(label1));
				return;
			}
			else if (nd->getIfcPtr<Oper_Cycle>()!= 0) {
				natural label = allocLabel();
				dumpLabel(genLabelName(label));
				dump(nrnd->getBranch(0));
				dumpInstruction(nd->getSourceLocation(), genInstWLabel("JUMP_IF_TRUE",label),v1,nd);
				return;
			}
			else if (nd->getIfcPtr<Oper_If>()!= 0) {
				natural label1 = allocLabel();
				natural label2 = allocLabel();
				dump(nrnd->getBranch(0));
				dumpInstruction(nd->getSourceLocation(), genInstWLabel("JUMP_IF_FALSE",label2),v1,nd);
				dump(nrnd->getBranch(1));
				dumpInstruction(nd->getSourceLocation(), genInstWLabel("JUMP",label2),null,nd);
				dumpLabel(genLabelName(label1));
				dump(nrnd->getBranch(2));
				dumpLabel(genLabelName(label2));
			}
			else if (nd->getIfcPtr<Oper_ForEach>()!= 0) {
				dumpInstruction(nd->getSourceLocation(), "LINK",null,nd);

				return;
			}
			else if ((itnr = nd->getIfcPtr<Oper_IncludeTrace>())!= 0) {
				dumpInstruction(nd->getSourceLocation(), "BEGIN_IMPORT",jsonf->newValue(itnr->getFilePath()),nd);
				dump(nrnd->getBranch(0));
				dumpInstruction(nd->getSourceLocation(), "END_IMPORT",jsonf->newValue(itnr->getFilePath()),nd);
			}
			else if ((ndwth = nd->getIfcPtr<Oper_WithDo>()) != 0) {
				dump(ndwth->getBranch(0));
				dumpInstruction(nd->getSourceLocation(), isolationStr[ndwth->getIsolation()], v1, nd);
				dump(ndwth->getBranch(1));
				dumpInstruction(nd->getSourceLocation(), "POP", null, nd);
				dumpInstruction(nd->getSourceLocation(), "SCOPE_LEAVE_PUSH_OBJECT", null, nd);
			}
			else if (nd->getIfcPtr<Oper_Comma>() != 0) {
				dump(nrnd->getBranch(0));
				dumpInstruction(nd->getSourceLocation(), "POP", v1, nd);
				dump(nrnd->getBranch(1));
			}
			else {
				for (natural i = 0; i < nrnd->getN(); i++) {
					dump(nrnd->getBranch(i));
				}


				if ((asnd = nd->getIfcPtr < Oper_Assign >()) != 0) {
					dumpInstruction(nd->getSourceLocation(), "SETVAR", v2, nd);
				}
				else if (nd->getIfcPtr<Oper_MemberAccess>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "MEMBER_REF", v2, nd);
				}
				else if ((fnnd = nd->getIfcPtr<Oper_FunctionCall>()) != 0) {
					dump(fnnd->getFnNameCode());
					Value v = jsonf->newValue(fnnd->getN()+1);
					dumpInstruction(nd->getSourceLocation(), "CALL", v, nd);
					return;
				}
				else if (nd->getIfcPtr<OutputResult>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "${ ... }", v1, nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_Unset>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "UNSET", v1, nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_IsNull>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "ISNULL", v1, nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_Exist>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "EXIST", v1, nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_Var>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "VARNAME", v1, nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_FirstDefined>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "FIRSTDEFINED", jsonf->newValue(nrnd->getN()), nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_New>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "NEW", jsonf->newValue(nrnd->getN()), nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_Link>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "LINK", v1, nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_ReferenceOper>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "GETREF", v1, nd);
					return;
				}
				else if ((fnnd1 = nd->getIfcPtr<Oper_Fn1>()) != 0) {
					dumpInstruction(nd->getSourceLocation(), getFnName(fnnd1->getFn()), v1, nd);
				}
				else if ((fnnd2 = nd->getIfcPtr<Oper_Fn2>()) != 0) {
					dumpInstruction(nd->getSourceLocation(), getFnName(fnnd2->getFn()), v2, nd);
				}
				else if ((fnnd3 = nd->getIfcPtr<Oper_Fn3>()) != 0) {
					dumpInstruction(nd->getSourceLocation(), getFnName(fnnd3->getFn()), v3, nd);
				}
				else if ((fnnd4 = nd->getIfcPtr<Oper_Fn4>()) != 0) {
					dumpInstruction(nd->getSourceLocation(), getFnName(fnnd4->getFn()), v4, nd);
				}
				else if (nd->getIfcPtr<Oper_Varname>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "GETVARNAME", v1, nd);
				}
				else if (nd->getIfcPtr<Oper_Dereference>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "DEREF", v1, nd);
				}
				else if (nd->getIfcPtr<Oper_ArrayCreate>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "CARRAY", jsonf->newValue(nrnd->getN()), nd);
				}
				else if (nd->getIfcPtr<Oper_ArrayAppend>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "ADD_TO_ARRAY", jsonf->newValue(nrnd->getN()), nd);
				}
				else if (nd->getIfcPtr<Oper_Throw>() != 0) {
					dumpInstruction(nd->getSourceLocation(), "THROW", v1, nd);
					return;
				}

				else {
					dumpInstruction(nd->getSourceLocation(), typeid(*nd).name(), nil, nd);
				}
			}
		}
	}
}


LightSpeed::StringA AbstractDumpCode::genLabelName(natural id)
{
	return ConstStrA("L") + ToString<natural>(id);
}

LightSpeed::StringA AbstractDumpCode::genInstWLabel(ConstStrA opcode, natural id)
{
	return opcode + ConstStrA(" ") + genLabelName(id);
}

LightSpeed::natural AbstractDumpCode::allocLabel()
{
	return labelCounter++;
}



} /* namespace Tempe */
