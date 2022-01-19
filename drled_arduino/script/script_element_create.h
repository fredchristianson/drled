#ifndef SCRIPT_ELEMENT_CREATE_H
#define SCRIPT_ELEMENT_CREATE_H

#include "../lib/log/logger.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "./script_value.h"
#include "./script_element.h"
#include "./json_names.h"

namespace DevRelief {
    extern Logger ScriptElementLogger;

    class ScriptElementCreator {
        public:
            ScriptElementCreator(IScriptContainer* container) {
                m_container = container;
                m_logger = &ScriptElementLogger;
            }

            IScriptElement* elementFromJson(IJsonElement* json){
                m_logger->debug("parse Json type=%d",json->getType());
                JsonObject* obj = json ? json->asObject() : NULL;
                if (obj == NULL){
                    m_logger->error("invalid IJsonElement to create script element");
                    return NULL;
                }

                const char * type = obj->getString("type",NULL);
                IScriptElement* element = NULL;
                if (type == NULL) {
                    m_logger->error("json is missing a type");
                    return NULL;
                } else if (Util::equal(type,S_VALUES)) {
                    element = new ValuesElement();
                } else if (Util::equalAny(type,S_HSL)) {
                    element = new HSLElement();
                } else if (Util::equalAny(type,S_RHSL)) {
                    element = new RainbowHSLElement();
                }
                if (element) {
                    element->fromJson(obj);
                }
                return element;
            }

        private:
            IScriptContainer* m_container;
            Logger* m_logger;
    };
};

#endif