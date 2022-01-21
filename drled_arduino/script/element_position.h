#ifndef ELEMENT_POSITION_H
#define ELEMENT_POSITION_H

#include "../lib/log/logger.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "./script_value.h"
#include "./json_names.h"

namespace DevRelief {
    extern Logger ScriptElementLogger;

    class RootElementPosition : public IElementPosition {
        public: 
            RootElementPosition() {
                m_clip = false;
                m_wrap = true;
                m_unit = POS_PERCENT;
                m_ledCount = 0;
            }

            virtual ~RootElementPosition() {

            }


            void destroy() override { delete this;}

            int getCount() const {
                return m_ledCount;
            }

            int getStart() const {
                return 0;
            }

            bool isCenter() const { return false;}
            bool isFlow() const { return false;}
            bool isCover() const { return true;}
            bool isPositionAbsolute() const  { return true;}
            bool isPositionRelative() const { return false;}
            bool isClip() const { return m_clip;}
            bool isWrap() const { return m_wrap;}

            /* the interface requires these, but the are not used for the root position */
            // offset is an IScriptValue from the script
            void setOffset(IScriptValue* value){}
            IScriptValue* getOffset() const { return NULL;}
            int evalOffset(IScriptContext* context){ return 0;}

            // length is an IScriptValue from the script;
            void setLength(IScriptValue* value){}
            IScriptValue* getLength() const {return NULL;}
            int evalLength(IScriptContext* context){return m_ledCount;}

            void setClip(IScriptValue* value){}
            IScriptValue* getClip() const {return NULL;}
            bool evalClip(IScriptContext* context){return m_clip;}

            void setWrap(IScriptValue* value){}
            IScriptValue* getWrap() const {return NULL;}
            int evalWrap(IScriptContext* context){return m_wrap;}            
            
            // unit (pixel or percent) and type come from the script. 
            void setUnit(PositionUnit unit) override { m_unit = unit;}
            IScriptValue* getUnit() const override { return NULL;}
            PositionUnit evalUnit(IScriptContext* context) const override  {
                 return m_unit;
            }

            void setStripNumber(IScriptValue* strip){}
            IScriptValue* getStripNumber() const { return NULL;}
            int evalStripNumber(IScriptContext* context) const { return -1;}

            void setType(int /*PositionType flags*/ type){}
            int /* PositionType flags */ getType() const { return POS_COVER;}

            // the layout sets start and count based on parent container and above values
            void setPosition(int start, int count,IScriptContext* context) override {}

            IElementPosition* getParent()const override {return NULL;};
            void setParent(IElementPosition*parent) {}
        protected:

            bool m_clip;
            bool m_wrap;
            PositionUnit m_unit;
            int m_ledCount;

    };

    class ScriptElementPosition : public IElementPosition {
        public:
            ScriptElementPosition() {
                m_logger = &ScriptLogger;
                m_offsetValue = NULL;
                m_lengthValue = NULL;
                m_clipValue = NULL;
                m_wrapValue = NULL;
                m_stripNumber = NULL;
                m_unit = POS_INHERIT;
                m_type = POS_RELATIVE | POS_FLOW;
                m_count = 0;
                m_start = 0;
                m_clip = false;
                m_wrap = false;
                m_parent = NULL;

            }

            ScriptElementPosition(JsonObject* json) {
                m_logger = &ScriptLogger;
                m_logger->never("get offset property");
                setOffset(ScriptValue::create(json->getPropertyValue("offset")));
                m_logger->never("get length property");
                setLength(ScriptValue::create(json->getPropertyValue("length")));
                m_logger->never("get unit property");
                setUnit(getUnit(json->getString("unit","percent")));
                setClip(ScriptValue::create(json->getPropertyValue("clip")));
                setWrap(ScriptValue::create(json->getPropertyValue("wrap")));
                m_logger->never("get position type property");
                setType(getPositionType(json));
                m_logger->never("get strip property");
                setStripNumber(getStripNumber(json));
                m_count = 0;
                m_start = 0;
            }

 

            virtual ~ScriptElementPosition() {
                if (m_offsetValue) { m_offsetValue->destroy();}
                if (m_lengthValue) { m_lengthValue->destroy();}
                if (m_stripNumber) { m_stripNumber->destroy();}
                if (m_stripNumber) { m_stripNumber->destroy();}
                if (m_stripNumber) { m_stripNumber->destroy();}
            }

            void destroy() override { delete this;}

            void toJson(JsonObject* json) {
                if (m_lengthValue) { 
                    json->set("length",m_lengthValue->toJson(json->getRoot()));
                }
                if (m_offsetValue) { 
                    json->set("offset",m_offsetValue->toJson(json->getRoot()));
                }
                if (m_stripNumber) { 
                    json->set("strip",m_stripNumber->toJson(json->getRoot()));
                }
                json->setString("unit", m_unit == POS_PERCENT? "percent": "pixel");
                json->setBool("center", isCenter());
                json->setBool("flow", isFlow());
                json->setBool("cover", isCover());
                json->setBool("absoute", isPositionAbsolute());
                json->setBool("relative", isPositionRelative());

            }


            bool isCenter() const { return (m_type & POS_CENTER) == POS_CENTER;}
            bool isFlow() const { return (m_type & POS_FLOW) == POS_FLOW;}
            bool isCover() const { return (m_type & POS_COVER) == POS_COVER;}
            bool isPositionAbsolute() const { return (m_type & POS_ABSOLUTE) == POS_ABSOLUTE;}
            bool isPositionRelative() const { return (m_type & POS_ABSOLUTE)!=POS_ABSOLUTE;}

            void setOffset(IScriptValue* value) override  { m_offsetValue = value;}
            IScriptValue* getOffset() const override  { return m_offsetValue;}
            int evalOffset(IScriptContext* context) override{
                return m_offsetValue ? m_offsetValue->getIntValue(context,0) : 0;

            }

            void setLength(IScriptValue* value) { m_lengthValue = value;}
            IScriptValue* getLength() const override { return m_lengthValue;}
            int evalLength(IScriptContext* context) override{
                return m_lengthValue ? m_lengthValue->getIntValue(context,100) : 0;

            }

            void setPosition(int start, int count,IScriptContext* context) override {
                if (getType() == POS_PERCENT && m_parent) {
                    int plength = m_parent->getCount();
                    m_start = start*0.01*plength;
                    m_count = count*0.01*plength;
                } else {
                    m_start = start;
                    m_count = count;
                }
            }

            void setUnit(PositionUnit unit) override { m_unit = unit;}
            PositionUnit getUnit() const override {
                if (m_unit == POS_INHERIT) {
                    return m_parent ? m_parent->getUnit() : POS_PIXEL;
                }
                return m_unit;
            }

            void setType(int  /* PositionType flags*/ type) override { m_type = type;}
            int /* PositionType flags */  getType() const override {
                return m_type;
            }

            void setStripNumber(IScriptValue* strip) {
                m_stripNumber = strip;
            }
            IScriptValue* getStripNumber() const {return m_stripNumber;}


            int getCount() const { return m_count;}
            int getStart() const { return m_start;}


            IElementPosition* getParent()const override {return m_parent;}
            void setParent(IElementPosition*parent) { m_parent = parent;}
        protected:
            IElementPosition* m_parent;
            IScriptValue* m_offsetValue;
            IScriptValue* m_lengthValue;
            IScriptValue* m_clipValue;
            IScriptValue* m_wrapValue;
            IScriptValue* m_stripNumber; // only if m_type has POS_STRIP bit set
            IScriptValue* m_wrapValue;
            IScriptValue* m_clipValue;
            PositionUnit    m_unit;

            int /*PositionType flags */    m_type;

            int m_count;
            int m_start;
            Logger* m_logger;

            IScriptValue* getStripNumber(JsonObject* json) {
                IJsonElement* propertyValue = json->getPropertyValue(S_STRIP);
                if (propertyValue == NULL) { 
                    return NULL;
                }
                return ScriptValue::create(propertyValue);
            }
            PositionUnit getUnit(const char * unitString){
                PositionUnit unit = POS_PERCENT;
                if (unitString) {
                    const char * text = unitString;
                    if (Util::equal(text,S_PIXEL)){
                        unit = POS_PIXEL;
                    } else if (Util::equal(text,S_PERCENT)){
                        unit = POS_PERCENT;
                    } else {
                        m_logger->error("Unknown unit type: %s",unitString);
                    }
                }
                return unit;
            }
            int /* PositionTypeFlags */ getPositionType(JsonObject* json){
                if (json == NULL) {
                    return POS_FLOW + POS_RELATIVE;
                }
                bool absolute = json->getBool("absolute",false);
                bool relative = json->getBool("relative",!absolute);
                bool cover = json->getBool("cover",false);
                bool center = !cover && json->getBool("center",false);
                bool flow = !cover && !center &&  json->getBool("flow",true);                
                bool strip = json->getProperty("strip") != NULL;
                int flags = (absolute ? POS_ABSOLUTE : 0)
                    +   (center? POS_CENTER : 0)
                    +   (flow ?  POS_FLOW : 0)
                    +   (cover ? POS_COVER : 0)
                    +   (strip ? POS_STRIP : 0);
                m_logger->debug("PositionType: 0x%x",flags);
                return flags;
            }
    };
};

#endif
