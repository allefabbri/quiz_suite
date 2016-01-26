# quiz_suite

This is a **C++** command line suite of programs to randomly generate, grade, correct and analyze a set of exams from a database of multiple choice questions. It is similar in spirit to [much](http://mk.eigen-space.org/much/) whose author, Mihalis Kolountzakis, I thank a lot for inspiration.

Content of the suite:
- `quiz_gen     :` random exam generator
- `quiz_grade   :` grading tool
- `quiz_correct :` solution generator
- `quiz_stat    :` statistical analyzer of the results

### Installation and Requirements
The program is distributed open-source and comes equipped with a makefile and also a VS solution. There are also two scripts, bash and PowerShell, to pilot the generator and automate the process.

What you need:
- a [Latex distribution](https://www.tug.org/texlive/)
- the [Boost libraries](http://www.boost.org/) (under VS Nuget is configured to manage the library autonomously while with make you have to install it in system's path or edit the makefile according to your system specs)
- [Gnuplot](http://www.gnuplot.info/)

### Usage    

The suite is intended as a unique sequential series of tools which help the instructor in handling the various step of the examination process. 