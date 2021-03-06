/*
 * interaces.h
 *
 *  Created on: 2.7.2012
 *      Author: ondra
 */

#ifndef AEXPRESS_INTERFACES_H_
#define AEXPRESS_INTERFACES_H_

#include <lightspeed/base/memory/refCntPtr.h>
#include <lightspeed/base/memory/dynobject.h>
#include <lightspeed/base/containers/string.h>
#include <lightspeed/base/interface.h>
#include <lightspeed/base/memory/sharedPtr.h>
#include <lightspeed/utils/json.h>
#include "SourceReader.h"

using LightSpeed::FilePath;


namespace Tempe {

	using namespace LightSpeed;

	typedef JSON::PNode Value;
	typedef ConstStrA VarNameRef;
	typedef StringA VarName;
	typedef SourceLocation ExprLocation;



	class IExprEnvironment: public IInterface {
	public:

		virtual Value getVar(VarNameRef name) const = 0;
		virtual void setVar(VarNameRef name, const Value val) = 0;
		virtual void unset(VarNameRef name) = 0;
		virtual bool varExists(VarNameRef name) const = 0;
		virtual JSON::IFactory &getFactory() const = 0;
		virtual IExprEnvironment &getGlobalEnv() = 0;
		virtual IExprEnvironment &getInternalGlobalEnv()  = 0;
		virtual const IExprEnvironment *getParentScope() const = 0;
		virtual const IExprEnvironment &getGlobalEnv() const = 0;
		virtual const IExprEnvironment &getInternalGlobalEnv() const = 0;
		virtual natural getCycleTimeout() const = 0;
		virtual IVtWriteIterator<char> *getTempeOutput()  = 0;
		virtual bool checkIncludeProcessed(const FilePath &) const = 0;
		virtual void markIncludeProcessed(const FilePath &)  = 0;		

		virtual void clear() = 0;		

		virtual ~IExprEnvironment() {}
	};

	class ILinkTarget {
	public:

		virtual SharedPtr<JSON::INode> getScopeWeakPointer() const = 0;
		virtual ~ILinkTarget() {}
	};


	class IExprNode: public RefCntObj, public DynObject, public IInterface {
	public:

		virtual Value calculate(IExprEnvironment &env) const = 0;
		virtual ExprLocation getSourceLocation() const = 0;
		///tries to evaluate subtree if it can be collapsed into a const
		/**
		  @param val variable where result will be posted
		  @retval true evaluation is possible, val contains result
		  @retval false evaluation is not possible 

		  @note operator ; cannot be evaluated, because its benefit is in side effects
		 */
		virtual bool tryToEvalConst(IExprEnvironment &env, Value &val) const = 0;

		virtual ~IExprNode() {}
	};

	typedef RefCntPtr<IExprNode> PExprNode;

	class AbstractNode: public IExprNode {
	public:

		AbstractNode(const ExprLocation &loc):loc(loc) {}
		virtual ExprLocation getSourceLocation() const {return loc;}
		///unknown nodes cannot be evaluated into const
		virtual bool tryToEvalConst(IExprEnvironment &, Value &) const { return false; }
	protected:
		ExprLocation loc;
	};

	void throwScriptException(const ProgramLocation &loc, const ExprLocation &eloc, Exception &e);
	void throwScriptException(const ProgramLocation &loc, const ExprLocation &eloc, const std::exception &e);


	class AbstrNaryNode: public AbstractNode {
	public:
		AbstrNaryNode(const ExprLocation &loc):AbstractNode(loc) {}
		virtual AbstrNaryNode *setBranch(natural b, PExprNode nd) = 0;
		virtual natural getN() const = 0;
		virtual const PExprNode &getBranch(natural b) const = 0;
	};

	template<natural n>
	class NaryNode: public AbstrNaryNode{
	public:

		NaryNode(const ExprLocation &loc):AbstrNaryNode(loc) {}
		NaryNode<n> *setBranch(natural b, PExprNode nd) {branch[b % n] = nd;return this;}
		const PExprNode &getBranch(natural b) const {return branch[b % n];}

		virtual Value calculate(IExprEnvironment &env) const  {
			try{
				Value subResults[n];
				for (natural i = 0; i < n; i++)
					subResults[i] = branch[i]->calculate(env);

				return calculate(env,subResults);
			} catch (Exception &e) {
				throwScriptException(THISLOCATION,this->loc,e);
				throw;
			} catch (const std::exception &e) {
				throwScriptException(THISLOCATION,this->loc,e);
				throw;
			}

		}

		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const = 0;
		virtual natural getN() const {return n;}

	protected:
		PExprNode branch[n];
	};

	///Sets value to variable carried by reference
	/** Variable carried by reference is stored to Value which remembers its
	 original context and name. Reading that value returns its value, if exists.
	 However, there is no other way, how to store new value to the variable. 

	 This function stores the value to the variable carried as the reference.
	 @param refVariable value constructed as reference to a variable. It can be received
		through a function call
     @param newValue new value. 
	 @exception InterfaceNotImplementedException first argument is not refVariable
	 @exception InvalidArgumentException first or second argument is nil
	 */
		
	void setValueToRef(Value refVariable, Value newValue);
}


#endif /* AEXPRESS_INTERFACES_H_ */

