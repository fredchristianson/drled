#ifndef JSON_TEST_H
#define JSON_TEST_H


#include "../lib/test/test_suite.h"
#include "../script/data_loader.h"

#if RUN_TESTS == 1
namespace DevRelief
{
    const char *SIMPLE_SCRIPT = R"script(
            {
                "name": "simple",
                "commands": [
                {
                    "type": "rgb",
                    "red": 250
                }
                ]
            }        
        )script";

    const char *VALUES_SCRIPT = R"script(
        {
            "commands": [
            {
                "type": "values",
                "x": 100,
                "yy": 200,
                "z": 300
            },
            {
                "type": "values",
                "x": "101"
                "y": "var(yy)",
                "timeAnimate": { "start": 10, "end": 200, "speed": 500},
                "posAnimate": { "start": 10, "end": 200},
                "posUnfold": { "start": 10, "end": 200, "unfold":true}
            }
            ]
        }       
        )script";

    const char *POSITION_SCRIPT = R"script(
        {
            "commands": [
            {
                "type": "position",
                "unit": "pixel",
                "start": 5,
                "count": 10,
                "wrap": true,
                "reverse": true,
                "skip": 3,
                "animate": {"speed":20}
            },
            {
                "type": "rgb",
                "red": 255
            }
            ]
        }       
        )script";


    class JsonTestSuite : public TestSuite
    {
    public:
        static TestSuite::TestFn jsonTests[];

        static bool Run(Logger *logger)
        {
#if RUN_JSON_TESTS
            JsonTestSuite test(logger);
            test.run();
            return test.isSuccess();
#else
            return true;
#endif
        }

        void run()
        {

            runTest("testJsonMemoryFree", [&](TestResult &r)
                    { testJsonMemory(r); });

 
                                  
        }

        JsonTestSuite(Logger *logger) : TestSuite("JSON Tests", logger,false)
        {
        }

    protected:
        void testJsonMemory(TestResult &result);
        //void testJsonValue(TestResult &result);
        //void testParseSimple(TestResult &result);
        //void testPosition(TestResult &result);
    };

    void JsonTestSuite::testJsonMemory(TestResult &result)
    {

    }



}
#endif

#endif