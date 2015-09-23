/*
 * dumpCode.cpp
 *
 *  Created on: Sep 23, 2015
 *      Author: ondra
 */

#include "dumpCode.h"

#include <lightspeed/base/interface.h>
#include "interfaces.h"
namespace Tempe {


void AbstractDumpCode::dump(const IExprNode* nd) {

	const AbstrNaryNode *nrnd = nd->getIfcPtr<AbstrNaryNode>();
	if (nrnd) {
		for (natural i = 0; i < nrnd->getN(); i++)
			dump(nrnd->getBranch(i));
	}
	dumpInstruction(nrnd->getSourceLocation(), typeid(*nrnd).name(), nil, nrnd);

}

} /* namespace Tempe */
