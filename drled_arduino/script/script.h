#ifndef DRSCRIPT_H
#define DRSCRIPT_H

#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../lib/util/list.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "./script_element.h"
#include "./script_container.h"
#include "./script_context.h"
#include "../loggers.h"

namespace DevRelief
{
 
    extern Logger ScriptLogger;
    
    class Script 
    {
    public:
        Script()
        {
            m_logger = &ScriptLogger;
            m_name = "unnamed";
            m_durationMsecs = 0;
            m_frequencyMsecs = 50;
            m_rootContext = NULL;
            m_rootContainer = new ScriptRootContainer();
        }

        virtual ~Script() {
            if (m_rootContext) {
                m_rootContext->destroy();
            }
            if (m_rootContainer) {
                m_rootContainer->destroy();
            }
        }

        virtual void destroy() {
            delete this;
        }


        void begin(HSLStrip* strip, JsonObject* params) {
            m_logger->debug("Begin script %x %s",strip,m_name.text());
            m_rootContext = new RootContext(strip,params);

        }

        void step() {
            m_logger->debug("Begin step %s",m_name.text());
            m_rootContext->beginStep();
            m_logger->debug("\tupdate layout");

            m_rootContainer->updateLayout(m_rootContext);
            m_logger->debug("\tdraw");
            m_rootContainer->draw(m_rootContext);
            m_logger->debug("\tend step");

            m_rootContext->endStep();
            m_logger->debug("\tstep done");

        }

        void setName(const char * name) { m_name = name; }
        const char * getName() { return m_name;}

        void setDuration(int durationMsecs) { m_durationMsecs = durationMsecs;}
        int getDuration() { return m_durationMsecs;}
        void setFrequency(int frequencyMsecs) { m_frequencyMsecs = frequencyMsecs;}
        int getFrequency() { return m_frequencyMsecs;}

        ScriptRootContainer* getRootContainer() { return m_rootContainer;}
    private:
        Logger *m_logger;
        DRString m_name;
        ScriptRootContainer* m_rootContainer;
        RootContext* m_rootContext;
        int         m_durationMsecs;
        int         m_frequencyMsecs;
    };

   
}
#endif