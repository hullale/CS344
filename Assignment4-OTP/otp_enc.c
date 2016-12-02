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
    
    //Create our listening socket, check if created successfully
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
//    char *sentence = "MY OWN TEST";  
//    
//    for(int i = 0 ; i < strlen(sentence) ; i++){
//        char encoded_char = encode_character(sentence[i], 'A' + i);
//        char decoded_char = decode_character(encoded_char, 'A' + i);
//        
//        printf("%c : %c : %c\n", sentence[i], encoded_char, decoded_char);
//    }
//    
//    exit (EXIT_SUCCESS);
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
    return 'e';
}

char encode_character(char input_letter, char key_letter){
    int letter_mapped = get_mapped_num_from_char(input_letter);
    int key_mapped = get_mapped_num_from_char(key_letter);
    
    int summed = (letter_mapped + key_mapped);
    
    if(summed > LETTER_OPTIONS){
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