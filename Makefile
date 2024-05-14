CC=g++ -std=c++20
OBJ_FILES := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: CodingChallenge

clean:
	rm $(OBJ_FILES)
	rm CodingChallenge

%.o: %.cpp
	$(CC) -c $<

CodingChallenge: $(OBJ_FILES)
	$(CC) -o $@ $(OBJ_FILES) -lboost_system -lboost_thread
