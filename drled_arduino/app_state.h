#ifndef DRLED_APP_STATE_H
#define DRLED_APP_STATE_H

#include "./lib/log/logger.h"

namespace DevRelief {

    typedef enum ExecuteType  {
        EXECUTE_NONE=0,
        EXECUTE_API=1,
        EXECUTE_SCRIPT=2
    };

    class AppState {
        public:
 
            AppState() {
                m_logger = &AppStateLogger;
                m_executeType = EXECUTE_NONE;
                m_starting = true;
                m_running = false;
            }

            ~AppState() { }

            void setIsStarting(bool is) { 
                m_logger->debug("setIsStarting %d",is?1:0);
                m_starting = is;
            }

            void setIsRunning(bool is) { 
                m_logger->debug("setIsRunning %d",is?1:0);
                m_running = is;
            }
            
            void setExecuteType(ExecuteType type) { m_executeType = type;}
            void setExecuteValue(const char * val) { m_executeValue = val;}
            void setParameters(JsonObject* params) {
                JsonObject* old = m_root.getTopObject();
                copyParameters(old,params);
            }

            void setApi(const char * api, JsonObject*params) {
                setExecuteValue(api);
                // when setApi is called, the API is running and has started
                setIsRunning(true);
                setIsStarting(false);
                setExecuteType(EXECUTE_API);
                copyParameters(m_root.getTopObject(),params);
            }
             
            void setScript(const char * name, JsonObject*params) {
                m_logger->debug("set script %x",name);
                setExecuteValue(name);
                m_logger->debug("\tset isRunning %s",name);
                setIsRunning(true);
                // set as starting until it has run long
                // enough to see it works (10 seconds?).
                // otherwise can get in a reboot-loop
                setIsStarting(true);
                setExecuteType(EXECUTE_SCRIPT);
                m_logger->debug("\tcopy parameters");
                copyParameters(m_root.getTopObject(),params);
                m_logger->debug("\tcopied parameters");
            }

            void copyParameters(JsonObject*  toObj,JsonObject*params=NULL) {
                m_logger->debug("copyParameters");
                if (toObj == NULL) {
                    m_logger->info("\tno toObj");
                     return;
                }
                toObj->clear();
                m_logger->debug("\tcleared");
                if (params == NULL){
                    m_logger->debug("\tget current");
                    params = m_root.getTopObject();
                }
                if (params == NULL) {
                    m_logger->debug("\tno params");

                    return;
                }

                m_logger->debug("\teachProperty");

                params->eachProperty([&](const char * name, IJsonElement*val){
                    m_logger->debug("\tcopy %s",name);
                    IJsonValueElement* ve = val->asValue();
                    toObj->setString(name,ve->getString(NULL));
                });
                m_logger->debug("done");
            }

            JsonObject* getParameters() { return m_root.getTopObject();}
            const DRString& getExecuteValue()const { return m_executeValue;}
            bool isStarting() { return m_starting;}
            bool isRunning() { return m_running;}
            ExecuteType getType() { return m_executeType;}
    private:            
            DRString        m_executeValue;
            bool            m_starting;
            bool            m_running;
            ExecuteType     m_executeType;
            Logger *        m_logger;
            JsonRoot        m_root;
    };

}


#endif