#pragma once
#include "interfaces.h"
#include <lightspeed/base/containers/constStr.h>
#include <lightspeed/base/containers/autoArray.h>

namespace Tempe {

	using namespace LightSpeed;

	class TokenReader: public SourceReader {
	public:

		enum Symbol {

			sValue,
			sVarname,

			symbSemicolon,  ///< symbol ;
			symbAsssign,		///< symbol :=
			symbEqual,
			symbNotEqual,
			symbAbove,
			symbBelow,
			symbAboveEqual,
			symbBelowEqual,
			symbPlus,
			symbMinus,
			symbAmp,
			symbIntDiv,
			symbDiv,
			symbMult,
			symbMod,
			symbOBracket,
			symbCBracket,
			symbComma,
			symbOSqBracket,
			symbCSqBracket,
			symbDot,
			symbOBrace,
			symbCBrace,
			symbMoney,
			symbExl,
			symbDblColon,
			symbQColon,
			symbHash,
			symbEscape,
			symbUnknown,
			symbThreeDots,
			

			kwOr,
			kwAnd,
			kwEnd,
			kwIf,
			kwThen,
			kwElse,
			kwElseIf,
			kwNot,
			kwDefined,
			kwFirstDefined,
			kwUnset,
			kwVar,
			kwNew,
			kwIsNull,
			kwLoop,
			kwTry,
			kwCatch,
			kwWith,
			kwScope,
			kwObject,
			kwWhile,
			kwFunction,
			kwBreak,
			kwThrow,
			kwTrue,
			kwFalse,
			kwNull,
			kwLink,
			kwJoin,
			kwMap,		
			kwInclude,
			kwImport,
			kwSet,
			kwDo,
			kwOptional,
			eof,
			begin

		};

		Symbol type;
		Value value;
		ConstStrA varname;

		Symbol getNext() ;
		void accept();

		TokenReader(IRuntimeAlloc &alloc, SeqFileInput srcStream, FilePath fname, natural offset = 0);
		
		void enterLevel();
		void leaveLevel();
		void resetLevel(bool interactive);

	protected:
		void eatWhite();
		void readNext();
		bool accepted;
		AutoArray<char> buffer;
		JSON::PFactory factory;

		natural level;

		


				
	};


	class Compiler2 {
	public:

		Compiler2(IRuntimeAlloc &alloc);

		virtual PExprNode compile(TokenReader &reader);
		virtual PExprNode compileInteractive(TokenReader &reader);

	protected:
		virtual PExprNode compileExprSeq(TokenReader& reader);
		virtual PExprNode compileAssign(TokenReader& reader);
		virtual PExprNode compileOR(TokenReader& reader);
		virtual PExprNode compileAND(TokenReader& reader);
		virtual PExprNode compileRELAT(TokenReader& reader);
		virtual PExprNode compilePLUSMINUS(TokenReader& reader);
		virtual PExprNode compileMULTDIV(TokenReader& reader);
		virtual PExprNode compileUNAR(TokenReader& reader);

		PExprNode compileSubExpr(ExprLocation loc, TokenReader &reader);

		PExprNode compileOpUnset(ExprLocation loc, TokenReader& reader);

		PExprNode compileOpFirstDefined(ExprLocation loc, TokenReader &reader);

//		virtual PExprNode compileIF(TokenReader& reader);
		virtual PExprNode compileUNARSuffix(TokenReader& reader);
		PExprNode compileFunctOp(PExprNode nd, TokenReader& reader);
		PExprNode compileArrayOp(PExprNode nd, TokenReader& reader);
		PExprNode compileMemberAccessOp(PExprNode nd, TokenReader& reader);
		PExprNode compileIF(ExprLocation loc, TokenReader& reader);
		PExprNode compileOpTryCatch(ExprLocation loc, TokenReader& reader);
		PExprNode compileOpWith(ExprLocation loc, TokenReader& reader);
		PExprNode compileOpScope(ExprLocation loc, TokenReader& reader);
		PExprNode compileOpObject(ExprLocation loc, TokenReader& reader);
		PExprNode compileOpWhile(ExprLocation loc, TokenReader& reader);
		PExprNode compileOpFunction(ExprLocation loc, TokenReader& reader);
		void throwExpectedError(const ExprLocation &loc, ConstStringT<TokenReader::Symbol> symbols);
		void throwUnexpectedError(const ExprLocation &loc, ConstStringT<TokenReader::Symbol> symbols);
	public:
		IRuntimeAlloc &alloc;
		JSON::PFactory valueFactory;

	};
}