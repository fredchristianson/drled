#ifndef JSON_INTERFACE_H
#define JSON_INTERFACE_H

#include "../log/logger.h"

namespace DevRelief {


extern Logger GenneratorLogger;
extern Logger JsonLogger;

typedef enum TokenType {
    TOK_EOD=1000,
    TOK_ERROR=1001,
    TOK_START=1002,
    TOK_NULL=1003,
    TOK_OBJECT_START=1,
    TOK_OBJECT_END=2,
    TOK_ARRAY_START=3,
    TOK_ARRAY_END=4,
    TOK_STRING=5,
    TOK_INT=6,
    TOK_FLOAT=7,
    TOK_COLON=8,
    TOK_COMMA=9,
    TOK_TRUE,
    TOK_FALSE
};

typedef enum JsonType {
    JSON_UNKNOWN=999,
    JSON_NULL=998,
    JSON_INTEGER=100,
    JSON_FLOAT=101,
    JSON_STRING=102,
    JSON_BOOLEAN=103,
    JSON_VARIABLE=104,
    JSON_OBJECT=5,
    JSON_ARRAY=6,
    JSON_ROOT=7,
    JSON_PROPERTY=8,
    JSON_ARRAY_ITEM=9
};

class JsonRoot;
class JsonArray;
class JsonObject;


class IJsonValueElement {
    public:
        virtual bool getBool(bool defaultValue)=0;
        virtual int getInt(int defaultValue)=0;
        virtual const char * getString(const char *defaultValue=NULL)=0;
        virtual double getFloat(double defaultValue)=0;
};

class IJsonElement {
    public:
        virtual void destroy()=0;
        virtual bool isArray()=0;
        virtual bool isObject()=0;
        virtual bool isString()=0;
        virtual bool isInt()=0;
        virtual bool isBool()=0;
        virtual bool isFloat()=0;
        virtual bool isVariable()=0;
        virtual bool isNumber()=0;
        virtual bool hasValue()=0;
        virtual bool isNull()=0;     

        virtual JsonRoot* getRoot()=0;
        virtual JsonArray* asArray()=0;
        virtual JsonObject* asObject()=0;
        virtual IJsonValueElement* asValue()=0;   

        virtual JsonType getType()=0;
};

class IParseGen {
    public:
};



}
#endif

