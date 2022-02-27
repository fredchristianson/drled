#include "./env.h"
#include "./lib/application.h"
#include "./lib/log/logger.h"
#include "./loggers.h"
#include "./drled_app.h"

#if RUN_TESTS
#include "./test/tests.h"
#endif

using namespace DevRelief;

#if LOGGING_ON==1
auto logConfig = new LogConfig(new LogSerialDestination());
#else 
auto logConfig = new NullLogConfig();
#endif 

Application * app=NULL;


void setup() {
#if RUN_TESTS==1
  if (!Tests::Run()) {
    DevRelief::AppLogger->error("Tests failed.  Not running application.");
  }
#endif  
  app = new DRLedApplication();
  wdt_enable(WDTO_4S);
}


void loop() {
  
  if (app) {
    app->loop();
  }
  wdt_reset();
}
