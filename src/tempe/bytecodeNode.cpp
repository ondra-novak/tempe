/*
 * bytecodeNode.cpp
 *
 *  Created on: 16. 3. 2016
 *      Author: ondra
 */

#include "bytecodeNode.h"

#include "basicOps.h"
#include "functions.h"
#include "functionVar.h"
#include "tempeOps.h"
#include "varTable.h"
namespace Tempe {


BytecodeNode::BytecodeNode(const ExprLocation &loc, ConstBin bytecode, ConstStringT<Value> constants, ConstStringT<StringA> strings)
:AbstractNode(loc)
,code(bytecode),constants(constants),strings(strings)
{

}


template<typename Stack>
static inline void runUnar(Stack &stack, IExprEnvironment &env,
		Value (*oper)(IExprEnvironment& , const Value& )) {
	Value v = stack.top();
	stack.pop();
	stack.push(oper(env,v));
}

template<typename Stack>
static inline void runBinar(Stack &stack, IExprEnvironment &env,
		Value (*oper)(IExprEnvironment& , const Value&, const Value& )) {

	Value a = stack.top();
	stack.pop();
	Value b = stack.top();
	stack.pop();
	stack.push(oper(env,a,b));
}




BytecodeNode::ExecutionState::~ExecutionState() {
	if (!envStack.empty()) {
		IExprEnvironment *env = envStack.top();
		while(!envStack.empty()) {
			delete env;
			envStack.pop();
			env = envStack.top();
		}
	}
}

BytecodeNode::ExecutionState *BytecodeNode::ExecutionState::singleStep() {

	try {

		//if there is anothe active state
		if (curState != this) {
				//execute this state instead and remember return value
				ExecutionState *s = curState->singleStep();
				//if return value is zeroe, current state finished
				if (s  == 0) {
					//if current state wasn't child state, try to find next state during next step
					if (curState != childState) curState = childState;
					//if current state was child state, then set both pointers to null
					//because we will continue by this state
					else curState = (childState = null);


				} else {

					resumeAfterReturn();

				}

		} else {

			IExprEnvironment &env = *envStack.top();

			OpCode op = (OpCode)curCode->code[ip];
			ip++;
			switch (op) {
				case opRet: return 0;
				case opPushConstant: stack.push(curCode->constants[readCompressed()]);
									 break;
				case opDelete: stack.pop();break;
				case opUnarMinus: runUnar(stack, env, operUnarMinus);break;
				case opUnarNot: runUnar(stack, env, operUnarNot);break;
				case opBinEqual: runBinar(stack, env, operEqual);break;
				case opBinGreater: runBinar(stack, env, operGreater);break;
				case opBinLess: runBinar(stack, env, operLess);break;
				case opBinGreatEqual: runBinar(stack, env, operGreatEqual);break;
				case opBinLessEqual: runBinar(stack, env, operLessEqual);break;
				case opBinNotEqual: runBinar(stack, env, operNotEqual);break;
				case opBinPlus: runBinar(stack, env, operPlus);break;
				case opBinMinus: runBinar(stack, env, operMinus);break;
				case opBinMul: runBinar(stack, env, operMul);break;
				case opBinIntegerDiv: runBinar(stack, env, operIntegerDiv);break;
				case opBinStringAppend: runBinar(stack, env, operStringAppend);break;
				case opArrIndex: runBinar(stack, env, fnArrIndex);break;
				case opScopeEnter:
						envStack.push(new LocalScope(env));
						break;
				case opScopeLeave:
						envStack.pop();
						if (!envStack.empty()) delete (&env);
						break;
				case opEnvPushObject:
						stack.push(env.getIfc<LocalScope>().getObject());
						break;
			};
			}
	} catch (const LightSpeed::Exception &e) {
		IExprEnvironment &env = *envStack.top();
		//if exception has been caught
		//convert exception to the value
		JSON::Value v = env.getFactory().newValue(e.getMessage());
		//set exception. In case, that exception cannot be set, throw exception out
		if (!setException(v)) throw;
		//no we are finished

	}
	return curState;



}

void BytecodeNode::createState(ExecutionState& state, IExprEnvironment& env) const{

	state.ip = 0;
	state.curState = &state;
	state.curCode = this;
	state.envStack.push(&env);

}

Value BytecodeNode::calculate(IExprEnvironment& env) const {

	ExecutionState state;
	createState(state,env);
	bool rep;
	do
		rep = state.singleStep() != 0;
	while (rep);
	return state.getReturnValue();

}


bool BytecodeNode::ExecutionState::setException(JSON::Value value) {
	if (childState != null && childState->setException(value)) return true;
	if (exceptionStack.empty()) return false;
	const ExceptionFrame &frame = exceptionStack.top();
	while (stack.length() > frame.stackSize) stack.pop();
	while (envStack.length() > frame.envStackSize) {
		delete envStack.top();
		envStack.pop();
	}
	ip = frame.handlerIP;
	exceptionStack.pop();
	return true;
}

natural BytecodeNode::ExecutionState::readCompressed()  {
	Utf8ToWideFilter flt;
	while (!flt.hasItems()) {
		flt.input((char)curCode->code[ip++]);
	}
	return (natural)flt.output();
}

JSON::Value BytecodeNode::ExecutionState::getReturnValue() {
	return stack.top();
}

BytecodeNode::ExecutionState::ExecutionState() {
}

bool BytecodeNode::ExecutionState::resumeAfterReturn() {
	if (childState == 0) return false;
	JSON::Value v = childState->getReturnValue();
	stack.push(v);
	childState = nil;
	return true;
}

BytecodeNode::BytecodeNode(const ExprLocation &loc):AbstractNode(loc) {}


class GeneratorState {
public:

	typedef AutoArray<byte, SmallAlloc<256> > Code;
	Code code;
	AutoArray<Value, SmallAlloc<16> > constants;
	AutoArray<StringA, SmallAlloc<16> > strings;
	IRuntimeAlloc &alloc;


	bool extractExecutableVar;

	natural scopesOpened;
	natural valuesPushed;
	natural exceptionFrames;


	GeneratorState(IRuntimeAlloc &alloc, bool extractExecutableVar):alloc(alloc),extractExecutableVar(extractExecutableVar)
		,scopesOpened(0),valuesPushed(0),exceptionFrames(0) {}

	void gen(PExprNode node);


	struct BreakFrame {
		natural scopesOpened;
		natural valuePushed;
		natural exceptionFrames;
		mutable AutoArray<natural, SmallAlloc<16> > fillAddrList;
	};

	Stack<BreakFrame> breakFrames;

	natural writeCompressed(natural num);
	natural encodeJumpTarget(natural from, natural to);
	natural writeJumpAddress(natural addrToWrite, natural toAddr, natural reservedSpace);

private:
	void beginCycle();
	void endCycle();
};

PExprNode BytecodeNode::generateBytecode(PExprNode node, IRuntimeAlloc& alloc, bool extractExecutableVar) {

	GeneratorState gen(alloc,extractExecutableVar);
	gen.gen(node);

	BytecodeNode *nd = new(alloc) BytecodeNode(node->getSourceLocation(), gen.code, gen.constants, gen.strings);
	PExprNode out = nd;

	return out;

}

void GeneratorState::beginCycle() {
	BreakFrame frm;
	frm.exceptionFrames = exceptionFrames;
	frm.scopesOpened = scopesOpened;
	frm.valuePushed = valuesPushed;
	breakFrames.push(frm);
}

void GeneratorState::endCycle() {
	natural endcode = code.length();
	const BreakFrame& frm = breakFrames.top();
	for (natural i = frm.fillAddrList.length(); i > 0;) {
		natural len = writeCompressed(
				encodeJumpTarget(frm.fillAddrList[i], endcode));
		for (natural i = 0; i < len; i++)
			code(frm.fillAddrList[i] + i + 1) = code[endcode + i];
		code.resize(endcode);
	}
}

void GeneratorState::gen(PExprNode nd) {

	for(Oper_Object *x = nd->getIfcPtr<Oper_Object>();x;) {
		code.add(opScopeEnter);
		scopesOpened++;
		gen(x);
		code.add(opDelete);
		code.add(opEnvPushObject);
		code.add(opScopeLeave);
		scopesOpened--;
		return;
	}

	for(Oper_Scope *x = nd->getIfcPtr<Oper_Scope>();x;) {
		code.add(opScopeEnter);
		scopesOpened++;
		gen(x);
		code.add(opScopeLeave);
		scopesOpened--;
		return;
	}

	for(Oper_Break *x = nd->getIfcPtr<Oper_Break>();x;) {
		const BreakFrame &frm = breakFrames.top();
		while (scopesOpened > frm.scopesOpened) {
			code.add(opScopeLeave);
			scopesOpened--;
		}
		while (valuesPushed > frm.valuePushed) {
			code.add(opDelete);
			valuesPushed--;
		}
		while (exceptionFrames > frm.exceptionFrames) {
			code.add(opEndTry);
			exceptionFrames--;
		}
		code.add(opPushNull);
		valuesPushed++;
		frm.fillAddrList.add(code.length());
		code.add(opJump);
		code.add(0,6);
		return;
	}

	for(OutputText *x = nd->getIfcPtr<OutputText>();x;) {
		code.add(opOutputText);
		writeCompressed(strings.length());
		strings.add(x->getText());
		return;
	}

	for(VariableRef *x = nd->getIfcPtr<VariableRef>();x;) {
		const VarName name = x->getName();
		code.add(opPushString);
		valuesPushed++;
		writeCompressed(strings.length());
		strings.add(name);
		return;
	}

	for (Constant *x = nd->getIfcPtr < Constant >();x;) {
		Value v = x->getValue();
		FunctionVar *fv = v->getIfcPtr<FunctionVar>();
		code.add(opPushConstant);
		writeCompressed(constants.length());
		valuesPushed++;
		if (fv && extractExecutableVar) {
			PExprNode code = fv->getCode();
			PExprNode node = BytecodeNode::generateBytecode(code,alloc,extractExecutableVar);
			Value v = new(alloc) FunctionVar(fv->getArguments(),node);
			constants.add(v);
		} else {
			constants.add(v);
		}
		return;
	}

	const AbstrNaryNode *nrnd = nd->getIfcPtr<AbstrNaryNode>();
	if (nrnd == 0) {
		Value v = new(alloc) FunctionVar(ConstStringT<FunctionVar::VarName_OutMode>(),nd);
		code.add(opExecTree);
		writeCompressed(constants.length());
		constants.add(v);
		return;
	}

	if (nrnd->getIfcPtr<Oper_TryCatch>()!= 0) {
		natural addr1 = code.length();
		code.add(opTry);
		code.add(0,10);
		gen(nrnd->getBranch(0));
	    natural addr2 = code.length();
	    code.add(opJump);
	    code.add(0,10);
	    natural addr3 = code.length();
	    valuesPushed++;//<here handler always starts with pushed exception on the stack
	    gen(nrnd->getBranch(1));
	    code.add(opCatch);
	    valuesPushed-=2; //<catch doesn't leave anyting at stack, so remove value and variable
	    gen(nrnd->getBranch(2));
	    natural shifted = writeJumpAddress(addr2,code.length(),10);
	    writeJumpAddress(addr1,addr3-shifted,10);
	    return;
	}

	if (nd->getIfcPtr<Oper_And>()!= 0) {

		gen(nrnd->getBranch(0));
		natural addr = code.length();
		code.add(opJumpF);
		code.add(0,10);
		code.add(opDelete); //<delete top value, it will be replaced by result of branch
		valuesPushed--;
		gen(nrnd->getBranch(1));
		writeJumpAddress(addr,code.length(),10);
		return;
	}
	if (nd->getIfcPtr<Oper_Or>()!= 0) {

		gen(nrnd->getBranch(0));
		natural addr = code.length();
		code.add(opJumpT);
		code.add(0,10);
		code.add(opDelete); //<delete top value, it will be replaced by result of branch
		valuesPushed--;
		gen(nrnd->getBranch(1));
		writeJumpAddress(addr,code.length(),10);
		return;

	}
	if (nd->getIfcPtr<Oper_Cycle>()!= 0) {

		//cycle can issue break, so begin cycle now and create break frame
		beginCycle();
		//we need to push null because first operation of the cycle is pop result of previous cycle

		code.add(opPushNull);
		valuesPushed++;
		natural addr = code.length();
		code.add(opDelete);
		valuesPushed--;
		gen(nrnd->getBranch(0));
		natural addr2 = code.length();
		writeCompressed(encodeJumpTarget(addr2,addr));
		//end cycle now
		endCycle();
		return;
	}
	else if (nd->getIfcPtr<Oper_If>()!= 0) {

		gen(nrnd->getBranch(0));
		natural addr1 = code.length();
		code.add(opJumpF);
		code.add(0,10);
		code.add(opDelete); //<delete top value, it will be replaced by result of branch
		valuesPushed--;
		gen(nrnd->getBranch(1));
		natural addr2 = code.length();
		code.add(opJump);
		code.add(0,10);
		natural addr3 = code.length();
		code.add(opDelete); //<delete top value, it will be replaced by result of branch
		valuesPushed--;
		gen(nrnd->getBranch(2));
		natural shift = writeJumpAddress(addr2,code.length(),10);
		writeJumpAddress(addr1, addr3-shift,10);
		return;
	}










}

natural GeneratorState::writeCompressed(natural num) {
	WideToUtf8Filter flt;
	flt.input((wchar_t)num);
	natural x = 0;
	while (flt.hasItems()) {
		code.add((byte)flt.output());
		x++;
	}
	return x;
}
natural GeneratorState::encodeJumpTarget(natural from, natural to) {
	if (from > to) return ((from - to) << 1)+1;
	else return (to - from) << 1;
}

natural GeneratorState::writeJumpAddress(natural addrToWrite, natural toAddr, natural reservedSpace) {
	//end of code
	natural endCode = code.length();
	//relative jump to write
	natural rel = encodeJumpTarget(addrToWrite,toAddr);
	//dry write and get length
	natural addrLen = writeCompressed(rel);
	//calculate how many bytes release
	natural freeExtra = reservedSpace - addrLen;
	//store current length
	natural prevAddrLen = reservedSpace;
	//find stable length
	while (prevAddrLen != addrLen) {

		//revert all writes
		code.resize(endCode);
		//update prevAddrLen
		prevAddrLen = addrLen;

		//calculate new target
		natural toAddr2 = toAddr>addrToWrite?toAddr-freeExtra:toAddr;

		//encode jump
		rel = encodeJumpTarget(addrToWrite,toAddr2);

		//write jump address
		addrLen = writeCompressed(rel);

		//calculate new freeExtra
		freeExtra = reservedSpace - addrLen;
		//repeat if differs from previous run
	}

	//now it is stable and we have jump address encoded at the end of code

	//write it to the place after jump
	addrToWrite++;
	for (natural i = endCode; i < code.length();i++) {
		code(addrToWrite++) = code[i];
	}

	//remove it from the end
	code.resize(endCode);

	//erase extra reserved space
	//we can move block code, because it should be relocateable until there is fixed
	//link between the block (should not be)
	code.erase(addrToWrite, freeExtra);

	//report, how many bytes has been code shifted
	return freeExtra;
}


}
