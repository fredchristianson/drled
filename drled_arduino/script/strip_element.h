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

            }

            virtual ~MirrorStrip() {

            }

            void setHue(int16_t hue,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { 
                    return;
                }
                int tidx = translateIndex(index);
                HSLOperation top = translateOp(op);
                m_parent->setHue(hue,tidx,op);
                m_parent->setHue(hue,m_parent->getLength()-tidx,top);
            }

            void setSaturation(int16_t saturation,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}            
                m_logger->never("ScriptHSLStrip.setSaturation op=%d",translateOp(op)); 
                m_parent->setSaturation(saturation,translateIndex(index),translateOp(op));
            }

            void setLightness(int16_t lightness,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                
                m_parent->setLightness(lightness,translateIndex(index),translateOp(op));
            }

            void setRGB(const CRGB& rgb,int index, HSLOperation op) override {
                
                m_parent->setRGB(rgb,translateIndex(index),translateOp(op));
            }   


        protected:
            friend class MirrorElement;

    };

    class CopyStrip : public ScriptHSLStrip {
        public:
            CopyStrip() : ScriptHSLStrip() {
                m_offset = 0;
            }

            virtual ~CopyStrip() {

            }

            void setCopyOffset(int offset) { m_offset = offset;}

        protected:
            friend class CopyElement;
            bool isPositionValid(int index) override { return true;}
            int translateIndex(int index) override {
                int translated = ScriptHSLStrip::translateIndex(index);
                m_logger->never("copy %d -> %d + %d",index,translated,m_offset);
                return translated + m_offset;
            }
        
            int m_offset;
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
        private:
            MirrorStrip  m_mirrorStrip;
            MultiStrip m_multiStrip;
    };

    class CopyElement : public StripElement {
        public:
            CopyElement(ScriptContainer*parent) : StripElement("Copy",&m_multiStrip,parent){
                m_countValue = NULL;
                m_count = 0;
                m_logger->never("create CopyElement");
            }

            virtual ~CopyElement() { 
                if (m_countValue) { m_countValue->destroy();}
                m_multiStrip.destroyAndClear();
            }

            virtual void draw(IScriptContext*context) override {
                m_count = m_countValue ? m_countValue->getIntValue(context,2) : 2;
                m_logger->never("CopyElement.draw");
                
                IScriptHSLStrip* parentStrip = context->getStrip();

                m_elementPosition.evaluateValues(context);
                
                if (m_count != m_multiStrip.getSize()-1) {
                    m_multiStrip.destroyAndClear();
                }
                int copyOffset = parentStrip->getLength()/m_count;
                m_logger->never("create %d copies offset %d",m_count,copyOffset);
                if (m_count <2) { return;}
                int nextOffset = 0;
                for(int i=0;i<m_count;i++) {
                    CopyStrip* strip = (CopyStrip*)m_multiStrip.getStrip(i);
                    if (strip == NULL) {
                        strip = new CopyStrip();
                        m_multiStrip.add(strip);
                    }
                    strip->setParent(parentStrip);
                    strip->updatePosition(m_position,context);
                    strip->setCopyOffset(nextOffset);
                    strip->setParent(parentStrip);
                    nextOffset += copyOffset;
                }
                m_multiStrip.setParent(parentStrip);
                m_multiStrip.updatePosition(m_position,context);
                context->setStrip(&m_multiStrip);
                m_logger->never("\tdone copyElement.draw()");
                m_logger->showMemory();

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
            MultiStrip m_multiStrip;
            int m_count;
            IScriptValue* m_countValue;
    };

}



#endif