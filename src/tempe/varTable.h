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

namespace Tempe {

class VarTable: public IExprEnvironment {
public:

	virtual Value getVar(VarNameRef name) const;
	virtual void setVar(VarNameRef name, const Value val);
	virtual void unset(VarNameRef name);
	virtual bool getTag(VarNameRef tagName) const;
	virtual bool varExists(VarNameRef name) const;
	virtual JSON::IFactory &getFactory() const;
	virtual IExprEnvironment &getGlobalEnv() ;
	virtual const IExprEnvironment &getGlobalEnv() const ;

	VarTable();

	void setStaticVar(VarNameRef name, const Value val);
	void clear();
	void clearStatic();
	void clearTags();
	void setTag(VarNameRef tagName, bool set);

	typedef Set<VarName,std::less<VarName> > Tags;

	const Tags &getTags() const {return tags;}
	Tags &getTags() {return tags;}


protected:

	void initFunctions();

	JSON::PFactory factory;
	JSON::PNode table;
	JSON::PNode staticTable;
	Tags tags;

};

class LocalScope: public IExprEnvironment {
public:
	LocalScope(IExprEnvironment &parent);
	LocalScope(IExprEnvironment &parent, JSON::PNode import);

	virtual Value getVar(VarNameRef name) const;
	virtual void setVar(VarNameRef name, const Value val);
	virtual void unset(VarNameRef name);
	virtual bool getTag(VarNameRef tagName) const;
	virtual bool varExists(VarNameRef name) const;
	virtual JSON::IFactory &getFactory() const;
	virtual IExprEnvironment &getGlobalEnv() ;
	virtual const IExprEnvironment &getGlobalEnv() const ;

	JSON::PNode getObject() const {return table;}


protected:
	IExprEnvironment &parent;
	JSON::PFactory factory;
	JSON::PNode table;
private:
	LocalScope(const LocalScope &parent);
	LocalScope &operator=(const LocalScope &parent);

};


} /* namespace Tempe */
#endif /* AEXPRESS_VARTABLE_H_ */
