#ifndef DRSCRIPT_CONTAINER_H
#define DRSCRIPT_CONTAINER_H

#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../lib/util/list.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "./script_element.h"
#include "../loggers.h"

namespace DevRelief
{
 
    extern Logger ScriptContainerLogger;
    
    class ScriptContainer: public PositionableElement, public IScriptContainer
    {
    public:
        ScriptContainer(const char * type, IScriptHSLStrip* strip, IElementPosition* position) : PositionableElement(type, strip,position)
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
            m_logger->info("Parse elements array");
            m_children.clear();
            if (array == NULL) {
                m_logger->debug("no child element array");
                return;
            }
            ScriptElementCreator creator(this);
            array->each([&](IJsonElement* element) {
                m_logger->debug("\tgot child json");
                IScriptElement*child = creator.elementFromJson(element);
                m_logger->debug("\tcreated child %x",child);
                if (child) {
                    m_children.add(child);
                }
            });
        }

        void updateLayout(IScriptContext* context) override {
            m_logger->always("updateLayout for container type %s",getType());
            auto previousElement = context->setCurrentElement(this);
            auto strip = getStrip();
            int stripPos = 0;
            int stripLength = strip->getLength();
            m_children.each([&](IScriptElement* child) {
                if (child->isPositionable()) {
                    m_logger->always("\tupdate positionable child %s",child->getType());
                    IElementPosition* pos = child->getPosition();
                    m_logger->always("\tgot position %x",pos);
                    pos->evaluateValues(context);
                    pos->setParent(getPosition());
                    IScriptHSLStrip* parent = pos->isPositionAbsolute() ? strip->getRoot() : strip;
                    child->getStrip()->setParent(parent);
                    if (pos->isCover()){
                        m_logger->always("cover %d",parent->getLength());
                        pos->setPosition(0,parent->getLength(),context); 
                    } else {
                        m_logger->always("\tgot IElementPosition %x",pos);
                        int offset = 0;
                        if (pos->hasOffset()) {
                            offset = pos->getOffset();
                            m_logger->always("\tgot offset %d",offset);
                        }
                        int length = stripLength - stripPos - offset;
                        if (pos->hasLength()){
                            length = pos->getLength();
                            m_logger->always("\tgot length %d",length);
                        }
                        m_logger->always("Set element position %d %d",stripPos+offset,length);
                        int pixelOffset = pos->toPixels(offset);
                        int pixelLength = pos->toPixels(length);
                        if (pos->isCenter()) {
                            
                            int center = (parent->getLength()-pixelLength)/2;
                            m_logger->always("parent %d,  length %d, center %d",parent->getLength(),pixelLength,center);
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
                    m_logger->always("\tchild is not positionable");
                }
                child->updateLayout(context);
            });
            context->setCurrentElement(previousElement);
            m_logger->always("\position update done");
        };

        void draw(IScriptContext* context) override {
            auto previousElement = context->setCurrentElement(this);
            m_logger->debug("draw container %s %d",m_type,m_position->getOverflow());
            m_strip->setOverflow(m_position->getOverflow());

            m_children.each([&](IScriptElement* child) {
                context->setCurrentElement(child);
                child->draw(context);
            });
            context->setCurrentElement(previousElement);
        };

        bool isPositionable()  const override { return true;}
    
        void valuesFromJson(JsonObject* json) override {
            m_logger->debug("Load container json %s %s",getType(),json->toString().text());
            JsonArray* elements = json->getArray("elements");
            elementsFromJson(elements);    
        }    

        void valuesToJson(JsonObject* json) const override {
            m_logger->debug("ScriptContainer.valuesToJson %s",getType());

            JsonArray* elements = json->createArray("elements");
            m_children.each([&](IScriptElement*child) {
                if (child == NULL) {
                    m_logger->error("NULL child found");
                } else {
                    m_logger->debug("\tchild %s",child->getType());
                    JsonObject* childJson = elements->addNewObject();
                    child->toJson(childJson);
                    m_logger->debug("\tchild done %s",child->getType());
                }
            });
            m_logger->debug("\tdone ScriptContainer.valuesToJson %s",getType());
        }

    protected:
        PtrList<IScriptElement*> m_children;
        Logger* m_logger;
    };

    class ScriptRootContainer : public ScriptContainer {
        public:
            ScriptRootContainer() : ScriptContainer(S_ROOT_CONTAINER,&m_rootStrip, &m_rootPosition) {
                m_logger->info("Created ScriptRootContainer");
            }

            void setStrip(IHSLStrip*strip) {
                m_rootStrip.setHSLStrip(strip);

                m_rootPosition.setRootPosition(0,strip->getCount());
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

    class ScriptSegmentContainer : public ScriptContainer {
        public:
            ScriptSegmentContainer() : ScriptContainer(S_SEGMENT,&m_segmentStrip,&m_segmentPosition) {
                m_logger->info("create ScriptSegmentContainer");
            }

            virtual ~ScriptSegmentContainer() {
                m_logger->info("~ScriptSegmentContainer");
            }


        private:
            ContainerElementHSLStrip m_segmentStrip;
            ScriptElementPosition m_segmentPosition;
    };

    ScriptElementCreator::ScriptElementCreator(IScriptContainer* container) {
        m_container = container;
        m_logger = &ScriptElementLogger;
    }

    IScriptElement* ScriptElementCreator::elementFromJson(IJsonElement* json){
        m_logger->debug("parse Json type=%d",json->getType());
        JsonObject* obj = json ? json->asObject() : NULL;
        if (obj == NULL){
            m_logger->error("invalid IJsonElement to create script element");
            return NULL;
        }

        const char * type = obj->getString("type",NULL);
        IScriptElement* element = NULL;
        if (type == NULL) {
            m_logger->error("json is missing a type");
            return NULL;
        } else if (Util::equal(type,S_VALUES)) {
            element = new ValuesElement();
        } else if (Util::equalAny(type,S_HSL)) {
            element = new HSLElement();
        } else if (Util::equalAny(type,S_RHSL)) {
            element = new RainbowHSLElement();
        } else if (Util::equalAny(type,S_SEGMENT)) {
            element = new ScriptSegmentContainer();
        }
        if (element) {
            element->fromJson(obj);
            m_logger->debug("created element type %s",element->getType());
        }
        return element;
    }
}
#endif