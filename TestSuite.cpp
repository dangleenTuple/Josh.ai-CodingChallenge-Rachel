#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <boost/thread/thread.hpp>
#include <fstream>
#include <string>

#include "cpp-httplib/httplib.h"
#include "TestSuite.h"
using namespace std;

/*
This test suite is a quick way to test out the program.
The test suite is setup to run in conjunction with CodingChallenge.cpp

SPAN_POST_TEST and TEST_API_CALLS fire a few API calls to cover all bases.
The output is then dumped into a file which is compared to another file with the "expected" resukts.

We need to supply the the correct XPECTED_RESULTS.txt based on the 'default_lights.json' to ensure the test passes.
*/
void POST_API_CALLS(httplib::Client& cli)
{
    int index = 10;
    while(index != 0)
    {
        cli.Post("/lights", "{\"name\":\"Bathroom Light\",\"room\":\"Bathroom\",\"on\":true,\"brightness\":200}", "application/json");
        index--;
    }
    sleep(10); //Let's wait for one interval to pass to give the main process time to create its output file
}

void TEST_API_CALLS(httplib::Client& cli)
{
    //Delete Pantry Light
    cli.Delete("/lights/2c85bb59-c136-49f1-a429-f02f52b6c765");

    //Turn off Kitchen Chandelier`
    cli.Put("/lights/af80e5c2-b235-471d-8df9-490703699eda", "{\"on\":true}", "application/json");

    //Set brightness for Office Sconce 2 to 100
    cli.Put("/lights/edc1b691-a5af-4524-9b57-80341d90bfa2", "{\"brightness\":100}", "application/json");

    sleep(10); //Let's wait for one interval to pass to give the main process time to create its output file
}

bool compareFiles()
{
    ifstream expectedResults, testResults;
    testResults.open("TEST_RESULTS.txt");
    if(!testResults)
    {
        cout << "RESULT FILE NOT CREATED" << endl;
        return false;
    }

    expectedResults.open("EXPECTED_RESULTS.txt");
    if(!expectedResults)
    {
        cout << "TEST COMPARISON FILE WAS NOT OPENED" << endl;
        return false;
    }

    //Let's compare the files and close them
    stringstream buff, buffRes;
    buff << expectedResults.rdbuf();
    buffRes << testResults.rdbuf();
    expectedResults.close();
    testResults.close();
    return buff.str().compare(buffRes.str());
}

void StartTests()
{
    httplib::Client cli("localhost", 8080); // host, port

    POST_API_CALLS(cli);
    cout << "POST_API_CALLS FINISHED API CALLS" << endl;

    TEST_API_CALLS(cli);
    cout << "TEST_API_CALLS FINISHED API CALLS" << endl;
    bool res = compareFiles();

    if(res)
        cout << "TESTS PASSED" << endl;
    else
        cout << "TESTS FAILED" << endl;

}
