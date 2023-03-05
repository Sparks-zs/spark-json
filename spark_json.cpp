#include "spark_json.hpp"
#include <assert.h>

using namespace std;

namespace SparkJson
{
    const Json& JsonValue::operator[](size_t i) const{
        return Json();
    }

    const Json& JsonValue::operator[](const string& key) const{
        return Json();
    }

    bool JsonValue::bool_value() const{
        return false;
    }
    
    int JsonValue::int_value() const{
        return 0;
    }
    
    double JsonValue::double_value() const{
        return 0.0;
    }
    
    const std::string& JsonValue::string_value() const{
        return "";
    }
    
    const Json::array& JsonValue::array_value() const{
        return {};
    }

    const Json::object& JsonValue::object_value() const{
        return {};
    }

    template<JsonType tag, typename T>
    class Value : public JsonValue{
      protected:
        explicit Value(const T& value) : _value(value){}
        explicit Value(T&& value) : _value(move(value)){}

        const size_t size() const override { return 1; }
        const JsonType type() const override { return tag; }
      
        const T _value;
    };

    struct Null{
        bool operator==(Null&) const { return true; }
        bool operator<(Null&) const { return false; }
    };
    
    class JsonNull : public Value<JSON_NULL, Null>{
      public:
        JsonNull() : Value({}) {}
    };

    class JsonBoolean : public Value<JSON_BOOL, bool>{
        bool bool_value() const override { return _value; }
      public:
        explicit JsonBoolean(bool value) : Value(value){}
    };

    class JsonInt : public Value<JSON_NUMBER, int>{
        int int_value() const override { return _value; }
        double double_value() const override { return _value; }
      public:
        explicit JsonInt(int value) : Value(value){}
    };

    class JsonDouble : public Value<JSON_NUMBER, double>{
        int int_value() const override { return static_cast<int>(_value); }
        double double_value() const override { return _value; }
      public:
        explicit JsonDouble(double value) : Value(value){}
    };

    class JsonString : public Value<JSON_STRING, string>{
        const string& string_value() const override { return _value; }
      public:
        explicit JsonString(const string& value) : Value(value){}
    };

    class JsonArray : public Value<JSON_ARRAY, Json::array>{
        const Json::array& array_value() const override { return _value; }
      public:
        explicit JsonArray(Json::array value) : Value(value){}
        const Json& operator[](size_t i) const override;
        const size_t size() const override{ return _value.size(); }
    };

    class JsonObject : public Value<JSON_OBJECT, Json::object>{
        const Json::object& object_value() const override { return _value; }
      public:
        explicit JsonObject(Json::object value) : Value(value){}
        const Json& operator[](const string& key) const override;
        const size_t size() const override{ return _value.size(); }
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
    /*Json::Json(const Json& json) : _value(json._value), _errorCode(json._errorCode){
        if(json.type() == JsonType::JSON_STRING){
            std::cout << json.to_string() << std::endl;
        }
    }*/

    size_t Json::size() const{
        return _value->size();
    }

    JsonType Json::type() const{
        return _value->type();
    }

    const Json& JsonArray::operator[](size_t i) const{
        if(i >= _value.size()) return Json();
        return _value[i];
    }

    const Json& JsonObject::operator[](const string& key) const{
        if(_value.count(key) == 0) return Json();
        return _value.find(key)->second;
    }

    bool Json::to_bool() const{
        return _value->bool_value();
    }
    
    int Json::to_int() const{
        return _value->int_value();
    }
    
    double Json::to_double() const{
        return _value->double_value();
    }
    
    const std::string& Json::to_string() const{
        return _value->string_value();
    }
    
    const Json::array& Json::to_array() const{
        return _value->array_value();
    }

    const Json::object& Json::to_object() const{
        return _value->object_value();
    }

    const Json& Json::operator[](size_t i) const{
        return (*_value)[i];
    }

    const Json& Json::operator[](const std::string& key) const{
        return (*_value)[key];
    }
    // parser

    class Parser{
      public:
        Parser(const string& value) 
            : _pos(0),
              _value(value),
              _code(PARSE_EXPECT_VALUE){}

        Json parse(){
            parseWhitespace();
            Json json(parseValue());
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
            
            while(_value[_pos] == ' ' || _value[_pos] == '\t' || _value[_pos] == '\r' || _value[_pos] == '\n'){
                    _pos++;
            }
        }

        Json parseLiteral(const string& expect, Json expect_json){
            size_t len = expect.length();
            assert(len > 0);
            bool ret = _value.substr(_pos, len).compare(expect) == 0;
            _pos += len;
            if(!ret){
               _code = PARSE_INVALID_VALUE;
                return Json();
            }
            _code = PARSE_OK;
            return expect_json;
        }

        Json parseNumber(){
            size_t start_pos = _pos;
            if(_value[_pos] == '-')
                _pos++;
            if(_value[_pos] == '0' && isdigit(_value[_pos + 1])){
                _code = PARSE_INVALID_VALUE;
                return Json(0);
            }

            for(; isdigit(_value[_pos]); _pos++);

            if(_value[_pos] == '.'){
                _pos++;
                if(!isdigit(_value[_pos])){
                    _code = PARSE_INVALID_VALUE;
                    return Json(0);
                }
                else for(; isdigit(_value[_pos]); _pos++);
            } 

            if(_value[_pos] == 'e' || _value[_pos] == 'E'){
                _pos++;
                if(_value[_pos] == '+' || _value[_pos] == '-')  _pos++;
                if(!isdigit(_value[_pos])){
                    _code = PARSE_INVALID_VALUE;
                    return Json(0);
                }
                for(; isdigit(_value[_pos]); _pos++);
            }

            double n;
            try{
                n = stod(_value.substr(start_pos, _pos));
            }
            catch(const std::out_of_range& oor){
                _code = PARSE_NUMBER_TOO_BIG;
                return Json(0);
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
            assert(_value[_pos++] == '\"');
            string out;
            unsigned u, u2;

            for(;;){
                char ch = _value[_pos++];
                switch(ch){
                    case '\"':
                        _code = PARSE_OK;
                        //out += '\0';
                        return out;
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
                                if(!parseHex4(&u)){
                                    _code = PARSE_INVALID_UNICODE_HEX;
                                    return "";
                                }
                                if(u >= 0xD800 && u <= 0xDBFF){
                                    if(_value[_pos++] != '\\'){
                                        _code = PARSE_INVALID_UNICODE_SURROGATE;
                                        return "";
                                    }
                                    if(_value[_pos++] != 'u'){
                                        _code = PARSE_INVALID_UNICODE_SURROGATE;
                                        return "";
                                    }
                                    if(!parseHex4(&u2)){
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
                        break;
                    case '\0':
                        _code = PARSE_MISS_QUOTATION_MARK;
                        return "";
                    default:
                        if(static_cast<unsigned char>(ch) < 0x20){
                            _code = PARSE_INVALID_STRING_CHAR;
                            return "";
                        }
                        out += ch;
                }
            }
        }

        Json parseObject(){
            assert(_value[_pos++] == '{');
            map<string, Json> out;
            parseWhitespace();
            if(_value[_pos] == '}'){
                _pos++;
                _code = PARSE_OK;
                return out;
            }

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
                if(_value[_pos] == '}'){
                    _pos++;
                    break;
                }
                else if(_value[_pos] == ','){
                    _pos++;
                    parseWhitespace();
                }
                else{
                    _code = PARSE_MISS_COMMA_OR_CURLY_BRACKET;
                    return Json();
                }
            }
            return out;
        }

        Json parseArray(){
            assert(_value[_pos++] == '[');
            vector<Json> out;
            parseWhitespace();
            if(_value[_pos] == ']'){
                _pos++;
                _code = PARSE_OK;
                return out;
            }
            for(;;){
                Json v = parseValue();
                if(_code != PARSE_OK){
                    return Json();
                }
                out.push_back(v);

                parseWhitespace();
                if(_value[_pos] == ']'){
                    _pos++;
                    break;
                }
                else if(_value[_pos] == ','){
                    _pos++;
                    parseWhitespace();
                }
                else{
                    _code = PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
                    return Json();
                }
            }
            return out;
        }

        Json parseValue(){
            switch(_value[_pos]){
                case 'n': return parseLiteral("null", nullptr);
                case 't': return parseLiteral("true", true);
                case 'f': return parseLiteral("false", false);
                case '"': return parseString();
                case '{': return parseObject();
                case '[': return parseArray();
                case '\0': return Json();
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
        json.setErrorCode(parser.getCode());
        return json;
    }

} // SparkJson