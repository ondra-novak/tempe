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
	AbstractDumpCode();
	virtual ~AbstractDumpCode() {}

	void dump(const IExprNode *nd);


	virtual void dumpInstruction(const ExprLocation &loc, ConstStrA opcode, Value arg, const IExprNode *addr) = 0;
	virtual void dumpLabel(ConstStrA name) = 0;

	natural labelCounter;
protected:
	StringA genLabelName(natural id);
	StringA genInstWLabel(ConstStrA opcode, natural id);
	StringA genJumpName(natural id);
	StringA genJumpOnFalseName(natural id);
	natural allocLabel();
	JSON::PFactory jsonf;
};

} /* namespace Tempe */

#endif /* TEMPE_DUMPCODE_H_ */
