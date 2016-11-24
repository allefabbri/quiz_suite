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
#ifndef _QUIZ_LIB2_HPP_
#define _QUIZ_LIB2_HPP_

#include <iostream>
#include <fstream>
#include <iomanip>
#include <random>
#include <map>

#include <boost/algorithm/string.hpp>

#include "config2.hpp"

using namespace std;
using namespace boost::algorithm;

/////////////////////////////// QUESTION class
class BaseQuestion {
public:
  string path, name;
  string text;
  vector<pair<string, int> > answers;
};


class BaseExam {
public:
  int serial;                                               // for generating/grading/corrections
  vector<BaseQuestion> questions;                               // for generating
  string answers, solutions, name, surname, student;        // for grading
  int grade;                                                // for grading/corrections
  double score, pardon_score, grade_d;                      // for grading/corrections
  vector<string> colors;                                    // for corrections
  BaseExam() {};
};

class GradedExam : public BaseExam {
public:
  GradedExam(vector<string> tokens) {
    if (tokens.size() > 8) {
      // sample line : 18 ABCD... ABCD... 27.33 27.33 0.00 28.00 28 ROSSI MARIO
      serial = stoi(tokens[0]);
      colors.resize(tokens[1].size());
      answers = tokens[1];
      solutions = tokens[2];
      score = stod(tokens[3]);
      pardon_score = stod(tokens[4]);
      grade_d = stod(tokens[6]);
      grade = stoi(tokens[7]);
      surname = tokens[8];
      name = tokens[9];
      student = surname + "\t" + name;
    }
  }
};

class PendingExam : public BaseExam {
public:
  PendingExam(vector<string> tokens) {
    if (tokens.size() > 3) {
      // sample line : 22 ABCD... Rossi Mario
      serial = stoi(tokens[0]);
      answers = tokens[1];
      surname = tokens[2];
      name = tokens[3];
      student = surname + "\t" + name;
      grade = 0;
      grade_d = 0.0;
      score = pardon_score = 0.0;
    }
  }
};

/////////////////////////////// 
class Outcome {
public:
  int correct, wrong, blank, bonus;
  double grade;
  vector<string> topics;
  Outcome() : correct(0), wrong(0), blank(0), bonus(0) {};

  // TODO vector<pair<int, string>> topics2; // [ { wrong_question_number , "Suggested topic" } ]
};

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


template<typename Exam_t> class Call2 {
public:
  //////// Vars and containers
  string name;                                              // general purpose
  string date, course, tag, cdl, commission;                // for latex headers
  double score_scale;
  vector<Exam_t> exams;
  map<int, pair<string, vector<string> > > serials_map;     // { serial, { solutions, {questions names, ...} } }

  //////// Methods
  // Import single line from SERIALS file
  // line sample : 34 question_name1 question_name2 ... question_nameN ABCD...
  void add_serial(const vector<string> &tokens) {
    serials_map[stoi(tokens[0])].first = tokens.back();
    for (int i = 1; i < tokens.size() - 1; i++) {
      serials_map[stoi(tokens[0])].second.push_back(tokens[i]);
    }
  }
  // Import SERIALS file
  // layout { serial number, question_name1, question_name2, ... , question_nameN, correct_answer }
  template<typename Conf_t> bool parse_serial(const Conf_t * confptr) {
    bool ret = true;
    string file_path = confptr->work_folder + "/" + confptr->serials_name;
    ifstream filein(file_path);
    if (!filein) {
      cout << "SERIALS file " << file_path << " not found. Quitting..." << endl;
      ret = false;
    }
    string line;
    vector<string> tokens;
    while (getline(filein, line)) {
      trim(line);
      if (line[0] == '%') continue;
      split(tokens, line, is_any_of("\t "), token_compress_on);
      if (tokens.size() > 6) {   // to skip segfault causing lines, if any
        add_serial(tokens);
      }
    }
    filein.close();

    return ret;
  }

  // Parse grades file with different purposes
  // according to the template specification
  // line sample : 18 ABCD... ABCD... 27.33 27.33 0.00 28.00 28 ROSSI MARIO
  template<typename Conf_t> bool parse_grades(const Conf_t * confptr) {
    bool ret = true;
    string line, file_path;
    vector<string> tokens;
    file_path = confptr->work_folder + "/" + confptr->grades_name;
    ifstream filein(file_path);
    if (!filein) {
      cerr << "GRADES file " << file_path << " not found. Quitting..." << endl;
      ret = false;
    }
    while (getline(filein, line)) {
      trim(line);
      split(tokens, line, is_any_of("\t"), token_compress_on);
      if (tokens.size() > 5) exams.push_back(Exam_t(tokens));
    }
    filein.close();
    return ret;
  }

  // Creates topic map
  // 
  map<string, string> topic_map;
  template<typename Conf_t> bool make_topic_map(const Conf_t * configptr) {
    bool ret = true;
    map<string, string> map;
    string file_path = configptr->work_folder + "/" + configptr->topics_name;
    ifstream topic_file(file_path);
    if (!topic_file) {
      cerr << "Topic file " << file_path << " not found." << endl;
      ret = false;
    }
    else {
      string qname, qtopic;
      cout << qname << " " << qtopic << endl;
      while (topic_file >> qname) {
        topic_file >> qtopic;
        string line;
        getline(topic_file, line);
        qtopic += line;
        topic_map[qname] = qtopic;
      }
    }

    return ret;
  }

  // Creates outcome map
  // 
  map<string, Outcome> outcome_map;
  bool make_outcome_map() {
    bool ret = true;
    for (size_t i = 0; i < exams.size(); i++) {

      Outcome outcome;
      outcome.grade = exams[i].grade_d;
      for (size_t j = 0; j < exams[i].answers.size(); j++) {
        if (exams[i].solutions[j] == '-') {
          outcome.bonus++;
        }
        else if (exams[i].answers[j] == '-') {
          outcome.blank++;
        }
        else if (exams[i].answers[j] == exams[i].solutions[j]) {
          outcome.correct++;
        }
        else {
          outcome.wrong++;
          // recover suggested topics (if any)
          string question_name = serials_map[exams[i].serial].second[j];
          outcome.topics.push_back(topic_map[question_name]);
        }
      }
      // Remove in duplicates duplicates
      auto top = &(outcome.topics);
      sort(top->begin(), top->end());
      top->erase(unique(top->begin(), top->end()), top->end());

      outcome_map[exams[i].student] = outcome;
    }
    if (outcome_map.size() == 0) {
      cerr << "TOPIC null map, size " << outcome_map.size() << endl;
      ret = false;
    }

    return ret;
  }
};

#endif
