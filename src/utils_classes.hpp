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
#ifndef _QUIZ_CLASSES_HPP_
#define _QUIZ_CLASSES_HPP_

#include <iostream>
#include <fstream>
#include <iomanip>
#include <random>
#include <map>

#include <boost/algorithm/string.hpp>

#include <utils_misc.hpp>

using namespace std;
using namespace boost::algorithm;

/////////////////////////////// QUESTION class
class BaseQuestion {
public:
  string path, name;
  string text;
  vector<pair<string, bool> > answers;    // { answer , correctness status (bool value, 1 true and size-1 false }

  BaseQuestion() {};
  BaseQuestion(const string & filepath) {
    string line;
    vector<string> all_lines(1), tokens;

    std::ifstream filein(filepath);
    while (std::getline(filein, line)) {
      if (line[0] != '.' && line[0] != '#' && line.size() != 0 && line != "\n") {
        all_lines.push_back(line);
      }
    }
    filein.close();

    path = filepath;
    split(tokens, filepath, is_any_of("/\\"), token_compress_on);
    name = tokens.back();
    text = all_lines[0];
    for (size_t k = 1; k < all_lines.size(); k++) {
      answers.push_back(make_pair(all_lines[k], (k == 1) ? true : false));
    }
  }
};



/////////////////////////////// EXAM classes

// An empty exam, for generating
class BaseExam {
public:
  int serial;                                               // for generating/grading/corrections
  vector<BaseQuestion> questions;                           // for generating
  string answers, solutions, name, surname, student;        // for grading
  int grade;                                                // for grading/corrections
  double score, pardon_score, grade_d;                      // for grading/corrections
  vector<string> colors;                                    // for corrections
  BaseExam() {};
};

// A student answered exam, for grading
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

// A graded exam, for correction/stats/query
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



/////////////////////////////// CALL classes

template<typename Exam_t> class Call {
public:
  //////// Vars and containers
  string name;                                              // general purpose
  string date, course, tag, cdl, commission;                // for latex headers
  double score_scale;
  vector<Exam_t> exams;                                     // contains the possible exam types
  map<int, pair<string, vector<string> > > serials_map;     // { serial, { solutions, {questions names, ...} } }


  //////// SERIALS
  // Import single line from SERIALS file
  // line sample : 34 question_name1 question_name2 ... question_nameN ABCD...
  void add_serial(const vector<string> &tokens) {
    serials_map[stoi(tokens[0])].first = tokens.back();
    for (int i = 1; i < tokens.size() - 1; i++) {
      serials_map[stoi(tokens[0])].second.push_back(tokens[i]);
    }
  }
  // Import SERIALS file to map
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
      if (tokens.size() > 6) add_serial(tokens);   // to skip segfault causing lines, if any
    }
    filein.close();
    return ret;
  }


  //////// RESULTS
  // Parse results file and fill exams vector
  // line sample : 18 ABCD... ROSSI MARIO
  template<typename Conf_t> bool parse_results(const Conf_t * confptr) {
    bool ret = true;
    string line, file_path;
    vector<string> tokens;
    file_path = confptr->work_folder + "/" + confptr->results_name;
    ifstream filein(file_path);
    if (!filein) {
      cout << "RESULTS file " << file_path << " not found. Quitting..." << endl;
      ret = false;
    }
    while (getline(filein, line)) {
      trim(line);
      split(tokens, line, is_any_of("\t"), token_compress_on);
      if (tokens.size() == 4) exams.push_back(Exam_t(tokens));
    }
    filein.close();
    return ret;
  }


  //////// GRADES
  // Parse grades file and fill exams vector
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
      if (tokens.size() == 10) exams.push_back(Exam_t(tokens));
    }
    filein.close();
    return ret;
  }


  //////// TOPICS
  // Creates topic map
  // line sample : quiz-1-name Suggested topics
  // map sample : { "quiz-1-name" , "Suggested topics" }
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
    if (topic_map.size() == 0) {
      cerr << "TOPIC null map, size " << topic_map.size() << endl;
      ret = false;
    }
    return ret;
  }


  //////// OUTCOME
  // Creates outcome map
  // layout : { "quiz-1-name" , Outcome object };
  map<string, Outcome> outcome_map;

  bool make_outcome_map() {
    bool ret = true;
    for (const auto & e : exams) {
      Outcome outcome;
      outcome.grade = e.grade_d;
      outcome.serial = e.serial;
      for (size_t j = 0; j < e.answers.size(); j++) {
        if (e.solutions[j] == '-')               outcome.bonus++;
        else if (e.answers[j] == '-')            outcome.blank++;
        else if (e.answers[j] == e.solutions[j]) outcome.correct++;
        else {
          outcome.wrong++;
          // recover suggested topics (if any)
          string question_name = serials_map[e.serial].second[j];
          string topic = topic_map.count(question_name) ? topic_map[question_name] : ("No match for " + question_name);
          outcome.topics.push_back(topic);
        }
      }
      // Remove duplicates
      auto top = &(outcome.topics);
      sort(top->begin(), top->end());
      top->erase(unique(top->begin(), top->end()), top->end());
      // Save element to map
      outcome_map[e.student] = outcome;
    }
    if (outcome_map.size() == 0) {
      cerr << "OUTCOME null map, size " << outcome_map.size() << endl;
      ret = false;
    }
    return ret;
  }
};

#endif
