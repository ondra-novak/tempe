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
#include "SourceReader.h"

namespace Tempe {

using namespace LightSpeed;

class Compiler: public IInterface {
public:

	Compiler(IRuntimeAlloc &alloc);
	Compiler();

	virtual PExprNode compile(SourceReader &reader);

	bool legacyFunctions;

protected:
	IRuntimeAlloc &alloc;
	JSON::PFactory valueFactory;
	natural level;

	virtual PExprNode compileAssign(SourceReader& reader);
	virtual PExprNode compileOR(SourceReader& reader);
	virtual PExprNode compileAND(SourceReader& reader);
	virtual PExprNode compileRELAT(SourceReader& reader);
	virtual PExprNode compilePLUSMINUS(SourceReader& reader);
	virtual PExprNode compileMULTDIV(SourceReader& reader);
	virtual PExprNode compileUNAR(SourceReader& reader);
	virtual PExprNode compileIF(SourceReader& reader);
	virtual PExprNode compileUNARSuffix(SourceReader& reader);

	Value adjustStringValue(ConstStrA text);

	typedef RefCntPtr<AbstrNaryNode> PNaryNode;
	virtual PNaryNode createFunction(ExprLocation loc, ConstStrA name);

	virtual PExprNode createConstant(ExprLocation loc, ConstStrA name);

protected:
	class LevelControl {
	public:
		LevelControl(SourceReader& reader, natural &level);
		~LevelControl();
	protected:
		SourceReader &reader;
		natural &level;
	};

	class NegLevelControl {
	public:
		NegLevelControl(SourceReader& reader, natural &level);
		~NegLevelControl();
	protected:
		SourceReader &reader;
		natural &level;
	};

};


} /* namespace LightSpeedTest */
#endif /* AEXPRESS_PARSER_H_ */
