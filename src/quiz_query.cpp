#include "quiz_lib.hpp"

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

constexpr char MODE_STUDENT = 0;
constexpr char MODE_SERIAL  = 1;

constexpr char MAJOR = '1';
constexpr char MINOR = '0';

int main(int argc, char** argv) {
  cout << "QuizQuery v" << MAJOR << "." << MINOR << endl;

  string config_name, input_mode;
  char mode;

  // Parsing command line
  if (argc == 2 && string(argv[1]) == "-conf_t") {
    cout << "Generating empty config file named \"query.config\"" << endl;
    std::ofstream config("query.config");
    config
      << "SERIALS     = serials.txt" << endl
      << "GRADES_FILE = voti.txt" << endl
      << "TOPICS_FILE = topics.txt" << endl
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
  string grades_name, serials_name, topics_name, work_folder;
  string key, equal, value;
  ifstream filein(config_name);
  if (!filein) {
    cerr << "Configuration file " << config_name << " not found. Quitting..." << endl;
    exit(3);
  }
  while (filein >> key >> equal >> value) {
    if (key == "GRADES_FILE") {
      grades_name = value;
    }
    else if (key == "SERIALS") {
      serials_name = value;
    }
    else if (key == "TOPICS_FILE") {
      topics_name = value;
    }
    else if (key == "WORK_FOLDER") {
      work_folder = value;
    }
    else {
      cerr << "Key " << key << " unknown. Edit " << config_name << endl;
      exit(3);
    }
  }
  filein.close();

  if (work_folder == "") {
    cerr << "WORKING folder unset. Edit " << config_name << endl;
    exit(3);
  }
  if (serials_name == "") {
    cerr << "SERIALS file unset. Edit " << config_name << endl;
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
  // Student mode
  case MODE_STUDENT: {
    string student_name;
    if (grades_name == "") {
      cerr << "GRADES file unset. Edit " << config_name << endl;
      exit(3);
    }
    if (topics_name == "") {
      cerr << "TOPICS fie unset. Edit " << config_name << endl;
      exit(3);
    }
    // finish parsing command line (argc > 3 for sure)
    if (argc == 5) {
      student_name = string(argv[3]) + "\t" + argv[4];
    }

    cout << "Topics file    : " << topics_name << endl;
    cout << "Student        : " << student_name << endl;

    // Create topic map
    auto topic_map = create_topic_map(work_folder + "/" + topics_name);
    if (topic_map.size() == 0) {
      cerr << "Failed to load topic map, size " << topic_map.size() << endl;
      exit(55);
    }

    // Importing grades file
    string line;
    vector<string> tokens;
    filein.open(work_folder + "/" + grades_name);
    if (!filein) {
      cout << "GRADES file " << grades_name << " not found. Quitting..." << endl;
      exit(4);
    }
    while (getline(filein, line)) {
      trim(line);
      split(tokens, line, is_any_of("\t"), token_compress_on);
      if (tokens.size() > 5) call.exams.push_back(Exam(tokens, 'c'));
    }
    filein.close();

    // Map construction
    map<string, Outcome> call_map;
    for (size_t i = 0; i < call.exams.size(); i++) {
      auto this_exam = &(call_map[call.exams[i].surname + "\t" + call.exams[i].name]);
      this_exam->grade = call.exams[i].grade_d;
      for (size_t j = 0; j < call.exams[i].answers.size(); j++) {
        if (call.exams[i].solutions[j] == '-') {
          this_exam->bonus++;
        }
        else if (call.exams[i].answers[j] == '-') {
          this_exam->blank++;
        }
        else if (call.exams[i].answers[j] == call.exams[i].solutions[j]) {
          this_exam->correct++;
        }
        else {
          this_exam->wrong++;
          // recover suggested topics (if any)
          string question_name = call.serials_map[call.exams[i].serial].second[j];
          this_exam->topics.push_back( topic_map[question_name] );
        }
      }
      auto top = &(this_exam->topics);
      sort(top->begin(), top->end());
      top->erase(unique(top->begin(), top->end()), top->end());
    }

    cout << "VOTO      : " << call_map[student_name].grade << endl;
    cout << "CORRETTE  : " << call_map[student_name].correct << endl;
    if(call_map[student_name].bonus ) cout << "BONUS     : " << call_map[student_name].bonus   << endl;
    cout << "BIANCHE   : " << call_map[student_name].blank   << endl;
    cout << "ERRATE    : " << call_map[student_name].wrong   << endl;
    cout << "RIPASSARE : ";
    for (auto t : call_map[student_name].topics)
      if (t.size())
        cout << endl << "\t- " << t;
      else
        cout << "No match";
    cout << endl;
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
