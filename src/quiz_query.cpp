#include <iostream>
#include <fstream>
#include <cstdlib>

#include "quiz_lib.hpp"

// quiz_query -[s|r] [filename] [PARAMS]

void usage(char* progname){
	cout << "Usage: " << progname << " /path/to/serial_file <ser> <quiz>" << endl
	     << "       Returns quiz-name string" << endl << endl
		 << "Usage: " << progname << "  /path/to/serial_file <ser> <quiz1> <quiz2> ... <quizN>" << endl
		 << "       Returns quiz1-name quiz2-name ... quizN-name" << endl << endl
		 << "Usage: " << progname << "  /path/to/serial_file <ser> <quiz1> : <quizN>" << endl 
		 << "       Returns quiz1-name quiz2-name ... quizN-name" << endl << endl
		 << "Usage: " << progname << "  /path/to/serial_file quiz-name" << endl 
		 << "       Returns quiz1-name quiz2-name ... quizN-name" << endl << endl;
}

int main(int argc, char** argv){
	int serial;
	vector<int> quiz_num;
	string quiz_name="", filename;
	if( argc > 3 ){
		filename = argv[1];
		serial = atoi(argv[2]);
		for(int i=3; i<argc; i++){
			if( string(argv[i]) == ":" ){
				int quiz_min=atoi(argv[i-1]);
				int quiz_max=atoi(argv[++i]);
				for(int j=quiz_min+1; j<=quiz_max; j++){
					quiz_num.push_back(j);
				}
			}
			else{
				quiz_num.push_back(atoi(argv[i]));
			}
		}
	}
	else if( argc == 3 ){
		filename = argv[1];
		quiz_name = argv[2];
	}
	else{
		usage(argv[0]);
		exit(1);
	}
	sort(quiz_num.begin(), quiz_num.end());

	// Vars
	Call call;
	string line;
	vector<string> tokens;

	// Serials file parsing
	ifstream filein(filename);
	if( !filein ) { 
	  cout << "SERIALS file " << filename << " not found. Quitting..." << endl; 
	  exit(5);
	}
	while( getline(filein,line) ){
	  trim(line);
	  if (line[0] == '%') continue;
	  split(tokens, line, is_any_of("\t "), token_compress_on);
	  if( tokens.size() > 6 ) {   // to skip segfault causing lines, if any
	    call.add_serial(tokens);
	  }
	}
	filein.close();

	// Runtime branches
	cout << "Serial file    : " << filename << endl;
	if( quiz_name == "" ){
		if( call.serials_map.count(serial) == 0 ) {
			cout << "Invalid serial : " << serial << endl;
			exit(2);
		}
		if( quiz_num[0] < 1 || quiz_num.back() >= call.serials_map.rbegin()->first ){
			cout << "Invalid quiz numbers : "; 
			for( auto n : quiz_num ) cout << n << "  ";
			cout << endl;
			exit(3);
		}

		cout << "Serial number  : " << serial << endl
			 << "Quiz number(s) : ";
		for( auto n : quiz_num ) cout << n << "  ";
		cout << endl;

		cout << endl << "Quiz names     : ";
		for( auto n : quiz_num ) cout << call.serials_map[serial].second[n-1] << "   ";
		cout << endl;
	}
	else{
		cout << "Quiz name      : " << quiz_name << endl;
		vector<pair<int, int>> result; 
		for( auto it = call.serials_map.begin(); it != call.serials_map.end(); it++){
			for( int i=0; i < it->second.second.size(); i++ ){
				if( it->second.second[i] == quiz_name ) result.push_back( make_pair(it->first, i+1) );
			}
		}

		if( result.size() == 0 ){
			cout << "Quiz name " << quiz_name << " not found." << endl;
			exit(4);
		}

		cout << endl << "Ser-Num pairs (" << result.size() << ") : ";
		for( auto r : result ) cout << r.first << "-" << r.second << "   ";
		cout << endl;
	}

	return 0;
}



