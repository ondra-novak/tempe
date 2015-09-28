/*
 * functionVar.cpp
 *
 *  Created on: 23.7.2012
 *      Author: ondra
 */

#include "lightspeed/base/exceptions/throws.tcc"
#include "lightspeed/base/containers/autoArray.tcc"
#include "lightspeed/base/memory/smallAlloc.h"
#include "functionVar.h"

#include <lightspeed/utils/json/jsonfast.tcc>
#include "exceptions.h"
#include "varTable.h"

namespace Tempe {
	using namespace LightSpeed;

	FunctionVar::FunctionVar(ConstStringT<VarName_OutMode> arguments, PExprNode code) :arguments(arguments), code(code) {

	}


	JSON::NodeType AbstractFunctionVar::getType() const {

		ConstStringT<VarName_OutMode> arguments = getArguments();

		natural id = 0;
		for (natural i = 0; i < arguments.length(); i++) {
			id = id << 1;
			if (arguments[i].second) id |= 1;
		}
		id = (id << 4) + arguments.length();
		return (JSON::NodeType)(1000 + id);
	}

	ConstStrW AbstractFunctionVar::getString() const {
		if (buff.empty()) buff = String(getStringUtf8());
		return buff;
	}

	integer AbstractFunctionVar::getInt() const {
		return 0;
	}

	double AbstractFunctionVar::getFloat() const {
		return 0;
	}

	bool AbstractFunctionVar::getBool() const {
		return true;
	}

	bool AbstractFunctionVar::isNull() const {
		return false;
	}
	bool AbstractFunctionVar::operator ==(const JSON::INode& other) const {
		return false;
	}
	bool AbstractFunctionVar::operator !=(const JSON::INode& other) const {
		return false;
	}

	FunctionScope::FunctionScope(IExprEnvironment &env,
		ConstStringT<AbstractFunctionVar::VarName_OutMode> arguments,
		ArrayRef<Value> values) :LocalScope(env)

	{
		natural cnt = arguments.length();

		for (natural i = 0; i < cnt; i++) {
			if (i >= values.length()) break;
			if (arguments[i].second == FunctionVar::variadic) {
				JSON::PNode arr = env.getFactory().array();
				setVar(arguments[i].first, arr);
				for (natural j = i; j < values.length(); j++) {
					arr->add(values[j]);
				}
			}
			else if (values[i] != nil) {
				setVar(arguments[i].first, values[i]);
			}
		}

	}





	Value FunctionVar::execute(IExprEnvironment& env, ArrayRef<Value> values, Value context) {

		if (context != nil) {
			LocalScope scope1(env, context);
			FunctionScope scope2(scope1, arguments, values);
			scope2.setVar("this", context);
			return code->calculate(scope2);
		}
		else {
			FunctionScope scope(env, arguments, values);
			return code->calculate(scope);
		}
	}

	JSON::INode* FunctionVar::clone(JSON::PFactory factory) const {
		return new (*factory->getAllocator()) FunctionVar(*this);
	}

	ConstStrA AbstractFunctionVar::getStringUtf8() const {
		if (buff8.empty()) {
			AutoArray<char, SmallAlloc<256> > buff;
			buff.append(ConstStrA("function("));
			ConstStringT<VarName_OutMode> arguments = getArguments();
			for (natural i = 0; i < arguments.length(); i++) {
				if (i > 0) buff.add(',');
				const char *pfix = "";
				switch (arguments[i].second) {
				case variadic:
				case byValue: pfix = "in "; break;
				case byReference: pfix = "out "; break;
				case optional: pfix = "optional in "; break;
				case optionalReference: pfix = "optional out "; break;
				}
				buff.append(ConstStrA(pfix));
				buff.append(arguments[i].first);
				if (arguments[i].second == variadic) {
					buff.append(ConstStrA(" ..."));
				}
			}
			buff.add(')');
			buff8 = buff;
		}
		return buff8;

	}

	linteger AbstractFunctionVar::getLongInt() const {
		return 0;
	}

	lnatural AbstractFunctionVar::getLongUInt() const {
		return 0;
	}

	void AbstractFunctionVar::serialize(IVtWriteIterator<char>& output, bool escapeUTF8) const {
		ConstStrA name = getStringUtf8();
		output.blockWrite(name);
	}



	BoundVar::BoundVar(Value context, VarName varname) :context(context), varname(varname) {
	}

	ConstStringT<AbstractFunctionVar::VarName_OutMode> BoundVar::getArguments() const {
		Value v = resolve();
		AbstractFunctionVar *x = v->getIfcPtr<AbstractFunctionVar>();
		if (x) return x->getArguments();
		else throw OperationIsUndefined(THISLOCATION);
	}

	Value BoundVar::execute(IExprEnvironment& env, ArrayRef<Value> values, Value context) {
		Value v = resolve();
		AbstractFunctionVar *x = v->getIfcPtr<AbstractFunctionVar>();
		if (x) return x->execute(env, values, this->context);
		else throw OperationIsUndefined(THISLOCATION);
	}

	JSON::NodeType BoundVar::getType() const {
		return resolve()->getType();
	}

	ConstStrW BoundVar::getString() const {
		return resolve()->getString();
	}

	integer BoundVar::getInt() const {
		return resolve()->getInt();
	}

	double BoundVar::getFloat() const {
		return resolve()->getFloat();
	}

	bool BoundVar::getBool() const {
		return resolve()->getBool();
	}

	bool BoundVar::isNull() const {
		return resolve()->isNull();
	}

	bool BoundVar::operator ==(const INode& other) const {
		const BoundVar *v = other.getIfcPtr<BoundVar>();
		if (v) return v->context == context && v->varname == varname;
		return resolve()->operator ==(other);
	}

	bool BoundVar::operator !=(const INode& other) const {
		return resolve()->operator !=(other);
	}

	ConstStrA BoundVar::getStringUtf8() const {
		return resolve()->getStringUtf8();
	}

	linteger BoundVar::getLongInt() const {
		return resolve()->getLongInt();
	}

	lnatural BoundVar::getLongUInt() const {
		return resolve()->getLongUInt();
	}

	bool BoundVar::empty() const
	{
		return true;
	}

	void BoundVar::setValue(Value newValue) {
		context->replace(varname, newValue);
	}

	void BoundVar::serialize(IVtWriteIterator<char>& output,
		bool escapeUTF8) const {
		Value v = resolve();
		ICustomNode *cv = v->getIfcPtr<ICustomNode>();
		if (cv) cv->serialize(output, escapeUTF8);
		else {
			JSON::serialize(v, output, escapeUTF8);
		}

	}

	JSON::INode* BoundVar::clone(JSON::PFactory factory) const {
		return new (*factory->getAllocator()) BoundVar(*this);

	}


	Value BoundVar::resolve() const {
		JSON::INode *v = context->getVariable(varname);
		if (v == 0 || v == this) throw VariableNotExistException(THISLOCATION, varname);
		else return v;
	}

	VarName BoundVar::getVarname()
	{
		return varname;
	}

	JSON::INode * BoundVar::getVariable(ConstStrA v) const
	{
		return resolve()->getVariable(v);
	}

	LightSpeed::natural BoundVar::getEntryCount() const
	{
		return resolve()->getEntryCount();
	}

	JSON::INode * BoundVar::getEntry(natural idx) const
	{
		return resolve()->getEntry(idx);
	
	}

	bool BoundVar::enumEntries(const JSON::IEntryEnum &fn) const
	{
		return resolve()->enumEntries(fn);
	}

	JSON::INode * BoundVar::add(JSON::PNode x)
	{
		resolve()->add(x); return this;
	}

	JSON::INode * BoundVar::add(ConstStrA a, JSON::PNode nd)
	{
		resolve()->add(a, nd); return this;
	}

	JSON::INode* BoundVar::erase(natural x)
	{
		resolve()->erase(x); return this;
	}

	JSON::INode* BoundVar::erase(ConstStrA x)
	{
		resolve()->erase(x); return this;
	}

	JSON::INode* BoundVar::enableMTAccess()
	{
		resolve()->enableMTAccess();
		return AbstractFunctionVar::enableMTAccess();
	}

	LightSpeed::JSON::Iterator BoundVar::getFwIter() const
	{
		return resolve()->getFwIter();
	}

	LightSpeed::natural BoundVar::getUInt() const
	{
		return resolve()->getUInt();
	}

	JSON::INode & BoundVar::operator[](ConstStrA v) const
	{
		return resolve()->operator[](v);
	}

	JSON::INode & BoundVar::operator[](natural index) const
	{
		return resolve()->operator[](index);
	}

	bool BoundVar::isUtf8() const
	{
		return resolve()->isUtf8();
	}

	const void * BoundVar::proxyInterface(IInterfaceRequest &p) const
	{
		if (typeid(BoundVar) == p.getType()) return static_cast<const BoundVar *>(this);
		else return IInterface::proxyInterface(p);
	}

	void * BoundVar::proxyInterface(IInterfaceRequest &p)
	{
		if (typeid(BoundVar) == p.getType()) return static_cast<BoundVar *>(this);
		else return IInterface::proxyInterface(p);
	}

	Tempe::Value BoundVar::getConext()
	{
		return context;
	}

	Tempe::Value BoundVar::dereference()
	{
		return resolve();
	}

}