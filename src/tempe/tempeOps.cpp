#include <math.h>


#include "lightspeed/utils/json/jsonfast.tcc"
#include "lightspeed/base/containers/convertString.h"
#include "lightspeed/utils/base64.tcc"
#include "lightspeed/utils/urlencode.h"
#include "lightspeed/base/namedEnum.tcc"
#include "lightspeed/utils/json/jsonfast.h"
#include "basicOps.h"
#include "tempeOps.h"


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

	class DateTimeVal : public JSON::FloatField_t, public DynObject {
	public:
		DateTimeVal(double x) :JSON::FloatField_t(x) {}
		static bool IsDate(const Value &v) {
			return (typeid(*v.get()) == typeid(DateTimeVal));
		}

	};


	OutputResult::OutputResult(const ExprLocation &loc, POutputConfig cfg, EscapeMode escModeOverride)
		: NaryNode<1>(loc)
		, cfg(cfg)
		, escModeOverride(escModeOverride)
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
		typedef VtWriteIterator<AutoArray<char, SmallAlloc<256> >::WriteIter &> Wrt;
		Wrt wrt(iter);
		JSON::Serializer<IVtWriteIterator<char> > s(wrt,false);
		s.serializeString(ConstStrA(v));
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

	template<typename T, typename Q>
	static void formatToBufferIntegerPart(const T &param1, POutputConfig cfg, IWriteIterator<char,Q> &buffer, ConstStrA strnum)
	{
		natural len = strnum.length();

		if (param1 < 0) {
			buffer.blockWrite(cfg->negPrefix);
		}
		else if (param1 > 0){
			buffer.blockWrite(cfg->posPrefix);
		}
		natural gr = cfg->groupsize ? cfg->groupsize : naturalNull;

		natural spcnt = (len - 1) / gr;
		natural flen = len + spcnt;
		if (flen < cfg->minwidth) {
			for (natural i = cfg->minwidth; i > flen; i--) {
				buffer.blockWrite(cfg->fillchar);
			}
		}
		for (natural i = len; i > 0;) {
			i--;
			natural spcnt2 = i / gr;
			if (spcnt2 < spcnt) {
				buffer.blockWrite(cfg->delimiter);
				spcnt = spcnt2;
			}
			buffer.write(strnum[len - i - 1]);
		}	
	}

	template<typename T, typename Q>
	static void formatToBufferSuffixes(const T &param1, POutputConfig cfg, IWriteIterator<char, Q> &buffer)
	{
		
		if (param1 < 0) {
			buffer.blockWrite(cfg->negSuffix);
		}
		else if (param1 > 0){
			buffer.blockWrite(cfg->posSuffix);
		}

	}

	static StringA formatIntNumber(POutputConfig cfg, linteger param1){

		ToString<linteger, char> strNum(linteger(abs(param1)));

		AutoArrayStream<char, SmallAlloc<256> > buffer;
		formatToBufferIntegerPart(param1, cfg, buffer, strNum);
		formatToBufferSuffixes(param1, cfg, buffer);

		return buffer.getArray();
	}

	static StringA formatFloatNumber(POutputConfig cfg, double param1){
		if (cfg->precision == 0) param1 = floor(param1 + 0.5);
		ToString<double> strNum(abs(param1), cfg->precision);
		natural len = strNum.findLast('.');
		if (len == naturalNull) len = strNum.length();
		AutoArrayStream<char, SmallAlloc<256> > buffer;
		formatToBufferIntegerPart(param1, cfg, buffer, strNum.head(len));
		if (cfg->precision) {
			natural len2 = strNum.length();
			if (!cfg->fixed) {
				while (strNum[len2 - 1] == '0') len2--;
			}
			if (strNum[len2 - 1] != '.') {
				buffer.blockWrite(cfg->decimalmark,true);
				natural pos = len + 1;
				while (pos < len2) {
					buffer.write(strNum[pos]);
					pos++;
				}
			}
		}
		formatToBufferSuffixes(param1, cfg, buffer);
		return buffer.getArray();
	}


	StringA formatDate(const OutputConfig *cfg, double val)
	{
		CArray<char, 1024> buffer;
		TimeStamp tmsp(val);
		return tmsp.formatTime(buffer, cfg->dateFormat);
	}


	Value OutputResult::calculate(IExprEnvironment &env, const Value *subResults) const
	{
		IVtWriteIterator<char> *out = env.getTempeOutput();
		if (out) {
			Value v = subResults[0];
			v = convertLink(v);
			if (!v->isNull()) {

				if (escModeOverride == emJSON) {
					JSON::serialize(v, *out, false);
				}
				else {
					ConstStrA val;
					StringA tmp;
					if (DateTimeVal::IsDate(v)) {
						tmp = formatDate(cfg, v->getLongUInt());
						val = tmp;

					} else if (v->isIntNum()) {
						tmp = formatIntNumber(cfg,v->getLongInt());
						val = tmp;
					}
					else if (v->isFloatNum()) {
						tmp = formatFloatNumber(cfg, v->getFloat());
						val = tmp;
					}
					else if (v->isBool()) {
						if (v->getBool()) val = cfg->strTrue;
						else val = cfg->strFalse;
					}
					else if (v->isNull()) {
						val = cfg->strNull;
					}
					else {
						val = v->getStringUtf8();
					}

					StringA strout;

					switch (escModeOverride) {

					case emPlain: strout = val; break;
					case emXml:strout = escapeXml(val, false); break;
					case emHtml:  strout = escapeXml(val,true); break;
					case emJS: strout = escapeJs(val); break;
					case emC: strout = escapeC(val); break;
					case emURI: strout = escapeURI(val); break;
					case emBase64: strout = escapeBase64(val); break;
					case emHex: strout = escapeHex(val); break;
					default: break;
					}

					out->blockWrite(strout, true);
				}
			}
		}
		return  JSON::getNullNode();
	}

	enum OutputConfigFields {


		ocfContent,
		ocfMinWidth,
		ocfGroupSize,
		ocfPrecision,
		ocfFillChar,
		ocfDelimiter,
		ocfDecimalMark,
		ocfFixed,
		ocfStrTrue,
		ocfStrFalse,
		ocfStrNull,
		ocfPosPrefix,
		ocfPosSuffix,
		ocfNegPrefix,
		ocfNegSuffix,


	};

	static NamedEnumDef<OutputConfigFields> strConfigFieldsDef[] = {
		{ ocfContent, "content" },
		{ ocfMinWidth, "minWidth" },
		{ ocfGroupSize, "groupSize" },
		{ ocfPrecision, "precision"},
		{ ocfFillChar, "fillChar" },
		{ ocfDelimiter, "delimiter" },
		{ ocfDecimalMark, "decimalMark" },
		{ ocfFixed, "fixed" },
		{ ocfStrTrue, "true" },
		{ ocfStrFalse, "false" },
		{ ocfStrNull, "null" },
		{ ocfPosPrefix, "prefixPos" },
		{ ocfNegPrefix, "prefixNeg" },
		{ ocfPosSuffix, "suffixPos" },
		{ ocfNegSuffix, "suffixNeg" },


	};
	static NamedEnum<OutputConfigFields> strConfigFields(strConfigFieldsDef);

	OutputConfig::OutputConfig() 
		:escMode(emPlain)
		, minwidth(0)
		, groupsize(0)
		, precision(2)
		, fillchar(ConstStrA(' '))
		, delimiter(ConstStrA(' '))
		, decimalmark(ConstStrA('.'))
		, fixed(false)
		, posPrefix(ConstStrA())
		, posSuffix(ConstStrA())
		, negPrefix(ConstStrA('-'))
		, negSuffix(ConstStrA())
		, strTrue(JSON::strTrue)
		, strFalse(JSON::strFalse)
		, strNull(JSON::strNull)
	{

	}




	void OutputConfig::load(JSON::PNode cfg)
	{
		for (JSON::Iterator iter = cfg->getFwIter(); iter.hasItems();){
			const JSON::KeyValue &kv = iter.getNext();
			switch (strConfigFields[kv.getStringKey()]) {
				case ocfContent: escMode = strEscapeMode[kv->getStringUtf8()]; break;
				case ocfDecimalMark: decimalmark = kv->getStringUtf8(); break;
				case ocfDelimiter: delimiter = kv->getStringUtf8(); break;
				case ocfFillChar: fillchar = kv->getStringUtf8(); break;
				case ocfPosPrefix: posPrefix = kv->getStringUtf8(); break;
				case ocfNegPrefix: negPrefix = kv->getStringUtf8(); break;
				case ocfPosSuffix: posSuffix = kv->getStringUtf8(); break;
				case ocfNegSuffix: negSuffix = kv->getStringUtf8(); break;
				case ocfFixed: fixed = kv->getBool(); break;
				case ocfGroupSize: groupsize = Bin::natural16(kv->getUInt()); break;
				case ocfMinWidth: minwidth = Bin::natural16(kv->getUInt()); break;
				case ocfPrecision: precision = Bin::natural16(kv->getUInt()); break;
				case ocfStrFalse: strFalse = kv->getStringUtf8(); break;
				case ocfStrTrue: strTrue = kv->getStringUtf8(); break;
				case ocfStrNull: strNull = kv->getStringUtf8(); break;				
			}
		}
	}

	void OutputConfig::setContent(ConstStrA content)
	{
		escMode = strEscapeMode[content];
	}


	Tempe::Value fnUnixtime(IExprEnvironment &env, const Value &a)
	{
		return new(*env.getFactory().getAllocator()) DateTimeVal(a->getFloat() / (double)TimeStamp::daySecs);
	}

	Tempe::Value fnLsTime(IExprEnvironment &env, const Value &a)
	{
		return new(*env.getFactory().getAllocator()) DateTimeVal(a->getFloat());

	}

	Tempe::Value fnDbTime(IExprEnvironment &env, const Value &a)
	{
		ConstStrA tm = a->getStringUtf8();
		TimeStamp tms = TimeStamp::fromDBDate(tm);
		return new(*env.getFactory().getAllocator()) DateTimeVal(tms.getFloat());

	}

	Tempe::Value fnIsoTime(IExprEnvironment &env, const Value &a)
	{
		ConstStrA tm = a->getStringUtf8();
		TimeStamp tms = TimeStamp::fromISO8601Time(tm);
		return new(*env.getFactory().getAllocator()) DateTimeVal(tms.getFloat());

	}

	Tempe::Value fnDate(IExprEnvironment &env, const Value &a, const Value &b, const Value &c)
	{
		ConstStrA tm = a->getStringUtf8();
		TimeStamp tms = TimeStamp::fromYMDhms(a->getUInt,b->getUInt(),c->getUInt(),0,0,0);
		return new(*env.getFactory().getAllocator()) DateTimeVal(tms.getFloat());

	}

	Tempe::Value fnTime(IExprEnvironment &env, const Value &a, const Value &b, const Value &c)
	{
		ConstStrA tm = a->getStringUtf8();
		TimeStamp tms = TimeStamp::fromYMDhms(0,0,0,a->getUInt, b->getUInt(), c->getUInt());
		return new(*env.getFactory().getAllocator()) DateTimeVal(tms.getFloat());

	}

}

