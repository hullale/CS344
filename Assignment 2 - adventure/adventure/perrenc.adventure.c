/* ///////////////////////////////////////// */
/* ///////// Includes and Defines ////////// */
/* ///////////////////////////////////////// */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <time.h>

/* ///////////////////////////////////// */
/* ///////// Global Variables ////////// */
/* ///////////////////////////////////// */
/* Room variables */
char* room_filenames[] = {
    "room_1.txt",
    "room_2.txt",
    "room_3.txt",
    "room_4.txt",
    "room_5.txt",
    "room_6.txt",
    "room_7.txt"
};

char* rooms_directory_base = "perrenc.rooms.";
char rooms_directory_full[255];

char *potential_room_names[10] = {
    "Dearborn",
    "Graf",
    "Rogers",
    "Covell",
    "Batcheller",
    "Kidder",
    "Valley Library",
    "Memorial Union",
    "Gilbert",
    "Weniger"
};

unsigned int min_connections = 3;
unsigned int max_connections = 6;

char* current_room_filename;

char path_taken[255][15];
unsigned int num_steps = 0;

/* File access variables */
FILE *file_pointer_main;
FILE *file_pointer_time;

/* Threading variables */
pthread_t time_thread_ID;
bool time_written = false;
char* time_file_name = "currentTime.txt";
pthread_mutex_t time_mutex;

/* State Handling Variables */
bool game_over = false;
bool delete_folder_and_files = true;

/* State Variables */
bool valid_input = false;
bool first_print = true;

/* //////////////////////////////////////// */
/* ///////// Function Prototypes ////////// */
/* //////////////////////////////////////// */
/* Initialization functions */
void program_init(void);
void generate_rooms(void);

/* User Input and Printing Functions */
void print_current_room_and_prompt(void);
void get_user_input_and_process(void);
void print_game_over(void);

/* Room Specific Functions*/
void create_room(char *room_filename, char *room_name, char* room_type);
void add_connection_to_room(char *room_filename, char *room_name_to_add);
unsigned int get_number_of_connections_from_room(char *room_filename);
bool is_room_used(char* room_name);
bool is_end_room(char* room_filename);
void get_room_name(char* room_filename, char* room_name_buffer);
int get_room_index_from_name(char* room_name);
int get_used_indexes(char* room_filename, int* output_list);

/* Threading Functions */
void print_time(void);

/* Threading Functions */
void* get_time(void *arguments);

/* Helper Functions */
FILE *open_file_local_folder(char *file_name, char *mode);
int delete_file_local_folder(char *file_name);

/* ///////////////////////// */
/* ///////// Main ////////// */
/* ///////////////////////// */
int main(int argc, char** argv) {
    program_init();
    generate_rooms();
    
    while(!game_over){
        print_current_room_and_prompt();
        while(!valid_input){
          get_user_input_and_process();  
        }
        valid_input = false;
    }
    print_game_over();
    return (EXIT_SUCCESS);
}

/* ///////////////////////////////////////////// */
/* ///////// Initialization Functions ////////// */
/* ///////////////////////////////////////////// */
void program_init(void){
    char pid_buffer[20];
    memset(pid_buffer, '\0', 20);

    /* Initialize our mutex */
    assert(pthread_mutex_init(&time_mutex, NULL) == 0);
    
    /* Make the rooms directory string we'll be using */
    memset(rooms_directory_full, '\0', 255);
    sprintf(pid_buffer, "%d", getpid());
    strcat(rooms_directory_full, rooms_directory_base);
    strcat(rooms_directory_full, pid_buffer);
    
    /* Create the directory now that we have the string */
    mkdir(rooms_directory_full, 0770);
    
    /* Select the start room as the start point */
    current_room_filename = room_filenames[0];
}

void generate_rooms(void){
    time_t t;
    bool start_used = false;
    bool end_used = false;
    
    char num_rooms = sizeof(room_filenames) / sizeof(room_filenames[0]);
    
    srand((unsigned) time(&t));
    
    /* Generate base rooms with room name and room type */
    int i;
    for(i = 0 ; i < num_rooms ; i++){
        /* Get a random room name */
        int random_num_name = rand() % 10;
        char* current_room_name = potential_room_names[random_num_name];
        
        /* If that name is already used, generate a unique one */
        while(is_room_used(current_room_name)){
           random_num_name = rand() % 10;
           current_room_name = potential_room_names[random_num_name];
        }
        
        /* Make the start room if it hasn't been made yet */
        if(!start_used){
            create_room(room_filenames[i], current_room_name, "START_ROOM");
            start_used = true;
        
        /* Make the end room if it hasn't been made yet */
        }else if(!end_used){
            create_room(room_filenames[i], current_room_name, "END_ROOM");
            end_used = true;
        /* Make the middle rooms, if the start and ends rooms are made */
        }else{
            create_room(room_filenames[i], current_room_name, "MID_ROOM");
        }
    }
    
    /* Generate the room connections for each room */
    for(i = 0 ; i < num_rooms ; i++){
        char buffer[255];
        int used_rooms[6];
        
        /* Initialize used rooms array with invalid numbers */
        int l;
        for(l = 0 ; l < 6 ; l++){
            used_rooms[l] = -1;
        }
        
        /* Get random number between min and max connections */
        int num_connections = (rand() % 4) + min_connections - \
            get_number_of_connections_from_room(room_filenames[i]);
        
        /* Generate the connections for an individual room */
        int j;
        for(j = 0 ; j < num_connections ; j++){
            int room_to_add;
            bool unused_found = false;
            
            /* Generate connections, making sure they're not itself or other 
               rooms that have already been used */
            while(!unused_found){
                room_to_add = rand() % 7;
                
                /* Skip if room is itself */
                if(room_to_add == i){
                    continue;
                }
                
                /* Make sure that we're not adding a room that's already a 
                   connection */
                bool already_exists = false;
                get_used_indexes(room_filenames[i], used_rooms);                
                
                int k;
                for(k = 0 ; k < 6 ; k++){
                    if(used_rooms[k] == room_to_add){
                        already_exists = true;
                        break;
                    }
                }
                
                /* Make sure the connecting room can handle the reverse 
                   connection */
                if(get_number_of_connections_from_room(\
                        room_filenames[room_to_add]) == max_connections){
                    already_exists = true;
                }
                
                /* If all is good, set flag to exit loop */
                if(!already_exists){
                    unused_found = true;   
                }                
            }
            
            /* Once a connection has been deemed valid, add forward and 
               backwards connections */
            memset(buffer, '\0', 255);
            get_room_name(room_filenames[room_to_add], buffer);
            add_connection_to_room(room_filenames[i], buffer);
            
            memset(buffer, '\0', 255);
            get_room_name(room_filenames[i], buffer);
            add_connection_to_room(room_filenames[room_to_add], buffer);
        }
    }
}

/* //////////////////////////////////////////////// */
/* ///////// User Input and Printing Functions //// */
/* //////////////////////////////////////////////// */
void print_current_room_and_prompt(void){
    char current_line[255];
    char print_buffer[255];
    char* tokenized;
    memset(current_line, '\0', 255);
    memset(print_buffer, '\0', 255);
    
    file_pointer_main = open_file_local_folder(current_room_filename, "r");
    
    /* Print location header */
    if(first_print){
        printf("CURRENT LOCATION: ");
        first_print = false;
    }else{
       printf("\nCURRENT LOCATION: "); 
    }
    
    /* Strip out and print current room name */
    fgets(current_line, 255, file_pointer_main);
    tokenized = strtok(current_line, ":");
    tokenized = strtok(NULL, " \n");
    
    bool first_entry = true;
    while(tokenized != NULL){
            if(first_entry){
                first_entry = false;
            }else{
              printf(" ");  
            }
           printf("%s", tokenized);
           tokenized = strtok(NULL, " \n");
    }
    
    /* Print possible connections header */
    printf("\nPOSSIBLE CONNECTIONS: ");
    
    /* Loop through connections and print names with separators */
    bool printing_connections = true;
    bool printed_first_connection = false;
    while(printing_connections){
        fgets(current_line, 255, file_pointer_main);
        tokenized = strtok(current_line, " ");
        
        if(strcmp(tokenized, "CONNECTION") != 0){
            printing_connections = false;
            break;
        }
        
        if(printed_first_connection){
            printf(", ");
        }else{
            printed_first_connection = true;
        }
        
        tokenized = strtok(NULL, ":");
        tokenized = strtok(NULL, " \n");
        
        bool first_entry = true;
        while(tokenized != NULL){
                if(first_entry){
                    first_entry = false;
                }else{
                  printf(" ");  
                }

               printf("%s", tokenized);
               tokenized = strtok(NULL, " \n");
        }
    }
    printf(".");
    fclose(file_pointer_main);
}

void get_user_input_and_process(void){
    char buffer[255];
    memset(buffer, '\0', 255);
    
    /* Print where to header */
    printf("\nWHERE TO? >");
    
    /* Read in newline terminated string from user, and strip out the newline*/
    fgets(buffer, 255, stdin);
    buffer[strlen(buffer)-1] = '\0';
    
    /* Check input to determine if valid and what it should do*/
    int room_index = get_room_index_from_name(buffer);
    if(room_index != -1){
        /* Room change is valid, change room and log */
        strcpy(path_taken[num_steps], buffer);
        num_steps++;
        current_room_filename = room_filenames[room_index];
        valid_input = true;
    }else if(strcmp(buffer, "time") == 0){
        /* Request was to print time, do so */
        print_time();
    }else{
        /* Bad input was entered, print input error message */
        printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
        valid_input = true;
    }    
    
    /* Set game over flag to the result of whether we're in the end room */
    game_over = is_end_room(current_room_filename);
}

void print_game_over(void){
    /* Print game over header */
    printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!");
    printf("\nYOU TOOK %u STEPS. YOUR PATH TO VICTORY WAS:", num_steps);
    
    /* Print path taken to game over*/
    int i;
    for(i = 0 ; i < num_steps ; i++){
        printf("\n%s", path_taken[i]);
    }
    printf("\n");
}

/* ////////////////////////////////////// */
/* ///////// Room Specific Functions //// */
/* ////////////////////////////////////// */
void create_room(char *room_filename, char *room_name, char* room_type){
    file_pointer_main = open_file_local_folder(room_filename, "w");
    assert(file_pointer_main != NULL);
    
    /* Print line for the room name */
    fputs("ROOM NAME: ", file_pointer_main);
    fputs(room_name, file_pointer_main);
    fputs("\n", file_pointer_main);

    /* Print line for the room type */
    fputs("ROOM TYPE: ", file_pointer_main);
    fputs(room_type, file_pointer_main);
    fclose(file_pointer_main);
    
}

void add_connection_to_room(char *room_filename, char *room_name_to_add){
    char output_string_buffer[255];
    char room_type_save_buffer[255];
    unsigned int newline_count = 0;
    unsigned int line_to_insert_at = 0;
    long insertion_position = 0;
    
    memset(output_string_buffer, '\0', 255);
    memset(room_type_save_buffer, '\0', 255);
    
    /* Determine where the new connection line will sit */
    line_to_insert_at = get_number_of_connections_from_room(room_filename);
    
    /* Add one to skip room name line */
    line_to_insert_at++; 
    
    file_pointer_main = open_file_local_folder(room_filename, "r+");
    
    /* Count out way to where we insert the new connection */
    while(newline_count != line_to_insert_at){
        if(fgetc(file_pointer_main) == '\n'){
            newline_count++;
        }
    }
    
    /* Save insertion position and make backup of line that will be wiped out */
    insertion_position = ftell(file_pointer_main);
    fgets(room_type_save_buffer, 255, file_pointer_main);
    fseek(file_pointer_main, insertion_position, SEEK_SET);
    
    /* Save new connection line and add the saved one after it */
    sprintf(output_string_buffer, "CONNECTION %u: %s\n", line_to_insert_at, \
            room_name_to_add);
    fputs(output_string_buffer, file_pointer_main);
    fputs(room_type_save_buffer, file_pointer_main);
    fclose(file_pointer_main);
    
}

unsigned int get_number_of_connections_from_room(char *room_filename){
    char current_line[255];
    char* tokenized;
    unsigned int number_of_rooms = 0;
    
    memset(current_line, '\0', 255);
    
    file_pointer_main = open_file_local_folder(room_filename, "r");
    
    /* Loop through the lines and count the number of connections we have */
    while(fgets(current_line, 255, file_pointer_main) != NULL){
        tokenized = strtok(current_line, " ");
        if(strcmp("CONNECTION", tokenized) == 0){
            number_of_rooms++;
        }
    }
    fclose(file_pointer_main);
    return number_of_rooms;
}

bool is_room_used(char* room_name){
    char num_rooms = sizeof(room_filenames) / sizeof(room_filenames[0]);
    char room_name_buffer[255];
    
    /* Loop through all files and strcmp to see if it's been used already */
    int i;
    for(i = 0 ; i < num_rooms ; i++){
        memset(room_name_buffer, '\0', 255);
        get_room_name(room_filenames[i], room_name_buffer);
        /* printf("Comparing %s to %s\n", room_name, room_name_buffer); */
        if(strcmp(room_name_buffer, room_name) == 0){
            return true;
        }
    }
    return false;
}

bool is_end_room(char* room_filename){
    char current_line[255];
    char* tokenized;
    
    memset(current_line, '\0', 255);
    
    file_pointer_main = open_file_local_folder(room_filename, "r");
    
    /* Run through file until room type, check if room is end room*/
    while(fgets(current_line, 255, file_pointer_main) != NULL){
        tokenized = strtok(current_line, ":");
        if(strcmp("ROOM TYPE", tokenized) == 0){
            tokenized = strtok(NULL, " \n");
            if(strcmp(tokenized, "END_ROOM") == 0){
                fclose(file_pointer_main);  
                return true;
            }
        }
    }
    
    fclose(file_pointer_main);
    return false;
}

void get_room_name(char* room_filename, char* room_name_buffer){
    char current_line[255];
    char* tokenized;
    
    memset(current_line, '\0', 255);
    
    file_pointer_main = open_file_local_folder(room_filename, "r");
    
    /* Return immediately if file does not yet exist */
    if(file_pointer_main == NULL){
        strcpy(room_name_buffer, "___NOT_A_ROOM_NAME____");
        return;
    }
    
    /* Run through file and get room name with special handling for spaces */
    while(fgets(current_line, 255, file_pointer_main) != NULL){
        /*strcpy(current_line_backup, current_line);*/
        tokenized = strtok(current_line, ":");
        
        bool first_entry = true;
        
        if(strcmp("ROOM NAME", tokenized) == 0){
            tokenized = strtok(NULL, " \n");
            while(tokenized != NULL){
                if(first_entry){
                    first_entry = false;
                }else{
                  strcat(room_name_buffer, " ");  
                }
                
               strcat(room_name_buffer, tokenized);
               tokenized = strtok(NULL, " \n");
            }
            break;
        }
    }
    
    fclose(file_pointer_main);
}

int get_room_index_from_name(char* room_name){
    char num_rooms = sizeof(room_filenames) / sizeof(room_filenames[0]);
    char room_name_buffer[255];
    
    /* Loop through files until the room name is found, return index*/
    int i;
    for(i = 0 ; i < num_rooms ; i++){
        memset(room_name_buffer, '\0', 255);
        get_room_name(room_filenames[i], room_name_buffer);
        if(strcmp(room_name_buffer, room_name) == 0){
           return i; 
        }
    }
    return -1;
}

int get_used_indexes(char* room_filename, int* output_list){
    char current_line[255];
    char used_names[6][15];
    char* tokenized;
    int return_index = 0;
    
    memset(current_line, '\0', 255);
    
    file_pointer_main = open_file_local_folder(room_filename, "r");
    
    /* Loop through and get names of all connections in file */
    while(fgets(current_line, 255, file_pointer_main) != NULL){
        tokenized = strtok(current_line, " ");
        if(strcmp("CONNECTION", tokenized) == 0){
            tokenized = strtok(NULL, ":");
            tokenized = strtok(NULL, " \n");
            
            bool first_entry = true;
            while(tokenized != NULL){
                    if(first_entry){
                        first_entry = false;
                    }else{
                     strcat(used_names[return_index], " "); 
                    }

                   strcat(used_names[return_index], tokenized);
                   tokenized = strtok(NULL, " \n");
            }
            return_index++;
        }
    }
    fclose(file_pointer_main);
    
    /* Get indices from names, and return */
    int i;
    for(i = 0 ; i < return_index ; i++){
        output_list[i] = get_room_index_from_name(used_names[i]);
    }
    
    return return_index;
}

/* /////////////////////////////////// */
/* ///////// Time Functions ////////// */
/* /////////////////////////////////// */
void print_time(void){    
    /* Make and run the thread. Join to block until the other thread is done. */
    assert(pthread_create(&time_thread_ID, NULL, get_time, NULL) == 0);
    assert(pthread_join(time_thread_ID, NULL) == 0);

    /* Lock and get a copy of whether time was written. Set original to false. */
    pthread_mutex_lock(&time_mutex);
    bool time_was_written = time_written;
    time_written = false;
    pthread_mutex_unlock(&time_mutex);
    
    /* Make sure that the time was written, otherwise massive error... */
    assert(time_was_written);
    
    /* Open file with time information and print it. */
    file_pointer_main = open_file_local_folder(time_file_name, "r");
    assert(file_pointer_main != NULL);
    
    printf("\n");
    char c = fgetc(file_pointer_main);
    while(c != EOF){
        printf("%c", c);
        c = fgetc(file_pointer_main);
    }
    printf("\n");
    fclose(file_pointer_main);
    
    /* Delete this temporary file */
    delete_file_local_folder(time_file_name);   
    
}

/* //////////////////////////////////////// */
/* ///////// Threading Functions ////////// */
/* //////////////////////////////////////// */
void* get_time(void *arguments){
    time_t current_time_raw;
    struct tm *time_as_struct;
    char buffer_size = 50;
    char buffer[buffer_size];
    memset(buffer, '\0', buffer_size);
    
    /* Get current time */
    time(&current_time_raw);
    time_as_struct = localtime(&current_time_raw);
    
    /* Format per specification */
    strftime(buffer, buffer_size, "%-l:%M%P, %A, %B %-e, %Y", time_as_struct);
    
    /* Write formatted time to file */
    file_pointer_time = open_file_local_folder(time_file_name, "w");
    fprintf(file_pointer_time, "%s", buffer);
    fclose(file_pointer_time);
    
    /* Lock, update, and unlock mutex variable */
    pthread_mutex_lock(&time_mutex);
    time_written = true;
    pthread_mutex_unlock(&time_mutex);
    return NULL;
}

/* ///////////////////////////////////// */
/* ///////// Helper Functions ////////// */
/* ///////////////////////////////////// */
FILE *open_file_local_folder(char *file_name, char *mode){
    /* Makes common concatenation of file with full path easier */
    char full_path[255];
    memset(full_path, '\0', 255);
    strcat(full_path, rooms_directory_full);
    strcat(full_path, "/");
    strcat(full_path, file_name);
    return fopen(full_path, mode);
}

int delete_file_local_folder(char *file_name){
    /* Easy delete that handles concatenation for time file */
    char full_path[255];
    memset(full_path, '\0', 255);
    strcat(full_path, rooms_directory_full);
    strcat(full_path, "/");
    strcat(full_path, file_name);
    return remove(full_path);
}