/*
 * Main.cpp
 *
 *  Created on: 6.7.2012
 *      Author: ondra
 */



#include <lightspeed/base/streams/standardIO.tcc>
#include <lightspeed/base/iter/iteratorFilter.tcc>
#include <lightspeed/utils/json/jsonfast.tcc>

#include <tempe/fileCompiler.h>
#include <tempe/varTable.h>
#include "tempe-cli.h"

namespace TempeCli {

using namespace Tempe;

static Main theApp;



class TempeEnv : public VarTable {
public:

	TempeEnv(SeqFileOutput out) :textout(out),writer(textout) {}
	virtual IVtWriteIterator<char> *getTempeOutput() { return &writer; }

	
protected:
	SeqTextOutA textout;
	AutoArrayStream<char> output;
	VtWriteIterator<SeqTextOutA &> writer;


};

integer Main::start(const Args& args) {

	ConsoleA console;

	if (args.length() < 4) {
		console.print("Usage: %1 <template-tempe> <input-json> <output>\n\n") << args[0];
		console.print("<template-tempe>   Name of script that will be executed with the data \n");
		console.print("<input-json>       JSON that contains data. Can be '-' for standard input \n");
		console.print("<output>           Output file of any type. Can be '-' for standard input \n");
		return 1;
	}

	PFolderIterator finfo =  IFileIOServices::getIOServices().getFileInfo(args[1]);

	FileCompiler compiler(StdAlloc::getInstance());
	PExprNode compiledScript;
	{
		SeqFileInBuff<> tempeInput(finfo->getFullPath(),OpenFlags::shareRead|OpenFlags::accessSeq);
		TokenReader src(StdAlloc::getInstance(),tempeInput,FilePath(finfo->getFullPath()));
		compiledScript = compiler.compile(src);
	}

	SeqFileInBuff<> dataInput(args[2],OpenFlags::shareRead|OpenFlags::accessSeq);
	JSON::PNode data = JSON::fromStream(dataInput);

	SeqFileOutBuff<> pageOutput(args[3],OpenFlags::commitOnClose|OpenFlags::create|OpenFlags::truncate);
	TempeEnv env(pageOutput);
	FakeGlobalScope scope(env, data);
	compiledScript->calculate(scope);
	pageOutput.flush();
	return 0;
}

}
 /* namespace TempeTest */
