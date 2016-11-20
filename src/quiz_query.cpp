#include <iostream>

#include "quiz_lib.hpp"

void usage(char* progname) {
  vector<string> tokens;
  boost::split(tokens, progname, boost::is_any_of("/\\"));
  cout << "Usage: " << tokens.back() << " /path/to/serial_file <ser> <quiz>" << endl
    << "       Returns quiz-name string" << endl << endl
    << "Usage: " << tokens.back() << "  /path/to/serial_file <ser> <quiz1> <quiz2> ... <quizN>" << endl
    << "       Returns quiz1-name quiz2-name ... quizN-name" << endl << endl
    << "Usage: " << tokens.back() << "  /path/to/serial_file <ser> <quiz1> : <quizN>" << endl
    << "       Returns quiz1-name quiz2-name ... quizN-name" << endl << endl
    << "Usage: " << tokens.back() << "  /path/to/serial_file quiz-name" << endl
    << "       Returns <ser1> - <quiz1> <ser2> - <quiz2> ..." << endl << endl;
}

int main(int argc, char** argv) {
  int serial;
  vector<int> quiz_num;
  string quiz_name = "", filename;
  if (argc > 3) {
    filename = argv[1];
    serial = atoi(argv[2]);
    for (int i = 3; i < argc; ++i) {
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
  else if (argc == 3) {
    filename = argv[1];
    quiz_name = argv[2];
  }
  else {
    usage(argv[0]);
    exit(1);
  }
  sort(quiz_num.begin(), quiz_num.end());

  // Vars
  Call call;
  string line;
  vector<string> tokens;

  // Serials file parsing
  call.parse_serial(filename);

  // Runtime branches
  cout << "Serial file    : " << filename << endl;
  if (quiz_name == "") {
    if (call.serials_map.count(serial) == 0) {
      cout << "Invalid serial : " << serial << endl;
      exit(2);
    }
    if (quiz_num[0] < 1 || quiz_num.back() >= call.serials_map.rbegin()->first) {   // reverse begin, i.e. last map element
      cout << "Invalid quiz numbers : ";
      for (auto n : quiz_num) cout << n << "  ";
      cout << endl;
      exit(3);
    }

    cout << "Serial number  : " << serial << endl
      << "Quiz number(s) : ";
    for (auto n : quiz_num) cout << n << "  ";
    cout << endl;

    cout << endl << "Quiz names     : ";
    for (auto n : quiz_num) cout << call.serials_map[serial].second[n - 1] << "   ";
    cout << endl;
  }
  else {
    cout << "Quiz name      : " << quiz_name << endl;
    vector<pair<int, int>> result;
    for (auto it = call.serials_map.begin(); it != call.serials_map.end(); ++it) {
      for (int i = 0; i < it->second.second.size(); i++) {
        if (it->second.second[i] == quiz_name) result.push_back(make_pair(it->first, i + 1));
      }
    }

    if (result.size() == 0) {
      cout << "Quiz name " << quiz_name << " not found." << endl;
      exit(4);
    }

    cout << endl << "Ser-Num pairs (" << result.size() << ") : ";
    for (auto r : result) cout << r.first << "-" << r.second << "   ";
    cout << endl;
  }

  return 0;
}



