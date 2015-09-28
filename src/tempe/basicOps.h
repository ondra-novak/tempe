/*
 * basicOps.h
 *
 *  Created on: 3.7.2012
 *      Author: ondra
 */

#ifndef AEXPRESS_BASICOPS_H_
#define AEXPRESS_BASICOPS_H_
#include "lightspeed/base/containers/autoArray.h"

#include "interfaces.h"
#include "objects.h"
namespace Tempe {

	Value convertLink(const Value &v);

	class Constant: public AbstractNode {
	public:

		Constant(const ExprLocation &loc, Value val);
		virtual bool tryToEvalConst(IExprEnvironment&, Value &val) const;
		virtual Value calculate(IExprEnvironment &env) const;
		const Value &getValue() const;
	protected:
		Value val;		
	};


	class IGetVarName: public IInterface {
	public:
		struct ValueWithContext {
			Value value;
			Value context;
			ValueWithContext(Value value,Value context):value(value),context(context) {}
		};

//		virtual const VarName &getName(IExprEnvironment &env) const = 0;
		virtual Value getValue(IExprEnvironment &env) const = 0;
		virtual void setValue(IExprEnvironment &env, const Value &val) = 0;
		virtual bool isDefined(IExprEnvironment &env) const = 0;
		virtual void unset(IExprEnvironment &env) = 0;
		virtual ValueWithContext getValueWithContext(IExprEnvironment &env) const = 0;
		virtual Value getContext(IExprEnvironment &env) const = 0;

	};

	class LocalVarRef: public IGetVarName {
	public:
		virtual const VarName &getName(IExprEnvironment &env) const = 0;
		virtual Value getValue(IExprEnvironment &env) const {return env.getVar(getName(env));}
		virtual void setValue(IExprEnvironment &env, const Value &val) {env.setVar(getName(env),val);}
		virtual bool isDefined(IExprEnvironment &env) const {return env.varExists(getName(env));}
		virtual void unset(IExprEnvironment &env) {env.unset(getName(env));}
		virtual ValueWithContext getValueWithContext(IExprEnvironment &env) const {
			return ValueWithContext(getValue(env),nil);
		}
		virtual Value getContext(IExprEnvironment &env) const { return 0; }

	};

	class VariableRef: public AbstractNode, public LocalVarRef {
	public:
		VariableRef(const ExprLocation &loc, const VarName &name);
		virtual Value calculate(IExprEnvironment &env) const;
		virtual const VarName &getName(IExprEnvironment &env) const {return name;}
		const VarName &getName() const {return name;}
	protected:
		VarName name;
	};


	class Oper_Fn1: public NaryNode<1> {
	public:
		typedef Value (*Fn)(IExprEnvironment &, const Value &);
		Oper_Fn1(const ExprLocation &loc, Fn fn) :NaryNode<1>(loc), fn(fn) {}
		virtual bool tryToEvalConst(IExprEnvironment &env, Value &val) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const;
		Fn getFn() const { return fn; }
	protected:
		Fn fn;
	};

	class Oper_Fn2: public NaryNode<2> {
	public:
		typedef Value (*Fn)(IExprEnvironment &, const Value &, const Value &);
		Oper_Fn2(const ExprLocation &loc,Fn fn):NaryNode<2>(loc),fn(fn) {}
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const;
		virtual bool tryToEvalConst(IExprEnvironment &env, Value &val) const;
		Fn getFn() const { return fn; }
	protected:
		Fn fn;
	};

	class Oper_Fn3: public NaryNode<3> {
	public:
		typedef Value (*Fn)(IExprEnvironment &,const Value &, const Value &, const Value &);
		Oper_Fn3(const ExprLocation &loc,Fn fn):NaryNode<3>(loc),fn(fn) {}
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const;
		virtual bool tryToEvalConst(IExprEnvironment &env, Value &val) const;
		Fn getFn() const { return fn; }
	protected:
		Fn fn;
	};

	class Oper_Fn4: public NaryNode<4> {
	public:
		typedef Value (*Fn)(IExprEnvironment &,const Value &, const Value &, const Value &, const Value &);
		Oper_Fn4(const ExprLocation &loc,Fn fn):NaryNode<4>(loc),fn(fn) {}
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const;
		virtual bool tryToEvalConst(IExprEnvironment &env, Value &val) const;
		Fn getFn() const { return fn; }
	protected:
		Fn fn;
	};

	class Oper_ArrayAppend: public NaryNode<1>, public LocalVarRef {
	public:
		Oper_ArrayAppend(const ExprLocation &loc):NaryNode<1>(loc) {}
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const;
		using NaryNode<1>::calculate;

		virtual Value getValue(IExprEnvironment &env) const {return calculate(env);}
		virtual void setValue(IExprEnvironment &env, const Value &val);
		virtual bool isDefined(IExprEnvironment &env) const;
		virtual void unset(IExprEnvironment &env);
		virtual const VarName & getName(IExprEnvironment &env) const;



	};



	class Oper_MemberAccess: public NaryNode<2>, public LocalVarRef {
	public:
		Oper_MemberAccess(const ExprLocation &loc,PExprNode left, PExprNode right):NaryNode<2>(loc) {
			setBranch(0,left);
			setBranch(1,right);
		}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value getValue(IExprEnvironment &env) const {return calculate(env);}
		virtual void setValue(IExprEnvironment &env, const Value &val);
		virtual bool isDefined(IExprEnvironment &env) const;
		virtual void unset(IExprEnvironment &env);
		virtual const VarName &getName(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
		virtual ValueWithContext getValueWithContext(IExprEnvironment &env) const;
		virtual Value getContext(IExprEnvironment &env) const;

	};

	class Oper_Or: public NaryNode<2> {
	public:
		Oper_Or(const ExprLocation &loc):NaryNode<2>(loc) {}
		virtual bool tryToEvalConst(IExprEnvironment &env, Value &val) const;
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
	};

	class Oper_And: public NaryNode<2> {
	public:
		Oper_And(const ExprLocation &loc):NaryNode<2>(loc) {}
		virtual bool tryToEvalConst(IExprEnvironment &env, Value &val) const;
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
	};

	class Oper_If: public NaryNode<3> {
	public:
		Oper_If(const ExprLocation &loc):NaryNode<3>(loc) {}
		virtual bool tryToEvalConst(IExprEnvironment &env, Value &val) const;
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
	};

	class Oper_Assign: public NaryNode<2> {
	public:
		Oper_Assign(const ExprLocation &loc):NaryNode<2>(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
	};

	class Oper_Unset: public NaryNode<1> {
	public:
		Oper_Unset(const ExprLocation &loc):NaryNode<1>(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
	};

	class Oper_Exist: public NaryNode<1> {
	public:
		Oper_Exist(const ExprLocation &loc):NaryNode<1>(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
	};

	class Oper_IsNull: public NaryNode<1> {
	public:
		Oper_IsNull(const ExprLocation &loc):NaryNode<1>(loc) {}
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const;
		virtual bool tryToEvalConst(IExprEnvironment &env, Value &val) const;
	};

	class Oper_Comma: public NaryNode<2> {
	public:
		Oper_Comma(const ExprLocation &loc):NaryNode<2>(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
	};

	class Oper_Cycle: public NaryNode<1> {
	public:
		Oper_Cycle(const ExprLocation &loc):NaryNode<1>(loc) {}
		virtual bool tryToEvalConst(IExprEnvironment &env, Value &val) const;
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
	};

	
	class VariadicNode: public AbstrNaryNode {
	public:
		VariadicNode(const ExprLocation &loc):AbstrNaryNode(loc) {}
		virtual AbstrNaryNode *setBranch(natural b, PExprNode nd);
		virtual natural getN() const {return nodes.length();}
		void setBranches(class ConstStringT<PExprNode> branches);
		const PExprNode &getBranch(natural b) const {return nodes[b];}


	protected:
		AutoArray<PExprNode> nodes;
	};

	class Oper_Exec: public VariadicNode {
	public:
		Oper_Exec(const ExprLocation &loc):VariadicNode(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
	};

	class Oper_LogOut: public VariadicNode {
	public:
		Oper_LogOut(const ExprLocation &loc):VariadicNode(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
	};

	class Oper_PrintOut: public VariadicNode {
	public:
		Oper_PrintOut(const ExprLocation &loc):VariadicNode(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
	};

	class Oper_TryCatch: public NaryNode<3> {
	public:
		Oper_TryCatch(const ExprLocation &loc):NaryNode<3>(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
	};

	class Oper_WithDo: public NaryNode<2> {
	public:
		enum Isolation {
			isoDefault,
			isoReadonly,
			isoFull,
		};

		Oper_WithDo(const ExprLocation &loc, Isolation isol) :NaryNode<2>(loc),isol(isol) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
		Isolation getIsolation() const;
		Isolation isol;
	};


	class Oper_Scope: public NaryNode<1> {
	public:
		Oper_Scope(const ExprLocation &loc):NaryNode<1>(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
	};

	class Oper_Object : public NaryNode<1> {
	public:
		Oper_Object(const ExprLocation &loc) :NaryNode<1>(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const { throw; }
	};

	class Oper_Throw: public NaryNode<1> {
	public:
		Oper_Throw(const ExprLocation &loc):NaryNode<1>(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
	};

	class Oper_Var: public NaryNode<1>, public LocalVarRef {
	public:
		Oper_Var(const ExprLocation &loc):NaryNode<1>(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const {throw;}
		virtual const VarName &getName(IExprEnvironment &env) const;
		mutable VarName result;
	};

	class Oper_Varname : public NaryNode<1> {
	public:
		Oper_Varname(const ExprLocation &loc) :NaryNode<1>(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const { throw; }
	};

	class Oper_FnChr: public VariadicNode {
	public:
		Oper_FnChr(const ExprLocation &loc):VariadicNode(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
	};

	class Oper_FnParse: public VariadicNode {
	public:
		Oper_FnParse(const ExprLocation &loc):VariadicNode(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
	};

	class Oper_Break: public AbstractNode {
	public:
		Oper_Break(const ExprLocation &loc):AbstractNode(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
	};


	class Oper_ArrayCreate : public VariadicNode {
	public:
		Oper_ArrayCreate(const ExprLocation &loc) :VariadicNode(loc) {}

		virtual Value calculate(IExprEnvironment &env) const ;


	};

	class FunctionVar;
	class IExecutableVar;

	class Oper_FunctionCall: public VariadicNode {
	public:
		Oper_FunctionCall(const ExprLocation &loc, PExprNode name):VariadicNode(loc),name(name) {}
		virtual Value calculate(IExprEnvironment &env) const;
		PExprNode getFnNameCode() const { return name; }
	protected:
		virtual Value executeFn(IExecutableVar *fnvar, IExprEnvironment &env, ArrayRef<Value> args, Value context, bool functor) const;
		virtual IExecutableVar *findExecutable(Value obj) const;
		PExprNode name;


	};

	class Oper_FirstDefined: public VariadicNode {
	public:
		Oper_FirstDefined(const ExprLocation &loc):VariadicNode(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		Value calculate(IExprEnvironment &env, natural at) const;
	};

	class Oper_New: public Oper_FunctionCall {
	public:
		Oper_New(const Oper_FunctionCall &other):Oper_FunctionCall(other) {}
		virtual Value executeFn(IExecutableVar *fnvar, IExprEnvironment &env, ArrayRef<Value> args, Value context, bool functor) const;
		virtual IExecutableVar *findExecutable(Value obj) const;

	};

	class Oper_Link: public NaryNode<1> {
	public:
		Oper_Link(const ExprLocation &loc):NaryNode<1>(loc) {}
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const;
	};

	class Oper_ForEach : public NaryNode<2>{
	public:
		Oper_ForEach(const ExprLocation &loc) : NaryNode<2>(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const { throw; }
	};

	class Oper_IncludeTrace: public AbstractNode {
	public:
		 Oper_IncludeTrace(const ExprLocation &loc, const FilePath &path, PExprNode nd)
			:AbstractNode(loc),path(path),expr(nd) {}
		virtual Value calculate(IExprEnvironment &env) const;
		const FilePath &getFilePath() const {return path;}
		PExprNode getExprNode() const {return expr;}

	protected:
		 FilePath path;
		 PExprNode expr;
	};

	class Oper_ReferenceOper: public NaryNode<1> {
	public:
		Oper_ReferenceOper(const ExprLocation &loc):NaryNode<1>(loc) {}
		virtual Value calculate(IExprEnvironment &env) const;
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const { throw; }


	};

	class Oper_Dereference : public NaryNode<1> {
	public:
		Oper_Dereference(const ExprLocation &loc) :NaryNode<1>(loc) {}
		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const;
		virtual bool tryToEvalConst(IExprEnvironment &env, Value &val);

	};
}



#endif /* AEXPRESS_BASICOPS_H_ */
