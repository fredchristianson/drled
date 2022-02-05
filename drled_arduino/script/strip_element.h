#ifndef STRIP_ELEMENT_H
#define STRIP_ELEMENT_H

// these ScriptElements add strips in the hierarchy to modify LED operations
#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../lib/util/list.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "./script_element.h"
#include "../loggers.h"

namespace DevRelief
{
 
    extern Logger StripElementLogger;

    class ModifiedHSLStrip : public ScriptHSLStrip {
        public:
            ModifiedHSLStrip() : ScriptHSLStrip() {
                m_logger = &StripElementLogger;
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

        protected:
            friend class MirrorElement;
            bool isPositionValid(int index) override { return true;}
            int translateIndex(int index) override {
                int translated = ScriptHSLStrip::translateIndex(index);
                //int length = m_parent->getLength();
                int length = getLength();
                return length - translated-1;
                m_logger->never("MirrorStrip.setHue index %d --> %d",index, -translated);
                return -translated;
            }
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
                m_logger->always("copy %d -> %d + %d",index,translated,m_offset);
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
                m_logger->never("MultStrip.setHue %d",index);
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

    
    class StripElement : public PositionableElement {
        public:
            StripElement(const char * type) : PositionableElement(type,&m_elementPosition) {

            }

        protected:
            ScriptElementPosition m_elementPosition;
    };

    class MirrorElement : public StripElement {
        public:
            MirrorElement() : StripElement("mirror"){

            }


            virtual void draw(IScriptContext*context) override {
                m_logger->never("MirrorElement.draw");
                m_logger->showMemory();
                
                IScriptHSLStrip* parentStrip = context->getStrip();

                m_elementPosition.evaluateValues(context);
                
                m_mirrorStrip.setParent(parentStrip);
                m_mirrorStrip.update(m_position,context);
                m_multiStrip.clear();
                m_multiStrip.add(&m_mirrorStrip);
                m_multiStrip.add(parentStrip);
                m_multiStrip.update(m_position,context);
                context->setStrip(&m_multiStrip);
                /*
                m_elementPosition.evaluateValues(context);
                context->setPosition(&m_elementPosition);
                m_logger->debug("ScriptLEDElement.draw()");
                DrawStrip strip(context,parentStrip,&m_elementPosition);
                strip.eachLED([&](IHSLStripLED& led) {
                    DrawLED* dl = (DrawLED*)&led;
                    m_logger->never("\tdraw child LED %d",dl->index()); 
                    drawLED(led);
                });
                */
                m_logger->never("\tdone MirrorElement.draw()");
                m_logger->showMemory();

            }


            void valuesToJson(JsonObject* json) const override {

            }

            void valuesFromJson(JsonObject* json) override {

            }            
        private:
            MirrorStrip  m_mirrorStrip;
            MultiStrip m_multiStrip;
    };

    class CopyElement : public StripElement {
        public:
            CopyElement() : StripElement("Copy"){
                m_countValue = NULL;
                m_count = 0;
                m_logger->always("create CopyElement");
            }

            virtual ~CopyElement() { 
                if (m_countValue) { m_countValue->destroy();}
                
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
                    strip->update(m_position,context);
                    strip->setCopyOffset(nextOffset);
                    strip->setParent(parentStrip);
                    nextOffset += copyOffset;
                }
                m_multiStrip.update(m_position,context);
                context->setStrip(&m_multiStrip);
                m_logger->never("\tdone copyElement.draw()");
                m_logger->showMemory();

            }


            void valuesToJson(JsonObject* json) const override {
                if (m_countValue) {
                    json->set("count",m_countValue->toJson(json->getRoot()));
                }
            }

            void valuesFromJson(JsonObject* json) override {
                m_countValue = ScriptValue::create(json->getPropertyValue("count"));

            }            
        private:
            MultiStrip m_multiStrip;
            int m_count;
            IScriptValue* m_countValue;
    };

}



#endif