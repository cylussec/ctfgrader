#include <stdio.h>			//printf
#include <unistd.h>			//getcwd, system	
#include <boost/filesystem.hpp>		//fs::path
#include <boost/program_options.hpp>	//fs::program_options
#include <strings.h>			//strings
#include <sqlite3.h>			//sqlite

using namespace std;

#define DATABASE	"ctf.db"

int validateargs(const string args);
const unsigned char* checker(string challenge, string retval);


/*
 * Database structure
 *
 * Table: 	CTF
 * Columns: 	CHALLENGE, ANSWER, KEY
 */

int main(int argc, char* argv[]) {
    namespace po = boost::program_options;
    string args;

    po::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "produce help message")
    ("challenge,c", po::value<string>()->required(), "challenge code")
    ("file", po::value<string>()->required(),
     "the source code to be evaluated")
    ("challengeargs,a", po::value<string>(),
     "Args to be passed to the binary compiled from FILE")
    ("compilerargs,o", po::value<string>(),
     "Args to be appended to the g++ command");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
      cout << desc << endl;
      return -1;
    }
    if (!vm.count("challenge")) {
      cout << desc << endl;
      return -1;
    }
    if (!vm.count("file")) {
      cout << desc << endl;
      return -1;
    }
    if (validateargs(vm["challengeargs"].as<string>()) != 0) {
        cerr << "Invalid challenge args: illegal char" << endl;
    }
    if (validateargs(vm["compilerargs"].as<string>()) != 0) {
        cerr << "Invalid compiler args: illegal char" << endl;
    }

    //lets build the full path
    namespace fs = boost::filesystem;
    fs::path dir(boost::filesystem::current_path());
    fs::path full_path = dir / vm["file"].as< string>();

    std::cout << "[+] Compiling " << full_path << std::endl;

#define CSTRLEN 	256
#define BINARYNAME	"ctfout"
    char systemstr[CSTRLEN];

    snprintf(systemstr, CSTRLEN, "g++ %s -o %s %s",
             full_path.string().c_str(), BINARYNAME, args.c_str());

    //compile the user's code
    FILE *systemcmd = popen(systemstr, "r");
#define CMDOUTPUTLEN 	1024
    char cmdoutput[CMDOUTPUTLEN];
    if (!systemcmd) return -1;

    cout << "Command: " << systemstr << endl;
    while (fread(cmdoutput, 1,1, systemcmd)){
	cout << cmdoutput;
    }

    //run the user's binary
    char binaryoutput[256];
    snprintf(systemstr, CSTRLEN, "./%s %s", BINARYNAME,
             vm["challengeargs"].as<string>().c_str());
    FILE *usercmd = popen(systemstr, "r");
    if (!usercmd) return -1;
    fgets(cmdoutput, CMDOUTPUTLEN, usercmd);
    cout << "Key: " << cmdoutput << endl;
    checker(vm["challenge"].as<string>(),
            cmdoutput);
}

int validateargs(const string iargs) {
    /*
     * This gets a set of arguments and returns if they are valid
     *
     * INPUT: iargs - arguments to validate
     * OUTPUT: -1 if they are invalid; 0 if they are valid
     */
#define VALIDCHARS " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-"
    string args(iargs);
    int i = args.find_first_not_of(VALIDCHARS);
    if(i >=0) {
        std::cerr << "Invalid character in argument: " << args << std::endl << "Valid Chars: " << VALIDCHARS << " at index: " << i << endl;
        return -1;
    } else {
	return 0;
    }
}


const unsigned char* checker(string challenge, string retval) {
    /*
     * checker - pass it a challenge and the binary's return value and it
     * will check it
     *
     * INPUT
     * 	challenge - the challenge code; usually character followed by
     * 		number
     * 	retval - the return value from the binary
     * OUTPUT
     * 	char* - the key if the check was successful (otherwise NULL)
     */

    sqlite3 *db;
    int status;

    if (sqlite3_open(DATABASE, &db)) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return NULL;
    }

    char *errMsg;
    sqlite3_stmt *statement;
#define SQLLEN	256
    char sql[256];
    strncpy(sql, "SELECT * from ctf WHERE challenge = ?", SQLLEN);

    int i = sqlite3_prepare(db,sql, -1, &statement, 0);
    if (i == SQLITE_OK) {
        sqlite3_bind_text(statement, 1, challenge.c_str(),
                          strlen(challenge.c_str()), 0);
        while(true) {
            if(sqlite3_step(statement)==SQLITE_ROW) {
                if (strcmp(retval.c_str(),(char*)sqlite3_column_text(statement, 1)) != 0) {
                    //column 1 is the answer. if it matches
                    //then we return the key

                    cout << "Success! Key is " <<
			 sqlite3_column_text(statement, 2) << endl;
			return sqlite3_column_text(statement, 2);
                } else {
		    cerr << "Failure: return from program (" << 
		          retval << ") for challenge " << challenge << endl; 
                    return NULL;
                }
            }
        }
    } else {
      cerr << "sqlite_prepare " << i << endl;
    }
}
