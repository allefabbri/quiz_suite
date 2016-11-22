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

#include "latex_headers.hpp"
#include "config.hpp"

constexpr int MAJOR = 2;
constexpr int MINOR = 0;

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
  cout << "QuizCorrections v" << MAJOR << "." << MINOR << endl;

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
  Call call;
  CorrectionConfig c(config_name, &call);
  c.parsefile();
  if (!c.check_params()) exit(4);

  // Variables and containers
  string line, file_path;
  vector<string> tokens;

  // Importing grades file
  file_path = c.work_folder + "/" + c.grades_name;
  ifstream filein(file_path);
  if (!filein) {
    cout << "GRADES file " << file_path << " not found. Quitting..." << endl;
    exit(4);
  }
  while (getline(filein, line)) {
    trim(line);
    split(tokens, line, is_any_of("\t"), token_compress_on);
    if (tokens.size() > 5) call.exams.push_back(Exam(tokens, 'c'));
  }
  filein.close();

  // Filling colors vector
  for (size_t i = 0; i < call.exams.size(); i++) {
    for (size_t j = 0; j < call.exams[i].answers.size(); j++) {
      if (call.exams[i].solutions[j] == '-') {
        call.exams[i].colors[j] = "green";
      }
      else {
        if (call.exams[i].answers[j] == '-')
          call.exams[i].colors[j] = "black";
        else if (call.exams[i].answers[j] == call.exams[i].solutions[j])
          call.exams[i].colors[j] = "green";
        else
          call.exams[i].colors[j] = "red";
      }
    }
  }

  // Creating latex content
  file_path = c.work_folder + "/corrections-content_" + call.name + ".tex";
  ofstream fileout(file_path);
  if (!fileout) {
    cout << "LATEX file " << file_path << " impossible to create. Quitting..." << endl;
    exit(4);
  }
  fileout << endl;
  for (size_t i = 0; i < call.exams.size(); i++) {
    fileout << "\\paperheader" << endl;
    fileout << "Seriale: {\\bf\\Large " << call.exams[i].serial << "} \\quad Cognome e Nome: \\underline{\\Large " << call.exams[i].student << "}" << endl;
    fileout << "\\def\\spazio{1.17cm}" << endl;
    fileout << "\\begin{center}" << endl;
    fileout << "\t\\begin{tabular}{ | ";
    int row_size;
    if (call.exams[i].answers.size() % 3 == 0)
      row_size = (int)call.exams[i].answers.size() / 3;
    else
      row_size = (int)call.exams[i].answers.size() / 3 + 1;
    for (size_t j = 0; j < row_size; j++) fileout << "p{\\spazio} | ";
    fileout << "}" << endl;
    fileout << "\t\t\\hline" << endl;
    for (size_t j = 0; j < 3 * row_size; j++) {
      if (j < call.exams[i].answers.size())
        fileout << ((j%row_size == 0) ? "\t\t" : "  ") << j + 1 << " \\newline \\centerline {\\Large \\textcolor{"
                << call.exams[i].colors[j] << "}{" << call.exams[i].solutions[j] << "} }  "
                << (((j + 1) % row_size) ? "&" : "\\\\[3ex]\n\t\t\\hline\n");
      else
        fileout << " \\ \\newline \\centerline \\ " << (((j + 1) % row_size) ? "&" : "\\\\[0.4cm]\n\t\t\\hline\n");
    }
    fileout << "\t\\end{tabular}" << endl;
    fileout << "\\end{center}" << endl;
    fileout << "Punteggio: {\\bf\\large " << fixed << setprecision(2) << call.exams[i].pardon_score
            << "} \\qquad Voto: {\\LARGE \\boxed{\\bf " << call.exams[i].grade << "} }" << endl;
    fileout << "\n\\noindent\n\\paperfooter" << endl;
    fileout << "\\newpage" << endl << endl << endl;
  }
  fileout.close();

  if (c.make_public_correction) {
    file_path = c.work_folder + "/public-content_" + call.name + ".tex";
    fileout.open(file_path);
    if (!fileout) {
      cout << "LATEX file " << file_path << " impossible to create. Quitting..." << endl;
      exit(4);
    }
    fileout << endl;
    for (size_t i = 0; i < call.exams.size(); i++) {
      fileout << "\\paperheader" << endl;
      fileout << "Seriale: {\\bf\\Large " << call.exams[i].serial << "} \\quad Cognome e Nome: \\underline{\\Large " << call.exams[i].student << "}" << endl;
      fileout << "\\def\\spazio{1.17cm}" << endl;
      fileout << "\\begin{center}" << endl;
      fileout << "\t\\begin{tabular}{ | ";
      int row_size;
      if (call.exams[i].answers.size() % 3 == 0)
        row_size = (int)call.exams[i].answers.size() / 3;
      else
        row_size = (int)call.exams[i].answers.size() / 3 + 1;
      for (size_t j = 0; j < row_size; j++) fileout << "p{\\spazio} | ";
      fileout << "}" << endl;
      fileout << "\t\t\\hline" << endl;
      for (size_t j = 0; j < 3 * row_size; j++) {
        if (j < call.exams[i].answers.size())
          fileout << ((j%row_size == 0) ? "\t\t" : "  ") << j + 1 << " \\newline \\centerline {\\Large \\textcolor{"
          << call.exams[i].colors[j] << "}{" << call.exams[i].solutions[j] << "} }  "
          << (((j + 1) % row_size) ? "&" : "\\\\[3ex]\n\t\t\\hline\n");
        else
          fileout << " \\ \\newline \\centerline \\ " << (((j + 1) % row_size) ? "&" : "\\\\[0.4cm]\n\t\t\\hline\n");
      }
      fileout << "\t\\end{tabular}" << endl;
      fileout << "\\end{center}" << endl;
      fileout << "Esito: {\\Large \\boxed{ \\bf " << grade2outcome(c.thresholds, call.exams[i].grade_d) << " } } " << endl;
      fileout << "\n\\noindent\n\\paperfooter" << endl;
      fileout << "\\newpage" << endl << endl << endl;
    }
    fileout.close();
  }

  // Creating latex forms
  file_path = c.work_folder + "/corrections-form.tex";
  fileout.open(file_path);
  if (!fileout) {
    cout << "LATEX file " << file_path << " impossible to create. Quitting..." << endl;
    exit(4);
  }
  fileout << corrections_form(call);
  fileout.close();

  if (c.make_public_correction) {
    file_path = c.work_folder + "/public-form.tex";
    fileout.open(file_path);
    if (!fileout) {
      cout << "LATEX file " << file_path << " impossible to create. Quitting..." << endl;
      exit(4);
    }
    fileout << public_form(call);
    fileout.close();
  }

  // Command line suggestion
  cout << "To generate the pdf's please type :\ncd " << c.work_folder << " && for form in *-form.tex; do pdflatex.exe $form; done && cd -" << endl;

  return 0;
}