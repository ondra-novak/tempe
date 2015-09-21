#pragma once

#include "compiler.h"
#include "lightspeed/base/containers/map.h"

namespace Tempe {

	class FileCompiler : public Compiler {
	public:

		FileCompiler(IRuntimeAlloc &alloc);
		virtual PExprNode compileFile(FilePath fname);

		virtual std::pair<PExprNode, FilePath> loadCode(ExprLocation loc, ConstStrA name);
		void clearCache();

	protected:

		typedef Map<FilePath, PExprNode> CodeCache;

		CodeCache codeCache;

	};


}