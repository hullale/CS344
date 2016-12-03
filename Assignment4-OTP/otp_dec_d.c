/* 
 * File:   otp_enc_d.c
 * Author: Corwin Perren
 *
 * Created on November 29, 2016, 7:26 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define PROGRAM_SUCCESS 0
#define PROGRAM_FAILURE 1

#define PORT_MAX 65535
#define PORT_MIN 0

#define OTP_ENC_IDENT "#"
#define OTP_DEC_IDENT "$"
#define TEXT_DONE "@@"
#define OTP_FAILURE "%"
#define OTP_CONTINUE "&"

#define LETTER_OPTIONS 27
int letter_number_assignment[LETTER_OPTIONS][2] = {
    'A', 10,
    'B', 11,
    'C', 12,
    'D', 13,
    'E', 14,
    'F', 15,
    'G', 16,
    'H', 17,
    'I', 18,
    'J', 19,
    'K', 20,
    'L', 21,
    'M', 22,
    'N', 23,
    'O', 24,
    'P', 25,
    'Q', 26,
    'R', 0,
    'S', 1,
    'T', 2,
    'U', 3,
    'V', 4,
    'W', 5,
    'X', 6,
    'Y', 7,
    'Z', 8,
    ' ', 9
};

/////////////////////////////////////////
////////// Function Prototypes //////////
/////////////////////////////////////////
int get_mapped_num_from_char(char letter);
char get_mapped_char_from_num(int number);
char encode_character(char letter, char key_letter);
char decode_character(char input_letter, char key_letter);

int main(int argc, char** argv) {
    int listen_sfd; //sfd == socked_file_descriptor
    int comms_sfd;
    unsigned long int listen_port_number;    //0 - 65535
    
    socklen_t client_length;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    
    //Check if we have enough arguments, error and exit if not.
    if(argc < 2){
        fprintf(stderr, "No port provided! Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    //Create our listening socket, check if created successfully
    listen_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_sfd < 0){
        fprintf(stderr, "Could not create listening socket! Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    //Get port number, and make sure it's valid
    listen_port_number = atoi(argv[1]);
    if((listen_port_number > PORT_MAX) || (listen_port_number < PORT_MIN)){
        fprintf(stderr, "Port out of range! Please choose a different port! "
                        "Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    //Set up listening parameters
    memset((char *)&server_address, '\0', sizeof(server_address));
    server_address.sin_port = htons(listen_port_number);
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_family = AF_INET;

    //Bind server to settings set above, let system know we're ready
    int bind_result = bind(listen_sfd, (struct sockaddr *)&server_address, \
            sizeof(server_address));
    if(bind_result < 0){
        fprintf(stderr, "Failed to bind listening port! "
                        "Please choose a different port! Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    //fprintf(stdout, ":: otp_enc_d :: Bound on port %u.\n", listen_port_number);
    
    
    while(1){
        //Call waitpid to kill off any zombie processes. Don't let it block.
        static int process_status;
        waitpid(-1, &process_status, WNOHANG);

        //Listen to the port, and allow up to five connections
        listen(listen_sfd, 5); //Listen to the port

        //Block until a valid connection is made, then accept it. 
        //Also make sure it's accepted correctly
        socklen_t client_length = sizeof(client_address);
        comms_sfd = accept(listen_sfd, (struct sockaddr *) &client_address, \
                           &client_length);
        if (comms_sfd < 0){
            fprintf(stderr, "Client accept failed. Trying next "
                            "connection...\n");
        }else{
            //We've made a valid connection, so spawn off a new process to
            //handle it to free up the main thread for accepting a new client.
            pid_t spawned_pid;
            spawned_pid = fork();
            
            //Only run the child code if it's a child process
            if(spawned_pid == 0){
                
                int read_return;
                int write_return;
                unsigned int concat_index = 0;
                
                char read_buffer[10];
                char* concat_buffer = malloc(sizeof(char) * 1000000);
                memset(read_buffer, '\0', 10);
                memset(concat_buffer, '\0', sizeof(char) * 1000000);
                
                while(strstr(concat_buffer, TEXT_DONE) == NULL){
                    read_return = read(comms_sfd, read_buffer, 1);
                    if(read_return != -1){
                        concat_buffer[concat_index] = read_buffer[0];
                        concat_index++;
                    }
                }
                
                //Choose a response appropriate for who is communicating
                if(strstr(concat_buffer, OTP_DEC_IDENT TEXT_DONE) != NULL){
                    write_return = write(comms_sfd, OTP_CONTINUE TEXT_DONE, 3);
                    if(write_return < 0){
                        fprintf(stderr, "Failed to alert sender. Exiting...\n");
                        exit(PROGRAM_FAILURE);
                    }
                    
                    //Reset variables to receive data
                    concat_index = 0;
                    memset(read_buffer, '\0', 10);
                    memset(concat_buffer, '\0', sizeof(char) * 1000000);
                
                    //Read in data until marker to stop is found
                    while(strstr(concat_buffer, TEXT_DONE) == NULL){
                        read_return = read(comms_sfd, read_buffer, 1);
                        if(read_return != -1){
                            concat_buffer[concat_index] = read_buffer[0];
                            concat_index++;
                        }
                    }
                    
                    //Null out our marker characters
                    int input_string_length = strlen(concat_buffer);
                    concat_buffer[input_string_length-1] = '\0';
                    concat_buffer[input_string_length-2] = '\0';
                    
                    //Store this for later use
                    char* input_text = malloc(sizeof(char) * \
                                             (strlen(concat_buffer) + 1));
                    memset(input_text, '\0', sizeof(char) * \
                                           (strlen(concat_buffer) + 1));
                    strcpy(input_text, concat_buffer);
                    
                    //Reset variables to receive data
                    concat_index = 0;
                    memset(read_buffer, '\0', 10);
                    memset(concat_buffer, '\0', sizeof(char) * 1000000);
                    
                    //Read in data until marker to stop is found
                    while(strstr(concat_buffer, TEXT_DONE) == NULL){
                        read_return = read(comms_sfd, read_buffer, 1);
                        if(read_return != -1){
                            concat_buffer[concat_index] = read_buffer[0];
                            concat_index++;
                        }
                    }
                    
                    //Null out our marker characters
                    input_string_length = strlen(concat_buffer);
                    concat_buffer[input_string_length-1] = '\0';
                    concat_buffer[input_string_length-2] = '\0';
                    
                    //Store this for later use
                    char* key_text = malloc(sizeof(char) * \
                                             (strlen(concat_buffer) + 1));
                    memset(key_text, '\0', sizeof(char) * \
                                           (strlen(concat_buffer) + 1));
                    strcpy(key_text, concat_buffer);
                    free(concat_buffer);
                    
                    for(unsigned long int i = 0 ; i < strlen(input_text) ;\
                        i++){
                        char new_char = decode_character(input_text[i], \
                                                         key_text[i]);
                        
                        //printf("%c : %c : %c\n", input_text[i], key_text[i], new_char);
                        write_return = write(comms_sfd, &new_char, 1);
                    }
                    
                    write_return = write(comms_sfd, TEXT_DONE, 2);
                    if(write_return < 0){
                        fprintf(stderr, "Failed to alert sender. Exiting...\n");
                        exit(PROGRAM_FAILURE);
                    }
                    
                    
                }else if(strstr(concat_buffer, OTP_ENC_IDENT TEXT_DONE) \
                         != NULL){
                    write_return = write(comms_sfd, OTP_FAILURE TEXT_DONE, 3);
                    if(write_return < 0){
                        fprintf(stderr, "Failed to alert sender. Exiting...\n");
                    }
                    exit(PROGRAM_FAILURE);
                }else{
                    fprintf(stderr, "Got bad data. Alerting sender and "
                                    "exiting...\n");
                    write_return = write(comms_sfd, OTP_FAILURE TEXT_DONE, 3);
                    if(write_return < 0){
                        fprintf(stderr, "Failed to alert sender. Exiting...\n");
                    }
                    exit(PROGRAM_FAILURE);
                }
                
                //Since this is a child process, once it's done it should die.
                exit(PROGRAM_SUCCESS);
            }
            
        }
    }

    close(listen_sfd);
    exit(PROGRAM_SUCCESS);
}

int get_mapped_num_from_char(char letter){
    for(int i = 0 ; i < LETTER_OPTIONS ; i++){
        if(letter == letter_number_assignment[i][0]){
            return letter_number_assignment[i][1];
        }
    }
    return -1;
}

char get_mapped_char_from_num(int number){
    for(int i = 0 ; i < LETTER_OPTIONS ; i++){
        if(number == letter_number_assignment[i][1]){
            return letter_number_assignment[i][0];
        }
    }
    return -1;
}

char encode_character(char input_letter, char key_letter){
    int letter_mapped = get_mapped_num_from_char(input_letter);
    int key_mapped = get_mapped_num_from_char(key_letter);
    
    int summed = (letter_mapped + key_mapped);
    
    if(summed >= LETTER_OPTIONS){
        summed -= LETTER_OPTIONS;
    }
    
    return get_mapped_char_from_num(summed);
}

char decode_character(char input_letter, char key_letter){
    int letter_mapped = get_mapped_num_from_char(input_letter);
    int key_mapped = get_mapped_num_from_char(key_letter);
    
    int subbed = (letter_mapped - key_mapped);
    
    if(subbed < 0){
        subbed += LETTER_OPTIONS;
    }
    
    return get_mapped_char_from_num(subbed);
}