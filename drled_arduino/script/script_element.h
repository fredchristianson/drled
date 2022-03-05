#ifndef SCRIPT_ELEMENT_H
#define SCRIPT_ELEMENT_H

#include "../lib/util/util.h"
#include "./script_interface.h"
#include "./script_value.h"
#include "./json_names.h"
#include "./element_position.h"
#include "./script_hsl_strip.h"


namespace DevRelief {

    class ScriptElement : public IScriptElement {
        public:
            ScriptElement(const char * type){
                SET_LOGGER(ScriptElementLogger);
                m_type = type;
                m_container = NULL;
                m_logger->debug("Create ScriptElement type=%s",m_type);
                m_runMsecs = 0; // msecs before deleting this element.  0 means forever
                m_startMsecs = millis(); // time this element started
            }

            virtual ~ScriptElement(){

            }

            
            void toJson(JsonObject* json) const override {
                m_logger->debug("ScriptElement.toJson %s",getType());
                json->setString("type",m_type);
                json->setInt("run-duration",m_runMsecs);
                valuesToJson(json);
                m_logger->debug("\tdone %s",getType());
            }

            void fromJson(JsonObject* json) override {
                m_logger->debug("ScriptElement fromJson %s",json==NULL?"<no json>":json->toString().text());
                m_logger->debug("\tvalues");
                m_runMsecs = json->getInt("run-duration",0);
                valuesFromJson(json);
                m_logger->debug("\tdone");
            }

            void destroy() override {
                m_logger->debug("destroy element type=%s %x",m_type,this);
                delete this;
                m_logger->debug("\tdestroyed element");
            }

            bool isContainer() const override {
                return false;
            }

            const char * getType() const{ return m_type;}



            void draw(IScriptContext* context) override {
                m_logger->never("ScriptElement draw - nothing");

            };

            bool isPositionable() const override { return false; }
            IElementPosition* getPosition() const override { return NULL;}

            virtual void updatePosition(IElementPosition* parentPosition, IScriptContext* parentContext) {
                IElementPosition* pos = getPosition();
                if (pos) {
                    m_logger->never("updatePosition %x->%x, parent %x",this,pos,parentPosition);
                    pos->setParent(parentPosition);
                    pos->evaluateValues(parentContext);
                } else {
                    m_logger->never("no position");
                }
            }

            ScriptStatus updateStatus(IScriptContext* context) override {
                if (m_runMsecs > 0 && m_runMsecs < (millis()-m_startMsecs)) {
                    m_logger->always("ScriptElement complete");
                    return SCRIPT_COMPLETE;
                }
                return SCRIPT_RUNNING;
            }
        protected:
            virtual void valuesToJson(JsonObject* json) const{
                m_logger->never("ScriptElement type %s does not implement valuesToJson",getType());
            }
            virtual void valuesFromJson(JsonObject* json){
                m_logger->never("ScriptElement type %s does not implement valuesFromJson",getType());
            }

            IScriptValue* getJsonValue(JsonObject*json, const char * name) {
                m_logger->debug("getJsonValue %s",name);
                IJsonElement* propertyValue = json->getPropertyValue(name);
                if (propertyValue == NULL) { 
                    m_logger->debug("\tnot found");
                    return NULL;
                }
                return ScriptValue::create(propertyValue,json);
            }

            void setJsonValue(JsonObject* json,const char * name, IScriptValue*val) const  {
                if (val) {
                    json->set(name,val->toJson(json->getRoot()));
                }
            }

            void destroy(IScriptValue* val) {
                m_logger->debug("destroy value");
                if (val) { val->destroy();}
            }

            int m_runMsecs;
            int m_startMsecs;
            const char * m_type;
            IScriptElement* m_container;
            DECLARE_LOGGER();
    };

    class PositionableElement : public ScriptElement {
        public:
            PositionableElement(const char * type, IElementPosition*position) : ScriptElement(type){
                m_position = position;
            }

            virtual ~PositionableElement() {

            }

            void toJson(JsonObject* json) const override {
                ScriptElement::toJson(json);
                m_logger->debug("PositionableElement.toJson %s",getType());                
                positionToJson(json);
                m_logger->debug("\tdone PositionableElement.toJson %s",getType());                
            }

            void fromJson(JsonObject* json) override {
                ScriptElement::fromJson(json);
                m_logger->never("PositionableElement.fromJson %s",getType());
                positionFromJson(json);
                m_logger->never("\tPositionableElement.fromJson done");
            }            

            bool isPositionable() const override { return true; }
            IElementPosition* getPosition() const override { return m_position;}
            


        protected:
            virtual void positionToJson(JsonObject* json) const{
                if (isPositionable()) {
                    IElementPosition* pos = getPosition();
                    if (pos == NULL) {
                        m_logger->error("IPositionable does not have a getPosition() %s",getType());
                    } else {
                        pos->toJson(json);
                    }
                }
            }
            virtual void positionFromJson(JsonObject* json){
                m_logger->never("positionFromJson %s",getType());
                if (isPositionable()) {
                    IElementPosition*pos = getPosition();
                    if (pos == NULL) {
                        m_logger->error("IPositionable does not have a getPosition() %s",getType());
                    } else {
                        pos->fromJson(json);
                    }
                } else {
                    m_logger->never("\tnot positionable");
                }
                m_logger->never("\treverse %d",m_position->isReverse());
                m_logger->never("\tunit %d",m_position->getUnit());
            }
            IElementPosition* m_position;
    };

    class ValuesElement : public ScriptElement {
        public:
            ValuesElement() :ScriptElement(S_VALUES) {

            }

            virtual ~ValuesElement() {

            }

            void add(const char * name, IScriptValue* val) {
                m_values.setValue(name,val);
            }

            virtual void draw(IScriptContext*context) override {
                m_values.each([&](NameValue*nameValue){
                    context->setValue(nameValue->getName(),new ScriptValueReference(nameValue->getValue()));
                });
            }

        protected:
            virtual void valuesFromJson(JsonObject* json){
                m_logger->debug("ValuesElement.fromJson %s",getType());                

                json->eachProperty([&](const char * name, IJsonElement* val) {
                    m_values.setValue(name,ScriptValue::create(val));
                });
                m_logger->debug("\t done ValuesElement.fromJson %s",getType());                
            }
            virtual void valuesToJson(JsonObject* json){
                m_logger->debug("ValuesElement.toJson %s",getType());                
                m_values.each([&](NameValue*nameValue) {
                    json->set(nameValue->getName(),nameValue->getValue()->toJson(json->getRoot()));
                });
                m_logger->debug("\done ValuesElement.toJson %s",getType());                
            }        
            ScriptValueList m_values;
    };

    class ScriptLEDElement : public PositionableElement{
        public:
            ScriptLEDElement(const char* type) : PositionableElement(type,&m_elementPosition) {

            }

            virtual ~ScriptLEDElement() {
            }

            virtual void draw(IScriptContext*context) override {
                m_logger->never("ScriptLEDElement.draw");
                
                IScriptHSLStrip* parentStrip = context->getStrip();
                
                //m_elementPosition.evaluateValues(context);
                m_logger->never("create DrawStrip");
                DrawStrip strip(context,parentStrip,&m_elementPosition);
                m_logger->never("\titerate LEDs");
                strip.eachLED([&](IHSLStripLED& led) {
                    DrawLED* dl = (DrawLED*)&led;
                    m_logger->never("\tdraw child LED %d",dl->index()); 
                    drawLED(led);
                });
                
                m_logger->never("\t done ScriptLEDElement.draw()");
                //m_logger->showMemory();

            }


            void valuesToJson(JsonObject* json) const override {
            }

            void valuesFromJson(JsonObject* json) override {

            }

        protected:
            
            virtual void drawLED(IHSLStripLED& led)=0;
            ScriptElementPosition m_elementPosition;
            
    };

    class HSLElement : public ScriptLEDElement {
        public:
            HSLElement(const char *type=S_HSL) : ScriptLEDElement(type) {
                m_hue = NULL;
                m_saturation = NULL;
                m_lightness = NULL;
            }

            virtual ~HSLElement() {
                destroy(m_hue);
                destroy(m_saturation);
                destroy(m_lightness);
            }

            void setHue(IScriptValue* val) {
                if (m_hue) { m_hue->destroy();}
                 m_hue = val;
            }
            void setSaturation(IScriptValue* val) { 
                if (m_saturation) { m_saturation->destroy();}
                m_saturation = val;
            }
            void setLightness(IScriptValue* val) { 
                if (m_lightness) { m_lightness->destroy();}
                m_lightness = val;
            }
        protected:
            void drawLED(IHSLStripLED& led) override {
                if (m_hue) {
                    int hue = m_hue->getIntValue(led.getContext(),-1);
                    if (hue != -1) {
                        m_logger->never("drawLED %d %d",led.getIndex(),hue);
                        led.setHue(adjustHue(hue));
                    }
                }
                
                if (m_lightness) {
                    int lightness = m_lightness->getIntValue(led.getContext(),-1);
                    if (lightness != -1) {
                        led.setLightness(adjustLightness( lightness));
                    }
                }
                if (m_saturation) {
                    int saturation = m_saturation->getIntValue(led.getContext(),-1);
                    if (saturation != -1) {
                        led.setSaturation(adjustSaturation( saturation));
                    }
                }
            }

            virtual int adjustHue(int hue) { return hue;}
            virtual int adjustSaturation(int saturation) { return saturation;}
            virtual int adjustLightness(int lightness) { return lightness;}

            void valuesToJson(JsonObject* json) const override{
                ScriptLEDElement::valuesToJson(json);
                m_logger->debug("HSLElement.valuesToJson");
                if (m_hue) { 
                    m_logger->debug("\set hue");
                    json->set("hue",m_hue->toJson(json->getRoot()));
                }
                if (m_saturation) { 
                    m_logger->debug("\set saturation");
                    json->set("saturation",m_saturation->toJson(json->getRoot()));
                }
                if (m_lightness) { 
                    m_logger->debug("\set lightness");
                    json->set("lightness",m_lightness->toJson(json->getRoot()));
                }
                m_logger->debug("\tdone HSLElement.valuesToJson");
            }
            void valuesFromJson(JsonObject* json) override {
                ScriptLEDElement::valuesFromJson(json);
                setHue(getJsonValue(json,"hue"));
                setLightness(getJsonValue(json,"lightness"));
                setSaturation(getJsonValue(json,"saturation"));
            }        
            IScriptValue* m_hue;
            IScriptValue* m_saturation;
            IScriptValue* m_lightness;
    };

    class RainbowHSLElement : public HSLElement {
        public:
            RainbowHSLElement() : HSLElement(S_RHSL) {

            }

            virtual ~RainbowHSLElement() {

            }

            void valuesFromJson(JsonObject* json) override {
                HSLElement::valuesFromJson(json);
                // if rhue is set, use it.  otherwise hue is set by HSLElement::valuesFromJson ("hue")
                if (json->getPropertyValue("rhue")) {
                    setHue(getJsonValue(json,"rhue"));    
                } 
            }  

        protected:
            virtual int adjustHue(int hue) { 
                if (hue <0) { return hue;}
                return m_map.calculate(1.0*hue/360.0)*360.0;
            }

        private:
            CubicBezierEase m_map;

    };

    class RGBElement : public ScriptLEDElement {
        public:
            RGBElement(const char *type=S_RGB) : ScriptLEDElement(type) {
                m_red = NULL;
                m_green = NULL;
                m_blue = NULL;
            }

            virtual ~RGBElement() {
                destroy(m_red);
                destroy(m_green);
                destroy(m_blue);
            }

            void setRed(IScriptValue* val) {
                if (m_red) { m_red->destroy();}
                 m_red = val;
            }
            void setGreen(IScriptValue* val) { 
                if (m_green) { m_green->destroy();}
                m_green = val;
            }
            void setBlue(IScriptValue* val) { 
                if (m_blue) { m_blue->destroy();}
                m_blue = val;
            }
        protected:
            void drawLED(IHSLStripLED& led) override {
                int red = 0;
                int green = 0;
                int blue = 0;
                if (m_red) {
                    red = m_red->getIntValue(led.getContext(),0);
                }
                
                if (m_blue) {
                    blue = m_blue->getIntValue(led.getContext(),0);
                }
                if (m_green) {
                    green = m_green->getIntValue(led.getContext(),0);
                }
                if (red != 0 || blue != 0 || green != 0) {
                    CRGB rgb(red,green,blue);
                    led.setRGB(rgb);
                }
            }


            void valuesToJson(JsonObject* json) const override{
                ScriptLEDElement::valuesToJson(json);
                m_logger->debug("RGBElement.valuesToJson");
                if (m_red) { 
                    m_logger->debug("\set red");
                    json->set("red",m_red->toJson(json->getRoot()));
                }
                if (m_green) { 
                    m_logger->debug("\set green");
                    json->set("green",m_green->toJson(json->getRoot()));
                }
                if (m_blue) { 
                    m_logger->debug("\set blue");
                    json->set("blue",m_blue->toJson(json->getRoot()));
                }
                m_logger->debug("\tdone RGBElement.valuesToJson");
            }
            void valuesFromJson(JsonObject* json) override {
                ScriptLEDElement::valuesFromJson(json);
                m_logger->debug("valuesFromJson");
                setRed(getJsonValue(json,"red"));
                setGreen(getJsonValue(json,"green"));
                setBlue(getJsonValue(json,"blue"));
            }        
            IScriptValue* m_red;
            IScriptValue* m_green;
            IScriptValue* m_blue;
    };

}

#endif