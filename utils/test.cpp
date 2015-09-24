/*
 * Main.cpp
 *
 *  Created on: 6.7.2012
 *      Author: ondra
 */

#include "test.h"


#include <lightspeed/base/streams/standardIO.tcc>
#include <tempe/varTable.h>
#include <lightspeed/base/iter/iteratorFilter.tcc>
#include <lightspeed/utils/json/jsonfast.tcc>

#include <tempe/compiler.h>
#include <tempe/dumpCode.h>
#include <tempe/fileCompiler.h>
namespace TempeTest {

using namespace Tempe;

static Main theApp;



class TempeEnv : public VarTable {
public:

	TempeEnv() :writer(output) {}

	StringA getOutput() const { return output.getArray(); }
	void clearOutput() { output.clear(); }
	virtual IVtWriteIterator<char> *getTempeOutput() { return &writer; }
	bool anyOutput() const { return !output.empty(); }

	
protected:
	AutoArrayStream<char> output;
	VtWriteIterator<AutoArrayStream<char> &> writer;


};

void dumpCode(PrintTextA &output, const IExprNode *nd) {
	class Dumper: public AbstractDumpCode {
	public:
		Dumper(PrintTextA &output):output(output) {}
		virtual void dumpInstruction(const ExprLocation &loc, ConstStrA opcode, Value arg, const IExprNode *addr){
			output("\t%6\t%4\t%5\t%1:%2:%3\n")
					<< ConstStrW(loc.getFileName())
					<< loc.getPosition()
					<< ConstStrA("")
					<< opcode
					<< ((arg==nil)?ConstStrA(""):ConstStrA(JSON::toString(arg,false)))
					<< ((natural)addr);
			output.nxChain().flush();
		}
		virtual void dumpLabel(ConstStrA name) {
			output("%1:\n") << name;
		}
		PrintTextA &output;

	};

	Dumper dmp(output);
	dmp.dump(nd);
}

integer Main::start(const Args& args) {

	ConsoleA console;
	TokenReader src(StdAlloc::getInstance(),console.in.nxChain(),FilePath(L"<stdin>"));

	//Our global environment
	TempeEnv env;
	//because global environment should be read only, we will create fake global scope 
	FakeGlobalScope global(env);

	FileCompiler comp(StdAlloc::getInstance());
	//console.scan.setWS(ConstStrA(" \t\n\r\a\b"));
	while (src.hasItems()) {

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

		env.clearOutput();
		
		dumpCode(console.print, nd);

		try {
			Value k = nd->calculate(global);
			if (env.anyOutput()) {
				console.print("----------- OUTPUT -----------\n");
				console.print("%1\n") << env.getOutput();
				console.print("-------- END OF OUTPUT -------\n");
			}
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
