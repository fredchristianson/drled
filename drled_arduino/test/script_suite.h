#ifndef SCRIPT_TEST_SUITE_H
#define SCRIPT_TEST_SUITE_H


#include "../lib/test/test_suite.h"
#include "../script/data_loader.h"
#include "../script/script.h"

#if RUN_TESTS==1
namespace DevRelief {

const char *HSL_SIMPLE_SCRIPT = R"script(
        {
            "name": "simple",
            "commands": [
            {
                "type": "hsl",
                "hue": 50
            }
            ]
        }        
    )script";    

class DummyStrip : public HSLFilter {
    public:
        DummyStrip(): HSLFilter(NULL) {}
        
};

class ScriptTestSuite : public TestSuite{
    public:

        static bool Run(Logger* logger) {
            ScriptTestSuite test(logger);
            test.run();
            return test.isSuccess();
        }

        void run() {
            runTest("destroyScript",[&](TestResult&r){destroyScript(r);});
        }

        ScriptTestSuite(Logger* logger) : TestSuite("Script Tests",logger){
        }

    protected: 



    void destroyScript(TestResult& result);
};


void ScriptTestSuite::destroyScript(TestResult& result) {
    ScriptDataLoader loader;
    Script* script = loader.parse(HSL_SIMPLE_SCRIPT);

    HSLStrip strip(new DummyStrip());
    script->begin(&strip,NULL);
    script->destroy();
}


}
#endif 

#endif
