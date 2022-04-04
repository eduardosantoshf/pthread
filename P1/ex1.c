#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <libgen.h> 
#include <unistd.h>

static void printUsage (char *cmdName);

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
int get_next_char(FILE *fp){
    int bytes, ch = fgetc(fp);
    //printf("%d \n", bytes);
    //printf("%d \n \n", ch);
    
    if(ch == -1) 
        return -1;

    if((ch & 0x80) == 0) 
        return ch;

    for (bytes = 1; ch & (0x80 >> bytes); bytes++){
        ch = ch & (1 << (7 - bytes)) - 1;
    }

    for(; bytes > 1; bytes --){

        int c = fgetc(fp);

        if (c == -1) 
            return -1;

        ch = (ch << 6) | (c & 0x3F);
    }

    return ch;

}


int main(int argc, char* argv[]){
    int num_vowels = 0;
    int num_cons = 0;
    int total_num_words = 0;

    int value_before = 0;

    int end_of_word = 0;

    int flag = 0;

    double t0, t1, t2; /* time limits */
    t2 = 0.0;

    int opt;                  /* selected option */
    char *fName = "no name";  /* file name (initialized to "no name" by default) */
    int val = -1;             /* numeric value (initialized to -1 by default) */
    opterr = 0;
    do
    { switch ((opt = getopt (argc, argv, "f:n:h"))) { 
        case 'f': /* file name */
                if (optarg[0] == '-')
                    { fprintf (stderr, "%s: file name is missing\n", basename (argv[0]));
                    printUsage (basename (argv[0]));
                    return EXIT_FAILURE;
                    }
                    fName = optarg;
                    break;
        case 'n': /* numeric argument */
                    if (atoi (optarg) <= 0)
                    { fprintf (stderr, "%s: non positive number\n", basename (argv[0]));
                        printUsage (basename (argv[0]));
                        return EXIT_FAILURE;
                    }
                    val = (int) atoi (optarg);
                    break;
        case 'h': /* help mode */
                    printUsage (basename (argv[0]));
                    return EXIT_SUCCESS;
        case '?': /* invalid option */
                    fprintf (stderr, "%s: invalid option\n", basename (argv[0]));
                printUsage (basename (argv[0]));
                    return EXIT_FAILURE;
        case -1:  break;
        }
    } while (opt != -1);

    if (argc == 1){ 
        fprintf (stderr, "%s: invalid format\n", basename (argv[0]));
        printUsage (basename (argv[0]));
        return EXIT_FAILURE;
    }

    int o; /* counting variable */

    printf ("File name = %s\n", fName);
    printf ("Numeric value = %d\n", val);

    for (o = 0; o < argc; o++)
        printf ("Word %d = %s\n", o, argv[o]);

    printf("\n");

    // loop through the arguments
    for(int i = 1; i < argc; i++){

        // open file
        FILE* file = fopen(fName, "r");

        t0 = ((double) clock ()) / CLOCKS_PER_SEC;

        do{
            // next char value
            int char_value = get_next_char(file);
            
            // check if first char of file is vowel
            if(flag == 0){
                if(is_vowel(char_value) == 1){
                    num_vowels += 1;
                }
                flag = 1;
            }

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

        t1 = ((double) clock ()) / CLOCKS_PER_SEC;
        t2 += t1 - t0;

        // close file and reset counting variables
        fclose(file);

        // print solutions
        printf("%s \n", argv[i]);
        printf("Number of words: %d \n", total_num_words);
        printf("Number of words starting with a vowel: %d \n", num_vowels);
        printf("Number of words ending with a consonant: %d \n\n", num_cons);

        num_vowels = 0;
        num_cons = 0;
        total_num_words = 0;
        
        end_of_word = 0;

        value_before = 0;

        flag = 0;
       
    }

    printf ("\nElapsed time = %.6f s\n", t2);

}

static void printUsage (char *cmdName) {
  fprintf (stderr, "\nSynopsis: %s OPTIONS [filename / positive number]\n"
           "  OPTIONS:\n"
           "  -h      --- print this help\n"
           "  -f      --- filename\n"
           "  -n      --- positive number\n", cmdName);
}