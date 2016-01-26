#define _SCL_SECURE_NO_WARNINGS

#include <iomanip>
#include <fstream>
#include <cstdlib>

#include "quiz_lib.hpp"

#define MAJOR     1
#define MINOR     0

int main(int argc, char ** argv){
  cout << "QuizStats v" << MAJOR << "." << MINOR << endl;

// Parse command line
  string config_name;
  if( argc == 2 ){
    config_name = argv[1];
  }
  else{
    cout << "Usage: " << argv[0] << " path/to/config" << endl;
    cout << "      path/to/config must be a valid config file" << endl;
    cout << endl;
    cout << "Usage: " << argv[0] << " -conf_t" << endl;
    cout << "      generate a dummy config file" << endl;
    exit(2);
  }

// Generating empty config file
  if( config_name == "-conf_t" ){
  std::cout << "Creating empty config file \"stats.config\"" << std::endl;
  std::ofstream fileout("stats.config");
  fileout 
    << "WORK_FOLDER = appello1" << endl
    << "GRADES      = voti.txt" << endl
    << "SERIALS     = serials.txt" << endl
    << "REPORT      = report.txt" << endl
    << "BINS        = [ 0 10 20 30 ]" << endl << endl;
  fileout.close();   
  exit(-1); 
  }

// Safe CONFIG file parsing
  string grades_name, serials_name, report_name, work_folder;
  vector<double> bin_pivot;
  string key, equal, value;
  ifstream filein(config_name);
  if( !filein ) {
    cout << "Configuration file " << config_name << " not found. Quitting..." << endl;
    exit(3);
  }
  while ( filein >> key >> equal >> value ){
    if ( key == "WORK_FOLDER" ){
      work_folder = value;
    }
    else if ( key == "GRADES" ){
      grades_name = value;
    }
    else if ( key == "SERIALS" ){
      serials_name = value;
    }
    else if ( key == "REPORT" ){
      report_name = value;
    }
    else if ( key == "BINS" && value == "[" ){
      while( 1 ){
        filein >> value;
        if( value == "]" ) break;
        bin_pivot.push_back( atof(value.c_str() ) );
      }
    }
    else{
      cout << "Key " << key << " unknown. Edit " << config_name << endl;
      exit(3);
    }
  }
  filein.close();

  if ( work_folder == "" ){
    cout << "WORKING folder unset. Edit " << config_name << endl;
    exit(3);
  }
  if ( grades_name == "" ){
    cout << "GRADES file unset. Edit " << config_name << endl;
    exit(3);
  }
  if ( serials_name == "" ){
    cout << "SERIALS file unset. Edit " << config_name << endl;
    exit(3);
  }
  if ( report_name == "" ){
    cout << "REPORT file unset. Edit " << config_name << endl;
    exit(3);
  }
  if ( bin_pivot.size() < 2 ){
    cout << "BINS values unset. Edit " << config_name << endl;
    exit(3);        
  }

// Variables and containers
  string line;
  vector<string> tokens;
  map<string, Question_param> question_map;
  map<int, vector<string> > serials_map;
  int exam_number = 0, pad_name = 0;
  map<int, int> pardon_bins;

// Import GRADES file
  Call call;
  filein.open( work_folder+"/"+grades_name );
  if( !filein ) { 
    cout << "GRADES file " << grades_name << " not found. Quitting..." << endl; 
    exit(4);
  }
  while ( getline(filein, line) ){
    trim(line);
    split(tokens, line, is_any_of("\t"), token_compress_on);
    if ( tokens.size() > 8 ) call.exams.push_back( Exam(tokens,'s') );
  }
  filein.close();

// Import SERIALS file
  filein.open(work_folder+"/"+serials_name);
  if(!filein){
    std::cout << "SERIALS file not found. Quitting..." << std::endl;
    exit(2);
  }
  getline(filein, line);             // here to skip header line
  while( getline(filein, line) ){
    trim_if(line, boost::is_any_of(" "));
    split(tokens, line, boost::is_any_of(" "), boost::token_compress_on);
    for(size_t i=1; i<tokens.size()-1; i++){
      if( tokens[i].size()>pad_name ) pad_name = (int) tokens[i].size();
      serials_map[atoi(tokens[0].c_str())].push_back(tokens[i]);
    }
    call.add_serial(tokens);
  }
  filein.close();

// Evaluate stats
  for( auto exam : call.exams ){    
    for( int i=0; i<exam.answers.size(); i++){      
      question_map[ call.serials_map[exam.serial].second[i] ].repetitions++;
      if ( exam.answers[i] == '-' ){
        question_map[ call.serials_map[exam.serial].second[i] ].blank++;
      }
      else if ( exam.answers[i] == exam.solutions[i] ){
        question_map[ call.serials_map[exam.serial].second[i] ].correct++;
      }
      else{
        question_map[ call.serials_map[exam.serial].second[i] ].wrong ++;
      }
    }
    pardon_bins[ (int) round(exam.pardon_score - exam.score) ]++;
  }

// Total question result counters
  int tot_r = 0, tot_c = 0, tot_w = 0, tot_b = 0;
  for( auto it = question_map.begin(); it != question_map.end(); it++){
    if( it->second.repetitions != it->second.correct + it->second.wrong + it->second.blank ) 
      cout << "ERROR --- " << it->first << "  counters mismatch" << endl;
    tot_r += it->second.repetitions;
    tot_c += it->second.correct;
    tot_w += it->second.wrong;
    tot_b += it->second.blank;
  }

// Evaluating grade bins
  vector<int> bin_freq(bin_pivot.size()-1);
  for( auto exam : call.exams ){
    int index=-1;
    for( auto b : bin_pivot ){
      if ( exam.grade < b ) break;
      index++;
      if ( index > bin_pivot.size()-2 ) index = (int) bin_pivot.size()-2; // to include grades in the last bin
    }
    bin_freq[index]++;
  }
  int bin_tot=0;
  for(auto f : bin_freq ){
    bin_tot += f;
  }

// Dumping REPORT
  ofstream fileout(work_folder+"/"+report_name);
  int pad_count = 5, pad_perc = 5;
  fileout << "Database size     : " << question_map.size() << " questions" << endl
      << "Call size         : " << call.exams.size() << " students" << endl
      << "Bugged exams      : " << endl;
  int tot = 0;
  for( auto it = pardon_bins.begin(); it != pardon_bins.end(); it++){
    fileout << "\tBugs : " << setw(2) << it->first/2 << " -> " << setw(3) << it->second << " ( " 
       << setw(5) << fixed << setprecision(1) << 100*it->second/double(call.exams.size()) << " % ) " << endl;
    tot += it->second;
  }
  fileout << "\tTotal -----> " << setw(3) << tot << " ( " << setw(5) << 100*tot/double(call.exams.size()) << " % ) " << endl;
  fileout << "Tot questions     : " << call.exams.size()*call.exams[0].answers.size() << " = " << tot_r << endl;
  fileout << "Coarse grade bins : " << endl;
  for( int i=0; i<bin_freq.size(); i++){
    fileout << "\t] " << setw(5) << bin_pivot[i] << " , " << setw(5) << bin_pivot[i+1] << " ] -> " 
        << setw(3) << bin_freq[i] << " ( " 
        << setw(5) << fixed << setprecision(1) << 100*bin_freq[i]/double(call.exams.size()) << " % )" << endl;
  }
  fileout << "\tTotal  ------------> " << setw(3) << bin_tot << " ( " 
      << setw(5) << fixed << setprecision(1) << 100*bin_tot/double(call.exams.size()) 
      << " % ) " << endl << endl;
  fileout << std::left  << std::setw(pad_name+4) << "EX NAME" 
      << std::right << std::setw(pad_count) << "TOT" 
      << std::right << std::setw(pad_count+2*pad_perc+2+9) << "CORRECT" 
      << std::right << std::setw(pad_count+2*pad_perc+2+7) << "WRONG" 
      << std::right << std::setw(pad_count+2*pad_perc+2+7) << "BLANK" 
      << std::endl << std::endl;
  for( auto it = question_map.begin(); it != question_map.end(); it++){
    fileout << std::left  << std::setw(pad_name)  << it->first << " ->" 
        << std::right << std::setw(pad_count) << it->second.repetitions << "  : "
        << std::right << std::setw(pad_count) << it->second.correct << " ( " 
        << std::fixed << std::setprecision(1) << std::setw(pad_perc) << (float) it->second.correct/it->second.repetitions*100 << " - "
        << std::fixed << std::setprecision(1) << std::setw(pad_perc-1) << binomial_dist(it->second.repetitions, it->second.correct, 0.25)*100 << " %) "
        << std::right << std::setw(pad_count) << it->second.wrong << " ( " 
        << std::fixed << std::setprecision(1) << std::setw(pad_perc) << (float) it->second.wrong/it->second.repetitions*100 << " - "
        << std::fixed << std::setprecision(1) << std::setw(pad_perc-1) << binomial_dist(it->second.repetitions, it->second.wrong, 0.75)*100 << " %) "
        << std::right << std::setw(pad_count) << it->second.blank << " ( " 
        << std::fixed << std::setprecision(1) << std::setw(pad_perc) << (float) it->second.blank/it->second.repetitions*100 << " - "
        << std::fixed << std::setprecision(1) << std::setw(pad_perc-1) << binomial_dist(it->second.repetitions, it->second.blank, 0.5)*100 << " %) " << std::endl;
  }
  fileout << std::string(pad_name+4+4*pad_count+6*pad_perc+6+23,'-') << endl;
  fileout << setw(pad_name+3) <<  "Total       ->" << std::setw(pad_count) << tot_r << "  : " 
      << std::setw(pad_count) << tot_c << " ( " 
      << std::setprecision(1) << std::setw(pad_perc) << (float) tot_c/tot_r*100 << " - "
      << std::fixed << std::setprecision(1) << std::setw(pad_perc-1) << binomial_dist(tot_r, tot_c, 0.25)*100 << " %) "
      << std::setw(pad_count) << tot_w << " ( " 
      << std::setprecision(1) << std::setw(pad_perc) << (float) tot_w/tot_r*100 << " - "
      << std::fixed << std::setprecision(1) << std::setw(pad_perc-1) << binomial_dist(tot_r, tot_w, 0.75)*100 << " %) "
      << std::setw(pad_count) << tot_b << " ( " 
      << std::setprecision(1) << std::setw(pad_perc) << (float) tot_b/tot_r*100 << " - "
      << std::fixed << std::setprecision(1) << std::setw(pad_perc-1) << binomial_dist(tot_r, tot_b, 0.5)*100 << " %) " << endl;
  fileout.close();

  return 0;
}