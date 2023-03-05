#include "spark_json.hpp"
#include <cstring>
using namespace SparkJson;

static int test_count = 0;
static int test_pass = 0;
static int main_ret = 0; 

#define EXPECT_EQ_BASE(equality, expect, actual) \
    do {\
        test_count++; \
        if(equality) \
            test_pass++; \
        else{ \
            std::cout << __FILE__ << ":" << __LINE__ << " expect: " << expect << " actual: " << actual << std::endl; \
            main_ret = 1;\
        }\
    }while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect == actual), expect, actual)
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect == actual), expect, actual)
#define EXPECT_EQ_STRING(expect, actual) \
    EXPECT_EQ_BASE(expect.compare(actual) == 0, expect, actual)
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true")
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual)

#define TEST_PARSE_JSON(json, json_type, code) \
    do{ \
        EXPECT_EQ_INT(json.getErrorCode(), code); \
        EXPECT_EQ_INT(json.type(), json_type); \
    }while(0)

#define TEST_NULL(expect, code) \
    do{ \
        Json json = Json::parse(expect); \
        TEST_PARSE_JSON(json, JsonType::JSON_NULL, code); \
    }while(0)

#define TEST_TRUE(expect, code) \
    do{\
        Json json = Json::parse(expect); \
        TEST_PARSE_JSON(json, JsonType::JSON_BOOL, code); \
        EXPECT_TRUE(json.to_bool()); \
    }while(0)

#define TEST_FALSE(expect, code) \
    do{\
        Json json = Json::parse(expect); \
        TEST_PARSE_JSON(json, JsonType::JSON_BOOL, code); \
        EXPECT_FALSE(json.to_bool()); \
    }while(0) 

#define TEST_INT(expect, actual, code) \
    do{\
        Json json = Json::parse(expect); \
        TEST_PARSE_JSON(json, JsonType::JSON_NUMBER, code); \
        EXPECT_EQ_INT(json.to_int(), actual); \
    }while(0)

#define TEST_DOUBLE(expect, actual, code) \
    do{\
        Json json = Json::parse(expect); \
        TEST_PARSE_JSON(json, JsonType::JSON_NUMBER, code); \
        EXPECT_EQ_DOUBLE(json.to_double(), actual); \
    }while(0)

#define TEST_STRING(expect, actual, code) \
    do{\
        Json json = Json::parse(expect); \
        TEST_PARSE_JSON(json, JsonType::JSON_STRING, code); \
        EXPECT_EQ_STRING(json.to_string(), actual); \
    }while(0)

#define TEST_ERROR(expect, code) \
    do{ \
        Json json = Json::parse(expect); \
        EXPECT_EQ_INT(json.getErrorCode(), code); \
    }while(0)

void test_construct(){
    // null
    EXPECT_EQ_INT(Json().type(), JsonType::JSON_NULL);
    EXPECT_EQ_INT(Json(nullptr).type(), JsonType::JSON_NULL);

    // bool
    EXPECT_EQ_INT(Json(true).type(), JsonType::JSON_BOOL);
    EXPECT_EQ_INT(Json(false).type(), JsonType::JSON_BOOL);

    // number
    EXPECT_EQ_INT(Json(1.0).type(), JsonType::JSON_NUMBER);
    EXPECT_EQ_INT(Json(1).type(), JsonType::JSON_NUMBER);
    EXPECT_EQ_INT(Json(1e3).type(), JsonType::JSON_NUMBER);
    EXPECT_EQ_INT(Json(0.01).type(), JsonType::JSON_NUMBER);

    // string
    EXPECT_EQ_INT(Json("string").type(), JsonType::JSON_STRING);
    EXPECT_EQ_INT(Json(" ").type(), JsonType::JSON_STRING);
    EXPECT_EQ_INT(Json(" string ").type(), JsonType::JSON_STRING);
}

void test_parse(){
    // null
    TEST_NULL("  ", ParseCode::PARSE_EXPECT_VALUE);
    TEST_NULL("null", ParseCode::PARSE_OK);
    TEST_NULL(" null ", ParseCode::PARSE_OK);

    // bool
    TEST_TRUE(" true ", ParseCode::PARSE_OK);
    TEST_FALSE(" false ", ParseCode::PARSE_OK);
    
    // number
    TEST_DOUBLE("0", 0.0, ParseCode::PARSE_OK);
    TEST_DOUBLE("-0", 0.0, ParseCode::PARSE_OK);
    TEST_DOUBLE("-0.0", 0.0, ParseCode::PARSE_OK);
    TEST_DOUBLE("1", 1.0, ParseCode::PARSE_OK);
    TEST_DOUBLE("-1", -1.0, ParseCode::PARSE_OK);
    TEST_DOUBLE("1.5", 1.5, ParseCode::PARSE_OK);
    TEST_DOUBLE("-1.5", -1.5, ParseCode::PARSE_OK);
    TEST_DOUBLE("3.1416", 3.1416, ParseCode::PARSE_OK);
    TEST_DOUBLE("1E10", 1E10, ParseCode::PARSE_OK);
    TEST_DOUBLE("1e10", 1e10, ParseCode::PARSE_OK);
    TEST_DOUBLE("1E+10", 1E+10, ParseCode::PARSE_OK);
    TEST_DOUBLE("1E-10", 1E-10, ParseCode::PARSE_OK);
    TEST_DOUBLE("-1E10", -1E10, ParseCode::PARSE_OK);
    TEST_DOUBLE("-1e10", -1e10, ParseCode::PARSE_OK);
    TEST_DOUBLE("-1E+10", -1E+10, ParseCode::PARSE_OK);
    TEST_DOUBLE("-1E-10", -1E-10, ParseCode::PARSE_OK);
    TEST_DOUBLE("1.234E+10", 1.234E+10, ParseCode::PARSE_OK);
    TEST_DOUBLE("1.234E-10", 1.234E-10, ParseCode::PARSE_OK);
    TEST_DOUBLE("1e-10000", 0.0, ParseCode::PARSE_NUMBER_TOO_BIG); /* must underflow */
    TEST_DOUBLE("1e309", 0, ParseCode::PARSE_NUMBER_TOO_BIG);
    TEST_DOUBLE("-1e309", 0, ParseCode::PARSE_NUMBER_TOO_BIG);
    TEST_DOUBLE("1.0000000000000002", 1.0000000000000002, ParseCode::PARSE_OK); /* the smallest number > 1 */
    //TEST_DOUBLE("4.9406564584124654e-324", 4.9406564584124654e-324, ParseCode::PARSE_OK); /* minimum denormal */
    //TEST_DOUBLE("-4.9406564584124654e-324", -4.9406564584124654e-324, ParseCode::PARSE_OK);
    //TEST_DOUBLE("2.2250738585072009e-308", 2.2250738585072009e-308, ParseCode::PARSE_OK);  /* Max subnormal double */
    //TEST_DOUBLE("-2.2250738585072009e-308", -2.2250738585072009e-308, ParseCode::PARSE_OK);
    //TEST_DOUBLE("2.2250738585072014e-308", 2.2250738585072014e-308, ParseCode::PARSE_OK);  /* Min normal positive double */
    //TEST_DOUBLE("-2.2250738585072014e-308", -2.2250738585072014e-308, ParseCode::PARSE_OK);
    //TEST_DOUBLE("1.7976931348623157e+308", 1.7976931348623157e+308, ParseCode::PARSE_OK);  /* Max double */
    //TEST_DOUBLE("-1.7976931348623157e+308", -1.7976931348623157e+308, ParseCode::PARSE_OK);

    // string
    TEST_STRING("\" \"", " ", ParseCode::PARSE_OK);
    TEST_STRING("\" hello\\nworld \"", " hello\nworld ", ParseCode::PARSE_OK);
    TEST_STRING("\"\\u0024\"", "\x24", ParseCode::PARSE_OK);         /* Dollar sign U+0024 */
    TEST_STRING("\"\\u00A2\"", "\xC2\xA2", ParseCode::PARSE_OK);     /* Cents sign U+00A2 */
    TEST_STRING("\"\\u20AC\"", "\xE2\x82\xAC", ParseCode::PARSE_OK); /* Euro sign U+20AC */
    TEST_STRING("\"\\uD834\\uDD1E\"", "\xF0\x9D\x84\x9E", ParseCode::PARSE_OK);  /* G clef sign U+1D11E */
    TEST_STRING("\"\\ud834\\udd1e\"", "\xF0\x9D\x84\x9E", ParseCode::PARSE_OK);  /* G clef sign U+1D11E */
}

void test_parse_invalid(){
    TEST_ERROR("\"", ParseCode::PARSE_MISS_QUOTATION_MARK);
    TEST_ERROR("\"abc", ParseCode::PARSE_MISS_QUOTATION_MARK);
    TEST_ERROR("\"\\v\"", ParseCode::PARSE_INVALID_STRING_ESCAPE);
    TEST_ERROR("\"\\'\"", ParseCode::PARSE_INVALID_STRING_ESCAPE);
    TEST_ERROR("\"\\0\"", ParseCode::PARSE_INVALID_STRING_ESCAPE);
    TEST_ERROR("\"\\x12\"", ParseCode::PARSE_INVALID_STRING_ESCAPE);
    TEST_ERROR("\"\x01\"", ParseCode::PARSE_INVALID_STRING_CHAR);
    TEST_ERROR("\"\x1F\"", ParseCode::PARSE_INVALID_STRING_CHAR);

    TEST_ERROR("\"\\u\"", ParseCode::PARSE_INVALID_UNICODE_HEX);
    TEST_ERROR("\"\\u0\"", ParseCode::PARSE_INVALID_UNICODE_HEX);
    TEST_ERROR("\"\\u01\"", ParseCode::PARSE_INVALID_UNICODE_HEX);
    TEST_ERROR("\"\\u012\"", ParseCode::PARSE_INVALID_UNICODE_HEX);
    TEST_ERROR("\"\\u/000\"", ParseCode::PARSE_INVALID_UNICODE_HEX);
    TEST_ERROR("\"\\uG000\"", ParseCode::PARSE_INVALID_UNICODE_HEX);
    TEST_ERROR("\"\\u0/00\"", ParseCode::PARSE_INVALID_UNICODE_HEX);
    TEST_ERROR("\"\\u0G00\"", ParseCode::PARSE_INVALID_UNICODE_HEX);
    TEST_ERROR("\"\\u00/0\"", ParseCode::PARSE_INVALID_UNICODE_HEX);
    TEST_ERROR("\"\\u00G0\"", ParseCode::PARSE_INVALID_UNICODE_HEX);
    TEST_ERROR("\"\\u000/\"", ParseCode::PARSE_INVALID_UNICODE_HEX);
    TEST_ERROR("\"\\u000G\"", ParseCode::PARSE_INVALID_UNICODE_HEX);
    TEST_ERROR("\"\\u 123\"", ParseCode::PARSE_INVALID_UNICODE_HEX);

    TEST_ERROR("\"\\uD800\"", ParseCode::PARSE_INVALID_UNICODE_SURROGATE);
    TEST_ERROR("\"\\uDBFF\"", ParseCode::PARSE_INVALID_UNICODE_SURROGATE);
    TEST_ERROR("\"\\uD800\\\\\"", ParseCode::PARSE_INVALID_UNICODE_SURROGATE);
    TEST_ERROR("\"\\uD800\\uDBFF\"", ParseCode::PARSE_INVALID_UNICODE_SURROGATE);
    TEST_ERROR("\"\\uD800\\uE000\"", ParseCode::PARSE_INVALID_UNICODE_SURROGATE);
}

void test_array(){
    Json json = Json::parse("[ ]");
    EXPECT_EQ_INT(json.getErrorCode(), ParseCode::PARSE_OK);
    EXPECT_EQ_INT(json.type(), JsonType::JSON_ARRAY);
    EXPECT_EQ_INT(json.size(), 0);

    json = Json::parse("[ null , false , true , 123 , \"abc\" ]");
    EXPECT_EQ_INT(json.getErrorCode(), ParseCode::PARSE_OK);
    EXPECT_EQ_INT(json.type(), JsonType::JSON_ARRAY);
    EXPECT_EQ_INT(json.size(), 5);
    TEST_PARSE_JSON(json[0], JsonType::JSON_NULL, ParseCode::PARSE_OK);
    TEST_PARSE_JSON(json[1], JsonType::JSON_BOOL, ParseCode::PARSE_OK);
    EXPECT_FALSE(json[1].to_bool());
    TEST_PARSE_JSON(json[2], JsonType::JSON_BOOL, ParseCode::PARSE_OK);
    EXPECT_TRUE(json[2].to_bool());
    TEST_PARSE_JSON(json[3], JsonType::JSON_NUMBER, ParseCode::PARSE_OK);
    EXPECT_EQ_INT(json[3].to_int(), 123);
    TEST_PARSE_JSON(json[4], JsonType::JSON_STRING, ParseCode::PARSE_OK);
    EXPECT_EQ_STRING(json[4].to_string(), "abc");

    json = Json::parse("[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]");
    EXPECT_EQ_INT(json.getErrorCode(), ParseCode::PARSE_OK);
    EXPECT_EQ_INT(json.type(), JsonType::JSON_ARRAY);
    EXPECT_EQ_INT(json.size(), 4);
    for(int i=0; i<4; i++){
        TEST_PARSE_JSON(json[i], JsonType::JSON_ARRAY, ParseCode::PARSE_OK);
        EXPECT_EQ_INT(json[i].size(), i);
        for(int j=0; j<i; j++){
            TEST_PARSE_JSON(json[i][j], JsonType::JSON_NUMBER, ParseCode::PARSE_OK);
            EXPECT_EQ_INT(json[i][j].to_int(), j);
        }
    }

    json = Json::object {
        {"key1", 15.7},
        {"key2", Json::object {{ "key3", "string"}} },
        {"key4", Json::array { 0, 1, 2, 3}}
    };

    std::cout << json.dump() << std::endl;
}

void test_object(){
    Json json = Json::parse("{ }");
    EXPECT_EQ_INT(json.getErrorCode(), ParseCode::PARSE_OK);
    EXPECT_EQ_INT(json.type(), JsonType::JSON_OBJECT);
    EXPECT_EQ_INT(json.size(), 0);

    json = Json::parse("{ \"key1\" : null , \"key2\": false , \"key3\": true , \"key4\": 123 , \"key5\": \"abc\" }");
    EXPECT_EQ_INT(json.getErrorCode(), ParseCode::PARSE_OK);
    EXPECT_EQ_INT(json.type(), JsonType::JSON_OBJECT);
    EXPECT_EQ_INT(json.size(), 5);
    TEST_PARSE_JSON(json["key1"], JsonType::JSON_NULL, ParseCode::PARSE_OK);
    TEST_PARSE_JSON(json["key2"], JsonType::JSON_BOOL, ParseCode::PARSE_OK);
    EXPECT_FALSE(json["key2"].to_bool());
    TEST_PARSE_JSON(json["key3"], JsonType::JSON_BOOL, ParseCode::PARSE_OK);
    EXPECT_TRUE(json["key3"].to_bool());
    TEST_PARSE_JSON(json["key4"], JsonType::JSON_NUMBER, ParseCode::PARSE_OK);
    EXPECT_EQ_INT(json["key4"].to_int(), 123);
    TEST_PARSE_JSON(json["key5"], JsonType::JSON_STRING, ParseCode::PARSE_OK);
    EXPECT_EQ_STRING(json["key5"].to_string(), "abc");

    json = Json::parse("{ \"key1\" : {} , \"key2\" : { \"key3\" : true} }");
    EXPECT_EQ_INT(json.getErrorCode(), ParseCode::PARSE_OK);
    EXPECT_EQ_INT(json.type(), JsonType::JSON_OBJECT);
    EXPECT_EQ_INT(json.size(), 2);
    TEST_PARSE_JSON(json["key1"], JsonType::JSON_OBJECT, ParseCode::PARSE_OK);
    EXPECT_EQ_INT(json["key1"].size(), 0);
    TEST_PARSE_JSON(json["key2"], JsonType::JSON_OBJECT, ParseCode::PARSE_OK);
    EXPECT_EQ_INT(json["key2"].size(), 1);
    EXPECT_TRUE(json["key2"]["key3"].to_bool());
}

int main(){
    test_construct();
    test_parse();
    test_parse_invalid();
    test_array();
    test_object();

    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}