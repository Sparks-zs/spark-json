add_library(spark_json
    STATIC
        spark_json.cpp
        spark_json.h
)

install(TARGETS spark_json DESTINATION lib)

install(FILES spark_json.h DESTINATION include/spark_json)