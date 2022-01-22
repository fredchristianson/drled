#ifndef LOGGERS_H
#define LOGGERS_H

#include "./env.h"
#include "./lib/log/logger.h"

namespace DevRelief
{
    

#ifdef DEBUG
Logger AdafruitLogger("AdafruitLED",ADAFRUIT_LED_LOGGER_LEVEL);
Logger AnimationLogger("Animation",ANIMATION_LOGGER_LEVEL);
Logger ApiResultLogger("ApiResult",API_RESULT_LOGGER_LEVEL);
Logger APP_LOGGER("DRLed",APP_LOGGER_LEVEL);
Logger AppStateLogger("AppState",APP_STATE_LOGGER_LEVEL);
Logger AppStateLoaderLogger("AppStateDataLoader",APP_STATE_LOGGER_LEVEL);
Logger ColorLogger("Color",COLOR_LOGGER_LEVEL);
Logger CompoundLogger("CompoundStrip",COMPOUND_STRIP_LOGGER_LEVEL);
Logger ConfigLogger("Config",CONFIG_LOGGER_LEVEL);
Logger ConfigLoaderLogger("Config Loader",CONFIG_LOADER_LOGGER_LEVEL);
Logger DataObjectLogger("DataObject",DATA_OBJECT_LOGGER_LEVEL);
Logger DRBufferLogger("DRBuffer",DR_BUFFER_LOGGER_LEVEL);
Logger GeneratorLogger("JsonGenerator",JSON_GENERATOR_LOGGER_LEVEL);
Logger HSLStripLogger("HSLStrip",HSL_STRIP_LOGGER_LEVEL);
Logger HttpServerLogger("HTTPServer",HTTP_SERVER_LOGGER_LEVEL);
Logger JsonLogger("Json",JSON_LOGGER_LEVEL);
Logger JsonParserLogger("Json",JSON_PARSER_LOGGER_LEVEL);
Logger JsonGeneratorLogger("Json",JSON_GENERATOR_LOGGER_LEVEL);
Logger LEDLogger("LED",LED_LOGGER_LEVEL);
Logger LinkedListLogger("LinkedList",LINKED_LIST_LOGGER_LEVEL);
Logger MemoryLogger("Memory",MEMORY_LOGGER_LEVEL);
Logger PtrListLogger("PtrList",PTR_LIST_LOGGER_LEVEL);
Logger ParserLogger("JsonParser",JSON_PARSER_LOGGER_LEVEL);
Logger ScriptLogger("Script",SCRIPT_LOGGER_LEVEL);
Logger ScriptContainerLogger("ScriptContainer",SCRIPT_CONTAINER_LOGGER_LEVEL);
Logger ScriptElementLogger("ScriptElement",SCRIPT_ELEMENT_LOGGER_LEVEL);
Logger ScriptHSLStripLogger("ScriptHSLStrip",SCRIPT_HSLSTRIP_LOGGER_LEVEL);
Logger ScriptLoaderLogger("ScriptLoader",SCRIPT_LOADER_LOGGER_LEVEL);
Logger ScriptValueLogger("ScriptValue",SCRIPT_VALUE_LOGGER_LEVEL);
Logger ScriptExecutorLogger("ScriptExecutor",SCRIPT_EXECUTOR_LOGGER_LEVEL);
Logger SharedPtrLogger("SharedPtr",SHARED_PTR_LOGGER_LEVEL);
Logger StringLogger("DRString",DRSTRING_LOGGER_LEVEL);
Logger TestLogger("Tests",TEST_LOGGER_LEVEL);

#else
Logger DUMMY_LOGGER;
#define AdafruitLogger DUMMY_LOGGER
#define AnimationLogger DUMMY_LOGGER
#define ApiResultLogger DUMMY_LOGGER
#define APP_LOGGER      DUMMY_LOGGER
#define  AppStateLogger DUMMY_LOGGER
#define StateLoaderLogger   DUMMY_LOGGER
#define CompoundLogger  DUMMY_LOGGER
#define ColorLogger  DUMMY_LOGGER
#define ConfigLogger DUMMY_LOGGER
#define ConfigLoaderLogger DUMMY_LOGGER
#define DataObjectLogger  DUMMY_LOGGER
#define DRBufferLogger  DUMMY_LOGGER
#define GeneratorLogger DUMMY_LOGGER
#define HSLStripLogger DUMMY_LOGGER;
#define HttpServerLogger DUMMY_LOGGER
#define JSONLogger      DUMMY_LOGGER
#define LEDLogger       DUMMY_LOGGER
#define LinkedListLogger DUMMY_LOGGER
#define MemoryLogger    DUMMY_LOGGER
#define  PtrListLogger  DUMMY_LOGGER
#define ParserLogger    DUMMY_LOGGER
#define ScriptLogger    DUMMY_LOGGER
#define ScriptContainerLogger   DUMMY_LOGGER
#define ScriptElementLogger   DUMMY_LOGGER
#define ScriptLoaderLogger DUMMY_LOGGER
#define ScriptValueLogger DUMMY_LOGGER
#define ScriptExecutorLogger    DUMMY_LOGGER
#define SharedPtrLogger  DUMMY_LOGGER
#define StringLogger    DUMMY_LOGGER
#define TestLogger      DUMMY_LOGGER
#endif

}

#endif