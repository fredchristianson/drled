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

            void updateValues(IScriptContext* context) override {
                // ***REMOVED*** to update for the root position?
            }

            void destroy() override { delete this;}

            bool hasOffset() const override { return true;}
            int getOffset() const override {
                return 0;
            }

            bool hasLength() const { return true;}
            int getLength() const { return m_ledCount;}
            

            bool hasStrip() const override { return false;}
            int getStrip() const override { return -1;}

            PositionUnit getUnit() const override { return m_unit;}

            bool isCenter() const override { return false;}
            bool isFlow() const override { return false;}
            bool isCover() const override { return true;}
            bool isPositionAbsolute() const override  { return true;}
            bool isPositionRelative() const  override { return false;}
            bool isClip() const  override { return m_clip;}
            bool isWrap() const  override { return m_wrap;}

            // the layout sets start and count based on parent container and above values
            void setPosition(int start, int count,IScriptContext* context) override { m_ledCount=count;}
            int getStart() const override { return 0; }
            int getCount() const override { return m_ledCount;}
            IElementPosition* getParent()const override {return NULL;};
            void setParent(IElementPosition*parent) {}
            int toPixels(int val)const override { return val;}
            PositionOverflow getOverflow() const override { 
                return m_clip ? OVERFLOW_CLIP : m_wrap ? OVERFLOW_WRAP : OVERFLOW_ALLOW;
            }
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
                m_stripNumber = NULL;
                m_unit = POS_INHERIT;
                
                m_offset = 0;
                m_length = 0;
                m_clip = false;
                m_wrap = false;
                m_parent = NULL;

            }

            ScriptElementPosition(JsonObject* json) {
                m_logger = &ScriptLogger;
                m_offsetValue = ScriptValue::create(json->getPropertyValue("offset"));
                m_lengthValue = ScriptValue::create(json->getPropertyValue("length"));
                m_stripNumberValue = ScriptValue::create(json->getPropertyValue("strip"));

                m_clip = json->getBool("clip",false);
                m_wrap = json->getBool("wrap",false);
                m_absolute = json->getBool("absolute",false);
                m_cover = json->getBool("cover",false);
                m_center = !m_cover && json->getBool("center",false);
                m_flow = !m_cover && !m_center &&  json->getBool("flow",true);      
                auto unitJson = json->getPropertyValue("unit");
                if (unitJson) {
                    auto unitVal = unitJson->asValue();
                    if (unitVal) {
                        if (Util::equalAny(unitVal->getString(),"percent","%")) {
                            m_unit = POS_PERCENT;
                        } else if (Util::equalAny(unitVal->getString(),"pixel","px")) {
                            m_unit = POS_PIXEL;
                        } else {
                            m_unit = POS_INHERIT;
                        }
                        m_logger->debug("JSON unit %s ==> %d",unitVal->getString(),m_unit);
                    } else {
                        m_logger->debug("Not unit value in JSON");
                        m_unit = POS_PERCENT;
                    }
                }

                // length & offset are the script values 
                // may be percent or pixel and contant or range/variable/...
                m_length = 0;
                m_offset = 0;

                // start & count are the led pixel positions relative to the parent strip.
                // these are set by the container during layout
                m_start = 0;
                m_count = 0;
            }

 

            virtual ~ScriptElementPosition() {
                m_logger->always("destroy offset");
                if (m_offsetValue) { m_offsetValue->destroy();}
                m_logger->always("destroy length");
                if (m_lengthValue) { m_lengthValue->destroy();}
                m_logger->always("destroy stripNumber");
                if (m_stripNumberValue) { m_stripNumberValue->destroy();}
                m_logger->always("destroy done");
            }

            void destroy() override { delete this;}

            void toJson(JsonObject* json) const {
                m_logger->debug("ElementPostion.toJson");
                
                if (m_lengthValue) { 
                    m_logger->debug("\tlength");
                    json->set("length",m_lengthValue->toJson(json->getRoot()));
                }
                if (m_offsetValue) { 
                    m_logger->debug("\offset");
                    json->set("offset",m_offsetValue->toJson(json->getRoot()));
                }
                if (m_stripNumberValue) { 
                    m_logger->debug("\tstrip number");
                    json->set("strip",m_stripNumberValue->toJson(json->getRoot()));
                }

                m_logger->debug("\tunit %d",m_unit);

                json->setString("unit", m_unit == POS_PERCENT? "percent": m_unit == POS_PIXEL? "pixel" : "inherit");
                m_logger->debug("\tunit %d",m_wrap);
                json->setBool("wrap", isWrap());
                json->setBool("clip", isClip());
                json->setBool("center", isCenter());
                json->setBool("flow", isFlow());
                json->setBool("cover", isCover());
                m_logger->debug("\tabsolute");
                if (m_absolute){
                    m_logger->debug("\ttrue");
                    json->setBool("absoute", true);
                } else {
                    m_logger->debug("\tfalse");
                    json->setBool("relative", true);
                }
                m_logger->debug("\tdone");


            }

            void updateValues(IScriptContext* context) {
                if (m_offsetValue) { m_offset = m_offsetValue->getIntValue(context,0);}
                if (m_lengthValue) { m_length = m_lengthValue->getIntValue(context,0);}
                if (m_stripNumberValue) { m_stripNumber = m_stripNumberValue->getIntValue(context,0);}
            }

            PositionOverflow getOverflow() const override { 
                return m_clip ? OVERFLOW_CLIP : m_wrap ? OVERFLOW_WRAP : OVERFLOW_ALLOW;
            }

            bool isClip() const { return m_clip;}
            bool isWrap() const { return m_wrap;}
            bool isCenter() const { return m_center;}
            bool isFlow() const { return m_flow;}
            bool isCover() const { return m_cover;}
            bool isPositionAbsolute() const { m_absolute;}
            bool isPositionRelative() const { return !m_absolute;}

            bool hasOffset() const { return m_offsetValue != NULL;}
            int getOffset() const { return m_offset;}
            bool hasLength() const { return m_lengthValue != NULL;}
            int getLength() const { return m_length;}

            bool hasStrip()const  { return m_stripNumberValue != NULL;}
            int getStrip()const { return m_stripNumber;}

            // layout sets the position in pixel units
            void setPosition(int start, int count,IScriptContext* context) override {
                m_logger->never("setPosition %d %d",start,count);
                m_start = start;
                m_count = count;
            }

            int toPixels(int val)const override { 
                int rval = val;
                PositionUnit unit = getUnit();
                if (unit == POS_PERCENT && m_parent) {
                    rval = (m_parent->getCount())*(val/100.0);
                }
                m_logger->never("toPixel %d(%d) %d=>%d",unit,m_unit,val,rval);
                return rval;
            }

            PositionUnit getUnit() const override {
                if (m_unit == POS_INHERIT) {
                    return m_parent ? m_parent->getUnit() : POS_PERCENT;
                }
                return m_unit;
            }


            int getCount() const override { return m_count;}
            int getStart() const override { return m_start;}


            IElementPosition* getParent()const override {return m_parent;}
            void setParent(IElementPosition*parent) { m_parent = parent;}
        protected:
            IElementPosition* m_parent;

            // evaluatable values
            IScriptValue* m_offsetValue;
            IScriptValue* m_lengthValue;
            IScriptValue* m_stripNumberValue;
            
            // evaluated values
            int16_t m_offset;
            int16_t m_length;
            bool    m_clip;
            bool    m_wrap;
            uint8_t m_stripNumber;
            PositionUnit    m_unit;
            bool m_center;
            bool m_cover;
            bool m_flow;
            bool m_absolute;

            // the values after layout
            int m_start;
            int m_count;

            Logger* m_logger;

 

           
    };
};

#endif
