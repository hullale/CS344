/* MY HEADER HERE */
/* ///////////////////////////// */
/* ///////// Includes ////////// */
/* ///////////////////////////// */
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
char* room_1_filename = "room_1.txt";
char* room_2_filename = "room_2.txt";
char* room_3_filename = "room_3.txt";
char* room_4_filename = "room_4.txt";
char* room_5_filename = "room_5.txt";
char* room_6_filename = "room_6.txt";
char* room_7_filename = "room_7.txt";

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
    "Weniger",
};
char used_room_names[10][15];

/* File access variables */
FILE *file_pointer_main;
FILE *file_pointer_time;

/* Threading variables */
pthread_t time_thread_ID;
bool time_written = false;
char* time_file_name = "currentTime.txt";

pthread_mutex_t time_mutex;

/* //////////////////////////////////////// */
/* ///////// Function Prototypes ////////// */
/* //////////////////////////////////////// */
/* Initialization functions */
void program_init(void);

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
    print_time();
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