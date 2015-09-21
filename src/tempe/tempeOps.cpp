#include "tempeOps.h"
#include "lightspeed/utils/json/jsonfast.tcc"
#include "lightspeed/base/containers/convertString.h"
#include "lightspeed/utils/base64.tcc"
#include "../../../lightspeed/src/lightspeed/utils/urlencode.h"
#include "basicOps.h"

namespace Tempe {

	OutputText::OutputText(const ExprLocation &loc, StringA text) :AbstractNode(loc), text(text)
	{

	}

	Value OutputText::calculate(IExprEnvironment &env) const
	{
		IVtWriteIterator<char> *out = env.getTempeOutput();
		if (out) {
			out->blockWrite(text, true);
		}
		return JSON::getNullNode();
	}

	OutputResult::OutputResult(const ExprLocation &loc, EscapeMode escMode) 
		: NaryNode<1>(loc)
		, escMode(escMode)
	{

	}

	static StringA escapeXml(StringA v, bool nlbr) {
		AutoArray<char, SmallAlloc<256> > buffer;
		for (StringA::Iterator iter = v.getFwIter(); iter.hasItems();) {
			char c = iter.getNext();
			switch (c) {
			case '<': buffer.append(ConstStrA("&lt;")); break;
			case '>': buffer.append(ConstStrA("&gt;")); break;
			case '"': buffer.append(ConstStrA("&quot;")); break;
			case '&': buffer.append(ConstStrA("&amp;")); break;
			case '\n': if (nlbr) buffer.append(ConstStrA("<br />")); else buffer.add(c); break;
			case '\r': if (!nlbr) buffer.add(c); break;
			default: buffer.add(c); break;
			}
		}
		return buffer;
	}

	static StringA escapeJs(StringA v) {
		AutoArray<char, SmallAlloc<256> > buffer;
		AutoArray<char, SmallAlloc<256> >::WriteIter iter = buffer.getWriteIterator();
		VtWriteIterator<AutoArray<char, SmallAlloc<256> >::WriteIter &> wrt(iter);
		JSON::serializeString(v, wrt);
		return buffer.crop(1, 1);
	}

	static StringA escapeC(StringA v) {
		//TODO: make it better
		return escapeJs(v);
	}

	static StringA escapeURI(StringA v) {
		StringA encoded = convertString(UrlEncoder(), ConstStrA(v));
		return encoded;
	}

	static StringA escapeBase64(StringA v) {
		StringA encoded = convertString(Base64EncoderT<char, char>(), ConstStrA(v));
		return encoded;
	}

	static StringA escapeHex(StringA v) {
		StringA encoded = convertString(HexEncoder<char, char>(), ConstStrA(v));
		return encoded;
	}



	Value OutputResult::calculate(IExprEnvironment &env, const Value *subResults) const
	{
		IVtWriteIterator<char> *out = env.getTempeOutput();
		if (out) {
			Value v = subResults[0];
			v = convertLink(v);
			if (!v->isNull()) {

				if (escMode == emJSON) {
					JSON::serialize(v, *out, false);
				}
				else {
					ConstStrA val = v->getStringUtf8();
					StringA strout;

					switch (escMode) {

					case emPlain: strout = val; break;
					case emXml:strout = escapeXml(val, false); break;
					case emHtml:  strout = escapeXml(val,true); break;
					case emJS: strout = escapeJs(val); break;
					case emURI: strout = escapeURI(val); break;
					default: break;
					}

					out->blockWrite(strout, true);
				}
			}
		}
		return  JSON::getNullNode();
	}

}