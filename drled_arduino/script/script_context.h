#ifndef DRSCRIPT_CONTEXT_H
#define DRSCRIPT_CONTEXT_H

#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../lib/util/list.h"
#include "../lib/json/json.h"
#include "../lib/led/led_strip.h"
#include "./script_interface.h"
#include "./script_element.h"
#include "./script_element_create.h"
#include "./script_hsl_strip.h"

namespace DevRelief
{
 
    extern Logger ScriptLogger;
    
    class ScriptStep : public IScriptStep {
        public:
            ScriptStep(ScriptStep* prev) {
                m_startTimeMsecs = millis();
                m_msecsSincePrev = m_startTimeMsecs-(prev ? prev->getStartMsecs(): 0);
                m_stepNumber = prev ? prev->getNumber()+1:0;
            }

            virtual ~ScriptStep() {

            }

            void destroy() override { delete this;}

            long getStartMsecs() override { return m_startTimeMsecs;}
            long getMsecsSincePrev() override { return m_msecsSincePrev;}
            int getNumber() override { return m_stepNumber;}
        protected:
            long m_startTimeMsecs;
            long m_msecsSincePrev;
            long m_stepNumber;
    };

    class ScriptContext: public IScriptContext
    {
        public:
            ScriptContext(const char * type,IScriptHSLStrip* strip) { 
                m_type = type;
                m_logger = &ScriptLogger;
                m_strip = strip;
                m_logger->debug("Create ScriptContext type: %s",m_type);
                m_currentStep = NULL;
                m_lastStep = NULL;
            }

            virtual ~ScriptContext() {
                m_logger->debug("delete ScriptContext type: %s",m_type);
                if (m_currentStep) { m_currentStep->destroy();}
                if (m_lastStep) { m_lastStep->destroy();}
            }

            void destroy() override { delete this;}

            IScriptHSLStrip* getStrip() override {
                return m_strip;
            };

            IScriptStep* getStep() override {
                return m_currentStep;
            };

            IScriptStep* getLastStep() override {
                return m_lastStep;
            };

            IScriptStep* beginStep() override {
                ScriptStep* step = new ScriptStep(m_currentStep);
                if (m_currentStep) {
                    m_currentStep->destroy();
                };
                m_currentStep = step;
                initializeStep();
                return m_currentStep;
            }

            void endStep() override {
                finalizeStep();
                if (m_lastStep) { m_lastStep->destroy();}
                m_lastStep = m_currentStep;
                m_currentStep = NULL;
            }

            
            IAnimationDomain* getAnimationPositionDomain() {

            };


            IScriptValue* getValue(const char * name) {

            };


        protected:
            virtual void initializeStep()=0;
            virtual void finalizeStep()=0;
            const char * m_type;
            Logger* m_logger;    
            IScriptHSLStrip * m_strip;
            ScriptStep * m_currentStep;
            ScriptStep * m_lastStep;
    };

    class RootContext : public ScriptContext {
        public:
        RootContext(HSLStrip* strip, JsonObject* params) : ScriptContext("RootContext",new RootHSLStrip(strip))
        {
            m_strip = strip;
            m_params = params;
        }

        virtual ~RootContext() {
            if (m_params) {
                m_params->destroy();
            }
        }

        void initializeStep() {

        }

        void finalizeStep() {
            m_strip->show();
        }

        protected:
        JsonObject* m_params;
        HSLStrip* m_strip;
    };



   
}
#endif