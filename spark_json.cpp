#include "spark_json.hpp"
#include <assert.h>
#include <errno.h>

using namespace std;

namespace SparkJson
{
    int Json::_error_code = PARSE_OK;

    const Json& JsonValue::operator[](size_t i) const{
        return Json();
    }

    const Json& JsonValue::operator[](const string& key) const{
        return Json();
    }

    template<JsonType tag, typename T>
    class Value : public JsonValue{
      public:
        explicit Value(T value) : _value(value){}

        JsonType type() const { return tag; }
      protected:
        T _value;
    };

    struct Null{
        bool operator==(Null&) const { return true; }
        bool operator<(Null&) const { return false; }
    };
    
    class JsonNull : public Value<JSON_NULL, Null>{
      public:
        JsonNull() : Value({}) {}
        size_t size() override{ return 1; }
    };

    class JsonBoolean : public Value<JSON_BOOL, bool>{
      public:
        explicit JsonBoolean(bool value) : Value(value){}
        size_t size() override{ return 1; }
    };

    class JsonInt : public Value<JSON_NUMBER, int>{
      public:
        explicit JsonInt(int value) : Value(value){}
        size_t size() override{ return 1; }
    };

    class JsonDouble : public Value<JSON_NUMBER, double>{
      public:
        explicit JsonDouble(double value) : Value(value){}
        size_t size() override{ return 1; }
    };

    class JsonString : public Value<JSON_STRING, const string&>{
      public:
        explicit JsonString(const string& value) : Value(value){}
        size_t size() override{ return 1; }
    };

    class JsonArray : public Value<JSON_ARRAY, Json::array>{
      public:
        explicit JsonArray(Json::array value) : Value(value){}
        const Json& operator[](size_t i) const override;
        size_t size() override{ return _value.size(); }
    };

    class JsonObject : public Value<JSON_OBJECT, Json::object>{
      public:
        explicit JsonObject(Json::object value) : Value(value){}
        const Json& operator[](const string& key) const override;
        size_t size() override{ return _value.size(); }
    };

    Json::Json() : _value(make_shared<JsonNull>()){}
    Json::Json(nullptr_t) : _value(make_shared<JsonNull>()){}
    Json::Json(bool value) : _value(make_shared<JsonBoolean>(value)){}
    Json::Json(int value) : _value(make_shared<JsonInt>(value)){}
    Json::Json(double value) : _value(make_shared<JsonDouble>(value)){}
    Json::Json(const string& value) : _value(make_shared<JsonString>(value)){}
    Json::Json(const char* value) : _value(make_shared<JsonString>(value)){}
    Json::Json(const Json::array& value) : _value(make_shared<JsonArray>(value)){}
    Json::Json(const Json::object& value) : _value(make_shared<JsonObject>(value)){}

    const Json& JsonArray::operator[](size_t i) const{
        if(i >= _value.size()) return Json();
        return _value[i];
    }

    const Json& JsonObject::operator[](const string& key) const{
        if(_value.count(key) == 0) return Json();
        return _value.find(key)->second;
    }
    // parser

    class Parser{
      public:
        Parser(const string& value) 
            : _pos(0),
              _value(value),
              _code(PARSE_EXPECT_VALUE){}

        Json parse(){
            Json json;
            parseWhitespace();
            json = parseValue();
            if(_code == PARSE_OK){
                parseWhitespace();
                if(_value.length() > _pos){
                    _code = PARSE_ROOT_NOT_SINGULAR;
                    return Json();
                }
            }
            return json;
        }
        
        void parseWhitespace(){
            if(_pos >= _value.length())
                return;
            
            for(auto v : _value){
                if(v == ' ' || v == '\t' || v == '\r' || v == '\n'){
                    _pos++;
                }
            }
        }

        Json parseLiteral(const string& expect, Json expect_json){
            size_t len = expect.length();
            assert(len > 0);
            bool ret = _value.substr(_pos, len).compare(expect);
            _pos += len;
            if(!ret){
               _code = PARSE_INVALID_VALUE;
                return Json();
            }
            _code = PARSE_OK;
            return expect_json;
        }

        Json parseNumber(){
            if(_value[_pos] == '-')
                _pos++;
            if(_value[_pos] == '0' && isdigit(_value[_pos + 1])){
                _code = PARSE_INVALID_VALUE;
                return Json();
            }

            for(; isdigit(_value[_pos]); _pos++);

            if(_value[_pos] == '.' && !isdigit(_value[_pos + 1])){
                _code = PARSE_INVALID_VALUE;
                return Json();
            }
            else{
                _pos++;
                for(; isdigit(_value[_pos]); _pos++);
            }

            if(_value[_pos] == 'e' || _value[_pos] == 'E'){
                _pos++;
                if(_value[_pos] == '+' || _value[_pos] == '-')  _pos++;
                if(!isdigit(_value[_pos])){
                    _code = PARSE_INVALID_VALUE;
                    return Json();
                }
                for(; isdigit(_value[_pos]); _pos++);
            }

            double n = stod(_value);
            if (errno == ERANGE){
                _code = PARSE_NUMBER_TOO_BIG;
                return Json();
            }
            _code = PARSE_OK;
            return Json(n);
        }

        bool parseHex4(unsigned* u){
            *u = 0;
            for(int i = 0; i < 4; i++){
                *u <<= 4;
                char ch = _value[_pos++];
                if      (ch >= '0' && ch <= '9')  *u |= ch - '0';
                else if (ch >= 'A' && ch <= 'F')  *u |= ch - ('A' - 10);
                else if (ch >= 'a' && ch <= 'f')  *u |= ch - ('a' - 10);
                else return false;
            }
            return true;
        }

        void encodeUtf8(unsigned u, string& out){
            if(u <= 0x7F){
                out += u & 0xFF;
            }
            else if(u <= 0x7FF){
                out += 0xC0 | ((u >> 6) & 0xFF);
                out += 0x80 | ((u       & 0x3F));
            }
            else if(u <= 0xFFFF){
                out += 0xE0 | ((u >> 12) & 0xFF);
                out += 0x80 | ((u >> 6 ) & 0x3F);
                out += 0x80 | ( u        & 0x3F);
            }
            else{
                assert(u <= 0x10FFFF);
                out += 0xF0 | ((u >> 18) & 0xFF);
                out += 0x80 | ((u >> 12) & 0x3F);
                out += 0x80 | ((u >>  6) & 0x3F);
                out += 0x80 | ((u      ) & 0x3F);
            }
        }

        string parseString(){
            assert(_value[_pos] == '\"');
            string out;
            unsigned u, u2;

            for(;;){
                switch(_value[_pos++]){
                    case '\"':
                        break;
                    case '\\':
                        switch(_value[_pos++]){
                            case '\"': out += '\"'; break; 
                            case '\\': out += '\\'; break;
                            case '/':  out += '/'; break;
                            case 'b':  out += '\b'; break;
                            case 'f':  out += '\f'; break;
                            case 'n':  out += '\n'; break;
                            case 'r':  out += '\r'; break;
                            case 't':  out += '\t'; break;
                            case 'u':
                                if(parseHex4(&u)){
                                    _code = PARSE_INVALID_UNICODE_HEX;
                                    return "";
                                }
                                if(u >= 0xD800 && u <= 0xD8FF){
                                    if(_value[_pos++] != '\\'){
                                        _code = PARSE_INVALID_UNICODE_SURROGATE;
                                        return "";
                                    }
                                    if(_value[_pos++] != 'u'){
                                        _code = PARSE_INVALID_UNICODE_SURROGATE;
                                        return "";
                                    }
                                    if(parseHex4(&u2)){
                                        _code = PARSE_INVALID_UNICODE_HEX;
                                        return "";
                                    }
                                    if(u2 < 0xDC00 || u2 > 0xDFFF){
                                        _code = PARSE_INVALID_UNICODE_SURROGATE;
                                        return "";
                                    }
                                    u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                                }
                                encodeUtf8(u, out);
                                break;
                            default:
                                _code = PARSE_INVALID_STRING_ESCAPE;
                                return "";
                        }
                    case '\0':
                        _code = PARSE_MISS_QUOTATION_MARK;
                        return "";
                    default:
                        if(static_cast<unsigned char>(_value[_pos]) < 0x20){
                            _code = PARSE_INVALID_STRING_CHAR;
                            return "";
                        }
                        out += _value[_pos];
                }
            }
            _code = PARSE_OK;
            return out;
        }

        Json parseObject(){
            assert(_value[_pos] == '{');
            map<string, Json> out;
            parseWhitespace();

            for(;;){
                string key = parseString();
                if(_code != PARSE_OK){
                    return Json();
                }

                parseWhitespace();
                if(_value[_pos++] != ':'){
                    _code = PARSE_MISS_COLON;
                    return Json();
                }

                parseWhitespace();
                Json v = parseValue();
                if(_code != PARSE_OK){
                    return Json();
                }
                out[key] = v;

                parseWhitespace();
                if(_value[_pos++] == '}'){
                    break;
                }
                else if(_value[_pos++] == ','){
                    parseWhitespace();
                }
                else{
                    _code = PARSE_MISS_COMMA_OR_CURLY_BRACKET;
                    return Json();
                }
            }
        }

        Json parseArray(){
            assert(_value[_pos++] == '[');
            vector<Json> out;
            parseWhitespace();
            
            for(;;){
                Json v = parseValue;
                if(_code != PARSE_OK){
                    return Json();
                }
                out.push_back(v);

                parseWhitespace();
                if(_value[_pos++] == ']'){
                    break;
                }
                else if(_value[_pos++] == ','){
                    parseWhitespace();
                }
                else{
                    _code = PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
                    return Json();
                }
            }

            return Json(out);
        }

        Json parseValue(){
            switch(_value[_pos]){
                case 'n': return parseLiteral("null", nullptr);
                case 't': return parseLiteral("true", true);
                case 'f': return parseLiteral("false", false);
                case '"': return parseString();
                case '{': return parseObject();
                case '[': return parseArray();
                case '\0': return PARSE_EXPECT_VALUE;
                default:
                    return parseNumber();
            }
        }

        ParseCode getCode() const { return _code; }
      private:
        size_t _pos;
        string _value;
        ParseCode _code;
    };

    Json Json::parse(const string& str){
        Parser parser(str);
        Json json = parser.parse(); 
        _error_code = parser.getCode();
        return json;
    }

} // SparkJson