SRC = src
BIN = bin

FLAG =-std=c++11

all : quiz_gen quiz_grade quiz_correct quiz_stats quiz_query

quiz_gen : $(SRC)/quiz_lib.hpp $(SRC)/quiz_gen.cpp 
	$(CXX) $(FLAG) -o $(BIN)/quiz_gen.exe $(SRC)/quiz_gen.cpp -lboost_filesystem -lboost_system

quiz_grade : $(SRC)/quiz_lib.hpp $(SRC)/quiz_grade.cpp 
	$(CXX) $(FLAG) -o $(BIN)/quiz_grade.exe $(SRC)/quiz_grade.cpp

quiz_correct : $(SRC)/quiz_lib.hpp $(SRC)/quiz_correct.cpp
	$(CXX) $(FLAG) -o $(BIN)/quiz_correct.exes $(SRC)/quiz_correct.cpp

quiz_stats : $(SRC)/quiz_lib.hpp $(SRC)/quiz_stats.cpp
	$(CXX) $(FLAG) -o $(BIN)/quiz_stats.exe $(SRC)/quiz_stats.cpp

quiz_query : $(SRC)/quiz_lib.hpp $(SRC)/quiz_query.cpp
	$(CXX) $(FLAG) -o $(BIN)/quiz_query.exe $(SRC)/quiz_query.cpp

clean : 
	rm -rf $(BIN)/*
