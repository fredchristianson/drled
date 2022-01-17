#ifndef DRSCRIPT_H
#define DRSCRIPT_H

#include "../lib/log/logger.h"
#include "../lib/led/led_strip.h"
#include "../lib/util/list.h"
#include "../lib/json/json.h"
#include "./script_interface.h"
#include "../loggers.h"

namespace DevRelief
{
 
    extern Logger ScriptLogger;
    
    class Script 
    {
    public:
        Script()
        {
            m_logger = &ScriptLogger;
        }

        virtual ~Script() {

        }

        virtual void destroy() {
            delete this;
        }

        void begin(HSLStrip* strip, JsonObject* params) {

        }

        void step() {

        }

    private:
        Logger *m_logger;

    };

   
}
#endif