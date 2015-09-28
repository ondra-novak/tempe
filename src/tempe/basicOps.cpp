/*
 * basicOps.cpp
 *
 *  Created on: 3.7.2012
 *      Author: ondra
 */


#include "basicOps.h"
#include <lightspeed/mt/process.h>
#include <lightspeed/base/exceptions/errorMessageException.h>
#include <lightspeed/base/debug/dbglog.h>
#include <lightspeed/base/streams/standardIO.tcc>
#include "lightspeed/base/interface.tcc"
#include "functionVar.h"
#include "exceptions.h"
#include "varTable.h"
#include "lightspeed/base/exceptions/invalidParamException.h"
#include <lightspeed/base/exceptions/stdexception.h>
#include <lightspeed/utils/json/jsonfast.tcc>

#include "functions.h"
#include "interfaces.h"
#include "linkValue.h"
namespace Tempe {

Value convertLink(const Value &v) {
	const LinkValue *s = v->getIfcPtr<LinkValue>();
	if (s) {
		JSON::INode *c = s->link.get();
		if (c == 0) throw NoLongerAvailableException(THISLOCATION);
		return c;
	} else {
		return v;
	}
}


Constant::Constant(const ExprLocation &loc, Value val)
	:AbstractNode(loc),val(val) {
}

bool Constant::tryToEvalConst(IExprEnvironment&, Value &val) const
{
	val = this->val;
	return true;
}

Value Constant::calculate(IExprEnvironment& env) const {
	return val;
}


const Value & Constant::getValue() const
{
	return val;
}

VariableRef::VariableRef(const ExprLocation &loc, const VarName &name)
	:AbstractNode(loc),name(name)
{
}

Value VariableRef::calculate(IExprEnvironment& env) const {
	try {
		return env.getVar(name);
	}
	catch (LightSpeed::Exception &e) {
		e.appendReason(ScriptException(THISLOCATION, loc));
		throw;
	}
}


bool Oper_Fn1::tryToEvalConst(IExprEnvironment &env, Value &val) const
{
	if (branch[0]->tryToEvalConst(env, val)) {
		val = fn(env, val);
		return true;
	}
	else {
		return false;
	}
}

Value Oper_Fn1::calculate(IExprEnvironment& env,
		const Value* subResults) const try {
		return fn(env,subResults[0]);
	} catch (LightSpeed::Exception &e) {
	throwScriptException(THISLOCATION,loc,e);throw;
} catch (const std::exception &e) {
	throwScriptException(THISLOCATION,loc,e);throw;
}



Value Oper_Fn2::calculate(IExprEnvironment& env,
	const Value* subResults) const {
	try {
		return fn(env, subResults[0], subResults[1]);
	}
	catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION, loc, e); throw;
	}
	catch (const std::exception &e) {
		throwScriptException(THISLOCATION, loc, e); throw;
	}
}

bool Oper_Fn2::tryToEvalConst(IExprEnvironment &env, Value &val) const
{
	Value v1, v2;
	if (branch[0]->tryToEvalConst(env,v1) && branch[1]->tryToEvalConst(env,v2)) {
		val = fn(env, v1, v2);
		return true;
	}
	else {
		return false;
	}
}

Value Oper_Fn3::calculate(IExprEnvironment& env,
	const Value* subResults) const {
	try {
		return fn(env, subResults[0], subResults[1], subResults[2]);
	}
	catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION, loc, e); throw;
	}
	catch (const std::exception &e) {
		throwScriptException(THISLOCATION, loc, e); throw;
	}
}


bool Oper_Fn3::tryToEvalConst(IExprEnvironment &env, Value &val) const
{
	Value v1, v2, v3;
	if (branch[0]->tryToEvalConst(env,v1) && branch[1]->tryToEvalConst(env,v2) && branch[1]->tryToEvalConst(env,v3)) {
		val = fn(env, v1, v2,v3);
		return true;
	}
	else {
		return false;
	}

}

Value Oper_Fn4::calculate(IExprEnvironment& env,
	const Value* subResults) const {
	try {
		return fn(env, subResults[0], subResults[1], subResults[2], subResults[3]);
	}
	catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION, loc, e); throw;
	}
	catch (const std::exception &e) {
		throwScriptException(THISLOCATION, loc, e); throw;
	}
}



bool Oper_Fn4::tryToEvalConst(IExprEnvironment &env, Value &val) const
{
	Value v1, v2, v3, v4;
	if (branch[0]->tryToEvalConst(env, v1) && branch[1]->tryToEvalConst(env, v2) && branch[1]->tryToEvalConst(env, v3) && branch[1]->tryToEvalConst(env, v4)) {
		val = fn(env, v1, v2, v3,v4);
		return true;
	}
	else {
		return false;
	}
}

bool Oper_Or::tryToEvalConst(IExprEnvironment &env, Value &val) const
	{
		Value v;
		if (branch[0]->tryToEvalConst(env,v) && v->getBool()) {
			val = v;
			return true;
		}
		else {
			return branch[1]->tryToEvalConst(env,val);
		}
	}

	Value Oper_Or::calculate(IExprEnvironment& env) const {
	try {
		Value k = branch[0]->calculate(env);
		if (k->getBool()) return k;
		else return branch[1]->calculate(env);
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	} catch (const std::exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	}
	throw;
}

	bool Oper_And::tryToEvalConst(IExprEnvironment &env, Value &val) const
	{
		Value v;
		if (branch[0]->tryToEvalConst(env,v)) {
			if (v->getBool()) {
				return branch[1]->tryToEvalConst(env,val);
			}
			else {
				val = v;
				return true;
			}
		}
		else
			return false;
	}

	bool Oper_If::tryToEvalConst(IExprEnvironment &env, Value &val) const
	{
		Value v;
		if (branch[0]->tryToEvalConst(env,v)) {
			if (v->getBool()) return branch[1]->tryToEvalConst(env,val);
			else return branch[2]->tryToEvalConst(env,val);
		}
		else
			return false;
	}


	Value Oper_And::calculate(IExprEnvironment& env) const {
	try {
		Value k = branch[0]->calculate(env);
		if (!k->getBool()) return k;
		else return branch[1]->calculate(env);
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	} catch (const std::exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	}
	throw;

}


Value Oper_If::calculate(IExprEnvironment& env) const {
	try {
		Value k = branch[0]->calculate(env);
		if (k->getBool()) return branch[1]->calculate(env);
		else return branch[2]->calculate(env);
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	} catch (const std::exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	}
	throw;
}


Value Oper_Assign::calculate(IExprEnvironment& env) const {
	try {
		IGetVarName &varNameBranch = branch[0]->getIfc<IGetVarName>();
		Value res = branch[1]->calculate(env);
		varNameBranch.setValue(env,res);
		return res;
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	} catch (const std::exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	}
	throw;

}

Value Oper_Unset::calculate(IExprEnvironment& env) const {
	try {
		IGetVarName &varNameBranch = branch[0]->getIfc<IGetVarName>();
		varNameBranch.unset(env);
		return env.getFactory().newNullNode();
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	} catch (const std::exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	}	throw;


}

Value Oper_Exist::calculate(IExprEnvironment& env) const {
	try {
		IGetVarName &varNameBranch = branch[0]->getIfc<IGetVarName>();
		return env.getFactory().newValue(varNameBranch.isDefined(env));
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	} catch (const std::exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	}	throw;

}

bool Oper_Cycle::tryToEvalConst(IExprEnvironment &env, Value &val) const
{
	if (branch[0]->tryToEvalConst(env,val)) {
		if (val->getBool() == false) return true;
		else throw ParseError(THISLOCATION, loc, "Infinite cycle");
	}
	else {
		return false;
	}
	
}

Value Oper_Cycle::calculate(IExprEnvironment& env) const {
	try {
		Timeout tm(env.getCycleTimeout());
		Value r = branch[0]->calculate(env);
		while (r->getBool()) { 
			if (tm.expired())
				throw ExecutionTimeout(THISLOCATION);
			r = branch[0]->calculate(env); 
		}
		return r;
	} catch (BreakException &) {
		return env.getFactory().newNullNode();
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	}	throw;


}





AbstrNaryNode* VariadicNode::setBranch(natural b, PExprNode nd) {
	if (nodes.length() < b + 1) nodes.resize(b + 1);
	nodes(b) = nd;
	return this;
}

void VariadicNode::setBranches(class ConstStringT<PExprNode> branches)
{
	nodes.clear();
	nodes.append(branches);
}

Value Oper_Exec::calculate(IExprEnvironment& env) const {
	try {
		if (nodes.length() < 1) throw ErrorMessageException(THISLOCATION,"Exec needs at least one argument");
		Value pgmName = nodes[0]->calculate(env);
		Process pgm(pgmName->getString());
		for (natural i = 1; i < nodes.length(); i++) {
			Value arg = nodes[i]->calculate(env);
			pgm.arg(arg->getString());
		}
		integer r = pgm.exec();
		return env.getFactory().newValue(r);
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);
		throw;
	}	throw;

}

Value Oper_LogOut::calculate(IExprEnvironment& env) const {
	AutoArrayStream<char> buff;
	for (natural i = 0; i < nodes.length(); i++) {
		JSON::PNode res = nodes[i]->calculate(env);
		JSON::serialize(res,buff,false);
	}
	StringA pos = String::getUtf8(this->loc.getFileName());
	ProgramLocation loc(pos.c_str(),this->loc.getPosition(),"");
	LogObject(loc).note("Script log: %1") << ConstStrA(buff.getArray());
	return env.getFactory().newValue(true);
}



Value Oper_PrintOut::calculate(IExprEnvironment& env) const {
	AutoArray<wchar_t> buff;
	for (natural i = 0; i < nodes.length(); i++) {
		buff.append(nodes[i]->calculate(env)->getString());
	}
	ConsoleW con;
	con.print(L"%1\n") << ConstStrW(buff);
	return env.getFactory().newValue(true);
}




Value Oper_IsNull::calculate(IExprEnvironment& env,
		const Value* subResults) const {
	return env.getFactory().newValue(subResults[0]->getType() == JSON::ndNull);
}


bool Oper_IsNull::tryToEvalConst(IExprEnvironment &env, Value &val) const
{
	if (branch[0]->tryToEvalConst(env,val)) {
		val = env.getFactory().newValue(val->isNull());
		return true;
	}
	else {
		return false;
	}
}

class CodeException : public Exception {
public:
	CodeException(const ProgramLocation &loc, const Value &val):LightSpeed::Exception(loc),val(val) {}
	~CodeException() throw() {}
	const Value &getVal() const {return val;}

	LIGHTSPEED_EXCEPTIONFINAL;

protected:

	Value val;
	void message(ExceptionMsg &msg) const {
		msg("Unhandled exception: %1") << val->getString();
	}

};


Value Oper_TryCatch::calculate(IExprEnvironment& env) const {
	try {
		return branch[0]->calculate(env);
	} catch (CodeException &e) {
		IGetVarName &varNameBranch = branch[1]->getIfc<IGetVarName>();
		varNameBranch.setValue(env,e.getVal());
		try {
			return branch[2]->calculate(env);
		} catch (LightSpeed::Exception &e) {
			throwScriptException(THISLOCATION,loc,e);throw;
		}
	} catch (const LightSpeed::Exception &e) {
		IGetVarName &varNameBranch = branch[1]->getIfc<IGetVarName>();
		varNameBranch.setValue(env,env.getFactory().newValue(e.getMessage()));
		try {
			return branch[2]->calculate(env);
		} catch (LightSpeed::Exception &e) {
			throwScriptException(THISLOCATION,loc,e);throw;
		}

	}
}

Value Oper_Throw::calculate(IExprEnvironment& env) const {
	try {
		throw CodeException(THISLOCATION,branch[0]->calculate(env));
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	}
}



Value Oper_Var::calculate(IExprEnvironment& env) const {
	try {
		StringA n = branch[0]->calculate(env)->getStringUtf8();
		return env.getVar(n);
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	}
}

const VarName& Oper_Var::getName(IExprEnvironment& env) const {
	try {
		StringA n = branch[0]->calculate(env)->getStringUtf8();
		result = n;
		return result;
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;

	}
}

Value Oper_FnChr::calculate(IExprEnvironment& env) const {
	try {
		AutoArray<wchar_t> buff;
		for (natural i = 0; i < nodes.length(); i++) {
			buff.add((wchar_t)nodes[i]->calculate(env)->getInt());
		}
		return env.getFactory().newValue(ConstStrW(buff));
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;

	}
}

Value Oper_FnParse::calculate(IExprEnvironment& env) const try {
	if (nodes.length() < 2) return env.getFactory().newValue(true);
	String subj = nodes[0]->calculate(env)->getString();
	String pattern = nodes[1]->calculate(env)->getString();
	TextParser<wchar_t> parser;
	if (parser(pattern,subj)) {

		for (natural i = 2; i < nodes.length(); i++) {
			IGetVarName &varNameBranch = nodes[i]->getIfc<IGetVarName>();
			varNameBranch.setValue(env,env.getFactory().newValue(parser[i-1].str()));
		}
		return env.getFactory().newValue(true);

	} else {
		return env.getFactory().newValue(false);
	}
} catch (LightSpeed::Exception &e) {
	throwScriptException(THISLOCATION,loc,e);throw;
} catch (const std::exception &e) {
	throwScriptException(THISLOCATION,loc,e);throw;
}

Value Oper_Break::calculate(IExprEnvironment& env) const {
  throw BreakException(THISLOCATION,this->loc);
}





Value Oper_FunctionCall::calculate(IExprEnvironment& env) const {
	try {
		Value context;
		Value fn;
		const IGetVarName *vn = name->getIfcPtr<IGetVarName>();
		if (vn) {
			IGetVarName::ValueWithContext wvc = vn->getValueWithContext(env);
			context = wvc.context;
			fn = wvc.value;
		} else {
			fn = name->calculate(env);
		}
		IExecutableVar *fnvar = dynamic_cast<AbstractFunctionVar *>(fn.get());
		Value fncontext;
		if (fnvar == 0) {
			fncontext = fn;
			fnvar = findExecutable(fn);
		}
		if (fnvar == 0) throw OperationIsUndefined(THISLOCATION);

		ConstStringT<FunctionVar::VarName_OutMode> varlist = fnvar->getArguments();
		natural cnt =varlist.length();

		bool longerisok = false;
		bool anybyref = false;

		natural nodecnt = nodes.length();

		for (natural i = 0; i < cnt; i++) {
			switch (varlist[i].second) {
			case FunctionVar::byValue: if (i>=nodecnt)
				throw InvalidParamException(THISLOCATION,i,"Missing argument");
				break;
			case FunctionVar::byReference:
				if (i>=nodecnt)
					throw InvalidParamException(THISLOCATION,i,"Missing argument");
				if (nodes[i]->getIfcPtr<IGetVarName>() == 0)
					throw InvalidParamException(THISLOCATION,i+1,"Argument cannot be an expression");
				anybyref = true;
				break;
			case FunctionVar::optional:
				break;
			case FunctionVar::optionalReference:
				if (nodes[i]->getIfcPtr<IGetVarName>() != 0)
					throw InvalidParamException(THISLOCATION,i+1,"Argument cannot be an expression");
				anybyref = true;
				break;
			case FunctionVar::variadic:
				longerisok = true;
				break;
			}
		}
		if (!longerisok && nodecnt> cnt) {
			StringA fname;
			const LocalVarRef *gn = name->getIfcPtr<LocalVarRef>();
			if (gn) fname = gn->getName(env);
			else fname = JSON::toString(fn,false);
			throw InvalidParamCountException(THISLOCATION, fname, nodecnt,cnt);
		}
		AutoArray<Value,SmallAlloc<23> > calcVals;

		for (natural i = 0; i < nodecnt; i++) {
			if (i < cnt && varlist[i].second >= FunctionVar::byReference && varlist[i].second <= FunctionVar::optionalReference) {
				LocalVarRef *n = nodes[i]->getIfcPtr<LocalVarRef>();
				if (n) {
					Value ctx = n->getContext(env);
					if (ctx == nil) ctx = env.getVar("_current");
					calcVals.add(new(*env.getFactory().getAllocator()) BoundVar(ctx, n->getName(env)));
					continue;
				}
			}
			calcVals.add(nodes[i]->calculate(env));
		}

		Value res;
		if (fncontext != nil) {
			LocalScope s(env,context);
			res = executeFn(fnvar, env, calcVals, fncontext, true);
		} else {
			res = executeFn(fnvar, env, calcVals, context, false);
		}

		return res;




	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;

	}
}

IExecutableVar* Oper_FunctionCall::findExecutable(Value obj) const {
	IExecutableVar *fnvar = 0;
	while (fnvar == 0) {
		if (obj->isObject()) {
			obj = obj->getPtr("()");
			if (obj!= nil) {
				fnvar = dynamic_cast<IExecutableVar *>(obj.get());
				continue;
			}
		}
		break;
	}
	return fnvar;
}


Value Oper_FunctionCall::executeFn(IExecutableVar *fnvar, IExprEnvironment &env, ArrayRef<Value> args, Value context, bool functor) const {
	return fnvar->execute(env,args,context);
}


Value Oper_FirstDefined::calculate(IExprEnvironment& env) const {
	try {
		if (nodes.empty()) throw InvalidParamCountException(THISLOCATION,"firstDefined",0,1);
		return calculate(env,0);
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	}
}

Value Oper_FirstDefined::calculate(IExprEnvironment& env, natural at) const {
	try {
		return nodes[at]->calculate(env);
	} catch (LightSpeed::Exception &e) {
		natural k = at+1;
		if (k >= nodes.length()) throw;
		try {
			return calculate(env,k);
		} catch (LightSpeed::Exception &e2) {
			e2.appendReason(e);
			throw;
		}
	} catch (std::exception &e) {
		natural k = at+1;
		if (k >= nodes.length()) throw;
		try {
			return calculate(env,k);
		} catch (LightSpeed::Exception &e2) {
			e2.appendReason(StdException(THISLOCATION,e));
			throw;
		}
	}
}

class WithScope: public LocalScope {
public:
	WithScope(IExprEnvironment&env, JSON::PNode data, natural index, natural count, Value prevValue)
		:LocalScope(env,data),prevVal(prevValue),index(index),count(count) {}


	virtual Value getVar(VarNameRef name) const {
		if (name == "__index") return getFactory().newValue(index);
		if (name == "__count") return getFactory().newValue(count);
		if (name == "__previous") return prevVal;
		return LocalScope::getVar(name);
	}

	const Value prevVal;

protected:
	natural index;
	natural count;
};


Value Oper_WithDo::calculate(IExprEnvironment& env) const {
	Value v = branch[0]->calculate(env);

	v = convertLink(v);


	if (v->isObject()) {
		switch (isol) {
		case isoDefault: {
			LocalScope scope(env, v);
			return branch[1]->calculate(scope);
		}
		case isoReadonly: {
			FakeGlobalScope scope(env, v);
			return branch[1]->calculate(scope);
		}
		case isoFull: {
			FakeGlobalScope scope(env.getInternalGlobalEnv(), v);
			return branch[1]->calculate(scope);
		}
		default:
			throw OperationIsUndefined(THISLOCATION);
			break;
		}
	}
	else {
		throw OperationIsUndefined(THISLOCATION);
	}


}

Oper_WithDo::Isolation Oper_WithDo::getIsolation() const
{
	return isol;
}

Value Oper_Scope::calculate(IExprEnvironment& env) const {
	LocalScope scope(env);
	return branch[0]->calculate(scope);
}

Value Oper_Object::calculate(IExprEnvironment& env) const {
	LocalScope scope(env);
	branch[0]->calculate(scope);
	return scope.getObject();
}



Value Oper_MemberAccess::calculate(IExprEnvironment& env) const {
	Value v = branch[0]->calculate(env);
	v = convertLink(v);
	if (v->isObject()) {
		LocalScope scope(env,v);
		return scope.getVar(branch[1]->getIfc<LocalVarRef>().getName(env));
	} else {
		throw OperationIsUndefined(THISLOCATION) << ScriptException(THISLOCATION,loc);
	}
}

void Oper_MemberAccess::setValue(IExprEnvironment& env, const Value& val) {
	Value v = branch[0]->calculate(env);
	v = convertLink(v);
	if (v->isObject()) {
		LocalScope scope(env,v);
		scope.setVar(branch[1]->getIfc<LocalVarRef>().getName(env),val);
	} else {
		throw OperationIsUndefined(THISLOCATION) << ScriptException(THISLOCATION,loc);
	}
}

bool Oper_MemberAccess::isDefined(IExprEnvironment& env) const {
	Value v = branch[0]->calculate(env);
	v = convertLink(v);
	if (v->isObject()) {
		LocalScope scope(env,v);
		return scope.varExists(branch[1]->getIfc<LocalVarRef>().getName(env));
	} else {
		return false;
	}
}

void Oper_MemberAccess::unset(IExprEnvironment& env) {
	Value v = branch[0]->calculate(env);
	v = convertLink(v);
	if (v->isObject()) {
		LocalScope scope(env,v);
		return scope.unset(branch[1]->getIfc<LocalVarRef>().getName(env));
	} else {
		throw OperationIsUndefined(THISLOCATION) << ScriptException(THISLOCATION,loc);
	}

}

Value Oper_Comma::calculate(IExprEnvironment& env) const {
	const Oper_Comma *n = this,*p = n;
	try {
		while (n != 0) {
			n->branch[0]->calculate(env);
			p = n;
			n = n->branch[1]->getIfcPtr<Oper_Comma>();
		}
		return p->branch[1]->calculate(env);
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,p->loc,e);throw;
	} catch (const std::exception &e) {
		throwScriptException(THISLOCATION,p->loc,e);throw;
	}
	throw;
}

const VarName& Oper_MemberAccess::getName(IExprEnvironment& env) const {
	return branch[1]->getIfc<LocalVarRef>().getName(env);
}

IGetVarName::ValueWithContext Oper_MemberAccess::getValueWithContext(IExprEnvironment& env) const {
	Value v = branch[0]->calculate(env);
	v = convertLink(v);
	if (v->isObject()) {
		LocalScope scope(env,v);
		return ValueWithContext(scope.getVar(branch[1]->getIfc<LocalVarRef>().getName(env)),v);
	} else {
		throw OperationIsUndefined(THISLOCATION) << ScriptException(THISLOCATION,loc);
	}
}

Tempe::Value Oper_MemberAccess::getContext(IExprEnvironment &env) const
{
	Value v = branch[0]->calculate(env);
	v = convertLink(v);
	if (v->isObject()) 
		return v;
	else
		throw OperationIsUndefined(THISLOCATION) << ScriptException(THISLOCATION, loc);
}

class EmptyConstructor : public IExecutableVar {
public:
	virtual ConstStringT<VarName_OutMode> getArguments() const {
		return ConstStringT<VarName_OutMode>();
	}
	virtual Value execute(IExprEnvironment &env, ArrayRef<Value> values, Value context) {
		LocalScope scope(env);
		if (context != null) scope.setVar("this",context);
		if (env.varExists("super")) {
			Value v = env.getVar("super");
			if (!v->isNull()) {
				IExecutableVar &e = v->getIfc<IExecutableVar>();
				if (e.getArguments().empty() || e.getArguments()[0].second >= AbstractFunctionVar::optional)
					e.execute(scope,ArrayRef<Value>(),null);
				else
					throw InvalidParamCountException(THISLOCATION,"init",0,e.getArguments().length());
			}
		}
		return JSON::getNullNode();
	}

};


class ExpandSuperClassFn:public AbstractFunctionVar {
public:
	ExpandSuperClassFn(Value classToClone):classToClone(classToClone) {
		Value initFn = classToClone->getPtr("init");
		if (initFn != nil) {
			this->initFn = initFn->getIfcPtr<IExecutableVar>();
		} else {
			this->initFn = 0;
		}
	}

protected:

	virtual Value execute(IExprEnvironment &env, ArrayRef<Value> values, Value context) {
		context = env.getVar("this");
		LocalScope local(env);
		cloneClass(local,classToClone,context);
		if (initFn) return initFn->execute(local,values,nil);
		else {
			EmptyConstructor ec;
			return ec.execute(local,ArrayRef<Value>(),nil);
		}
	}

	virtual ConstStringT<VarName_OutMode> getArguments() const {
		if (initFn) return initFn->getArguments();
		else return ConstStringT<VarName_OutMode>();
	}
	virtual INode *clone(JSON::PFactory factory) const {
		return new(*factory->getAllocator()) ExpandSuperClassFn(*this);
	}

	IExecutableVar *initFn;
	Value classToClone;
public:
	static void cloneClass(IExprEnvironment& env, Value context, Value where) {
		Value super;
		for (JSON::Iterator iter = context->getFwIter(); iter.hasItems();) {
			const JSON::KeyValue &kv = iter.getNext();
			if (kv.getStringKey() == "init") continue;
			if (kv.getStringKey() == "super") {
				super = kv.node;
				continue;
			}
			where->add(kv.getStringKey(),kv.node->clone(&env.getFactory()));
		}
		if (super != nil) {
			env.setVar("super", new (*env.getFactory().getAllocator()) ExpandSuperClassFn(super));
		} else {
			env.setVar("super", JSON::getNullNode());
		}
	}

};


static EmptyConstructor emptyConstructor;


Value Oper_New::executeFn(IExecutableVar* fnvar, IExprEnvironment& env,
		ArrayRef<Value> args, Value context, bool functor) const {
	Value obj = env.getFactory().object();
	if (functor) {
		obj->add("class", context);
		ExpandSuperClassFn::cloneClass(env,context,obj);
	} else {
		obj->add("class", dynamic_cast<JSON::INode *>(fnvar));
	}

	Oper_FunctionCall::executeFn(fnvar,env,args,obj,functor);
	return obj;
}

IExecutableVar* Oper_New::findExecutable(Value obj) const {
	JSON::INode *nd = obj->getPtr("init");
	IExecutableVar *fn = 0;
	if (nd != 0) {
		fn =nd->getIfcPtr<IExecutableVar>();
		if  (fn == 0) {
			if (obj->isObject())
				fn = Oper_FunctionCall::findExecutable(nd);
		}
	} else {
		fn = &emptyConstructor;
	}
	return fn;
}


Value Oper_Link::calculate(IExprEnvironment& env,const Value* subResults) const {
	if (subResults[0]->isNull()) return env.getFactory().newNullNode();
	ILinkTarget *obj = convertLink(subResults[0])->getIfcPtr<ILinkTarget>();
	if (obj == 0) throw OperationIsUndefined(THISLOCATION);
	return new(*env.getFactory().getAllocator()) LinkValue(obj->getScopeWeakPointer());
}

void Oper_ArrayAppend::setValue(IExprEnvironment& env, const Value& val) {
	Value v = branch[0]->calculate(env);
	v = convertLink(v);
	if (v->isArray()) {
		v->add(val);
	} else {
		throw OperationIsUndefined(THISLOCATION) << ScriptException(THISLOCATION,loc);
	}

}

bool Oper_ArrayAppend::isDefined(IExprEnvironment& env) const {
	return false;
}

void Oper_ArrayAppend::unset(IExprEnvironment& env) {
	Value v = branch[0]->calculate(env);
	v = convertLink(v);
	if (v->isArray()) {
		if (v->empty())
			throw ArrayIsEmptyException(THISLOCATION) << ScriptException(THISLOCATION, loc);
		else
			v->erase(v->length());
	}
	else {
		throw OperationIsUndefined(THISLOCATION) << ScriptException(THISLOCATION, loc);
	}

}

const VarName & Oper_ArrayAppend::getName(IExprEnvironment &env) const
{
	try {
		LocalVarRef &rf = branch[0]->getIfc<LocalVarRef>();
		return rf.getName(env);
	}
	catch (LightSpeed::Exception &e) {
		e.appendReason(ScriptException(THISLOCATION, loc));
		throw;
	}
}

Value Oper_ArrayAppend::calculate(IExprEnvironment& env, const Value* subResults) const {
	throw OperationIsUndefined(THISLOCATION) << ScriptException(THISLOCATION,loc);
}

Tempe::Value Oper_ForEach::calculate(IExprEnvironment &env) const
{
	Value v = branch[0]->calculate(env);
	v = convertLink(v);
	LocalScope scope(env);
	scope.setVar("count", scope.getFactory().newValue(v->getEntryCount()));
	for (JSON::Iterator iter = v->getFwIter(); iter.hasItems();) {
		const JSON::KeyValue kv = iter.getNext();
		scope.setVar("this", kv.node);
		if (kv.getKeyType() == JSON::IKey::index)
			scope.setVar("index", scope.getFactory().newValue(kv.getIndex()));
		else
			scope.setVar("index", scope.getFactory().newValue(kv.getStringKey()));
		if (kv->isObject()) {
			LocalScope objscope(env, kv.node);
			LocalScope blockScope(objscope, scope.getObject());
			branch[1]->calculate(blockScope);
		}
		else {
			branch[1]->calculate(scope);
		}
	}
	return JSON::getNullNode();
}

Value Oper_IncludeTrace::calculate(IExprEnvironment& env) const {
	if (env.checkIncludeProcessed(path)) return JSON::getNullNode();
	else {
		env.markIncludeProcessed(path);
		return expr->calculate(env);
	}
}

Value Oper_ReferenceOper::calculate(IExprEnvironment& env) const {
	try {
		LocalVarRef &varNameBranch = branch[0]->getIfc<LocalVarRef>();
		Value context = varNameBranch.getContext(env);
		VarName name = varNameBranch.getName(env);
		if (context == nil) context = env.getVar("_current");
		JSON::INode *nd = context->getVariable(name);
		if (nd) {
			BoundVar *bv = nd->getIfcPtr<BoundVar>();
			if (bv) return nd;
		}
		return BoundVar(context,name).clone(&env.getFactory());
	} catch (LightSpeed::Exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	} catch (const std::exception &e) {
		throwScriptException(THISLOCATION,loc,e);throw;
	}
	throw;
}


Tempe::Value Oper_ArrayCreate::calculate(IExprEnvironment &env) const
{
	Value res = env.getFactory().array();
	for (natural i = 0; i < getN(); i++) {
		Value v = getBranch(i)->calculate(env);
		res->add(v);
	}
	return res;
}


Tempe::Value Oper_Varname::calculate(IExprEnvironment &env) const
{
	try {
		LocalVarRef &ref = branch[0]->getIfc<LocalVarRef>();
		if (ref.isDefined(env)) {
			Value v = ref.getValue(env);
			BoundVar *bv = v->getIfcPtr<BoundVar>();
			if (bv) {
				return env.getFactory().newValue(bv->getVarname());
			}
		}
		return env.getFactory().newValue(ref.getName(env));
	}
	catch (LightSpeed::Exception &e) {
		e.appendReason(ScriptException(THISLOCATION, this->loc));
		throw;
	}
}


Value dereferenceOfValue(const Value &v) 
{
		LinkValue *ln = v->getIfcPtr<LinkValue>();
	if (ln) return ln->link.get();

	BoundVar *bv = v->getIfcPtr<BoundVar>();
	if (bv) return bv->dereference();

	return v;
}

Tempe::Value Oper_Dereference::calculate(IExprEnvironment &env, const Value *subResults) const
{

	const Value v = subResults[0];
	return dereferenceOfValue(v);
}

bool Oper_Dereference::tryToEvalConst(IExprEnvironment &env, Value &val)
{
	Value x;
	if (branch[0]->tryToEvalConst(env, x)) {
		val = dereferenceOfValue(x);
		return true;
	}
	else {
		return false;
	}
}


}

