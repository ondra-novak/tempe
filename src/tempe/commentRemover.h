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



/*
template<typename T>
class CommentRemover: public LightSpeed::IteratorFilterBase<T,T,CommentRemover<T> > {
public:
	CommentRemover():chr(0),incomment(false) {}

    bool needItems() const {return chr == nil;}
    void input(const T &x) { chr = &x;}
    bool hasItems() const {return chr != nil;}
    T output() {
   		if (*chr == '\n') incomment = false;
   		else if (*chr == '#') incomment = true;
    	if (incomment) return ' ';
    	else return *chr;
    }

protected:
	const T *chr;
	bool incomment;

};
*/
class CommentRemover: public LightSpeed::IInputStream {
public:
	CommentRemover(LightSpeed::PInputStream source);

    virtual LightSpeed::natural read(void *buffer,  LightSpeed::natural size);
	virtual LightSpeed::natural peek(void *buffer, LightSpeed::natural size) const;
	virtual bool canRead() const;
	virtual LightSpeed::natural dataReady() const;

protected:
	LightSpeed::PInputStream source;
	bool incomment;

	static void removeComments(void *buffer,LightSpeed::natural size,bool &incomment);
};

}


#endif /* AEXPRESS_COMMENTREMOVER_H_ */
