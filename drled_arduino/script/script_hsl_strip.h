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
                m_flowIndex = 0;
                m_position = NULL;
                m_reverse = false;
            }

            virtual ~ScriptHSLStrip() {

            }

            int getOffset() override { return m_offset;}
            int getLength() override { return m_length;}

            void setParent(IScriptHSLStrip* parent) override { m_parent = parent;}
            IScriptHSLStrip* getParent() const override {  return m_parent;}

            void setHue(int16_t hue,int index, HSLOperation op) override {
                m_logger->never("ScriptHSLStrip.setHue(%d,%d)",hue,index);
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
  
            int getFlowIndex() const { 
                return m_flowIndex;
            }
            void setFlowIndex(int index) { 
                m_flowIndex = index;
                if (m_position && !m_position->hasLength()) {
                    if (m_position->isFlow() && m_parent) {
                        m_parent->setFlowIndex(m_flowIndex+m_offset);
                    }
                }
            }
        protected:

  

            int unitToPixel(const UnitValue& uv) {
                m_logger->never("unitToPixel %f %d   %d",uv.getValue(),uv.getUnit(),m_parentLength);
                int val = uv.getValue();
                PositionUnit unit = uv.getUnit();
                if (unit == POS_INHERIT) {
                    m_logger->never("\tinherit %d",m_unit);
                    unit = m_unit;
                }
                if (unit != POS_PERCENT) { return val;}
                double pct = val/100.0*m_parentLength;
                m_logger->never("\tpct %f",pct);
                return pct;

            }


            void update(IElementPosition * pos, IScriptContext* context) override  {
                m_logger->never("ScriptHSLStrip.update %x %x",pos,context);
                m_reverse = pos->isReverse();
                m_logger->never("\treverse %d",m_reverse);
                
                m_position = pos;
                m_flowIndex = 0; // update() called start start of draw().  begin re-flowing children at 0
                m_unit = pos->getUnit();
                if (pos->isPositionAbsolute()) {
                    m_logger->never("getRootStrip()");
                    m_parent = context->getRootStrip();
                    m_logger->never("\tgot root %x",m_parent);
                } else {
                    m_parent = context->getStrip();
                    m_logger->never("\tgot parent %x",m_parent);
                }
                m_parentLength = m_parent->getLength();
                m_logger->never("\m_parentLength %d",m_parentLength);
                if (pos->isCover()) {
                    m_offset = 0;
                    m_length = m_parentLength;
                    m_logger->debug("\tcover %d %d",m_offset,m_length);
                } else {
                    m_length = pos->hasLength() ? unitToPixel(pos->getLength()) : m_parentLength;
                    m_offset = pos->hasOffset() ? unitToPixel(pos->getOffset()) : 0;
                    m_logger->never("\len %d",m_length);
                    if (pos->isCenter()) {
                        int margin = (m_parentLength - m_length)/2;
                        m_offset += margin;
                        m_logger->never("\tcenter %d %d",m_offset,m_length);
                    } else {
                        if (pos->isFlow()) {
                            m_offset += m_parent->getFlowIndex();
                            m_logger->never("\tflow %d",m_offset);
                        } else {
                            m_logger->never("\tno flow %d",m_offset);
                        }
                        m_logger->never("\toffset/len %d/%d",m_offset,m_length);
                    }
                }
                m_overflow = pos->getOverflow();
                m_parent->setFlowIndex(m_offset+m_length);
                m_logger->never("\tflow %d %d %d",m_offset,m_length,m_parent->getFlowIndex());

            }

            virtual bool isPositionValid(int index) {
                
                if (m_parent == NULL || m_length <= 0) { return false;}
                if (m_overflow != OVERFLOW_CLIP) { return true;}
                return index >= 0 || index < m_length;
            }

            virtual int translateIndex(int index){
                if (m_reverse) {
                    index = m_length - index-1;
                }                
                int tidx = index+m_offset;
                if (m_overflow == OVERFLOW_WRAP) {
                    if (index<0) { tidx = m_length- (index%m_length);}
                    else { tidx = m_offset + (index %m_length);}
                } else if (m_overflow == OVERFLOW_CLIP) {
                    // shouldn't happen since isPositionValid() was false if tidx out of range
                    if (tidx < m_offset) { tidx = m_offset;}
                    if (tidx >= m_offset+m_length) {tidx = m_offset+m_length-1;}
                }
                m_logger->never("translated index  %d %d %d %d==>%d",m_offset, m_length, m_overflow, index,tidx);

                return tidx;
            }
            

            IScriptHSLStrip* m_parent;
            IElementPosition* m_position;
            int m_parentLength;
            int m_length;
            int m_offset;
            int m_flowIndex;
            bool m_reverse;
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
                m_position = pos;
                m_flowIndex = 0; // update() called start start of draw().  begin re-flowing children at 0

                m_logger->debug("RootHSLStrip.update %x %x",pos,context);
                m_logger->debug("\tplen %d",m_parentLength);
                m_unit = pos->getUnit();
                m_length = unitToPixel(pos->getLength());
                m_logger->debug("\len %d",m_length);
                m_offset = unitToPixel(pos->getOffset());
                m_logger->debug("\toffset %d",m_offset);
                m_overflow = pos->getOverflow();
                m_logger->debug("\toverflow %d",m_overflow);
                m_parentLength = m_length;
                m_logger->debug("\toverflow=%d offset=%d length=%d unit=%d",m_overflow,m_offset,m_length,m_unit);
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
                if (m_length == 0) {
                    return;
                }
                PositionDomain* domain = m_context->getAnimationPositionDomain();
                if (domain) { domain->setPosition(0,0,m_length-1); }
                for(int i=0;i<m_length;i++){
                    if (domain) {domain->setPos(i);}
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