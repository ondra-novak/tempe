/*
 * commentRemover.h
 *
 *  Created on: 22.7.2012
 *      Author: ondra
 */

#ifndef AEXPRESS_COMMENTREMOVER_H_
#define AEXPRESS_COMMENTREMOVER_H_
#include "lightspeed/base/streams/fileio_ifc.h"





namespace Tempe {

using namespace LightSpeed;

class CommentRemover: public IInputStream {
public:
	CommentRemover(PInputStream source);

    virtual natural read(void *buffer,  natural size);
	virtual natural peek(void *buffer, natural size) const;
	virtual bool canRead() const;
	virtual natural dataReady() const;

protected:
	PInputStream source;
	bool incomment;

	static void removeComments(void *buffer,natural size,bool &incomment);
};

}


#endif /* AEXPRESS_COMMENTREMOVER_H_ */
