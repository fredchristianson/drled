#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>
#include <string.h>
#include <stdarg.h>
#include "../../env.h"
#include "../system/board.h"
#include "../util/util.h"
#include "./interface.h"

extern EspBoardClass EspBoard;

namespace DevRelief {



char lastErrorMessage[100];
long lastErrorTime=0;
bool logTestingMessage = false;

class DRLogger : public ILogger {
public:
    DRLogger(const char * name, int level = 100) {
        m_name = NULL;
        m_lastError = NULL;
        m_lastErrorTime = 0;
        setModuleName(name);
    }


    virtual ~DRLogger() {
        Util::freeText(m_name);
    }

    void setModuleName(const char * name) {
        if (name == NULL) {
            name = "???";
        }
        m_name = Util::allocText(name);
    }


    virtual void write(int level, const char * message, va_list args ){
        return;
        ILogConfig* cfg = ILogConfig::Instance();
        if (cfg == NULL) {
            // need LogConfig before doing anything.
            return;
        }
        ILogDestination* dest = cfg->getDestination();
        if (dest == NULL) {
            return;
        }
        ILogFilter* filter = cfg->getFilter();
        if (filter && !filter->shouldLog(level,m_name,message)) {
            return;
        }
        const char* output = message;
        ILogFormatter* formatter = cfg->getFormatter();
        if (formatter) {
            output = formatter->format(m_name,level,message,args);
        }

        if (dest) {
            dest->write(output);
        }
        
       
    }


    void write(int level, const char * message,...) {
        va_list args;
        va_start(args,message);
        write(level,message,args);
    }

    void test(const char * message,...) {
        if (!logTestingMessage) { return;}
        va_list args;
        va_start(args,message);
        write(TEST_LEVEL,message,args);
    }



    void debug(const char * message,...) {

        va_list args;
        va_start(args,message);
        write(DEBUG_LEVEL,message,args);
      
    }



    void info(const char * message,...) {
        va_list args;
        va_start(args,message);
        write(INFO_LEVEL,message,args);
      
    }


    void warn(const char * message,...) {
        va_list args;
        va_start(args,message);
        write(WARN_LEVEL,message,args);
    }


    
    void error(const char * message,...) {
        va_list args;
        va_start(args,message);
        write(ERROR_LEVEL,message,args);
    }

    // mainly for new messages during development.  don't need to turn debug level on and get old debug messages.
    // quick search-replace of "always"->"debug" when done
    void always(const char * message,...) {
        va_list args;
        va_start(args,message);
        write(ALWAYS_LEVEL,message,args);
    }

     void never(const char * message,...) {
     }

    void conditional(bool test, const char * message,...) {
        if (!test) { return;}
        va_list args;
        va_start(args,message);
        write(CONDITION_LEVEL,message,args);
    }

    void errorNoRepeat(const char * message,...) {
        if (message == m_lastError){
            return; //don't repeat the error message
        }
        if (EspBoard.currentMsecs()>m_lastErrorTime+500) {
            return; //don't show any errors too fast - even different message;
        }
        m_lastError = message;
        lastErrorTime = EspBoard.currentMsecs();
        va_list args;
        va_start(args,message);
        write(ERROR_LEVEL,message,args);
    }

    void showMemory(const char * label="Memory") {
        write(INFO_LEVEL,"%s: stack=%d,  heap=%d, max block size=%d, fragmentation=%d",label,EspBoard.getFreeContStack(),EspBoard.getFreeHeap(),EspBoard.getMaxFreeBlockSize(),EspBoard.getHeapFragmentation());
    }

    void showMemory(int level, const char * label="Memory") {
        write(level,"%s: stack=%d,  heap=%d, max block size=%d, fragmentation=%d",label,EspBoard.getFreeContStack(),EspBoard.getFreeHeap(),EspBoard.getMaxFreeBlockSize(),EspBoard.getHeapFragmentation());
    }


private: 
    char * m_name;
    const char * m_lastError;
    int m_lastErrorTime;

};

/*
 * PeriodicLogger has a maximum frequency it will write.  
 * It can be used as any other DRLogger, but all messages
 * will be ignored until the frequency elapses.  
 * This allows something to be writen at the start of every loop()
 * but the write only happens at a specified frequency
 */
class PeriodicLogger : public DRLogger {
    public:
    PeriodicLogger(int maxFrequencyMsecs, const char * name, int level = 100) : DRLogger(name,level) {
        m_maxFrequencyMsecs = maxFrequencyMsecs;
        m_lastWriteTime = 0;
    }

    void write(int level, const char * message, va_list args ) override {
        if (EspBoard.currentMsecs() > m_lastWriteTime+m_maxFrequencyMsecs){
            return;
        }
        m_lastWriteTime = EspBoard.currentMsecs();
        DRLogger::write(level,message,args);
    }
    private:
        int m_maxFrequencyMsecs;
        int m_lastWriteTime;
};

}
#endif