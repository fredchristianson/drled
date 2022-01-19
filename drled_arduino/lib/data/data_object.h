#ifndef DATA_OBJECT_H
#define DATA_OBJECT_H

#include "../log/logger.h"
#include "../json/parser.h"

namespace DevRelief
{
extern Logger DataObjectLogger;

class DataObject {
    public:
        DataObject() {
            m_logger = &DataObjectLogger;
            m_json = m_jsonRoot.getTopObject();

        }

        void addProperty(const char* name, int val){
            m_json->setInt(name,val);
        }

        void addProperty(const char* name, const char * val){
            m_json->setString(name,val);
        }

        void addProperty(const char* name, double val){
            m_json->setFloat(name,val);
        }

        void addProperty(const char* name, bool val){
            m_json->setBool(name,val);
        }

        

    protected:
        JsonRoot    m_jsonRoot;
        JsonObject* m_json;
        Logger* m_logger; 
};


} // namespace DevRelief

#endif