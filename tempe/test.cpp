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
namespace TempeTest {

using namespace Tempe;

static Main theApp;


integer Main::start(const Args& args) {

	CommentRemover commentless(StdInput().getStream());commentless.setStaticObj();
	ConsoleA console(&commentless,
			ConsoleA::getStdOutput(),ConsoleA::getStdError());


	VarTable vtable;
	//console.scan.setWS(ConstStrA(" \t\n\r\a\b"));
	while (console.scan.hasItems()) {

		TempeCompiler comp(TempeCompiler::ctnPlain);
		PExprNode nd;
		try {
			console.scan("\b%");
			 nd = comp.compile(console.scan);
		} catch (const std::exception &e) {
			console.error("Compile error: %1\n") << e.what();
			console.scan("%(*)0\n%");
			console.flush();
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
		console.scan("%(*)0\n%");

	}

	return 0;

}

}
 /* namespace TempeTest */
