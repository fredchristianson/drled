#include "./env.h"
#include "./lib/application.h"
#include "./lib/log/logger.h"
#include "./loggers.h"
#include "./drled_app.h"

#if RUN_TESTS
#include "./test/tests.h"
#endif

using namespace DevRelief;

auto logConfig = new LogConfig(new LogSerialDestination());

Application * app=NULL;

void setup() {
  DRLogger logger("test",100);
  logger.always("running");
#if RUN_TESTS==1
  if (!Tests::Run()) {
    DevRelief::AppLogger->error("Tests failed.  Not running application.");
  }
#endif  
  DevRelief::AppLogger->info("creating app");
  app = new DRLedApplication();
  DevRelief::AppLogger->info("created app");
  wdt_enable(WDTO_4S);
}


void loop() {
  
  if (app) {
    app->loop();
  }
  wdt_reset();
}
