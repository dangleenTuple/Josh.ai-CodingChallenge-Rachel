#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <boost/thread/thread.hpp>
#include <string>
#include <algorithm>
#include <fstream>

#include "cpp-httplib/httplib.h"
#include "json/single_include/nlohmann/json.hpp"
#include "TestSuite.h"

using namespace std;
using nlohmann::literals::operator ""_json;
using namespace nlohmann;

map<string,string> oldMap;
ofstream testFile;

/*startLightSimulator : Function to run our Light Simulator executable (on Linux)

Parameters: None
Return value: Int (status)
*/
int startLightSimulator() {
   int pid, status;

   if (pid = fork()) {
       // pid != 0: this is the parent process (i.e. the process of the test)
       waitpid(pid, &status, 0); // wait for the child to exit.
   } else {
       /* pid == 0: this is the child process. */
       const char executable[] = "./LightSimulator_linux_amd64";
       // Run 'man 3 exec' to read about the below command
       // execl takes the arguments as parameters. execv takes them as an array
       // No parameters needed.
       execl(executable, executable);

       /* exec does not return unless the program couldn't be started. 
          If we made it to this point, that means something went wrong.
          when the child process stops, the waitpid() above will return.
       */
       cout << "FAILED TO START LIGHT SIMULATOR EXECUTABLE" << endl;
   }
   return status; // this should be the parent process if there were no issues
}

/*JSONParseMap : Function to parse a JSON to a map. We also need to do extra get calls to get 
more information about the state.

Parameters: One JSON Object and one reference to a client socket (to do GET calls)
Return Value: Return the resulting parsed map.
*/
map<string,string> JSONParseMap(json j, httplib::Client& cli)
{
    map<string,string> currS;
    for (const auto curr : j.items())
    {
        string requestState = to_string(curr.value().at("id"));
        //When we fetch the id, the json returns an id with quotations. The substr is here to chop them off and use the id in our state request.
        string id = requestState.substr(1, requestState.length()-2);

        //Let's get the complete state of each entry as we load everything into our map.
        auto res = cli.Get("/lights/" + id);
        currS.insert({id, res->body});
    }

    return currS;
}

/* checkEqual : Function to deeply check if two maps are equal. 
(Specifically key:string, value:string. Improvement: Make the maps type agnostic)

Parameters: Two maps with strings as the key AND value
Return value: bool which indicates whether or not they are equal.
*/
bool checkEqual(map<string,string> j, map<string,string> newJ)
{
    for (auto it = j.begin(); it != j.end(); ++it )
    {
        if(newJ.find(it->first) == newJ.end() || newJ[it->first] != it->second)
            return false;
    }
    return newJ.size() == j.size();
}

/* PrintDifference : We need to determine if an element has been added, deleted, and/or changed. Once we determine that, we print the difference.
An edge case is if multiple changes occurred between a short poll interval.
Because of this, we need to always check the entire map for all of the changes.

Parameters: One map (key:string, value:string) to compare to our current global map.
bool testing to indicate we are in test mode or not.

Return value: None
*/
void PrintDifference(map<string,string> s, bool testing)
{
    string serverChanges = "";
    //Loop to check for deleted elements
    for(auto it = oldMap.begin(); it != oldMap.end(); ++it )
    {
        if(s.find(it->first) == s.end())
        {
            string name;
            stringstream findName(it->second);
            while(getline(findName,name))
            {
                int pos = name.find("name");
                if(pos != string::npos)
                {
                    name = name.substr(pos+8, name.length()-pos-10);
                    break;
                }
            }
            serverChanges += name + " (" + it->first + ") has been removed\n";
        }
    }

    //Loop to check if either an element has been changed and/or added
    for(auto it = s.begin(); it != s.end(); ++it )
    {
        if(oldMap.find(it->first) == oldMap.end())
        {
            serverChanges += it->second + "\n";
            continue;
        }
        
        if(oldMap[it->first] != it->second)
        {
            string newLine, oldLine;
            stringstream newBody(it->second);
            stringstream oldBody(oldMap[it->first]);
            while(getline(newBody, newLine) && getline(oldBody, oldLine))
            {
                if(newLine == oldLine)
                    continue;
                
                serverChanges += "{\n    \"id\": \"" + it->first + "\"\n" + newLine + "\n}\n";
            }

        }
    }

    cout << serverChanges;
    if(testing && testFile.is_open())
    {
        testFile << serverChanges;
    }

}

/*Our Client will perform the Short Polling technique. After every certain interval, the client will request for an updated version
of the complete list of elements that the server has. Then, it will print out any differences.

Parameters: bool testing to indicate we are in test mode or not.
Return value: None
*/
void StartClient(bool testing)
{
    httplib::Client cli("localhost", 8080); // host, port

    
    while(true)
    {
        if(auto newResponse = cli.Get("/lights"))
        {
            newResponse->status;
            string oldBody = newResponse->body;
            json j;

            try {
                j = json::parse(oldBody);
            } catch (json::parse_error& error){
                cerr << "Parse error at byte: " << error.byte << endl;
                continue;
            }
  

            //let's map our initial map to compare to for any changes
            oldMap = JSONParseMap(j, cli);

            //Print the initial map
            for(auto it = oldMap.begin(); it != oldMap.end(); it++)
                cout << it->second << endl;
            
            if(testing)
            {
                testFile.open("TEST_RESULTS.txt");

                if(!testFile)
                    cout << "FAILED TO OPEN FILE" << endl;
            }

            while(true)
            {
                sleep(10); //Let the other process go first before we send another request
                if(newResponse = cli.Get("/lights"))
                {
                    newResponse = cli.Get("/lights");

                    //Convert the body we received to a map
                    string newBody = newResponse->body;
                    json jNew;

                    try {
                        jNew = json::parse(newBody);
                    } catch (json::parse_error& error){
                        cerr << "Parse error at byte: " << error.byte << endl;
                        continue;
                    }
                    auto newMap = JSONParseMap(jNew, cli);

                    if(checkEqual(oldMap, newMap)){
                        continue;
                    }else {
                        PrintDifference(newMap, testing);

                        //When we're done processing, let's map the global map to the new one.
                        oldMap = newMap;
                    }
                } else {
                    break;
                }

            }
        } else {
            //Immediately reconnect if it failed to start
            auto err = newResponse.error();
            cout << "HTTP error: " << httplib::to_string(err) << endl;
            boost::thread s(&startLightSimulator);
            sleep(5); //Let's give our light simulator program a bit to start
        }
    }
}

int main(int argc, char *argv[])
{
    bool testing = false;
    if(argc > 1 && (strcmp(argv[1], "-t") == 0 || strcmp(argv[1], "-test") == 0))
        testing = true;
    boost::thread s(&startLightSimulator);
    sleep(5); //Let's give our light simulator program a bit to start
    boost::thread c(&StartClient, testing);
    if(testing)
    {
        sleep(30); //Before we start testing, let's give the client some time to print the default values.
        cout << "STARTING TESTS" << endl;
        StartTests();
        testFile.close();
        return 0;
    }
    c.join();
    s.join();
    return 0;
}