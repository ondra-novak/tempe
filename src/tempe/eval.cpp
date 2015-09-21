#include "eval.h"
#include "exceptions.h"
#include "compiler2.h"
#include <lightspeed/base/streams/memfile.h>
#include "varTable.h"

namespace Tempe {

	Value fnEval(IExprEnvironment& env, ArrayRef<Value> values) {
		if (values.length() != 1) {
			throw InvalidParamCountException(THISLOCATION, "eval", values.length(), 1);
		}
		StringA code = values[0]->getStringUtf8();
		IRuntimeAlloc *alloc = env.getFactory().getAllocator();
		Compiler2 compiler(*alloc);
		MemFileStr strfile(code);
		strfile.setStaticObj();
		TokenReader rd(*alloc, SeqFileInput(&strfile), FilePath(L"<eval>"));
		PExprNode nd = compiler.compile(rd);
		LocalScope scope(env);
		return nd->calculate(scope);
	}


}