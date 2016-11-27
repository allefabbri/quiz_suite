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

#include <algorithm>
#include <regex>

#include <boost/filesystem.hpp>

#include "latex_utils.hpp"

#include "quiz_database.hpp"

typedef Call<BaseExam>     GenCall;
typedef GenConfig<GenCall> GenConf;

using namespace std;
using namespace boost::filesystem;

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
  cout << "QuizGenerator v" << MAJOR << "." << MINOR << endl;

  // Parse command line
  string config_name;
  if (argc == 2) {
    config_name = argv[1];
  }
  else {
    usage(argv[0]);
    exit(2);
  }

  // Create config
  GenCall call;
  GenConf c(config_name, &call);
  if (!c.check_params()) exit(4);

  // Start log file dumping
  int log_counter = 1;
  string file_path = c.work_folder + "/gen.log";
  std::ofstream log(file_path);
  log << "PARAMETERS IN USE" << endl
    << "Name            : " << call.name << endl
    << "Date            : " << call.date << endl
    << "Course          : " << call.course << endl
    << "Commission      : " << call.commission << endl
    << "Cdl             : " << call.cdl << endl
    << "Tag             : " << call.tag << endl
    << "Starting serial : " << c.starting_serial << endl
    << "Random seed     : " << c.random_seed << endl
    << "Grading scale   : " << fixed << setprecision(2) << call.score_scale << endl
    << "Working folder  : " << c.work_folder << endl
    << "Database folder : " << c.db_folder << endl << endl << endl
    << "OPERATIONS PERFORMED" << endl;

  // Create DB
  QuizDatabase database(log, log_counter);
  if (!database.populate(c)) exit(-1);

  // Randomizer
  log << log_counter++ << ") Initializing randomizer" << endl;
  Rnd r(c.random_seed);

  // Filling the call
  log << log_counter++ << ") Generating the call" << endl;
  for (int i = 0; i < c.exam_number; i++) {
    log << log_counter++ << ") Generating exam " << i + 1 << " of " << c.exam_number << endl;
    BaseExam exam;
    exam.serial = c.starting_serial + i;
    // extract fully randomized question set from db
    exam.questions = database.get_rnd_question_set(r);
    // create solution string for each exam 
    for (auto &q : exam.questions) {
      for (size_t j = 0; j < q.answers.size(); j++) {
        if (q.answers[j].second == true) exam.solutions.push_back('A' + char(j));
      }
    }
    call.exams.push_back(exam);
  }

  // Writing SERIALS file
  log << log_counter++ << ") Writing serial file" << endl;
  file_path = c.work_folder + "/serials_" + call.name + ".txt";
  std::ofstream fileout(file_path);
  if (!fileout) {
    cerr << "SERIAL file " << file_path << " impossible to create. Quitting..." << endl;
    exit(12);
  }
  fileout << "% Serials for call <" << call.name << "> date " << call.date << endl;
  for (auto e : call.exams) {
    fileout << "\t" << e.serial << "\t";
    for (auto q : e.questions) {
      fileout << q.name << "\t";
    }
    fileout << e.solutions << endl;
  }
  fileout.close();

  // Writing EXAM latex content file
  log << log_counter++ << ") Writing exam content" << endl;
  file_path = c.work_folder + "/exam-content_" + call.name + ".tex";
  fileout.open(file_path);
  if (!fileout) {
    cerr << "LATEX EXAM content " << file_path << " impossible to create. Quitting..." << endl;
    exit(13);
  }
  fileout << "% Content of call <" << call.name << "> date " << call.date << endl << endl;
  for (auto e : call.exams) {
    fileout << "% Exam - serial " << e.serial << endl;
    fileout << "\\def\\serialnumber{" << e.serial << "}" << endl;
    fileout << "\\paperheader" << endl << endl << endl;
    for (size_t i = 0; i < e.questions.size(); i++) {
      fileout << "\\def\\questionnumber{" << i + 1 << "}" << endl << endl;
      fileout << "\\questionheader " << e.questions[i].text << endl;
      fileout << "\\\\" << endl;
      for (size_t j = 0; j < e.questions[i].answers.size(); j++) {
        fileout << "{$" << char('A' + j) << "$}: " << e.questions[i].answers[j].first << endl << "\\ \\ ";
      }
      fileout << endl << endl << endl;
    }
    fileout << endl << "\\paperfooter" << endl << endl << endl;
  }
  fileout.close();

  // Writing DATABASE latex content file
  log << log_counter++ << ") Writing database content" << endl;
  file_path = c.work_folder + "/database-content_" + call.name + ".tex";
  fileout.open(file_path);
  if (!fileout) {
    cerr << "LATEX DATABASE content " << file_path << " impossible to create. Quitting..." << endl;
    exit(14);
  }
  fileout << "% Content of database for call <" << call.name << "> date " << call.date << endl << endl;
  // Header
  fileout << R"(\section*{Database}
\begin{center}
\begin{tabular}{| c | c | c | c |}
  \hline
  Total quiz & Total slots & Ave quiz per slot & Possible different exams \\ \hline)" << endl
  << database.size() << " & " << database.slot_size() << " & "
  << fixed << setprecision(1) << database.ave_question_per_slot()
    << " & " << num_to_latex_scientific(database.possible_exams()) << R"( \\ \hline 
\end{tabular}
    \end{center}

)";
  // Slots
  int slot_counter = 1;
  int question_counter = 1;
  for (const auto & s : database.slots){
    fileout << "\\subsection*{Slot " << slot_counter++ << "/" << database.slot_size() << " , size " << s.size() << "}" << endl << endl;
    for (const auto & q_pair : s) {
      auto q = q_pair.second;
      fileout << "\\noindent" << endl
        << "{\\large \\textbf{" << question_counter++ << "} - }{\\tt [" << q.name << "]}" << q.text << endl << endl;
      for (size_t j = 0; j < q.answers.size(); j++) {
        if(j==0){
          fileout << "{\\boxed{$" << char('A' + j) << "$}}: " << q.answers[j].first << endl << "\\ \\" << endl;
        }
        else{
          fileout << "{$" << char('A' + j) << "$}: " << q.answers[j].first << endl << "\\ \\" << endl;
        }
      }
      fileout << endl;
    }
  }
  fileout.close();

  // Writing EXAM form
  log << log_counter++ << ") Writing exam form" << endl;
  file_path = c.work_folder + "/exam-form.tex";
  fileout.open(file_path);
  if (!fileout) {
    cerr << "LATEX DATABASE content " << file_path << " impossible to create. Quitting..." << endl;
    exit(15);
  }
  fileout << exam_form(call);
  fileout.close();

  // Writing DATABASE form
  log << log_counter++ << ") Writing database form" << endl;
  file_path = c.work_folder + "/database-form.tex";
  fileout.open(file_path);
  if (!fileout) {
    cerr << "LATEX DATABASE content " << file_path << " impossible to create. Quitting..." << endl;
    exit(15);
  }
  fileout << database_form(call);
  fileout.close();

  // Command line suggestion
  cout << "To generate the pdf's please type :\ncd " << c.work_folder << " && for form in *-form.tex; do pdflatex.exe $form; done && cd -" << endl;

  return 0;
}
