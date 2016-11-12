/* MY HEADER HERE */
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
    
    int i;
    for(i = 0 ; i < 1 ; i++){
        strcpy(path_taken[i], "______________");
    }
    
    /* Select the start room as the start point */
    current_room_filename = room_filenames[0];
}

void generate_rooms(void){
    time_t t;
    bool start_used = false;
    bool end_used = false;
    
    char num_rooms = sizeof(room_filenames) / sizeof(room_filenames[0]);
    
    srand((unsigned) time(&t));
    
    int i;
    
    for(i = 0 ; i < num_rooms ; i++){
        int random_num_name = rand() % 10;
        char* current_room_name = potential_room_names[random_num_name];
        
        while(is_room_used(current_room_name)){
           random_num_name = rand() % 10;
           current_room_name = potential_room_names[random_num_name];
        }
        
        if(!start_used){
            create_room(room_filenames[i], current_room_name, "START_ROOM");
            start_used = true;
            
        }else if(!end_used){
            create_room(room_filenames[i], current_room_name, "END_ROOM");
            end_used = true;
            
        }else{
            create_room(room_filenames[i], current_room_name, "MID_ROOM");
        }
    }
    
    for(i = 0 ; i < num_rooms ; i++){
        char buffer[255];
        int used_rooms[6];
        
        /* Initialize used rooms array with invalid numbers */
        int l;
        for(l = 0 ; l < 6 ; l++){
            used_rooms[l] = -1;
        }
        
        /* Get random number between min and max connections */
        int num_connections = (rand() % 4) + min_connections;
        
        assert(num_connections >= min_connections);
        assert(num_connections <= max_connections);
        
        int j;
        for(j = 0 ; j < num_connections ; j++){
            
           
            int room_to_add;
            
            bool unused_found = false;
            
            while(!unused_found){
                room_to_add = rand() % 7;
                if(room_to_add == i){
                    continue;
                }
                
                bool already_exists = false;
                int k;
                for(k = 0 ; k < 6 ; k++){
                    if(used_rooms[k] == room_to_add){
                        already_exists = true;
                        break;
                    }
                }
                
                if(!already_exists){
                    unused_found = true;   
                }                
            }
            
            memset(buffer, '\0', 255);
            used_rooms[j] = room_to_add;
            get_room_name(room_filenames[room_to_add], buffer);
            add_connection_to_room(room_filenames[i], buffer);
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
    
    /* Room Name Printing */
    fgets(current_line, 255, file_pointer_main);
    
    if(first_print){
        printf("CURRENT LOCATION: ");
        first_print = false;
    }else{
       printf("\nCURRENT LOCATION: "); 
    }
    

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
    
    /* Connections Printing */
    printf("\nPOSSIBLE CONNECTIONS: ");
    
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
    
    printf("\nWHERE TO? >");
    fgets(buffer, 255, stdin);
    buffer[strlen(buffer)-1] = '\0';
    
    int room_index = get_room_index_from_name(buffer);
    
    if(room_index != -1){
        strcpy(path_taken[num_steps], buffer);
        num_steps++;
        current_room_filename = room_filenames[room_index];
        valid_input = true;
    }else if(strcmp(buffer, "time") == 0){
        print_time();
    }else{
        printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
        valid_input = true;
    }    
    
    game_over = is_end_room(current_room_filename);
}

void print_game_over(void){
    printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!");
    printf("\nYOU TOOK %u STEPS. YOUR PATH TO VICTORY WAS:", num_steps);
    
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
    
    fputs("ROOM NAME: ", file_pointer_main);
    fputs(room_name, file_pointer_main);
    fputs("\n", file_pointer_main);

    
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
    
    line_to_insert_at = get_number_of_connections_from_room(room_filename);
    
    /* Add on to skip room name line */
    line_to_insert_at++; 
    
    file_pointer_main = open_file_local_folder(room_filename, "r+");
    
    while(newline_count != line_to_insert_at){
        if(fgetc(file_pointer_main) == '\n'){
            newline_count++;
        }
    }
    
    insertion_position = ftell(file_pointer_main);
    fgets(room_type_save_buffer, 255, file_pointer_main);
    fseek(file_pointer_main, insertion_position, SEEK_SET);
    
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
    /*char current_line_backup[255];*/
    char* tokenized;
    
    memset(current_line, '\0', 255);
    /*memset(current_line_backup, '\0', 255);*/
    
    file_pointer_main = open_file_local_folder(room_filename, "r");
    
    if(file_pointer_main == NULL){
        strcpy(room_name_buffer, "___NOT_A_ROOM_NAME____");
        return;
    }
    
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
    
    time(&current_time_raw);
    time_as_struct = localtime(&current_time_raw);
    
    strftime(buffer, buffer_size, "%-l:%M%P, %A, %B %-e, %Y", time_as_struct);
    
    file_pointer_time = open_file_local_folder(time_file_name, "w");
    fprintf(file_pointer_time, "%s", buffer);
    fclose(file_pointer_time);
    
    pthread_mutex_lock(&time_mutex);
    time_written = true;
    pthread_mutex_unlock(&time_mutex);
    return NULL;
}

/* ///////////////////////////////////// */
/* ///////// Helper Functions ////////// */
/* ///////////////////////////////////// */
FILE *open_file_local_folder(char *file_name, char *mode){
    char full_path[255];
    memset(full_path, '\0', 255);
    strcat(full_path, rooms_directory_full);
    strcat(full_path, "/");
    strcat(full_path, file_name);
    return fopen(full_path, mode);
}

int delete_file_local_folder(char *file_name){
    char full_path[255];
    memset(full_path, '\0', 255);
    strcat(full_path, rooms_directory_full);
    strcat(full_path, "/");
    strcat(full_path, file_name);
    return remove(full_path);
}
