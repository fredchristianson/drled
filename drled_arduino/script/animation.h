#ifndef DR_ANIMATION_H
#define DR_ANIMATION_H

#include "../lib/log/logger.h"
#include "./script_interface.h"

namespace DevRelief
{
    extern Logger AnimationLogger;

    class AnimationRange : public IAnimationRange
    {
    public:
        AnimationRange(bool unfold=false){
            m_low = 0;
            m_high = 0;
            m_unfold = unfold;

        }
        AnimationRange(double low, double high, bool unfold=false)
        {
            m_low = low;
            m_high = high;
            m_logger = &AnimationLogger;
            m_logger->debug("create AnimationRange %f-%f  %s",low,high,unfold?"unfold":"");
            m_lastPosition = 99999999;
            m_lastValue = 0;
            m_unfold = unfold;
        }

        virtual ~AnimationRange() {

        }
        void destroy() override { delete this;}

        void update(IScriptContext* ctx) override { }

        bool unfold() { return m_unfold;}
        void setUnfold(bool unfold) { m_unfold = true;}

        double getValue(double position)
        {
            if (position == m_lastPosition) { return m_lastValue;}
            m_logger->never("AnimationRange.getValue(%f)  %f-%f",position,m_low,m_high);

            if (position <= 0 || m_high == m_low)
            {
                m_logger->never("\t return low %f",position,m_low);

                return m_low;
            }
            if (position >= 1)
            {
                m_logger->never("\treturn high %f",position,m_high);
                return m_high;
            }
            double diff = m_high - m_low;
            double value = m_low + position * diff;
            m_logger->never("\t %f  %f %f-%f",value,diff,m_low,m_high);
            m_lastPosition = position;
            m_lastValue = value;
            return value;
        }

        double getMinValue() override { return m_low; }
        double getMaxValue() override { return m_high; }
        double getDistance() { return (abs(m_high-m_low)+1);}
       
    protected:
        double m_lastPosition;
        double m_lastValue;
        double m_low;
        double m_high;
        bool m_unfold;
        Logger* m_logger;
    };


    class AnimationDomain : public IAnimationDomain
    {
    public:
        AnimationDomain()
        {
            m_logger = &AnimationLogger;
            m_min = 0;
            m_max = 0;
            m_pos = 0;
            m_logger->never("create AnimationDomain");
        }

        virtual ~AnimationDomain(){

        }

        void update(IScriptContext* ctx) override { }


        void destroy() override { delete this;}
   
        double getPercent() override {
            if (getMin() == getMax() || getMin()==getValue()) { return 0;}
            double distance = getMax()-getMin();
            double current = getValue()-getMin();
            m_logger->never("AnimationDomain %f %f %f",current,distance,current/distance);
            return current/distance;
        }

        void setPosition(double pos, double min, double max) {
            m_pos = pos;
            m_min = min;
            m_max = max;
        }
        void setPos(double p) { m_pos = p;}
        void setMin(double m) { m_min = m;}
        void setMax(double m) { m_max = m;}

        double getValue() const override
        {
            return (double)m_pos;
        }
        double getMax() const override
        {
            return (double)m_max;
        }
        double getMin() const override
        {
            return m_min;
        }

        double getDistance() const override { return abs(m_max-m_min)+1;}

    protected:
        Logger *m_logger;


        
    protected:
        double m_min;        
        double m_max;
        double m_pos;
    };

 

    class PositionDomain : public AnimationDomain
    {
    public:
        PositionDomain()
        {

        }


        void update(IScriptContext* ctx) override { 

        }

    private:

    };

    class ContextPositionDomain : public AnimationDomain
    {
    public:
        ContextPositionDomain()
        {

        }


        void update(IScriptContext* ctx) override { 
            IAnimationDomain*ctxpos = ctx->getAnimationPositionDomain();
            setMin(ctxpos->getMin());
            setMax(ctxpos->getMax());
            setPos(ctxpos->getValue());
        }

    private:

    };

    class TimeDomain : public AnimationDomain {
        public:
            TimeDomain(unsigned long startMSecs){
                m_startMsecs = startMSecs > 0 ? startMSecs : millis();
                m_durationMsecs = 0;
                setPosition(startMSecs,startMSecs,startMSecs);
                m_step = -1;
                m_pos = millis();
            }

            void setDuration(int durationMsecs){
                m_durationMsecs = durationMsecs;
                m_min = m_startMsecs;
                m_max = m_startMsecs + m_durationMsecs;
                while(m_max < m_pos) {
                    m_min += m_durationMsecs;
                    m_max += m_durationMsecs;
                }
            }

            void update(IScriptContext* ctx){
                m_logger->never("TimeDomainUpdate %d %d",m_step,ctx->getStep()->getNumber());
                if (m_step == ctx->getStep()->getNumber()) {
                    return;
                }
                m_step = ctx->getStep()->getNumber();
                m_pos = millis();
                if (m_pos > m_max) {
                    m_logger->never("repeat");
                    m_min = m_max;
                    m_max += m_durationMsecs;
                }
            }

        protected:
            int m_step;
            unsigned long m_startMsecs;
            unsigned long m_durationMsecs;

    };

    class SpeedDomain : public TimeDomain
    {  
        public:
            SpeedDomain(double stepsPerSecond, IAnimationRange* range) : TimeDomain(0){
                m_speedValue = NULL;
                m_range = range;
                m_speedStepsPerSecond = stepsPerSecond;
                setRange(range,stepsPerSecond);
            }
            SpeedDomain(IScriptValue* speedValue, IAnimationRange* range) : TimeDomain(0){
                m_speedValue = speedValue;
                m_range = range;
            }
            
            ~SpeedDomain() {
                if (m_speedValue) { m_speedValue->destroy();}
            }

            void setRange(IAnimationRange* range, double stepsPerSecond) {
                m_range = range;
                m_speedStepsPerSecond = stepsPerSecond;
                if (range != NULL && stepsPerSecond>0) {
                    setDuration(1000.0*range->getDistance()/stepsPerSecond);
                } else {
                    setDuration(0);
                }
            }

            void update(IScriptContext* ctx){
                if (m_speedValue) {
                    double val = m_speedValue->getFloatValue(ctx,0);
                    if (val > 0) {
                        setRange(m_range, val);
                    }                    
                }    
                TimeDomain::update(ctx);            
            }

        private: 
            IScriptValue* m_speedValue;
            double m_speedStepsPerSecond;
            IAnimationRange* m_range;
    };

    class DurationDomain : public TimeDomain
    {  
        public:
            DurationDomain(unsigned long startMsecs) :TimeDomain(startMsecs) {
                m_durationValue = NULL;
            }


            ~DurationDomain() {
                if (m_durationValue) { m_durationValue->destroy();}
            }

            DurationDomain(IScriptValue*value) :TimeDomain(0) {
                m_durationValue = value;
            }

            void update(IScriptContext* ctx){
                if (m_durationValue) {
                    double val = m_durationValue->getFloatValue(ctx,0);
                    setDuration(val);
                }                
                TimeDomain::update(ctx);
            }     
        private:
            IScriptValue* m_durationValue;                   
    };

    

    class AnimationEase : public IAnimationEase 
    {
    public:
        AnimationEase() {
            m_logger = &AnimationLogger;
        }

        void destroy() { delete this;}
        virtual double calculate(double position) = 0;
        void update(IScriptContext* ctx) override { }

    protected:
        Logger* m_logger;
    };

    class LinearEase : public AnimationEase
    {
    public:
        static LinearEase* INSTANCE;
        double calculate(double position)
        {
            m_logger->never("Linear ease %f",position);
            return position;
        }
    };

    LinearEase DefaultEase;
    LinearEase* LinearEase::INSTANCE = &DefaultEase;
    class CubicBezierEase : public AnimationEase
    {
    public:
        CubicBezierEase(double in=1, double out=1){
            m_in = in;
            m_out = out;
            m_inValue = NULL;
            m_outValue = NULL;
        }

        CubicBezierEase(IScriptValue* in, IScriptValue* out){
            m_in = 1;
            m_out = 1;
            m_inValue = in;
            m_outValue = out;
        }

        virtual ~CubicBezierEase() {
            if (m_inValue) { m_inValue->destroy();}
            if (m_outValue) { m_outValue->destroy();}
        }

        void update(IScriptContext* ctx) override {
            m_in = 1;
            m_out = 1;
            if (m_inValue) {
                m_in = (m_inValue->getFloatValue(ctx,1));
            } 
            if (m_outValue) {
                m_out = m_outValue->getFloatValue(ctx,0);
            }
         }

        void setValues(IScriptValue* in, IScriptValue* out= NULL) {
            if (m_inValue) { m_inValue->destroy();}
            if (m_outValue) { m_outValue->destroy();}
            m_inValue = in;
            m_outValue = out;
        }

        void setInValue(IScriptValue*in) {
            if (m_inValue) { m_inValue->destroy();}
            m_inValue = in;
        }

        void setOutValue(IScriptValue*out) {
            if (m_outValue) { m_outValue->destroy();}
            m_outValue = out;
        }

        void setValues(double in, double out) {
            m_in = in;
            m_out = out;
        }
        double calculate(double position)
        {
            double in = 1-m_in;
            double out = m_out;
            double val = 3 * pow(1 - position, 2) * position * in +
                            3 * (1 - position) * pow(position, 2) * out +
                            pow(position, 3);
            AnimationLogger.never("ease: %f==>%f  %f  %f",position,val,in,out);
            return val;
        }
    private:
        double m_in;
        double m_out;

        IScriptValue* m_inValue;
        IScriptValue* m_outValue;
        
    };


    class Animator : public IValueAnimator
    {
    public:
        Animator(IAnimationDomain* domain, IAnimationRange* range, IAnimationEase *ease = &DefaultEase) 
        {
            m_logger = &AnimationLogger;
            m_logger->debug("create Animator()");
            m_domain = domain;
            m_range = range;
            m_ease = ease;
            m_folding = true;
        }

        virtual ~Animator() {
            if (m_domain) { m_domain->destroy();}
            if (m_range) { m_range->destroy();}
            if (m_ease) { m_ease->destroy();}
        }

        void destroy() override{ delete this;}

        void update(IScriptContext* ctx) {
            if (m_domain) { m_domain->update(ctx);}
            if (m_range) { m_range->update(ctx);}
            if (m_ease) { m_ease->update(ctx);}
        }


        double getRangeValue(IScriptContext* ctx)
        {

            if (m_domain == NULL || m_range==NULL) {
                m_logger->error("Animator missing domain (%x) or range(%x)",m_domain,m_range);
                return 0;
            }
            double percent = m_domain->getPercent();
            m_logger->never("Animator percent %f",percent);
            double ease = m_ease ? m_ease->calculate(percent) : percent;
            m_logger->never("Animator ease %f",percent);

            if (m_range->unfold()) {
                m_logger->never("Animator unfold");
                if (ease<=0.5) {
                    m_logger->never("fold   %f %f",ease,ease*2);
                    if (!m_folding) {
                        ease = 0;  // on the switch from unfolding make sure 0 is returned 
                    } else {
                        ease=ease*2;
                    }
                    m_folding = true;
                } else {
                    m_logger->never("unfold %f %f",ease,(1-ease)*2);
                    if (m_folding) {
                        ease = 1;// on the switch from folding make sure 1 is returned 
                    } else {
                        ease= (1-ease)*2;
                    }
                    m_folding = false;
                }
            }
            double result = m_range->getValue(ease);
            m_logger->never("\tpos %f.  ease %f. result %f.  ",result);
            return result;
        };

        void setEase(IAnimationEase* ease) { m_ease = ease;}
        IAnimationEase* getEase() { return m_ease;}

        void setDomain(IAnimationDomain* domain) {
            m_domain = domain;
        }
        IAnimationDomain* getDomain() { return m_domain;}

        void setRange(IAnimationRange* range) { m_range=range;}
        IAnimationRange* getRange() { return m_range;}

        Animator* clone(IScriptContext* ctx) override { 
            m_logger->debug("not implemented Animator::clone");
            return this;
        }
    private:
        IAnimationDomain* m_domain;
        IAnimationRange*  m_range;
        IAnimationEase* m_ease;
        bool m_folding;
        Logger *m_logger;
    };

  


}

#endif