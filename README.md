# INSTRUCTIONS (MY PROGRAM ONLY WORKS FOR LINUX)

To run the program manually (without running the test suite):

chmod a+x LightSimulator_linux_amd64

chmod +x run_codingchallenge.sh

./run_codingchallenge.sh


To run the program with the test suite (which exits upon completion):

chmod a+x LightSimulator_linux_amd64

chmod +x run_tests.sh

./run_tests.sh


# SHORT POLLING APPROACH

Since the problem statement involves working with an already established server (via an executable), the best solution to
maintain any changes that happen to the server is to perform a client-based short polling approach.

Essentially, after a specified interval, the client will send a GET request to gather all of the values the server currently
holds. Then, we'll parse this data, throw it all into a map, then check for any changes (as per the API documentation, this
could be PUT, POST, DELETE). 

If we had the job to design the entire server-client relationship, other approaches could've been more suitable such as,

### LONG POLLING
### SERVER-SENT EVENTS
### UTILIZING WEBSOCKETS
### API CALLS THAT SEND UPDATES TO THE CLIENTS (plus using one of the above approaches)


# EDGE CASES

Since Short Polling is only utilizing the already existing APIs that the server supplies, we can get some funky results
working around the API calls that happen every interval.

Perhaps a bunch of changes occur right after we poll. Then we have to wait until the next interval to see these changes. This
makes short polling pretty slow in comparison to other approaches.

Either way, my program handles the cases where a bunch of changes occur in one interval, failed HTTP requests, different types
of HTTP requests in a given interval, JSON Parsing errors, and if we lose a connection to the server.

# TEST SUITE

The test suite only contains a couple of unit tests that send different types of HTTP requests that will change the values
in the server. The coding challenge program already runs the executable for the light simulator server in a child process,
so the only difference is it will run this test suite after we establish a connection to the server and print out
all of the default values and their states.


# THIRD PARTY LIBRARIES USED

[C++11 header-only HTTP/HTTPS client library](https://github.com/yhirose/cpp-httplib)
[JSON for Modern C++](https://github.com/nlohmann/json)

# PROBLEM STATEMENT

[Josh.ai C++ Coding Challenge](https://github.com/jstarllc/JoshCodingChallenge?tab=readme-ov-file)

