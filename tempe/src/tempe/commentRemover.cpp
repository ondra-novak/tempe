/*
 * commentRemover.cpp
 *
 *  Created on: 22.7.2012
 *      Author: ondra
 */


#include "commentRemover.h"

namespace Tempe {

using namespace LightSpeed;

CommentRemover::CommentRemover(PInputStream source)
:source(source),incomment(false)
{
}

natural CommentRemover::read(void* buffer, natural size) {
	natural k = source->read(buffer,size);
	removeComments(buffer,k,incomment);
	return k;
}


natural CommentRemover::peek(void* buffer,
		natural size) const {
	natural k = source->read(buffer,size);
	bool inc = incomment;
	removeComments(buffer,k,inc);
	return k;
}

bool CommentRemover::canRead() const {
	return source->canRead();
}


natural CommentRemover::dataReady() const {
	return source->dataReady();
}



void CommentRemover::removeComments(void* buffer,
		natural size, bool& incomment) {

	char *k = reinterpret_cast<char *>(buffer);
	for (natural i = 0; i < size; i++) {
		char z = k[i];
		if (z == '#') incomment = true;
		if (z == '\n' || z== '\r') incomment = false;
		if (incomment) k[i] = 32;
	}
}

}
