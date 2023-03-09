#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <stdlib.h>  /* NULL, strtod() */


#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
/*断言lept_context c的第一个字符等于char ch，并将c->json的指针后移*/
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')


/*首先为了减少解析函数之间传递多个参数，我们把这些数据都放进一个 `lept_context` 结构体*/
typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {/*去掉空白*/
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

/**
合并以下三个函数（重构）
解析null
static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
解析true
static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}
解析false
static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}
**/

static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal, lept_type type) {
    size_t i;
    EXPECT(c, literal[0]);
    for (i = 0; literal[i + 1]; i++)/* literal[i + 1] 判断下一位是不是\0 */
        if (c->json[i] != literal[i + 1])
            return LEPT_PARSE_INVALID_VALUE;
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
}

/*解析number*/
/* strtod(const char *nptr,char **endptr)会扫描参数nptr字符串，跳过前面的空格字符，直到遇上数字或正负符号才开始做转换，
    到出现非数字或字符串结束时('\0')才结束转换，并将结果返回。
    若endptr不为NULL，则会将遇到不合条件而终止的nptr中的字符指针由endptr传回。
    参数nptr字符串可包含正负号、小数点或E(e)来表示指数部分。如123.456或123e-2。*/
static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* p = c->json;
    if (*p == '-') p++;

    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }

    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }

    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return LEPT_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    
    errno = 0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
        return LEPT_PARSE_NUMBER_TOO_BIG;
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}


static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 'n':  return lept_parse_literal(c, v, "null", LEPT_NULL);
        case 't':  return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':  return lept_parse_literal(c, v, "false", LEPT_FALSE);
        default:   return lept_parse_number(c, v);/*将剩余的情况给数字 -/0-9*/
        case '\0': return LEPT_PARSE_EXPECT_VALUE;/*结束*/
        
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    assert(v != NULL);/*？声明以后就不是空了吗*/
    c.json = json;
    v->type = LEPT_NULL;/*若 lept_parse() 失败，会把 v 设为 null 类型，所以这里先把它设为 null*/
    lept_parse_whitespace(&c);
    /*return lept_parse_value(&c, v);//缺少后处理*/
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);/*后处理空白*/
        if (*c.json != '\0')/*最后一个不是\0就说明格式不正确*/
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value *v){
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
