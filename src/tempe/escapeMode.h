#pragma  once
#include "lightspeed/base/namedEnum.h"

namespace Tempe {


	enum EscapeMode {
		///Output is plain text - no escaping applied - results are written as they are
		emPlain,
		///Output is HTML - escape HTML special chars : < > & ' " etc
		emHtml,
		///Output is XML - escape XML special chars : < > & ' " etc
		emXml,
		///Output is Javascript - escape quotes: " -> \"
		/** In compare to emJSON, this allows to build Javascript strings, because escape mode
		doesn't add beginning and ending quotes */
		emJS,
		///Output is JSON - result is converted to JSON string complete 
		/** In compare to emJS, this escape mode outputs pure JSON, so if string is outputted,
		beginning and ending quotes are added. You can also output whole objects and arrays */
		emJSON,
		///Output is C source code
		emC,
		///Output is URI - apply encodeURIComponent, escape URI dangerous characters with %XX
		emURI,
		///Output is quote printable text -- escape characters with =XX
		emQuotePrintable,
		///Output will be rendered in BASE64
		emBase64,
		///Output will be rendered in HEX
		emHex,
		///No output
		emVoid

	};

	extern NamedEnum<EscapeMode> strEscapeMode;
	extern NamedEnum<EscapeMode> strMimeCt;

	class OutputConfig;
	typedef RefCntPtr<OutputConfig> POutputConfig;

}