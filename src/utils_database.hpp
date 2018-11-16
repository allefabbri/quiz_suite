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
#ifndef _QUIZ_DATABASE_HPP_
#define _QUIZ_DATABASE_HPP_

#include <vector>
#include <regex>

#include <boost/filesystem.hpp>

#include <utils_classes.hpp>

using namespace std;
using namespace boost::filesystem;

class QuizDatabase {
public:
  vector<map<string, BaseQuestion>> slots; // map layout [ { { "quiz1_name" , quiz1_obj } , { "quiz2_name" , quiz2_obj } , ... } ]

  std::ofstream * log;
  int * log_counter;
  QuizDatabase(std::ofstream & _log, int & _log_counter){
    log = &_log;
    log_counter = &_log_counter;
  };

  ////////////////////////// Utilities
  // Total size operator
  size_t size() {
    size_t size = 0;
    for (auto &s : slots) size += s.size();
    return size;
  }
  // Total size operator
  size_t slot_size() {
    return slots.size();
  }
  // Average question per slot operator
  double ave_question_per_slot() {
    return ( double(size()) / slots.size() );
  }
  // Possible exam number operator
  size_t possible_exams() {
    size_t res = 1;
    for (auto &s : slots) res *= s.size();
    return res;
  }


  ////////////////////////// IMPORTING
  // Parse db folder and store filenames
  // in slots (according to config regex params)
  // [ [ "slot1_quiz1_file" , "slot1_quiz2_file" ,...  ] , [ "slot2_quiz1_file" , "slot2_quiz2_file" ,...  ] ]
  // also check slots' sizes and fail if any is empty
  template<typename Conf_t> bool populate(const Conf_t & c) {
    bool ret = true;
    path p(c.db_folder);
    slots.resize(c.slot_specs.size());
    try {
      if (exists(p)) {
        if (is_directory(p)) {
          *log << (*log_counter)++ << ") Entering database directory : " << p << endl;
          for (directory_iterator it(p), end; it != end; it++) {
            *log << (*log_counter)++ << ") Analyzing question : " << it->path().filename() << endl;
            for (size_t i = 0; i < c.slot_specs.size(); i++) {
              // populate slot
              for (auto patt : c.slot_specs[i]) {
                regex r(patt);
                if (std::regex_search(it->path().filename().generic_string(), r)) {
                  *log << (*log_counter)++ << ") Question " << it->path().filename() << " matches \"" << patt << "\", stored in SLOT" << i + 1 << endl;
                  *log << (*log_counter)++ << ") Importing question : " << it->path().generic_string() << endl;
                  slots[i][it->path().generic_string()] = BaseQuestion(it->path().generic_string());
                }
              }
            }
          }
        }
        else {
          *log << (*log_counter)++ << ") Database folder is not a directory : " << p << endl;
          ret = false;
        }
      }
      else {
        *log << (*log_counter)++ << ") Database directory not found : " << p << endl;
        ret = false;
      }
    }
    catch (const filesystem_error &ex) {
      cerr << "ERROR during db name parsing : " << ex.what() << endl;
      ret = false;
    }
    // safety check on slots size
    *log << (*log_counter)++ << ") Checking and sorting database names " << endl;
    for (size_t i = 0; i < slots.size(); ++i) {
      if (slots[i].size() == 0) {
        *log << (*log_counter)++ << ") Unable to populate slot #" << i + 1 << endl;
        cerr << "SLOT #" << i + 1 << " is empty. Regex : ";
        for(const auto & r : c.slot_specs[i]) cerr << r << " ";
         cerr << endl << "Quitting..." << endl;
        ret = false;
        break;
      }
    }
    return ret;
  }

  ////////////////////////// QUIZ_GEN
  // Extract a set of fully randomized question
  vector<BaseQuestion> get_rnd_question_set(Rnd & r) {
    vector<BaseQuestion> set;
    // extract ONE question from each slot
    for (const auto & s : slots) {
      vector<BaseQuestion> vslot;
      for (const auto & q : s) vslot.push_back(q.second);
      r.shuffle(vslot);
      set.push_back( r.shuffle(vslot).front() );
    }
    // shuffle question order
    set = r.shuffle(set);
    // shuffle answers
    for (auto &q : set) q.answers = r.shuffle(q.answers);

    return set;
  }
};

#endif
