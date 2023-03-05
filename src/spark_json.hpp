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
        Json(array&& value);        // array
        Json(const object& value);  // object
        Json(object&& value);       // object
        //Json(const Json& json);
        //Json(Json&& json);

        static Json parse(const std::string& str);
        void dump(std::string& out) const;
        const std::string dump() const{
            std::string out;
            dump(out);
            return out;
        }

        void setErrorCode(int code) { _errorCode = code; }
        int getErrorCode() const { return _errorCode; }

        size_t size() const;
        JsonType type() const;

        // 访问内部原始数据
        bool to_bool() const;
        int to_int() const;
        double to_double() const;
        const std::string& to_string() const;
        const array& to_array() const;
        const object& to_object() const;

        const Json& operator[](size_t i) const;
        const Json& operator[](const std::string& key) const;

      private:
        std::shared_ptr<JsonValue> _value;
        int _errorCode = 0;
    };

    class JsonValue{
      protected:
        friend class Json;
        virtual const size_t size() const = 0;
        virtual const JsonType type() const = 0;
        virtual void dump(std::string& out) const = 0;
        virtual bool bool_value() const;
        virtual int int_value() const;
        virtual double double_value() const;
        virtual const std::string& string_value() const;
        virtual const Json::array& array_value() const;
        virtual const Json::object& object_value() const;
        virtual const Json& operator[](size_t i) const;
        virtual const Json& operator[](const std::string& key) const;
    };

} // SparkJson

#endif // SPARK_JSON_H