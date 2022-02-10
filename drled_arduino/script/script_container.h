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
        ScriptContainer(const char * type,IScriptContext* context, IScriptHSLStrip* strip, IElementPosition* position) : PositionableElement(type, position)
        {
            m_logger = &ScriptContainerLogger;
            m_logger->debug("Create ScriptContainer type: %s",m_type);
            m_strip = strip;
            m_context = context;
        }

        virtual ~ScriptContainer() {
            m_logger->debug("delete ScriptContainer type: %s",m_type);
        }


        bool isContainer()const  override {
            return true;
        }

        void add(IScriptElement* child) {
            m_children.add(child);
        }

        const PtrList<IScriptElement*>& getChildren() const override { return m_children;}

        void draw(IScriptContext* parentContext) override {
            m_logger->debug("draw %x %s  parent: %x",this,getType(),parentContext);
            m_logger->debug("\tstrip %x,  parent-strip %x",m_strip,parentContext->getStrip());
            // use the container's strip at the start of draw().
            // children may replace (add to) the strip but only for this drawing
            m_logger->debug("setStrip %x",m_strip);
            m_context->setStrip(m_strip);
            
            m_logger->never("getPosition");
            // update position based on current parentContext values
            IElementPosition*pos = getPosition();
            m_context->setPosition(pos);
            updatePosition(parentContext->getPosition(),parentContext);

            auto parentStrip = parentContext->getStrip();
            m_logger->debug("parent %x len=%d",parentStrip,parentStrip->getLength());
            m_strip->setParent(parentContext->getStrip());
            m_strip->updatePosition(pos,m_context);
            drawChildren();
        }

        void drawChildren() override {
            m_logger->debug("draw children %x %s context %x strip: %x",this,getType(),m_context, m_context->getStrip());
            m_children.each([&](IScriptElement* child) {
                m_logger->debug("\tchild %s",child->getType());
                drawChild(m_context,child);
            });
        }

        void drawChild(IScriptContext* context, IScriptElement * child) {
            m_logger->debug("set current element  %x %s",child, child->getType());
            m_logger->indent();
            context->setCurrentElement(child);
            m_logger->debug("drawChild  %s",child->getType());
            auto pos = child->getPosition();
            if (pos) {
                m_logger->debug("update position");
                pos->setParent(getPosition());
                pos->evaluateValues(context);
            }

            child->draw(m_context);
            m_logger->outdent();
            m_logger->debug("\tdone draw() child");
            
        };

        IScriptContext* getContext()const { return m_context;}
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
        
        void elementsFromJson(JsonArray* array) override {
            m_logger->debug("Parse elements array");
            m_children.clear();
            if (array == NULL) {
                m_logger->debug("no child element array");
                return;
            }
            ScriptElementCreator creator(this);
            array->each([&](IJsonElement* element) {
                m_logger->debug("\tgot child json");
                IScriptElement*child = creator.elementFromJson(element,this);
                m_logger->debug("\tcreated child %x",child);
                if (child) {
                    m_children.add(child);
                }
            });
        }

        PtrList<IScriptElement*> m_children;
        IScriptHSLStrip* m_strip;
        IScriptContext* m_context;
        Logger* m_logger;
    };

    class ScriptRootContainer : public ScriptContainer {
        public:
            ScriptRootContainer() : ScriptContainer(S_ROOT_CONTAINER,&m_rootContext,&m_rootStrip, &m_rootPosition) {
                m_logger->info("Created ScriptRootContainer");
            }

            virtual ~ScriptRootContainer() {}
            void setStrip(IHSLStrip*strip) {
                m_rootStrip.setHSLStrip(strip);
                m_rootPosition.setRootPosition(0,strip->getCount());
            }

            void draw() { 
                m_logger->debug("ScriptRootContainer.draw %x %x %x",this,&m_rootContext,&m_rootStrip);
                m_logger->debug("\tlength=%d",m_rootStrip.getLength());
                m_rootContext.setStrip(&m_rootStrip);
                m_rootPosition.evaluateValues(&m_rootContext);
                m_rootStrip.updatePosition(&m_rootPosition,&m_rootContext);
                m_rootContext.beginStep();
                drawChildren();
                m_rootContext.endStep();
            }



            RootContext* getContext() { return &m_rootContext;}

            void setParams(JsonObject* params) {
                m_rootContext.setParams(params);
            }

        private:
            RootHSLStrip m_rootStrip;
            RootElementPosition m_rootPosition;
            RootContext m_rootContext;
    };

    class ScriptSegmentContainer : public ScriptContainer {
        public:
            ScriptSegmentContainer(IScriptContainer* parent) : m_context(parent->getContext()), ScriptContainer(S_SEGMENT,&m_context,&m_segmentStrip,&m_segmentPosition) {
                m_logger->info("create ScriptSegmentContainer");
                m_parent = parent;
                m_context.setParentContext(parent->getContext());
            }

            virtual ~ScriptSegmentContainer() {
                m_logger->info("~ScriptSegmentContainer");
            }


        private:
            IScriptContainer* m_parent;
            ContainerElementHSLStrip m_segmentStrip;
            ScriptElementPosition m_segmentPosition;
            ChildContext m_context;
    };

    class MakerContext : public ChildContext {
        public:
            MakerContext(IScriptContext* ownerContext, ScriptValueList* values) :  ChildContext(ownerContext,"maker") {
                m_logger->showMemory("MakerContext::MakerContext()");
                m_valueList->initialize(values,ownerContext);
               
            }

            virtual ~MakerContext() {

            }

            bool isComplete(int maxDuration) {
                if (maxDuration>0 && m_startTimeMsecs+maxDuration < millis()) {
                    return true;
                }
                return false;
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
        private:
            ScriptStep  m_currentStep;
            ScriptStep  m_lastStep;
    };

    class MakerContainer : public ScriptContainer {
        public:
            MakerContainer(IScriptContainer* parent) : m_context(parent->getContext()),ScriptContainer(S_SEGMENT,&m_context, &m_segmentStrip,&m_segmentPosition) {
                m_logger->info("create MakerContainer");
                m_countValue = NULL;
                m_minCountValue = NULL;
                m_maxCountValue = NULL;
                m_maxDurationMsecs = NULL;
                m_chancePerSecondValue = NULL;
                m_frequencyMsecsValue  = NULL;
                m_lastCreateMsecs = 0;
            }

            virtual ~MakerContainer() {
                m_logger->info("~MakerContainer");
                destroy(m_countValue);
                destroy(m_minCountValue);
                destroy(m_maxCountValue);
                destroy(m_maxDurationMsecs);
                destroy(m_chancePerSecondValue);
                destroy(m_frequencyMsecsValue );
            }

            void valuesFromJson(JsonObject* json) override {
                ScriptContainer::valuesFromJson(json);
                m_countValue = ScriptValue::create(json->getPropertyValue("count"));
                m_minCountValue = ScriptValue::create(json->getPropertyValue("min-count"));
                m_maxCountValue = ScriptValue::create(json->getPropertyValue("max-count"));
                m_maxDurationMsecs = ScriptValue::create(json->getPropertyValue("max-duration"));
                m_chancePerSecondValue = ScriptValue::create(json->getPropertyValue("chance-per-second"));
                m_frequencyMsecsValue  = ScriptValue::create(json->getPropertyValue("frequency-msecs"));
                if (!m_chancePerSecondValue) {
                    m_chancePerSecondValue = ScriptValue::create(json->getPropertyValue("chance"));
                }
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
                setJsonValue(json,"count",m_countValue);
                setJsonValue(json,"min-count",m_minCountValue);
                setJsonValue(json,"max-count",m_maxCountValue);
                setJsonValue(json,"chance-per-second",m_chancePerSecondValue);
                setJsonValue(json,"max-duration",m_maxDurationMsecs);
                setJsonValue(json,"frequency-msecs",m_maxDurationMsecs);
            }

            void drawChildren() override {
                m_logger->never("MakerContainer.draw");
                checkContextList();
                m_contextList.each([&](MakerContext* ctx) {
                    m_logger->never("\t\tupdate strip pos");
                    m_segmentPosition.evaluateValues(&m_context);
                    m_logger->never("\t\tupdate strip %x %x",ctx->getStrip(),&m_segmentStrip);
                    m_segmentStrip.updatePosition(&m_segmentPosition,&m_context);
                    ctx->setParentContext(&m_context);
                    ctx->beginStep();
                    m_children.each([&](IScriptElement* child) {
                        drawChild(ctx,child);
                    });
                    ctx->endStep();
                });
            }

        protected:
            void checkContextList() {
                IScriptContext* parentContext = &m_context;
                int minCount = 0;
                int maxCount = 1;
                int maxDuration = 0;

                if (m_minCountValue) {
                    minCount = m_minCountValue->getIntValue(parentContext,0);
                } else if (m_countValue) {
                    minCount = m_countValue->getIntValue(parentContext,0);
                }
                if (m_maxCountValue) {
                    maxCount = m_maxCountValue->getIntValue(parentContext,0);
                } else if (m_countValue) {
                    maxCount = m_countValue->getIntValue(parentContext,0);
                }

                if (m_maxDurationMsecs) {
                    maxDuration = m_maxDurationMsecs->getIntValue(parentContext,0);
                }


                // remove any completed contexts
                int i=0;
                while(i<m_contextList.size()) {
                    MakerContext* ctx = m_contextList.get(i);
                    if (ctx->isComplete(maxDuration)) {
                        m_logger->never("remove complete");
                        m_contextList.removeAt(i);
                    } else {
                        i += 1;
                    }
                }

                if (maxCount > m_contextList.size() && shouldCreate(parentContext)){
                    m_logger->never("should create");
                    createContext(parentContext);
                }


                // remove any extra contexts if creating one by chance created too many.
                while(maxCount < m_contextList.size()) {
                    m_logger->never("remove too many");
                    m_contextList.removeAt(0);
                }

                m_logger->never("create MakerContexts");
                m_logger->showMemoryNever();

                // create new contexts if there are fewer than "min-count";
                while(minCount > m_contextList.size()) {
                    m_logger->never("create to min");

                    createContext(parentContext);
                }
                m_logger->showMemoryNever("\tcreated all contexts");
            }

            void createContext(IScriptContext* parentContext) {
                // todo: make sure there is enough heap left.  keep track of heap needed to create previous contexts
                m_logger->never("create new context");
                MakerContext* mc = new MakerContext(parentContext,&m_initValues);
                m_logger->never("\tadd to list");
                m_contextList.add(mc);
                m_logger->never("\tadded to list");
                m_lastCreateMsecs = millis();
            }

            bool shouldCreate(IScriptContext* parentContext) {
                double chance = m_chancePerSecondValue ? m_chancePerSecondValue->getFloatValue(parentContext,0) : 0;
                double frequency = m_frequencyMsecsValue ? m_frequencyMsecsValue->getFloatValue(parentContext,0) : 0;
                long time = parentContext->getStep()->getMsecsSincePrev();
                if (chance != 0) {
                    double shouldPercent = 100*chance*time/1000.0;
                    int r = random(100);
                    if (r < shouldPercent) { 
                        m_logger->never("create %d < %f (%f)",r,shouldPercent,chance);
                        return true;
                    }
                } else if (frequency>0) {
                    time = millis();
                    if (m_lastCreateMsecs+frequency<time) {
                        m_logger->never("create frequency  %d %f %d",m_lastCreateMsecs,frequency,time);
                        return true;
                    }
                }
                return false;
            }
        private:
            IScriptValue* m_countValue;
            IScriptValue* m_minCountValue;
            IScriptValue* m_maxCountValue;
            IScriptValue* m_chancePerSecondValue;
            IScriptValue* m_frequencyMsecsValue;
            IScriptValue* m_maxDurationMsecs;
            ContainerElementHSLStrip m_segmentStrip;
            ScriptElementPosition m_segmentPosition;
            ChildContext m_context;
            PtrList<MakerContext*> m_contextList;
            ScriptValueList m_initValues;
            int m_lastCreateMsecs;
    };



    ScriptElementCreator::ScriptElementCreator(IScriptContainer* container) {
        m_container = container;
        m_logger = &ScriptElementLogger;
    }

    IScriptElement* ScriptElementCreator::elementFromJson(IJsonElement* json,ScriptContainer* container){
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
            element = new ScriptSegmentContainer(container);
        }  else if (Util::equalAny(type,S_MAKER)) {
            element = new MakerContainer(container);
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