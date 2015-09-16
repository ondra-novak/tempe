/*
 * functions.h
 *
 *  Created on: 4.7.2012
 *      Author: ondra
 */

#ifndef AEXPRESS_FUNCTIONS_H_
#define AEXPRESS_FUNCTIONS_H_

#include "interfaces.h"

namespace Tempe {



Value operUnarMinus(IExprEnvironment &env,const Value &a);
Value operUnarNot(IExprEnvironment &env,const Value &a);
Value operEqual(IExprEnvironment &env,const Value &a,const Value &b);
Value operGreater(IExprEnvironment &env,const Value &a,const Value &b);
Value operLess(IExprEnvironment &env,const Value &a,const Value &b);
Value operGreatEqual(IExprEnvironment &env,const Value &a,const Value &b);
Value operLessEqual(IExprEnvironment &env,const Value &a,const Value &b);
Value operNotEqual(IExprEnvironment &env,const Value &a,const Value &b);
Value operPlus(IExprEnvironment &env,const Value &a,const Value &b);
Value operMinus(IExprEnvironment &env,const Value &a,const Value &b);
Value operMul(IExprEnvironment &env,const Value &a,const Value &b);
Value operDiv(IExprEnvironment &env,const Value &a,const Value &b);
Value operMod(IExprEnvironment &env,const Value &a,const Value &b);
Value operIntegerDiv(IExprEnvironment &env,const Value &a,const Value &b);
Value operStringAppend(IExprEnvironment &env,const Value &a,const Value &b);
Value fnContain(IExprEnvironment &env,const Value &a,const Value &b);
Value fnContainWord(IExprEnvironment &env,const Value &a,const Value &b);
Value fnContainExact(IExprEnvironment &env,const Value &a,const Value &b);
Value fnContainWordExact(IExprEnvironment &env,const Value &a,const Value &b);
Value fnHead(IExprEnvironment &env,const Value &a,const Value &b);
Value fnTail(IExprEnvironment &env,const Value &a,const Value &b);
Value fnOffset(IExprEnvironment &env,const Value &a,const Value &b);
Value fnRoffset(IExprEnvironment &env,const Value &a,const Value &b);
Value fnSplitAt(IExprEnvironment &env,const Value &a,const Value &b,const Value &c);
Value fnRsplitAt(IExprEnvironment &env,const Value &a,const Value &b,const Value &c);
Value fnToString(IExprEnvironment &env,const Value &a);
Value fnToInt(IExprEnvironment &env,const Value &a);
Value fnToReal(IExprEnvironment &env,const Value &a);
Value fnRound(IExprEnvironment &env,const Value &a);
Value fnFloor(IExprEnvironment &env,const Value &a);
Value fnCeil(IExprEnvironment &env,const Value &a);
Value fnExp(IExprEnvironment &env,const Value &a);
Value fnPow(IExprEnvironment &env,const Value &a, const Value& b);
Value fnSqrt(IExprEnvironment &env,const Value &a);
Value fnSin(IExprEnvironment &env,const Value &a);
Value fnCos(IExprEnvironment &env,const Value &a);
Value fnTan(IExprEnvironment &env,const Value &a);
Value fnASin(IExprEnvironment &env,const Value &a);
Value fnACos(IExprEnvironment &env,const Value &a);
Value fnATan(IExprEnvironment &env,const Value &a);
Value fnATan2(IExprEnvironment &env,const Value &a,const Value &b);
Value fnLog(IExprEnvironment &env,const Value &a);
Value fnLog10(IExprEnvironment &env,const Value &a);
Value fnTypeOf(IExprEnvironment &env,const Value &a);
Value fnCharAt(IExprEnvironment &env,const Value &a,const Value &b);
Value fnReplace(IExprEnvironment &env,const Value &a,const Value &b,const Value &c, const Value &d);
Value fnCode(IExprEnvironment &env,const Value &a);
Value fnLength(IExprEnvironment &env,const Value &a);
Value fnDebug(IExprEnvironment &env,ArrayRef<Value> values);
Value fnPrint(IExprEnvironment &env,ArrayRef<Value> values);
Value fnExec(IExprEnvironment &env,const Value &a, ArrayRef<Value> values);
Value fnScan(IExprEnvironment &env,const Value &a, const Value &b);
Value fnChr(IExprEnvironment &env,ArrayRef<Value> values);
Value fnArray(IExprEnvironment &env,ArrayRef<Value> values);


class AbstractFunctionVar;

template<typename Fn>
AbstractFunctionVar *createFnCall(IRuntimeAlloc &factory, Fn fn);


}


#endif /* AEXPRESS_FUNCTIONS_H_ */
