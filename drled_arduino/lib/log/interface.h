#ifndef LOGGER_INTERFACE_H
#define LOGGER_INTERFACE_H

#include <cstdio>
#include <string.h>
#include <stdarg.h>
#include "../../env.h"
#include "../system/board.h"

extern EspBoardClass EspBoard;

namespace DevRelief {



enum LogLevel {
    DEBUG_LEVEL=100,
    INFO_LEVEL=80,
    WARN_LEVEL=60,
    ERROR_LEVEL=40,
    ALWAYS_LEVEL=1,
    TEST_LEVEL=-1,  // only & always matches during tests
    NEVER_LEVEL=-2,
    CONDITION_LEVEL=-3
};


class ILogDestination {
    public:
        virtual void write(const char * message)const=0;
};

class ILogFilter {
    public:
        virtual void setLevel(int level)=0;
        virtual void setTesting(bool isUnitTest)=0;
        virtual bool shouldLog(int level, const char* module, const char*message)const = 0;


};


class ILogFormatter {
    public:
        virtual const char * format(const char*moduleName,int level,const char * message,va_list args)const=0;
        virtual void indent()=0;
        virtual void outdent()=0;
};


class ILogConfig {
    public:
        ILogConfig() { m_instance = this;}
        static ILogConfig* Instance() { return m_instance;}

        virtual void indent()=0;
        virtual void outdent()=0;

        virtual void setLevel(LogLevel l)=0;
        virtual int getLevel() const =0;
        virtual bool isDebug() const =0;
        virtual void setTesting(bool isUnitTest)=0;

        virtual ILogFilter* getFilter()const=0;
        virtual ILogFormatter* getFormatter()const = 0;
        virtual ILogDestination* getDestination()const=0;
    protected:
        static ILogConfig* m_instance;
};

class ILogger {
    public:
        virtual void write(int level, const char * message, va_list args )=0;
        virtual void write(int level, const char * message,...)=0;
    
        virtual void test(const char * message,...)=0;  // message only writen during unit test

        virtual void debug(const char * message,...) =0;
        virtual void info(const char * message,...) =0;
        virtual void warn(const char * message,...) =0;
        virtual void error(const char * message,...) =0;
        virtual void always(const char * message,...) =0;
        virtual void never(const char * message,...) =0;
        virtual void conditional(bool test, const char * message,...)=0;
        virtual void errorNoRepeat(const char * message,...)=0;
        virtual void showMemory(const char * label="Memory")=0;
        virtual void showMemory(int level, const char * label="Memory") =0;
    
};

class NullLogger : public ILogger {
    public:
        void write(int level, const char * message, va_list args ) override {}
        void write(int level, const char * message,...) override {}
    
        void test(const char * message,...) override {}

        void debug(const char * message,...)  override {}
        void info(const char * message,...)  override {}
        void warn(const char * message,...)  override {}
        void error(const char * message,...)  override {}
        void always(const char * message,...)  override {}
        void never(const char * message,...)  override {}
        void conditional(bool test, const char * message,...) override {}
        void errorNoRepeat(const char * message,...) override {}
        void showMemory(const char * label="Memory") override{}
        void showMemory(int level, const char * label="Memory")override {}

};

/*
 * A LogIndent object causes log messages to be indented by an additional tab
 * until it is destroyed.
 * {
 *      m_logger->debug("unindented message");
 *      LogIndent indent1;
 *      m_logger->debug("message indented by 1 tab");
 *      if (...) {
 *          LogIndent indent2;
 *          m_logger->debug("message indented by 2 tab");
 *          // indent2 destructor
 *      }
 *      m_logger->debug("message indented by 1 tab");
 * }
*/

class LogIndent {
    public:
        LogIndent() {
            ILogConfig::Instance()->indent();
        }

        ~LogIndent() {
            ILogConfig::Instance()->outdent();
        }
};

ILogConfig* ILogConfig::m_instance = NULL;


}


#if LOGGING_ON==1
// set and declare m_logger member of a class.  
#define SET_LOGGER(logger)  extern ILogger* logger; m_logger= logger
#define DECLARE_LOGGER()    ILogger* m_logger    
// set and declare a logger member of a class with name other than m_logger.  
#define SET_CUSTOM_LOGGER(var,logger) extern ILogger* logger; var = logger
#define DECLARE_CUSTOM_LOGGER(var)  ILogger* var 

// declare and initialize a global variable logger
#define DECLARE_GLOBAL_LOGGER(var,logger)  extern ILogger* logger; ILogger* var =logger
#else
NullLogger nullLogger;
ILogger* m_logger = &nullLogger; // m_logger-> can be used anywhere even though members are not declared
#define SET_LOGGER(LOGGER) /*nothing*/
#define DECLARE_LOGGER     /*nothing*/ 
// standard loggers are named m_logger.  If LOGGIN_ON is not 1, m_logger is a global NullLogger
// custom loggers have names other than m_logger so must be declared members
#define SET_CUSTOM_LOGGER(name,LOGGER) name=LOGGER
#define DECLARE_CUSTOM_LOGGER(name,LOGGER) ILogger* name=&NullLogger

#define DECLARE_GLOBAL_LOGGER(var,logger)  extern ILogger* nullLogger; ILogger* var =&nullLogger

#endif

#endif

