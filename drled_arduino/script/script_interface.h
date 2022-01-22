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

    class IScriptContext;
    class IScriptValue;
    class IScriptHSLStrip;
    class IScriptElement;



    class IElementPosition {
        public:
            virtual void destroy()=0;
            virtual void updateValues(IScriptContext*context)=0;

            virtual bool hasOffset() const=0;
            virtual int getOffset() const=0;
            virtual bool hasLength() const=0;
            virtual int getLength() const=0;

            virtual bool hasStrip()const =0;
            virtual int getStrip()const=0;

            virtual PositionUnit getUnit() const=0;


            virtual bool isCenter() const=0;
            virtual bool isFlow() const=0;
            virtual bool isCover() const=0;
            virtual bool isPositionAbsolute() const=0;
            virtual bool isPositionRelative() const=0;
            virtual bool isClip() const=0;
            virtual bool isWrap() const=0;
            virtual PositionOverflow getOverflow() const=0;
            virtual IElementPosition* getParent()const=0;
            virtual void setParent(IElementPosition*parent)=0;
            
            // the layout sets start and count based on parent container and above values
            virtual void setPosition(int start, int count,IScriptContext* context)=0;
            virtual int getStart()const =0;
            virtual int getCount()const =0;

            // convert val to pixel count if needed (for percent or inherited unit)
            virtual int toPixels(int val)const=0;
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
        
            /* most clients should use the above metheds.  the methods below 
             * allow hierarchies of strips to position elements */
            virtual void setHue(int16_t hue, int position, HSLOperation op)=0;
            virtual void setSaturation(int16_t saturation, int position, HSLOperation op)=0;
            virtual void setLightness(int16_t lightness, int position, HSLOperation op)=0;
            virtual void setRGB(const CRGB& rgb, int position, HSLOperation op)=0;

            virtual void setParent(IScriptHSLStrip*parent)=0;
            virtual PositionOverflow setOverflow(PositionOverflow overflow)=0;
            virtual IScriptHSLStrip* getRoot()=0;
    };

    class IScriptContext {
        public: 
            virtual void destroy()=0;

            virtual IScriptStep* getStep()=0;
            virtual IScriptStep* getLastStep()=0;

            virtual IScriptStep* beginStep();
            virtual void endStep();

            virtual IAnimationDomain* getAnimationPositionDomain()=0;

            virtual IScriptValue* getValue(const char * name)=0;

            virtual void setStrip(IScriptHSLStrip*strip)=0;
            virtual IScriptHSLStrip* getStrip() const = 0;
            virtual IScriptElement* getCurrentElement() const = 0;
            // set the current element and return previous one
            virtual IScriptElement* setCurrentElement(IScriptElement*element) = 0;
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

            virtual bool isPositionable()const=0;
            virtual IElementPosition* getPosition() const =0;

            virtual IScriptHSLStrip* getStrip() const = 0;
    };

    class IScriptContainer {
        public:
            virtual void elementsFromJson(JsonArray* json)=0;
            virtual const PtrList<IScriptElement*>& getChildren() const =0;
    };
    

};

#endif
