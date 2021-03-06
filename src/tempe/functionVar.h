/*
 * functionVar.h
 *
 *  Created on: 23.7.2012
 *      Author: ondra
 */

#ifndef AEXPRESS_FUNCTIONVAR_H_
#define AEXPRESS_FUNCTIONVAR_H_


#include "lightspeed/base/containers/arrayref.h"
#include <lightspeed/base/memory/dynobject.h>
#include "lightspeed/base/containers/autoArray.h"
#include <lightspeed/base/containers/string.h>
#include "interfaces.h"
#include "lightspeed/utils/json.h"
#include <lightspeed/utils/json/jsondefs.h>

#include "varTable.h"
#include "../../../lightspeed/src/lightspeed/utils/json/json.h"



namespace Tempe {

using namespace LightSpeed;

class IExecutableVar {
public:
	enum VarType {
		byValue,
		byReference,
		optional, ///<optional variable - allowed at the end of the list - are undefined if not specified
		optionalReference, ///optional reference variable
		variadic   ///<variadic type must be specified at the end of the list
	};
	typedef std::pair<VarName, VarType> VarName_OutMode;

	virtual ConstStringT<VarName_OutMode> getArguments() const = 0;
	virtual Value execute(IExprEnvironment &env, ArrayRef<Value> values, Value context) = 0;
};

class AbstractFunctionVar: public JSON::LeafNode, public DynObject, public JSON::ICustomNode, public IExecutableVar {
public:

	virtual JSON::NodeType getType() const;
	virtual ConstStrW getString() const ;
	virtual integer getInt() const ;
	virtual double getFloat() const ;
	virtual bool getBool() const ;
	virtual bool isNull() const ;
	virtual bool operator==(const INode &other) const ;
	virtual bool operator!=(const INode &other) const ;
	virtual ConstStrA getStringUtf8() const;
	virtual linteger getLongInt() const ;
	virtual lnatural getLongUInt() const;
	virtual bool empty() const {return true;}
	virtual void serialize(IVtWriteIterator<char> &output, bool escapeUTF8) const;
public:
	mutable String buff;
	mutable StringA buff8;

};

class FunctionScope: public LocalScope {
public:
	FunctionScope(IExprEnvironment &env,
			ConstStringT<AbstractFunctionVar::VarName_OutMode> argdef,
			ArrayRef<Value> values);

};



class FunctionVar: public AbstractFunctionVar{
public:

	FunctionVar(ConstStringT<VarName_OutMode> arguments, PExprNode code);

	virtual ConstStringT<VarName_OutMode> getArguments() const {return arguments;}
	virtual Value execute(IExprEnvironment &env, ArrayRef<Value> values, Value context);
	virtual JSON::INode *clone(JSON::PFactory factory) const;

	PExprNode getCode() const { return code; }

protected:
	AutoArray<VarName_OutMode> arguments;
	PExprNode code;
};

class BoundVar: public AbstractFunctionVar {
public:
	BoundVar(Value context, VarName varname);
	void setValue(Value newValue);
	virtual ConstStringT<VarName_OutMode> getArguments() const ;
	virtual Value execute(IExprEnvironment &env, ArrayRef<Value> values, Value context) ;

	virtual JSON::NodeType getType() const;
	virtual ConstStrW getString() const ;
	virtual integer getInt() const ;
	virtual double getFloat() const ;
	virtual bool getBool() const ;
	virtual bool isNull() const ;
	virtual bool operator==(const INode &other) const ;
	virtual bool operator!=(const INode &other) const ;
	virtual ConstStrA getStringUtf8() const;
	virtual linteger getLongInt() const ;
	virtual lnatural getLongUInt() const;
	virtual bool empty() const;
	virtual void serialize(IVtWriteIterator<char> &output, bool escapeUTF8) const;
	virtual JSON::INode *clone(JSON::PFactory factory) const;

	

	Value resolve() const;
	VarName getVarname();

	virtual INode * getVariable(ConstStrA) const ;

	virtual natural getEntryCount() const ;

	virtual INode * getEntry(natural idx) const ;

	virtual bool enumEntries(const JSON::IEntryEnum &fn) const ;

	virtual INode * add(JSON::PNode) ;

	virtual INode * add(ConstStrA, JSON::PNode nd) ;

	virtual INode* erase(natural) ;

	virtual INode* erase(ConstStrA) ;

	virtual INode* enableMTAccess() ;

	virtual JSON::Iterator getFwIter() const ;

	virtual natural getUInt() const ;

	virtual JSON::INode &operator [](ConstStrA v) const;
	virtual JSON::INode &operator [](natural index) const;

	virtual bool isUtf8() const ;

	///Faster access to a variable - dynamic cast is slow
	virtual void *proxyInterface(IInterfaceRequest &p) override;
	///Faster access to a variable - dynamic cast is slow
	virtual const void *proxyInterface(const IInterfaceRequest &p) const override;
	Value getConext();
	Value dereference();
public:
	Value context;
	VarName varname;
};


} /* namespace AAA */
#endif /* AEXPRESS_FUNCTIONVAR_H_ */
