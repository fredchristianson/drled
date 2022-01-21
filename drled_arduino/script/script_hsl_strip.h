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
            }

            virtual ~ScriptHSLStrip() {

            }

            void setPosition(int index){
                m_position = index;

            }
            int getPosition() { return m_position;}
            
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
                if (hue<0 || m_parent == NULL) { return;}
                m_parent->setHue(hue,getPosition(),getOp());
            }

            void setSaturation(int16_t saturation) override {
                if (saturation<0 || m_parent == NULL) { return;}
                m_parent->setSaturation(saturation,getPosition(),getOp());
            }

            void setLightness(int16_t lightness) override {
                if (lightness<0 || m_parent == NULL) { return;}
                m_parent->setLightness(lightness,getPosition(),getOp());
            }

            void setRGB(const CRGB& rgb) override {
                if (m_parent == NULL) { return;}
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
                m_parent->setRGB(rgb);
            }   

            virtual void setParent(IScriptHSLStrip*parent) {
                m_parent = parent;
            }
        protected:

            int m_position;
            IScriptHSLStrip* m_parent;
            HSLOperation m_op;
            Logger* m_logger;
    };

    class RootHSLStrip : public ScriptHSLStrip {
        public:
            RootHSLStrip( ) : ScriptHSLStrip(){
                m_base = NULL;
            }

            ~RootHSLStrip() {

            }

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
                m_base->setHue(position,hue,op);
            }

            void setSaturation(int16_t saturation,int position, HSLOperation op) override {
                if (saturation<0) { return;}
                m_base->setSaturation(position,saturation,op);
            }

            void setLightness(int16_t lightness,int position, HSLOperation op) override {
                if (lightness<0) { return;}
                m_base->setLightness(position,lightness,op);
            }

            void setRGB(const CRGB& rgb) override {

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