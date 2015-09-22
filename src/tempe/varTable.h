/*
 * VarTable.h
 *
 *  Created on: 6.7.2012
 *      Author: ondra
 */

#ifndef AEXPRESS_VARTABLE_H_
#define AEXPRESS_VARTABLE_H_

#include "lightspeed/base/containers/set.h"
#include "lightspeed/base/memory/rtAlloc.h"
#include "interfaces.h"
#include "objects.h"

namespace Tempe {

class IncludeMap {
public:

};

class AbstractEnv: public IExprEnvironment {
public:


	bool checkIncludeProcessed(const FilePath &) const;
	void markIncludeProcessed(const FilePath &);

	virtual void clear();

protected:
	Set<FilePath> includeMap;
};

class VarTable: public AbstractEnv {
public:

	virtual Value getVar(VarNameRef name) const;
	virtual void setVar(VarNameRef name, const Value val);
	virtual void unset(VarNameRef name);
	virtual bool varExists(VarNameRef name) const;
	virtual JSON::IFactory &getFactory() const;
	virtual IExprEnvironment &getGlobalEnv() ;
	virtual const IExprEnvironment *getParentScope() const;
	virtual IExprEnvironment &getInternalGlobalEnv() ;
	virtual const IExprEnvironment &getGlobalEnv() const ;
	virtual const IExprEnvironment &getInternalGlobalEnv() const;

	VarTable();
	~VarTable();

	void setStaticVar(VarNameRef name, const Value val);
	void clear();
	void clearStatic();

	typedef Set<VarName,std::less<VarName> > Tags;

	void setCycleTimeout(natural tmInMs);
	virtual natural getCycleTimeout() const ;

	virtual IVtWriteIterator<char> *getTempeOutput()  { return 0; }	


protected:

	void initFunctions();


	class Factory_t;

	JSON::PFactory factory;
	JSON::PNode table;
	JSON::PNode staticTable;
	natural cycleTm;

	class GCRegRoot: public GCReg {void clear() {};};
	GCRegRoot gcreg;

	JSON::PNode regToGc(const JSON::PNode &obj);

};

class LocalScope: public AbstractEnv {
public:
	LocalScope(IExprEnvironment &parent);
	LocalScope(IExprEnvironment &parent, JSON::PNode import);

	virtual Value getVar(VarNameRef name) const;
	virtual void setVar(VarNameRef name, const Value val);
	virtual void unset(VarNameRef name);
	virtual bool varExists(VarNameRef name) const;
	virtual JSON::IFactory &getFactory() const;
	virtual IExprEnvironment &getGlobalEnv() ;
	virtual IExprEnvironment &getInternalGlobalEnv() ;
	virtual const IExprEnvironment *getParentScope() const;
	virtual const IExprEnvironment &getInternalGlobalEnv() const;
	virtual const IExprEnvironment &getGlobalEnv() const ;

	void clear();


	JSON::PNode getObject() const {return table;}

	void setCycleTimeout(natural tmInMs);
	virtual natural getCycleTimeout() const;

	virtual IVtWriteIterator<char> *getTempeOutput()  { return parent.getTempeOutput(); }
protected:

	IExprEnvironment &parent;
	IExprEnvironment &global;
	JSON::PFactory factory;
	JSON::PNode table;
	natural cycleTm;
private:
	LocalScope(const LocalScope &parent);
	LocalScope &operator=(const LocalScope &parent);

};

class FakeGlobalScope : public LocalScope {
public:
	FakeGlobalScope(IExprEnvironment &parent);
	FakeGlobalScope(IExprEnvironment &parent, JSON::PNode import);

	virtual Value getVar(VarNameRef name) const;
	virtual bool varExists(VarNameRef name) const;
	virtual IExprEnvironment &getGlobalEnv();
	virtual const IExprEnvironment *getParentScope() const;

};


} /* namespace Tempe */
#endif /* AEXPRESS_VARTABLE_H_ */
