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
#include <math.h>

#include "probConst.h"


/** \brief consumer threads return status array */
extern int statusCons[N + 1];


//static unsigned int char_values[K];
extern FILE* fp;
extern char **filenames;

int num_bytes = 1;

int ready_read = 0;


/** \brief insertion pointer */
//static unsigned int ii;

/** \brief retrieval pointer */
//static unsigned int ri;

/** \brief flag signaling the data transfer region is full */
static bool full;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t vars_access = PTHREAD_MUTEX_INITIALIZER;

/** \brief flag which warrants that the data transfer region is initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;;

/** \brief producers synchronization point when the data transfer region is full */
//static pthread_cond_t fifoFull;

/** \brief consumers synchronization point when the data transfer region is empty */
//static pthread_cond_t fifoEmpty;

static pthread_cond_t open_fi;
static pthread_cond_t close_fi;


int files_idx = 0;
int file_opened = 0;
int file_closed = 0;
int done_reading = 0;
int num_vowels = 0;
int num_cons = 0;
int total_num_words = 1;
int value_before = 0;
int end_of_word = 0;
int flag = 0;

extern int num_files;

static void initialization (void)
{
                                                                                   /* initialize FIFO in empty state */
  full = false;                                                                                  /* FIFO is not full */

  pthread_cond_init (&open_fi, NULL);                                 /* initialize producers synchronization point */
  pthread_cond_init (&close_fi, NULL);                                /* initialize consumers synchronization point */
}


int file_available(unsigned int id){
  if ((statusCons[id] = pthread_mutex_lock (&vars_access)) != 0)                                   /* enter monitor */
     { errno = statusCons[id];                                                            /* save error in errno */
       perror ("error on entering monitor(CF)");
       statusCons[id] = EXIT_FAILURE;
       pthread_exit (&statusCons[id]);
     }

  pthread_once (&init, initialization);
  
  ready_read++;
  
  

  while(ready_read != N) {
      
    if((statusCons[id] = pthread_cond_wait(&open_fi, &vars_access)) != 0 ){
      errno = statusCons[id];                                                            /* save error in errno */
       perror ("error waiting for files");
       statusCons[id] = EXIT_FAILURE;
       pthread_exit (&statusCons[id]);
    }
    
  }

    //printf("worker in file available %d \n", id);
   // usleep((unsigned int) floor (40.0 * random () / RAND_MAX + 1.5));
  //printf("id %d ready_read %d \n", id, ready_read);

  
  
  int flag_file = 1;

  if(files_idx < num_files){
      //printf("%d %d \n", files_idx, num_files);
    if(!file_opened){
      file_opened = 1;
      file_closed = 0;
      done_reading = 0;
      //printf("%d \n", files_idx);
      fp = fopen(filenames[files_idx], "r");
      printf("file opened: %s \n", filenames[files_idx]);

      if(fp == NULL){
        printf("Can't open file %s\n", filenames[files_idx]);
        flag_file = 0;
        file_opened = 0;
      }

    }
  }
  else
    flag_file = 0;

  if ((statusCons[id] = pthread_cond_signal(&open_fi)) != 0) {                          /* */
        errno = statusCons[id];                                                            /* save error in errno */
        perror("error on signaling in close file");
        statusCons[id] = EXIT_FAILURE;
        pthread_exit(&statusCons[id]);
    }
    //printf("depois do signal: %d \n", id);

    if ((statusCons[id] = pthread_mutex_unlock(&vars_access)) != 0) {                             /* exit monitor */
        errno = statusCons[id];                                                            /* save error in errno */
        perror("error on exiting monitor(CF)");
        statusCons[id] = EXIT_FAILURE;
        pthread_exit(&statusCons[id]);
    }
    //printf("%d %d \n", id, flag_file);
    return flag_file;

}

void closeFile(unsigned int id){
    //printf("id %d done_reading %d \n", id, done_reading);
    //printf("entrou \n");
    //printf("Worker id 2: %d \n", id);
    //printf("%d %d\n", num_files, files_idx);
  if ((statusCons[id] = pthread_mutex_lock(&vars_access)) != 0) {                              /* enter monitor */
        errno = statusCons[id];                                                            /* save error in errno */
        perror("error on entering monitor(CF)");
        statusCons[id] = EXIT_FAILURE;
        pthread_exit(&statusCons[id]);
    }
    //printf("id %d done_reading %d \n", id, done_reading);
    //pthread_once (&init, initialization);
    //printf("Worker id 2: %d \n", id);
    
    done_reading++;
    //printf("done reading: %d \n", done_reading);
    while (done_reading != N) {                         /* wait until all the workers have finished reading the file */
        if ((statusCons[id] = pthread_cond_wait(&close_fi, &vars_access)) != 0) {
            errno = statusCons[id];                                                        /* save error in errno */
            perror("error on waiting in close file");
            statusCons[id] = EXIT_FAILURE;
            pthread_exit(&statusCons[id]);
        }
    }

    //printf("ola");

    if (!file_closed) {
        printf("file num %d closed. \n", files_idx);
        fclose(fp);
        files_idx++;
        file_opened = 0;
        file_closed = 1;
        ready_read = 0;
        total_num_words = 0;
        num_vowels = 0;
        num_cons = 0;
    }

    if ((statusCons[id] = pthread_cond_signal(&close_fi)) != 0) {                          /* */
        errno = statusCons[id];                                                            /* save error in errno */
        perror("error on signaling in close file");
        statusCons[id] = EXIT_FAILURE;
        pthread_exit(&statusCons[id]);
    }

    if ((statusCons[id] = pthread_mutex_unlock(&vars_access)) != 0) {                             /* exit monitor */
        errno = statusCons[id];                                                            /* save error in errno */
        perror("error on exiting monitor(CF)");
        statusCons[id] = EXIT_FAILURE;
        pthread_exit(&statusCons[id]);
    }
}

// function that returns the next char in integer value
int get_int(FILE *fp){
    
    int ch_value = fgetc(fp);
    //printf("%d \n", ch_value);
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

//static void initialization (void)
//{
//                                                                                   /* initialize FIFO in empty state */
//  ii = ri = 0;                                        /* FIFO insertion and retrieval pointers set to the same value */
//  full = false;     /* FIFO is not full */
//  fp = fopen("countWords/text0.txt", "r");
//  num_vowels = 0;
//  num_cons = 0;
//  total_num_words = 0;
//
//  value_before = 0;
//
//  end_of_word = 0;
//
//  flag = 0;                                                                             
//
//  pthread_cond_init (&fifoFull, NULL);                                 /* initialize producers synchronization point */
//  pthread_cond_init (&fifoEmpty, NULL);                                /* initialize consumers synchronization point */
//}

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
  //unsigned int val;      
                                                                        /* retrieved value */

  if ((statusCons[consId] = pthread_mutex_lock (&vars_access)) != 0)                                   /* enter monitor */
     { errno = statusCons[consId];                                                            /* save error in errno */
       perror ("error on entering monitor(CF)");
       statusCons[consId] = EXIT_FAILURE;
       pthread_exit (&statusCons[consId]);
     }

    //printf("%d \n", consId);
    //printf("ID: %d \n", consId); 
  pthread_once (&init, initialization);                                              /* internal data initialization */
//
  //while ((ii == ri) && !full)                                           /* wait if the data transfer region is empty */
  //{ if ((statusCons[consId] = pthread_cond_wait (&fifoEmpty, &vars_access)) != 0)
  //     { errno = statusCons[consId];                                                          /* save error in errno */
  //       perror ("error on waiting in fifoEmpty");
  //       statusCons[consId] = EXIT_FAILURE;
  //       pthread_exit (&statusCons[consId]);
  //     }
  //}

  //val = mem[ri];  
    int flag_file_over = 0;

  for(int counter = 0; counter < num_bytes; counter++){
      int ch_value = get_int(fp);
      //printf("%d %d \n", ch_value, consId);
      
      if(ch_value == -1){
          //printf(" chegou ao fim -1 \n");
          //printf("%d \n", consId);
          flag_file_over = 1;
          break;
      }
      
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
            continue;
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
  //ri = (ri + 1) % K;
  full = false;

  //if ((statusCons[consId] = pthread_cond_signal (&fifoFull)) != 0)       /* let a producer know that a value has been
  //                                                                                                          retrieved */
  //   { errno = statusCons[consId];                                                             /* save error in errno */
  //     perror ("error on signaling in fifoFull");
  //     statusCons[consId] = EXIT_FAILURE;
  //     pthread_exit (&statusCons[consId]);
  //   }

  if ((statusCons[consId] = pthread_mutex_unlock (&vars_access)) != 0)                                   /* exit monitor */
     { errno = statusCons[consId];                                                             /* save error in errno */
       perror ("error on exiting monitor(CF)");
       statusCons[consId] = EXIT_FAILURE;
       pthread_exit (&statusCons[consId]);
     }

  if(flag_file_over == 1){
      return 0;
  }

  //printf("%d \n", total_num_words);
  return 1;
}


