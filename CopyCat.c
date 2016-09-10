#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int param; 

void processString(int argc, char** argv){

	if(argc < 2)
		printf("Invalid input!");
	else{
		char key[]= "-b"; 
		if(!strcmp(key, argv[1])){
			param = atoi(argv[2]);

			if(param == 0){
				printf("Invalid parameter detected!\n"); 
			} 
			printf("%d", param); 
			for(int i = 3; i < argc; i++){
				printf("%s\n", argv[i]); 
			} 
		}
		else{
			for(int i = 1; i < argc; i++){
				printf("%s\n", argv[i]);   
			}
		}
	}
 
}

int main(int argc, char** argv){

	processString(argc, argv); 
	return 0; 
}

