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

typedef Call<PendingExam>      GradeCall;
typedef GradeConfig<GradeCall> GradeConf;

constexpr int MAJOR = 2;
constexpr int MINOR = 1;

void usage(char * progname) {
  vector<string> tokens;
  boost::split(tokens, progname, boost::is_any_of("/\\"));
  cout << "Usage: " << tokens.back() << " path/to/config" << endl;
  cout << "      path/to/config must be a valid config file" << endl;
  cout << endl;
  cout << "Usage: " << tokens.back() << " path/to/config -v [student_surname]" << endl;
  cout << "      enable verbose grading for [student_surname]" << endl;
  cout << endl;
  cout << "Usage: " << tokens.back() << " -conf_t" << endl;
  cout << "      generate a dummy config file" << endl;
}

int main(int argc, char ** argv) {
  cout << "QuizGrader v" << MAJOR << "." << MINOR << endl;

  // Parse command line
  string config_name, target_student;
  bool verbose_correction = false;
  if (argc == 2) {
    config_name = argv[1];
  }
  else if (argc == 4) {
    config_name = argv[1];
    target_student = argv[3];
    verbose_correction = true;
  }
  else {
    usage(argv[0]);
    exit(2);
  }

  // Variables and containers
  string line, file_path;
  vector<string> tokens;
  map<int, vector<int> > bugs_map, healthy_map;

  // Create config and call object
  GradeCall call;
  GradeConf c(config_name, &call);
  if (!c.check_params()) exit(4);

  // Import SERIALS file
  if (!call.parse_serial(&c)) exit(5);

  // Importing RESULTS file
  if (!call.parse_results(&c)) exit(6);

  // Associate to each exam its solutions
  for (auto &exam : call.exams) exam.solutions = call.serials_map[exam.serial].first;

  // Calculating grade scales and question points
  double grade_min, grade_max, score_min, score_max;
  double correct_score, wrong_score, blank_score;
  int question_number = (int)call.exams[0].answers.size();
  score_max = call.score_scale;
  score_min = -call.score_scale / double(c.choices_number - 1);
  grade_min = 0;
  grade_max = 30;
  correct_score = score_max / question_number;
  wrong_score = -correct_score / (c.choices_number - 1);
  blank_score = 0.0;

  // BUGGED mode specific operations
  if (c.is_call_bugged) {
    // Import WRONG QUESTION list file
    vector<string> error_list;
    ifstream filein(c.work_folder + "/" + c.bugs_name);
    if (!filein) {
      cout << "BUGS file " << c.bugs_name << " not found. Quitting..." << endl;
      exit(4);
    }
    while (filein >> line) error_list.push_back(line);
    filein.close();

    // Creating BUGS and HEALTHY MAP
    for (const auto & s : call.serials_map) {
      for (int i = 0; i < s.second.second.size(); i++) {
        for (auto err : error_list) {
          if (s.second.second[i] == err) {
            bugs_map[s.first].push_back(i);                               // storing bugs positions            
          }
        }
      }
      if (bugs_map.count(s.first) == 0) healthy_map[s.first].push_back(1);
    }

    // Dump LOG to file
    string flag;
    ofstream fileout(c.work_folder + "/" + c.grade_report_basename + ".log");
    if (!fileout) {
      cout << "LOG file " << c.grade_report_basename << ".log impossible to create. Quitting..." << endl;
      exit(6);
    }
    fileout << endl << "BUGGED question : " << error_list.size() << endl;
    for (auto e : error_list) fileout << " - " << e << endl;
    fileout << endl << endl;
    fileout << "Number of serials : " << call.serials_map.size() << endl << endl;
    string ending;
    for (const auto & s : call.serials_map) {
      if (bugs_map.count(s.first) == 0) {
        ending = "------ HEALTHY exam\n";
        flag = "-     ";
        for (int i = 0; i < s.second.second.size(); i++) {
          fileout << flag << "\t-\tSerial " << s.first << "\tQuestion " << i + 1 << "\t\t" << s.second.second[i] << endl;
        }
      }
      else {
        ending = "------ BUGGED exam\n";
        for (int i = 0; i < s.second.second.size(); i++) {
          for (int index = 0; index < bugs_map.find(s.first)->second.size(); index++) {
            if (bugs_map.find(s.first)->second[index] == i) {
              flag = "BUGGED";
              break;
            }
            else {
              flag = "-     ";
            }
          }
          fileout << flag << "\t-\tSerial " << s.first << "\tQuestion " << i + 1 << "\t\t" << s.second.second[i] << endl;
        }
      }
      fileout << ending << endl;
    }
    fileout.close();

    // Dumping BUGS and HEALTHY MAP to file
    fileout.open(c.work_folder + "/" + c.grade_report_basename + ".bugs_map");
    if (!fileout) {
      cout << "BUGS MAP file " << c.grade_report_basename << ".bugs_map impossible to create. Quitting..." << endl;
      exit(7);
    }
    fileout << "BUGGED serials  : " << bugs_map.size()
      << " (" << fixed << setprecision(2) << bugs_map.size() / double(call.serials_map.size()) << " %)" << endl
      << "HEALTHY serials : " << healthy_map.size()
      << " (" << fixed << setprecision(2) << healthy_map.size() / double(call.serials_map.size()) << " %)" << endl << endl;
    fileout << "BUGS map :" << endl;
    for (const auto & b : bugs_map) {
      fileout << std::setw(3) << b.first << "  ->  " << b.second.size() << "\t{ ";
      for (auto i : b.second) {
        fileout << setw(2) << i + 1 << " " << call.serials_map[b.first].first[i] << ((i == (b.second).back()) ? " }\n" : " , ");
      }
    }
    fileout << endl << endl << "HEALTHY map :" << endl;
    for (const auto & h : healthy_map) {
      fileout << std::setw(3) << h.first << "  ->  " << " ok " << endl;
    }
    fileout.close();

    // Amending answers and solutions
    for (auto & exam : call.exams) {
      if (bugs_map.count(exam.serial) != 0) {
        for (auto & i : bugs_map[exam.serial]) {
          exam.answers[i] = '-';
          exam.solutions[i] = '-';
        }
      }
    }
  }         // end of BUGGED mode specific operations


// GRADING loop
  for (auto &exam : call.exams) {
    double last_score;
    for (size_t i = 0; i < exam.answers.size(); i++) {
      last_score = exam.pardon_score;
      if (exam.solutions[i] == '-') {
        exam.pardon_score += correct_score;
      }
      else if (exam.answers[i] == exam.solutions[i]) {
        exam.pardon_score += correct_score;
      }
      else if (exam.answers[i] == '-') {
        exam.pardon_score += blank_score;
      }
      else {
        exam.pardon_score += wrong_score;
      }
      if (verbose_correction) {
        if (exam.surname == target_student) {
          if (i == 0) cout << "VERBOSE GRADING - " << exam.student << endl;
          cout << setw(3) << exam.serial << " " << setw(2) << i + 1 << " " << exam.answers[i] << " "
            << exam.solutions[i] << " "
            << fixed << setprecision(2) << setw(6) << exam.pardon_score << "  "
            << fixed << setprecision(2) << setw(6) << exam.pardon_score - last_score << endl;
        }
      }
    }
    if (bugs_map.count(exam.serial) != 0) {
      exam.score = exam.pardon_score - bugs_map.find(exam.serial)->second.size()*correct_score;
    }
    else {
      exam.score = exam.pardon_score;
    }
    exam.grade_d = mapping(exam.pardon_score, score_min, score_max, grade_min, grade_max);
    exam.grade = int(exam.grade_d + .5);   // +.5 to round to nearest integer
    if (verbose_correction) {
      if (exam.surname == target_student) {
        cout << "FINAL SCORE - " << fixed << setprecision(2) << setw(6) << exam.score << "\t"
          << fixed << setprecision(2) << setw(6) << exam.pardon_score << "\t"
          << fixed << setprecision(2) << setw(6) << exam.pardon_score - exam.score << "\t"
          << " -> " << exam.grade << endl << endl;
      }
    }
  }

  // Dump results to GRADES
  file_path = c.work_folder + "/" + c.grades_name;
  ofstream fileout(file_path);
  if (!fileout) {
    cout << "GRADES file " << file_path << " impossible to create. Quitting..." << endl;
    exit(8);
  }
  fileout << "Score range       : [ " << fixed << setprecision(2) << setw(6)
    << score_min << " , " << score_max << " ]" << endl
    << "Grade range       : [ " << fixed << setprecision(2) << setw(6)
    << grade_min << " , " << grade_max << " ]" << endl
    << "Question per exam : " << setw(2) << question_number << endl
    << "Students parsed   : " << call.exams.size() << endl
    << "Serials parsed    : " << call.serials_map.size() << endl
    << "Correct score     : " << fixed << setprecision(2) << setw(6)
    << correct_score << endl
    << "Wrong score       : " << fixed << setprecision(2) << setw(6)
    << wrong_score << endl
    << "Blank score       : " << fixed << setprecision(2) << setw(6)
    << 0.0 << endl << endl;
  for (auto exam : call.exams) {
    fileout << exam.serial << "\t\t"
      << exam.answers << "\t"
      << exam.solutions << "\t\t"
      << setw(5) << fixed << setprecision(2) << exam.score << "\t"
      << setw(5) << fixed << setprecision(2) << exam.pardon_score << "\t"
      << setw(5) << fixed << setprecision(2) << exam.pardon_score - exam.score << "\t"
      << setw(5) << fixed << setprecision(2) << exam.grade_d << "\t"
      << setw(2) << exam.grade << "\t\t"
      << exam.student << endl;
  }
  fileout.close();

  return 0;
}
