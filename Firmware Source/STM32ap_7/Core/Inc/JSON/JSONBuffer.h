
#define JSONBUFFER_BANKNUM CODERBUFFER_BANKNUM
typedef struct{
    Addstr_Str *buffer;
    json_t *json;
    json_t *pool;
} JSONBuff, *JSONBuffer;

JSONBuffer JSONBuffer_make(Addstr_Str* str);
void JSONBuffer_free(JSONBuffer buffer);