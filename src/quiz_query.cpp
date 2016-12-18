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

#include "quiz_config.hpp"
#include "quiz_classes.hpp"  

typedef Call<GradedExam>       QueryCall;
typedef QueryConfig<QueryCall> QueryConf;

enum {
  MODE_STUDENT = 0,
  MODE_SERIAL  = 1
};

constexpr int MAJOR = 2;
constexpr int MINOR = 1;

void usage(char* progname) {
  vector<string> tokens;
  boost::split(tokens, progname, boost::is_any_of("/\\"));
  cout
    << "Usage: " << tokens.back() << " /path/to/config [MODE] [PARAMS]" << R"(

MODE   : serial
INFO   : query for exam composition by serial
PARAMS - Mode 1 : serial quiz_number          
         Return : quiz_name
PARAMS - Mode 2 : serial quiz_number1 : quiz_number2
         Return : list of quiz_name
PARAMS - Mode 3 : quiz_name
         Return : list of quiz_name

MODE   : student
INFO   : query for student outcome
PARAMS - Mode 1 : surname name
         Return : student score and topic suggestion
PARAMS - Mode 2 : surname1 name1 ... surnameN nameN
         Return : list of student score and topic suggestion
PARAMS - Mode 3 : filename
         Return : list of student score and topic suggestion

)";
}

int main(int argc, char** argv) {
  cout << "QuizQuery v" << MAJOR << "." << MINOR << endl;

  string config_name, input_mode;
  char mode;

  // Parsing command line
  if (argc > 3) {
    config_name = argv[1];
    input_mode = argv[2];
    if (input_mode == "student") mode = MODE_STUDENT;
    else if (input_mode == "serial") mode = MODE_SERIAL;
    else {
      cerr << "Unknown mode : " << input_mode << endl;
      usage(argv[0]);
      exit(-2);
    }
  }
  else if (argc == 2) {
    config_name = argv[1];
  }
  else {
    usage(argv[0]);
    exit(-1);
  }

  // Variables and containers
  int serial;
  vector<int> quiz_num;
  string quiz_name = "";

  // Create config object
  QueryCall call;
  QueryConf c(config_name, &call);
  if (!c.check_params()) exit(4);

  // Parse serial file
  if (!call.parse_serial(&c))   exit(6);

  switch (mode) {
    // Student mode
  case MODE_STUDENT: {
    // finish parsing command line (argc > 3 for sure)
    string students_filename;
    vector<string> students_name;
    if (argc >= 5) {
      for (int i = 3; i < argc - 1; i += 2) {
        students_name.push_back(string(argv[i]) + "\t" + argv[i + 1]);
      }
    }
    else if (argc == 4) {
      students_filename = argv[3];
      string line;
      vector<string> tokens;
      ifstream filein(students_filename);
      if (!filein) {
        cerr << "Students file " << students_filename << " not found. Quitting..." << endl;
        exit(-2);
      }
      while (getline(filein, line)) {
        split(tokens, line, is_any_of("\t"), token_compress_off);
        students_name.push_back(tokens.front() + "\t" + tokens.back());
      }
      filein.close();
    }
    else {
      usage(argv[0]);
      exit(-3);
    }

    cout << "Serial file    : " << c.serials_name << endl;
    cout << "Topics file    : " << c.topics_name << endl;
    cout << "Student number : " << students_name.size() << endl;

    // Call operations
    if (!call.parse_grades(&c))   exit(5);
    if (!call.make_topic_map(&c)) exit(7);
    if (!call.make_outcome_map()) exit(8);

    // Display results
    int counter = 0;
    for (auto s : students_name) {
      cout << ++counter << "/" << students_name.size() << ")" << endl;
      auto om = &(call.outcome_map);    // to shorten code
      if (om->count(s)) {
        cout << "STUDENTE  : " << s << endl;
        cout << "SERIALE   : " << om->at(s).serial << endl;
        cout << "VOTO      : " << om->at(s).grade << endl;
        cout << "CORRETTE  : " << om->at(s).correct << endl;
        if (om->at(s).bonus) cout << "BONUS     : " << om->at(s).bonus << endl;
        cout << "BIANCHE   : " << om->at(s).blank << endl;
        cout << "ERRATE    : " << om->at(s).wrong << endl;
        if ( om->at(s).wrong ){
          cout << "RIPASSARE : " << endl;
          for (auto t : om->at(s).topics) cout << "\t- " << t << endl;
        }
        cout << endl;
      }
      else {
        cout << "No match for student : " << s << endl;
      }
    }

    break;
  }
  case MODE_SERIAL:
    // finish parsing command line (argc > 3)
    try {
      serial = stoi(argv[3]);
      for (int i = 4; i < argc; ++i) {
        if (string(argv[i]) == ":") {
          int quiz_min = stoi(argv[i - 1]);
          int quiz_max = stoi(argv[++i]);
          if (quiz_min > quiz_max) {           // swap variables without temp, example qmin = 5 qmax = 2
            quiz_min = quiz_min + quiz_max;    // qmin = 5 + 2 = 7
            quiz_max = quiz_min - quiz_max;    // qmax = 7 - 2 = 5
            quiz_min = quiz_min - quiz_max;    // qmin = 7 - 5 = 2
          }
          quiz_num.clear();
          for (int j = quiz_min; j <= quiz_max; j++) {
            quiz_num.push_back(j);
          }
        }
        else {
          quiz_num.push_back(atoi(argv[i]));
        }
      }
    }
    catch (...) {
      quiz_name = argv[3];
    }
    sort(quiz_num.begin(), quiz_num.end());

    // serial query
    if (quiz_name == "" && quiz_num.size()) {
      // number number(s) mode
      if (call.serials_map.count(serial) == 0) {
        cerr << "Invalid serial : " << serial << endl;
        exit(2);
      }
      if (quiz_num[0] < 1 || quiz_num.back() >= call.serials_map.rbegin()->first) {   // reverse begin, i.e. last map element
        cerr << "Invalid quiz numbers : ";
        for (auto n : quiz_num) cout << n << "  ";
        cout << endl;
        exit(3);
      }

      cout
        << "Serial file    : " << c.serials_name << endl
        << "Serial number  : " << serial << endl
        << "Quiz number(s) : ";
      for (auto n : quiz_num) cout << n << "  ";
      cout << endl;

      cout << "Quiz names     : ";
      for (auto n : quiz_num) cout << call.serials_map[serial].second[n - 1] << "   ";
      cout << endl;
    }
    else if (quiz_name != "" && !quiz_num.size()) {
      // quiz name mode
      cout << "Quiz name      : " << quiz_name << endl;
      vector<pair<int, int>> result;
      for (auto it = call.serials_map.begin(); it != call.serials_map.end(); ++it) {
        for (int i = 0; i < it->second.second.size(); i++) {
          if (it->second.second[i] == quiz_name) result.push_back(make_pair(it->first, i + 1));
        }
      }

      if (result.size() == 0) {
        cerr << "Quiz name " << quiz_name << " not found." << endl;
        exit(4);
      }

      cout << "Ser-Num pairs (" << result.size() << ") : ";
      for (auto r : result) cout << r.first << "-" << r.second << "   ";
      cout << endl;
    }
    else {
      cerr << "Problems with parameters" << endl;
    }
    break;

  default:
    cerr << "Unknown mode : " << mode << endl;
    break;
  }

  return 0;
}
