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
                m_sanity1 = 0xa5a5;
                m_sanity2 = 0x5a5a;
                checkSanity();
            }

            ~AppState() { }

            bool checkSanity() {
                m_logger->never("sanity check %x %x %x",m_sanity1,m_sanity2,m_parameterRoot.getJsonId());
                m_logger->showMemory();
                if (m_sanity1 != 0xa5a5 || m_sanity2 != 0x5a5a) {
                    m_logger->error("AppState is corrupt %x(%d) %x(%d)",m_sanity1,(m_sanity1!=0xa5a5), m_sanity2,(m_sanity2!=0x5a5a));
                    return false;
                }
                m_logger->never("\tparameters",m_parameterRoot.getTopObject()->toString().text());
                return true;
            }

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
                checkSanity(); 
                if (params == NULL){
                    m_logger->debug("AppState parameters = NULL");
                } else {
                    m_logger->debug("set parameters %s",params->toString().text());
                }
                JsonObject* paramObject = m_parameterRoot.getTopObject();
                copyParameters(paramObject,params);
                checkSanity(); 
            }

            void setApi(const char * api, JsonObject*params) {
                checkSanity(); 
                setExecuteValue(api);
                // when setApi is called, the API is running and has started
                setIsRunning(true);
                setIsStarting(false);
                setExecuteType(EXECUTE_API);
                copyParameters(m_parameterRoot.getTopObject(),params);
                checkSanity(); 
            }
             
            void setScript(const char * name, JsonObject*params) {
                checkSanity(); 
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
                copyParameters(m_parameterRoot.getTopObject(),params);
                m_logger->debug("\tcopied parameters");
                checkSanity(); 
            }

            void copyParameters(JsonObject*  toObj,JsonObject*params=NULL) {
                checkSanity(); 
                m_logger->debug("copyParameters %x",toObj);
                if (toObj == NULL) {
                    m_logger->info("\tno toObj");
                     return;
                }
                toObj->clear();
                m_logger->debug("\tcleared");
                if (params == NULL){
                    m_logger->debug("\tget current");
                    params = m_parameterRoot.getTopObject();
                }
                if (params == NULL) {
                    m_logger->debug("\tno params");

                    return;
                }

                m_logger->debug("\teachProperty");

                params->eachProperty([&](const char * name, IJsonElement*val){
                    m_logger->debug("\tcopy %s",name);
                    IJsonValueElement* ve = val->asValue();
                    if (ve) {
                        toObj->setString(name,ve->getString(NULL));
                    } else {
                        m_logger->error("AppState only supports string parameter values");
                    }
                });
                m_logger->debug("done");
                checkSanity(); 

            }

            JsonObject* getParameters() { return m_parameterRoot.getTopObject();}
            const char* getExecuteValue()const { return m_executeValue.text();}
            bool isStarting() { return m_starting;}
            bool isRunning() { return m_running;}
            ExecuteType getType() { return m_executeType;}
    private:            
            DRString        m_executeValue;
            bool            m_starting;
            bool            m_running;
            ExecuteType     m_executeType;
            Logger *        m_logger;
            int             m_sanity1;
            JsonRoot        m_parameterRoot;
            int             m_sanity2;
    };

}


#endif