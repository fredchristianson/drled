#ifndef DRSCRIPT_CONTEXT_H
#define DRSCRIPT_CONTEXT_H

#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../lib/util/list.h"
#include "../lib/json/json.h"
#include "../lib/led/led_strip.h"
#include "../lib/led/color.h"
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
                m_valueList = new ScriptValueList(NULL);
            }

            virtual ~ScriptContext() {
                m_logger->debug("delete ScriptContext type: %s",m_type);
                if (m_currentStep) { m_currentStep->destroy();}
                if (m_lastStep) { m_lastStep->destroy();}
            }

            void destroy() override { delete this;}

            void setStrip(IScriptHSLStrip*strip) {
                m_strip = strip;
                if (strip) {
                    m_positionDomain.setPosition(0,0,strip->getLength());
                } else {
                    m_positionDomain.setPosition(0,0,0);
                }
            }
            IScriptHSLStrip* getStrip() const override { return m_strip;}
            IScriptHSLStrip* getRootStrip() const override {
                IScriptHSLStrip* root = m_strip;
                while(root != NULL && root->getParent() != NULL) {
                    root = root->getParent();
                }
                return root;
            }

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

            
            PositionDomain* getAnimationPositionDomain() {
                return &m_positionDomain;
            };

            void setValue(const char * name, IScriptValue* value) override  {
                m_valueList->setValue(name,value);
            }

            IScriptValue* getValue(const char * name)override  {
                if (m_valueList == NULL) { return NULL; }
                return m_valueList->getValue(name);
            };

            IScriptValue* getSysValue(const char * name)override  {
                if (m_valueList == NULL) { return NULL; }
                DRFormattedString fullName("sys:%s",name);
                return m_valueList->getValue(fullName);
            };

            void setSysValue(const char * name, IScriptValue* value)  {
                DRFormattedString fullName("sys:%s",name);
                m_valueList->setValue(fullName,value);
            }
            

            IScriptElement* getCurrentElement() const override { return m_currentElement;}
            IScriptElement* setCurrentElement(IScriptElement* element) override { 
                auto prev = m_currentElement;
                m_currentElement = element;

                return prev;
            }

            void enterScope() override {
                m_valueList = new ScriptValueList(m_valueList);
            }

            void leaveScope() override {
                if (m_valueList) {
                    auto prev = m_valueList;
                    m_valueList = prev->getParentScope();
                    prev->destroy();
                }
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
            IScriptValueProvider * m_valueList;
            PositionDomain m_positionDomain;
    };

    class RootContext : public ScriptContext {
        public:
        RootContext(IHSLStrip* strip, JsonObject* params) : ScriptContext("RootContext")
        {
            m_logger->debug("RootContext(%x,%x)",strip,params);
            m_baseStrip = strip;
            if (params) {
                m_logger->debug("\tcopy params %s",params->toString().text());
                params->eachProperty([&](const char * name, IJsonElement* jsonVal){
                    IScriptValue* scriptValue = ScriptValue::create(jsonVal);
                    if (scriptValue) {
                        setValue(name,scriptValue);
                    }
                });
            } else {
                m_logger->debug("\tno params passed");
            }
            createSystemValues();
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
        void createSystemValues() {
            setSysValue("red",new ScriptNumberValue(HUE::RED));
            setSysValue("orange",new ScriptNumberValue(HUE::ORANGE));
            setSysValue("yellow",new ScriptNumberValue(HUE::YELLOW));
            setSysValue("green",new ScriptNumberValue(HUE::GREEN));
            setSysValue("cyan",new ScriptNumberValue(HUE::CYAN));
            setSysValue("blue",new ScriptNumberValue(HUE::BLUE));
            setSysValue("magenta",new ScriptNumberValue(HUE::MAGENTA));
            setSysValue("purple",new ScriptNumberValue(HUE::PURPLE));
            
        }
        IHSLStrip* m_baseStrip;
    };



   
}
#endif