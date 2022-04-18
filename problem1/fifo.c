/**
 *  \file fifo.c (implementation file)
 *
 *  \brief Problem name: Producers / Consumers.
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Data transfer region implemented as a monitor.
 *
 *  Definition of the operations carried out by the producers / consumers:
 *     \li putVal
 *     \li getVal.
 *
 *  \author António Rui Borges - March 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "probConst.h"

/** \brief producer threads return status array */
extern int statusProd[N];

/** \brief consumer threads return status array */
extern int statusCons[N];

/** \brief storage region */
static unsigned int mem[K];

//static unsigned int char_values[K];
static FILE *file_pointer;
FILE* file;
int num_vowels;
int num_cons;
int total_num_words;

int value_before;

int end_of_word;

int flag;

int num_bytes = 5;

/** \brief insertion pointer */
static unsigned int ii;

/** \brief retrieval pointer */
static unsigned int ri;

/** \brief flag signaling the data transfer region is full */
static bool full;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/** \brief flag which warrants that the data transfer region is initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;;

/** \brief producers synchronization point when the data transfer region is full */
static pthread_cond_t fifoFull;

/** \brief consumers synchronization point when the data transfer region is empty */
static pthread_cond_t fifoEmpty;

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

/**
 *  \brief Initialization of the data transfer region.
 *
 *  Internal monitor operation.
 */

static void initialization (void)
{
                                                                                   /* initialize FIFO in empty state */
  ii = ri = 0;                                        /* FIFO insertion and retrieval pointers set to the same value */
  full = false;     /* FIFO is not full */
  file = fopen("countWords/text1.txt", "r");
  num_vowels = 0;
  num_cons = 0;
  total_num_words = 0;

  value_before = 0;

  end_of_word = 0;

  flag = 0;                                                                             

  pthread_cond_init (&fifoFull, NULL);                                 /* initialize producers synchronization point */
  pthread_cond_init (&fifoEmpty, NULL);                                /* initialize consumers synchronization point */
}

/**
 *  \brief Store a value in the data transfer region.
 *
 *  Operation carried out by the producers.
 *
 *  \param prodId producer identification
 *  \param val value to be stored
 */

void putVal (unsigned int prodId, unsigned int val, FILE *f)
{
  if ((statusProd[prodId] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
     { errno = statusProd[prodId];                                                            /* save error in errno */
       perror ("error on entering monitor(CF)");
       statusProd[prodId] = EXIT_FAILURE;
       pthread_exit (&statusProd[prodId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  while (full)                                                           /* wait if the data transfer region is full */
  { if ((statusProd[prodId] = pthread_cond_wait (&fifoFull, &accessCR)) != 0)
       { errno = statusProd[prodId];                                                          /* save error in errno */
         perror ("error on waiting in fifoFull");
         statusProd[prodId] = EXIT_FAILURE;
         pthread_exit (&statusProd[prodId]);
       }
  }

  mem[ii] = val;       
  
  file_pointer = f;
  //int ch_value = get_int(f);
  //char_values[ii] = ch_value;
                                                                     /* store value in the FIFO */
  ii = (ii + 1) % K;
  full = (ii == ri);

  if ((statusProd[prodId] = pthread_cond_signal (&fifoEmpty)) != 0)      /* let a consumer know that a value has been
                                                                                                               stored */
     { errno = statusProd[prodId];                                                             /* save error in errno */
       perror ("error on signaling in fifoEmpty");
       statusProd[prodId] = EXIT_FAILURE;
       pthread_exit (&statusProd[prodId]);
     }

  if ((statusProd[prodId] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
     { errno = statusProd[prodId];                                                            /* save error in errno */
       perror ("error on exiting monitor(CF)");
       statusProd[prodId] = EXIT_FAILURE;
       pthread_exit (&statusProd[prodId]);
     }
}

/**
 *  \brief Get a value from the data transfer region.
 *
 *  Operation carried out by the consumers.
 *
 *  \param consId consumer identification
 *
 *  \return value
 */

unsigned int getVal (unsigned int consId)
{
  //unsigned int val;                                                                             /* retrieved value */

  if ((statusCons[consId] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
     { errno = statusCons[consId];                                                            /* save error in errno */
       perror ("error on entering monitor(CF)");
       statusCons[consId] = EXIT_FAILURE;
       pthread_exit (&statusCons[consId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  while ((ii == ri) && !full)                                           /* wait if the data transfer region is empty */
  { if ((statusCons[consId] = pthread_cond_wait (&fifoEmpty, &accessCR)) != 0)
       { errno = statusCons[consId];                                                          /* save error in errno */
         perror ("error on waiting in fifoEmpty");
         statusCons[consId] = EXIT_FAILURE;
         pthread_exit (&statusCons[consId]);
       }
  }

  //val = mem[ri];  

  for(int counter = 0; counter < num_bytes; counter++){

      int ch_value = get_int(file);
      //printf("%d \n", ch_value);
      //printf("%d \n\n", consId);
      //val = char_values[ri];
  
      // check if first char of file is vowel

      if(flag == 0){
          if(is_vowel(ch_value) == 1){
              num_vowels += 1;
          }
          flag = 1;
      }

      // check if is a lonely apostrophe to avoid counting as word
      if(ch_value == 39 || ch_value == 8216 || ch_value == 8217){
          if(is_split(value_before)){
          }
      }

      // if is split char
      if(is_split(ch_value)){

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
              if(is_vowel(ch_value) == 1){

                  num_vowels += 1;
              }
                  
          }
      }

      // save previous char to check in next iteration
      value_before = ch_value;
  }

  
  
  
                                                                   /* retrieve a  value from the FIFO */
  ri = (ri + 1) % K;
  full = false;

  if ((statusCons[consId] = pthread_cond_signal (&fifoFull)) != 0)       /* let a producer know that a value has been
                                                                                                            retrieved */
     { errno = statusCons[consId];                                                             /* save error in errno */
       perror ("error on signaling in fifoFull");
       statusCons[consId] = EXIT_FAILURE;
       pthread_exit (&statusCons[consId]);
     }

  if ((statusCons[consId] = pthread_mutex_unlock (&accessCR)) != 0)                                   /* exit monitor */
     { errno = statusCons[consId];                                                             /* save error in errno */
       perror ("error on exiting monitor(CF)");
       statusCons[consId] = EXIT_FAILURE;
       pthread_exit (&statusCons[consId]);
     }

  return total_num_words;
}


