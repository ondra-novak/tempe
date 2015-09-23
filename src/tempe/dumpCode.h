/*
 * dumpCode.h
 *
 *  Created on: Sep 23, 2015
 *      Author: ondra
 */

#ifndef TEMPE_DUMPCODE_H_
#define TEMPE_DUMPCODE_H_
#include "interfaces.h"

namespace Tempe {

class AbstractDumpCode {
public:
	virtual ~AbstractDumpCode() {}

	void dump(const IExprNode *nd);


	virtual void dumpInstruction(const ExprLocation &loc, ConstStrA opcode, Value arg, const IExprNode *addr) = 0;
};

} /* namespace Tempe */

#endif /* TEMPE_DUMPCODE_H_ */
