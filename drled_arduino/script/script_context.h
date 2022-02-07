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
            ScriptStep() {
                m_startTimeMsecs = 0;
                m_msecsSincePrev = m_startTimeMsecs;
                m_stepNumber = 0;
                ScriptLogger.never("ScriptStep # %d",m_stepNumber);
            }

            virtual ~ScriptStep() {
                ScriptLogger.never("~ScriptStep # %d  %x",m_stepNumber);

            }

            void destroy() override { delete this;}

            void begin(ScriptStep* prevHolder) {
                if (prevHolder) {
                    prevHolder->copy(this);
                }
                long now = millis();
                m_msecsSincePrev = now-m_startTimeMsecs;
                m_startTimeMsecs = now;
                m_stepNumber += 1;
            }

            void end(ScriptStep* prevHolder) {
                if (prevHolder) {
                    prevHolder->copy(this);
                }
            }

            void copy(ScriptStep* other) {
                m_startTimeMsecs = other->m_startTimeMsecs;
                m_msecsSincePrev = other->m_msecsSincePrev;
                m_stepNumber = other->m_stepNumber;
            }

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
                m_logger->never("Create ScriptContext type: %s",m_type);
                m_strip = NULL;
                m_position = NULL;
                m_parentContext = NULL;
                m_startTimeMsecs = millis();
                m_valueList = new ScriptValueList();
            }

            virtual ~ScriptContext() {
                m_logger->debug("delete ScriptContext type: %s",m_type);
                if (m_valueList) { m_valueList->destroy();}
            }


            void destroy() override { delete this;}

            void setPosition(IElementPosition* position) override  { m_position = position;}
            IElementPosition* getPosition() const override { return m_position;}

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


            
            PositionDomain* getAnimationPositionDomain() {
                return &m_positionDomain;
            };

            void setValue(const char * name, IScriptValue* value) override  {
                m_valueList->setValue(name,value);
            }

            IScriptValue* getValue(const char * name)override  {
                if (m_valueList == NULL) { return NULL; }
                IScriptValue* val = m_valueList->getValue(name);
                if (val == NULL && m_parentContext != NULL) {
                    val = m_parentContext->getValue(name);
                }
                return val;
            };

            IScriptValue* getSysValue(const char * name)override  {
                if (m_valueList == NULL) { return NULL; }
                DRFormattedString fullName("sys:%s",name);
                return getValue(fullName.text());
                //return m_valueList->getValue(fullName);
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

            void setParentContext(IScriptContext* parent) { 
                m_parentContext = parent;
                if (parent) {
                    m_strip = parent->getStrip();
                }
            }
          
        protected:
            long m_startTimeMsecs;
            IScriptContext* m_parentContext;
            IScriptHSLStrip* m_strip;
            const char * m_type;
            Logger* m_logger;    

            IScriptElement* m_currentElement; 
            IScriptValueProvider * m_valueList;
            PositionDomain m_positionDomain;
            IElementPosition* m_position;
    };

    class RootContext : public ScriptContext {
        public:
        RootContext(IHSLStrip* strip, JsonObject* params) : ScriptContext("RootContext")
        {
            m_logger->never("RootContext(%x,%x)",strip,params);
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

        IScriptStep* getStep() override {
            return &m_currentStep;
        };

        IScriptStep* getLastStep() override {
            return &m_lastStep;
        };

        IScriptStep* beginStep()  {
            m_currentStep.begin(&m_lastStep);
            return &m_currentStep;
        }

        void endStep()  {
           m_currentStep.end(&m_lastStep);
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
            ScriptStep  m_currentStep;
            ScriptStep  m_lastStep;
    };

    class ChildContext : public ScriptContext {
        public:
            ChildContext(const char * type="child") : ScriptContext(type) {
                
            }


            IScriptStep* getStep() override {
                return m_parentContext ? m_parentContext->getStep() : NULL;
            };

            IScriptStep* getLastStep() override {
                return m_parentContext ? m_parentContext->getLastStep() : NULL;
            };

        protected:

    };

    bool StepWatcher::isChanged(IScriptContext*ctx) { 
        if (ctx == NULL) { return true;}
        int number = ctx->getStep()->getNumber();
        if (number != m_stepNumber) {
            m_stepNumber = number;
            return true;
        }
        return false;
    }
   
}
#endif