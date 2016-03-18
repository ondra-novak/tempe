/*
 * bytecodeNode.h
 *
 *  Created on: 16. 3. 2016
 *      Author: ondra
 */

#ifndef SRC_TEMPE_BYTECODENODE_H_
#define SRC_TEMPE_BYTECODENODE_H_

#include <lightspeed/base/containers/constStr.h>
#include <lightspeed/base/containers/stack.h>

#include "interfaces.h"

using Tempe::Value;


namespace Tempe {

using namespace LightSpeed;

enum OpCode {
	opRet, ///< push stack, and finish code execution
	opDelete, ///< delete single value on the stack
	opPushConstant, ///< push constant from the constant container
	opPushNull,
	opPushTrue,
	opPushFalse,
	opUnarMinus,
	opUnarNot,
	opBinEqual,
	opBinGreater,
	opBinLess,
	opBinGreatEqual,
	opBinLessEqual,
	opBinNotEqual,
	opBinPlus,
	opBinMinus,
	opBinMul,
	opBinIntegerDiv,
	opBinStringAppend,
	opArrIndex,
	opScopeEnter,
	opScopeLeave,
	opEnvPushObject, ///< push object to stack extracted from current environment, doesn't close scope
	opOutputText,  ///<outputs constant text - immediatelly follows compressed index of string
	opPushString, ///<push string from the strings
	opJump, ///<jump relative
	opJumpT, ///< jump if true
	opJumpF, ///< jump if false
	opExecTree, ///< execute a unknown tree. Tree is specified as function var, however,it is not treat as function
	opTry, ///<begin of exception frame - follows relative jump to handler
	opEndTry, ///<end of exception frame
	opCatch,  ///<begin of excep.handler. At stack is exception and reference to variable. Stores exception to variable

};


class BytecodeNode: public AbstractNode {
public:
	BytecodeNode(const ExprLocation &loc, ConstBin bytecode, ConstStringT<Value> constants, ConstStringT<StringA> strings);


	///compiles subtree and generates bytecode node
	/**
	 * @param node root of tree, where bytecode should be created
	 * @param alloc allocator to allocate new node
	 * @param extractExecutableVar If node creates executable variable, its subtree will be also
	 *    converted to bytecode
	 * @return
	 */
	static PExprNode generateBytecode(PExprNode node, IRuntimeAlloc &alloc, bool extractExecutableVar = true);

	///Holds state of execution
	class ExecutionState {
	public:

		ExecutionState();
		~ExecutionState();


		struct ExceptionFrame {
			///size of stack, when frame was created
			/** it will be used  to unwind stack when exception is thrown*/
			natural stackSize;
			///size of environment stack, when frame was created
			/** it will be used  to unwind stack when exception is thrown*/
			natural envStackSize;
			/// IP of handler
			natural handlerIP;


			ExceptionFrame(natural stackSize,natural envStackSize,natural handlerIP)
				:stackSize(stackSize),envStackSize(envStackSize),handlerIP(handlerIP) {}
		};

		///current instruction pointer
		natural ip;
		///pointer to current code which refers this state
		RefCntPtr<const BytecodeNode> curCode;
		///calculation stack
		Stack<Value, SmallAlloc<32> > stack;
		///environment stack
		Stack<IExprEnvironment *, SmallAlloc<8> > envStack;
		///pointer to child execution state - if exists
		AllocPointer<ExecutionState> childState;
		///current valid execution state (contains this, if there is no child execution state)
		ExecutionState *curState;


		Stack<ExceptionFrame, SmallAlloc<8> > exceptionStack;


		///Perform single execution step
		/** You can run program steb by step and anytime interrupt its execution regardless on
		 * its internal state (regardless on how many times the code enters into the function)
		 *
		 * @return function returns pointer to execution state which is currently in progress.
		 *   This can be different than argument. It allows to optimize stepping. When different
		 *   execution state is activated, code may be stepped separatedly, however it should
		 *   also corretly handle returning or exception handling. If function returns 0,
		 *   current code has been finished
		 * @exception any in case that code throws uncaught exception, it is thrown out also
		 * as exception. If exception is caught inside of the code, the exception handler is called.
		 * Uncaugh exception is exception thrown from the code when there is no exception handler defined.
		 */
		ExecutionState *singleStep();

		///Sets exception for this exectuion state
		/** finds top exception handler, unwinds stacks, sets instruction pointer to the handler,
		 *  pushes the exception value to the stack, removes the exception frame. Doesn't execute the code.
		 *
		 *  You should avoid to set exception again, because it is interpreted as exception in the
		 *  handler. Every call of setException remove one exception frame from the stack and unwinds
		 *  all other stacks.
		 *
		 * @param value exception value
		 * @retval true exception succesfully set
		 * @retval false exception cannot be set, no handler exists
		 */
		bool setException(JSON::Value value);

		natural readCompressed() ;

		///Retrieves return value from the state (finished code)
		JSON::Value getReturnValue();

		///Resumes current execution after all stacked execution states finished
		/** Function picks return value from childState
		 *
		 * @retval true success
		 * @retval false can't resume, no childState available
		 *
		 * */
		bool resumeAfterReturn();

	private:
		ExecutionState(const ExecutionState &);
		ExecutionState&operator=(const ExecutionState &);
	};

	friend class ExecutionState;



	///Creates exectution state to start the execution
	void createState(ExecutionState &state, IExprEnvironment &env) const;

protected:

	virtual Value calculate(IExprEnvironment &env) const;


	typedef AutoArray<byte, SmallAlloc<256> > Code;
	Code code;
	AutoArray<Value, SmallAlloc<16> > constants;
	AutoArray<StringA, SmallAlloc<16> > strings;


	void generateCode(PExprNode node, bool extractExecutableVar);
	BytecodeNode(const ExprLocation &loc);


};



} /* namespace tempe */


#endif /* SRC_TEMPE_BYTECODENODE_H_ */
