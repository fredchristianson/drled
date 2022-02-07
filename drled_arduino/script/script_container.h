#ifndef DRSCRIPT_CONTAINER_H
#define DRSCRIPT_CONTAINER_H

#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../lib/util/list.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "./script_element.h"
#include "./strip_element.h"
#include "./script_context.h"
#include "../loggers.h"

namespace DevRelief
{
 
    extern Logger ScriptContainerLogger;
    
    class ScriptContainer: public PositionableElement, public IScriptContainer
    {
    public:
        ScriptContainer(const char * type, IScriptHSLStrip* strip, IElementPosition* position) : PositionableElement(type, position)
        {
            m_logger = &ScriptContainerLogger;
            m_logger->debug("Create ScriptContainer type: %s",m_type);
            m_strip = strip;
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
                    child->setParent(this);
                }
            });
        }

        void draw(IScriptContext* parentContext) override {
            m_logger->never("setCurrentElement %s %x",getType(),parentContext);
            ChildContext scope;
            scope.setParentContext(parentContext);
            auto previousElement = scope.setCurrentElement(this);
            
            m_logger->never("getPosition");
            IElementPosition*pos = getPosition();

            pos->evaluateValues(&scope);
            m_logger->never("update strip %x",m_strip);
         
            m_strip->update(pos,&scope);
            scope.setStrip(m_strip);
            m_logger->never("draw container %s %d",m_type,m_position->isReverse());

            m_children.each([&](IScriptElement* child) {
                m_logger->never("set current element  %x %s",child, child->getType());
                scope.setCurrentElement(child);
                m_logger->never("drawChild  %x",child);
                child->draw(&scope);

                m_logger->never("\tdone draw() child");
            });
            
            m_logger->never("finished draw %s",getType());
        };

        bool isPositionable()  const override { return true;}
    
        void valuesFromJson(JsonObject* json) override {
            PositionableElement::valuesFromJson(json);
            m_logger->never("Load container json %s %s",getType(),json->toString().text());
            m_logger->never("Containter %s reverse %d",getType(),m_position->isReverse());
            JsonArray* elements = json->getArray("elements");
            elementsFromJson(elements);    
        }    

        void valuesToJson(JsonObject* json) const override {
            PositionableElement::valuesToJson(json);
            m_logger->never("ScriptContainer.valuesToJson %s",getType());

            JsonArray* elements = json->createArray("elements");
            m_children.each([&](IScriptElement*child) {
                if (child == NULL) {
                    m_logger->error("NULL child found");
                } else {
                    m_logger->never("\tchild %s",child->getType());
                    JsonObject* childJson = elements->addNewObject();
                    child->toJson(childJson);
                    m_logger->never("\tchild done %s",child->getType());
                }
            });
            m_logger->never("\tdone ScriptContainer.valuesToJson %s",getType());
        }

    protected:
        PtrList<IScriptElement*> m_children;
        IScriptHSLStrip* m_strip;
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

    class MakerContext : public ChildContext {
        public:
            MakerContext(IScriptContext* ownerContext, ScriptValueList* values) : ChildContext("maker") {
                m_logger->showMemoryAlways("MakerContext::MakerContext()");
               m_valueList->initialize(values,ownerContext);
            }

            virtual ~MakerContext() {

            }
    };

    class MakerContainer : public ScriptContainer {
        public:
            MakerContainer() : ScriptContainer(S_SEGMENT,&m_segmentStrip,&m_segmentPosition) {
                m_logger->info("create MakerContainer");

            }

            virtual ~MakerContainer() {
                m_logger->info("~MakerContainer");
            }

            void valuesFromJson(JsonObject* json) override {
                ScriptContainer::valuesFromJson(json);
                m_countValue = ScriptValue::create(json->getPropertyValue("count"));
                JsonObject* obj = json->getChild("init");
                m_initValues.clear();
            
                if (obj) {
                    obj->eachProperty([&](const char * name, IJsonElement* jsonVal){
                        IScriptValue* val = ScriptValue::create(jsonVal);
                        if (val) {
                            m_initValues.setValue(name,val);
                        }
                    });
                }
            }    

            void valuesToJson(JsonObject* json) const override {
                ScriptContainer::valuesToJson(json);
                if (m_countValue) {
                    json->set("count",m_countValue->toJson(json->getRoot()));
                }
            }

            void draw(IScriptContext* parentContext) override {
                checkContextList(parentContext);
                m_contextList.each([&](MakerContext* ctx) {
                    ctx->setParentContext(parentContext);
                    ScriptContainer::draw(ctx);
                });
            }

        protected:
            void checkContextList(IScriptContext* parentContext) {
                int count = 1;
                if (m_countValue != NULL) {
                    count = m_countValue->getIntValue(parentContext,1);
                }
                m_logger->always("create MakerContexts");
                m_logger->showMemoryAlways();
                while(count > m_contextList.size()) {
                    m_logger->showMemoryAlways("\t\tcreate context");
                    m_logger->indent();
                    m_logger->indent();
                    MakerContext* mc = new MakerContext(parentContext,&m_initValues);
                    m_contextList.add(mc);
                    m_logger->outdent();
                    m_logger->outdent();
                    m_logger->showMemoryAlways("\t\tcreated");
                }
                m_logger->showMemoryAlways("\tcreated all contexts");
            }

        private:
            IScriptValue* m_countValue;
            ContainerElementHSLStrip m_segmentStrip;
            ScriptElementPosition m_segmentPosition;
            PtrList<MakerContext*> m_contextList;
            ScriptValueList m_initValues;
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
        if (type == NULL) {
            type = guessType(obj);
        }
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
        } else if (Util::equalAny(type,S_RGB)) {
            element = new RGBElement();
        } else if (Util::equalAny(type,S_SEGMENT)) {
            element = new ScriptSegmentContainer();
        }  else if (Util::equalAny(type,S_MAKER)) {
            element = new MakerContainer();
        } else if (Util::equalAny(type,S_MIRROR)) {
            element = new MirrorElement();
        } else if (Util::equalAny(type,S_COPY)) {
            element = new CopyElement();
        }
        if (element) {
            element->fromJson(obj);
            m_logger->debug("created element type %s %x",element->getType(),element);
        }
        return element;
    }

    const char * ScriptElementCreator::guessType(JsonObject* json){
        if (json->getPropertyValue("hue")||json->getPropertyValue("lightness")||json->getPropertyValue("saturation")){
            return S_RHSL;
        } else if (json->getPropertyValue("red")||json->getPropertyValue("blue")||json->getPropertyValue("green")){
            return S_RGB;
        } else if (json->getPropertyValue("elements")){
            return S_SEGMENT;
        } else {
            return S_VALUES;
        }
        return NULL;
    }
}
#endif