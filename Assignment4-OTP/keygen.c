/* 
 * File:   keygen.c
 * Author: Corwin Perren
 *
 * Created on November 29, 2016, 7:27 PM
 */

#include <stdio.h>
#include <stdlib.h>

#define PROGRAM_SUCESS 0
#define PROGRAM_FAILURE 1

/*
 * 
 */
int main(int argc, char** argv) {
    if(argc < 2){
        fprintf(stderr, "No arguments entered.\nPlease try again.\n");
        exit(PROGRAM_FAILURE);
    }
    
    
    
    
    return (PROGRAM_SUCESS);
}

