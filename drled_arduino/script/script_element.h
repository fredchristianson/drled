#ifndef SCRIPT_ELEMENT_H
#define SCRIPT_ELEMENT_H

#include "../lib/util/util.h"
#include "./script_interface.h"
#include "./script_value.h"
#include "./json_names.h"
#include "./element_position.h"


namespace DevRelief {
    extern Logger ScriptElementLogger;

    class ScriptElement : public IScriptElement {
        public:
            ScriptElement(const char * type, IScriptHSLStrip* strip){
                m_logger = &ScriptElementLogger;
                m_type = type;
                m_strip = strip;
                m_logger->debug("Create ScriptElement type=%s",m_type);
            }

            virtual ~ScriptElement(){

            }

            void toJson(JsonObject* json) override {
                json->setString("type",m_type);
                positionToJson(json);
                valuesToJson(json);
            }

            void fromJson(JsonObject* json) override {
                positionFromJson(json);
                valuesFromJson(json);
            }

            void destroy() override {
                m_logger->debug("destroy element type=%s %x",m_type,this);
                delete this;
                m_logger->debug("\tdestroyed element");
            }

            bool isContainer() const override {
                return false;
            }

            const char * getType(){ return m_type;}

            void updateLayout(IScriptContext* context) override {

            };

            void draw(IScriptContext* context) override {
                m_logger->never("ScriptElement draw - nothing");

            };

            bool isPositionable() const override { return false; }
            IElementPosition* getPosition() const { return NULL;}

            IScriptHSLStrip* getStrip() { return m_strip;}
        protected:
            virtual void positionToJson(JsonObject* json){

            }
            virtual void positionFromJson(JsonObject* json){
                
            }

            virtual void valuesToJson(JsonObject* json){

            }
            virtual void valuesFromJson(JsonObject* json){
                
            }

            IScriptValue* getJsonValue(JsonObject*json, const char * name) {
                m_logger->debug("getJsonValue %s",name);
                IJsonElement* propertyValue = json->getPropertyValue(name);
                if (propertyValue == NULL) { 
                    m_logger->debug("\tnot found");
                    return NULL;
                }
                return ScriptValue::create(propertyValue);
            }

            void destroy(IScriptValue* val) {
                m_logger->debug("destroy value");
                if (val) { val->destroy();}
            }

            const char * m_type;
            Logger* m_logger;

            IScriptHSLStrip* m_strip;
    };

    class ValuesElement : public ScriptElement {
        public:
            ValuesElement() :ScriptElement(S_VALUES,NULL) {

            }

            virtual ~ValuesElement() {

            }

            void add(const char * name, IScriptValue* val) {
                m_values.addValue(name,val);
            }
        protected:
            virtual void valuesFromJson(JsonObject* json){
                json->eachProperty([&](const char * name, IJsonElement* val) {
                    m_values.addValue(name,ScriptValue::create(val));
                });
            }
            virtual void valuesToJson(JsonObject* json){
                m_values.each([&](NameValue*nameValue) {
                    json->set(nameValue->getName(),nameValue->getValue()->toJson(json->getRoot()));
                });
            }        
            ScriptValueList m_values;
    };

    class ScriptLEDElement : public ScriptElement{
        public:
            ScriptLEDElement(const char* type) : ScriptElement(type, &m_strip) {
                m_position = NULL;
            }

            virtual ~ScriptLEDElement() {
                if (m_position) { m_position->destroy();}
            }

            virtual void draw(IScriptContext*context) {
                IScriptHSLStrip* strip = context->getStrip();
                int stripLength = strip->getLength();
                m_logger->never("ScriptLEDElement draw %d",strip->getLength());
                int start = m_position ? m_position->getStart() : 0;
                int count = m_position ? m_position->getCount() : stripLength-start;
                for(int i=0;i<count;i++) {
                    strip->setPosition(i+start);
                    drawLED(context,strip,i);
                }
            }


            void valuesToJson(JsonObject* json) override {
                if (m_position) {
                    m_position->toJson(json);
                }
            }

            void valuesFromJson(JsonObject* json) override {
                if (m_position) { m_position->destroy();}
                m_logger->debug("create ElementPosition from json %x",json);
                m_position = new ScriptElementPosition(json);                
                m_logger->debug("\tElementPosition created");
            }

            bool isPositionable()  const override { return true;}
            IElementPosition* getPosition() const override { 
                m_logger->debug("return m_position  %x",m_position);
                return m_position;
            }
        protected:
            
            virtual void drawLED(IScriptContext*context,IScriptHSLStrip* strip,int index)=0;
            ScriptElementPosition* m_position;
            ElementHSLStrip m_strip;
            
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
            void drawLED(IScriptContext*context,IScriptHSLStrip* strip,int index) override {
                if (m_hue) {
                    int hue = m_hue->getIntValue(context,-1);
                    strip->setHue(adjustHue(context,strip, hue));
                }
                
                if (m_lightness) {
                    int lightness = m_lightness->getIntValue(context,-1);
                    strip->setLightness(adjustLightness(context,strip, lightness));
                }
                if (m_saturation) {
                    int saturation = m_saturation->getIntValue(context,-1);
                    strip->setSaturation(adjustSaturation(context,strip, saturation));
                }
            }

            virtual int adjustHue(IScriptContext*context,IScriptHSLStrip* strip,int hue) { return hue;}
            virtual int adjustSaturation(IScriptContext*context,IScriptHSLStrip* strip,int saturation) { return saturation;}
            virtual int adjustLightness(IScriptContext*context,IScriptHSLStrip* strip,int lightness) { return lightness;}

            virtual void valuesToJson(JsonObject* json){
                ScriptLEDElement::valuesToJson(json);
                m_logger->debug("valuesToJson");
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
            }
            virtual void valuesFromJson(JsonObject* json){
                ScriptLEDElement::valuesFromJson(json);
                m_logger->debug("valuesFromJson");
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

        protected:
            virtual int adjustHue(IScriptContext*context,IScriptHSLStrip* strip,int hue) { 
                if (hue <0) { return hue;}
                return m_map.calculate(1.0*hue/360.0)*360.0;
            }

        private:
            CubicBezierEase m_map;

    };

}

#endif