/*
 * exceptions.h
 *
 *  Created on: 4.7.2012
 *      Author: ondra
 */

#ifndef AEXPRESS_EXCEPTIONS_H_
#define AEXPRESS_EXCEPTIONS_H_

#include <lightspeed/base/exceptions/exception.h>
#include <lightspeed/base/exceptions/stdexception.h>

#include "interfaces.h"
namespace Tempe {

using namespace LightSpeed;


	class Exception: public virtual LightSpeed::Exception {
	public:
		Exception():LightSpeed::Exception(THISLOCATION) {}
		virtual ~Exception() throw() {}
	};

	class ExceptionWithLocation: public Exception {
	public:
		ExceptionWithLocation(const ExprLocation &eloc):eloc(eloc) {}
		~ExceptionWithLocation() throw () {}
		const ExprLocation &getScriptLoc() const {return eloc;}
	protected:
		ExprLocation eloc;
	};

	class ScriptException: public ExceptionWithLocation {
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		ScriptException(const ProgramLocation &loc, const ExprLocation &eloc)
			:LightSpeed::Exception(loc)
			,ExceptionWithLocation(eloc) {}

		virtual ~ScriptException() throw() {}

		void message(ExceptionMsg &msg) const;
	protected:

	};

	class ParseError: public ExceptionWithLocation {
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		ParseError(const ProgramLocation &loc, const ExprLocation &eloc, String comment);


		virtual ~ParseError() throw () {}
		void message(ExceptionMsg &msg) const;
	protected:
		String comment;

	};

	class OperationIsUndefined: public Exception {
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		OperationIsUndefined(const ProgramLocation &loc);
		void message(ExceptionMsg &msg) const;
	};

	class NoLongerAvailableException: public Exception {
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		NoLongerAvailableException(const ProgramLocation &loc);
		void message(ExceptionMsg &msg) const;
	};

	class ImpossibleConversion: public Exception {
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		ImpossibleConversion(const ProgramLocation &loc);
		void message(ExceptionMsg &msg) const;
	};

	class ExecutionTimeout : public Exception {
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		ExecutionTimeout(const ProgramLocation &loc):LightSpeed::Exception(loc) {}
		void message(ExceptionMsg &msg) const;
	};


	class VariableNotExistException: public Exception {
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		VariableNotExistException(const ProgramLocation &loc, const VarName &varName);
		~VariableNotExistException() throw () {}
		const VarName &getVariable() const;
		void message(ExceptionMsg &msg) const;
	protected:

		VarName varName;
	};

	class InvalidParamCountException: public Exception {
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		InvalidParamCountException(const ProgramLocation &loc, const VarName &fnName, const natural &count, const natural &needCount);
		~InvalidParamCountException() throw () {}
		const natural &getCount() const;
		const natural &getNeedCount() const;
		const VarName &getVariable() const;
		void message(ExceptionMsg &msg) const;
	protected:

		VarName fnName;
		natural count;
		natural needCount;

	};

	class RangeException: public Exception {
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		RangeException(const ProgramLocation &loc, const natural &count, const natural &needCount);
		~RangeException() throw () {}
		const natural &getCount() const;
		const natural &getNeedCount() const;
		void message(ExceptionMsg &msg) const;
	protected:

		natural count;
		natural needCount;

	};

	class ArrayIsEmptyException: public Exception {
	public:
		LIGHTSPEED_EXCEPTIONFINAL;

		ArrayIsEmptyException(const ProgramLocation &loc) :LightSpeed::Exception(loc) {}
		~ArrayIsEmptyException() throw () {}
		void message(ExceptionMsg &msg) const;

	};

	class BreakException: public Exception {
	public:
		BreakException(const ProgramLocation &loc, const ExprLocation &eloc)
		:LightSpeed::Exception(loc),eloc(eloc) {}
		~BreakException() throw() {}

		const ExprLocation getLoc() const {return eloc;}

		LIGHTSPEED_EXCEPTIONFINAL;

	protected:

		ExprLocation eloc;

		void message(ExceptionMsg &msg) const;

	};

}



#endif /* AEXPRESS_EXCEPTIONS_H_ */
