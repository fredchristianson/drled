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
            
            // unit (pixel or percent) and type come from the script. 
            void setUnit(PositionUnit unit){ m_unit = unit;}
            PositionUnit getUnit() const { return m_unit;}

            void setStripNumber(IScriptValue* strip){}
            IScriptValue* getStripNumber() const { return NULL;}

            void setType(int /*PositionType flags*/ type){}
            int /* PositionType flags */ getType() const { return POS_COVER;}

            // the layout sets start and count based on parent container and above values
            void setPosition(int start, int count,IScriptContext* context){}


        private:

            bool m_clip;
            bool m_wrap;
            PositionUnit m_unit;
            int m_ledCount;

    };

    class ScriptElementPosition : public IElementPosition {
        public:
            ScriptElementPosition() {
                m_logger = &ScriptLogger;
                m_offset = NULL;
                m_length = NULL;
                m_stripNumber = NULL;
                m_unit = POS_INHERIT;
                m_type = POS_RELATIVE | POS_FLOW;
                m_count = 0;
                m_start = 0;
            }

            ScriptElementPosition(JsonObject* json) {
                m_logger = &ScriptLogger;
                m_logger->debug("get offset property");
                setOffset(ScriptValue::create(json->getPropertyValue("offset")));
                m_logger->debug("get length property");
                setLength(ScriptValue::create(json->getPropertyValue("length")));
                m_logger->debug("get unit property");
                setUnit(getUnit(json->getString("unit","percent")));
                m_logger->debug("get position type property");
                setType(getPositionType(json));
                m_logger->debug("get strip property");
                setStripNumber(getStripNumber(json));
                m_count = 0;
                m_start = 0;
            }

 

            virtual ~ScriptElementPosition() {
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


            int getCount() const { return m_count;}
            int getStart() const { return m_start;}

            bool isClip() const { return false;}
            bool isWrap() const { return false;}

        protected:

            IScriptValue* m_offset;
            IScriptValue* m_length;
            IScriptValue* m_stripNumber; // only if m_type has POS_STRIP bit set
            IScriptValue* m_wrap;
            IScriptValue* m_clip;
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
