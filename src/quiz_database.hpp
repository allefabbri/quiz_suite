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

#include "quiz_classes.hpp"

using namespace std;
using namespace boost::filesystem;

class QuizDatabase {
public:
  vector<vector<string>> slot_names;
  vector<vector<BaseQuestion>> slot_questions;

  std::ofstream * log;
  int * log_counter;
  QuizDatabase(std::ofstream & _log, int & _log_counter){
    log = &_log;
    log_counter = &_log_counter;
  };

  // Parse db folder and store filenames 
  // in slots (according to config regex params)
  // [ [ "slot1_quiz1_file" , "slot1_quiz2_file" ,...  ] , [ "slot2_quiz1_file" , "slot2_quiz2_file" ,...  ] ]
  template<typename Conf_t> bool populate_names(const Conf_t & c) {
    bool ret = true;
    path p(c.db_folder);
    slot_names.resize(c.slot_specs.size());
    try {
      if (exists(p)) {
        if (is_directory(p)) {
          *log << "INTRUDER " <<(*log_counter)++ << ") Entering database directory : " << p << endl;
          for (directory_iterator it(p), end; it != end; it++) {
            *log << "INTRUDER " << (*log_counter)++ << ") Analyzing question : " << it->path().filename() << endl;
            for (size_t i = 0; i < c.slot_specs.size(); i++) {
              for (auto patt : c.slot_specs[i]) {
                regex r(patt);
                if (std::regex_search(it->path().filename().generic_string(), r)) {
                  *log << "INTRUDER " <<(*log_counter)++ << ") Question " << it->path().filename() << " matches \"" << patt << "\", stored in SLOT" << i + 1 << endl;
                  slot_names[i].push_back(it->path().generic_string());
                }
              }
            }
          }
        }
        else {
          *log << "INTRUDER " <<(*log_counter)++ << ") Database folder is not a directory : " << p << endl;
          ret = false;
        }
      }
      else {
        *log << "INTRUDER " <<(*log_counter)++ << ") Database directory not found : " << p << endl;
        ret = false;
      }
    }
    catch (const filesystem_error &ex) {
      cerr << "ERROR during db name parsing : " << ex.what() << endl;
      ret = false;
    }

    return ret;
  }

};

#endif
