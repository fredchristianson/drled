#ifndef SCRIPT_ELEMENT_H
#define SCRIPT_ELEMENT_H

#include "../lib/util/util.h"
#include "./script_interface.h"
#include "./script_value.h"
#include "./json_names.h"
#include "./element_position.h"
#include "./script_hsl_strip.h"


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

            void toJson(JsonObject* json) const override {
                m_logger->debug("ScriptElement.toJson %s",getType());
                json->setString("type",m_type);
                valuesToJson(json);
                m_logger->debug("\tdone %s",getType());
            }

            void fromJson(JsonObject* json) override {
                m_logger->debug("ScriptElement fromJson %s",json==NULL?"<no json>":json->toString().text());
                m_logger->debug("\tvalues");
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

            void updateLayout(IScriptContext* context) override {

            };

            void draw(IScriptContext* context) override {
                m_logger->never("ScriptElement draw - ***REMOVED***");

            };

            bool isPositionable() const override { return false; }
            IElementPosition* getPosition() const override { return NULL;}

            IScriptHSLStrip* getStrip() const { return m_strip;}
        protected:


            virtual void valuesToJson(JsonObject* json) const{
                m_logger->warn("ScriptElement type %s does not implement valuesToJson",getType());
            }
            virtual void valuesFromJson(JsonObject* json){
                m_logger->warn("ScriptElement type %s does not implement valuesFromJson",getType());
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

    class PositionableElement : public ScriptElement {
        public:
            PositionableElement(const char * type, IScriptHSLStrip* strip, IElementPosition*position) : ScriptElement(type,strip){
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
                m_logger->debug("\tposition");
                positionFromJson(json);
                m_logger->debug("\tdone");
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
                if (isPositionable()) {
                    IElementPosition*pos = getPosition();
                    if (pos == NULL) {
                        m_logger->error("IPositionable does not have a getPosition() %s",getType());
                    } else {
                        pos->fromJson(json);
                    }
                }
            }
            IElementPosition* m_position;
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
                m_logger->debug("ValuesElement.fromJson %s",getType());                

                json->eachProperty([&](const char * name, IJsonElement* val) {
                    m_values.addValue(name,ScriptValue::create(val));
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
            ScriptLEDElement(const char* type) : PositionableElement(type, &m_elementStrip,&m_elementPosition) {

            }

            virtual ~ScriptLEDElement() {
            }

            virtual void draw(IScriptContext*context) {
                IScriptHSLStrip* strip = &m_elementStrip;
                strip->setOverflow(m_position->getOverflow());
                int stripLength = strip->getLength();
                m_logger->never("ScriptLEDElement draw %d",strip->getLength());
                int start = m_position ? m_position->getStart() : 0;
                int count = m_position ? m_position->getCount() : stripLength-start;
                for(int i=0;i<count;i++) {
                    strip->setPosition(i+start);
                    drawLED(context,strip,i);
                }
            }


            void valuesToJson(JsonObject* json) const override {

            }

            void valuesFromJson(JsonObject* json) override {

            }

        protected:
            
            virtual void drawLED(IScriptContext*context,IScriptHSLStrip* strip,int index)=0;
            ElementHSLStrip m_elementStrip;
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