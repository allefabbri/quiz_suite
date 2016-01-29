#define _SCL_SECURE_NO_WARNINGS

#include <iomanip>
#include <fstream>
#include <cstdlib>

#include "quiz_lib.hpp"

#define MAJOR       1
#define MINOR       0

int main(int argc, char ** argv){
  cout << "QuizGrader v" << MAJOR << "." << MINOR << endl;

// Parse command line
  string config_name, target_student;
  bool verbose_correction = false;
  if( argc == 2 ){
    config_name = argv[1];
  }
  else if ( argc == 4 ){
    config_name = argv[1];
    verbose_correction = true;
    target_student = argv[3];
  }
  else{
    cout << "Usage: " << argv[0] << " path/to/config" << endl;
    cout << "      path/to/config must be a valid config file" << endl;
    cout << endl;
    cout << "Usage: " << argv[0] << " path/to/config -v [student_surname]" << endl;
    cout << "      enable verbose grading for [student_surname]" << endl;
    cout << endl;
    cout << "Usage: " << argv[0] << " -conf_t" << endl;
    cout << "      generate a dummy config file" << endl;
    exit(2);
  }

// Generating empty config file
  if( config_name == "-conf_t" ){
    std::cout << "Creating empty config file \"grade.config\"" << std::endl;
    std::ofstream fileout("grade.config");
    fileout 
    << "BUGS            = errors.txt" << endl
    << "SERIALS         = serials.txt" << endl
    << "REPORT_BASENAME = appello1" << endl
    << "RESULTS         = risultati.txt" << endl
    << "GRADES          = voti.txt" << endl
    << "CHOICES         = 4" << endl
    << "SCORE_SCALE     = 30" << endl
    << "WORK_FOLDER     = appello1" << endl << endl;
    fileout.close();   
    exit(-1); 
  }

// Safe CONFIG file parsing
  string bugs_name, serials_name, report_basename, results_name, grade_name, work_folder;
  string key, equal, value;
  int choices_number = -1, score_scale = -1;
  bool is_call_bugged;
  ifstream filein(config_name);
  if( !filein ) {
    cout << "Configuration file " << config_name << " not found. Quitting..." << endl;
    exit(3);
  }
  while ( filein >> key >> equal >> value ){
    if ( key == "BUGS" ){
      bugs_name = value;
    }
    else if ( key == "SERIALS" ){
      serials_name = value;
    }
    else if ( key == "REPORT_BASENAME" ){
      report_basename = value;
    }
    else if ( key == "RESULTS" ){
      results_name = value;
    }
    else if ( key == "GRADES" ){
      grade_name = value;
    }
    else if ( key == "CHOICES" ){
      choices_number = atoi(value.c_str());
    }
    else if ( key == "SCORE_SCALE" ){
      score_scale = atoi(value.c_str());
    }
    else if ( key == "WORK_FOLDER" ){
      work_folder = value;
    }
    else{
      cout << "Key " << key << " unknown. Edit " << config_name << endl;
      exit(3);
    }
  }
  filein.close();

  if ( bugs_name == "" ){
    cout << "BUGS file unset, entering HEALTHY mode" << endl;
    is_call_bugged = false;
  }
  else{
    cout << "BUGS file set, entering BUGGED mode" << endl;    
    is_call_bugged = true;
  }
  if ( serials_name == "" ){
    cout << "SERIALS file unset. Edit " << config_name << endl;
    exit(3);
  }
  if ( report_basename == "" ){
    cout << "REPORT BASENAME unset. Edit " << config_name << endl;
    exit(3);
  }
  if ( results_name == "" ){
    cout << "RESULTS file unset. Edit " << config_name << endl;
    exit(3);
  }
  if ( grade_name == "" ){
    cout << "GRADES file unset. Edit " << config_name << endl;
    exit(3);
  }
  if ( choices_number == -1 ){
    cout << "CHOICES value unset. Edit " << config_name << endl;
    exit(3);
  }
  if ( score_scale == -1 ){
    cout << "SCORE SCALE value unset. Edit " << config_name << endl;
    exit(3);
  }
  if ( work_folder == "" ){
    cout << "WORK FOLDER unset. Edit " << config_name << endl;
    exit(3);
  }

// Variables and containers
  ofstream fileout;
  string line, file_path;
  vector<string> tokens;
  map<int, vector<int> > bugs_map, healthy_map;
  Call call;

// Import RESULTS handwritten, structure { serials, answers, surname, name }
  file_path = work_folder + "/" + results_name;
  filein.open(file_path);
  if (!filein) {
    cout << "RESULTS file " << file_path << " not found. Quitting..." << endl;
    exit(4);
  }
  while ( getline(filein, line) ){
    trim(line);
    split(tokens, line, is_any_of("\t"), token_compress_on);
    if ( tokens.size() == 4 ) call.exams.push_back( Exam(tokens, 'g' ) );    // 'g' is for grading mode
  }
  filein.close();

// Import SERIALS file whose layout is 
// { serial number, question_name1, question_name2, ... , question_nameN, correct_answer }
// into serials_v
  file_path = work_folder + "/" + serials_name;
  filein.open(file_path);
  if( !filein ) { 
    cout << "SERIALS file " << file_path << " not found. Quitting..." << endl; 
    exit(5);
  }
  while( getline(filein,line) ){
    trim(line);
    if (line[0] == '%') continue;
    split(tokens, line, is_any_of("\t "), token_compress_on);
    if( tokens.size() > 6 ) {   // to skip header line, if any
      call.add_serial(tokens);
    }
  }
  filein.close();

// Associate to each exam its solutions
  for (auto &exam : call.exams) {
    exam.solutions = call.serials_map[exam.serial].first;
  }


// Calculating grade scales and question points
  double grade_min, grade_max, score_min, score_max;
  double correct_score, wrong_score, blank_score;
  int question_number = (int) call.exams[0].answers.size();
  score_max = score_scale;
  score_min = -score_scale/double(choices_number-1);
  grade_min = 0;
  grade_max = 30;
  correct_score = score_max/question_number;
  wrong_score   = - correct_score / ( choices_number - 1);
  blank_score   = 0.0;

// BUGGED mode specific operations
  if( is_call_bugged ){
  // Import WRONG QUESTION list file
    vector<string> error_list;
    file_path = work_folder + "/" + bugs_name;
    filein.open(file_path);
    if( !filein ) { 
      cout << "BUGS file " << file_path << " not found. Quitting..." << endl; 
      exit(4);
    }
    while( filein >> line ) error_list.push_back(line);
    filein.close();

// Creating the MAPS
    for( auto s_it=call.serials_map.begin(); s_it != call.serials_map.end(); s_it++){
      for( int i=0; i<s_it->second.second.size(); i++ ){
        for( auto err : error_list ){
          if( s_it->second.second[i] == err ){
            bugs_map[s_it->first].push_back(i);                               // storing bugs positions            
          }
        }
      }
      if( bugs_map.find(s_it->first) == bugs_map.end() ) healthy_map[s_it->first].push_back(1);
    }

// Dump LOG to file
    string flag;
    fileout.open(work_folder+"/"+report_basename+".log");
    if( !fileout ) { 
      cout << "LOG file " << report_basename << ".log not found. Quitting..." << endl; 
      exit(6);
    }
    fileout << endl << "BUGGED question : " << error_list.size() << endl;
    for( auto e : error_list ) fileout << " - " << e << endl;
    fileout << endl << endl;
    fileout << "Number of serials : " << call.serials_map.size() << endl << endl;
    string ending;
    for( auto s_it=call.serials_map.begin(); s_it != call.serials_map.end(); s_it++){
      if( bugs_map.find(s_it->first) == bugs_map.end() ){
        ending = "------ HEALTHY exam\n";
        flag = "-     ";
        for( int i=0; i<s_it->second.second.size(); i++ ){
          fileout << flag << "\t-\tSerial " << s_it->first <<  "\tQuestion " << i+1 << "\t\t" << s_it->second.second[i] << endl;
        }
      }
      else{
        ending = "------ BUGGED exam\n";
        for( int i=0; i<s_it->second.second.size(); i++ ){
          for( int index=0; index<bugs_map.find(s_it->first)->second.size(); index++ ){
            if( bugs_map.find(s_it->first)->second[index] == i ){
              flag = "BUGGED";
              break;
            }
            else {
              flag = "-     ";
            }            
          }
          fileout << flag << "\t-\tSerial " << s_it->first <<  "\tQuestion " << i+1 << "\t\t" << s_it->second.second[i] << endl;
        }
      }
      fileout << ending << endl;
    }
    fileout.close();

// Dumping BUGS and HEALTHY MAP to file
    file_path = work_folder + "/" + report_basename + ".bugs_map";
    fileout.open(file_path );
    fileout << "BUGGED serials  : " << bugs_map.size() 
            << " (" << fixed << setprecision(2) << bugs_map.size()/double(call.serials_map.size()) << " %)" << endl 
            << "HEALTHY serials : " << healthy_map.size()
            << " (" << fixed << setprecision(2) << healthy_map.size()/double(call.serials_map.size()) << " %)" << endl << endl;
    fileout << "BUGS map :" << endl;
    for( auto it=bugs_map.begin(); it != bugs_map.end(); it++){
      fileout << std::setw(3) << it->first << "  ->  " << it->second.size() << "\t{ ";
      for( auto i : it->second ) {
        fileout << setw(2) << i+1 << " " << call.serials_map[it->first].first[i] << ( (i==(it->second).back())?" }\n":" , " );
      }
    }
    fileout << endl << endl << "HEALTHY map :" << endl;
    for( auto it=healthy_map.begin(); it != healthy_map.end(); it++){
      fileout << std::setw(3) << it->first << "  ->  " << " ok " << endl;
    }
    fileout.close();

// Amending answers and solutions
    for( auto &exam : call.exams ){
      if( bugs_map.find(exam.serial) != bugs_map.end() ){
        for( int i : bugs_map[exam.serial] ){
          exam.answers[i] = '-';
          exam.solutions[i] = '-';
        }
      }
    }
  }         // end of BUGGED mode specific operations


// GRADING loop
  for( auto &exam : call.exams ){
    double last_score;
    for( size_t i=0; i<exam.answers.size(); i++){
      last_score = exam.pardon_score;
      if( exam.solutions[i] == '-' ){
        exam.pardon_score += correct_score;
      }
      else if( exam.answers[i] == exam.solutions[i] ){
        exam.pardon_score += correct_score;
      }
      else if( exam.answers[i] == '-' ){
        exam.pardon_score += blank_score;
      }
      else{
        exam.pardon_score += wrong_score;
      }
      if ( verbose_correction ){
        if( exam.surname == target_student ) {
          if ( i == 0 ) cout << "VERBOSE GRADING - " << exam.student << endl;
          cout << setw(3) << exam.serial << " " << setw(2) << i+1 << " " << exam.answers[i] << " "
          << exam.solutions[i] << " " 
          << fixed << setprecision(2) << setw(6) << exam.pardon_score << "  " 
          << fixed << setprecision(2) << setw(6) << exam.pardon_score-last_score << endl;
        }
      }               
    }
    if( bugs_map.find(exam.serial) != bugs_map.end() ){
      exam.score = exam.pardon_score - bugs_map.find(exam.serial)->second.size()*correct_score;
    }
    else{
      exam.score = exam.pardon_score;
    }
    exam.grade_d = mapping(exam.pardon_score, score_min, score_max, grade_min, grade_max);
    exam.grade = int(exam.grade_d+.5);   // +.5 to round to nearest integer
    if ( verbose_correction ){
      if( exam.surname == target_student ) {
        cout << "FINAL SCORE - " << fixed << setprecision(2) << setw(6) << exam.score << "\t"
        << fixed << setprecision(2) << setw(6) << exam.pardon_score << "\t"
        << fixed << setprecision(2) << setw(6) << exam.pardon_score-exam.score << "\t" 
        << " -> " << exam.grade << endl << endl;
      }               
    }
  }
  
// Dump results to GRADES
  fileout.open(work_folder+"/"+grade_name);
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
  for( auto exam : call.exams ){
    fileout << exam.serial << "\t\t"
            << exam.answers << "\t"
            << exam.solutions << "\t\t"
            << setw(5) << fixed << setprecision(2) << exam.score << "\t"
            << setw(5) << fixed << setprecision(2) << exam.pardon_score << "\t"
            << setw(5) << fixed << setprecision(2) << exam.pardon_score-exam.score << "\t"
            << setw(5) << fixed << setprecision(2) << exam.grade_d << "\t"
            << setw(2) << exam.grade << "\t\t"
            << exam.student << endl;
  }
  fileout.close();

  return 0;
}
