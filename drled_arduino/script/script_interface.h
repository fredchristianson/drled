#ifndef SCRIPT_STATUS_H
#define SCRIPT_STATUS_H

#include "../lib/led/led_strip.h"

namespace DevRelief{
    typedef enum PositionUnit
    {
        POS_PERCENT = 0,
        POS_PIXEL = 1,
        POS_INHERIT = 2
    };
    typedef enum PositionType
    {
        POS_RELATIVE = 0,
        POS_ABSOLUTE = 1,
        POS_AFTER = 3,
        POS_STRIP = 4
    };

    typedef enum ScriptStatus {
        SCRIPT_CREATED,
        SCRIPT_RUNNING,
        SCRIPT_COMPLETE,
        SCRIPT_ERROR,
        SCRIPT_PAUSED
    };

    class IScriptContext;
    class IScriptValue;

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
            virtual void update(IScriptContext*)=0;
            virtual double getPosition()=0;
    };
    
    class IAnimationRange {
        public:
            virtual void destroy()=0;
            virtual void setUnfolded(bool unfold)=0;
            virtual bool isUnfolded()=0;
            virtual double getHigh()=0;
            virtual double getLow()=0;
            virtual double getDistance()=0;
            virtual double getValue(double percent)=0;
    };

    class IScriptHSLStrip {
        public:
            virtual void setPosition(int index)=0;
            virtual int getPosition()=0;
            virtual int getLength()=0;
            virtual HSLOperation getOp()=0;
            virtual void setOp(HSLOperation op)=0;
            virtual void setHue(int16_t hue)=0;
            virtual void setSaturation(int16_t saturation)=0;
            virtual void setLightness(int16_t lightness)=0;
            virtual void setRGB(const CRGB& rgb)=0;
    };

    class IScriptContext {
        public: 
            virtual void destroy()=0;

            virtual IScriptHSLStrip* getStrip()=0;
            
            virtual IScriptStep* getStep()=0;
            virtual IScriptStep* beginStep();
            virtual void endStep();

            virtual IAnimationDomain* getAnimationPositionDomain()=0;

            virtual IScriptValue* getValue(const char * name)=0;
    };

    class IScriptValue
    {
    public:
        virtual void destroy() =0; // cannot delete pure virtual interfaces. they must all implement destroy
        virtual int getIntValue(IScriptContext* cmd,  int defaultValue) = 0;         
        virtual double getFloatValue(IScriptContext* cmd,  double defaultValue) = 0; 
        virtual bool getBoolValue(IScriptContext* cmd,  bool defaultValue) = 0; 
        virtual int getMsecValue(IScriptContext* cmd,  int defaultValue) = 0; 

        virtual bool isString(IScriptContext* cmd)=0;
        virtual bool isNumber(IScriptContext* cmd)=0;
        virtual bool isBool(IScriptContext* cmd)=0;
        virtual bool isNull(IScriptContext* cmd)=0;

        virtual bool equals(IScriptContext*cmd, const char * match)=0;


        // evaluate this IScriptValue with the given command and return a new
        // IScriptValue.  mainly useful to get a random number one time
        virtual IScriptValue* eval(IScriptContext*cmd, double defaultValue)=0; 


        virtual bool isRecursing() = 0; // mainly for evaluating variable values

        virtual IJsonElement* toJson(JsonRoot* jsonRoot)=0;
        // for debugging
        virtual DRString toString() = 0;

    };

    class IScriptValueProvider
    {
    public:
        virtual void destroy() =0; // cannot delete pure virtual interfaces. they must all implement destroy
        virtual bool hasValue(const char *name) = 0;
        virtual IScriptValue *getValue(const char *name) = 0;
    };

    class IValueAnimator {
        public:
            virtual void destroy()=0;
            virtual double get(IScriptContext* ctx, IAnimationRange* range)=0;
            virtual IValueAnimator* clone(IScriptContext* ctx)=0;
            virtual IAnimationDomain* getDomain(IScriptContext* ctx,IAnimationRange*range)=0;
    };

    class IScriptElement {
        public:
            virtual void destroy()=0;
            virtual bool isContainer() const =0;
            virtual const char* getType()=0;
            virtual void toJson(JsonObject* json)=0;
            virtual void fromJson(JsonObject* json)=0;

            virtual void updateLayout(IScriptContext* context)=0;
            virtual void draw(IScriptContext* context)=0;
    };

    class IScriptContainer {
        public:
            virtual void elementsFromJson(JsonArray* json)=0;
            virtual const PtrList<IScriptElement*>& getChildren() const =0;
    };
    

};

#endif