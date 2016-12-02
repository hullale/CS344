/* 
 * File:   keygen.c
 * Author: Corwin Perren
 *
 * Created on November 29, 2016, 7:27 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PROGRAM_SUCCESS 0
#define PROGRAM_FAILURE 1

#define ASCII_START 65

int main(int argc, char** argv) {
    //Some error handling for bad inputs
    if(argc < 2){
        fprintf(stderr, "No arguments entered. Please try again.\n");
        exit(PROGRAM_FAILURE);
    }else if (argc > 2){
        fprintf(stderr, "Extra arguments entered. Ignoring all but the "
                        "first.\n");
    }else if(atoi(argv[1]) <= 0){
        fprintf(stderr, "Key length too small. Number must be greater than "
                        "zero.\n");
        exit(PROGRAM_FAILURE);
    }
    
    time_t t;
    srand((unsigned) time(&t));
    
    //Get the number passed in as an argument
    int key_length = atoi(argv[1]);
    
    //Loop through and output a new character based on the number generated
    for(int i = 0 ; i < key_length ; i++){
        int rand_char = rand() % 27;
        if(rand_char == 26){
            printf(" ");
        }else{
            printf("%c", (char)(rand_char + ASCII_START));
        }
    }
    
    //Print the final newline and exit successfully.
    printf("\n");
    exit(PROGRAM_SUCCESS);
}

