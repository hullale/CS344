/* 
 * File:   otp_enc.c
 * Author: Corwin Perren
 *
 * Created on November 29, 2016, 7:26 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    char *sentence = "MY OWN TEST";
    
    for(int i = 0 ; i < strlen(sentence) ; i++){
        char encoded_char = encode_character(sentence[i], 'A' + i);
        char decoded_char = decode_character(encoded_char, 'A' + i);
        
        printf("%c : %c : %c\n", sentence[i], encoded_char, decoded_char);
    }
    
    exit (EXIT_SUCCESS);
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