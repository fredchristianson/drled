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
            m_rootContainer = NULL;
        }

        virtual ~Script() {
            m_logger->test("~Script %x",this);
            if (m_rootContext) {
                m_logger->test("destroy rootcontext %x",m_rootContext);
                m_rootContext->destroy();
            }
            if (m_rootContainer) {
                m_logger->test("destroy rootcontainer %x",m_rootContainer);
                m_rootContainer->destroy();
            }
            m_logger->test("~Script done");
        }

        virtual void destroy() {
            m_logger->test("destroy Script");
            delete this;
        }


        void begin(IHSLStrip* strip, JsonObject* params) {
            m_logger->debug("Begin script %x %s",strip,m_name.text());
            m_realStrip = strip;
            getRootContainer()->setStrip(strip);

            if (m_rootContext) {
                m_rootContext->destroy();
            }
            m_rootContext = new RootContext(strip,params);

        }

        void step() {
            auto lastStep = m_rootContext->getLastStep();
            if (lastStep && m_frequencyMsecs>0 && lastStep->getStartMsecs() + m_frequencyMsecs > millis()) {
                m_logger->test("too soon %d %d %d",lastStep?lastStep->getStartMsecs():-1, m_frequencyMsecs , millis());
                return; // to soon to start next step
            }
            m_logger->never("step %d %d %d",lastStep?lastStep->getStartMsecs():-1, m_frequencyMsecs , millis());
            m_logger->never("Begin step %s",m_name.text());
            
            m_realStrip->clear();
            m_rootContext->beginStep();
            m_logger->never("\tupdate layout");

            m_rootContainer->updateLayout(m_rootContext);
            m_logger->never("\tdraw");
            m_rootContainer->draw(m_rootContext);
            m_logger->never("\tend step");

            m_rootContext->endStep();
            m_realStrip->show();
            m_logger->never("\tstep done");

        }

        void setName(const char * name) { m_name = name; }
        const char * getName() { return m_name;}

        void setDuration(int durationMsecs) { m_durationMsecs = durationMsecs;}
        int getDuration() { return m_durationMsecs;}
        void setFrequency(int frequencyMsecs) {
            m_logger->info("script frequency %d",frequencyMsecs);
            m_frequencyMsecs = frequencyMsecs;
        }
        int getFrequency() { return m_frequencyMsecs;}

        ScriptRootContainer* getRootContainer() { 
            if (m_rootContainer == NULL){
                m_rootContainer = new ScriptRootContainer();
            }
            return m_rootContainer;
        }
    private:
        Logger *m_logger;
        DRString m_name;
        ScriptRootContainer* m_rootContainer;
        RootContext* m_rootContext;
        IHSLStrip* m_realStrip;
        int         m_durationMsecs;
        int         m_frequencyMsecs;
    };

   
}
#endif