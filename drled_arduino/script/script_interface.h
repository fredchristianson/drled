#ifndef SCRIPT_STATUS_H
#define SCRIPT_STATUS_H

#include "../lib/led/led_strip.h"

namespace DevRelief{
    typedef enum PositionUnit
    {
        POS_PERCENT = 0,
        POS_PIXEL = 1,
        POS_INHERIT = 2,
        POS_UNSET=999
    };

    typedef enum PositionOverflow {
        OVERFLOW_ALLOW=0,
        OVERFLOW_CLIP=1,
        OVERFLOW_WRAP=2
    };

    typedef enum ScriptStatus {
        SCRIPT_CREATED,
        SCRIPT_RUNNING,
        SCRIPT_COMPLETE,
        SCRIPT_ERROR,
        SCRIPT_PAUSED
    };

    typedef enum RunState {
        STATE_RUNNING,
        STATE_PAUSED,
        STATE_COMPLETE
    };

    class IScriptContext;
    class IScriptValue;
    class IScriptHSLStrip;
    class IScriptElement;
    class PositionDomain;
    class ScriptValueList;
    class ScriptContainer;

    class UnitValue {
        public:
            UnitValue(double value=0, PositionUnit unit=POS_UNSET) { m_value = value; m_unit = unit;}
            double getValue() const { return m_value;}
            PositionUnit getUnit() const { return m_unit;}
        private: 
            int m_value;
            PositionUnit m_unit;
    };

    class IElementPosition {
        public:
            virtual void destroy()=0;
            virtual void evaluateValues(IScriptContext*context)=0;

            virtual bool hasOffset() const=0;
            virtual UnitValue getOffset() const=0;
            virtual bool hasLength() const=0;
            virtual UnitValue getLength() const=0;
    
            virtual UnitValue getGap() const=0;

            virtual PositionUnit getUnit() const=0;
            virtual HSLOperation getHSLOperation() const=0;
            virtual bool isCenter() const=0;
            virtual bool isFlow() const=0;
            virtual bool isCover() const=0;
            virtual bool isPositionAbsolute() const=0;
            virtual bool isPositionRelative() const=0;
            virtual bool isClip() const=0;
            virtual bool isWrap() const=0;
            virtual bool isReverse() const=0;
            virtual PositionOverflow getOverflow() const=0;

            // parent is used for inherited values
            virtual IElementPosition* getParent()const=0;
            virtual void setParent(IElementPosition*parent)=0;
            
            virtual bool toJson(JsonObject* json) const=0;
            virtual bool fromJson(JsonObject* json)=0;

        protected:

    };


    class IScriptStep {
        public:
            virtual void destroy()=0;
            virtual int getNumber()=0;
            virtual long getStartMsecs()=0;
            virtual long getMsecsSincePrev()=0;
    };

    class IAnimationDomain {
        public:
            virtual void destroy()=0;
            virtual double getPercent()=0; // return current posion as % from min to max (0..1)
            virtual double getMin() const = 0;
            virtual double getMax() const= 0;
            virtual double getValue() const= 0;
            virtual double getDistance()const =0; // distance from min to max (max-min+1)
            virtual void update(IScriptContext* ctx)=0;
            virtual bool isTime() const = 0;
            virtual RunState getState() const =0;
            virtual bool toJson(JsonObject* json) const=0;
    };
    
    class IAnimationRange {
        public:
            virtual void destroy()=0;
            
            virtual double getMinValue()=0;
            virtual double getMaxValue()=0;
            virtual double getValue(double percent)=0;
            virtual double getDistance()=0; // distance from min to max (max-min+1)
            virtual bool unfold()=0;
            virtual void update(IScriptContext* ctx)=0;
            virtual double getDelayValue(IScriptContext* ctx)=0;
            virtual double getCompleteValue(IScriptContext* ctx)=0;
            virtual bool toJson(JsonObject* json) const=0;
    };

    class IAnimationEase {
        public:
            virtual void destroy()=0;
            virtual double calculate(double position) = 0;
            virtual void update(IScriptContext* ctx)=0;
            virtual bool toJson(JsonObject* json) const=0;
    };

    class IValueAnimator {
    public:
        virtual void destroy()=0;
        virtual double getRangeValue(IScriptContext* ctx)=0;
        virtual IValueAnimator* clone(IScriptContext* ctx)=0;
        virtual void update(IScriptContext* ctx)=0;
        virtual bool toJson(JsonObject* json) const=0;
    };

    class IHSLStripLED {
        public:
            virtual void setHue(int hue)=0;
            virtual void setSaturation(int saturation)=0;
            virtual void setLightness(int lightnext)=0;
            virtual void setRGB(const CRGB& rgb)=0;

            virtual IScriptContext* getContext() const=0;
            virtual IScriptHSLStrip* getStrip() const = 0;
            virtual int getIndex()const=0;
    };
    
    
    class IScriptHSLStrip {
        public:
            virtual void destroy();
            virtual int getOffset()=0;
            virtual int getLength()=0;


            // only needed for class DrawStip
            //virtual void eachLED(IHSLLEDDrawer* drawer)=0;

            virtual void setHue(int16_t hue, int index, HSLOperation op)=0;
            virtual void setSaturation(int16_t saturation, int index, HSLOperation op)=0;
            virtual void setLightness(int16_t lightness, int index, HSLOperation op)=0;
            virtual void setRGB(const CRGB& rgb, int index, HSLOperation op)=0;

            virtual void updatePosition(IElementPosition * pos, IScriptContext* context)=0;

            virtual IScriptHSLStrip* getParent() const=0;
            virtual void setParent(IScriptHSLStrip* parent)=0;

            virtual int getFlowIndex() const=0;
            virtual void setFlowIndex(int index)=0;

    };

    class IScriptContext {
        public: 
            virtual void destroy()=0;

            virtual IScriptStep* getStep()=0;
            virtual IScriptStep* getLastStep()=0;

            virtual PositionDomain* getAnimationPositionDomain()=0;

            virtual void setValue(const char* name, IScriptValue* value)=0;
            virtual IScriptValue* getValue(const char * name)=0;
            virtual IScriptValue* getSysValue(const char * name)=0;

            virtual void setStrip(IScriptHSLStrip*strip)=0;
            virtual IScriptHSLStrip* getStrip() const = 0;
            virtual IScriptHSLStrip* getRootStrip() const = 0;

            virtual IScriptElement* getCurrentElement() const = 0;
            // set the current element and return previous one
            virtual IScriptElement* setCurrentElement(IScriptElement*element) = 0;

            virtual void setPosition(IElementPosition* position)=0;
            virtual IElementPosition* getPosition()const=0;

            virtual void setParentContext(IScriptContext* parent)=0;


    };

    class IScriptValue
    {
    public:
        virtual void destroy() =0; // cannot delete pure virtual interfaces. they must all implement destroy
        virtual int getIntValue(IScriptContext* cmd,  int defaultValue) = 0;         
        virtual double getFloatValue(IScriptContext* cmd,  double defaultValue) = 0; 
        virtual bool getBoolValue(IScriptContext* cmd,  bool defaultValue) = 0; 
        virtual int getMsecValue(IScriptContext* cmd,  int defaultValue) = 0; 
        virtual UnitValue getUnitValue(IScriptContext* cmd,  double defaultValue, PositionUnit defaultUnit)=0;

        // is...() methods return true if they are able to return that type of value
        virtual bool isString(IScriptContext* cmd)=0;
        virtual bool isNumber(IScriptContext* cmd)=0;
        virtual bool isBool(IScriptContext* cmd)=0;
        virtual bool isNull(IScriptContext* cmd)=0;
        virtual bool isUnitValue(IScriptContext* cmd)=0;

        // allow testing for value matches without evaluating nested string values
        virtual bool equals(IScriptContext*cmd, const char * match)=0;


        // evaluate this IScriptValue with the given command and return a new
        // IScriptValue.  
        // mainly useful to get a random number one time and copy to new value
        // functions calculate the value at the time eval is called.  other types return a copy
        virtual IScriptValue* eval(IScriptContext*cmd, double defaultValue=0)=0; 


        virtual bool isRecursing() = 0; // mainly for evaluating variable values

        // used to generate JSON text to save or return to value
        virtual IJsonElement* toJson(JsonRoot* jsonRoot)=0;
        
        // stringify to value that can be used in a variable's default 
        virtual DRString stringify()=0;
        
        // for debugging
        virtual DRString toString() = 0;

    };

    class IScriptValueProvider
    {
    public:
        virtual void destroy() =0; // cannot delete pure virtual interfaces. they must all implement destroy
        virtual bool hasValue(const char *name) = 0;
        virtual IScriptValue *getValue(const char *name) = 0;
        virtual void setValue(const char *name, IScriptValue*val)=0;
        virtual void initialize(ScriptValueList* source, IScriptContext* context)=0;
        virtual void clear()=0;
    };



    class IScriptElement {
        public:
            virtual void destroy()=0;
            virtual bool isContainer() const =0;
            virtual const char* getType() const =0;
            virtual void toJson(JsonObject* json) const=0;
            virtual void fromJson(JsonObject* json)=0;

            virtual void draw(IScriptContext* context)=0;

            virtual bool isPositionable()const=0;
            virtual IElementPosition* getPosition() const =0;
            virtual void updatePosition(IElementPosition* parentPosition, IScriptContext* parentContext)=0;
            
    };

    class IScriptContainer {
        public:
            virtual void elementsFromJson(JsonArray* json)=0;
            virtual const PtrList<IScriptElement*>& getChildren() const =0;
            virtual void drawChildren()=0;
            virtual IScriptContext* getContext()const = 0;
    };
    
    class ScriptElementCreator {
        public:
            ScriptElementCreator(IScriptContainer* container);

            IScriptElement* elementFromJson(IJsonElement* json, ScriptContainer* container);
        protected:
            const char * guessType(JsonObject* json);
        private:
            IScriptContainer* m_container;
            DECLARE_LOGGER();
    };

    class ScriptPatternElement
    {
    public:
        ScriptPatternElement(IScriptValue* repeatCount, PositionUnit repeatUnit, IScriptValue* value)
        {
            m_value = value;
            m_repeatCount = repeatCount;
            m_pixelCount = 0;
        }
        virtual ~ScriptPatternElement()
        {
            if (m_value) {m_value->destroy();} 
            if (m_repeatCount) {m_repeatCount->destroy();}
        }

        void update(IScriptContext* ctx);

        int getPixelCount() const { return m_pixelCount;}
        IScriptValue* getValue() const { return m_value;}
        virtual void destroy() { delete this;}

        IJsonElement* toJson(JsonRoot* jsonRoot) { 
            JsonObject*obj = jsonRoot->createObject();
            if (m_repeatCount) {
                obj->set("count",m_repeatCount->toJson(jsonRoot));
            }
            obj->set("value",m_value ? m_value->toJson(jsonRoot) : new JsonNull(*jsonRoot));
            return obj;
        }
        DRString toString() {
            return m_value->toString();
        };

    private:
        IScriptValue *m_value;
        IScriptValue* m_repeatCount;
        int m_pixelCount;
    };

    typedef enum PatternExtend {
        REPEAT_PATTERN=0,
        STRETCH_PATTERN=1,
        NO_EXTEND=2
    };

    class StepWatcher {
        // this lets objects perform updates once for every step.  mainly useful for IScriptValue
        // instances that may update every step but are accessed every LED.
        public:
            StepWatcher() { m_stepNumber=-1;}
            bool isChanged(IScriptContext*ctx);

        private: 
            int m_stepNumber;


    };
};

#endif
