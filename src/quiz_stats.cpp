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

typedef Call<GradedExam>       StatsCall;
typedef StatsConfig<StatsCall> StatsConf;

constexpr int MAJOR = 2;
constexpr int MINOR = 1;

void usage(char * progname) {
  vector<string> tokens;
  boost::split(tokens, progname, boost::is_any_of("/\\"));
  cout << "Usage: " << tokens.back() << " path/to/config" << endl;
  cout << "      path/to/config must be a valid config file" << endl;
  cout << endl;
  cout << "Usage: " << tokens.back() << " -conf_t" << endl;
  cout << "      generate a dummy config file" << endl;
}

int main(int argc, char ** argv) {
  cout << "QuizStats v" << MAJOR << "." << MINOR << endl;

  // Parse command line
  string config_name;
  if (argc == 2) {
    config_name = argv[1];
  }
  else {
    usage(argv[0]);
    exit(2);
  }

  // Variables and containers
  string line, file_path;
  vector<string> tokens;
  map<string, Question_param> question_map;
  //int exam_number = 0;
  int pad_name = 0;
  map<int, int> pardon_bins;
  
  // Create config
  StatsCall call;
  StatsConf c(config_name, &call);
  if (!c.check_params()) exit(4);

  // Import SERIALS file
  if (!call.parse_serial(&c)) exit(5);

  // Importing GRADES file
  if (!call.parse_grades(&c)) exit(6);

  // Evaluate stats
  for (auto exam : call.exams) {
    for (int i = 0; i < exam.answers.size(); i++) {
      string question_name = call.serials_map[exam.serial].second[i];
      if ( size_t(pad_name) < question_name.size() ) pad_name = int(question_name.size());
      question_map[question_name].repetitions++;
      if (exam.answers[i] == '-') {
        question_map[question_name].blank++;
      }
      else if (exam.answers[i] == exam.solutions[i]) {
        question_map[question_name].correct++;
      }
      else {
        question_map[question_name].wrong++;
      }
    }
    pardon_bins[(int)round(exam.pardon_score - exam.score)]++;
  }

  // Total question result counters
  int tot_r = 0, tot_c = 0, tot_w = 0, tot_b = 0;
  for (auto & q : question_map) {
    if (q.second.repetitions != q.second.correct + q.second.wrong + q.second.blank)
      cout << "ERROR --- " << q.first << "  counters mismatch" << endl;
    tot_r += q.second.repetitions;
    tot_c += q.second.correct;
    tot_w += q.second.wrong;
    tot_b += q.second.blank;
  }

  // Evaluating grade bins
  vector<int> bin_freq(c.bin_pivot.size() - 1);
  for (auto exam : call.exams) {
    int index = -1;
    for (auto b : c.bin_pivot) {
      if (exam.grade_d < b) break;
      index++;
      if (index > c.bin_pivot.size() - 2) index = (int)c.bin_pivot.size() - 2; // to include grades in the last bin
    }
    bin_freq[index]++;
  }
  int bin_tot = 0;
  for (auto f : bin_freq) {
    bin_tot += f;
  }

  // Dumping REPORT
  file_path = c.work_folder + "/" + c.stat_report;
  ofstream fileout(file_path);
  if (!fileout) {
    cout << "REPORT file " << file_path << " impossible to create. Quitting..." << endl;
    exit(8);
  }
  int pad_count = 5, pad_perc = 5;
  fileout << "Database size     : " << question_map.size() << " questions" << endl
    << "Call size         : " << call.exams.size() << " students" << endl
    << "Bugged exams      : " << endl;
  int tot = 0;
  for (const auto & b : pardon_bins) {
    fileout << "\tBugs : " << setw(2) << b.first / 2 << " -> " << setw(3) << b.second << " ( "
      << setw(5) << fixed << setprecision(1) << 100 * b.second / double(call.exams.size()) << " % ) " << endl;
    tot += b.second;
  }
  fileout << "\tTotal -----> " << setw(3) << tot << " ( " << setw(5) << 100 * tot / double(call.exams.size()) << " % ) " << endl;
  fileout << "Tot questions     : " << call.exams.size()*call.exams[0].answers.size() << " = " << tot_r << endl;
  fileout << "Coarse grade bins : " << endl;
  for (int i = 0; i < bin_freq.size(); i++) {
    fileout << "\t] " << setw(5) << c.bin_pivot[i] << " , " << setw(5) << c.bin_pivot[i + 1] << " ] -> "
      << setw(3) << bin_freq[i] << " ( "
      << setw(5) << fixed << setprecision(1) << 100 * bin_freq[i] / double(call.exams.size()) << " % )" << endl;
  }
  fileout << "\tTotal  ------------> " << setw(3) << bin_tot << " ( "
    << setw(5) << fixed << setprecision(1) << 100 * bin_tot / double(call.exams.size())
    << " % ) " << endl << endl;
  fileout << std::left << std::setw(pad_name + 4) << "EX NAME"
    << std::right << std::setw(pad_count) << "TOT"
    << std::right << std::setw(pad_count + 2 * pad_perc + 2 + 9) << "CORRECT"
    << std::right << std::setw(pad_count + 2 * pad_perc + 2 + 7) << "WRONG"
    << std::right << std::setw(pad_count + 2 * pad_perc + 2 + 7) << "BLANK"
    << std::endl << std::endl;
  for (const auto & q : question_map) {
    fileout << std::left << std::setw(pad_name) << q.first << " ->"
      << std::right << std::setw(pad_count) << q.second.repetitions << "  : "
      << std::right << std::setw(pad_count) << q.second.correct << " ( "
      << std::fixed << std::setprecision(1) << std::setw(pad_perc) << (float)q.second.correct / q.second.repetitions * 100 << " - "
      << std::fixed << std::setprecision(1) << std::setw(pad_perc - 1) << binomial_dist(q.second.repetitions, q.second.correct, 0.25) * 100 << " %) "
      << std::right << std::setw(pad_count) << q.second.wrong << " ( "
      << std::fixed << std::setprecision(1) << std::setw(pad_perc) << (float)q.second.wrong / q.second.repetitions * 100 << " - "
      << std::fixed << std::setprecision(1) << std::setw(pad_perc - 1) << binomial_dist(q.second.repetitions, q.second.wrong, 0.75) * 100 << " %) "
      << std::right << std::setw(pad_count) << q.second.blank << " ( "
      << std::fixed << std::setprecision(1) << std::setw(pad_perc) << (float)q.second.blank / q.second.repetitions * 100 << " - "
      << std::fixed << std::setprecision(1) << std::setw(pad_perc - 1) << binomial_dist(q.second.repetitions, q.second.blank, 0.5) * 100 << " %) " << std::endl;
  }
  fileout << std::string(pad_name + 4 + 4 * pad_count + 6 * pad_perc + 6 + 23, '-') << endl;
  fileout << setw(pad_name + 3) << "Total       ->" << std::setw(pad_count) << tot_r << "  : "
    << std::setw(pad_count) << tot_c << " ( "
    << std::setprecision(1) << std::setw(pad_perc) << (float)tot_c / tot_r * 100 << " - "
    << std::fixed << std::setprecision(1) << std::setw(pad_perc - 1) << binomial_dist(tot_r, tot_c, 0.25) * 100 << " %) "
    << std::setw(pad_count) << tot_w << " ( "
    << std::setprecision(1) << std::setw(pad_perc) << (float)tot_w / tot_r * 100 << " - "
    << std::fixed << std::setprecision(1) << std::setw(pad_perc - 1) << binomial_dist(tot_r, tot_w, 0.75) * 100 << " %) "
    << std::setw(pad_count) << tot_b << " ( "
    << std::setprecision(1) << std::setw(pad_perc) << (float)tot_b / tot_r * 100 << " - "
    << std::fixed << std::setprecision(1) << std::setw(pad_perc - 1) << binomial_dist(tot_r, tot_b, 0.5) * 100 << " %) " << endl;
  fileout.close();

  return 0;
}
