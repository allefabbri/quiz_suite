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
#ifndef _QUIZ_UTILS_HPP_
#define _QUIZ_UTILS_HPP_

#include <iostream>
#include <random>

using namespace std;

/////////////////////////////// QUIZ_GEN Randomizer
class Rnd {
public:
  default_random_engine engine;
  uniform_int_distribution<int> u_int;
  Rnd(unsigned int s) {
    engine.seed(s);
  };
  int operator() (int min, int max) {
    return uniform_int_distribution<int>{min, max}(engine);
  };
  template<class T>
  std::vector<T> shuffle(std::vector<T> v) {
    std::vector<T> shuffled;
    int i;
    while (v.size() > 0) {
      i = uniform_int_distribution<int>{ 0, (int)v.size() - 1 }(engine);
      shuffled.push_back(v[i]);
      v.erase(v.begin() + i);
    }
    return shuffled;
  }
};



/////////////////////////////// QUIZ_GRADE
double mapping(double x, double old_min, double old_max, double new_min, double new_max) {
  return (x - old_min) / (old_max - old_min)*(new_max - new_min) + new_min;
}



/////////////////////////////// QUIZ_CORRECTIONS
string grade2outcome(vector<double> thresholds, double grade) {
  string outcome;
  if (thresholds.size() == 1) {
    // admitted / rejected
    outcome = "size 1 coming soon";
    if (grade < thresholds[0]) outcome = "Non \\ Ammesso";
    else outcome = "Ammesso";
  }
  else if (thresholds.size() == 2) {
    // admitted / c.u. / rejected
    if (grade < thresholds[0]) outcome = "Non \\ Ammesso";
    else if (grade < thresholds[1]) outcome = "Ammesso \\ con \\ riserva";
    else outcome = "Ammesso";
  }
  else if (thresholds.size() == 4) {
    // a,b,c,d,nc
    outcome = "size 4 coming soon";
  }
  else {
    outcome = "Sconosciuto";
  }
  return outcome;
}



/////////////////////////////// QUIZ_STATS and QUIZ_QUERY
class Outcome {
public:
  int correct, wrong, blank, bonus;
  double grade;
  vector<string> topics;
  Outcome() : correct(0), wrong(0), blank(0), bonus(0) {};

  // TODO vector<pair<int, string>> topics2; // [ { wrong_question_number , "Suggested topic" } ]
};



/////////////////////////////// QUIZ_STATS 
class Question_param {
public:
  int repetitions, correct, wrong, blank;
  std::vector<std::pair<int, int> > pos_map; // { a , b } a = serial, b = question index
  Question_param() : repetitions(0), correct(0), wrong(0), blank(0) {};
};

// Find a way to avoid blowing up of these two
// for about 5k args
double binomial_coeff(int n, int k) {
  double bin = 1;
  for (int i = 0; i < k; i++) {
    bin *= (double)(n - i) / (k - i);
  }
  return bin;
}

double binomial_dist(int n, int k, double p) {
  return binomial_coeff(n, k)*pow(p, k)*pow(1 - p, n - k);
}


#endif
