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

#include <algorithm>
#include <regex>

#include <boost/filesystem.hpp>

#include "latex_headers.hpp"

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
  Call2<BaseExam> call;
  GenConfig<decltype(call)> c(config_name, &call);
  c.parsefile();
  if (!c.check_params()) exit(4);

  // Start log file dumping
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

  // DB 1 - Browsing database
  path p(c.db_folder);
  vector<vector <string> > db(c.slot_specs.size());
  int log_counter = 1;
  try {
    if (exists(p)) {
      if (is_directory(p)) {
        log << log_counter << ") Entering database directory : " << p << endl; log_counter++;
        for (directory_iterator it(p), end; it != end; it++) {
          log << log_counter << ") Analyzing question : " << it->path().filename() << endl; log_counter++;
          for (size_t i = 0; i < c.slot_specs.size(); i++) {
            for (auto patt : c.slot_specs[i]) {
              regex r(patt);
              if (std::regex_search(it->path().filename().generic_string(), r)) {
                log << log_counter << ") Question " << it->path().filename() << " matches \"" << patt << "\", stored in SLOT" << i + 1 << endl; log_counter++;
                db[i].push_back(it->path().generic_string());
              }
            }
          }
        }
      }
      else {
        log << log_counter << ") Database folder is not a directory : " << p << endl; log_counter++;
        exit(66);
      }
    }
    else {
      log << log_counter << ") Database directory not found : " << p << endl; log_counter++;
      exit(6);
    }
  }
  catch (const filesystem_error &ex) {
    cout << ex.what() << endl;
  }

  // DB 2 - Sorting names, database size, safety size check loop
  log << log_counter << ") Sorting database names " << endl; log_counter++;
  size_t database_size = 0;
  for (size_t i = 0; i < db.size(); ++i) {
    if (db[i].size() == 0) {
      log << log_counter << ") Unable to populate slot #" << i+1 << endl; log_counter++;
      exit(7);
    }
    database_size += db[i].size();
    sort(db[i].begin(), db[i].end());
  }

  // DB 3 - Importing questions in organized database
  vector<vector <BaseQuestion> > database;
  std::ifstream filein;
  for (size_t i = 0; i < db.size(); ++i) {
    vector<BaseQuestion> database_slot;
    for (size_t j = 0; j < db[i].size(); j++) {
      string line;
      vector<string> all_lines;
      log << log_counter << ") Importing question : " << db[i][j] << endl; log_counter++;
      filein.open(db[i][j]);
      if (!filein) {
        log << log_counter << ") Error opening : " << db[i][j] << endl;
        log_counter++;
      }
      while (std::getline(filein, line)) {
        if (line[0] != '.' && line[0] != '#' && line.size() != 0 && line != "\n") {
          all_lines.push_back(line);
        }
      }
      filein.close();

      vector<string> tokens;
      split(tokens, db[i][j], is_any_of("/\\"), token_compress_on);

      BaseQuestion q;
      q.path = db[i][j];
      q.name = tokens.back();
      q.text = all_lines[0];
      for (size_t k = 1; k < all_lines.size(); k++) {
        q.answers.push_back(make_pair(all_lines[k], (k == 1) ? 1 : 0));
      }
      database_slot.push_back(q);
    }
    database.push_back(database_slot);
  }

  // Randomizer
  log << log_counter << ") Initializing randomizer" << endl; log_counter++;
  Rnd r(c.random_seed);

  // Filling the call
  log << log_counter << ") Generating the call" << endl; log_counter++;
  for (int i = 0; i < c.exam_number; i++) {
    log << log_counter << ") Generating exam " << i + 1 << " of " << c.exam_number << endl; log_counter++;
    BaseExam exam;
    exam.serial = c.starting_serial + i;
    // shuffling questions database
    for (auto slot : database) {
      exam.questions.push_back(r.shuffle(slot)[0]);
    }
    // shuffling question order
    exam.questions = r.shuffle(exam.questions);
    // shuffling answer order and saving 
    for (auto &q : exam.questions) {
      q.answers = r.shuffle(q.answers);
      for (size_t j = 0; j < q.answers.size(); j++) {
        if (q.answers[j].second == 1) exam.solutions.push_back('A' + char(j));
      }
    }
    call.exams.push_back(exam);
  }

  // Writing SERIALS file
  log << log_counter << ") Writing serial file" << endl; log_counter++;
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
  log << log_counter << ") Writing exam content" << endl; log_counter++;
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
  log << log_counter << ") Writing database content" << endl; log_counter++;
  file_path = c.work_folder + "/database-content_" + call.name + ".tex";
  fileout.open(file_path);
  if (!fileout) {
    cerr << "LATEX DATABASE content " << file_path << " impossible to create. Quitting..." << endl;
    exit(14);
  }
  int all_counter = 1;
  fileout << "% Content of database for call <" << call.name << "> date " << call.date << endl << endl;
  for (size_t i = 0; i < database.size(); i++) {
    fileout << "\\subsection*{Slot " << i + 1 << "/" << database.size() << " , size " << database[i].size() << "}" << endl << endl;
    for (auto q : database[i]) {
      fileout << "\\noindent" << endl
        << "{\\large \\textbf{" << all_counter << "} - }{\\tt [" << q.name << "]}" << q.text << endl << endl;
      for (size_t j = 0; j < q.answers.size(); j++) {
        fileout << "{$" << char('A' + j) << "$}: " << q.answers[j].first << endl << "\\ \\" << endl;
      }
      fileout << endl;
      all_counter++;
    }
  }
  fileout.close();

  // Writing EXAM form
  log << log_counter << ") Writing exam form" << endl; log_counter++;
  file_path = c.work_folder + "/exam-form.tex";
  fileout.open(file_path);
  if (!fileout) {
    cerr << "LATEX DATABASE content " << file_path << " impossible to create. Quitting..." << endl;
    exit(15);
  }
  fileout << exam_form(call);
  fileout.close();

  // Writing DATABASE form
  log << log_counter << ") Writing database form" << endl; log_counter++;
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
