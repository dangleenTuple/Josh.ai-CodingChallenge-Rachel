#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR
make all
trap "" SIGINT #We disallow an interrupt signal in our current shell
(trap SIGINT; ./CodingChallenge -t) #we make a subshell where we run our program. We allow an interrupt signal in this subshell
pkill -f LightSimulator #In case if the Light Simulator process continues after killing the main process, this would ensure this doesn't happen.
#Otherwise, the newly invoked server that binds to port 8080 might be in use and would return an error.
make clean #Now that we left the subshell, let's clean everything up
