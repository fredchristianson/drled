#include "./env.h"
#include "./lib/application.h"
#include "./lib/log/logger.h"
#include "./loggers.h"
#include "./drled_app.h"

#if RUN_TESTS
#include "./test/tests.h"
#endif

using namespace DevRelief;

LogSerialDestination serialLog;


DECLARE_GLOBAL_LOGGER(drapp_logger,DevRelief::AppLogger);
Application * app=NULL;

void setup() {
  
serialLog.write("DRLED setup");
#if RUN_TESTS==1
  if (!Tests::Run()) {
    drapp_logger->error("Tests failed.  Not running application.");
  }
#endif  
  drapp_logger->info("creating app");
  app = new DRLedApplication();
  drapp_logger->info("created app");
  wdt_enable(WDTO_4S);
}


void loop() {
  serialLog.write("DRLED loop");
  if (app) {
    app->loop();
  }
  wdt_reset();
}
