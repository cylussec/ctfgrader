#include <stdio.h>		//printf
#include <unistd.h>		//getcwd, system	
#include <boost/filesystem.hpp>	//fs::path
#include <strings.h>

using namespace std;

int main(int argc, char* argv[]){
	string args;
	if (argc == 4){
		printf("args: %s\n", argv[3]);
		args = string(argv[3]);
		#define VALIDCHARS " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_-"
		int i; 
		i = args.find_first_not_of(VALIDCHARS);
		if (args.find_first_not_of(VALIDCHARS)){
			std::cout << "Invalid character in argument: " << args << std::endl << "Valid Chars: " << VALIDCHARS << " invalid at " << i << endl;
			return -1;
		}
	}else if(argc != 3){
		//if they provided anything other than 3 or 4 args
		printf("USAGE: %s CHALLENGE FILE COMPILERARGS\n"\
			"\tCHALLENGE is the challenge code\n"\
			"\tFILE is the source code to be evaluated.\n\n"\
			"\tCOMPILERARGS are appended to the end of the command"\
			"'g++ FILE'\n"\
			"On success, the key will be printed to stdout\n",
			argv[0]);
		return -1;
	}

	//lets build the full path
	namespace fs = boost::filesystem;
	fs::path dir(boost::filesystem::current_path());
	fs::path file(argv[2]);
	fs::path full_path = dir/file;

	std::cout << "[+] Compiling " << full_path << std::endl;

	#define CSTRLEN 	256
	char compilestr[CSTRLEN];
	snprintf(compilestr, CSTRLEN, "g++ %s", full_path.string().c_str());
	system(compilestr);

}
