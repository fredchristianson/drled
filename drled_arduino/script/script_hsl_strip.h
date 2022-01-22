#ifndef SCRIPT_HSL_STRIP_H
#define SCRIPT_HSL_STRIP_H

#include "../lib/led/led_strip.h"
#include "./script_interface.h"

namespace DevRelief{
    extern Logger ScriptHSLStripLogger;
    class ScriptHSLStrip : public IScriptHSLStrip {
        public:
            ScriptHSLStrip(IScriptHSLStrip* parent=NULL) {
                m_logger = &ScriptHSLStripLogger;
                m_position = 0;
                m_parent = parent;
                m_op = INHERIT;
                m_overflow = OVERFLOW_ALLOW;
            }

            virtual ~ScriptHSLStrip() {

            }

            void setPosition(int index){
                m_logger->debug("position=%d",index);
                m_position = index;

            }

            virtual int getPosition() { 
                if (m_overflow == OVERFLOW_ALLOW || (m_position>=0 && m_position<getLength())){
                    m_logger->never("allow pos %d %d %d",m_overflow,m_position,getLength());
                    return m_position;
                }
                int len = getLength();
                if (m_overflow == OVERFLOW_WRAP && len > 0) { 
                    m_logger->never("wrap pos %d %d %d %d",m_overflow,m_position,getLength(),m_position%len);
                    return m_position % len;
                }
                if (m_overflow == OVERFLOW_CLIP) {
                    m_logger->never("clip pos %d %d %d %d",m_overflow,m_position,getLength(),m_position%len,m_position<0?0 : m_position>= len ? len-1:m_position);
                    return m_position<0?0 : m_position>= len ? len-1:m_position;
                }
                m_logger->never("getPosition %d %d",m_overflow,m_position);
                return m_position;
            }
            
            void setOp(HSLOperation op) override { m_op = op;}

            HSLOperation getOp() override {
                if (m_op == INHERIT) {
                    return m_parent ? m_parent->getOp() : REPLACE;
                }
                return m_op;
            }

            int getLength() override {
                return m_parent ? m_parent->getLength() : 0;
            }


            void setHue(int16_t hue) override {
                if (hue<0 || m_parent == NULL || invalidPosition()) { return;}
                m_parent->setHue(hue,getPosition(),getOp());
            }

            void setSaturation(int16_t saturation) override {
                if (saturation<0 || m_parent == NULL|| invalidPosition()) { return;}
                m_parent->setSaturation(saturation,getPosition(),getOp());
            }

            void setLightness(int16_t lightness) override {
                if (lightness<0 || m_parent == NULL|| invalidPosition()) { return;}
                m_parent->setLightness(lightness,getPosition(),getOp());
            }

            void setRGB(const CRGB& rgb) override {
                if (m_parent == NULL|| invalidPosition()) { return;}
                m_parent->setRGB(rgb);
            }            

            void setHue(int16_t hue,int position, HSLOperation op) override {
                if (hue<0) { return;}
                m_parent->setHue(hue,position,op);
            }

            void setSaturation(int16_t saturation,int position, HSLOperation op) override {
                if (saturation<0) { return;}
                m_parent->setSaturation(saturation);
            }

            void setLightness(int16_t lightness,int position, HSLOperation op) override {
                if (lightness<0) { return;}
                m_parent->setLightness(lightness);
            }

            void setRGB(const CRGB& rgb,int position, HSLOperation op) override {
                m_parent->setRGB(rgb,position,op);
            }   

            virtual void setParent(IScriptHSLStrip*parent) {
                m_parent = parent;
            }

            IScriptHSLStrip* getRoot()  override {
                if (m_parent) {
                    return m_parent->getRoot();
                }
                return NULL;
            }

            /* return previous value*/
            PositionOverflow setOverflow(PositionOverflow overflow) override {PositionOverflow prev = m_overflow; m_overflow = overflow; return prev;}
        protected:
            virtual bool invalidPosition() {
                if (m_overflow != OVERFLOW_CLIP) { return false;}
                return m_position < 0 || m_position >= getLength();
            }
            int m_position;
            IScriptHSLStrip* m_parent;
            HSLOperation m_op;
            Logger* m_logger;
            
            PositionOverflow m_overflow;
    };

    class RootHSLStrip : public ScriptHSLStrip {
        public:
            RootHSLStrip( ) : ScriptHSLStrip(){
                m_base = NULL;
            }

            virtual ~RootHSLStrip() {

            }

            IScriptHSLStrip* getRoot()  override { return this;}
            void setHue(int16_t hue) override {
                if (hue<0) { return;}
                m_base->setHue(getPosition(),hue,getOp());
            }

            void setSaturation(int16_t saturation) override {
                if (saturation<0) { return;}
                m_base->setSaturation(getPosition(),saturation,getOp());
            }

            void setLightness(int16_t lightness) override {
                if (lightness<0) { return;}
                m_base->setLightness(getPosition(),lightness,getOp());
            }

            
            void setHue(int16_t hue,int position, HSLOperation op) override {
                if (hue<0) { return;}
                setPosition(position);
                m_base->setHue(getPosition(),hue,op);
            }

            void setSaturation(int16_t saturation,int position, HSLOperation op) override {
                if (saturation<0) { return;}
                setPosition(position);                
                m_base->setSaturation(getPosition(),saturation,op);
            }

            void setLightness(int16_t lightness,int position, HSLOperation op) override {
                if (lightness<0) { return;}
                setPosition(position);
                m_base->setLightness(getPosition(),lightness,op);
            }

            void setRGB(const CRGB& rgb,int position, HSLOperation op) override {
                setPosition(position);
                m_base->setRGB(getPosition(),rgb,op);
            }  

            int getLength() override {
                return m_base->getCount();
            }

            void setHSLStrip(IHSLStrip* base) {
                m_base = base;
            }



        protected:
            IHSLStrip* m_base;
    };

    class ElementHSLStrip : public ScriptHSLStrip {
        public:
            ElementHSLStrip(){

            }

            virtual ~ElementHSLStrip() {

            }

    };

    
    class ContainerElementHSLStrip : public ScriptHSLStrip {
        public:
            ContainerElementHSLStrip(){

            }

            virtual ~ContainerElementHSLStrip() {

            }

    };

}

#endif