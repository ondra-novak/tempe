#include "fileCompiler.h"
#include "lightspeed/base/streams/fileiobuff.h"

namespace Tempe {



	FileCompiler::FileCompiler(IRuntimeAlloc &alloc) :Compiler(alloc)
	{

	}

	Tempe::PExprNode FileCompiler::compileFile(FilePath fname)
	{
		SeqFileInBuff<> fin(fname, OpenFlags::shareRead);

		TokenReader rd(alloc, fin, fname, 0);
		return compile(rd);

	}

	std::pair<PExprNode, FilePath> FileCompiler::loadCode(ExprLocation loc, ConstStrA name)
	{


		FilePath p = loc.getFileName();
		if (p.getPath().empty()) p = FilePath(ConstStrW(L"."), true);
		for (ConstStrA::SplitIterator iter = name.split('/'); iter.hasItems();) {
			ConstStrA part = iter.getNext();
			if (part.empty()) {
				continue;
			}
			else if (part == "..") {
				p = p / parent;
			}
			else {
				p = p / part;
			}			
		}

		const PExprNode *code = codeCache.find(p);
		PExprNode out;
		if (code) out = *code;
		else {
			out = compileFile(p);
			codeCache(p, out);
		}
		return std::make_pair(out, p);

	}

	void FileCompiler::clearCache()
	{
		codeCache.clear();
	}

}
