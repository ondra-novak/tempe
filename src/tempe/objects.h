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

	virtual GCReg *clear() = 0;

	bool isRegistered() const;


};

class Object: public JSON::Object, public GCReg {
public:
	virtual Object *clear();
};

class Array: public JSON::Array, public GCReg {
public:
	virtual Array *clear();
	Array();
	Array(ConstStringT<JSON::Value> x):JSON::Array(x) {}
	Array(ConstStringT<JSON::INode *> x):JSON::Array(x) {}

};

class GCRegRoot : public GCReg { 
public:
	GCRegRoot *clear();
	void clearAll();
};


}



#endif /* TEMPE_OBJECTS_H_ */
