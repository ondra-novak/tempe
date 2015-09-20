/*
 * SourceReader.cpp
 *
 *  Created on: 17. 9. 2015
 *      Author: ondra
 */

#include <lightspeed/base/iter/iteratorFilter.tcc>

#include "SourceReader.h"

namespace Tempe {



SourceReader::SourceReader(SeqFileInput srcStream, FilePath fname, natural offset)
	:Super(srcStream),fname(fname),offset(offset)
{
	flt = &(this->getBuffer().getSourceIterator().getFilterInstance());
}

SourceLocation SourceReader::getLocation() const {
	return SourceLocation(fname, offset+flt->getCurLine());
}

natural Tempe::SourceLocation::getPosition() const {
	return position;
}

FilePath Tempe::SourceLocation::getFileName() const {
	return scriptPath;
}

Tempe::SourceInputFilter::SourceInputFilter()
	:feed(true)
	,inQuotes(false)
	,inComment(false)
	,wascr(false)
	,skipnext(false)
	,lineCounter(0)
	,charCounter(0)
{
}

natural Tempe::SourceInputFilter::getCurLine() const {
	return lineCounter;
}

natural Tempe::SourceInputFilter::getCurChar() const {
	return charCounter;
}

bool Tempe::SourceInputFilter::needItems() const {
	return feed;
}

void Tempe::SourceInputFilter::input(const char& x) {
	b = x;
	if (x == '\n') {
		if (wascr) {
			wascr = false;
		} else {
			inComment = false;
			lineCounter++;
			charCounter=0;
			skipnext = false;
		}
	} else if (x == '\r') {
		wascr = true;
		inComment = false;
		lineCounter++;
		charCounter = 0;
		skipnext = false;
	} else if (!skipnext) {
		if (x == '\\') {
			skipnext = true;
		} else if (x == '"' && !inComment) {
			inQuotes = !inQuotes;
		} else if (x == '#' && !inQuotes) {
			inComment = true;
		}
	}
	if (inComment) b = 32;
	feed = false;
}

bool Tempe::SourceInputFilter::hasItems() const {
	return !feed;
}

char Tempe::SourceInputFilter::output() {
	feed = true;
	return b;
}

} /* namespace Tempe */

