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
#include "../../../lightspeed/src/lightspeed/base/exceptions/invalidParamException.h"

namespace Tempe {


template<typename T>
class ScopeObject: public JSON::DynNode<T>, public ILinkTarget {
	typedef JSON::DynNode<T> Super;
public:

	template<typename X>
	ScopeObject(const X &x):JSON::DynNode<T>(x) {}
	virtual SharedPtr<JSON::INode> getScopeWeakPointer() const {return weakPtr;}
	///Faster access to a variable - dynamic cast is slow
	virtual void *proxyInterface(typename Super::IInterfaceRequest &p) override  {
		if (typeid(T) == p.getType()) return static_cast<T *>(this);
		//to make faster reject that this is not BoundVar
		else  if (typeid(BoundVar) == p.getType()) return 0;
		else return IInterface::proxyInterface(p);
	}
	///Faster access to a variable - dynamic cast is slow
	virtual const void *proxyInterface(const typename Super::IInterfaceRequest &p) const override {
		if (typeid(T) == p.getType()) return static_cast<const T *>(this);
		//to make faster reject that this is not BoundVar
		else  if (typeid(BoundVar) == p.getType()) return 0;
		else return IInterface::proxyInterface(p);
	}

	ScopeObject():weakPtr(this) {}
	~ScopeObject() {
		weakPtr.redirect(0);
	}

protected:
	SharedPtr<JSON::INode> weakPtr;
};

class VarTable::Factory_t: public JSON::FactoryAlloc<ScopeObject> {
public:
	
	VarTable &owner;

	Factory_t(VarTable &owner):FactoryAlloc<ScopeObject>(owner.alloc),owner(owner) {}
	virtual IFactory *clone() {return new Factory_t(owner);}

	virtual JSON::Value createObject() override {
		return owner.regToGc(new(owner.alloc) ScopeObject<Object>);
	}
	virtual JSON::Value createArray(ConstStringT<JSON::Value> v) override {
		return owner.regToGc(new(owner.alloc) ScopeObject<Array>(v));
	}
	virtual JSON::Value createArray(ConstStringT<JSON::INode *> v) override {
		return owner.regToGc(new(owner.alloc) ScopeObject<Array>(v));
	}


};

JSON::PNode VarTable::regToGc(const JSON::PNode &obj) {
	GCReg &r = obj->getIfc<GCReg>();
	r.registerToGC(gcreg);
	return obj;
}


VarTable::VarTable()
			:alloc(StdAlloc::getInstance())
			,factory(new Factory_t(*this))
			,table(factory->newClass())
			,staticTable(factory->newClass())
			,cycleTm(5000) {
	initFunctions();
}

VarTable::VarTable(IRuntimeAlloc &alloc) 
	:alloc(alloc)
	,factory(new Factory_t(*this))
	, table(factory->newClass())
	, staticTable(factory->newClass())
	, cycleTm(5000) {
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


static void setVariable(JSON::PNode table, VarNameRef name, const Value val)
{
	Value var = table->getVariable(name);
	if (var) {
		BoundVar *bv = var->getIfcPtr<BoundVar>();
		if (bv) {
			bv->setValue(val);
			return;
		}
	}
	table->replace(name, val);
}

void VarTable::setVar(VarNameRef name, const Value val) {
	setVariable(table, name, val);
	return;
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
	gcreg.clearAll();

}

void VarTable::clearStatic() {
	staticTable.clear();
	staticTable = factory->newClass();
}


VarTable::~VarTable() {
	clear();
}

LocalScope::LocalScope(IExprEnvironment& parent)
	:parent(parent)
	,global(parent.getGlobalEnv())
	,internalGlobal(parent.getInternalGlobalEnv())
	,factory(&parent.getFactory()), table(factory->newClass()), cycleTm(naturalNull)
 {
}

Value LocalScope::getVar(VarNameRef name) const {
	if (name == "_current") return table;
	if (name == "_super") return parent.getVar("_current");
	if (name == "_global") return getGlobalEnv().getVar("_current");
	JSON::INode *nd = table->getVariable(name);
	if (nd == 0) return parent.getVar(name);
	else return nd;

}

void LocalScope::setVar(VarNameRef name, const Value val) {
	setVariable(table, name, val);
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
	return internalGlobal;

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
	:parent(parent)
	, global(parent.getGlobalEnv())
	, internalGlobal(parent.getInternalGlobalEnv())
	,factory(&parent.getFactory())
	, table(import)
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
	setVar("eval", Value(createFnCall(alloc, &fnEval)));
	setVar("rand", Value(createFnCall(alloc, &fnRand)));
	setVar("unixtime", Value(createFnCall(alloc, &fnUnixtime)));
	setVar("lstime", Value(createFnCall(alloc, &fnLsTime)));
	setVar("dbtime", Value(createFnCall(alloc, &fnDbTime)));
	setVar("isotime", Value(createFnCall(alloc, &fnIsoTime)));
	setVar("date", Value(createFnCall(alloc, &fnDate)));
	setVar("time", Value(createFnCall(alloc, &fnTime)));
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


void setValueToRef(Value refVariable, Value newValue) {
	if (refVariable == nil) throw InvalidParamException(THISLOCATION, 1, "Argument is nil");
	if (newValue == nil) throw InvalidParamException(THISLOCATION, 2, "Argument is nil");
	BoundVar &bv = refVariable->getIfc<BoundVar>();
	bv.setValue(newValue);
}
}

