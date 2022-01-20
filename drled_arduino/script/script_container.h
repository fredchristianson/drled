#ifndef DRSCRIPT_CONTAINER_H
#define DRSCRIPT_CONTAINER_H

#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../lib/util/list.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "./script_element.h"
#include "./script_element_create.h"
#include "../loggers.h"

namespace DevRelief
{
 
    extern Logger ScriptContainerLogger;
    
    class ScriptContainer: public ScriptElement, public IScriptContainer
    {
    public:
        ScriptContainer(const char * type) : ScriptElement(type)
        {
            m_logger = &ScriptContainerLogger;
            m_logger->debug("Create ScriptContainer type: %s",m_type);
        }

        virtual ~ScriptContainer() {
            m_logger->debug("delete ScriptContainer type: %s",m_type);
        }

        void destroy() override {
            m_logger->debug("destroy ScriptContainer type: %s",m_type);
            delete this;
            m_logger->debug("\tdestroyed ScriptContainer");
        }

        bool isContainer()const  override {
            return true;
        }

        void add(IScriptElement* child) {
            m_children.add(child);
        }

        const PtrList<IScriptElement*>& getChildren() const override { return m_children;}
        
        void elementsFromJson(JsonArray* array) override {
            m_children.clear();
            if (array == NULL) {
                return;
            }
            ScriptElementCreator creator(this);
            array->each([&](IJsonElement* element) {
                IScriptElement*child = creator.elementFromJson(element);
                if (child) {
                    m_children.add(child);
                }
            });
        }

        void updateLayout(IScriptContext* context) override {
            auto strip = context->getStrip();
            int stripPos = 0;
            int stripLength = strip->getLength();
            m_children.each([&](IScriptElement* child) {
                if (child->isPositionable()) {
                    IPositionable* positionable = (IPositionable*)child;
                    IElementPosition* pos = positionable->getPosition();
                    int offset = 0;
                    if (pos->getOffset()) {
                        offset = pos->evalOffset(context);
                    }
                    int length = stripLength - stripPos - offset;
                    if (pos->getLength()){
                        length = pos->evalLength(context);
                    }
                    pos->setPosition(stripPos+offset,length,context);
                    stripPos += offset+length;
                }
                child->updateLayout(context);
            });
        };

        void draw(IScriptContext* context) override {
            m_children.each([&](IScriptElement* child) {
                child->draw(context);
            });
        };

    protected:
        PtrList<IScriptElement*> m_children;
        Logger* m_logger;
    };

    class ScriptRootContainer : public ScriptContainer {
        public:
            ScriptRootContainer() : ScriptContainer(S_ROOT_CONTAINER) {

            }

            virtual ~ScriptRootContainer() {
                
            }

        private:
    };

   
}
#endif