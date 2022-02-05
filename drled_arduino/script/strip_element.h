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
                int length = m_parent->getLength();
                return length - translated;
                m_logger->never("MirrorStrip.setHue index %d --> %d",index, -translated);
                return -translated;
            }
    };

    class MultiStrip : public ScriptHSLStrip {
        public:
            MultiStrip() : ScriptHSLStrip() {

            }

            virtual ~MultiStrip() {

            }

            void clear() { m_strips.clear();}

            void add(IScriptHSLStrip*strip) { m_strips.add(strip);}

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

}



#endif