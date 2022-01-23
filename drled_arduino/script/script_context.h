#ifndef DRSCRIPT_CONTEXT_H
#define DRSCRIPT_CONTEXT_H

#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../lib/util/list.h"
#include "../lib/json/json.h"
#include "../lib/led/led_strip.h"
#include "./script_interface.h"
#include "./script_element.h"
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
            ScriptContext(const char * type) { 
                m_type = type;
                m_logger = &ScriptLogger;
                m_logger->debug("Create ScriptContext type: %s",m_type);
                m_currentStep = NULL;
                m_lastStep = NULL;
                m_strip = NULL;
            }

            virtual ~ScriptContext() {
                m_logger->debug("delete ScriptContext type: %s",m_type);
                if (m_currentStep) { m_currentStep->destroy();}
                if (m_lastStep) { m_lastStep->destroy();}
            }

            void destroy() override { delete this;}

            void setStrip(IScriptHSLStrip*strip) { m_strip = strip;}
            IScriptHSLStrip* getStrip() const override { return m_strip;}
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
                m_logger->error("getAnimationPositionDomain not implemented");
                return NULL;
            };


            IScriptValue* getValue(const char * name) {
                m_logger->error("getValue not implemented");
                return NULL;
            };

            IScriptElement* getCurrentElement() const override { return m_currentElement;}
            IScriptElement* setCurrentElement(IScriptElement* element) override { 
                auto prev = m_currentElement;
                m_currentElement = element;

                return prev;
            }


        protected:
            virtual void initializeStep()=0;
            virtual void finalizeStep()=0;

            IScriptHSLStrip* m_strip;
            const char * m_type;
            Logger* m_logger;    
            ScriptStep * m_currentStep;
            ScriptStep * m_lastStep;
            IScriptElement* m_currentElement; 
    };

    class RootContext : public ScriptContext {
        public:
        RootContext(IHSLStrip* strip, JsonObject* params) : ScriptContext("RootContext")
        {
            m_logger->debug("RootContext(%x,%x)",strip,params);
            m_strip = strip;
            m_logger->debug("\tget paramRoot object");
            m_params = m_paramRoot.getTopObject();

            if (params) {
                m_logger->debug("\tcopy params %s",params->toString().text());
                params->eachProperty([&](const char * name, IJsonElement* value){
                    auto v = value->asValue();
                    m_params->setString(name,v? v->getString("unset"):"unset");
                });
            } else {
                m_logger->debug("\tno params passed");
            }
            m_logger->debug("created RootContext %x",this);
        }

        virtual ~RootContext() {
            m_logger->debug("~RootContext %x",this);

        }

        void initializeStep() {

        }

        void finalizeStep() {
        }

        protected:
        JsonRoot  m_paramRoot;
        JsonObject* m_params;
        IHSLStrip* m_strip;
    };



   
}
#endif