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
        AnimationRange(double low, double high, bool unfold=false)
        {
            m_low = low;
            m_high = high;
            m_unfold = unfold;
            m_logger = &AnimationLogger;
            m_logger->debug("create AnimationRange %f-%f  %s",low,high,unfold?"unfold":"");
            m_lastPosition = 99999999;
            m_lastValue = 0;
        }

        virtual ~AnimationRange() {

        }
        void destroy() override { delete this;}
        double getValue(double position)
        {
            if (position == m_lastPosition) { return m_lastValue;}
            m_logger->never("AnimationRange.getValue(%f)  %f-%f",position,m_low,m_high);
            if (m_unfold) {
                if (position<=0.5) {
                    position=position*2;
                } else {
                    position= (1-position)*2;
                }
            }
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
        double getDistance() { return (m_high-m_low) * (m_unfold ? 2 : 1);}
        void setUnfolded(bool unfold=true) { m_unfold = unfold;}
        bool isUnfolded() { return m_unfold;}
    private:
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

        void destroy() override { delete this;}
   
        double getPercent() override {
            if (getMin() == getMax() || getMin()==getValue()) { return 0;}
            double distance = getMax()-getMin();
            double current = getValue()-getMin();
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

    public:




    private:

    };

    class TimeDomain : public AnimationDomain {
        public:
            TimeDomain(unsigned long startMSecs){
                m_startMsecs = startMSecs;
                m_durationMsecs = 0;
                setPosition(startMSecs,startMSecs,startMSecs);
            }

            void setDuration(int durationMsecs){
                m_durationMsecs = durationMsecs;
                m_min = m_startMsecs;
                m_max = m_startMsecs + m_durationMsecs;
                m_pos = millis();
                while(m_max < m_pos) {
                    m_min += m_durationMsecs;
                    m_max += m_durationMsecs;
                }
            }

        protected:
            unsigned long m_startMsecs;
            unsigned long m_durationMsecs;

    };

    class SpeedDomain : public TimeDomain
    {  
        public:
            SpeedDomain(unsigned long startMsecs) : TimeDomain(startMsecs){

            }
            
            void setRange(AnimationRange* range, double stepsPerSecond) {
                setDuration(1000.0*range->getDistance()/stepsPerSecond);
            }
    };

    class DurationDomain : public TimeDomain
    {  
        public:
            DurationDomain(unsigned long startMsecs) :TimeDomain(startMsecs) {
            }
            
    };

    

    class AnimationEase
    {
    public:
        AnimationEase() {
            m_logger = &AnimationLogger;
        }
        virtual double calculate(double position) = 0;

    protected:
        Logger* m_logger;
    };

    class LinearEase : public AnimationEase
    {
    public:
        static LinearEase* INSTANCE;
        double calculate(double position)
        {
            m_logger->debug("Linear ease %f",position);
            return position;
        }
    };

    LinearEase DefaultEase;
    LinearEase* LinearEase::INSTANCE = &DefaultEase;
    class CubicBezierEase : public AnimationEase
    {
    public:
        CubicBezierEase(double in=0, double out=1){
            m_in = in;
            m_out = out;
        }

        void setValues(double in, double out) {
            m_in = in;
            m_out = out;
        }
        double calculate(double position)
        {

            double val = 3 * pow(1 - position, 2) * position * m_in +
                            3 * (1 - position) * pow(position, 2) * m_out +
                            pow(position, 3);
            AnimationLogger.never("ease: %f==>%f  %f  %f",position,val,m_in,m_out);
            return val;
        }
    private:
        double m_in;
        double m_out;
        
    };


    class Animator
    {
    public:
        Animator(IAnimationDomain* domain, IAnimationRange* range, AnimationEase *ease = &DefaultEase) 
        {
            m_logger = &AnimationLogger;
            m_logger->debug("create Animator()");
            m_domain = domain;
            m_range = range;
            m_ease = ease;
        }

        double getRangeValue()
        {
            if (m_ease == NULL) {
                m_ease = &DefaultEase;
            }
            double percent = m_domain->getPercent();
            double ease = m_ease->calculate(percent);
            double result = m_range->getValue(ease);
            m_logger->never("\tpos %f.  ease %f. result %f.  ",result);
            return result;
        };

        void setEase(AnimationEase* ease) { m_ease = ease;}
    private:
        IAnimationDomain* m_domain;
        IAnimationRange*  m_range;
        AnimationEase* m_ease;
        Logger *m_logger;
    };

  


}

#endif