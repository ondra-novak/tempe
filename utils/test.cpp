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


//Our customized environment
/*We need to create custom environment because we need to override function getTempeOutput

Original implementation has output disabled.

*/
class TempeEnv : public VarTable {
public:

	TempeEnv() :writer(output) {}

	StringA getOutput() const { return output.getArray(); }
	void clearOutput() { output.clear(); }
	bool anyOutput() const { return !output.empty(); }


	//this method have to return pointer to output stream.
	virtual IVtWriteIterator<char> *getTempeOutput() { return &writer; }

	
protected:
	//output is created using AutoArrayStream which collects all bytes into single array
	AutoArrayStream<char> output;
	//we also need to create VtWriteIterator which holds a reference to the output
	VtWriteIterator<AutoArrayStream<char> &> writer;


};

//dumps code to console
void dumpCode(PrintTextA &output, const IExprNode *nd) {
	//function implements AbstractDumpCode inside class Dumper
	//because AbstractDumpCode can walk through compiled code and reports every
	//instruction through abstract function dumpInstruction
	class Dumper: public AbstractDumpCode {
	public:
		Dumper(PrintTextA &output):output(output) {}

		//our implementation of dumpInstruction
		virtual void dumpInstruction(const ExprLocation &loc, ConstStrA opcode, Value arg, const IExprNode *addr){
			//simple output all arguments to stdout
			output("\t%6\t%4\t%5\t%1:%2:%3\n")
					<< ConstStrW(loc.getFileName())
					<< loc.getPosition()
					<< ConstStrA("")
					<< opcode
					<< ((arg==nil)?ConstStrA(""):ConstStrA(JSON::toString(arg,false)))
					<< ((natural)addr);
			//flush output
			output.nxChain().flush();
		}
		//our implementation of dumping jump labels
		virtual void dumpLabel(ConstStrA name) {
			//output label
			output("%1:\n") << name;
		}
		PrintTextA &output;

	};

	Dumper dmp(output);
	dmp.dump(nd);
}

///Customized compiler
/* Tempe allows to customize compiler by simply override any compileXXX method */
class CompilerWithDump : public FileCompiler {
public:
	CompilerWithDump(PrintTextA &output, IRuntimeAlloc &alloc) :FileCompiler(alloc),output(output) {}

	/*
	Our new keyword is "dumpcode". It accepts expression till next semicolon
	
	dumpcode <expr>;

	dumpcode is unary operation, we need to override compileUNAR.
	*/
	virtual PExprNode compileUNAR(TokenReader& reader)
	{
		//because token reader doesn't know the keyword, it appear as variable name
		//test whether next symbol is variable name which carries name "dumpcode"
		if (reader.getNext() == TokenReader::sVarname && reader.varname == "dumpcode") {
			//accept this symbol (will be removed from stream)
			reader.accept();
			//compile single expression (until semicolon is reached)
			PExprNode nd = compileSingleExpression(reader);
			//dump code of expression
			dumpCode(output, nd);
			//return compiled expression back to the compiler
			return nd;
		}
		else {
			//otherwise, call original method
			return FileCompiler::compileUNAR(reader);
		}
	}

protected:
	PrintTextA &output;
};


void prompt(ConsoleA &console)
{
	console.print("tempe$ "); console.flush();
}

integer Main::start(const Args& args) {

	ConsoleA console;
	console.print("Tempe interactive console (c)2015 Ondrej Novak (MIT Licence).\n");
	console.print("Hint: Use dumpcode command to show compiled code of an expression\n");
	//Token reader is source of symbols connected to any of byte-stream.
	TokenReader src(StdAlloc::getInstance(),console.in.nxChain(),FilePath(L"<stdin>"));	
	//we need reset level for interactive mode - otherwise, it is not necesery
	src.resetLevel(true);
	//we need to accept BOF symbol, because of interactive mode - otherwise, it is not necesery
	src.accept();

	//Our global environment
	TempeEnv env;
	//because global environment should be read only, we will create fake global scope 
	FakeGlobalScope global(env);
	//Customized compile instance that supports keyword "dumpcode".
	CompilerWithDump comp(console.print, StdAlloc::getInstance());
	//show prompt
	prompt(console);
	//repeat until CTRL+D (linux) or CTRL+Z (windows) is pressed
	while (src.hasItems()) {

		//skip any empty line - because empty code is considered as error
		//in interactive mode, end of line is reported as eof
		if (src.getNext() == TokenReader::eof) {
			//accept this symbol
			src.accept();
			//show prompt
			prompt(console);
			//try again
			continue;;
		}

		//there we will hold compiled code
		PExprNode nd;
		try {		
			//compile from input stream in interactive mode
			nd = comp.compileInteractive(src);

		} catch (const std::exception &e) {
			//show compile error
			console.error("Compile error: %1\n") << e.what();
			//flush console buffers
			console.flush();
			//reset level back to interactive mode
			// - because inside of expression, interactive mode can be temporary disabled
			src.resetLevel(true);
			//read any symbols to then of line
			while (src.getNext() != TokenReader::eof) src.accept();
			//acceote end of line
			src.accept();
			//show prompt
			prompt(console);
			continue;
		}

		//now code is compiled, we can run it
		try {
			env.clearOutput();
			//run compiled code inside our environment and retrieve result
			Value k = nd->calculate(global);
			//show generated output if any
			if (env.anyOutput()) {
				console.print("----------- OUTPUT -----------\n");
				console.print("%1\n") << env.getOutput();
				console.print("-------- END OF OUTPUT -------\n");
			}
			//show result
			console.print("Result is: %1\n") << JSON::toString(k,false);
		} catch (const std::exception &e) {
			//show any runtime error
			console.error("Runtime error: %1\n") << e.what();
			//flush console
			console.flush();
		}		

		//shiw prompt
		prompt(console);

	}

	return 0;

}

}
 /* namespace TempeTest */
