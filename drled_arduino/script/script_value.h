#ifndef DRSCRIPT_VALUE_H
#define DRSCRIPT_VALUE_H

#include "../lib/log/logger.h"
#include "../lib/util/drstring.h"
#include "../lib/util/util.h"
#include "../lib/util/list.h"
#include "../lib/led/led_strip.h"
#include "../lib/led/color.h"
#include "./script_interface.h"
#include "./animation.h"


namespace DevRelief
{
    extern Logger ScriptValueLogger;

    class ScriptValue : public IScriptValue {
        public:
            static ScriptValue* create(IJsonElement*json);
            ScriptValue() {
                m_logger = &ScriptValueLogger;
            }
            virtual ~ScriptValue() {
                m_logger->never("~ScriptValue");
            }

            void destroy() override { delete this;}

            bool isRecursing() override { return false;}

            bool isString(IScriptContext* ctx)  override{
              return false;  
            } 
            bool isNumber(IScriptContext* ctx)  override{
              return false;  
            } 
            bool isBool(IScriptContext* ctx)  override{
              return false;  
            } 

            bool isNull(IScriptContext* ctx)  override{
              return false;  
            } 

            bool isUnitValue(IScriptContext* ctx) override { return false;}
            UnitValue getUnitValue(IScriptContext* ctx,  double defaultValue, PositionUnit defaultUnit) override{
                UnitValue uv(getIntValue(ctx,defaultValue),defaultUnit);
                return uv;
            }

            bool equals(IScriptContext*ctx,const char * match) override { return false;}

            IScriptValue* eval(IScriptContext * ctx, double defaultValue=0) override;
            
            IJsonElement* toJson(JsonRoot* jsonRoot) override {
                JsonObject* obj = new JsonObject(*jsonRoot);
                obj->setString("toJson","not implemented");
                return obj;
            }

        protected:

            PositionUnit getUnit(const char *val, PositionUnit defaultUnit) {
                const char * p = val;
                while(p != NULL && *p!= 0 && *p!='p'&& *p!='%' && *p!='i' ) {
                    p++;
                }
                if (Util::equalAny(p,"px","pixel")) { return POS_PIXEL;}
                if (Util::equalAny(p,"%","percent")) { return POS_PERCENT;}
                if (Util::equalAny(p,"inherit")) { return POS_INHERIT;}
                return defaultUnit;
            }        
            Logger* m_logger;
    };

    class ScriptSystemValue : public ScriptValue {
        public:
            ScriptSystemValue(const char* name) {
                LinkedList<DRString> vals;
                if (Util::split(name,':',vals)==2) {
                    m_logger->debug("got name and scope");
                    m_scope = vals.get(0);
                    m_name = vals.get(1);
                } else {
                    m_name = vals.get(0);
                }
            }

            int getIntValue(IScriptContext* ctx,  int defaultValue) override
            {
                return (int)getFloatValue(ctx,(double)defaultValue);
            }

            double getFloatValue(IScriptContext* ctx,  double defaultValue) override
            {
                return get(ctx,defaultValue);
            }

        bool getBoolValue(IScriptContext* ctx,  bool defaultValue) override
            {
                double d = getFloatValue(ctx,0);
                return d != 0;
            }
        
        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { return defaultValue;}
        bool isNumber(IScriptContext* ctx) override { return true;}

    
        DRString toString() { return DRString("System Value: ").append(m_name); }
        private:
            double get(IScriptContext* ctx, double defaultValue){
                double val = defaultValue;
                if (Util::equal(m_name,"start")) {
                    val = 0; // todo: where is start in this version ctx->getStrip()->getStart();
                } else if (Util::equal(m_name,"count")) {
                    val = ctx->getStrip()->getLength();
                } else if (Util::equal(m_name,"step")) {
                    val = ctx->getStep()->getNumber();
                } else if (Util::equal(m_name,"red")) {
                    val = HUE::RED;
                }  else if (Util::equal(m_name,"orange")) {
                    val = HUE::ORANGE;
                }  else if (Util::equal(m_name,"yellow")) {
                    val = HUE::YELLOW;
                }  else if (Util::equal(m_name,"green")) {
                    val = HUE::GREEN;
                }  else if (Util::equal(m_name,"cyan")) {
                    val = HUE::CYAN;
                }  else if (Util::equal(m_name,"blue")) {
                    val = HUE::BLUE;
                }  else if (Util::equal(m_name,"magenta")) {
                    val = HUE::MAGENTA;
                }  else if (Util::equal(m_name,"purple")) {
                    val = HUE::PURPLE;
                } 


                m_logger->never("SystemValue %s:%s %f",m_scope.get(),m_name.get(),val);
                return val;
            }
            DRString m_scope;
            DRString m_name;


    };
  
    class NameValue
    {
    public:
        NameValue(const char *name, IScriptValue *value)
        {
            m_name = name;
            m_value = value;
        }

        virtual ~NameValue()
        {
            m_value->destroy();
        }

        virtual void destroy() { delete this;}

        const char *getName() { return m_name.text(); }
        IScriptValue *getValue() { return m_value; }

    private:
        DRString m_name;
        IScriptValue *m_value;
    };
    // ScriptVariableGenerator: ??? rand, trig, ...
    class FunctionArgs {
        public:
            FunctionArgs() {}
            virtual ~FunctionArgs() {}

            void add(IScriptValue* val) { args.add(val);}
            IScriptValue* get(int index) { return args.get(index);}
            size_t length() { return args.size();}
        PtrList<IScriptValue*> args;
    };

    int randTotal = millis();

    class ScriptFunction : public ScriptValue
    {
    public:
        ScriptFunction(const char *name, FunctionArgs* args) : m_name(name), m_args(args)
        {
            randomSeed(analogRead(0)+millis());
            m_funcState = -1;
        }

        virtual ~ScriptFunction()
        {
            delete m_args;
        }
        int getIntValue(IScriptContext* ctx,  int defaultValue) override
        {
            return (int)getFloatValue(ctx,(double)defaultValue);
        }

        double getFloatValue(IScriptContext* ctx,  double defaultValue) override
        {
            return invoke(ctx,defaultValue);
        }

       bool getBoolValue(IScriptContext* ctx,  bool defaultValue) override
        {
            double d = getFloatValue(ctx,0);
            return d != 0;
        }

        DRString toString() { return DRString("Function: ").append(m_name); }
        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            if (Util::equal("millis",m_name)) {
                return millis();
            }
            return defaultValue;
        }
        bool isNumber(IScriptContext* ctx) override { return true;}
        
    protected:
        double invoke(IScriptContext * ctx,double defaultValue) {
            double result = 0;
            const char * name = m_name.get();
            if (Util::equal("rand",name)){
                result = invokeRand(ctx,defaultValue);
            } else if (Util::equal("add",name) || Util::equal("+",name)) {
                result = invokeAdd(ctx,defaultValue);
            } else if (Util::equal("subtract",name) ||Util::equal("sub",name) || Util::equal("-",name)) {
                result = invokeSubtract(ctx,defaultValue);
            } else if (Util::equal("multiply",name) || Util::equal("mult",name) || Util::equal("*",name)) {
                result = invokeMultiply(ctx,defaultValue);
            } else if (Util::equal("divide",name) || Util::equal("div",name) || Util::equal("/",name)) {
                result = invokeDivide(ctx,defaultValue);
            } else if (Util::equal("mod",name) || Util::equal("%",name)) {
                result = invokeMod(ctx,defaultValue);
            } else if (Util::equal("min",name)) {
                result = invokeMin(ctx,defaultValue);
            } else if (Util::equal("max",name)) {
                result = invokeMax(ctx,defaultValue);
            } else if (Util::equal("randOf",name)) {
                result = invokeRandomOf(ctx,defaultValue);
            } else if (Util::equal("seq",name)||Util::equal("sequence",name)) {
                result = invokeSequence(ctx,defaultValue);
            } else {
                m_logger->error("unknown function: %s",name);
            }
            m_logger->never("function: %s=%f",m_name.get(),result);
            return result;
        }

        double invokeRand(IScriptContext*ctx ,double defaultValue) {
            int low = getArgValue(ctx,0,0);
            int high = getArgValue(ctx, 1,low);
            if (high == low) {
                low = 0;
            }
            if (high < low) {
                int t = low;
                low = high;
                high = t;
            }
            int val = random(low,high+1);
            return val;
            
        }

        double invokeAdd(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return first + second;
        }

        double invokeSubtract(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return first - second;
        }

        double invokeMultiply(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return first * second;
        }

        double invokeDivide(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return (second == 0) ? 0 : first / second;
        }

        
        double invokeMod(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return (double)((int)first % (int)second);
        }
        
        double invokeMin(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return first < second ? first : second;
        }
        
        double invokeMax(IScriptContext*ctx,double defaultValue) {
            double first = getArgValue(ctx,0,defaultValue);
            double second = getArgValue(ctx,1,defaultValue);
            return first > second ? first : second;
        }

        double invokeRandomOf(IScriptContext*ctx,double defaultValue) {
            int count = m_args->length();
            int idx = random(count);
            return getArgValue(ctx,idx,defaultValue);
        }

        double invokeSequence(IScriptContext*ctx,double defaultValue) {
            
            int start = getArgValue(ctx,0,0);
            int end = getArgValue(ctx,1,100);
            int step = getArgValue(ctx,2,1);

            if (m_funcState < start){ 
                m_funcState = start;
            } else {
                m_funcState += step;
            }
            if (m_funcState > end){
                m_funcState = start;
            }

            m_logger->never("Sequence %d %d %d ==> %f",start,end,step,m_funcState);
            return m_funcState;
        }

        double getArgValue(IScriptContext*ctx, int idx, double defaultValue){
            if (m_args == NULL) {
                return defaultValue;
            }
            IScriptValue* val = m_args->get(idx);
            if (val == NULL) { return defaultValue;}
            return val->getFloatValue(ctx,defaultValue);
        }

        DRString m_name;
        FunctionArgs * m_args;
        double m_funcState; // different functions can use in their way
    };

    class ScriptNumberValue : public ScriptValue
    {
    public:
        ScriptNumberValue(double value) : m_value(value)
        {

        }



        ScriptNumberValue(IScriptContext*ctx, IScriptValue* base,double defaultValue) {
            double val = base->getFloatValue(ctx,defaultValue);
            m_value = val;
        }

        virtual ~ScriptNumberValue()
        {
        }

        virtual int getIntValue(IScriptContext* ctx,  int defaultValue) override
        {
            int v = m_value;
            m_logger->test("getIntValue %f %d",m_value,v);
            return v;
        }

        virtual double getFloatValue(IScriptContext* ctx,  double defaultValue) override
        {
            return m_value;
        }

        virtual bool getBoolValue(IScriptContext* ctx,  bool defaultValue) override
        {
            return m_value != 0;
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            return m_value;
        }
        bool isNumber(IScriptContext* ctx) override { return true;}

        virtual DRString toString() { return DRString::fromFloat(m_value); }
        IJsonElement* toJson(JsonRoot*root) override { return new JsonFloat(*root,m_value);}

    protected:
        double m_value;
    };

    class ScriptBoolValue : public ScriptValue
    {
    public:
        ScriptBoolValue(bool value) : m_value(value)
        {
            m_logger->debug("ScriptBoolValue()");
        }

        virtual ~ScriptBoolValue()
        {
            m_logger->debug("~ScriptBoolValue()");
        }

        int getIntValue(IScriptContext* ctx,  int defaultValue) override
        {
            return m_value ? 1 : 0;
        }

        double getFloatValue(IScriptContext* ctx,  double defaultValue) override
        {
            return m_value ? 1 : 0;
        }

        bool getBoolValue(IScriptContext* ctx,  bool defaultValue) override {
            return m_value;
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            return defaultValue;
        }
        bool isBool(IScriptContext* ctx) override { return true;}
        IJsonElement* toJson(JsonRoot*root) override { return new JsonBool(*root,m_value);}

        DRString toString() override { 
            const char * val =  m_value ? "true":"false"; 
            DRString drv(val);
            return drv;
        }

    protected:
        bool m_value;
    };

    
    class ScriptNullValue : public ScriptValue
    {
    public:
        ScriptNullValue() 
        {
            m_logger->debug("ScriptNullValue()");
        }

        virtual ~ScriptNullValue()
        {
            m_logger->debug("~ScriptNullValue()");
        }

        int getIntValue(IScriptContext* ctx,  int defaultValue) override
        {
            return defaultValue;
        }

        double getFloatValue(IScriptContext* ctx,  double defaultValue) override
        {
            return defaultValue;
        }

        bool getBoolValue(IScriptContext* ctx,  bool defaultValue) override {
            return defaultValue;
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            return defaultValue;
        }

        bool isBool(IScriptContext* ctx) override { return false;}
        bool isNull(IScriptContext* ctx)  override{
            return true;  
        } 

        IJsonElement* toJson(JsonRoot*root) override { return new JsonNull(*root);}

        DRString toString() override { 
            m_logger->debug("ScriptNulllValue.toString()");
            DRString drv("ScriptNullValue");
            m_logger->debug("\tcreated DRString");
            return drv;
        }

    protected:

    };


    class ScriptStringValue : public ScriptValue
    {
    public:
        ScriptStringValue(const char *value) : m_value(value)
        {
            m_logger->never("ScriptStringValue 0x%04X %s",this,value ? value : "<null>");
        }

        virtual ~ScriptStringValue()
        {
        }

        int getIntValue(IScriptContext* ctx,  int defaultValue) override
        {
            const char *n = m_value.text();
            if (n != NULL)
            {
                return atoi(n);
            }
            return defaultValue;
        }

        double getFloatValue(IScriptContext* ctx,  double defaultValue) override
        {
            const char *n = m_value.text();
            if (n != NULL)
            {
                return atof(n);
            }
            return defaultValue;
        }

        bool getBoolValue(IScriptContext* ctx,  bool defaultValue) override
        {
            const char *n = m_value.text();
            if (n != NULL)
            {
                return Util::equal(n,"true");
            }
            return defaultValue;
        }

        bool equals(IScriptContext*ctx, const char * match) override { 
            m_logger->never("ScriptStringValue.equals %s==%s",m_value.get(),match);
            return Util::equal(m_value.text(),match);
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            return Util::toMsecs(m_value);
        }

        UnitValue getUnitValue(IScriptContext* ctx,  double defaultValue, PositionUnit defaultUnit) override{
            UnitValue uv(getIntValue(ctx,defaultValue),getUnit(m_value.text(),defaultUnit));
            return uv;
        }

        bool isString(IScriptContext* ctx) override { return true;}
        IJsonElement* toJson(JsonRoot*root) override { return new JsonString(*root,m_value);}

        const char * getValue() { return m_value.text();}


        DRString toString() override { return m_value; }

    protected:
        DRString m_value;
    };

    class ScriptRangeValue : public ScriptValue
    {
    public:
        ScriptRangeValue(IScriptValue *start, IScriptValue *end,IValueAnimator * animate=NULL )
        {
            m_start = start;
            m_end = end;
            m_animate = animate;
            m_unit = POS_UNSET;

        }

        virtual ~ScriptRangeValue()
        {
            if (m_start) {m_start->destroy();}
            if (m_end) {m_end->destroy();}
            if (m_animate) {m_animate->destroy();}
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            return getIntValue(ctx,defaultValue);
        }
        bool isNumber(IScriptContext* ctx) override { return true;}

        virtual int getIntValue(IScriptContext* ctx,  int defaultValue)
        {
            return (int)getFloatValue(ctx,(double)defaultValue);
        }

        virtual double getFloatValue(IScriptContext* ctx,  double defaultValue)
        {
            if (m_start == NULL)
            {
                m_logger->debug("\tno start.  return end %f");
                return m_end ? m_end->getIntValue(ctx, defaultValue) : defaultValue;
            }
            else if (m_end == NULL)
            {
                m_logger->debug("\tno end.  return start %f");
                return m_start ? m_start->getIntValue(ctx, defaultValue) : defaultValue;
            }
            double start = m_start->getFloatValue(ctx, 0);
            double end = m_end->getFloatValue(ctx,  1);
            double value = 0;
            if (m_animate) {
                AnimationRange range(start,end);
                value = m_animate->get(ctx,&range);
            } else {
                AnimationRange range(start,end,false);
                Animator animator((ctx->getAnimationPositionDomain()));
                CubicBezierEase ease;
                animator.setEase(&ease);

                value = animator.get(&range,ctx);
                
            }
            m_logger->never("Range %f-%f got %f",start,end,value);
            return value;
  
        }
        virtual bool getBoolValue(IScriptContext* ctx,  bool defaultValue)
        {
            int start = m_start->getIntValue(ctx, 0);
            int end = m_end->getIntValue(ctx, start);
            // bool probably doesn't make sense for a range.  return true if there is a range rather than single value
           return start != end;
        }

        virtual DRString toString()
        {
            m_logger->never("format range DRString");
            DRString result("range:");
            result.append(m_start ? m_start->toString() : "NULL")
                .append("--")
                .append(m_end ? m_end->toString() : "NULL");
            return result;
        }

        void setAnimator(IValueAnimator* animator) {
            m_animate = animator;
        }

        virtual UnitValue getUnitValue(IScriptContext* ctx,  double defaultValue, PositionUnit defaultUnit) {
            if (m_start == NULL) { return UnitValue(defaultValue,defaultUnit);}
            if (m_unit == POS_UNSET) {
                UnitValue uv = m_start->getUnitValue(ctx,defaultValue,defaultUnit);
                m_unit = uv.getUnit();
            }
            double val = getFloatValue(ctx,defaultValue);  
            return UnitValue(val,m_unit);      
        }


        IScriptValue* eval(IScriptContext * ctx, double defaultValue=0) override{
            auto start = m_start ? m_start->eval(ctx,defaultValue) : NULL;
            auto end = m_end ? m_end->eval(ctx,defaultValue) : NULL;
            auto animator = m_animate ? m_animate->clone(ctx) : NULL;
            return new ScriptRangeValue(start,end,animator);
        }


    protected:
        IScriptValue *m_start;
        IScriptValue *m_end;
        PositionUnit m_unit;

        IValueAnimator* m_animate;
    };



    class ScriptPatternElement
    {
    public:
        ScriptPatternElement(int repeatCount, IScriptValue* value)
        {
            m_value = value;
            m_repeatCount = repeatCount;
        }
        virtual ~ScriptPatternElement()
        {
            if (m_value) {m_value->destroy();}
        }

        int getRepeatCount() { return m_repeatCount;}

        IScriptValue* getValue() { return m_value;}
        virtual void destroy() { delete this;}

    private:
        IScriptValue *m_value;
        int m_repeatCount;
    };

    typedef enum PatternExtend {
        REPEAT_PATTERN=0,
        STRETCH_PATTERN=1,
        NO_EXTEND=2
    };

   class PatternValue : public ScriptValue
    {
    public:
        PatternValue(IValueAnimator* animate=NULL)
        {
            m_animate = animate;
            m_extend = REPEAT_PATTERN;
            m_count = 0;

        }

        virtual ~PatternValue()
        {
            if (m_animate) {m_animate->destroy();}
        }

        void setExtend(PatternExtend val) {
            m_extend = val;
        }

        void addElement(ScriptPatternElement* element) {
            if (element == NULL) {
                m_logger->error("ScriptPatternElement cannot be NULL");
                return;
            }
            m_elements.add(element);
            m_count += element->getRepeatCount();
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            return getIntValue(ctx,defaultValue);
        }
        bool isNumber(IScriptContext* ctx) override { return false;}

        virtual int getIntValue(IScriptContext* ctx,  int defaultValue)
        {
            return (int)getFloatValue(ctx,(double)defaultValue);
        }

        virtual double getFloatValue(IScriptContext* ctx,  double defaultValue)
        {
            UnitValue val = getFloatUnitValue(ctx, defaultValue,POS_INHERIT);
            return val.getValue();
        }

        virtual UnitValue getFloatUnitValue(IScriptContext* ctx,  double defaultValue, PositionUnit defaultUnit)
        {
            if (m_count == 0) { return UnitValue(defaultValue,defaultUnit); }

            double pos = 0;
            IAnimationDomain* domain = ctx->getAnimationPositionDomain();
            if (m_animate) {
                AnimationRange range(0,m_count);
                pos  = m_animate->get(ctx,&range);
                domain = m_animate->getDomain(ctx,&range);
 
            } else {
                pos = domain->getPosition();
            }
            UnitValue val = getValueAt(ctx, domain, pos,defaultValue,POS_INHERIT);
            return val;
        }

        UnitValue getValueAt(IScriptContext* ctx,IAnimationDomain* domain, int pos, double defaultValue,PositionUnit defaultUnit) {
            if (pos < 0 || pos >= m_count) {
                if (m_extend == NO_EXTEND) {
                    return UnitValue(defaultValue,defaultUnit);
                } else if (m_extend == REPEAT_PATTERN) {
                    pos = abs(pos)%m_count;
                }
            }
            
            int index = pos % m_count;
            if (m_extend == STRETCH_PATTERN) {
                double pct = (index*1.0/m_count);   
                double domainVal = domain->getPosition();
                index = m_count * domainVal;

            }
            int elementIndex = 0;
            ScriptPatternElement* element = m_elements.get(0);
            while(index >= element->getRepeatCount()) {
                elementIndex ++;
                index -= element->getRepeatCount();
                element = m_elements.get(elementIndex);
            }
            IScriptValue* val = element?element->getValue() : NULL;
            if (val == NULL || val->isNull(ctx)) {
                return UnitValue(defaultValue,POS_INHERIT);
            }
            return val->getUnitValue(ctx,defaultValue,defaultUnit);
  
        }
        virtual bool getBoolValue(IScriptContext* ctx,  bool defaultValue)
        {
            // bool probably doesn't make sense for a pattern.  return true if there are elements
           return m_count > 0;
        }

        virtual DRString toString()
        {
            DRString result("pattern:");
            return result;
        }

        void setAnimator(IValueAnimator* animator) {
            m_animate = animator;
        }

        UnitValue getUnitValue(IScriptContext*ctx, double defaultValue, PositionUnit defaultUnit) {
            return getFloatUnitValue(ctx,defaultValue,defaultUnit);
        }

        IScriptValue* eval(IScriptContext * ctx, double defaultValue=0) override{
            auto animator = m_animate ? m_animate->clone(ctx) : NULL;
            auto clone = new PatternValue(animator);
            m_elements.each([&](ScriptPatternElement*element) {
                IScriptValue* vclone = element->getValue()->eval(ctx,defaultValue);

                clone->addElement(new ScriptPatternElement(element->getRepeatCount(),vclone));
            });
            return clone;
        }


    protected:
        PtrList<ScriptPatternElement*> m_elements;
        size_t m_count;

        IValueAnimator* m_animate;
        PatternExtend m_extend;
    };

    class ScriptVariableValue : public IScriptValue
    {
    public:
        ScriptVariableValue(const char *value) : m_name(value)
        {
            m_logger = &ScriptValueLogger;
            m_logger->debug("Created ScriptVariableValue %s.", value);
            m_hasDefaultValue = false;
            m_recurse = false;
        }

        virtual ~ScriptVariableValue()
        {
            
        }

        void setDefaultValue(double val) {
            m_defaultValue = val;
            m_hasDefaultValue = true;
        }

        void destroy() override { delete this;}
        virtual int getIntValue(IScriptContext*ctx,  int defaultValue) override
        {
            return getFloatValue(ctx,defaultValue);
        }

        virtual double getFloatValue(IScriptContext*ctx,  double defaultValue) override
        {
            int dv = m_hasDefaultValue ? m_defaultValue : defaultValue;
            if (m_recurse) {
                m_logger->never("variable getFloatValue() recurse");
                return dv;
            }
            m_recurse = true;
            IScriptValue * val = ctx->getValue(m_name);
            if (val != NULL) {
                dv = val->getFloatValue(ctx,dv);
            }
            m_recurse = false;
            return dv;
        }

        virtual bool getBoolValue(IScriptContext*ctx,  bool defaultValue) override
        {
            m_recurse = true;
            IScriptValue * val = ctx->getValue(m_name);
            bool dv = m_hasDefaultValue ? (m_defaultValue != 0) : defaultValue;
            if (val != NULL) {
                dv = val->getBoolValue(ctx,dv);
            }
            m_recurse = false;
            return dv;
        }

        bool equals(IScriptContext*ctx, const char * match) override {
            IScriptValue * val = ctx->getValue(m_name);
            return val ? val->equals(ctx,match) : false;
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            IScriptValue * val = ctx->getValue(m_name);

            return val ? val->getMsecValue(ctx,defaultValue) : defaultValue;
        }

        bool isNumber(IScriptContext* ctx) { 
            IScriptValue * val = ctx->getValue(m_name);
            return val ? val->isNumber(ctx) : false;

         }
        bool isString(IScriptContext* ctx) { 
            IScriptValue * val = ctx->getValue(m_name);
            return val ? val->isString(ctx) : false;

         }
        bool isBool(IScriptContext* ctx) { 
            IScriptValue * val = ctx->getValue(m_name);
            return val ? val->isBool(ctx) : false;
         }
         bool isNull(IScriptContext* ctx) { 
            IScriptValue * val = ctx->getValue(m_name);
            return val ? val->isNull(ctx) : false;
         }

        IScriptValue* eval(IScriptContext * ctx, double defaultValue=0) override{
            return new ScriptNumberValue(getFloatValue(ctx,defaultValue));

        }

        
        virtual DRString toString() { return DRString("Variable: ").append(m_name); }

        bool isRecursing() { return m_recurse;}
    protected:
        DRString m_name;
        bool m_hasDefaultValue;
        double m_defaultValue;
        Logger *m_logger;
        bool m_recurse;
    };

    class ScriptValueList : public IScriptValueProvider {
        public:
            ScriptValueList() {
                m_logger = &ScriptValueLogger;
                m_logger->debug("create ScriptValueList()");
            }

            virtual ~ScriptValueList() {
                m_logger->debug("delete ~ScriptValueList()");
            }

            bool hasValue(const char *name) override  {
                return getValue(name) != NULL;
            }
            
            IScriptValue *getValue(const char *name)  override {
                NameValue** first = m_values.first([&](NameValue*&nv) {

                    return strcmp(nv->getName(),name)==0;
                });
                return first ? (*first)->getValue() : NULL;
            }

            void addValue(const char * name,IScriptValue * value) {
                if (Util::isEmpty(name) || value == NULL) {
                    return;
                }
                m_logger->debug("add NameValue %s  0x%04X",name,value);
                NameValue* nv = new NameValue(name,value);
                m_values.add(nv);
            }

            void each(auto&& lambda) const {
                m_values.each(lambda);
            }
 
            void destroy() { delete this;}

            int count() { return m_values.size();}
        private:
            PtrList<NameValue*> m_values;
            Logger* m_logger;
   };

   IScriptValue* ScriptValue::eval(IScriptContext * ctx, double defaultValue) {
        return new ScriptNumberValue(getFloatValue(ctx,defaultValue));
   }

   ScriptValue* ScriptValue::create(IJsonElement*json){
       if (json == NULL) { 
           return NULL;
       }
       if (json->asValue()) {
           IJsonValueElement* jsonVal = json->asValue();
            if (json->isNumber()) {
                return new ScriptNumberValue(jsonVal->getInt(0));
            } else if (json->isString()) {
                return new ScriptStringValue(jsonVal->getString(""));
            } else if (json->isBool()) {
                return new ScriptBoolValue(jsonVal->getBool(false));
            } else if (json->isNull()) {
                return new ScriptNullValue();
            }
       }
       return NULL;
   }
}
#endif