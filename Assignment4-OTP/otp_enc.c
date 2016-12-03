/* 
 * File:   otp_enc.c
 * Author: Corwin Perren
 *
 * Created on November 29, 2016, 7:26 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>

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

/////////////////////////////////////////
////////// Function Prototypes //////////
/////////////////////////////////////////
int main(int argc, char** argv) {
    int comms_sfd;
    unsigned long int comms_port_number;    //0 - 65535
    struct hostent *server;
    
    struct sockaddr_in server_address;
    
    FILE *input_file;
    FILE *key_file;
    
    //Verify correct number of arguments
    if(argc < 4){
        fprintf(stderr, "Not enough arguments provided. Exiting...\n");
        fprintf(stderr, "Correct usage: "
                        "%s input_file key_file port\n", argv[0]);
        exit(PROGRAM_FAILURE);
    }else if(argc > 4){
        fprintf(stderr, "Extra arguments entered. Ignoring all but the first "
                        "three.\n");
    }
    
    //Get port number, and make sure it's valid
    comms_port_number = atoi(argv[3]);
    if((comms_port_number > PORT_MAX) || (comms_port_number < PORT_MIN)){
        fprintf(stderr, "Port out of range! Please choose a different port! "
                        "Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    //Open the files
    input_file = fopen(argv[1], "r");
    key_file = fopen(argv[2], "r");
    
    //Make sure they opened successfully
    if(input_file == NULL){
        fprintf(stderr, "Input file does not exist or cannot be read! "
                        "Exiting...\n");
        exit(PROGRAM_FAILURE);
    }else if(key_file == NULL){
        fprintf(stderr, "Key file does not exist or cannot be read! "
                        "Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    //Read in input file
    char* temp_buffer = malloc(sizeof(char) * 1000000);
    memset(temp_buffer, '\0', sizeof(char) * 1000000);
    fgets(temp_buffer, 1000000, input_file);
    
    char* input_text = malloc(sizeof(char) * (strlen(temp_buffer) + 1));
    memset(input_text, '\0', sizeof(char) * (strlen(temp_buffer) + 1));
    strcpy(input_text, temp_buffer);
    
    //Remove input file newline
    int input_str_len = strlen(input_text);    
    if(input_text[input_str_len - 1] == '\n'){
       input_text[input_str_len - 1] = '\0';
    }
    
    //Read in key file
    memset(temp_buffer, '\0', sizeof(char) * 1000000);
    fgets(temp_buffer, 1000000, key_file);
    
    char* key_text = malloc(sizeof(char) * (strlen(temp_buffer) + 1));
    memset(key_text, '\0', sizeof(char) * (strlen(temp_buffer) + 1));
    strcpy(key_text, temp_buffer);
    
    //Remove input file newline
    input_str_len = strlen(key_text);    
    if(key_text[input_str_len - 1] == '\n'){
       key_text[input_str_len - 1] = '\0';
    }
    
//    for(unsigned long int i = 0 ; i < strlen(input_text) -1 ;\
//        i++){
//        char new_char = encode_character(input_text[i], \
//                                         key_text[i]);
//        
//        char decoded = decode_character(new_char, \
//                                         key_text[i]);
//
//        printf("%c : %c : %c : %c\n", input_text[i], key_text[i], new_char, decoded);
//    }
//    printf("\n");
    free(temp_buffer);
    
    //Exit as error is input_string is longer than the key
    if(strlen(input_text) > strlen(key_text)){
        fprintf(stderr, "Key file too small. Please try again with larger "
                        "file. Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    //Create listening socket, check if created successfully
    comms_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(comms_sfd < 0){
        fprintf(stderr, "Could not create comms socket! Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    //Get localhost as server, check if valid
    server = gethostbyname("localhost");
    if(server == NULL){
        fprintf(stderr, "localhost is not a valid host! Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    
    //Set up comms parameters for the server we're connecting to.
    memset((char *)&server_address, '\0', sizeof(server_address));
    server_address.sin_port = htons(comms_port_number);
    memcpy(&server_address.sin_addr, server->h_addr_list[0], server->h_length);
    server_address.sin_family = AF_INET;
    
    //Make the server connection and verify it didn't fail.
    int connect_result = connect(comms_sfd, \
            (struct sockaddr *)&server_address, \
             sizeof(server_address));
    if(connect_result < 0){
        fprintf(stderr, "Connection failed! Is the port correct? Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    int read_return;
    int write_return;
    unsigned int concat_index = 0;
                
    char read_buffer[10];
    char* concat_buffer = malloc(sizeof(char) * 1000000);
    memset(read_buffer, '\0', 1);
    memset(concat_buffer, '\0', sizeof(char) * 1000000);
    
    write_return = write(comms_sfd, OTP_ENC_IDENT TEXT_DONE, 3);
    if(write_return < 0){
        fprintf(stderr, "Failed to write data. Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
    while(strstr(concat_buffer, TEXT_DONE) == NULL){
        read_return = read(comms_sfd, read_buffer, 1);
        if(read_return != -1){
            concat_buffer[concat_index] = read_buffer[0];
            concat_index++;
        }
    }

    if(strstr(concat_buffer, OTP_CONTINUE TEXT_DONE) != NULL){
        write_return = write(comms_sfd, input_text, strlen(input_text));
        if(write_return < 0){
            fprintf(stderr, "Failed to write data. Exiting...\n");
            exit(PROGRAM_FAILURE);
        }
        
        write_return = write(comms_sfd, TEXT_DONE, 2);
        if(write_return < 0){
            fprintf(stderr, "Failed to write data. Exiting...\n");
            exit(PROGRAM_FAILURE);
        }
        
        write_return = write(comms_sfd, key_text, strlen(key_text));
        if(write_return < 0){
            fprintf(stderr, "Failed to write data. Exiting...\n");
            exit(PROGRAM_FAILURE);
        }
        
        write_return = write(comms_sfd, TEXT_DONE, 2);
        if(write_return < 0){
            fprintf(stderr, "Failed to write data. Exiting...\n");
            exit(PROGRAM_FAILURE);
        }
        
        //Reset variables to receive data
        concat_index = 0;
        memset(read_buffer, '\0', 10);
        memset(concat_buffer, '\0', sizeof(char) * 1000000);
        
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
        
        
        for(int i = 0 ; i < strlen(concat_buffer) ; i++){
            printf("%c", concat_buffer[i]);
        }
        printf("\n");
        
        
    }else if(strstr(concat_buffer, OTP_FAILURE TEXT_DONE) != NULL){
        fprintf(stderr, "Incorrect client/server pair. Exiting...\n");
        exit(PROGRAM_FAILURE);
    }else{
        fprintf(stderr, "Got bad data. Exiting...\n");
        exit(PROGRAM_FAILURE);
    }
    
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