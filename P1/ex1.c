#include <stdio.h>
#include <string.h>
#include <ctype.h>

// function to check if it is a vowel
int is_vowel(int char_value) {
    int vowels[] = {97, 101, 105, 111, 117, 65, 69, 73, 79, 85, 224, 225, 226, 227, 232, 233, 234, 236, 237, 238,
                          242, 243, 244, 245, 249, 250,  192, 193, 194, 195, 200, 201, 202, 204, 205, 206, 210,
                          211, 212, 213, 217, 218, 219, 251 };

    for (int i = 0; i < sizeof(vowels) / sizeof(vowels[0]); i++)
        if (vowels[i] == char_value){
            return 1;
        }

    return 0;
}

// function to check if it is a consonant
int is_consonant(int char_value) {
    int consonants[] = {98, 99, 100, 102, 103, 104, 106, 107, 108, 109, 110, 112, 113, 114, 115, 116, 118, 119, 120, 121, 122,
                        66, 67, 68, 70, 71, 72, 74, 75, 76, 77, 78, 80, 81, 82, 83, 84, 86, 87, 88, 89, 90,
                        231, 199};

    for (int i = 0; i < sizeof(consonants) / sizeof(consonants[0]); i++)
        if (consonants[i] == char_value)
            return 1;

    return 0;
}

// function to check if it is a split char
int is_split(int char_value) {
    int splits[] = {32, 9, 10, 45, 34, 8220, 8221, 91, 93, 123, 125, 40, 41, 46, 44,
                                58, 59, 63, 33, 8211, 8212, 8230, 171, 187, 96};

    for (int i = 0; i < sizeof(splits) / sizeof(splits[0]); i++)
        if (splits[i] == char_value)
            return 1;

    return 0;
}

// function that returns the next char in integer value
int get_int(FILE *fp){
    
    int ch_value = fgetc(fp);
    int b = 0;
    
    // if EOF
    if(ch_value == -1) 
        return -1;

    // if is only 1 byte char, return it
    if((ch_value & 128) == 0) {
        return ch_value;
    }

    // if contains 226 ('e2'), then it is a 3 byte char
    if(ch_value == 226){
        b = 3;
        ch_value = ch_value & (1 << 4) - 1;
    }
    // else, is a 2 byte char
    else{
        b = 2;
        ch_value = ch_value & (1 << 5) - 1;
    }

    // go through number of the char bytes
    for(int x = 1; x < b; x ++){

        // get next byte
        int next_ch_value = fgetc(fp);

        // if EOF
        if (next_ch_value == -1) 
            return -1;
        
        // calculate int value of the char
        ch_value = (ch_value << 6) | (next_ch_value & 63);
    }

    return ch_value;
}


int main(int argc, char* argv[]){

    int num_vowels = 0;
    int num_cons = 0;
    int total_num_words = 0;

    int value_before = 0;

    int end_of_word = 0;

    int flag = 0;

    // loop through the arguments
    for(int i = 1; i < argc; i++){

        // open file
        FILE* file = fopen(argv[i], "r");

        do{
            // next char value
            int char_value = get_int(file);
            
            // check if first char of file is vowel
            if(flag == 0){
                if(is_vowel(char_value) == 1){
                    num_vowels += 1;
                }
                flag = 1;
            }

            // check if is a lonely apostrophe to avoid counting as word
            if(char_value == 39 || char_value == 8216 || char_value == 8217){
                if(is_split(value_before)){
                    continue;
                }
            }

            // if is split char
            if(is_split(char_value)){

                // check if previous char was a consonant
                if(is_consonant(value_before))
                    num_cons += 1;
                
                end_of_word = 1;
            }

            // not a split chat
            else{

                // check if is end of word to sum total words
                if(end_of_word == 1){

                    total_num_words += 1;
                    end_of_word = 0;

                    // if first char of new word is vowel
                    if(is_vowel(char_value) == 1){

                        num_vowels += 1;
                    }
                        
                }
            }

            // save previous char to check in next iteration
            value_before = char_value;
            
        } while(!feof(file));
        

        

        // print solutions
        printf("%s \n", argv[i]);
        printf("Number of words: %d \n", total_num_words);
        printf("Number of words starting with a vowel: %d \n", num_vowels);
        printf("Number of words ending with a consonant: %d \n\n", num_cons);

        // close file and reset counting variables
        fclose(file);

        num_vowels = 0;
        num_cons = 0;
        total_num_words = 0;
        
        end_of_word = 0;

        value_before = 0;

        flag = 0;
       
    }

    

}


