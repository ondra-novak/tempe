/*
 * functions.cpp
 *
 *  Created on: 4.7.2012
 *      Author: ondra
 */



#include "functions.h"

#include <lightspeed/base/countof.h>

#include "exceptions.h"
#include <math.h>
#include <cwchar>
#include <wctype.h>
#include <lightspeed/base/exceptions/errorMessageException.h>
#include <lightspeed/base/debug/dbglog.h>
#include <lightspeed/base/streams/standardIO.tcc>
#include <lightspeed/mt/process.h>
#include <lightspeed/utils/json/jsonfast.tcc>

#include "functionVar.h"
#include "../../../lightspeed/src/lightspeed/base/streams/secureRandom.h"
using LightSpeed::countof;
using std::wcschr;

namespace Tempe {

Value convertAndDoOper(IExprEnvironment &env, const Value &a, const Value &b, Value (*fn)(IExprEnvironment &, const Value &, const Value &)) {
	if (a->getType() == JSON::ndNull) return a;
	if (b->getType() == JSON::ndNull) return b;
	if (a->getType() == JSON::ndString) {
		return fn(env,a,env.getFactory().newValue(b->getString()));
	} else if (a->getType() == JSON::ndFloat) {
		if (b->getType() == JSON::ndString) {
			return fn(env,env.getFactory().newValue(a->getString()),b);
		} else if (b->getType() == JSON::ndInt) {
			return fn(env,a,env.getFactory().newValue(b->getFloat()));
		} else if (b->getType() == JSON::ndBool) {
			return fn(env,a,env.getFactory().newValue(b->getFloat()));
		}
	} else if (a->getType() == JSON::ndInt) {
		if (b->getType() == JSON::ndString) {
			return fn(env,env.getFactory().newValue(a->getString()),b);
		} else if (b->getType() == JSON::ndFloat) {
			return fn(env,env.getFactory().newValue(a->getFloat()),b);
		} else if (b->getType() == JSON::ndBool) {
			return fn(env,a,env.getFactory().newValue(b->getInt()));
		}

	} else if (a->getType() == JSON::ndBool) {
		if (b->getType() == JSON::ndString) {
			return fn(env,env.getFactory().newValue(a->getString()),b);
		} else if (b->getType() == JSON::ndFloat) {
			return fn(env,env.getFactory().newValue(a->getFloat()),b);
		} else if (b->getType() == JSON::ndInt) {
			return fn(env,env.getFactory().newValue(a->getInt()),b);
		}
	}
	throw ImpossibleConversion(THISLOCATION);
}


Value operEqual(IExprEnvironment& env,const Value &a,const Value &b) {
	if (a.get() == b.get()) return env.getFactory().newValue(true);
	if (a->getType() == JSON::ndArray || a->getType() == JSON::ndObject
			|| b->getType() == JSON::ndArray || b->getType() == JSON::ndObject)
		return env.getFactory().newValue(false);
	if (a->getType() != b->getType())
		return convertAndDoOper(env,a,b,&operGreatEqual);
	switch (a->getType()) {
		case JSON::ndString: return env.getFactory().newValue(a->getString() == b->getString());
		case JSON::ndInt: return env.getFactory().newValue(a->getInt() == b->getInt());
		case JSON::ndFloat: return env.getFactory().newValue(a->getFloat() == b->getFloat());
		case JSON::ndBool: return env.getFactory().newValue(a->getBool() == b->getBool());
		case JSON::ndNull: return a;
		default: throw OperationIsUndefined(THISLOCATION);
	}
}

Value operGreater(IExprEnvironment& env,const Value &a,const Value &b) {
	if (a->getType() != b->getType())
		return convertAndDoOper(env,a,b,&operGreater);
	switch (a->getType()) {
		case JSON::ndString: return env.getFactory().newValue(a->getString() > b->getString());
		case JSON::ndInt: return env.getFactory().newValue(a->getInt() > b->getInt());
		case JSON::ndFloat: return env.getFactory().newValue(a->getFloat() > b->getFloat());
		case JSON::ndBool: return env.getFactory().newValue(a->getBool() == true && b->getBool() == false);
		case JSON::ndNull: return a;
		default: throw OperationIsUndefined(THISLOCATION);
	}
}

Value operLess(IExprEnvironment& env,const Value &a,const Value &b) {
	return operGreater(env,b,a);
}

Value operGreatEqual(IExprEnvironment& env,const Value &a,const Value &b) {
	if (a->getType() != b->getType())
		return convertAndDoOper(env,a,b,&operGreatEqual);
	switch (a->getType()) {
		case JSON::ndString: return env.getFactory().newValue(a->getString() >= b->getString());
		case JSON::ndInt: return env.getFactory().newValue(a->getInt() >= b->getInt());
		case JSON::ndFloat: return env.getFactory().newValue(a->getFloat() >= b->getFloat());
		case JSON::ndBool: return env.getFactory().newValue(a->getBool() == true);
		case JSON::ndNull: return a;
		default: throw OperationIsUndefined(THISLOCATION);
	}
}

Value operLessEqual(IExprEnvironment& env,const Value &a,const Value &b) {
	return operGreatEqual(env,b,a);
}

Value operNotEqual(IExprEnvironment& env,const Value &a,const Value &b) {
	return operUnarNot(env,operEqual(env,a,b));
}

Value operPlus(IExprEnvironment& env, const Value& a, const Value& b) {
	if (a->getType() != b->getType())
		return convertAndDoOper(env,a,b,&operPlus);
	switch (a->getType()) {
		case JSON::ndString: return env.getFactory().newValue(String(a->getString() + b->getString()));
		case JSON::ndInt: return env.getFactory().newValue(a->getInt() + b->getInt());
		case JSON::ndFloat: return env.getFactory().newValue(a->getFloat() + b->getFloat());
		case JSON::ndArray:
				if (b->getType() == JSON::ndArray) {
					JSON::PNode out = env.getFactory().array();
					for (JSON::Iterator iter = a->getFwIter(); iter.hasItems();) out->add(iter.getNext().node);
					for (JSON::Iterator iter = b->getFwIter(); iter.hasItems();) out->add(iter.getNext().node);
					return out;
				} else {
					JSON::PNode out = env.getFactory().array();
					for (JSON::Iterator iter = a->getFwIter(); iter.hasItems();) out->add(iter.getNext().node);
					out->add(b);
					return out;
				}
		default: throw OperationIsUndefined(THISLOCATION);
	}
}

Value operMinus(IExprEnvironment& env, const Value& a, const Value& b) {
	if (a->getType() != b->getType())
		return convertAndDoOper(env,a,b,&operMinus);
	switch (a->getType()) {
		case JSON::ndInt: return env.getFactory().newValue(a->getInt() - b->getInt());
		case JSON::ndFloat: return env.getFactory().newValue(a->getFloat() - b->getFloat());
		default: throw OperationIsUndefined(THISLOCATION);
	}
}

Value operMul(IExprEnvironment& env, const Value& a, const Value& b) {
	if (a->getType() != b->getType())
		return convertAndDoOper(env,a,b,&operMul);
	switch (a->getType()) {
		case JSON::ndInt: return env.getFactory().newValue(a->getInt() * b->getInt());
		case JSON::ndFloat: return env.getFactory().newValue(a->getFloat() * b->getFloat());
		default: throw OperationIsUndefined(THISLOCATION);
	}
}

Value operDiv(IExprEnvironment& env, const Value& a, const Value& b) {
	if (a->getType() != b->getType())
		return convertAndDoOper(env,a,b,&operDiv);
	switch (a->getType()) {
		case JSON::ndInt:
		case JSON::ndFloat: return env.getFactory().newValue(a->getFloat() / b->getFloat());
		default: throw OperationIsUndefined(THISLOCATION);
	}
}

Value operMod(IExprEnvironment& env, const Value& a, const Value& b) {
	if (a->getType() != b->getType())
		return convertAndDoOper(env,a,b,&operMod);
	switch (a->getType()) {
		case JSON::ndInt: return env.getFactory().newValue(a->getInt() % b->getInt());
		case JSON::ndFloat: {
			double k = a->getFloat();
			double l = b->getFloat();
			return env.getFactory().newValue(k - floor( k / l) * l);
		}
		default: throw OperationIsUndefined(THISLOCATION);
	}
}

Value operIntegerDiv(IExprEnvironment& env, const Value& a, const Value& b) {
	if (a->getType() != b->getType())
		return convertAndDoOper(env,a,b,&operIntegerDiv);
	switch (a->getType()) {
		case JSON::ndInt: {
			int k = b->getInt();
			if (k == 0) throw ErrorMessageException(THISLOCATION,"Division by zero");
			return env.getFactory().newValue(a->getInt() / k);
		}
		case JSON::ndFloat: return env.getFactory().newValue((integer)(a->getFloat() / b->getFloat()));
		default: throw OperationIsUndefined(THISLOCATION);
	}

}

Value operStringAppend(IExprEnvironment& env, const Value& a, const Value& b) {
	return env.getFactory().newValue(String(a->getString() + b->getString()));
}

class WChrI {
public:
	WChrI(wchar_t x):x(x) {}

	operator wchar_t() const { return x; }

	bool operator==(const WChrI &y) const {return towupper(x) == towupper(y.x);}
	bool operator!=(const WChrI &y) const {return towupper(x) != towupper(y.x);}
	bool operator>=(const WChrI &y) const {return towupper(x) >= towupper(y.x);}
	bool operator<=(const WChrI &y) const {return towupper(x) <= towupper(y.x);}
	bool operator>(const WChrI &y) const {return towupper(x) > towupper(y.x);}
	bool operator<(const WChrI &y) const {return towupper(x) < towupper(y.x);}


protected:
	wchar_t x;
};


inline static bool isSeparator(wchar_t x) {
	wchar_t seps[]=L"\t\n\r\a\b !@#$%^&*()_-+=}{][;'\\:\"|<>?,./";
	return wcschr(seps,x) != 0;
}

template<typename X>
bool containWordT(ConstStringT<X> stra, ConstStringT<X> strb) {
	if (stra == strb) return true;
	natural k = stra.find(strb);
	while (k != naturalNull) {
		if ((stra.length() == k+strb.length() || isSeparator(stra[k+strb.length()]))
			&& (k == 0 || isSeparator(stra[k-1]))){
				return true;
		}
		k = stra.find(strb,k+1);
	}
	return false;

}

Value fnContain(IExprEnvironment& env, const Value& a, const Value& b) {
	ConstStrW stra=a->getString();
	ConstStrW strb=b->getString();
	ConstStringT<WChrI> istra(reinterpret_cast<const WChrI*>(stra.data()),stra.length());
	ConstStringT<WChrI> istrb(reinterpret_cast<const WChrI*>(strb.data()),strb.length());
	bool found = istra.find(istrb) != naturalNull;
	return env.getFactory().newValue(found);
}

Value fnContainWord(IExprEnvironment& env, const Value& a, const Value& b) {
	ConstStrW stra=a->getString();
	ConstStrW strb=b->getString();
	ConstStringT<WChrI> istra(reinterpret_cast<const WChrI*>(stra.data()),stra.length());
	ConstStringT<WChrI> istrb(reinterpret_cast<const WChrI*>(strb.data()),strb.length());
	return env.getFactory().newValue(containWordT(istra,istrb));

}

Value fnContainExact(IExprEnvironment& env, const Value& a, const Value& b) {
	ConstStrW stra=a->getString();
	ConstStrW strb=b->getString();
	bool found = stra.find(strb) != naturalNull;
	return env.getFactory().newValue(found);
}

Value fnContainWordExact(IExprEnvironment& env, const Value& a,
		const Value& b) {
	ConstStrW stra=a->getString();
	ConstStrW strb=b->getString();
	return env.getFactory().newValue(containWordT(stra,strb));
}

Value fnHead(IExprEnvironment& env, const Value& a, const Value& b) {
	if (a->isArray()) {
		Value r = env.getFactory().array();
		natural mx = b->getUInt();
		if (mx > a->length()) mx = a->length();
		for (natural i = 0; i < mx; i++) r->add(a[i]);
		return r;
	} else
		return env.getFactory().newValue(String(a->getString().head(b->getInt())));
}

Value fnTail(IExprEnvironment& env, const Value& a, const Value& b) {
	if (a->isArray()) {
		Value r = env.getFactory().array();
		natural mx = b->getUInt();
		natural l = a->length();
		if (mx > l) mx = l;
		for (natural i = mx; i > 0; i--) r->add(a[l-i]);
		return r;
	} else
	   return env.getFactory().newValue(String(a->getString().tail(b->getInt())));
}

Value fnOffset(IExprEnvironment& env, const Value& a, const Value& b) {
	if (a->isArray()) {
		Value r = env.getFactory().array();
		natural mx = b->getUInt();
		natural l = a->length();
		if (mx > l) mx = l;
		for (natural i = mx; i < l; i++) r->add(a[i]);
		return r;
	} else
		return env.getFactory().newValue(String(a->getString().offset(b->getInt())));
}

Value fnRoffset(IExprEnvironment& env, const Value& a, const Value& b) {
	if (a->isArray()) {
		Value r = env.getFactory().array();
		natural mx = b->getUInt();
		natural l = a->length();
		if (mx > l) mx = l;
		mx = l - mx;
		for (natural i = 0; i < mx; i++) r->add(a[i]);
		return r;
	}
	else
		return env.getFactory().newValue(String(a->getString().crop(0,b->getInt())));
}

typedef std::pair<ConstStrW,ConstStrW> DoSplitRes;
static DoSplitRes doSplit(const String &a, const String &b, int count) {
	if (count == 0) return DoSplitRes(a,ConstStrW());
	if (count > 0) {
		natural q = a.find(b);
		if (q == naturalNull) return DoSplitRes(a,ConstStrW());
		ConstStrW l = a.head(q);
		ConstStrW r = a.offset(q+b.length());
		while (count > 1) {
			q = r.find(b);
			if (q == naturalNull) return DoSplitRes(a,ConstStrW());
			q += l.length() + b.length();
			l = a.head(q);
			r = a.offset(q+b.length());
			count--;
		}
		return DoSplitRes(l,r);
	} else {
		natural q = a.findLast(b);
		if (q == naturalNull) return DoSplitRes(a,ConstStrW());
		ConstStrW l = a.head(q);
		ConstStrW r = a.offset(q+b.length());
		while (count < -1) {
			q = l.findLast(b);
			if (q == naturalNull) return DoSplitRes(a,ConstStrW());
			l = a.head(q);
			r = a.offset(q+b.length());
			count++;
		}
		return DoSplitRes(l,r);

	}
}

Value fnSplitAt(IExprEnvironment& env, const Value& a, const Value& b,const Value& c) {
	String what = a->getString();
	String where = b->getString();
	int count = c->getInt();
	return env.getFactory().newValue(doSplit(what,where,count).first);
}

Value fnRsplitAt(IExprEnvironment& env, const Value& a, const Value& b, const Value& c) {
	String what = a->getString();
	String where = b->getString();
	int count = c->getInt();
	return env.getFactory().newValue(doSplit(what,where,count).second);
}

Value fnToString(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(a->getString());
}

Value fnToInt(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(a->getInt());
}

Value fnToReal(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(a->getFloat());
}

Value fnRound(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(round(a->getFloat()));
}

Value fnFloor(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(floor(a->getFloat()));
}

Value fnCeil(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(ceil(a->getFloat()));
}

Value fnExp(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(exp(a->getFloat()));
}

Value fnPow(IExprEnvironment& env, const Value& a,  const Value& b) {
	return env.getFactory().newValue(pow(a->getFloat(),b->getFloat()));
}

Value fnSqrt(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(sqrt(a->getFloat()));
}

Value fnSin(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(sin(a->getFloat()));
}

Value fnCos(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(cos(a->getFloat()));
}

Value fnTan(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(tan(a->getFloat()));
}

Value fnASin(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(asin(a->getFloat()));
}

Value fnACos(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(acos(a->getFloat()));
}

Value operUnarMinus(IExprEnvironment& env, const Value& a) {
	switch (a->getType()) {
		case JSON::ndInt: return env.getFactory().newValue(-a->getInt());
		case JSON::ndFloat: return env.getFactory().newValue(-a->getFloat());
		case JSON::ndBool: return env.getFactory().newValue(a->getBool());
		case JSON::ndNull: return a;
		default: throw OperationIsUndefined(THISLOCATION);
	}
}

Value operUnarNot(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(!(a->getBool()));
}

Value fnATan(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(atan(a->getFloat()));
}

Value fnATan2(IExprEnvironment& env, const Value& a, const Value& b) {
	return env.getFactory().newValue(atan2(a->getFloat(),b->getFloat()));
}

Value fnLog(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(log(a->getFloat()));
}

Value fnLog10(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue(log10(a->getFloat()));
}

Value fnTypeOf(IExprEnvironment& env, const Value& a) {
	return env.getFactory().newValue((integer)(a->getType()));
}

Value fnCharAt(IExprEnvironment& env, const Value& a, const Value& b) {
	String k = a->getString();
	natural idx = (natural)(b->getInt());
	if (idx >= k.length())
		return env.getFactory().newValue(String());
	else
		return env.getFactory().newValue(ConstStrW(k[idx]));
}
Value fnReplace(IExprEnvironment& env, const Value& a, const Value& b,
		const Value& c, const Value& d) {
	String src = a->getString();
	String newVal = b->getString();
	natural idx = (natural)(c->getInt());
	natural count = (natural)(d->getInt());
	return env.getFactory().newValue(String(src.head(idx) + newVal + src.offset(idx+count)));
}

Value fnArrIndex(IExprEnvironment &env, const Value &arr, const Value &index) {
	if (arr->isArray()) {
		natural idx = index->getUInt();
		natural len = arr->length();
		if (idx < len) {
			return arr[idx];
		}
		else {
			throw RangeException(THISLOCATION, idx, len);
		}
	}
	else {
		String x = arr->getString();
		natural idx = index->getUInt();
		natural len = x.length();
		if (idx < len) {
			return env.getFactory().newValue(ConstStrW(x.mid(idx, 1)));
		}
		else {
			throw RangeException(THISLOCATION, idx, len );
		}
	}
}



Value fnCode(IExprEnvironment& env, const Value& a) {
	String k = a->getString();
	if (k.empty()) return env.getFactory().newNullNode();
	else return env.getFactory().newValue((integer)k[0]);
}

Value fnLength(IExprEnvironment& env, const Value& a) {
	if (a->isArray()) return env.getFactory().newValue(a->length());
	String k = a->getString();
	return env.getFactory().newValue(k.length());
}



Value fnDebug(IExprEnvironment& env , ArrayRef<Value> values) {
	AutoArrayStream<char> buff;
	for (natural i = 0; i < values.length(); i++) {
		JSON::PNode res = values[i];
		JSON::serialize(res,buff,false);
	}
	LS_LOG.note("Script log: %1") << ConstStrA(buff.getArray());
	return env.getFactory().newValue(true);

}

Value fnPrint(IExprEnvironment& env, ArrayRef<Value> values) {
	AutoArray<wchar_t> buff;
	for (natural i = 0; i < values.length(); i++) {
		buff.append(values[i]->getString());
	}
	ConsoleW con;
	con.print(L"%1\n") << ConstStrW(buff);
	return env.getFactory().newValue(true);
}


Value fnExec(IExprEnvironment& env, const Value& path, ArrayRef<Value> values) {
	Process pgm(path->getString());
	for (natural i = 0; i < values.length(); i++) {
		Value arg = values[i];
		pgm.arg(arg->getString());
	}
	integer r = pgm.exec();
	return env.getFactory().newValue(r);

}



Tempe::Value fnRand(IExprEnvironment &env, const Value &a)
{
	class RandomStream : public JSON::Null_t, public DynObject {
	public:
		SecureRandom &getRandomStream() { return r; }
	protected:
		SecureRandom r;
	};

	static ConstStrA name = "$__randomGeneratorInstance__$";

	JSON::IFactory &f = env.getFactory();
	IExprEnvironment &global = env.getInternalGlobalEnv();
	if (!global.varExists(name)) {
		global.setVar(name, new(*f.getAllocator()) RandomStream);
	}
	Value randomStream = global.getVar(name);
	SecureRandom &srand = randomStream->getIfc<RandomStream>().getRandomStream();
	Value res = f.array();
	integer count = a->getUInt();
	for (integer i = 0; i < count; i++) {
		res->add(f.newValue((natural)srand.getNext()));
	}
	return res;
}

Value fnScan(IExprEnvironment& env, const Value&  vsubj, const Value& vpattern) {

	class Parser: public TextParser<wchar_t> {
	public:
		natural count() const {return fragments.length();}
	};

	String subj = vsubj->getString();
	String pattern = vpattern->getString();
	Parser parser;
	if (parser(pattern,subj)) {

		JSON::PNode res = env.getFactory().array();
		for (natural i = 0; i < parser.count(); i++) {
			res->add(env.getFactory().newValue(parser[i].str()));
		}
		return res;

	} else {
		return env.getFactory().newValue(false);
	}
}

Value fnChr(IExprEnvironment& env,ArrayRef<Value> values) {
	AutoArray<wchar_t, SmallAlloc<32> > buffer;
	for (natural i = 0; i < values.length(); i++) {
		buffer.add((wchar_t)(values[i]->getUInt()));
	}
	return env.getFactory().newValue(ConstStrW(buffer));
}




template<typename Fn>
class NativeFunctionCall: public AbstractFunctionVar{
public:
	NativeFunctionCall(Fn fn):fn(fn) {}

	virtual Value execute(IExprEnvironment &env, ArrayRef<Value> values, Value context);
	virtual ConstStringT<VarName_OutMode> getArguments() const {
		return ConstStringT<VarName_OutMode>(args,sizeof(args)/sizeof(VarName_OutMode));
	}
	virtual INode *clone(JSON::PFactory factory) const {
		return new(*factory->getAllocator()) NativeFunctionCall(fn);
	}

	static const VarName_OutMode args[];

	Fn fn;

};
template<typename Fn> const AbstractFunctionVar::VarName_OutMode NativeFunctionCall<Fn>::args[] =
	{AbstractFunctionVar::VarName_OutMode("a",AbstractFunctionVar::byValue)};
template<>
const AbstractFunctionVar::VarName_OutMode
	NativeFunctionCall<Value (*)(IExprEnvironment&, const Value&)>::args[] =
		{AbstractFunctionVar::VarName_OutMode("a",AbstractFunctionVar::byValue)};

template<>
const AbstractFunctionVar::VarName_OutMode
	NativeFunctionCall<Value (*)(IExprEnvironment&, const Value&, const Value&)>::args[] =
		{AbstractFunctionVar::VarName_OutMode("a",AbstractFunctionVar::byValue),
		 AbstractFunctionVar::VarName_OutMode("b",AbstractFunctionVar::byValue)
		};

template<>
const AbstractFunctionVar::VarName_OutMode
	NativeFunctionCall<Value (*)(IExprEnvironment&, const Value&, const Value&, const Value&)>::args[] =
		{AbstractFunctionVar::VarName_OutMode("a",AbstractFunctionVar::byValue),
		 AbstractFunctionVar::VarName_OutMode("b",AbstractFunctionVar::byValue),
		 AbstractFunctionVar::VarName_OutMode("c",AbstractFunctionVar::byValue)
		};

template<>
const AbstractFunctionVar::VarName_OutMode
	NativeFunctionCall<Value (*)(IExprEnvironment&, const Value&, const Value&, const Value&, const Value&)>::args[] =
		{AbstractFunctionVar::VarName_OutMode("a",AbstractFunctionVar::byValue),
		 AbstractFunctionVar::VarName_OutMode("b",AbstractFunctionVar::byValue),
		 AbstractFunctionVar::VarName_OutMode("c",AbstractFunctionVar::byValue),
		 AbstractFunctionVar::VarName_OutMode("d",AbstractFunctionVar::byValue)
		};

template<>
const AbstractFunctionVar::VarName_OutMode
	NativeFunctionCall<Value (*)(IExprEnvironment&,  ArrayRef<Value> )>::args[] =
		{AbstractFunctionVar::VarName_OutMode("a",AbstractFunctionVar::variadic)
		};

template<>
const AbstractFunctionVar::VarName_OutMode
	NativeFunctionCall<Value (*)(IExprEnvironment&, const Value&, ArrayRef<Value>)>::args[] =
		{AbstractFunctionVar::VarName_OutMode("a",AbstractFunctionVar::byValue),
		 AbstractFunctionVar::VarName_OutMode("b",AbstractFunctionVar::variadic)};


template<>
Value NativeFunctionCall<Value (*)(IExprEnvironment&, const Value&)>::execute(IExprEnvironment &env, ArrayRef<Value> values, Value) {
	return fn(env,values[0]);
}

template<>
Value NativeFunctionCall<Value (*)(IExprEnvironment&, const Value&, const Value&)>::execute(IExprEnvironment &env, ArrayRef<Value> values, Value) {
	return fn(env,values[0],values[1]);
}

template<>
Value NativeFunctionCall<Value (*)(IExprEnvironment&, const Value&, const Value&, const Value&)>::execute(IExprEnvironment &env, ArrayRef<Value> values, Value) {
	return fn(env,values[0],values[1],values[2]);
}

template<>
Value NativeFunctionCall<Value (*)(IExprEnvironment&, const Value&, const Value&, const Value&, const Value&)>::execute(IExprEnvironment &env, ArrayRef<Value> values, Value) {
	return fn(env,values[0],values[1],values[2],values[3]);
}

template<>
Value NativeFunctionCall<Value (*)(IExprEnvironment&, ArrayRef<Value> )>::execute(IExprEnvironment &env, ArrayRef<Value> values, Value) {
	return fn(env,values);
}

template<>
Value NativeFunctionCall<Value (*)(IExprEnvironment&, const Value&, ArrayRef<Value> )>::execute(IExprEnvironment &env, ArrayRef<Value> values, Value) {
	return fn(env,values[0],values.offset(1));
}



template<typename Fn> AbstractFunctionVar *createFnCall(IRuntimeAlloc &factory, Fn fn) {
	return new(factory) NativeFunctionCall<Fn>(fn);
}


template AbstractFunctionVar *createFnCall(IRuntimeAlloc &factory, Value (*)(IExprEnvironment&, const Value&));
template AbstractFunctionVar *createFnCall(IRuntimeAlloc &factory, Value (*)(IExprEnvironment&, const Value&, const Value&));
template AbstractFunctionVar *createFnCall(IRuntimeAlloc &factory, Value (*)(IExprEnvironment&, const Value&, const Value&, const Value&));
template AbstractFunctionVar *createFnCall(IRuntimeAlloc &factory, Value (*)(IExprEnvironment&, const Value&, const Value&, const Value&, const Value&));
template AbstractFunctionVar *createFnCall(IRuntimeAlloc &factory, Value (*)(IExprEnvironment&, ArrayRef<Value> ));
template AbstractFunctionVar *createFnCall(IRuntimeAlloc &factory, Value (*)(IExprEnvironment&, const Value&, ArrayRef<Value> ));



}

