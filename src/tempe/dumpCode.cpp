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

	AbstractDumpCode::AbstractDumpCode() :labelCounter(0)
	{

	}

	void AbstractDumpCode::dump(const IExprNode* nd) {

	
	const AbstrNaryNode *nrnd = nd->getIfcPtr<AbstrNaryNode>();
	if (nrnd) {
		for (natural i = 0; i < nrnd->getN(); i++)
			dump(nrnd->getBranch(i));
	}
	const Constant *ndc = nd->getIfcPtr < Constant > ();
	if (ndc) {
		Value v = ndc->getValue();
		FunctionVar *fv = v->getIfcPtr<FunctionVar>();
		if (fv) {
			natural label = allocLabel();
			dumpInstruction(ndc->getSourceLocation(), genJumpName(label),nil,nd);
			dump(fv->getCode());
			dumpLabel(genLabelName(label));
			dumpInstruction(ndc->getSourceLocation(), "PUSHCODE", nil, nd);
		}
		else {
			dumpInstruction(ndc->getSourceLocation(),"PUSH", ndc->getValue(), ndc);
		}
	}
	else {
		dumpInstruction(nd->getSourceLocation(), typeid(*nd).name(), nil, nd);
	}

}

LightSpeed::StringA AbstractDumpCode::genLabelName(natural id)
{
	return ConstStrA("Label_") + ToString<natural>(id);
}

LightSpeed::natural AbstractDumpCode::allocLabel()
{
	return labelCounter++;
}

StringA AbstractDumpCode::genJumpOnFalseName(natural id)
{
	return ConstStrA("JF ") + genLabelName(id);
}

StringA AbstractDumpCode::genJumpName(natural id)
{
	return ConstStrA("JUMP ") + genLabelName(id);
}


} /* namespace Tempe */
