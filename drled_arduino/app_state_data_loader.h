#ifndef APP_STATE_DATA_LOADER_H
#define APP_STATE_DATA_LOADER_H

#include "./lib/log/logger.h"
#include "./lib/data/data_object.h"
#include "./lib/data/data_loader.h"
#include "./lib/json/parser.h"
#include "./app_state.h"


namespace DevRelief {

const char * STATE_PATH_BASE="/state/";
extern Logger AppStateLogger;

class AppStateDataLoader : public DataLoader {
    public:
        AppStateDataLoader() {
            m_logger = &AppStateLogger;
        }

        DRString getPath(const char * name) {
            DRString path= STATE_PATH_BASE;
            path += name;
            path += ".json";
            m_logger->debug("AppStateDataLoader getPath(%s)==>%s",name,path.text());
            return path;
        }


        bool save(AppState& state){
            m_logger->debug("save AppState");

            SharedPtr<JsonRoot> jsonRoot = toJson(state);
            
            bool success = writeJsonFile(getPath("state"),jsonRoot->getTopElement());
            m_logger->debug("\twrite %s",success?"success":"failed");
            return success;
        }

        bool load(AppState& state) {
            return loadJsonFile(getPath("state"),[&](JsonObject * obj) {
                if (obj != NULL) {
                    state.setExecuteType((ExecuteType)obj->getInt("type",EXECUTE_NONE));
                    state.setExecuteValue(obj->getString("value",(const char *)NULL));
                    state.setIsRunning(obj->getBool("is-running",false));
                    state.setIsStarting(obj->getBool("is-starting",false));
                    state.setParameters(obj->getChild("parameters"));
                    auto paramJson = state.getParameters();
                    DRString json = paramJson? paramJson->toJsonString() : DRString();
                    m_logger->debug("Load AppState: %s %s %d %s %s",
                                state.isStarting()?"starting":"",
                                state.isRunning()?"running":"",
                                (int)state.getType(),
                                state.getExecuteValue().text(),
                                json.text());

                    return true;
                }
                return false;
            });
        }

        SharedPtr<JsonRoot> toJson(AppState& state) {
            m_logger->debug("toJson");
            SharedPtr<JsonRoot> jsonRoot = new JsonRoot();
            m_logger->debug("\tcreateObj");
            JsonObject* obj = jsonRoot->getTopObject();
            m_logger->debug("\tset type");
            obj->setInt("type",(int)state.getType());
            m_logger->debug("\tset running");
            obj->setBool("is-running",state.isRunning());
            m_logger->debug("\tset starting");
            obj->setBool("is-starting",state.isStarting());
            m_logger->debug("\tset value");
            obj->setString("value",state.getExecuteValue().text());
            m_logger->debug("\tcreate params");
            JsonObject* params = obj->createObject("parameters");
            m_logger->debug("\tcopy params");
            state.copyParameters(params);
            m_logger->debug("\tobj: %s",obj->toJsonString().text());
            m_logger->debug("\treturn root: %s",jsonRoot->toJsonString().text());
            return jsonRoot;            
        }

      
    protected:

    private:
};
};
#endif
