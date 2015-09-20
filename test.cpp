/*
 * Main.cpp
 *
 *  Created on: 6.7.2012
 *      Author: ondra
 */

#include "test.h"


#include "tempe/compiler.h"
#include "lightspeed/base/streams/standardIO.tcc"
#include "tempe/varTable.h"
#include "tempe/commentRemover.h"
#include "lightspeed/base/iter/iteratorFilter.tcc"
#include <lightspeed/utils/json/jsonfast.tcc>

#include "tempe/tempeCompiler.h"
#include "src/tempe/compiler2.h"
namespace TempeTest {

using namespace Tempe;

static Main theApp;


integer Main::start(const Args& args) {

	ConsoleA console;
	TokenReader src(StdAlloc::getInstance(),console.in.nxChain(),FilePath(L"<stdin>"));

	VarTable vtable;
	//console.scan.setWS(ConstStrA(" \t\n\r\a\b"));
	while (src.hasItems()) {

		Compiler2 comp(StdAlloc::getInstance());
		PExprNode nd;
		try {			
			nd = comp.compileInteractive(src);
		} catch (const std::exception &e) {
			console.error("Compile error: %1\n") << e.what();
			console.flush();
			src.resetLevel(true);
			while (src.getNext() != TokenReader::eof) src.accept();
			src.accept();
			continue;
		}

		try {
			Value k = nd->calculate(vtable);
			console.print("Result is: %1\n") << JSON::toString(k,false);
		} catch (const std::exception &e) {
			console.error("Runtime error: %1\n") << e.what();
			console.flush();
			continue;
		}		

	}

	return 0;

}

}
 /* namespace TempeTest */
