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
        ScriptContainer(const char * type, IScriptHSLStrip* strip, IElementPosition* position) : ScriptElement(type, strip)
        {
            m_logger = &ScriptContainerLogger;
            m_logger->debug("Create ScriptContainer type: %s",m_type);
            m_position = position;
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
            m_logger->never("updateLayout for container type %s",getType());
            auto previousElement = context->setCurrentElement(this);
            auto strip = getStrip();
            int stripPos = 0;
            int stripLength = strip->getLength();
            m_children.each([&](IScriptElement* child) {
                if (child->isPositionable()) {
                    m_logger->never("\tupdate positionable child");
                    IElementPosition* pos = child->getPosition();
                    pos->updateValues(context);
                    pos->setParent(getPosition());
                    IScriptHSLStrip* parent = pos->isPositionAbsolute() ? strip->getRoot() : strip;
                    child->getStrip()->setParent(parent);
                    if (pos->isCover()){
                        m_logger->never("cover %d",parent->getLength());
                        pos->setPosition(0,parent->getLength(),context); 
                    } else {
                        m_logger->never("\tgot IElementPosition %x",pos);
                        int offset = 0;
                        if (pos->hasOffset()) {
                            offset = pos->getOffset();
                            m_logger->never("\tgot offset %d",offset);
                        }
                        int length = stripLength - stripPos - offset;
                        if (pos->hasLength()){
                            length = pos->getLength();
                            m_logger->never("\tgot length %d",length);
                        }
                        m_logger->never("Set element position %d %d",stripPos+offset,length);
                        int pixelOffset = pos->toPixels(offset);
                        int pixelLength = pos->toPixels(length);
                        if (pos->isCenter()) {
                            
                            int center = (parent->getLength()-pixelLength)/2;
                            m_logger->never("parent %d,  length %d, center %d",parent->getLength(),pixelLength,center);
                            pos->setPosition(center,pixelLength,context);
                            stripPos = center+pixelLength;
                        } else {
                            pos->setPosition(stripPos+pixelOffset,pixelLength,context);
                            if (pos->isFlow() && pos->hasLength()) {
                                stripPos += offset+pixelLength;
                            }
                        }
                    }
                } else {
                    m_logger->never("\tchild is not positionable");
                }
                child->updateLayout(context);
            });
            context->setCurrentElement(previousElement);
            m_logger->never("\position update done");
        };

        void draw(IScriptContext* context) override {
            auto previousElement = context->setCurrentElement(this);
            m_logger->always("draw container %s %d",m_type,m_position->getOverflow());
            m_strip->setOverflow(m_position->getOverflow());

            m_children.each([&](IScriptElement* child) {
                context->setCurrentElement(child);
                child->draw(context);
            });
            context->setCurrentElement(previousElement);
        };

        bool isPositionable()  const override { return true;}
        IElementPosition* getPosition() const override { 
            m_logger->never("return m_position  %x",m_position);
            return m_position;
        }
    
    protected:
        IElementPosition* m_position;

    protected:
        PtrList<IScriptElement*> m_children;
        Logger* m_logger;
    };

    class ScriptRootContainer : public ScriptContainer {
        public:
            ScriptRootContainer() : ScriptContainer(S_ROOT_CONTAINER,&m_rootStrip, &m_rootPosition) {
            }

            void setStrip(IHSLStrip*strip) {
                m_rootStrip.setHSLStrip(strip);

                m_rootPosition.setPosition(0,strip->getCount(),NULL);
            }

            virtual ~ScriptRootContainer() {
            }


            void updateLayout(IScriptContext* context) override {
                context->setStrip(&m_rootStrip);
                int length = m_rootStrip.getLength();
                m_position->setPosition(0,length,NULL);
                ScriptContainer::updateLayout(context);
            }
        private:
            RootHSLStrip m_rootStrip;
            RootElementPosition m_rootPosition;
    };


   
}
#endif