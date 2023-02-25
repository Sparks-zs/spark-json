#ifndef SPARK_JSON_H
#define SPARK_JSON_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace SparkJson
{

    enum JsonType{
        JSON_NULL = 0,
        JSON_BOOL,
        JSON_NUMBER,
        JSON_STRING,
        JSON_ARRAY,
        JSON_OBJECT
    };

    enum ParseCode{
        PARSE_OK = 0,
        PARSE_EXPECT_VALUE,
        PARSE_INVALID_VALUE,
        PARSE_ROOT_NOT_SINGULAR,
        PARSE_NUMBER_TOO_BIG,
        PARSE_MISS_QUOTATION_MARK,
        PARSE_INVALID_STRING_ESCAPE,
        PARSE_INVALID_STRING_CHAR,
        PARSE_INVALID_UNICODE_HEX,
        PARSE_INVALID_UNICODE_SURROGATE,
        PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
        PARSE_MISS_KEY,
        PARSE_MISS_COLON,
        PARSE_MISS_COMMA_OR_CURLY_BRACKET
    };

    class JsonValue;

    class Json final{
      public:

        typedef std::vector<Json> array;
        typedef std::map<std::string, Json> object;

        Json();     // null
        Json(nullptr_t);    // null
        Json(bool value);   // true/false
        Json(int value);    // number
        Json(double value); // number
        Json(const std::string& value);   // string
        Json(const char* value);    // string
        Json(const array& value);   // array
        Json(const object& value);  // object
        //Json(const Json& json);
        //Json(Json&& json);

        static Json parse(const std::string& str);

        static int getErrorCode() { return _error_code; }

        size_t size() const { return _value->size(); }
        JsonType type() const { return _value->type(); }
        
      private:
        std::shared_ptr<JsonValue> _value;
        static int _error_code;
    };

    class JsonValue{
    public:
        virtual size_t size() = 0;
        virtual JsonType type() const = 0;
        virtual const Json& operator[](size_t i) const;
        virtual const Json& operator[](const std::string& key) const;
    };

} // SparkJson

#endif // SPARK_JSON_H