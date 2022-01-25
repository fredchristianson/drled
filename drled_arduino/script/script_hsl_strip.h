#ifndef SCRIPT_HSL_STRIP_H
#define SCRIPT_HSL_STRIP_H

#include "../lib/led/led_strip.h"
#include "./script_interface.h"

namespace DevRelief{
    extern Logger ScriptHSLStripLogger;
    class ScriptHSLStrip : public IScriptHSLStrip {
        public:
            ScriptHSLStrip() {
                m_logger = &ScriptHSLStripLogger;
                m_offset = 0;
                m_length = 0;
                m_overflow = OVERFLOW_CLIP;
                m_parent = NULL;
                m_parentLength = 0;
                m_unit = POS_PERCENT;
            }

            virtual ~ScriptHSLStrip() {

            }

            int getOffset() override { return m_offset;}
            int getLength() override { return m_length;}

            void setParent(IScriptHSLStrip* parent) override { m_parent = parent;}
            IScriptHSLStrip* getParent() const override {  return m_parent;}

            void setHue(int16_t hue,int index, HSLOperation op) override {
                m_logger->debug("ScriptHSLStrip.setHue(%d,%d)",hue,index);
                if (!isPositionValid(index)) { 
                    m_logger->debug("Invalid index %d, %d",index,m_length);
                    return;
                }
                m_parent->setHue(hue,translateIndex(index),op);
            }

            void setSaturation(int16_t saturation,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                m_parent->setSaturation(saturation,translateIndex(index),op);
            }

            void setLightness(int16_t lightness,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                
                m_parent->setLightness(lightness,translateIndex(index),op);
            }

            void setRGB(const CRGB& rgb,int index, HSLOperation op) override {
                
                m_parent->setRGB(rgb,translateIndex(index),op);
            }  
        protected:

            int unitToPixel(const UnitValue& uv) {
                m_logger->debug("unitToPixel %f %d   %d",uv.getValue(),uv.getUnit(),m_parentLength);
                int val = uv.getValue();
                PositionUnit unit = uv.getUnit();
                if (unit == POS_INHERIT) {
                    unit = m_unit;
                }
                if (uv.getUnit() != POS_PERCENT) { return val;}
                return val/100.0*m_parentLength;
            }


            void update(IElementPosition * pos, IScriptContext* context) override  {
                m_logger->debug("ScriptHSLStrip.update %x %x",pos,context);
                m_unit = pos->getUnit();
                m_logger->debug("\tunit %d",m_unit);
                if (pos->isPositionAbsolute()) {
                    m_logger->always("getRootStrip()");
                    m_parent = context->getRootStrip();
                    m_logger->debug("\tgot root %x",m_parent);
                } else {
                    m_parent = context->getStrip();
                    m_logger->debug("\tgot parent %x",m_parent);
                }
                m_parentLength = m_parent->getLength();
                m_logger->debug("\tplen %d",m_parentLength);
                m_length = unitToPixel(pos->getLength());
                m_logger->debug("\len %d",m_length);
                m_offset = unitToPixel(pos->getOffset());
                m_logger->debug("\toffset %d",m_offset);
                m_overflow = pos->getOverflow();
                m_logger->debug("\toverflow %d",m_overflow);

            }

            virtual bool isPositionValid(int index) {
                if (m_parent == NULL || m_length <= 0) { return false;}
                if (m_overflow != OVERFLOW_CLIP) { return true;}
                return index >= 0 || index < m_length;
            }

            virtual int translateIndex(int index){
                int tidx = index+m_offset;
                if (m_overflow == OVERFLOW_WRAP) {
                    if (tidx<0) { tidx = m_length- (tidx%m_length);}
                    if (tidx>=m_length) { tidx = (tidx %m_length);}
                } else if (m_overflow == OVERFLOW_CLIP) {
                    // shouldn't happen since isPositionValid() was false if tidx out of range
                    if (tidx < 0) { tidx = 0;}
                    if (tidx >= m_length) {tidx = m_length-1;}
                }
                m_logger->debug("translated index  %d %d %d %d==>%d",m_offset, m_length, m_overflow, index,tidx);
                return tidx;
            }
            

            IScriptHSLStrip* m_parent;
            int m_parentLength;
            int m_length;
            int m_offset;
            PositionUnit m_unit;
            PositionOverflow m_overflow;
            Logger* m_logger;
    };

    class RootHSLStrip : public ScriptHSLStrip {
        public:
            RootHSLStrip( ) : ScriptHSLStrip(){
                m_base = NULL;
                m_offset = 0;
                m_overflow = OVERFLOW_WRAP;
                m_parentLength = 0;
            }

            virtual ~RootHSLStrip() {

            }


            void update(IElementPosition * pos, IScriptContext* context) override  {
                m_logger->debug("RootHSLStrip.update %x %x",pos,context);
                m_parentLength = m_length;
            }

            void setHue(int16_t hue,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}
                m_base->setHue(translateIndex(index),hue,op);
            }

            void setSaturation(int16_t saturation,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                m_base->setSaturation(translateIndex(index),saturation,op);
            }

            void setLightness(int16_t lightness,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                
                m_base->setLightness(translateIndex(index),lightness,op);
            }

            void setRGB(const CRGB& rgb,int index, HSLOperation op) override {
                
                m_base->setRGB(translateIndex(index),rgb,op);
            }  

            void setHSLStrip(IHSLStrip* base) {
                m_base = base;
                m_length = base ? base->getCount() : 0;
                m_offset = 0;
                m_parentLength = m_length;
            }


            virtual bool isPositionValid(int index) {
                return index >= 0 || index < m_length;
            }


        protected:
            IHSLStrip* m_base;
    };

    class ContainerElementHSLStrip : public ScriptHSLStrip {
        public:
            ContainerElementHSLStrip() : ScriptHSLStrip() {

            }

            virtual ~ContainerElementHSLStrip() {

            }

    };

    class DrawLED : public IHSLStripLED {
        public:
            DrawLED(IScriptHSLStrip* strip, IScriptContext*context,HSLOperation op) {
                m_strip = strip;
                m_context = context;
                m_operation = op;
            }

            void setIndex(int p) {
                m_index = p;
            }

            int index() { return m_index;}


            void setHue(int hue) override {
                m_strip->setHue(hue,m_index,m_operation);
            }
            void setSaturation(int saturation) override {
                m_strip->setSaturation(saturation,m_index,m_operation);
            }
            void setLightness(int lightness) override {
                m_strip->setLightness(lightness,m_index,m_operation);
            }
            void setRGB(const CRGB& rgb)  override {
                m_strip->setRGB(rgb,m_index,m_operation);
            }

            IScriptContext* getContext() const override { return m_context;}
            IScriptHSLStrip* getStrip() const override { return m_strip;}
        private:
            int m_index;
            HSLOperation m_operation;
            IScriptHSLStrip* m_strip;
            IScriptContext * m_context;
    };

    class DrawStrip : public ScriptHSLStrip {
        public:
            DrawStrip(IScriptContext* context, IScriptHSLStrip*parent, IElementPosition*position) : ScriptHSLStrip(){
                m_context = context;
                m_parent = parent;
                m_position = position;
                m_position->evaluateValues(context);
                update(position,context);
            }

            virtual ~DrawStrip() {

            }

            void eachLED(auto&& drawer) {

                HSLOperation op = m_position->getHSLOperation();
                DrawLED led(this,m_context,op);
                for(int i=0;i<m_length;i++){
                    led.setIndex(i);
                    drawer(led);
                }
            }



            // void setHue(int16_t hue, int position, HSLOperation op) { m_parent->setHue(hue,position+m_offset,op);}
            // void setSaturation(int16_t saturation, int position, HSLOperation op) { m_parent->setSaturation(saturation,position+m_offset,op);}
            // void setLightness(int16_t lightness, int position, HSLOperation op) { m_parent->setLightness(lightness,position+m_offset,op);}
            // void setRGB(const CRGB& rgb, int position, HSLOperation op) { m_parent->setRGB(rgb,position+m_offset,op);}


        private:
            IScriptContext* m_context;
            IElementPosition*m_position;


    };

    

}

#endif