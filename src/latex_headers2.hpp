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

#include <sstream>

#include "quiz_lib2.hpp"

//////////////////////////////////////////// GENERAL
const char * latex_header = R"(
\documentclass[11pt,a4paper]{article}
\usepackage{amsfonts,amsmath,latexsym,color}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\voffset        -2.0cm
\oddsidemargin  -1.3cm
\evensidemargin -1.3cm
\textwidth      19.1cm
\textheight     27.0cm
\topmargin       0.0cm
\headheight      0.0cm
\footskip        0.0cm
\parskip         2.5mm
%%% defs
\def \parton#1{\left( #1 \right)}                   % Adaptive round brackets
\def \parqua#1{\left[ #1 \right]}                   % Adaptive square brackets
\def \pargra#1{\left\{ #1 \right\}}                 % Adaptive curly brackets
\def \mod#1{\left| #1 \right|}                      % Adaptive modulus
\def \der#1#2{\frac{d #1}{d #2}}                    % Ordinary derivative df/dx
\def \pder#1#2{\frac{\partial #1}{\partial #2}}     % Partial derivative
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\begin{document}

%%% Print no page numbers
\pagestyle{empty}

)";


//////////////////////////////////////////// QUIZ_GEN
string exam_tex_title = R"(
{\sc Fisica Generale T-1}\\
\Large 4/12/2015 - II Parziale - A.A. 2015/16 \\
Corso di Laurea in Ingegneria dell'Automazione e Ingegneria dell'Energia Elettrica \\
)";

string exam_tex_instructions(double correct, double blank, double wrong){
  stringstream ss;
  ss << "\\underline{ISTRUZIONI}: Compilare la tabella sottostante con la lettera corrispondente all'\\textbf{unica} risposta esatta per ciascun quesito. " 
     << "Ogni risposta esatta vale \\textbf{" << fixed << setprecision(2) << correct 
     << " punti}, ogni risposta lasciata in bianco vale \\textbf{" << fixed << setprecision(2) << blank
     << " punti}, ogni risposta errata vale \\textbf{" << fixed << setprecision(2) << wrong << " punti}.";

  return ss.str();
}

string exam_tex_table(int question_number) {
  stringstream ss;
  ss << R"(\def\spazio{ 1.17 }
    \begin{center}
)";
  ss << R"(\begin{tabular}{)";

  int column_number;
  if( question_number%3 )
    column_number = question_number / 3 +1;
  else
    column_number = question_number / 3;

  for (int i = 0; i < column_number; i++) {
    ss << R"(| p{\spazio cm} )";
  }
  ss << "|}\n\t\\hline" << endl << "\t";
  for (int i = 0; i < question_number; i++) {
    ss << i+1 << ( ((i+1)%column_number==0)?" \\\\[5ex] \\hline \n\t":" & " ) ;
  }

  ss << "\t" << R"(\end{tabular}
    \end{center}
\normalsize
}
)";

  return ss.str();
}

template<typename Call_t> string exam_form(Call_t call) {
  stringstream ss;
  ss << latex_header << endl;
  ss << R"(%%% Print this on each test, before the questions
\newcommand{\paperheader}{
%\newpage
%\thispagestyle{empty}
%\mbox{}
\newpage
%
 \begin{center}
\LARGE
{\sc )" << call.course << R"(}\\
\Large)" << call.date << " - " << call.tag << R"(\\)" << endl
<< call.cdl << R"(\\
\end{center}
\ \\
\large
Seriale: {\bf\Large \serialnumber} \quad Cognome: \underline{\hspace{6.2cm}} Nome: \underline{\hspace{7cm}}

\vspace{1cm}
\noindent
Firma: \underline{\hspace{17.7cm}}

\vspace{1cm}
\noindent
)";
  double correct = call.score_scale / call.exams[0].questions.size(),
    wrong = -correct / (call.exams[0].questions[0].answers.size() - 1),
    blank = 0.0;
  ss << exam_tex_instructions(correct, blank, wrong ) << endl;
  ss << endl;
  ss << exam_tex_table( (int) call.exams[0].questions.size()) << endl;

  ss << R"(
%%% Print this on each test, after the questions
\newcommand{\paperfooter}{

\vspace{1cm}

\noindent
\hrulefill

\noindent
{\small

\noindent \large
Commissione: )" << call.commission << R"( \hfill Bologna, )" << call.date << R"(

\vspace{0.5cm}
\centerline{\Large \bf FOGLIO DA RICONSEGNARE!}
}

})";

  ss << R"(%%% Print this before each question
\newcommand{\questionheader}{\noindent{\bf \serialnumber \ - Quesito \questionnumber}: }
)" << endl;

  ss << R"(%%% Now read in the actual tests
\input exam-content_)" << call.name << R"(.tex

\end{document}
)";



  return ss.str();
}

template<typename Call_t> string database_form(Call_t call) {
  stringstream ss;
  ss << latex_header << endl;

  ss << R"(%%% Read in the database content
\input database-content_)" << call.name << R"(.tex

\end{document}
)";

  return ss.str();
}


//////////////////////////////////////////// QUIZ_CORRECT
template<typename Call_t> string corrections_form(Call_t call) {
  stringstream ss;
  ss << latex_header << endl;
  ss << R"(%%% Print this on each test, before the questions
\newcommand{\paperheader}{
%\newpage
%\thispagestyle{empty}
%\mbox{}
%\newpage
%
\begin{center}
\LARGE
{\sc )" << call.course << R"(}\\
\Large)" << call.date << " - " << call.tag << R"(\\)" << endl
<< call.cdl << R"(\\
\end{center}
}
)";

  ss << R"(
%%% Print this on each test, after the questions
\newcommand{\paperfooter}{LEGENDA: 

\textcolor{green}{Risposta corretta} 

\textcolor{red}{Risposta errata} 

\textcolor{black}{Risposta mancante}  
}

)";

  ss << R"(%%% Now read in the actual tests
\input corrections-content_)" << call.name << R"(.tex

\end{document}
)";

  return ss.str();
}

template<typename Call_t> string public_form(Call_t call) {
  stringstream ss;
  ss << latex_header << endl;
  ss << R"(%%% Print this on each test, before the questions
\newcommand{\paperheader}{
%\newpage
%\thispagestyle{empty}
%\mbox{}
%\newpage
%
\begin{center}
\LARGE
{\sc )" << call.course << R"(}\\
\Large)" << call.date << " - " << call.tag << R"(\\)" << endl
<< call.cdl << R"(\\
\end{center}
}
)";

  ss << R"(
%%% Print this on each test, after the questions
\newcommand{\paperfooter}{LEGENDA: 

\textcolor{green}{Risposta corretta} 

\textcolor{red}{Risposta errata} 

\textcolor{black}{Risposta mancante}  
}

)";

  ss << R"(%%% Now read in the actual tests
\input public-content_)" << call.name << R"(.tex

\end{document}
)";

  return ss.str();
}
