#include <iostream>
#include <random>
#include <map>

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost::algorithm;

// GENERAL
class Question{
public:
  string path, name;
  string text;
  vector<pair<string, int> > answers;
};

class Exam{
public:
  int serial;                                               // for generating/grading/corrections
  vector<Question> questions;                               // for generating
  string answers, solutions, name, surname, student;        // for grading
  int grade;                                                // for grading/corrections
  double score, pardon_score, grade_d;                      // for grading/corrections
  vector<string> colors;                                    // for corrections
  Exam(){};
  Exam( vector<string> tokens, char mode){
    if( mode == 'c' ){              // 'c' is for CORRECTIONS mode
      if( tokens.size() > 8 ){
        colors.resize(tokens[1].size());
        serial = atoi(tokens[0].c_str());
        answers = tokens[1];
        solutions = tokens[2];
        pardon_score = atof(tokens[4].c_str());
        grade_d = atof(tokens[6].c_str());
        grade = atoi(tokens[7].c_str());
        surname = tokens[8];
        name = tokens[9];
        student = surname + "\t" + name;
      }
    }
    else if( mode == 'g' ){       // 'g' is for GRADING mode
      if( tokens.size() > 3 ){
        serial = atoi(tokens[0].c_str()); 
        answers = tokens[1];
        surname = tokens[2];
        name = tokens[3];
        student = surname + "\t" + name;
        grade = 0;
        score = pardon_score = 0.0;
      }
    }
    else if( mode == 's' ){       // 's' is for STATISTICS mode
      if( tokens.size() > 8 ){
        serial = atoi(tokens[0].c_str());
        answers = tokens[1];
        solutions = tokens[2];
        score = atof(tokens[3].c_str());
        pardon_score = atof(tokens[4].c_str());
        grade = atoi(tokens[7].c_str());
        surname = tokens[8];
        name = tokens[9];
        student = surname + "\t" + name;
      }
    }
    else{
      std::cout << "Exam constructor mode " << mode << " unknown. Quitting..." << std::endl;
      exit(77);
    }
  }
};

class Call{
public:
  string name;                                              // general purpose
  string date, course, tag, cdl, commission;                // for latex headers
  double scale;
  vector<Exam> exams;
  map<int, pair<string, vector<string> > > serials_map;     // { serial, { solutions, {questions names, ...} } }
  void add_serial( vector<string> tokens ){
    serials_map[atoi(tokens[0].c_str())].first = tokens.back();
    for( int i = 1; i<tokens.size()-1; i++){
      serials_map[atoi(tokens[0].c_str())].second.push_back( tokens[i] );
    }
  }
};


// QUIZ_GEN Randomizer
class Rnd{
public:
  default_random_engine engine;
  uniform_int_distribution<int> u_int;
  Rnd(unsigned int s){
    engine.seed(s);
  };
  int operator() (int min, int max){
    return uniform_int_distribution<int>{min,max}(engine);
  };
  template<class T>
  std::vector<T> shuffle(std::vector<T> v){
    std::vector<T> shuffled;
    int i;
    while( v.size() > 0 ){
      i = uniform_int_distribution<int>{0, (int) v.size()-1}(engine);
      shuffled.push_back(v[i]);
      v.erase(v.begin()+i);
    }
    return shuffled;
  }
};


// QUIZ_GRADE
double mapping( double x, double old_min, double old_max, double new_min, double new_max){
  return (x-old_min)/(old_max-old_min)*(new_max-new_min)+new_min;
}


// QUIZ_STATS 
class Question_param {
public:
  int repetitions, correct, wrong, blank;
  std::vector<std::pair<int,int> > pos_map; // { a , b } a = serial, b = question index
  Question_param(){
    repetitions = 0;
    correct = 0;
    wrong = 0;
    blank = 0;
  }
};

double binomial_coeff(int n, int k){
  double bin = 1;
  for(int i= 0; i<k; i++){
    bin *= (double) (n-i)/(k-i);
  }
  return bin;
}

double binomial_dist(int n, int k, double p){
  return binomial_coeff(n,k)*pow(p,k)*pow(1-p,n-k);
}
