SRC = src
BIN = bin

FLAG =-std=c++11

all : dirs quiz_gen quiz_grade quiz_correct quiz_stats quiz_query

quiz_gen : $(SRC)/quiz_config.hpp $(SRC)/quiz_utils.hpp $(SRC)/quiz_classes.hpp $(SRC)/latex_utils.hpp $(SRC)/quiz_gen.cpp 
	$(CXX) $(FLAG) -o $(BIN)/quiz_gen.exe $(SRC)/quiz_gen.cpp -lboost_filesystem -lboost_system

quiz_grade : $(SRC)/quiz_config.hpp $(SRC)/quiz_utils.hpp $(SRC)/quiz_classes.hpp $(SRC)/quiz_grade.cpp 
	$(CXX) $(FLAG) -o $(BIN)/quiz_grade.exe $(SRC)/quiz_grade.cpp

quiz_correct : $(SRC)/quiz_config.hpp $(SRC)/quiz_utils.hpp $(SRC)/quiz_classes.hpp $(SRC)/latex_utils.hpp $(SRC)/quiz_correct.cpp
	$(CXX) $(FLAG) -o $(BIN)/quiz_correct.exe $(SRC)/quiz_correct.cpp

quiz_stats : $(SRC)/quiz_config.hpp $(SRC)/quiz_utils.hpp $(SRC)/quiz_classes.hpp $(SRC)/quiz_stats.cpp
	$(CXX) $(FLAG) -o $(BIN)/quiz_stats.exe $(SRC)/quiz_stats.cpp

quiz_query : $(SRC)/quiz_config.hpp $(SRC)/quiz_utils.hpp $(SRC)/quiz_classes.hpp $(SRC)/quiz_query.cpp
	$(CXX) $(FLAG) -o $(BIN)/quiz_query.exe $(SRC)/quiz_query.cpp

dirs :
	mkdir -p $(BIN)

clean : 
	rm -rf $(BIN)/*
