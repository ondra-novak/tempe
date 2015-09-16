/*
 * parser.h
 *
 *  Created on: 4.7.2012
 *      Author: ondra
 */

#ifndef AEXPRESS_PARSER_H_
#define AEXPRESS_PARSER_H_

#include <lightspeed/base/text/textstream.h>

#include "interfaces.h"

namespace Tempe {

using namespace LightSpeed;

class Compiler: public IInterface {
public:

	Compiler(IRuntimeAlloc &alloc);
	Compiler();

	virtual PExprNode compile(ScanTextA &reader);

	bool legacyFunctions;

protected:
	IRuntimeAlloc &alloc;
	JSON::PFactory valueFactory;
	natural level;

	virtual PExprNode compileAssign(ScanTextA& reader);
	virtual PExprNode compileOR(ScanTextA& reader);
	virtual PExprNode compileAND(ScanTextA& reader);
	virtual PExprNode compileRELAT(ScanTextA& reader);
	virtual PExprNode compilePLUSMINUS(ScanTextA& reader);
	virtual PExprNode compileMULTDIV(ScanTextA& reader);
	virtual PExprNode compileUNAR(ScanTextA& reader);
	virtual PExprNode compileIF(ScanTextA& reader);
	virtual PExprNode compileUNARSuffix(ScanTextA& reader);

	Value adjustStringValue(ConstStrA text);

	typedef RefCntPtr<AbstrNaryNode> PNaryNode;
	virtual PNaryNode createFunction(ExprLocation loc, ConstStrA name);

	virtual PExprNode createConstant(ExprLocation loc, ConstStrA name);

protected:
	class LevelControl {
	public:
		LevelControl(ScanTextA& reader, natural &level);
		~LevelControl();
	protected:
		ScanTextA &reader;
		natural &level;
	};

	class NegLevelControl {
	public:
		NegLevelControl(ScanTextA& reader, natural &level);
		~NegLevelControl();
	protected:
		ScanTextA &reader;
		natural &level;
	};

};


} /* namespace LightSpeedTest */
#endif /* AEXPRESS_PARSER_H_ */
