#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */

//断言lept_context c的第一个字符等于char ch，并将c->json的指针后移
#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

//首先为了减少解析函数之间传递多个参数，我们把这些数据都放进一个 `lept_context` 结构体
typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {//去掉空白
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

//解析null
static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
////解析true
static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}
//解析false
static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 'n':  return lept_parse_null(c, v);
        case 't':  return lept_parse_true(c, v);
        case 'f':  return lept_parse_false(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;//结束
        default:   return LEPT_PARSE_INVALID_VALUE;//无效
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    assert(v != NULL);//？声明以后就不是空了吗
    c.json = json;
    v->type = LEPT_NULL;//若 lept_parse() 失败，会把 v 设为 null 类型，所以这里先把它设为 null
    lept_parse_whitespace(&c);
    //return lept_parse_value(&c, v);//缺少后处理
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);//后处理空白
        if (*c.json != '\0')//最后一个不是\0就说明格式不正确
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}
