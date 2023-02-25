#include "spark_json.hpp"

using namespace SparkJson;

static int test_count = 0;
static int test_pass = 0;
static int main_ret = 0; 

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++; \
        if(equality) \
            test_pass++; \
        else{ \
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    }while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect == actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect == actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == alength && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")

void parse_json_null(){
    Json json1;
    Json json2(nullptr);

    EXPECT_EQ_INT(json1.type(), JsonType::JSON_NULL);
    EXPECT_EQ_INT(json2.type(), JsonType::JSON_NULL);
}

void parse_json_bool(){
    Json json1(true);
    Json json2(false);

    EXPECT_EQ_INT(json1.type(), JsonType::JSON_BOOL);
    EXPECT_EQ_INT(json2.type(), JsonType::JSON_BOOL);
}

void parse_json_number(){
    Json json1(3);
    Json json2(3.0);
    Json json3 = 3.0;

    EXPECT_EQ_INT(json1.type(), JsonType::JSON_NUMBER);
    EXPECT_EQ_INT(json2.type(), JsonType::JSON_NUMBER);
    EXPECT_EQ_INT(json3.type(), JsonType::JSON_NUMBER);
}

void parse_json_string(){
    Json json1("STRING");

    EXPECT_EQ_INT(json1.type(), JsonType::JSON_STRING);
}

void parse_invalid_value(){
    Json json1 = Json::parse(" true ");
    Json json2 = Json::parse("true");
    Json json3 = Json::parse(" ");
    Json json4 = Json::parse(" true false");

    EXPECT_EQ_INT(json1.type(), JsonType::JSON_BOOL);
    EXPECT_EQ_INT(json2.type(), JsonType::JSON_BOOL);
    EXPECT_EQ_INT(json3.type(), JsonType::JSON_NULL);
    EXPECT_EQ_INT(json3.getErrorCode(), ParseCode::PARSE_EXPECT_VALUE);
    EXPECT_EQ_INT(json4.getErrorCode(), ParseCode::PARSE_ROOT_NOT_SINGULAR);

}

int main(){
    parse_json_null();
    parse_json_bool();
    parse_json_number();
    parse_json_string();

    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}