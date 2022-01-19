#ifndef SCRIPT_DATA_GENERATOR_H
#define SCRIPT_DATA_GENERATOR_H

#include "../lib/log/logger.h"
#include "../lib/data/data_loader.h"
#include "../lib/json/parser.h"
#include "./script.h"
#include "./script_element.h"
#include "./script_container.h"
#include "./data_generator.h"
#include "./json_names.h"

namespace DevRelief {

    class ScriptDataGenerator {
        public:
            ScriptDataGenerator(){
                m_logger = &ScriptLoaderLogger;
            }


            JsonRoot* toJson(Script* script) {
                JsonRoot* root = new JsonRoot();
                JsonObject* obj  = root->getTopObject();
                obj->setString(S_SCRIPT_NAME,script->getName());
                obj->setInt(S_DURATION,script->getDuration());
                obj->setInt(S_FREQUENCY,script->getFrequency());
                addContainerElements(obj,script->getRootContainer());
                return root;
            }

            void addContainerElements(JsonObject* parent, ScriptContainer* container) {
                JsonArray* array = parent->createArray(S_ELEMENTS);
                container->getChildren().each([&](IScriptElement* child){
                    JsonObject* childObj = array->addNewObject();
                    elementToJson(childObj,child);
                });
            }

            void elementToJson(JsonObject*json, IScriptElement* element) {
                m_logger->debug("elementToJson: %s",element->getType());
                json->setString("type",element->getType());
                element->toJson(json);
                if (element->isContainer()) {
                    addContainerElements(json,(ScriptContainer*)element);
                }
            }


        private:
            Logger* m_logger;
    };

};

#endif
