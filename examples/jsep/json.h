#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__
#include <vector>
#include <string>
#include <sstream>
#include <cstdarg>
#include <cassert>
#include <map>
enum {
    /** ��������,�߱�ԭ���Ե�JSON���� */
    JSON_PRIMITIVE      = 1,
    /** �ַ��� */
    JSON_STRING         = 2,
    /** ������� */
    JSON_ARRAY          = 3,
    /** ���ϵ��ֵ���� */
    JSON_OBJECT         = 4,
};

enum {
    /** JSON�������������tokens, ���Ȳ��� */
    JSON_ERROR_NOMEM    = -1,
    /** �Ƿ���JSON�ַ��� */
    JSON_ERROR_INVAL    = -2,
    /** �ַ������Ǹ�������JSON��ʽ */
    JSON_ERROR_PART     = -3,
};

/**
 * JSON���������.
 * @param[in] type ����, ������JSON_OBJECT, JSON_ARRAY, JSON_STRING ֮һ
 * @param[in] start ��������JSON�ַ�������ʼλ��
 * @param[in] end ��������JSON�ַ����Ľ���λ��
 */
typedef struct {
    int         type;
    int         start;
    int         end;
    int         size;
    int         parent;
} json_t;

/**
 * ������. 
 * ������0,��ʼ��.
 */
typedef struct {
    /*< private >*/
    unsigned    _[3];
} json_p;

/**
 * ����JSON�ַ���.
 * ����tokens�н�������ÿ��JSON���������.
 * JSON������ָԭ�Ӷ���, �������򸴺϶���.
 *
 * @param[in] parser ������
 * @param[in] json ������JSON�ַ���
 * @param[in] len �ַ���json����Ч����
 * @param[out] tokens �����󵥸�JSON���������
 * @param[in] num_tokens ����������tokens����Ч����
 * @return �ɹ�����0,��֮����JSON_ERROR_*ֵ
 */
int json_parse(json_p *parser, const char *json, unsigned len, json_t *tokens, unsigned num_tokens);

/**
 * ����strcmp,�Ƚ��ַ���.
 * ���ǱȽ��ǻ���JSON����.
 * JSON������ָԭ�Ӷ���, �������򸴺϶���.
 *
 * @param[in] json ������JSON�ַ���
 * @param[in] t ָ�򵥸�JSON���ڵ�λ��
 * @param[in] key �ȽϵĶ���
 * @return ���0, ����1, С��-1
 */
int json_strcmp(const char *json, json_t *token, const char *key);

/**
 * �����﷨�ϵ���JSON����
 * JSON������ָԭ�Ӷ���, �������򸴺϶���.
 * ͨ��Ҫ�������json_t.
 *
 * @param[in] t ��ǰjson_t�������ڵ�λ��
 * @param[in] end ��ǰjson_t���������λ��
 * @return ����·����json_t�������
 */
int json_skip(const json_t *t, const json_t* end);

/** ����ַ����Ƿ�߱�ԭ���� */
int json_atomic(const char* str);


struct json_o;
///JSON������ֵ�
typedef std::map<std::string, json_o> json_map;
///JSON���������
typedef std::vector<json_o>  json_vec;
/// ���C++,��ճɸ�����Ķ���
struct json_o
{
    static const json_o null;//�Ƿ��Ŀ�JSON����

    json_o():type(0),str(0){}
    json_o(const json_o& o):type(o.type), str(o.str) { o.type = 0; }
    ~json_o() { clean(); }
    ///���ַ�������JSON����
    explicit json_o(const std::string& json):type(0),str(0) { if (!from(json)) clean(); }
    explicit json_o(const char *format, va_list argv):type(0),str(0) { if (!fromv(format, argv)) clean(); }
    explicit json_o(const char *format, ...):type(0),str(0) {
        va_list argv;
        va_start(argv, format);
        fromv(format, argv);
        va_end(argv);
    }
    ///���ַ�������JSON����
    bool fromv(const char *format, va_list argv);
    bool from(const std::string& json);
    bool operator()(const char* format, ...) {
        va_list argv;
        va_start(argv, format);
        bool ret = fromv(format, argv);
        va_end(argv);
        return ret;
    }
    bool operator()(const std::string& json) { return from(json); }
    bool operator()(const char* format, va_list argv) { return fromv(format, argv); }

    ///��JSON����ԭ���ַ���
    std::string to_str() const;

    ///�Ƿ�Ϊ��JSON����
    bool atomic() const {return (type == JSON_STRING || type == JSON_PRIMITIVE);}

    ///Ϊ�����ַ�����ǰ׺,���ط���JSON��ʽ���ַ���
    static std::string escape(const std::string& str);

    ///ȥ��JSON�������ַ���ǰ׺,����ԭ�ַ���
    static std::string unescape(const std::string& json);

    ///����ַ����Ƿ�߱�ԭ����
    static bool atomic(const std::string& str) { return json_atomic(str.c_str()) != 0; }

    mutable int type;
    union {
        std::string *str;
        json_map    *map;
        json_vec    *vec;
    };
    
    ///�Ƚϵ�ǰ��JSON�������ַ����Ƿ����
    bool operator ==(const char* s) const { return atomic() && *str == s;}
    bool operator ==(const std::string& s) const { return atomic() && *str == s; }
    bool operator !=(const std::string& s) const { return !(*this == s);}

    ///�����ֵ���Ԫ��,�������ڷ���null
    const json_o& operator[](const std::string& key) const {
        if (type != JSON_OBJECT) return null;
        json_map::const_iterator iter = map->find(key);
        if (iter == map->end()) return null;
        return iter->second;
    }
    const json_o& operator[](const char* key) const { return (*this)[std::string(key)]; }

    ///�����ֵ���Ԫ��,��������������
    json_o& operator[](const std::string& key);
    json_o& operator[](const char* key) { return (*this)[std::string(key)]; }
    json_o& operator=(const json_o& o) { clean(); type = o.type; str = o.str; o.type = 0; return *this; }

    ///����������Ԫ��,�������ڷ���null
    const json_o& operator[](const int index) const {
        if (type != JSON_ARRAY) return null;
        if (index < 0 || index >= (int)vec->size()) return null;
        return (*vec)[index];
    }

    ///����������Ԫ��,��������������
    json_o& operator[](const int index);

    ///�ж��Ƿ�Ϊ��ЧJOSN����
    operator bool () const { return type != 0;}
    bool operator!() const { return type == 0;}

    ///���ؼ�JSON������ַ���ֵ
    operator std::string& () { assert(atomic()); return *str; }
    operator const std::string& () const{ assert(atomic()); return *str; }
    operator const char* () const { return atomic() ? str->c_str() : 0;}

    ///��յ�ǰJSON����,��ɷǷ�JSON����
    void clean();
};
#endif /* __JSON_PARSER_H__ */
