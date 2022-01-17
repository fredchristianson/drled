#include "./env.h"
#include "./lib/log/logger.h"
#include "./loggers.h"
#include "./drled_app.h"

#if RUN_TESTS
#include "./test/tests.h"
#endif

using namespace DevRelief;

Logger * drapp_logger=&APP_LOGGER;
Application * app=NULL;

void setup() {
  
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
  if (app) {
    app->loop();
  }
  wdt_reset();
}
