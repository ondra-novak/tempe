/*
 * SourceReader.h
 *
 *  Created on: 17. 9. 2015
 *      Author: ondra
 */

#ifndef TEMPE_SOURCEREADER_H_
#define TEMPE_SOURCEREADER_H_
#include <lightspeed/base/iter/iteratorFilter.h>
#include <lightspeed/base/text/textstream.h>
#include <lightspeed/utils/FilePath.h>


namespace Tempe {

using namespace LightSpeed;

class SourceLocation {
public:
	SourceLocation(FilePath scriptPath, natural position)
		:position(position),scriptPath(scriptPath) {}

	natural getPosition() const;
	FilePath getFileName() const;

protected:
	natural position;
	FilePath scriptPath;
};

class SourceInputFilter: public IteratorFilterBase<char, char, SourceInputFilter>{
public:
	SourceInputFilter();

	natural getCurLine() const;
	natural getCurChar() const;

    bool needItems() const;
    void input(const char &x);
    bool hasItems() const;
    char output() ;



protected:
	char b;
	bool feed;

	bool inQuotes;
	bool inComment;
	bool wascr;
	bool skipnext;
	natural lineCounter;
	natural charCounter;

};

typedef FltChain<SeqTextInA, Filter<SourceInputFilter>::Read<SeqTextInA> > SourceInput;


class SourceReader: public TextIn<SourceInput, SmallAlloc<256> > {
public:
	typedef TextIn<SourceInput, SmallAlloc<256> > Super;

	SourceReader(SeqFileInput srcStream, FilePath fname, natural offset = 0);

	SourceLocation getLocation() const;

protected:
	FilePath fname;
	natural offset;
	SourceInputFilter *flt;

};

} /* namespace Tempe */

#endif /* TEMPE_SOURCEREADER_H_ */
