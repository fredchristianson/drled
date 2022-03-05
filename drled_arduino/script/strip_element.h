#ifndef STRIP_ELEMENT_H
#define STRIP_ELEMENT_H

// these ScriptElements add strips in the hierarchy to modify LED operations
#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../lib/util/list.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "./script_element.h"
#include "./script_container.h"
#include "../loggers.h"

namespace DevRelief
{
 
    
    class ModifiedHSLStrip : public ScriptHSLStrip {
        public:
            ModifiedHSLStrip() : ScriptHSLStrip() {
                SET_LOGGER(StripElementLogger);
            }

            virtual ~ModifiedHSLStrip() {

            }

        protected:


    };

    class MirrorStrip : public ScriptHSLStrip {
        public:
            MirrorStrip() : ScriptHSLStrip() {
                m_lastLed = 0;
            }

            virtual ~MirrorStrip() {

            }

            void updatePosition() { 
                int len = m_parentLength;
                if (m_position->hasLength()){
                    UnitValue uv = m_position->getLength();
                    LogIndent li(m_logger,"MirrorStrip.updatePosition",NEVER_LEVEL);
                    len = unitToPixel(uv);
                    m_logger->never("defined len %d  %d %3.3f",len,(int)uv.getUnit(),uv.getValue());
                }
                m_lastLed = len-1;
                m_length = len/2;
                m_logger->never("mirror last=%d len=%d",m_lastLed,m_length);
            }

            void setHue(int16_t hue,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { 
                    return;
                }
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                m_parent->setHue(hue,tidx,op);
                m_parent->setHue(hue,m_lastLed-tidx,top);
            }

            void setSaturation(int16_t saturation,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}            
                m_logger->never("ScriptHSLStrip.setSaturation op=%d",translateOp(op)); 
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                m_parent->setSaturation(saturation,tidx,op);
                m_parent->setSaturation(saturation,m_lastLed-tidx,top);
            }

            void setLightness(int16_t lightness,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                m_parent->setLightness(lightness,tidx,op);
                m_parent->setLightness(lightness,m_lastLed-tidx,top);
                
            }

            void setRGB(const CRGB& rgb,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                m_parent->setRGB(rgb,tidx,op);
                m_parent->setRGB(rgb,m_lastLed-tidx,top);
                
            }   


        protected:
            friend class MirrorElement;
            int m_lastLed;

    };

    class CopyStrip : public ScriptHSLStrip {
        public:
            CopyStrip() : ScriptHSLStrip() {
                m_count = 0;
                m_repeatOffset = 0;
            }

            virtual ~CopyStrip() {

            }

            void setCount(int count) { 
                m_count = count;
                if (m_count == 0) {
                    m_length = 0;
                    m_repeatOffset = 0;
                } else {
                    m_length = m_parentLength/m_count;
                    m_repeatOffset = m_count==0 ? 0  : m_length;
                    m_overflow = OVERFLOW_ALLOW;
                    m_logger->never("copy: %d %d %d %d",m_count,m_length,m_parentLength,m_repeatOffset);
                }
            }


            void setHue(int16_t hue,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { 
                    return;
                }
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                for(int i=0;i<m_count;i++) {
                    m_parent->setHue(hue,tidx+i*m_repeatOffset,top);
                }
            }

            void setSaturation(int16_t saturation,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}            
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                for(int i=0;i<m_count;i++) {
                    m_parent->setSaturation(saturation,tidx+i*m_repeatOffset,top);
                }
            }

            void setLightness(int16_t lightness,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                for(int i=0;i<m_count;i++) {
                    m_parent->setLightness(lightness,tidx+i*m_repeatOffset,top);
                }
            }

            void setRGB(const CRGB& rgb,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}            
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                for(int i=0;i<m_count;i++) {
                    m_parent->setRGB(rgb,tidx+i*m_repeatOffset,top);
                }                
            }              

        protected:
            friend class CopyElement;

            int m_count;
            int m_repeatOffset;

    };

    class MultiStrip : public ScriptHSLStrip {
        public:
            MultiStrip() : ScriptHSLStrip() {

            }

            virtual ~MultiStrip() {

            }

            void clear() { m_strips.clear();}
            void destroyAndClear() {
                m_strips.each([](IScriptHSLStrip* strip) { strip->destroy();});
                 m_strips.clear();
            }

            void add(IScriptHSLStrip*strip) { m_strips.add(strip);}
            int getSize() const { return m_strips.size();}
            IScriptHSLStrip* getStrip(int index) { return m_strips.get(index);}
            void setHue(int16_t hue,int index, HSLOperation op) override {
                m_logger->never("MultStrip.setHue %d %d",index,op);
                m_strips.each([&](IScriptHSLStrip*strip) { strip->setHue(hue,index,op);});
            }

            void setSaturation(int16_t saturation,int index, HSLOperation op) override {
                m_strips.each([&](IScriptHSLStrip*strip) { strip->setSaturation(saturation,index,op);});
            }

            void setLightness(int16_t lightness,int index, HSLOperation op) override {
                m_strips.each([&](IScriptHSLStrip*strip) { strip->setLightness(lightness,index,op);});
            }

            void setRGB(const CRGB& rgb,int index, HSLOperation op) override {
                m_strips.each([&](IScriptHSLStrip*strip) { strip->setRGB(rgb,index,op);});
            }  

        protected:
            friend class MirrorElement;
            friend class CopyElement;
           LinkedList<IScriptHSLStrip*> m_strips;
    };

    
    class StripElement : public ScriptContainer {
        public:
            StripElement(const char * type, IScriptHSLStrip* strip,ScriptContainer*parent) 
            : m_context(parent->getContext()), ScriptContainer(type,&m_context,strip, &m_elementPosition) {

            }

            void valuesToJson(JsonObject* json) const override {
                ScriptContainer::valuesToJson(json);
            }

            void valuesFromJson(JsonObject* json) override {
                ScriptContainer::valuesFromJson(json);

            }            

        protected:
            ScriptElementPosition m_elementPosition;
            ChildContext m_context;
    };

    /* every LED operation is drawn "normal" and mirrored around the center of the parent strip.
     * the length of the strip is 1/2 the parent length so %-based values only cover the original, not mirrored leds
     */
    class MirrorElement : public StripElement {
        public:
            MirrorElement(ScriptContainer*parent) : StripElement("mirror",&m_mirrorStrip,parent){

            }

            void valuesToJson(JsonObject* json) const override {
                StripElement::valuesToJson(json);
            }

            void valuesFromJson(JsonObject* json) override {
                StripElement::valuesFromJson(json);

            }    

            virtual bool beforeDrawChildren() { 
                m_mirrorStrip.updatePosition();
                return true;
            }                
        private:
            MirrorStrip  m_mirrorStrip;
    };

    /* the parent strip is divided into "m_count" sections that are copies the original.
        the length of this strip is (1/m_count X parent_length) calculated at the start of each step.  
        so percent-based values work on one section.
     */
    class CopyElement : public StripElement {
        public:
            CopyElement(ScriptContainer*parent) : StripElement("Copy",&m_copyStrip,parent){
                m_countValue = NULL;
                m_count = 1;
                m_logger->never("create CopyElement");
            }

            virtual ~CopyElement() { 
                if (m_countValue) { m_countValue->destroy();}
            }

            virtual bool beforeDrawChildren() { 
                if (m_countValue) {
                    m_count = m_countValue->getIntValue(getContext(),1);
                }
                m_copyStrip.setCount(m_count);
                return true;
            }

            void valuesToJson(JsonObject* json) const override {
                StripElement::valuesToJson(json);
                if (m_countValue) {
                    json->set("count",m_countValue->toJson(json->getRoot()));
                }
            }

            void valuesFromJson(JsonObject* json) override {
                StripElement::valuesFromJson(json);
                m_countValue = ScriptValue::create(json->getPropertyValue("count"));

            }            
        private:
            CopyStrip m_copyStrip;
            int m_count;
            IScriptValue* m_countValue;
    };

}



#endif