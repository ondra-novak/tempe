/*
 * VarTable.cpp
 *
 *  Created on: 6.7.2012
 *      Author: ondra
 */

#include "varTable.h"

#include <lightspeed/base/memory/dynobject.h>
#include <lightspeed/utils/json/jsonimpl.h>
#include <lightspeed/utils/json/jsonfast.tcc>
#include <lightspeed/base/containers/autoArray.tcc>

#include "exceptions.h"
#include "functions.h"
#include "functionVar.h"
#include "eval.h"

namespace Tempe {


template<typename T>
class ScopeObject: public JSON::DynNode_t<T>, public ILinkTarget {
public:

	template<typename X>
	ScopeObject(const X &x):JSON::DynNode_t<T>(x) {}
	virtual SharedPtr<JSON::INode> getScopeWeakPointer() const {return weakPtr;}

	ScopeObject():weakPtr(this) {}
	~ScopeObject() {
		weakPtr.redirect(0);
	}

protected:
	SharedPtr<JSON::INode> weakPtr;
};

class VarTable::Factory_t: public JSON::FactoryAlloc_t<ScopeObject> {
public:

	ClusterAlloc alloc;
	VarTable &owner;

	Factory_t(VarTable &owner):FactoryAlloc_t<ScopeObject>(alloc),owner(owner) {}
	virtual IFactory *clone() {return new Factory_t(owner);}

	virtual JSON::PNode newClass() {
		return owner.regToGc(new(alloc) ScopeObject<Object>);
	}
	virtual JSON::PNode newArray() {
		return owner.regToGc(new(alloc) ScopeObject<Array>);
	}


};

JSON::PNode VarTable::regToGc(const JSON::PNode &obj) {
	GCReg &r = obj->getIfc<GCReg>();
	r.registerToGC(gcreg);
	return obj;
}


VarTable::VarTable():factory(new Factory_t(*this))
			,table(factory->newClass())
			,staticTable(factory->newClass())
			,cycleTm(5000) {
	initFunctions();
}


Value VarTable::getVar(VarNameRef name) const {
	if (name == "_current") return table;
	if (name == "_global") return table;
	JSON::INode *nd = table->getPtr(name);
	if (nd == 0) {
		nd = staticTable->getPtr(name);
		if (nd == 0) throw VariableNotExistException(THISLOCATION, name);
	}
	return nd;
}

void VarTable::setVar(VarNameRef name, const Value val) {
	table->erase(name);
	table->add(name,val);
}

void VarTable::unset(VarNameRef name) {
	table->erase(name);
}


bool VarTable::varExists(VarNameRef name) const {
	return (table->getVariable(name) || staticTable->getVariable(name));
}

JSON::IFactory& VarTable::getFactory() const {
	return *factory;
}


void VarTable::setStaticVar(VarNameRef name, const Value val) {
	staticTable->erase(name);
	staticTable->add(name,val);
}

void VarTable::clear() {
	table.clear();
	table = factory->newClass();
	AbstractEnv::clear();
	GCReg *x = gcreg.next;
	AutoArray<std::pair<Value, GCReg *>, SmallAlloc<256> > regs;
	while (x) {
		regs.add(std::make_pair(Value(dynamic_cast<JSON::INode *>(x)),x));
		x = x->next;
	}
	for (natural i = 0; i < regs.length(); i++) {
		regs[i].second->clear();
	}

}

void VarTable::clearStatic() {
	staticTable.clear();
	staticTable = factory->newClass();
}


VarTable::~VarTable() {
	clear();
}

LocalScope::LocalScope(IExprEnvironment& parent)
	:parent(parent), global(parent.getInternalGlobalEnv()),factory(&parent.getFactory()), table(factory->newClass()), cycleTm(naturalNull)
 {
}

Value LocalScope::getVar(VarNameRef name) const {
	if (name == "_current") return table;
	if (name == "_super") return parent.getVar("_current");
	if (name == "_global") return getGlobalEnv().getVar("_global");
	JSON::INode *nd = table->getVariable(name);
	if (nd == 0) return parent.getVar(name);
	else return nd;

}

void LocalScope::setVar(VarNameRef name, const Value val) {
	table->erase(name);
	table->add(name,val);
}

void LocalScope::unset(VarNameRef name) {
	table->erase(name);
}


bool LocalScope::varExists(VarNameRef name) const {
	return name == "_global" || name == "_current" || name=="_super" || table->getVariable(name) || parent.varExists(name);
}

JSON::IFactory& LocalScope::getFactory() const {
	return *factory;
}

IExprEnvironment& VarTable::getGlobalEnv() {
	return *this;
}

const IExprEnvironment& VarTable::getGlobalEnv() const {
	return *this;
}


IExprEnvironment& LocalScope::getGlobalEnv() {
	return global;
}

const IExprEnvironment& LocalScope::getGlobalEnv() const {
	return global;
}

void LocalScope::setCycleTimeout(natural tmInMs)
{
	cycleTm = tmInMs;
}

IExprEnvironment& LocalScope::getInternalGlobalEnv() {
	return parent.getInternalGlobalEnv();

}

const IExprEnvironment* LocalScope::getParentScope() const {
	return &parent;
}

void LocalScope::clear() {
	table = factory->object();
	AbstractEnv::clear();
}

LightSpeed::natural LocalScope::getCycleTimeout() const
{
	if (cycleTm == naturalNull) return parent.getCycleTimeout();
	else return cycleTm;
}

LocalScope::LocalScope(IExprEnvironment& parent, JSON::PNode import)
:parent(parent),global(parent.getInternalGlobalEnv()),factory(&parent.getFactory()),table(import)
{
}



void VarTable::initFunctions() {
	IRuntimeAlloc &alloc = *factory->getAllocator();
	setVar("contain", Value(createFnCall(alloc,&fnContain)));
	setVar("containWord", Value(createFnCall(alloc,&fnContainWord)));
	setVar("containExact", Value(createFnCall(alloc,&fnContainExact)));
	setVar("containWordExact", Value(createFnCall(alloc,&fnContainWordExact)));
	setVar("head", Value(createFnCall(alloc,&fnHead)));
	setVar("tail", Value(createFnCall(alloc,&fnTail)));
	setVar("offset", Value(createFnCall(alloc,&fnOffset)));
	setVar("roffset", Value(createFnCall(alloc,&fnRoffset)));
	setVar("splitAt", Value(createFnCall(alloc,&fnSplitAt)));
	setVar("rsplitAt", Value(createFnCall(alloc,&fnRsplitAt)));
	setVar("toString", Value(createFnCall(alloc,&fnToString)));
	setVar("toInt", Value(createFnCall(alloc,&fnToInt)));
	setVar("toReal", Value(createFnCall(alloc,&fnToReal)));
	setVar("round", Value(createFnCall(alloc,&fnRound)));
	setVar("floor", Value(createFnCall(alloc,&fnFloor)));
	setVar("ceil", Value(createFnCall(alloc,&fnCeil)));
	setVar("exp", Value(createFnCall(alloc,&fnExp)));
	setVar("pow", Value(createFnCall(alloc,&fnPow)));
	setVar("sin", Value(createFnCall(alloc,&fnSin)));
	setVar("cos", Value(createFnCall(alloc,&fnCos)));
	setVar("tan", Value(createFnCall(alloc,&fnTan)));
	setVar("asin", Value(createFnCall(alloc,&fnASin)));
	setVar("acos", Value(createFnCall(alloc,&fnACos)));
	setVar("atan", Value(createFnCall(alloc,&fnATan)));
	setVar("atan2", Value(createFnCall(alloc,&fnATan2)));
	setVar("log", Value(createFnCall(alloc,&fnLog)));
	setVar("log10", Value(createFnCall(alloc,&fnLog10)));
	setVar("typeof", Value(createFnCall(alloc,&fnTypeOf)));
	setVar("charAt", Value(createFnCall(alloc,&fnCharAt)));
	setVar("code", Value(createFnCall(alloc,&fnCode)));
	setVar("length", Value(createFnCall(alloc,&fnLength)));
	setVar("replace", Value(createFnCall(alloc,&fnReplace)));
	setVar("debug", Value(createFnCall(alloc,&fnDebug)));
	setVar("print", Value(createFnCall(alloc,&fnPrint)));
	setVar("exec", Value(createFnCall(alloc,&fnExec)));
	setVar("scan", Value(createFnCall(alloc,&fnScan)));
	setVar("chr", Value(createFnCall(alloc,&fnChr)));
	setVar("array", Value(createFnCall(alloc,&fnArray)));
	setVar("eval", Value(createFnCall(alloc, &fnEval)));
}


FakeGlobalScope::FakeGlobalScope(IExprEnvironment &parent) :LocalScope(parent)
{

}

FakeGlobalScope::FakeGlobalScope(IExprEnvironment &parent, JSON::PNode import) : LocalScope(parent,import)
{

}

Value FakeGlobalScope::getVar(VarNameRef name) const
{
	if (name == "_current") return table;
	if (name == "_super") throw VariableNotExistException(THISLOCATION, name);
	if (name == "_global") return table;
	return LocalScope::getVar(name);

}

bool FakeGlobalScope::varExists(VarNameRef name) const
{
	if (name == "_super") return false;
	else return LocalScope::varExists(name);
}

IExprEnvironment & FakeGlobalScope::getGlobalEnv()
{
	return *this;
}

IExprEnvironment& VarTable::getInternalGlobalEnv() {
	return *this;
}

const IExprEnvironment* VarTable::getParentScope() const {
	return 0;
}

natural VarTable::getCycleTimeout() const
{
	return cycleTm;
}


const IExprEnvironment& VarTable::getInternalGlobalEnv() const {
	return *this;
}

void VarTable::setCycleTimeout(natural tmInMs) {
	cycleTm = tmInMs;
}

const IExprEnvironment& LocalScope::getInternalGlobalEnv() const {
	return global;
}


/* namespace Tempe */

bool AbstractEnv::checkIncludeProcessed(const FilePath&f) const {
	if (includeMap.find(f) != 0) return true;
	const IExprEnvironment *p = getParentScope();
	if (p) return p->checkIncludeProcessed(f);
	else return false;

}

void AbstractEnv::markIncludeProcessed(const FilePath&f) {
	includeMap.insert(f);
}

void AbstractEnv::clear() {
	includeMap.clear();
}

const IExprEnvironment* FakeGlobalScope::getParentScope() const {
	return 0;
}

}

