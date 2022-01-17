#ifndef SCRIPT_DATA_LOADER_H
#define SCRIPT_DATA_LOADER_H

#include "../lib/log/logger.h"
#include "../lib/data/data_loader.h"
#include "./script.h"

namespace DevRelief {

    class ScriptDataLoader : DataLoader {
        public:
            ScriptDataLoader(){
                m_logger = &ScriptLoaderLogger;
            }

            Script* load(const char * name) {
                return NULL;
            }

            bool save(const char * name, Script& script) {
                return false;
            }

            bool save(const char * name, const char * text) {
                return false;
            }

            SharedPtr<JsonRoot> toJson(Script& script) {
                return NULL;
            }

        private:
            Logger* m_logger;
    };

};

#endif
