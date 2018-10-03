#ifndef EXIT_FAILURE
#define EXIT_FAILURE 42
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#include "csh_main.c"

int main(int argc, char **argv){
	//load config files
	
	//run main loop
	csh_main_loop();
	
	//cleanup
	
	return EXIT_SUCCESS;
}
