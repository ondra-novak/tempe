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

	GCReg();
	virtual ~GCReg();

	void registerToGC(GCReg &root);

	void unregisterFromGC();

	virtual void clear() = 0;

	bool isRegistered() const;


};

class Object: public JSON::Object_t, public GCReg {
public:
	virtual void clear();
};

class Array: public JSON::Array_t, public GCReg {
public:
	virtual void clear();

};

class GCRegRoot : public GCReg { 
public:
	void clear();
	void clearAll();
};


}



#endif /* TEMPE_OBJECTS_H_ */
