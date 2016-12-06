/////////////////////////////
////////// SMALLSH //////////
/////// Corwin Perren ///////
/////////////////////////////

//////////////////////////////////////////
////////// Includes and Defines //////////
//////////////////////////////////////////
//Includes
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

//Defines
#define INPUT_LENGTH_MAX 2048
#define INPUT_ARGS_MAX 513  //Includes one extra for the program itself

/////////////////////////////////////////
////////// Function Prototypes //////////
/////////////////////////////////////////
void interrupt_signal_handler(int signal_number);
void terminate_signal_handler(int signal_number);
bool is_blank_input_string(char* input_string);
int split_input_to_array(char* input_string, char** output_array);
void clean_newline(char* input_string);
void clean_array(char** to_clean);

//////////////////////////
////////// Main //////////
//////////////////////////
int main() {
    ////////// SMALLSH Variables //////////
    char user_input_string[INPUT_LENGTH_MAX];
    int input_arg_count = 0;
    char** user_input_array = malloc(INPUT_ARGS_MAX * sizeof(char*));
    int smallsh_status = 0;
    
    //Assign initial pointers to NULL so cleaning functions are happy
    //This is for user_input_array
    for(int i = 0 ; i < INPUT_ARGS_MAX ; i++){
        user_input_array[i] = NULL;
    }
    
    ////////// SMALLSH Initialization //////////
    //Clear previous terminal so we've just got our new one
    //Not necessary, but looks nice
    system("clear");
    
    //Assign signal handlers
    //signal(SIGINT, interrupt_signal_handler);
    //signal(SIGTERM, terminate_signal_handler);

    ////////// Print welcome screen //////////
    printf("----------------------\n");
    printf("Welcome to Small Shell\n");
    printf("http://caperren.com\n");
    printf("----------------------\n");
    
    ////////// Main program code //////////
    while(1){
        //Process child processes 

        //Print prompt
        printf(": ");
        fflush(stdout);
        
        //Clear input buffer and read in new command
        memset(user_input_string, '\0', INPUT_LENGTH_MAX);
        fgets(user_input_string, INPUT_LENGTH_MAX, stdin);
        
        //Check if input is blank
        if(is_blank_input_string(user_input_string)){
            continue;
        }
        
        //Clean off newline
        clean_newline(user_input_string);
        
        //Break input string into an array of the arguments
        input_arg_count = split_input_to_array(user_input_string, \
                                               user_input_array);
        
        //Check what kind of input we got
        if(strcmp(user_input_array[0], "exit") == 0){
            //We should clean up malloc'd memory and exit
            clean_array(user_input_array);
            free(user_input_array);
            exit(EXIT_SUCCESS);
            
        }else if(strcmp(user_input_array[0], "status") == 0){
            //Print our exit status variable, and continue
            printf("exit value %d\n", WEXITSTATUS(smallsh_status));
            fflush(stdout);
            continue;
            
        }else if(strcmp(user_input_array[0], "cd") == 0){
            //Check if we're going to the home directory, or somewhere else
            if((user_input_array[1] == NULL) || \
               (strcmp(user_input_array[1], "~/") == 0) || \
               (strcmp(user_input_array[1], "~") == 0)){
                //Change to the user's home directory as requested
                chdir(getenv("HOME"));
                
            }else{
                //Change to the user's requested directory, error is not
                //accessible, or doesn't exist
                int chdir_result = chdir(user_input_array[1]);
                if(chdir_result == -1){
                    printf("Cannot change to directory \"%s\"\n", \
                           user_input_array[1]);
                    fflush(stdout);
                }
            }
            continue;
        }else if(user_input_array[0][0] == '#'){
            //We got a comment line, so do nothing and continue
            continue;
        }
        
        
    }

    exit(EXIT_SUCCESS);
}

///////////////////////////////////////////
////////// Function Declarations //////////
///////////////////////////////////////////
void interrupt_signal_handler(int signal_number){
    
}

void terminate_signal_handler(int signal_number){
    
}

bool is_blank_input_string(char* input_string){
    int length = strlen(input_string);
    char current_char = '\0';
    for(int i = 0 ; i < length ; i++){
        current_char = input_string[i];
        if((current_char != ' ') && (current_char != '\0') && \
                                    (current_char != '\n')){
            return false;
        }
    }
    return true;
}

int split_input_to_array(char* input_string, char** output_array){
    int args_count = 0;
    char* token_result;
    
    clean_array(output_array);
    
    for(args_count = 0 ; args_count < INPUT_ARGS_MAX ; args_count++){
        //Get the current tokenizer result
        if(args_count == 0){
            token_result = strtok(input_string, " ");
        }else{
            token_result = strtok(NULL, " ");
        }
        
        //Check if we're done parsing input string
        if(token_result == NULL){
            break;
        }
        
        //Allocate space for next string
        output_array[args_count] = malloc((strlen(token_result) + 1) * \
                                           sizeof(char));
        
        //Copy the current string into this allocated space
        strcpy(output_array[args_count], token_result);
    }
    
    return args_count;
}

void clean_newline(char* input_string){
    input_string[strlen(input_string) - 1] = '\0';
}

void clean_array(char** to_clean){
    for(int i = 0 ; i < INPUT_ARGS_MAX ; i++){
        if(to_clean[i] != NULL){
            free(to_clean[i]);
            to_clean[i] = NULL;
        }else{
            break;
        }
    }
}