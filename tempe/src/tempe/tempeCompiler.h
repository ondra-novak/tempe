/*
 * tempeCompiler.h
 *
 *  Created on: 28. 4. 2015
 *      Author: ondra
 */

#ifndef AEXPRESS_TEMPECOMPILER_H_
#define AEXPRESS_TEMPECOMPILER_H_

#include <lightspeed/base/namedEnum.h>
#include "compiler.h"

using LightSpeed::NamedEnum;

namespace Tempe {

class TempeCompiler: public Compiler {
public:

	enum ContentType {
		ctnPlain,
		ctnHtml,
		ctnXml,
		ctnJS,
		ctnC,
		ctnURI,
		ctnBase64,
		ctnHex,
		ctnVoid

	};

	TempeCompiler(ContentType defaultContentType):defaultContentType(defaultContentType) {}
	TempeCompiler() {}

	ContentType getDefaultContentType() const {
		return defaultContentType;
	}

	void setDefaultContentType(ContentType defaultContentType) {
		this->defaultContentType = defaultContentType;
	}

	static PExprNode compileTemplate(ConstStrA text, ContentType ctype);

	static NamedEnum<ContentType> strContentType;
	static NamedEnum<ContentType> strMimeCt;

	static ContentType getCtFromMime(ConstStrA mime);




protected:

	virtual PExprNode compileUNAR(ScanTextA& reader);

	virtual PExprNode compileTemplate(ScanTextA &reader);
	virtual PExprNode compileTemplateExpression(ScanTextA &reader, ContentType ctx, bool noEnding);
	virtual PExprNode compilePlaceholder(ScanTextA &reader, ContentType ctx);
	virtual PExprNode compileTextID(ScanTextA &reader, ContentType ctx, char firstChar);

	ContentType defaultContentType;

	PExprNode createStringNode(const ExprLocation &loc, const String &str);

};


} /* namespace Tempe */

#endif /* AEXPRESS_TEMPECOMPILER_H_ */
