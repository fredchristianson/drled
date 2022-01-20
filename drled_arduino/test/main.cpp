#define FAKE_ARDUINO
#include <cstdlib>
#include <stddef.h>
#include "../lib/log/logger.h"

#define RUN_TESTS 1
#include "./tests.h"

using namespace  DevRelief; 

Logger TestLogger("TestMain",DEBUG_LEVEL);


int main() {
    Tests::Run();
}