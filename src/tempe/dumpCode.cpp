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
#include "lightspeed/base/text/toString.h"
namespace Tempe {

	using namespace LightSpeed;

	AbstractDumpCode::AbstractDumpCode() :labelCounter(0), jsonf(JSON::create())
	{

	}

	void AbstractDumpCode::dump(const IExprNode* nd) {
	const Oper_Object *ndobj;
	const Oper_Scope *ndscp;
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
	else {
		const AbstrNaryNode *nrnd = nd->getIfcPtr<AbstrNaryNode>();
		if (nrnd) {
			for (natural i = 0; i < nrnd->getN(); i++)
				dump(nrnd->getBranch(i));
		}
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
				dumpLabel(label2);
				dump(fv->getCode());
				dumpLabel(genLabelName(label));
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
		else {
			dumpInstruction(nd->getSourceLocation(), typeid(*nd).name(), nil, nd);
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
