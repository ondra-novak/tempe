#pragma once
#include "interfaces.h"
#include <lightspeed/base/containers/constStr.h>
#include <lightspeed/base/containers/autoArray.h>
#include "escapeMode.h"
#include "lightspeed/base/namedEnum.h"
#include "varTable.h"

namespace Tempe {

	using namespace LightSpeed;

	class TokenReader: public SourceReader {
	public:

		enum Symbol {

			sValue,
			sVarname,
			sLabel,

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
			symbBeginTemplate,
			symbEndTemplate,
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
			kwTemplate,
			kwForeach,
			kwConst,
			kwEcho,

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

		void expectLabel();

	protected:
		void eatWhite();
		void readNext();
		bool accepted;
		AutoArray<char> buffer;
		JSON::PFactory factory;

		natural level;
		bool expLabel;

		


				
	};


	class Compiler {
	public:

		Compiler(IRuntimeAlloc &alloc);

		virtual PExprNode compile(TokenReader &reader);
		virtual PExprNode compileInteractive(TokenReader &reader);
		virtual PExprNode compileTemplate(TokenReader &reader, EscapeMode em);

		virtual PExprNode compileExprSeq(TokenReader& reader);
		virtual PExprNode compileAssign(TokenReader& reader);
		virtual PExprNode compileOR(TokenReader& reader);
		virtual PExprNode compileAND(TokenReader& reader);
		virtual PExprNode compileRELAT(TokenReader& reader);
		virtual PExprNode compilePLUSMINUS(TokenReader& reader);
		virtual PExprNode compileMULTDIV(TokenReader& reader);
		virtual PExprNode compileUNAR(TokenReader& reader);
		virtual PExprNode compileSubExpr(ExprLocation loc, TokenReader &reader);
		virtual PExprNode compileOpUnset(ExprLocation loc, TokenReader& reader);
		virtual PExprNode compileOpFirstDefined(ExprLocation loc, TokenReader &reader);
		virtual PExprNode compileUNARSuffix(TokenReader& reader);
		virtual PExprNode compileFunctOp(PExprNode nd, TokenReader& reader);
		virtual PExprNode compileArrayOp(PExprNode nd, TokenReader& reader);
		virtual PExprNode compileMemberAccessOp(PExprNode nd, TokenReader& reader);
		virtual PExprNode compileIF(ExprLocation loc, TokenReader& reader);
		virtual PExprNode compileOpTryCatch(ExprLocation loc, TokenReader& reader);
		virtual PExprNode compileOpWith(ExprLocation loc, TokenReader& reader);
		virtual PExprNode compileOpScope(ExprLocation loc, TokenReader& reader);
		virtual PExprNode compileOpObject(ExprLocation loc, TokenReader& reader);
		virtual PExprNode compileOpWhile(ExprLocation loc, TokenReader& reader);
		virtual PExprNode compileOpFunction(ExprLocation loc, TokenReader& reader);
		virtual void throwExpectedError(const ExprLocation &loc, ConstStringT<TokenReader::Symbol> symbols);
		virtual void throwUnexpectedError(const ExprLocation &loc, ConstStringT<TokenReader::Symbol> symbols);
		virtual PExprNode compileTemplateText(ExprLocation loc, TokenReader& reader);
		virtual PExprNode compileTemplateSubExpr(ExprLocation loc, TokenReader& reader);
		virtual PExprNode compileForEach(ExprLocation loc, TokenReader& reader);
		virtual PExprNode compileConst(ExprLocation loc, TokenReader& reader);
		virtual PExprNode compileTemplateCmd(ExprLocation loc, TokenReader& reader);
		virtual PExprNode compileInclude(ExprLocation loc, TokenReader &reader);

		static EscapeMode getCtFromMime(ConstStrA contentType);

		virtual std::pair<PExprNode,FilePath> loadCode(ExprLocation loc, ConstStrA name);
		PExprNode compileSetCommand(ExprLocation loc, TokenReader& reader);
	public:
		IRuntimeAlloc &alloc;
		JSON::PFactory valueFactory;
		EscapeMode curEscapeMode;
		AutoArray<char> buffer;

		VarTable globalConstContext;
		FakeGlobalScope constContext;
	};

	extern NamedEnum<EscapeMode> strEscapeMode;
}
