#ifndef ELEMENT_POSITION_H
#define ELEMENT_POSITION_H

#include "../lib/log/logger.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "./script_value.h"
#include "./json_names.h"

namespace DevRelief {
    extern Logger ScriptLogger;

    class ElementPosition : public IElementPosition {
        public:
            ElementPosition() {
                m_logger = &ScriptLogger;
                m_offset = NULL;
                m_length = NULL;
                m_stripNumber = NULL;
                m_unit = POS_INHERIT;
                m_type = POS_RELATIVE | POS_FLOW;
                m_count = 0;
                m_start = 0;
            }

            ElementPosition(JsonObject* json) {
                setOffset(ScriptValue::create(json->getPropertyValue("offset")));
                setLength(ScriptValue::create(json->getPropertyValue("length")));
                setUnit(getUnit(json->getString("unit","percent")));
                setType(getPositionType(json));
                setStripNumber(getStripNumber(json));
            }

 

            ~ElementPosition() {
                if (m_offset) { m_offset->destroy();}
                if (m_length) { m_length->destroy();}
                if (m_stripNumber) { m_stripNumber->destroy();}
            }

            void destroy() override { delete this;}

            void toJson(JsonObject* json) {
                if (m_length) { 
                    json->set("length",m_length->toJson(json->getRoot()));
                }
                if (m_offset) { 
                    json->set("offset",m_offset->toJson(json->getRoot()));
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


            bool isCenter() const { return m_type & POS_CENTER;}
            bool isFlow() const { return m_type & POS_FLOW;}
            bool isCover() const { return m_type & POS_COVER;}
            bool isPositionAbsolute() const { return m_type & POS_ABSOLUTE;}
            bool isPositionRelative() const { return !(m_type & POS_ABSOLUTE);}

            void setOffset(IScriptValue* value) override  { m_offset = value;}
            IScriptValue* getOffset() const override  { return m_offset;}
            int evalOffset(IScriptContext* context) override{
                return m_offset ? m_offset->getIntValue(context,0) : 0;

            }

            void setLength(IScriptValue* value) { m_length = value;}
            IScriptValue* getLength() const override { return m_length;}
            int evalLength(IScriptContext* context) override{
                return m_length ? m_length->getIntValue(context,100) : 0;

            }

            void setPosition(int start, int count,IScriptContext* context) override {
                m_start = start;
                m_count = count;
            }

            void setUnit(PositionUnit unit) override { m_unit = unit;}
            PositionUnit getUnit() const override {
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


            int getCount() { return m_count;}
            int getStart() { return m_start;}

        protected:

            IScriptValue* m_offset;
            IScriptValue* m_length;
            IScriptValue* m_stripNumber; // only if m_type has POS_STRIP bit set
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
