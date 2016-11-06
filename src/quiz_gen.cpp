#define _SCL_SECURE_NO_WARNINGS

#include <fstream>
#include <algorithm>
#include <regex>

#include <boost/filesystem.hpp>

#include "latex_headers.hpp"

using namespace boost::filesystem;

#define MAJOR_VERSION          1
#define MINOR_VERSION          1

int main(int argc, char ** argv) {
  cout << "QuizGenerator v" << MAJOR_VERSION << "." << MINOR_VERSION << endl;

  // Parse command line
  string config_name;
  if (argc == 2) {
    config_name = argv[1];
  }
  else {
    cout << "Usage: " << argv[0] << " path/to/config" << endl;
    cout << "      path/to/config must be a valid config file" << endl;
    cout << endl;
    cout << "Usage: " << argv[0] << " -conf_t" << endl;
    cout << "      generate a dummy config file" << endl;
    exit(2);
  }

  // Generate empty config file and quit
  if (std::string(argv[1]) == "-conf_t") {
    cout << "Generating empty config file named \"gen.config\"" << endl;
    std::ofstream config("gen.config");
    config
      << "CALL_NAME       = appello1" << endl
      << "CALL_DATE       = 17/9/1993" << endl
      << "COURSE          = Paleontologia Applicata" << endl
      << "TAG             = Appello I - Sessione autunnale - A.A. 1993/94" << endl
      << "CDL             = Corso di Laurea in Paleontologia" << endl
      << "COMMISSION      = Prof. A. Grant" << endl
      << "EXAM_NUMBER     = 15" << endl
      << "SLOT1           = regex1.1-* regex1.2-* ;" << endl
      << "[...]" << endl
      << "SLOTn           = regexn.1-* regexn.2-* ;" << endl
      << "STARTING_SERIAL = 1" << endl
      << "RANDOM_SEED     = 1010" << endl
      << "SCALE           = 30" << endl
      << "WORK_FOLDER     = appello1" << endl
      << "DATABASE_FOLDER = database" << endl << endl;
    config.close();
    exit(0);
  }

  // Safe CONFIG file parsing
  string db_folder, work_folder;
  vector<vector<string>> slot_specs;
  int exam_number = -1, starting_serial = -1, random_seed = -1;
  Call call;
  string key, equal, value;
  std::ifstream filein(config_name);
  if (!filein) {
    cout << "Configuration file " << config_name << " not found. Quitting..." << endl;
    exit(3);
  }
  while (filein >> key >> equal >> value) {
    if (key == "CALL_NAME") {
      call.name = value;
    }
    else if (key == "CALL_DATE") {
      call.date = value;
    }
    else if (key == "COURSE") {
      std::getline(filein, call.course);
      call.course = value + call.course;
    }
    else if (key == "TAG") {
      std::getline(filein, call.tag);
      call.tag = value + call.tag;
    }
    else if (key == "CDL") {
      std::getline(filein, call.cdl);
      call.cdl = value + call.cdl;
    }
    else if (key == "COMMISSION") {
      std::getline(filein, call.commission);
      call.commission = value + call.commission;
    }
    else if (key == "EXAM_NUMBER") {
      exam_number = atoi(value.c_str());
    }
    else if (key.substr(0, 4) == "SLOT") {
      vector<string> specs;
      specs.push_back(value);
      while (true) {
        filein >> value;
        if (value == ";") break;
        specs.push_back(value);
      }
      slot_specs.push_back(specs);
    }
    else if (key == "STARTING_SERIAL") {
      starting_serial = atoi(value.c_str());
    }
    else if (key == "RANDOM_SEED") {
      random_seed = atoi(value.c_str());
    }
    else if (key == "SCALE") {
      call.scale = atof(value.c_str());
    }
    else if (key == "WORK_FOLDER") {
      work_folder = value;
    }
    else if (key == "DATABASE_FOLDER") {
      db_folder = value;
    }
    else {
      cout << "Key " << key << " unknown. Edit " << config_name << endl;
      exit(3);
    }
  }
  filein.close();

  if (call.name == "") {
    cout << "CALL NAME unset. Edit " << config_name << endl;
    exit(3);
  }
  if (call.date == "") {
    cout << "CALL DATE unset. Edit " << config_name << endl;
    exit(3);
  }
  if (call.course == "") {
    cout << "CALL COURSE unset. Edit " << config_name << endl;
    exit(3);
  }
  if (call.tag == "") {
    cout << "CALL TAG unset. Edit " << config_name << endl;
    exit(3);
  }
  if (call.cdl == "") {
    cout << "CALL CDL unset. Edit " << config_name << endl;
    exit(3);
  }
  if (call.commission == "") {
    cout << "COMMISSION unset. Edit " << config_name << endl;
    exit(3);
  }
  if (exam_number == -1) {
    cout << "EXAM NUMBER unset. Edit " << config_name << endl;
    exit(3);
  }
  if (slot_specs.size() == 0) {
    cout << "SPECS unset. Edit " << config_name << endl;
    exit(3);
  }
  else {
    for (auto spec : slot_specs) {
      if (spec.size() == 0) {
        cout << "SLOT_SPEC is empty. Edit " << config_name << endl;
        exit(3);
      }
    }
  }
  if (starting_serial == -1) {
    cout << "STARTING SERIAL unset. Edit " << config_name << endl;
    exit(3);
  }
  if (random_seed == -1) {
    cout << "RANDOM SEED unset. Edit " << config_name << endl;
    exit(3);
  }
  if (call.scale == 0.0) {
    cout << "GRADING SCALE unset. Edit " << config_name << endl;
    exit(3);
  }
  if (work_folder == "") {
    cout << "WORKING FOLDER unset. Edit " << config_name << endl;
    exit(3);
  }
  if (db_folder == "") {
    cout << "DATABASE FOLDER unset. Edit " << config_name << endl;
    exit(3);
  }

  // Start log file dumping
  std::ofstream log(work_folder + "/gen.log");
  log << "PARAMETERS IN USE" << endl
    << "Name            : " << call.name << endl
    << "Date            : " << call.date << endl
    << "Course          : " << call.course << endl
    << "Commission      : " << call.commission << endl
    << "Cdl             : " << call.cdl << endl
    << "Tag             : " << call.tag << endl
    << "Starting serial : " << starting_serial << endl
    << "Random seed     : " << random_seed << endl
    << "Grading scale   : " << fixed << setprecision(2) << call.scale << endl
    << "Working folder  : " << work_folder << endl
    << "Database folder : " << db_folder << endl << endl << endl
    << "OPERATIONS PERFORMED" << endl;

  // Browsing database
  path p(db_folder);
  vector<vector <string> > db(slot_specs.size());
  int log_counter = 1;
  try {
    if (exists(p)) {
      if (is_directory(p)) {
        log << log_counter << ") Entering database directory : " << p << endl; log_counter++;
        for (directory_iterator it(p), end; it != end; it++) {
          log << log_counter << ") Analyzing question : " << it->path().filename() << endl; log_counter++;
          for (size_t i = 0; i < slot_specs.size(); i++) {
            for (auto patt : slot_specs[i]) {
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

  // Sorting names, database size, safety size check loop
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

  // Importing questions in organized database
  vector<vector <Question> > database;
  for (size_t i = 0; i < db.size(); ++i) {
    vector<Question> database_slot;
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
      split(tokens, db[i][j], is_any_of(R"(/\)"), token_compress_on);

      Question q;
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
  Rnd r(random_seed);

  // Filling the call
  log << log_counter << ") Generating the call" << endl; log_counter++;
  for (int i = 0; i < exam_number; i++) {
    log << log_counter << ") Generating exam " << i + 1 << " of " << exam_number << endl; log_counter++;
    Exam exam;
    exam.serial = starting_serial + i;
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
  std::ofstream serials(work_folder + "/serials_" + call.name + ".txt");
  serials << "% Serials for call <" << call.name << "> date " << call.date << endl;
  for (auto e : call.exams) {
    serials << "\t" << e.serial << "\t";
    for (auto q : e.questions) {
      serials << q.name << "\t";
    }
    serials << e.solutions << endl;
  }
  serials.close();

  // Writing EXAM latex content file
  log << log_counter << ") Writing exam content" << endl; log_counter++;
  std::ofstream content(work_folder + "/exam-content_" + call.name + ".tex");
  content << "% Content of call <" << call.name << "> date " << call.date << endl << endl;
  for (auto e : call.exams) {
    content << "% Exam - serial " << e.serial << endl;
    content << "\\def\\serialnumber{" << e.serial << "}" << endl;
    content << "\\paperheader" << endl << endl << endl;
    for (size_t i = 0; i < e.questions.size(); i++) {
      content << "\\def\\questionnumber{" << i + 1 << "}" << endl << endl;
      content << "\\questionheader " << e.questions[i].text << endl;
      content << "\\\\" << endl;
      for (size_t j = 0; j < e.questions[i].answers.size(); j++) {
        content << "{$" << char('A' + j) << "$}: " << e.questions[i].answers[j].first << endl << "\\ \\ ";
      }
      content << endl << endl << endl;
    }
    content << endl << "\\paperfooter" << endl << endl << endl;
  }
  content.close();

  // Writing DATABASE latex content file
  log << log_counter << ") Writing database content" << endl; log_counter++;
  content.open(work_folder + "/database-content_" + call.name + ".tex");
  int all_counter = 1;
  content << "% Content of database for call <" << call.name << "> date " << call.date << endl << endl;
  for (size_t i = 0; i < database.size(); i++) {
    content << "\\subsection*{Slot " << i + 1 << "/" << database.size() << " , size " << database[i].size() << "}" << endl << endl;
    for (auto q : database[i]) {
      content << "\\noindent" << endl
        << "{\\large \\textbf{" << all_counter << "} - }{\\tt [" << q.name << "]}" << q.text << endl << endl;
      for (size_t j = 0; j < q.answers.size(); j++) {
        content << "{$" << char('A' + j) << "$}: " << q.answers[j].first << endl << "\\ \\" << endl;
      }
      content << endl;
      all_counter++;
    }
  }
  content.close();

  // Writing EXAM form
  log << log_counter << ") Writing exam form" << endl; log_counter++;
  std::ofstream form(work_folder + "/exam-form.tex");
  form << exam_form(call);
  form.close();

  // Writing EXAM form
  log << log_counter << ") Writing database form" << endl; log_counter++;
  form.open(work_folder + "/database-form.tex");
  form << database_form(call);
  form.close();

  // Command line suggestion
  cout << "To generate the pdf's please type :\ncd " << work_folder << " && for form in *-form.tex; do pdflatex.exe $form; done && cd -" << endl;

  return 0;
}
