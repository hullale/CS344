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
#include <fcntl.h>

//Defines
#define INPUT_LENGTH_MAX 2048
#define INPUT_ARGS_MAX 513  //Includes one extra for the program itself
#define BACKGROUND_WAIT_COUNT 100000

/////////////////////////////////////////
////////// Function Prototypes //////////
/////////////////////////////////////////
void interrupt_signal_handler(int signal_number);
void terminate_signal_handler(int signal_number);
void check_background_processes(int* status);
bool is_blank_input_string(char* input_string);
int split_input_to_array(char* input_string, char** output_array);
void clean_newline(char* input_string);
void clean_array(char** to_clean);
bool check_if_run_in_background(char** arguments_array, int* args_count);
bool check_if_output_redirect(char** arguments_array, int* args_count, \
                              char* output_filename);
bool check_if_input_redirect(char** arguments_array, int* args_count, \
                             char* input_filename);
void clean_extra_args(char** arguments_array, int args_count);

//////////////////////////
////////// Main //////////
//////////////////////////
int main() {
    ////////// SMALLSH Management Variables //////////
    char user_input_string[INPUT_LENGTH_MAX];
    int input_arg_count = 0;
    char** user_input_array = malloc(INPUT_ARGS_MAX * sizeof(char*));
    static int smallsh_status = 0;
    
    //Assign initial pointers to NULL so cleaning functions are happy
    //This is for user_input_array
    for(int i = 0 ; i < INPUT_ARGS_MAX ; i++){
        user_input_array[i] = NULL;
    }
    
    ////////// SMALLSH Process Spawn Variables //////////
    bool spawn_in_background = false;
    bool redirect_output = false;
    bool redirect_input = false;
    
    char output_filename[INPUT_LENGTH_MAX];
    char input_filename[INPUT_LENGTH_MAX];
    
    int spawn_id = 0;
    
    ////////// SMALLSH Initialization //////////    
    //Assign signal handlers
    signal(SIGINT, interrupt_signal_handler);
    signal(SIGTERM, terminate_signal_handler);

    ////////// Print welcome screen //////////
    printf("----------------------\n");
    printf("Welcome to Small Shell\n");
    printf("http://caperren.com\n");
    printf("----------------------\n");
    
    ////////// Main program code //////////
    while(1){
        //Reset process management variables
        spawn_in_background = false;
        redirect_output = false;
        redirect_input = false;
        
        //Process child processes 
        check_background_processes(&smallsh_status);
        
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
        
        //If we've gotten here, that means the command given is one we're 
        //going to be spawning using an exec call.
        
        //First check is to see whether it should be foreground or not.
        spawn_in_background = check_if_run_in_background(user_input_array, \
                                                         &input_arg_count);
        
        //Now check whether or not we need to redirect output
        redirect_output = check_if_output_redirect(user_input_array, \
                                                   &input_arg_count, \
                                                   output_filename);
        
        //Then the same, but for redirection of input from file
        redirect_input = check_if_input_redirect(user_input_array, \
                                                 &input_arg_count, \
                                                 input_filename);
        
        //Clean out these input/output args we don't want passed to our process
        clean_extra_args(user_input_array, input_arg_count);

        //Time to fork for our new process
        spawn_id = fork();
        
        if(spawn_id == 0){
            //We're the child process, get ready to execute.
            
            if(spawn_in_background){
                //If we're supposed to be in the background, set stdin and
                //stdout file descriptors for the process to /dev/null
                int null_rw = open("/dev/null", O_RDWR);
                int null_read = open("/dev/null", O_RDONLY);
                
                dup2(null_read, 0);
                dup2(null_rw, 1);
            }
            
            if(redirect_input){
                //Even if background, if we redirect input, attempt to open
                //file and pass it in
                int input_fd = open(input_filename, O_RDONLY, 0644);

                if(input_fd < 0){
                    printf("File %s cannot be accessed.\n", input_filename);
                    fflush(stdout);
                    continue;
                }

                dup2(input_fd, 0);
            }

            if(redirect_output){
                //Even if background, attempt to make output file and redirect
                int output_fd = open(output_filename, \
                                 O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if(output_fd < 0){
                    printf("File %s cannot be accessed.\n", input_filename);
                    fflush(stdout);
                    continue;
                }

                dup2(output_fd, 1);
            }
            
            //Execute the command, including searching the path variable
            execvp(user_input_array[0], user_input_array);
            
            printf("Failed to run program: %s\n", user_input_array[0]);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }else{
            //We're the parent
            if(spawn_in_background){
                //Print the process id, and return to prompt
                printf("background pid is %d\n", spawn_id);
                fflush(stdout);
            }else{
                //Wait for the process to die, as this is foreground
                spawn_id = waitpid(spawn_id, &smallsh_status, 0);
                
            } 
        } 
    }

    exit(EXIT_SUCCESS);
}

///////////////////////////////////////////
////////// Function Declarations //////////
///////////////////////////////////////////
void interrupt_signal_handler(int signal_number){
    printf("terminated by signal %d\n", signal_number);
    signal(SIGINT, interrupt_signal_handler);
}

void terminate_signal_handler(int signal_number){
    printf("terminated by signal %d\n", signal_number);
    exit(EXIT_FAILURE);
    signal(SIGTERM, terminate_signal_handler);
}

void check_background_processes(int* status){
        int temp_pid = 0;
        int temp_count = 0;
        
        //Loop through a bunch of times and print out child exit statuses
        while(temp_count < BACKGROUND_WAIT_COUNT){
            temp_pid = waitpid(-1, status, WNOHANG);
            
            if(temp_pid > 0){
                if(WIFEXITED(*status)){
                    printf("process %d exited with exit value %d", temp_pid, \
                            WEXITSTATUS(*status));
                }
                
                if(WIFSIGNALED(*status)){
                    printf("process %d terminated by signal %d", temp_pid, \
                            WTERMSIG(*status));
                }
                
                printf("\n");
                fflush(stdout);
            }
            
            temp_count++;
        }
           
}

bool is_blank_input_string(char* input_string){
    //This does a simple check as to whether we have no input on a line
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
    
    //Clean the array first so we don't memory leak
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

bool check_if_run_in_background(char** arguments_array, int* args_count){
    if(strcmp(arguments_array[*args_count-1], "&") == 0){
        *args_count -= 1;
        return true;
    }else{
        return false;
    }
}

bool check_if_output_redirect(char** arguments_array, int* args_count, \
                              char* output_filename){
    memset(output_filename, '\0', INPUT_LENGTH_MAX);
    
    //Check for output redirect. If found, copy the filename and lower
    //argument count so we don't process it later
    for(int i = 0 ; i < *args_count ; i++){
        if(strcmp(arguments_array[i], ">") == 0){
            strcpy(output_filename, arguments_array[i + 1]);
            *args_count -= 2;
            return true;
        }
    }
    return false;
}

bool check_if_input_redirect(char** arguments_array, int* args_count, \
                             char* input_filename){
    memset(input_filename, '\0', INPUT_LENGTH_MAX);
    
    //Check for input redirect. If found, copy the filename and lower
    //argument count so we don't process it later
    for(int i = 0 ; i < *args_count ; i++){
        if(strcmp(arguments_array[i], "<") == 0){
            strcpy(input_filename, arguments_array[i + 1]);
            *args_count -= 2;
            return true;
        }
    }
    return false;
}

void clean_extra_args(char** arguments_array, int args_count){
    //This clean up the rest on the input line, after our redirects if they 
    //happened
    for(int i = args_count ; i < INPUT_ARGS_MAX ; i++){
        if(arguments_array[i] != NULL){
            free(arguments_array[i]);
            arguments_array[i] = NULL;
        }else{
            break;
        }
    }
}