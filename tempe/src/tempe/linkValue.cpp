/*
 * LinkValue.cpp
 *
 *  Created on: May 17, 2015
 *      Author: ondra
 */

#include "linkValue.h"

#include <lightspeed/base/text/textFormat.tcc>

#include "lightspeed/base/memory/smallAlloc.h"
namespace Tempe {


void LinkValue::serialize(IVtWriteIterator<char>& output, bool escapeUTF8) const {
	ConstStrA text = getStringUtf8();
	output.blockWrite(text,true);
}

JSON::NodeType LinkValue::getType() const {
	if (link == nil) return JSON::ndNull;
	else return (JSON::NodeType)(JSON::ndDelete+1);
}

ConstStrW LinkValue::getString() const {
	if (str.empty()) {
		TextFormatBuff<wchar_t, SmallAlloc<200> > fmt;
		fmt.setBase(16);
		fmt(L"link %1") << (natural)link.get();
		str = fmt.write();
	}
	return str;
}

integer LinkValue::getInt() const {
	return 0;
}

linteger LinkValue::getLongInt() const {
	return 0;
}

double LinkValue::getFloat() const {
	return 0;
}

bool LinkValue::getBool() const {
	return false;
}

bool LinkValue::isNull() const {
	return link == nil;
}

bool LinkValue::operator ==(const JSON::INode& other) const {
	 const LinkValue *p = dynamic_cast<const LinkValue *>(&other);
	 if (p == 0) return false;
	 return p->link == link;
}

ConstStrA LinkValue::getStringUtf8() const {
	if (stra.empty()) {
		TextFormatBuff<char, SmallAlloc<200> > fmt;
		fmt.setBase(16);
		fmt("link %1") << (natural)link.get();
		stra = fmt.write();
	}
	return stra;
}

JSON::INode* LinkValue::clone(JSON::PFactory factory) const {
	return new(*factory->getAllocator()) LinkValue(*this);
}


} /* namespace Tempe */
