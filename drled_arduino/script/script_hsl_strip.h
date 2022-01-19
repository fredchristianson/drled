#ifndef SCRIPT_HSL_STRIP_H
#define SCRIPT_HSL_STRIP_H

#include "../lib/led/led_strip.h"
#include "./script_interface.h"

namespace DevRelief{
    extern Logger ScriptHSLStripLogger;
    class ScriptHSLStrip : public IScriptHSLStrip {
        public:
            ScriptHSLStrip(IScriptHSLStrip* parent) {
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

        protected:
            int m_position;
            IScriptHSLStrip* m_parent;
            HSLOperation m_op;
            Logger* m_logger;
    };

    class RootHSLStrip : public ScriptHSLStrip {
        public:
            RootHSLStrip(IHSLStrip* base) : ScriptHSLStrip(NULL){
                m_base = base;
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

            void setRGB(const CRGB& rgb) override {

            }

            int getLength() override {
                return m_base->getCount();
            }

        protected:
            IHSLStrip* m_base;
    };

}

#endif