#pragma  once
#include "interfaces.h"
#include "escapeMode.h"

namespace Tempe {

	class OutputText : public AbstractNode {
	public:
		OutputText(const ExprLocation &loc, StringA text);

		virtual Value calculate(IExprEnvironment &env) const;
	protected:
		StringA text;
	};


	class OutputResult : public NaryNode<1> {
	public:

		OutputResult(const ExprLocation &loc, EscapeMode escMode);

		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const;


	protected:
		EscapeMode escMode;
	};
}