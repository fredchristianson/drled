#ifndef DRSCRIPT_VALUE_H
#define DRSCRIPT_VALUE_H

#include "../lib/log/logger.h"
#include "../lib/json/json.h"
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

    // ScriptValueReference is a pointer to another ScriptValue 
    // the pointer can be deleted while the real value remains
    class ScriptValueReference : public IScriptValue {
        public:
            ScriptValueReference(IScriptValue* ref) {
                m_reference = ref;
            }

            virtual ~ScriptValueReference() { /* do not delete m_reference*/}
            void destroy() { delete this;}

            int getIntValue(IScriptContext* ctx,  int defaultValue)  { return m_reference->getIntValue(ctx,defaultValue);}
            double getFloatValue(IScriptContext* ctx,  double defaultValue)  { return m_reference->getFloatValue(ctx,defaultValue);} 
            bool getBoolValue(IScriptContext* ctx,  bool defaultValue)  { return m_reference->getBoolValue(ctx,defaultValue); }
            int getMsecValue(IScriptContext* ctx,  int defaultValue)  { return m_reference->getMsecValue(ctx,defaultValue);}
            UnitValue getUnitValue(IScriptContext* ctx,  double defaultValue, PositionUnit defaultUnit) { return m_reference->getUnitValue(ctx,defaultValue,defaultUnit);}
            bool isString(IScriptContext* ctx) { return m_reference->isString(ctx);}
            bool isNumber(IScriptContext* ctx) { return m_reference->isNumber(ctx);}
            bool isBool(IScriptContext* ctx) { return m_reference->isBool(ctx);}
            bool isNull(IScriptContext* ctx) { return m_reference->isNull(ctx);}
            bool isUnitValue(IScriptContext* ctx) { return m_reference->isUnitValue(ctx);}

            bool equals(IScriptContext*ctx, const char * match) { return m_reference->equals(ctx,match);}


            // evaluate this IScriptValue with the given command and return a new
            // IScriptValue.  mainly useful to get a random number one time
            IScriptValue* eval(IScriptContext*ctx, double defaultValue) { m_reference->eval(ctx,defaultValue); }


            bool isRecursing() { return m_reference->isRecursing();} 

            IJsonElement* toJson(JsonRoot* jsonRoot) { m_reference-> toJson(jsonRoot);}
            // for debugging
            DRString toString() { return m_reference->toString();}           

            DRString stringify() { return m_reference->stringify();}
        private:
            IScriptValue* m_reference;
    };

    class ScriptValue : public IScriptValue {
        public:
            static IScriptValue* create(IJsonElement*json);
            static IScriptValue* createFromString(const char * string);
            static IScriptValue* createFromArray(JsonArray* json);
            static IScriptValue* createFromObject(JsonObject* json);
            static ScriptPatternElement* createPatternElement(IJsonElement*json);
            static IAnimationEase* createEase(JsonObject* json);
            static IAnimationDomain* createDomain(JsonObject* json, IAnimationRange* range);

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
                UnitValue uv(getFloatValue(ctx,defaultValue),defaultUnit);
                return uv;
            }

            bool equals(IScriptContext*ctx,const char * match) override { return false;}

            IScriptValue* eval(IScriptContext * ctx, double defaultValue=0) override;
            
            IJsonElement* toJson(JsonRoot* jsonRoot) override {
                JsonObject* obj = new JsonObject(*jsonRoot);
                obj->setString("toJson","not implemented");
                return obj;
            }

            DRString stringify() override { return "";}

        protected:

            static PositionUnit getUnit(const char *val, PositionUnit defaultUnit) {
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
        ScriptFunction(const char *name, FunctionArgs* args=NULL) : m_name(name)
        {
            randomSeed(analogRead(0)+millis());
            if (args == NULL) {
                m_args = new FunctionArgs();
            } else {
                m_args = args;
            }
            m_funcState = -1;
        }

        virtual ~ScriptFunction()
        {
            delete m_args;
        }

        void addArg(IScriptValue*val) {
            m_args->add(val);
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
            if (m_args->length()==1) {
                return -first;
            }
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
        DRString stringify() override { return DRString::fromFloat(m_value);}
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

        DRString stringify() override { return m_value ? "true":"false";}
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

        DRString stringify() override { return "null";}

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
        DRString stringify() override { return m_value;}

    protected:
        DRString m_value;
    };

    class AnimatedValue : public ScriptValue {
        public:
            AnimatedValue( ) {
                m_startMSecs = millis();
            }

            ~AnimatedValue()  {
                if (m_animator) { m_animator->destroy();}
            }

            void setAnimator(IValueAnimator* animator) {
                m_animator = animator;
            }


            
            int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
                return getIntValue(ctx,defaultValue);
            }
            bool isNumber(IScriptContext* ctx) override { return false;}

            virtual int getIntValue(IScriptContext* ctx,  int defaultValue)
            {
                return getFloatValue(ctx,(double)defaultValue);
            }

            virtual double getFloatValue(IScriptContext* ctx,  double defaultValue)
            {
                UnitValue uv = getUnitValue(ctx,(double)defaultValue,POS_INHERIT);
                return uv.getValue();
            }


            virtual bool getBoolValue(IScriptContext* ctx,  bool defaultValue)
            {
                return getFloatValue(ctx,defaultValue?1:0) != 0;
            }
        protected:
 

            unsigned long m_startMSecs;
            IValueAnimator* m_animator;
    };

    class ScriptRangeValue : public AnimatedValue
    {
    public:
        ScriptRangeValue(IScriptValue *start, IScriptValue *end) : AnimatedValue()
        {
            m_start = start;
            m_end = end;
        }

        virtual ~ScriptRangeValue()
        {
            if (m_start) {m_start->destroy();}
            if (m_end) {m_end->destroy();}
        }

        UnitValue getUnitValue(IScriptContext*ctx, double defaultValue, PositionUnit defaultUnit) {
            if (m_animator == NULL) {
                return UnitValue(defaultValue,defaultUnit);
            }
            m_animator->update(ctx);

            double value = m_animator->getRangeValue(ctx);
            return value;
  
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


/*

        IScriptValue* eval(IScriptContext * ctx, double defaultValue=0) override{
            auto start = m_start ? m_start->eval(ctx,defaultValue) : NULL;
            auto end = m_end ? m_end->eval(ctx,defaultValue) : NULL;
            return new ScriptRangeValue(start,end,NULL);
        }
*/

    protected:
        IScriptValue *m_start;
        IScriptValue *m_end;

    };

     

   class PatternValue : public AnimatedValue
    {
    public:
        PatternValue(JsonObject* json) : AnimatedValue()
        {
            m_animate = NULL;
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

        
        UnitValue getUnitValue(IScriptContext*ctx, double defaultValue, PositionUnit defaultUnit) {
            if (m_animator == NULL) {
                return UnitValue(defaultValue,defaultUnit);
            }
            m_animator->update(ctx);

            double value = m_animator->getRangeValue(ctx);
             
            UnitValue val = getValueAt(ctx,  value,defaultValue,POS_INHERIT);
            return val;
        }

        UnitValue getValueAt(IScriptContext* ctx, double value, double defaultValue,PositionUnit defaultUnit) {
  

            int index = value;
            
            int elementIndex = 0;
            ScriptPatternElement* element = m_elements.get(0);
            while(element != NULL && index >= element->getRepeatCount()) {
                elementIndex ++;
                index -= element->getRepeatCount();
                element = m_elements.get(elementIndex);
            }
            
            if (elementIndex >= m_elements.size()) {
                elementIndex = m_elements.size()-1;
            }
            m_logger->always("getValueAt(%f) index=%d elementidx=%d repeat=%d",value,index,elementIndex,element ? element->getRepeatCount() : -1);

            
            IScriptValue* val = element?element->getValue() : NULL;
            if (val == NULL || val->isNull(ctx)) {
                m_logger->never("\tnull");
                return UnitValue(defaultValue,POS_INHERIT);
            }
            UnitValue uv = val->getUnitValue(ctx,defaultValue,defaultUnit);
            m_logger->never("\tuv %f %d",uv.getValue(),uv.getUnit());
            return uv;
  
        }
       

        virtual DRString toString()
        {
            DRString result("pattern:");
            return result;
        }

        virtual bool getBoolValue(IScriptContext* ctx,  bool defaultValue)
        {
            // bool probably doesn't make sense for a pattern.  return true if there are elements
        return m_count > 0;
        }

        IScriptValue* eval(IScriptContext * ctx, double defaultValue=0) override{
            return NULL;

        }

        size_t getCount() { return m_count;}

    protected:
        PtrList<ScriptPatternElement*> m_elements;
        size_t m_count;

        IValueAnimator* m_animate;
        PatternExtend m_extend;
    };

    class PatternRange : public AnimationRange {
        public:
            PatternRange(PatternValue * pattern,bool unfold) : AnimationRange(unfold) {
                m_logger = &ScriptValueLogger;
                m_pattern = pattern;
            }

            virtual ~PatternRange() {

            }

            void destroy() { delete this;}

    
            void update(IScriptContext* ctx){
                if (m_pattern) {
                    m_high = m_pattern->getCount()-1;
                }
            }

            /* extends pattern to full length of strip */
            double getValue(double position) {
                int count = m_pattern ? m_pattern->getCount() : 0;
                if (count < 2) { return 0;}

                double val = AnimationRange::getValue(position);
                int index = (val/(count-1))*count; // stretch
                m_logger->never("PatternRange %.2f-%.2f %.2f=>%.2f",getMinValue(),getMaxValue(),position,val);
                return index;
            }


        protected:
            Logger* m_logger;
            PatternValue* m_pattern;
    };

    class RepeatPatternRange : public PatternRange {
        public:
            RepeatPatternRange(PatternValue* pattern,bool alternate) : PatternRange(pattern,false) {
                // cannot unfold like other animations.  need to alternate drawing the pattern forward then backward
                m_alternate = alternate;  
            }
            virtual ~RepeatPatternRange() {}

            /* 1-to-1 mapping from strip LEDs to value */
            void update(IScriptContext* ctx){
               // m_high = (ctx->getStrip()->getLength())-1; 
               IAnimationDomain* domain = ctx->getAnimationPositionDomain();
               int length = domain->getMax()-domain->getMin()+1;
               m_high = length;
            }

            /* repeat pattern */
            double getValue(double position) {
                int count = m_pattern ? m_pattern->getCount() : 0;
                if (count < 2) { return 0;}

                double val = AnimationRange::getValue(position);
                int index = ((int)round(val)) % (count);
                if (m_alternate) {
                    // mod the countx2.  first half runs pattern forward.  2nd have goes backward;
                    index = ((int)round(val)) % ((count-1)*2);
                    if (index >= (count-1)) { 
                        index = (count-1)*2-index;
                    }
                }
                
                m_logger->never("RepeatPatternRange count=%d pos=%f val=%f index=%d",count,position,val,index);
                return index;
            }

        protected:
            bool m_alternate;
    };

    class ScriptVariableValue : public IScriptValue
    {
    public:
        ScriptVariableValue(bool isSysValue, const char *value,IScriptValue* defaultValue) : m_name(value)
        {
            m_logger = &ScriptValueLogger;
            m_logger->debug("Created ScriptVariableValue %s %d.", value, isSysValue);
            m_defaultValue = defaultValue;
            m_isSysValue = isSysValue;
            m_recurse = false;
        }

        virtual ~ScriptVariableValue()
        {
            if (m_defaultValue) { m_defaultValue->destroy();}
        }


        void destroy() override { delete this;}

        virtual int getIntValue(IScriptContext*ctx,  int defaultValue) override
        {
            return getFloatValue(ctx,defaultValue);
        }

        virtual double getFloatValue(IScriptContext*ctx,  double defaultValue) override
        {
            if (m_recurse) {
                m_logger->never("variable getFloatValue() recurse");
                return m_defaultValue ? m_defaultValue->getFloatValue(ctx,defaultValue) : defaultValue;
            }
            m_recurse = true;
            IScriptValue * val = getScriptValue(ctx);
            double result = val->getFloatValue(ctx,defaultValue);
            m_recurse = false;
            return result;
        }

        virtual bool getBoolValue(IScriptContext*ctx,  bool defaultValue) override
        {
            if (m_recurse) {
                m_logger->never("variable getFloatValue() recurse");
                return m_defaultValue ? m_defaultValue->getBoolValue(ctx,defaultValue) : defaultValue;
            }
            m_recurse = true;
            IScriptValue * val = getScriptValue(ctx);
            bool result =  val->getBoolValue(ctx,defaultValue);
           
            m_recurse = false;
            return result;
        }

        bool equals(IScriptContext*ctx, const char * match) override {
            IScriptValue * val = getScriptValue(ctx);
            return val ? val->equals(ctx,match) : false;
        }

        int getMsecValue(IScriptContext* ctx,  int defaultValue) override { 
            IScriptValue * val = getScriptValue(ctx);

            return val ? val->getMsecValue(ctx,defaultValue) : defaultValue;
        }

        UnitValue getUnitValue(IScriptContext* ctx,  double defaultValue, PositionUnit defaultUnit) override { 
            IScriptValue * val = getScriptValue(ctx);

            return val ? val->getUnitValue(ctx,defaultValue,defaultUnit) : UnitValue(defaultValue,defaultUnit);
        }

        bool isNumber(IScriptContext* ctx) { 
            IScriptValue * val = getScriptValue(ctx);
            return val ? val->isNumber(ctx) : false;

         }
        bool isString(IScriptContext* ctx) { 
            IScriptValue * val = getScriptValue(ctx);
            return val->isString(ctx);

         }
        bool isBool(IScriptContext* ctx) { 
            IScriptValue * val = getScriptValue(ctx);
            return val->isBool(ctx);
         }
         bool isNull(IScriptContext* ctx) { 
            IScriptValue * val = getScriptValue(ctx);
            return val->isNull(ctx);
         }

        bool isUnitValue(IScriptContext* ctx) override { 
            IScriptValue * val = getScriptValue(ctx);
            return val->isUnitValue(ctx);
         }

        IScriptValue* eval(IScriptContext * ctx, double defaultValue=0) override{
            return new ScriptNumberValue(getFloatValue(ctx,defaultValue));

        }

        IJsonElement* toJson(JsonRoot* jsonRoot) override {
            DRFormattedString val("%s(%s)",(m_isSysValue?"sys":"var"),m_name.text());
            if (m_defaultValue) {
                DRString text = m_defaultValue->stringify();
                if (text.getLength()>0) {
                    val.append("|");
                    val.append(text);
                }
                
            }
            return new JsonString(*jsonRoot,val.text());
        }
        
        virtual DRString toString() { return DRString("Variable: ").append(m_name); }

        bool isRecursing() { return m_recurse;}

        DRString stringify() { return "";} // cannot stringify vars
    protected:
        IScriptValue* getScriptValue(IScriptContext*context) {
            IScriptValue* val = m_isSysValue ? context->getSysValue(m_name) : context->getValue(m_name);
            if (val == NULL)  {
                val = m_defaultValue;
            }

            return val ? val : &NULL_VALUE;
        }
        DRString m_name;
        bool m_isSysValue;
        IScriptValue*  m_defaultValue;
        Logger *m_logger;
        bool m_recurse;

        static ScriptNullValue NULL_VALUE;
    };

    ScriptNullValue ScriptVariableValue::NULL_VALUE;

    class ScriptValueList : public IScriptValueProvider {
        public:
            ScriptValueList(IScriptValueProvider* parentScope=NULL) {
                m_logger = &ScriptValueLogger;
                m_logger->never("create ScriptValueList()");
                m_parentScope = parentScope;
            }



            virtual ~ScriptValueList() {
                m_logger->never("delete ~ScriptValueList()");
            }

            IScriptValueProvider* getParentScope() { return m_parentScope;}
            bool hasValue(const char *name) override  {
                return getValue(name) != NULL;
            }
            
            IScriptValue *getValue(const char *name)  override {
                m_logger->never("getValue %s",name);
                NameValue** first = m_values.first([&](NameValue*&nv) {

                    return strcmp(nv->getName(),name)==0;
                });
                if (first) {
                    IScriptValue* found = (*first)->getValue();
                    if (found) {
                        m_logger->never("\tfound %s",found->toString().text());
                        return found;
                    }
                }
                if (m_parentScope) {
                    m_logger->never("\tuse parent scope");
                    return m_parentScope->getValue(name);
                }
                m_logger->never("\tnot found");
                return NULL;
            }

            void setValue(const char * name,IScriptValue * value) {
                if (Util::isEmpty(name) || value == NULL) {
                    return;
                }
                m_logger->never("add NameValue %s  0x%04X",name,value);
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
            IScriptValueProvider* m_parentScope;
            Logger* m_logger;
   };

   IScriptValue* ScriptValue::eval(IScriptContext * ctx, double defaultValue) {
        return new ScriptNumberValue(getFloatValue(ctx,defaultValue));
   }

   IScriptValue* ScriptValue::create(IJsonElement*json){
       if (json == NULL) { 
           return NULL;
       }
       IScriptValue * result = NULL;
       if (json->asValue()) {
           IJsonValueElement* jsonVal = json->asValue();
            if (json->isNumber()) {
                result = new ScriptNumberValue(jsonVal->getFloat(0));
            } else if (json->isString()) {
                //return new ScriptStringValue(jsonVal->getString(""));
                result = createFromString(jsonVal->getString(""));
            } else if (json->isBool()) {
                result = new ScriptBoolValue(jsonVal->getBool(false));
            } else if (json->isNull()) {
                result = new ScriptNullValue();
            }
       } else if (json->asArray()) {
           result = createFromArray(json->asArray());
       } else if (json->asObject()) {
           result = createFromObject(json->asObject());
       }
       return result;
   }

   IScriptValue* ScriptValue::createFromString(const char * string){
       while(string != NULL && isspace(*string)) {
           string++;
       }
       if (string == NULL || string[0] == 0) { return NULL;}
       IScriptValue* result = NULL;
       if (Util::startsWith(string,"var(") || Util::startsWith(string,"sys(")){
           LinkedList<DRString> nameDefault;
           bool isSysVar = string[0] == 's';
           Util::split(string+4,')',nameDefault);
           if (nameDefault.size() == 1) {
               ScriptValueLogger.never("create variable %d %s",isSysVar,nameDefault.get(0));
               result = new ScriptVariableValue(isSysVar, nameDefault.get(0).text(),NULL);
           } else if (nameDefault.size() == 2) {
               const char * defString = nameDefault.get(1).text();
               while (defString != NULL && defString[0] != 0 && defString[0] == '|'){
                   defString++;
               }
               ScriptValueLogger.never("create variable %d %s | %s",isSysVar,nameDefault.get(0), defString);

                result = new ScriptVariableValue(isSysVar, nameDefault.get(0),createFromString(defString));
           } else {
               ScriptValueLogger.error("Cannot create variable for %s",string);
           }
       } else if (Util::equal(string,"null")) { result = new ScriptNullValue();}
       else if (Util::equal(string,"true")) { result = new ScriptBoolValue(true);}
       else if (Util::equal(string,"false")) { result = new ScriptBoolValue(false);}
       else { result = new ScriptStringValue(string);}
       return result;
   }

   IScriptValue* ScriptValue::createFromArray(JsonArray* json){
       if (json == NULL || json->getCount() == 0) { return NULL;}
       int count = json->getCount();
       ScriptFunction* func = new ScriptFunction(json->getAt(0)->asValue()->getString());
       for(int i=1;i<count;i++){
           func->addArg(create(json->getAt(i)));
       }
       return func;
   }

   IScriptValue* ScriptValue::createFromObject(JsonObject* json){
        AnimatedValue* result = NULL;
        JsonArray* pattern = json->getArray("pattern");
        IJsonElement* start = json->getPropertyValue("start");
        IJsonElement* end = json->getPropertyValue("end");
        if (pattern) {
            //ScriptValueLogger.error("pattern not implemented");
            PatternValue* patternValue = new PatternValue(json);
            result = patternValue;
            pattern->each([&](IJsonElement*elemenValue){
                patternValue->addElement(createPatternElement(elemenValue));
            });
            IAnimationEase* ease = createEase(json);
            bool unfold = json->getBool("unfold",false);
            bool repeat = json->getBool("repeat",false);
            IAnimationRange* range = NULL;
            if (repeat) {
                range = new RepeatPatternRange(patternValue,unfold);
            } else {
                range = new PatternRange(patternValue,unfold);
            }
            IAnimationDomain* domain = createDomain(json,range);
            IValueAnimator* animator = new Animator(domain,range,ease);
            patternValue->setAnimator(animator);
        } else if (start && end) {
           // result = new ScriptRangeValue(create(start),create(end),json);
        }
        
        return result;
   }

    IAnimationEase* ScriptValue::createEase(JsonObject* json){
        IAnimationEase* ease = NULL;
        double easeValue =1;
        IJsonElement* easeJson = json->getPropertyValue("ease");
        IJsonValueElement* easeVal = easeJson ? easeJson->asValue() : NULL;
        IJsonElement* easeIn = json->getPropertyValue("ease-in");
        IJsonValueElement* easeInVal = easeIn ? easeIn->asValue() : NULL;
        IJsonElement* easeOut = json->getPropertyValue("ease-out");
        IJsonValueElement* easeOutVal = easeOut ? easeOut->asValue() : NULL;
        if (easeVal) {
            if (Util::equal(easeVal->getString(),"linear")){
                return new LinearEase();
            }
        }
        IScriptValue* inValue=NULL;
        IScriptValue* outValue=NULL;
        if (easeInVal != NULL) {
            inValue = ScriptValue::create(easeIn);
        } else {
            inValue = ScriptValue::create(easeJson);
        }
        if (easeOutVal != NULL) {
            outValue = ScriptValue::create(easeOut);
        } else {
            outValue = ScriptValue::create(easeJson);
        }
        return new CubicBezierEase(inValue,outValue);
        
    }

    IAnimationDomain* ScriptValue::createDomain(JsonObject* json, IAnimationRange* range){
        IAnimationDomain* domain = NULL;
        IJsonElement* speedJson = json->getPropertyValue("speed");
        IJsonElement* durationJson = json->getPropertyValue("duration");
        if (speedJson) {
            IScriptValue* speedValue = create(speedJson);
            if (speedValue) {
                domain = new SpeedDomain(speedValue,range);
            }
        } else if (durationJson) {
            IScriptValue* durationValue = create(durationJson);
            if (durationJson) {
                domain = new DurationDomain(durationValue);
            }
        } else {
            domain = new ContextPositionDomain();
        }
        return domain;
    }




   ScriptPatternElement* ScriptValue::createPatternElement(IJsonElement*json){
        if (json == NULL) { return NULL;}
        ScriptPatternElement* element = NULL;
        IScriptValue * val=NULL;
        int count = 0;
        PositionUnit unit = POS_PIXEL;
        if (json->isObject()) {
            JsonObject* obj = json->asObject();
            val = create(obj->getPropertyValue("value"));
            unit = getUnit(obj->getString("unit",NULL),POS_PIXEL);
            IJsonElement* countProp = obj->getPropertyValue("count");
            if (countProp && countProp->hasValue()) {
                IJsonValueElement* countVal = countProp->asValue();
                if (countProp->isString()){
                    const char * countStr = countVal->getString();
                }
                count = obj->getInt("count",1);
            }
        } else if (json->isString()) {
            const char * str = json->asValue()->getString(NULL);
            if (str != NULL) { 
                LinkedList<DRString> valCount;
                Util::split(str,'x',valCount);
                
                if (valCount.size()==1) {
                    val = createFromString(valCount.get(0));
                    count = 1;
                } else if (valCount.size()==2) {
                    const char * countAndUnit = valCount.get(1).text();
                    count = atoi(countAndUnit);
                    unit = getUnit(countAndUnit,POS_PIXEL);
                    val = createFromString(valCount.get(0).text());
                }
            }
        } else if (json->isNull()) {
            val = new ScriptNullValue();
            count = 1;
        } else {
            val = create(json);
            count = 1;
        }
        if (val == NULL) {
            val = new ScriptNullValue();
        }
        ScriptValueLogger.always("PatternElement %d %d %s",count,unit,val->toString().text());
        element = new ScriptPatternElement(count,unit, val);
        return element;
   }

}
#endif