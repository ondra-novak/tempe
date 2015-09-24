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
namespace Tempe {

	using namespace LightSpeed;

	AbstractDumpCode::AbstractDumpCode() :labelCounter(0), jsonf(JSON::create())
	{

	}

	void AbstractDumpCode::dump(const IExprNode* nd) {
	const Oper_Object *ndobj;
	const Oper_Scope *ndscp;
	const Oper_WithDo *ndwth;
	if ((ndobj = nd->getIfcPtr<Oper_Object>()) != 0) {
		dumpInstruction(nd->getSourceLocation(), "SCOPE_ENTER", null, nd);
		dump(ndobj->getBranch(0));
		dumpInstruction(nd->getSourceLocation(), "SCOPE_PUSH_AS_OBJECT", null, nd);
	}
	else if ((ndscp = nd->getIfcPtr<Oper_Scope>()) != 0) {
		dumpInstruction(nd->getSourceLocation(), "SCOPE_ENTER", null, nd);
		dump(ndscp->getBranch(0));
		dumpInstruction(nd->getSourceLocation(), "SCOPE_LEAVE", null, nd);
	}
	else if ((ndwth = nd->getIfcPtr<Oper_WithDo>()) != 0) {
		dumpInstruction(nd->getSourceLocation(), "SCOPE_ENTER_IMPORT", null, nd);
		dump(ndscp->getBranch(0));
		dumpInstruction(nd->getSourceLocation(), "SCOPE_PUSH_AS_OBJECT", null, nd);
	}
	else if (nd->getIfcPtr<Oper_Throw>()!= 0) {
		dumpInstruction(nd->getSourceLocation(), "THROW",null,nd);
		return;
	}
	else if (nd->getIfcPtr<Oper_Break>()!= 0) {
		dumpInstruction(nd->getSourceLocation(), "BREAK",null,nd);
		return;
	}
	else if (nd->getIfcPtr<OutputText>()!= 0) {
		dumpInstruction(nd->getSourceLocation(), "{$ ... $}",null,nd);
		return;
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
				dumpLabel(genLabelName(label2));
				dumpInstruction(nd->getSourceLocation(), genInstWLabel("EXIT_TRY",label1),null,nd);
				return;
			}
			else if (nd->getIfcPtr<Oper_And>()!= 0) {
				natural label1 = allocLabel();
				dump(nrnd->getBranch(0));
				dumpInstruction(nd->getSourceLocation(), genInstWLabel("JUMP_IF_FALSE",label1),null,nd);
				dump(nrnd->getBranch(1));
				dumpLabel(genLabelName(label1));
				return;
			}
			else if (nd->getIfcPtr<Oper_Or>()!= 0) {
				natural label1 = allocLabel();
				dump(nrnd->getBranch(0));
				dumpInstruction(nd->getSourceLocation(), genInstWLabel("JUMP_IF_TRUE",label1),null,nd);
				dump(nrnd->getBranch(1));
				dumpLabel(genLabelName(label1));
				return;
			}
			else if (nd->getIfcPtr<Oper_Cycle>()!= 0) {
				natural label = allocLabel();
				dumpLabel(genLabelName(label));
				dump(nrnd->getBranch(0));
				dumpInstruction(nd->getSourceLocation(), genInstWLabel("JUMP_IF_TRUE",label),null,nd);
				return;
			}
			else if (nd->getIfcPtr<Oper_If>()!= 0) {
				natural label1 = allocLabel();
				natural label2 = allocLabel();
				dump(nrnd->getBranch(0));
				dumpInstruction(nd->getSourceLocation(), genInstWLabel("JUMP_IF_FALSE",label2),null,nd);
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
			else for (natural i = 0; i < nrnd->getN(); i++) {
				dump(nrnd->getBranch(i));


				const Constant *ndc;
				const VariableRef *vrf;
				const Oper_Assign *asnd;
				if ((ndc = nd->getIfcPtr < Constant >()) != 0) {
					Value v = ndc->getValue();
					FunctionVar *fv = v->getIfcPtr<FunctionVar>();
					if (fv) {
						natural label = allocLabel();
						natural label2 = allocLabel();
						dumpInstruction(ndc->getSourceLocation(), genInstWLabel("JUMP", label), nil, nd);
						dumpLabel(genLabelName(label2));
						dump(fv->getCode());
						dumpLabel(genLabelName(label));
						dumpInstruction(ndc->getSourceLocation(), "RET", nil, nd);
						dumpInstruction(ndc->getSourceLocation(), genInstWLabel("PUSH addr", label2), nil, nd);
					}
					else {
						dumpInstruction(ndc->getSourceLocation(), "PUSH value", v, ndc);
					}
				}
				else if ((vrf = nd->getIfcPtr<VariableRef>()) != 0) {
					Value v = jsonf->newValue(vrf->getName());
					dumpInstruction(vrf->getSourceLocation(), "PUSH var", v, vrf);
				}
				else if ((asnd = nd->getIfcPtr < Oper_Assign >()) != 0) {
					dumpInstruction(nd->getSourceLocation(), "SETVAR",nil,nd);
				}
				else if (nd->getIfcPtr<Oper_MemberAccess>()!= 0) {
					dumpInstruction(nd->getSourceLocation(), "MEMBER_REF",nil,nd);
				}
				else if (nd->getIfcPtr<Oper_Comma>()!= 0) {
					dumpInstruction(nd->getSourceLocation(), "POP",nil,nd);
				}
				else if (nd->getIfcPtr<Oper_FunctionCall>()!= 0) {
					dumpInstruction(nd->getSourceLocation(), "CALL",null,nd);
					return;
				}
				else if (nd->getIfcPtr<OutputResult>()!= 0) {
					dumpInstruction(nd->getSourceLocation(), "${ ... }",null,nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_Unset>()!= 0) {
					dumpInstruction(nd->getSourceLocation(), "UNSET",null,nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_IsNull>()!= 0) {
					dumpInstruction(nd->getSourceLocation(), "ISNULL",null,nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_Exist>()!= 0) {
					dumpInstruction(nd->getSourceLocation(), "EXIST",null,nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_Var>()!= 0) {
					dumpInstruction(nd->getSourceLocation(), "VAR",null,nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_FirstDefined>()!= 0) {
					dumpInstruction(nd->getSourceLocation(), "FIRSTDEFINED",null,nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_New>()!= 0) {
					dumpInstruction(nd->getSourceLocation(), "NEW",null,nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_Link>()!= 0) {
					dumpInstruction(nd->getSourceLocation(), "LINK",null,nd);
					return;
				}
				else if (nd->getIfcPtr<Oper_ReferenceOper>()!= 0) {
					dumpInstruction(nd->getSourceLocation(), "GETREF",null,nd);
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
	return ConstStrA("Label_") + ToString<natural>(id);
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
