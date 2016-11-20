#include "quiz_lib.hpp"

#define MODE_STUDENT     0
#define MODE_SERIAL      1

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
  string config_name, input_mode;
  char mode;

  // Parsing command line
  if (argc == 2 && string(argv[1]) == "-conf_t") {
    cout << "Generating empty config file named \"query.config\"" << endl;
    std::ofstream config("query.config");
    config
      << "SERIALS     = serials.txt" << endl
      << "GRADES_FILE = voti.txt" << endl
      << "WORK_FOLDER = appello1" << endl << endl;
    config.close();
    exit(-1);
  }
  else if (argc > 3) {
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
  else {
    usage(argv[0]);
    exit(1);
  }

  // Safe CONFIG file parsing
  string grades_name, serials_name, work_folder;
  string key, equal, value;
  ifstream filein(config_name);
  if (!filein) {
    cout << "Configuration file " << config_name << " not found. Quitting..." << endl;
    exit(3);
  }
  while (filein >> key >> equal >> value) {
    if (key == "GRADES_FILE") {
      grades_name = value;
    }
    else if (key == "SERIALS") {
      serials_name = value;
    }
    else if (key == "WORK_FOLDER") {
      work_folder = value;
    }
    else {
      cout << "Key " << key << " unknown. Edit " << config_name << endl;
      exit(3);
    }
  }
  filein.close();

  if (work_folder == "") {
    cout << "WORKING folder unset. Edit " << config_name << endl;
    exit(3);
  }
  if (serials_name == "") {
    cout << "SERIALS file unset. Edit " << config_name << endl;
    exit(3);
  }

  // Variables and containers
  int serial;
  vector<int> quiz_num;
  string quiz_name = "";
  Call call;

  // Import serial file
  call.parse_serial(work_folder + "/" + serials_name);

  // Console info
  cout << "Serial file    : " << serials_name << endl;

  // Runtime branches
  switch (mode) {
  case MODE_STUDENT:
    if (grades_name == "") {
      cout << "GRADES file unset. Edit " << config_name << endl;
      exit(3);
    }
    // finire di parsare command line
    break;

  case MODE_SERIAL:
    // finish parsing command line (argc > 3 for sure)
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
    break;

  default:
    break;
  }

  //    }
  //  }
  //  else if (argc == 3) {
  //    filename = argv[1];
  //    quiz_name = argv[2];





  // Runtime branches

  return 0;
}



