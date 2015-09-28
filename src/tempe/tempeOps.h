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



	class OutputConfig : public RefCntObj, public DynObject {
	public:
		EscapeMode escMode;
		Bin::natural16 minwidth;
		Bin::natural16 groupsize;
		Bin::natural16 precision;
		wchar_t fillchar;
		wchar_t delimiter;
		wchar_t decimalmark;
		wchar_t posPrefix;
		wchar_t negPrefix;
		wchar_t posSuffix;
		wchar_t negSuffix;
		bool fixed;
		
		StringA strTrue;
		StringA strFalse;
		StringA strNull;

		OutputConfig();

		void load(JSON::PNode cfg);
		void setContent(ConstStrA content);
	};

	typedef RefCntPtr<OutputConfig>  POutputConfig;

	class OutputResult : public NaryNode<1> {
	public:

		OutputResult(const ExprLocation &loc, POutputConfig cfg, EscapeMode escModeOverride);

		virtual Value calculate(IExprEnvironment &env, const Value *subResults) const;
	protected:
		POutputConfig cfg;
		EscapeMode escModeOverride;
	};
}
