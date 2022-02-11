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

            void destroy() { delete this;}

            int getOffset() override { return m_offset;}
            int getLength() override { return m_length;}

            void setParent(IScriptHSLStrip* parent) override { 
                if (parent == this) { return;}
                m_parent = parent;
            }
            IScriptHSLStrip* getParent() const override {  return m_parent;}

            void setHue(int16_t hue,int index, HSLOperation op) override {
                m_logger->debug("ScriptHSLStrip.setHue(%d,%d) strip=%x parent=%x op=%d",hue,index,this,m_parent,translateOp(op));
                if (!isPositionValid(index)) { 
                    m_logger->debug("Invalid index %d, %d",index,m_length);
                    return;
                }
                m_parent->setHue(hue,translateIndex(index),translateOp(op));
                m_logger->debug("hue set %x",this);
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
  
            int getFlowIndex() const { 
                return m_flowIndex;
            }
            void setFlowIndex(int index) { 
                UnitValue unitGap = m_position->getGap();
                int gap = unitToPixel(unitGap);
                m_flowIndex = index+gap;
                if (m_position && !m_position->hasLength()) {
                    if (m_position->isFlow() && m_parent) {
                        m_parent->setFlowIndex(m_flowIndex+m_offset);
                    }
                }
            }
       
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


            void updatePosition(IElementPosition * pos, IScriptContext* context) override  {
                m_logger->never("ScriptHSLStrip.update %x %x",pos,context);
                m_reverse = pos->isReverse();
                m_logger->never("\treverse %d",m_reverse);
                
                m_position = pos;
                m_flowIndex = 0; // update() called start start of draw().  begin re-flowing children at 0
                m_logger->never("\tgetUnit %x",pos);
                m_unit = pos->getUnit();
                m_logger->never("\tunit=%d",m_unit);
                if (pos->isPositionAbsolute()) {
                    m_logger->debug("getRootStrip()");
                    m_parent = context->getRootStrip();
                    m_logger->debug("\tgot root %x",m_parent);
                }
                m_parentLength = m_parent->getLength();
                m_logger->debug("\m_parentLength %d",m_parentLength);
                if (pos->isCover()) {
                    m_offset = 0;
                    m_length = m_parentLength;
                    m_logger->debug("\tcover %d %d",m_offset,m_length);
                } else {
                    m_length = pos->hasLength() ? unitToPixel(pos->getLength()) : m_parentLength;
                    m_offset = pos->hasOffset() ? unitToPixel(pos->getOffset()) : 0;
                    m_logger->debug("\len %d",m_length);
                    if (pos->isCenter()) {
                        int margin = (m_parentLength - m_length)/2;
                        m_offset += margin;
                        m_logger->debug("\tcenter %d %d",m_offset,m_length);
                    } else {
                        if (pos->isFlow()) {
                            m_offset += m_parent->getFlowIndex();
                            m_logger->debug("\tflow %d",m_offset);
                        } else {
                            m_logger->debug("\tno flow %d",m_offset);
                        }
                        m_logger->debug("\toffset/len %d/%d",m_offset,m_length);
                    }
                }
                m_overflow = pos->getOverflow();
                if (pos->isFlow()) {

                    m_parent->setFlowIndex(m_offset+m_length);
                }
                m_logger->debug("\tflow %d %d %d",m_offset,m_length,m_parent->getFlowIndex());

            }

            virtual bool isPositionValid(int index) {
                
                if (m_parent == NULL ) { return false;}
                if (m_overflow != OVERFLOW_CLIP) { return true;}
                return index >= 0 || index < m_length;
            }

            virtual int translateIndex(int index){
               
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
                if (m_reverse) {
                    tidx = m_length - tidx -1;
                } 
                return tidx;
            }
            
            virtual HSLOperation translateOp(HSLOperation op) {
                return op;
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


            void updatePosition(IElementPosition * pos, IScriptContext* context) override  {
                m_position = pos;
                m_flowIndex = 0; // update() called start start of draw().  begin re-flowing children at 0
                m_parentLength = m_base->getCount();

                m_logger->never("RootHSLStrip.update %x %x op=%d %x",pos,context,pos->getHSLOperation(),m_position);
                m_logger->debug("\tplen %d",m_parentLength);
                m_unit = pos->getUnit();
                if (pos->hasLength()) {
                    m_length = unitToPixel(pos->getLength());
                } else {
                    m_length = m_parentLength;
                }
                m_logger->debug("\len %d",m_length);
                if (pos->hasOffset()) {
                    m_offset = unitToPixel(pos->getOffset());
                } else {
                    m_offset = 0;
                }
                m_reverse = pos->isReverse();

                m_logger->debug("\toffset %d",m_offset);
                m_overflow = pos->getOverflow();
                m_logger->never("\toverflow %d",m_overflow);
                m_logger->debug("\toverflow=%d offset=%d length=%d unit=%d",m_overflow,m_offset,m_length,m_unit);
            }

            void setHue(int16_t hue,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}
                m_logger->write(index==0?NEVER:NEVER,"RootHSLStrip setHue %d  op=%d top=%d",index,op,translateOp(op));
                m_base->setHue(translateIndex(index),hue,translateOp(op));
            }

            void setSaturation(int16_t saturation,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}     
                m_logger->never("RootHSLStrip.setSaturation op=%d",translateOp(op));        
                m_base->setSaturation(translateIndex(index),saturation,translateOp(op));
            }

            void setLightness(int16_t lightness,int index, HSLOperation op) override {
                if (!isPositionValid(index)) { return;}             
                
                m_base->setLightness(translateIndex(index),lightness,translateOp(op));
            }

            void setRGB(const CRGB& rgb,int index, HSLOperation op) override {
                
                m_base->setRGB(translateIndex(index),rgb,translateOp(op));
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
            virtual HSLOperation translateOp(HSLOperation op) {
                m_logger->never("root translate %x op %d %d",m_position,op,m_position->getHSLOperation());
                HSLOperation pop = m_position ? m_position->getHSLOperation() : ADD;
                if (op == INHERIT || op == UNSET) {
                    return pop;
                }
                return op;
            }

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
                ScriptHSLStripLogger.never("DrawLED() op %d",m_operation);
            }

            void setIndex(int p) {
                m_index = p;
            }

            int index() { return m_index;}


            void setHue(int hue) override {
                ScriptHSLStripLogger.never("DrawLED.setHue op %d op=%d",m_index,m_operation);
                m_strip->setHue(hue,m_index,m_operation);
            }
            void setSaturation(int saturation) override {
                ScriptHSLStripLogger.never("DrawLED.setSaturation op %d",m_operation);
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
            virtual int getIndex()const {return m_index;}
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
                updatePosition(position,context);
            }

            virtual ~DrawStrip() {

            }

            void eachLED(auto&& drawer) {
                
                HSLOperation op = m_position->getHSLOperation();
                m_logger->never("eachLED HSL op: %d",op);
                DrawLED led(this,m_context,op);
                if (m_length == 0) {
                    return;
                }
                PositionDomain* domain = m_context->getAnimationPositionDomain();
                if (domain) { domain->setPosition(0,0,m_length-1); }
                int count = abs(m_length);
                int neg = m_length<0?-1 : 1;
                for(int i=0;i<count;i++){
                    if (domain) {domain->setPos(i);}
                    led.setIndex(i*neg);
                    drawer(led);
                }
            }



            // void setHue(int16_t hue, int position, HSLOperation op) { m_parent->setHue(hue,position+m_offset,translateOp(op));}
            // void setSaturation(int16_t saturation, int position, HSLOperation op) { m_parent->setSaturation(saturation,position+m_offset,translateOp(op));}
            // void setLightness(int16_t lightness, int position, HSLOperation op) { m_parent->setLightness(lightness,position+m_offset,translateOp(op));}
            // void setRGB(const CRGB& rgb, int position, HSLOperation op) { m_parent->setRGB(rgb,position+m_offset,translateOp(op));}


        private:
            IScriptContext* m_context;
            IElementPosition*m_position;


    };

    

}

#endif