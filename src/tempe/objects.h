/*
 * objects.h
 *
 *  Created on: 22. 9. 2015
 *      Author: ondra
 */

#ifndef TEMPE_OBJECTS_H_
#define TEMPE_OBJECTS_H_
#include <lightspeed/base/debug/dbglog.h>
#include <lightspeed/utils/json/jsonimpl.h>

using LightSpeed::LogObject;


namespace Tempe {

using namespace LightSpeed;


class GCReg {
public:
	GCReg *prev, *next;

	GCReg():prev(0),next(0) {}
	virtual ~GCReg() {
		unregisterFromGC();
	}

	void registerToGC(GCReg &root) {
		next = root.next;
		prev = &root;
		if (next) next->prev = this;
		root.next = this;
	//	LS_LOG.debug("Registered object: %1") << (natural)this;
	}

	void unregisterFromGC() {
		if (next) next->prev = prev;
		if (prev) prev->next = next;
		next = prev = 0;
//		LS_LOG.debug("Unregistered object: %1") << (natural)this;
	}

	virtual void clear() = 0;

};

class Object: public JSON::Object_t, public GCReg {
public:
	virtual void clear() {
		JSON::PNode hold = this;
		fields.clear();
	}
};

class Array: public JSON::Array_t, public GCReg {
public:
	virtual void clear() {
		JSON::PNode hold = this;
		list.clear();
	}

};


}



#endif /* TEMPE_OBJECTS_H_ */
