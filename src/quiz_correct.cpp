#define _SCL_SECURE_NO_WARNINGS

#include <iomanip>
#include <fstream>
#include <cstdlib>

#include "latex_headers.hpp"

#define MAJOR      1
#define MINOR      0

int main(int argc, char ** argv){
  cout << "QuizCorrections v" << MAJOR << "." << MINOR << endl;

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

// Generate empty config file and quit
  if ( config_name == "-conf_t") {
    cout << "Generating empty config file named \"correction.config\"" << endl;
    std::ofstream config("correction.config");
    config
      << "CALL_NAME   = appello1" << endl
      << "CALL_DATE   = 17/9/1993" << endl
      << "COURSE      = Paleontologia Applicata" << endl
      << "TAG         = Appello I - Sessione autunnale - A.A. 1993/94" << endl
      << "CDL         = Corso di Laurea in Paleontologia" << endl
      << "WORK_FOLDER = appello1" << endl
      << "GRADES_FILE = voti.txt" << endl << endl;
    config.close();
    exit(-1);
  }

// Safe CONFIG file parsing
  Call call;
  string grades_name, work_folder;
  string key, equal, value;
  ifstream filein(config_name);
  if( !filein ) {
    cout << "Configuration file " << config_name << " not found. Quitting..." << endl;
    exit(3);
  }
  while ( filein >> key >> equal >> value ){
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
    else if ( key == "WORK_FOLDER" ){
      work_folder = value;
    }
    else if ( key == "GRADES_FILE" ){
      grades_name = value;
    }
    else{
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
  if (work_folder == "") {
    cout << "WORKING FOLDER unset. Edit " << config_name << endl;
    exit(3);
  }
  if (grades_name == "") {
    cout << "GRADES FILE unset. Edit " << config_name << endl;
    exit(3);
  }

// Variables and container
  string line;
  vector<string> tokens;

// Importing grades file
  filein.open(work_folder+"/"+grades_name);
  if( !filein ) { 
    cout << "GRADES file " << grades_name << " not found. Quitting..." << endl; 
    exit(4);
  }


  while ( getline(filein, line) ){
    trim(line);
    split(tokens, line, is_any_of("\t"), token_compress_on);
    if ( tokens.size() > 5 ) call.exams.push_back( Exam(tokens, 'c') );
  }
  filein.close();

  
// Filling colors vector
  for(size_t i=0; i<call.exams.size(); i++){
    for(size_t j=0; j<call.exams[i].answers.size(); j++){
      if( call.exams[i].solutions[j] == '-' ){
        call.exams[i].colors[j] = "green";
      }
      else{
        if( call.exams[i].answers[j] == '-' )
          call.exams[i].colors[j] = "black"; 
        else if( call.exams[i].answers[j] == call.exams[i].solutions[j] )
          call.exams[i].colors[j] = "green";
        else
          call.exams[i].colors[j] = "red";
      }
    }
  }
  
// Dumping latex content
  ofstream fileout;
  fileout.open( work_folder+"/corrections-content_"+call.name+".tex" );
  if( !fileout ){
    cout << "LATEX file not opened. Quitting..." << endl;
    exit(4);
  }
  fileout << endl;
  for( size_t i=0; i<call.exams.size(); i++ ){
    fileout << "\\paperheader" << endl;
    fileout << "Seriale: {\\bf\\Large " << call.exams[i].serial << "} \\quad Cognome e Nome: \\underline{\\Large " << call.exams[i].student << "}" << endl;
    fileout << "\\def\\spazio{1.17cm}" << endl;
    fileout << "\\begin{center}" << endl;
    fileout << "\t\\begin{tabular}{ | ";
    int row_size;
    if( call.exams[i].answers.size()%3 == 0 )
      row_size = (int) call.exams[i].answers.size()/3;
    else
      row_size = (int) call.exams[i].answers.size()/3 +1;
    for(size_t j=0; j<row_size; j++) fileout << "p{\\spazio} | ";
      fileout << "}" << endl;
    fileout << "\t\t\\hline" << endl;
    for(size_t j=0; j<3*row_size; j++){
      if( j < call.exams[i].answers.size() )
        fileout << ( (j%row_size==0)?"\t\t":"  " ) << j+1 << " \\newline \\centerline {\\Large \\textcolor{" 
                << call.exams[i].colors[j] << "}{" << call.exams[i].solutions[j] << "} }  " 
                << ( ( (j+1)%row_size )?"&":"\\\\[3ex]\n\t\t\\hline\n" );
      else
        fileout << " \\ \\newline \\centerline \\ " << ( ( (j+1)%row_size )?"&":"\\\\[0.4cm]\n\t\t\\hline\n" );
    }
    fileout << "\t\\end{tabular}" << endl;
    fileout << "\\end{center}" << endl;
    fileout << "Punteggio: {\\bf\\large " << fixed << setprecision(2) << call.exams[i].pardon_score
            << "} \\qquad Voto: {\\LARGE \\boxed{\\bf " << call.exams[i].grade << "} }" << endl;
    fileout << "\n\\noindent\n\\paperfooter" << endl;
    fileout << "\\newpage" << endl << endl << endl;
  }
  fileout.close();

// Creating latex form
  fileout.open(work_folder + "/corrections-form.tex");
  fileout << corrections_form(call);
  fileout.close();

  // Command line suggestion
  cout << "To generate the pdf please type :\ncd " << work_folder << " && pdflatex.exe corrections-form.tex && cd -" << endl;

  return 0;
}