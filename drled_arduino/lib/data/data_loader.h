#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include "../log/logger.h"
#include "../json/parser.h"
#include "../json/generator.h"
#include "../file_system.h"

namespace DevRelief
{
    
extern Logger DataObjectLogger;    

class DataLoader {
    public:
        DataLoader() {
            m_logger = &DataObjectLogger;

        }

        bool writeJsonFile(const char * path,IJsonElement* json) {
            m_logger->debug("write JSON file %s",path);
            DRString buffer;
            m_logger->debug("\tgen JSON");
            JsonGenerator gen(buffer);
            gen.generate(json);
            m_logger->debug("\twrite JSON: %s",buffer.text());
            return m_fileSystem.write(path,buffer.text());
        }

        bool writeFile(const char * path, const char * text) {
            return m_fileSystem.write(path,text);
        }

        bool loadJsonFile(const char * path,auto reader) {
            DRFileBuffer buffer;
            if (m_fileSystem.read(path,buffer)){
                JsonParser parser;
                JsonRoot* root = parser.read(buffer.text());
                if (root) {
                    buffer.clear(); // free file read memory before parsing json
                    return reader(root->asObject());
                }
            }
            return false;
        }

    protected:
        Logger* m_logger; 
        static DRFileSystem m_fileSystem;
};

DRFileSystem DataLoader::m_fileSystem;

} // namespace DevRelief

#endif