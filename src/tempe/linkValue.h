/*
 * LinkValue.h
 *
 *  Created on: May 17, 2015
 *      Author: ondra
 */

#ifndef AEXPRESS_LINKVALUE_H_
#define AEXPRESS_LINKVALUE_H_

#include <lightspeed/base/containers/string.h>
#include <lightspeed/base/memory/sharedPtr.h>
#include <lightspeed/utils/json/json.h>
#include <lightspeed/utils/json/jsondefs.h>

namespace Tempe {

using namespace LightSpeed;

class LinkValue: public JSON::LeafNode, public DynObject, public JSON::ICustomNode  {
public:
	LinkValue(const SharedPtr<JSON::INode> &link):link(link) {}

	const SharedPtr<JSON::INode> link;

	virtual void serialize(IVtWriteIterator<char> &output, bool escapeUTF8) const ;
	virtual JSON::NodeType getType() const;
	virtual ConstStrW getString() const;
	virtual ConstStrA getStringUtf8() const;
	virtual integer getInt() const;
	virtual linteger getLongInt() const;
	virtual double getFloat() const;
	virtual bool getBool() const ;
	virtual bool isNull() const ;
	virtual bool operator==(const JSON::INode &other) const;
	virtual INode *clone(JSON::PFactory factory) const;

protected:
	mutable String str;
	mutable StringA stra;

};

} /* namespace Tempe */

#endif /* AEXPRESS_LINKVALUE_H_ */
