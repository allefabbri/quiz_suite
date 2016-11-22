// Copyright 2015, Alessandro Fabbri

/************************************************************************
* This program is free software: you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation, either version 3 of the License, or     *
* (at your option) any later version.                                   *
*                                                                       *
* This program is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
* GNU General Public License for more details.                          *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with this program.  If not, see <http://www.gnu.org/licenses/>. *
************************************************************************/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <random>
#include <map>

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost::algorithm;


/////////////////////////////// QUESTION class
class Question{
public:
  string path, name;
  string text;
  vector<pair<string, int> > answers;
};


/////////////////////////////// EXAM class
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
        // sample line : 22 ABCD... Rossi Mario
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
        grade_d = atof(tokens[6].c_str());
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


/////////////////////////////// CALL class
class Call{
public:
  // Vars and containers
  string name;                                              // general purpose
  string date, course, tag, cdl, commission;                // for latex headers
  double score_scale;
  vector<Exam> exams;
  map<int, pair<string, vector<string> > > serials_map;     // { serial, { solutions, {questions names, ...} } }

  // Methods
  void add_serial( vector<string> tokens ){
    serials_map[atoi(tokens[0].c_str())].first = tokens.back();
    for( int i = 1; i<tokens.size()-1; i++){
      serials_map[atoi(tokens[0].c_str())].second.push_back( tokens[i] );
    }
  }
  void parse_serial(string filename) {
    ifstream filein(filename);
    if (!filein) {
      cout << "SERIALS file " << filename << " not found. Quitting..." << endl;
      exit(5);
    }
    string line;
    vector<string> tokens;
    while (getline(filein, line)) {
      trim(line);
      if (line[0] == '%') continue;
      split(tokens, line, is_any_of("\t "), token_compress_on);
      if (tokens.size() > 6) {   // to skip segfault causing lines, if any
        this->add_serial(tokens);
      }
    }
    filein.close();
  }
};


/////////////////////////////// QUIZ_GEN Randomizer
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


/////////////////////////////// QUIZ_GRADE
double mapping( double x, double old_min, double old_max, double new_min, double new_max){
  return (x-old_min)/(old_max-old_min)*(new_max-new_min)+new_min;
}


/////////////////////////////// QUIZ_CORRECTIONS
string grade2outcome(vector<double> thresholds, double grade) {
  string outcome;
  if (thresholds.size() == 1) {
    // admitted / rejected
    outcome = "size 1 coming soon";
  }
  else if (thresholds.size() == 2) {
    if (grade < thresholds[0]) outcome = "Non \\ Ammesso";
    else if (grade < thresholds[1]) outcome = "Ammesso \\ con \\ riserva";
    else outcome = "Ammesso";
  }
  else if (thresholds.size() == 4) {
    // a,b,c,d,nc
    outcome = "size 4 coming soon";
  }
  else {
    outcome = "size unknown coming soon";
  }
  return outcome;
}


/////////////////////////////// QUIZ_STATS 
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


/////////////////////////////// QUIZ_QUERY
class Outcome {
public:
  int correct, wrong, blank, bonus;
  double grade;
  vector<string> topics;
};

// creates a topic map { "question name" , "Suggested topics" }
map<string, string> create_topic_map(string filename) {
  map<string, string> map;
  ifstream topic_file(filename);
  if (!topic_file) {
    cerr << "Topic file " << filename << " not found." << endl;
  }
  else {
    string qname, qtopic;
    cout << qname << " " << qtopic << endl;
    while (topic_file >> qname) {
      topic_file >> qtopic;
      string line;
      getline(topic_file, line);
      qtopic += line;

      map[qname] = qtopic;
    }
  }

  return map;
}
