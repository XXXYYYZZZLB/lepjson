#ifndef LEPTJSON_H__
#define LEPTJSON_H__
//用于防止头文件被反复包含

//json的几种数据类型
typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

//通用的json的一个数据节点的结构
typedef struct {
    lept_type type;
}lept_value;

//相当于宏定义，是几种提示解析后的状态
enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR
};

//解析，将字符串转换为lept_value树
int lept_parse(lept_value* v, const char* json);

//获取类型
lept_type lept_get_type(const lept_value* v);

#endif /* LEPTJSON_H__ */
