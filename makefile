SRC = src
BIN = bin

FLAGS = -std=c++11

all: gen grade correct stats

gen: $(SRC)/quiz_gen.cpp
	$(CXX) $(FLAGS) -o $(BIN)/quiz_gen.exe $(SRC)/quiz_gen.cpp -lboost_filesystem -lboost_system

grade: $(SRC)/quiz_grade.cpp
	$(CXX) $(FLAGS) -o $(BIN)/quiz_grade.exe $(SRC)/quiz_grade.cpp

correct: $(SRC)/quiz_correct.cpp
	$(CXX) $(FLAGS) -o $(BIN)/quiz_correct.exe $(SRC)/quiz_correct.cpp

stats: $(SRC)/quiz_stats.cpp
	$(CXX) $(FLAGS) -o $(BIN)/quiz_stats.exe $(SRC)/quiz_stats.cpp

clean:
	rm -rf $(BIN)/*
