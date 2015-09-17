/*
 * exceptions.cpp
 *
 *  Created on: 4.7.2012
 *      Author: ondra
 */


#include "exceptions.h"
#include "interfaces.h"

namespace Tempe {



void ScriptException::message(ExceptionMsg& msg) const {

	msg("In expression at position (%1:%2)") << eloc.getFileName() << eloc.getPosition();

}

void throwScriptException(const ProgramLocation &loc, const ExprLocation &eloc, LightSpeed::Exception &e) {
	e.appendReason(ScriptException(loc,eloc));
	throw;
}
void throwScriptException(const ProgramLocation &loc, const ExprLocation &eloc, const std::exception &e) {
	throw StdException(loc,e) << ScriptException(loc,eloc);
}



ParseError::ParseError(const ProgramLocation& loc,
		const ExprLocation& eloc, String comment):LightSpeed::Exception(loc),ExceptionWithLocation(eloc),comment(comment) {
}

void ParseError::message(ExceptionMsg& msg) const {
	msg("Parse error at position (%1:%2) %3") << eloc.getFileName() << eloc.getPosition() << comment;
}

OperationIsUndefined::OperationIsUndefined(
		const ProgramLocation& loc):LightSpeed::Exception(loc) {
}

void OperationIsUndefined::message(ExceptionMsg& msg) const {
	msg("Operation is not defined");
}



ImpossibleConversion::ImpossibleConversion(
		const ProgramLocation& loc):LightSpeed::Exception(loc) {
}

void ImpossibleConversion::message(ExceptionMsg& msg) const {
	msg("Impossible conversion");
}

NoLongerAvailableException::NoLongerAvailableException(
		const ProgramLocation& loc):LightSpeed::Exception(loc) {
}

void NoLongerAvailableException::message(ExceptionMsg& msg) const {
	msg("Object is no longer available (link)");
}





VariableNotExistException::VariableNotExistException(
		const ProgramLocation& loc, const VarName& varName)
:LightSpeed::Exception(loc),varName(varName)
{
}

const VarName& VariableNotExistException::getVariable() const {
	return varName;
}

void VariableNotExistException::message(ExceptionMsg& msg) const {
	msg("Variable has not been created yet: `%1`") << varName;
}



Tempe::InvalidParamCountException::InvalidParamCountException(
		const ProgramLocation& loc, const VarName &fnName,
		const natural& count, const natural &needCount)
	:LightSpeed::Exception(loc),fnName(fnName),count(count),needCount(needCount)
{
}

const natural& Tempe::InvalidParamCountException::getCount() const {
	return count;
}

const natural& Tempe::InvalidParamCountException::getNeedCount() const {
	return needCount;
}

const VarName& InvalidParamCountException::getVariable() const {
	return fnName;
}

void InvalidParamCountException::message(ExceptionMsg& msg) const {
	msg("Function '%1' needs %2 parameters (%3 specified)") << fnName << needCount << count;
}


Tempe::RangeException::RangeException(
		const ProgramLocation& loc,
		const natural& count, const natural &needCount)
	:LightSpeed::Exception(loc),count(count),needCount(needCount)
{
}

const natural& Tempe::RangeException::getCount() const {
	return count;
}

const natural& Tempe::RangeException::getNeedCount() const {
	return needCount;
}


void Tempe::RangeException::message(ExceptionMsg& msg) const {
	msg("Range error: !( %2 < %1 )") << needCount << count;
}


void BreakException::message(ExceptionMsg& msg) const {
	{
				msg("Break outside of loop");
			}
}


}

