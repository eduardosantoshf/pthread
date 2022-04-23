/**
 *  \file shared.c (implementation file)
 *
 *  \brief Problem name: Total number of words, number of words beginning with a vowel and ending with a consonant.
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Data transfer region implemented as a monitor.
 *
 *  Definition of the operations carried out by the workers:
 *     \li check_for_file
 *     \li check_close_file
 *     \li getVal
 *     \li save_file_results.
 * 
 *  Definition of the operations carried out by the main thread:
 *     \li print_final_results.
 *
 *  \author Eduardo Santos and Pedro Bastos - April 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>

#include "probConst.h"

/** \brief worker threads return status array */
extern int statusCons[N];

/** \brief pointer to the file */
FILE *fp;

/** \brief array of the filenames retrieved from the main file */
extern char **filenames;

/** \brief variable to save the number of files */
extern int num_files;

/** \brief array to save the total number of words for each file */
extern int *array_num_words;

/** \brief array to save the number of words beginning with a vowel for each file */
extern int *array_num_vowels;

/** \brief array to save the number of words ending with a consonant for each file */
extern int *array_num_cons;

/** \brief Size of the chunk to read */
int num_bytes = 10;

/** \brief variable to count the number of workers ready to read the file */
int wait_for_read = 0;

/** \brief variable to count the number of workers ready to close the file */
int end_of_file = 0;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t vars_access = PTHREAD_MUTEX_INITIALIZER;

/** \brief flag which warrants that the conditions are initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;;

/** \brief condition which warrants that all the workers are ready to read the file */
static pthread_cond_t wait_file_open;

/** \brief condition which warrants that all the workers are ready to close the file */
static pthread_cond_t wait_file_close;

/** \brief variable to save the current file index */
int index_file = 0;

/** \brief flag that indicates if the file was already opened */
int open_file = 0;

/** \brief flag that indicates if the file was already closed */
int close_file = 0;

/** \brief variable to save the number of words beginning with a vowel for each file */
int num_vowels = 0;

/** \brief variable to save the number of words ending with a consonant for each file */
int num_cons = 0;

/** \brief variable to save the total number of words for each file */
int total_num_words = 1;

/** \brief variables used in the computation of each file */
int value_before = 0;
int end_of_word = 0;
int flag = 0;

/** \brief flag that indicates the partial results are being saved */
int partial_results = 0;

/** 
 *  \brief Initialize the needed conditions.
 *  
 *  Operation carried out by the workers. 
 */

static void initialization(void) {
  pthread_cond_init(&wait_file_open, NULL);                                 /* initialize condition to wait for workers to open a file */
  pthread_cond_init(&wait_file_close, NULL);                               /* initialize condition to wait for workers to close a file */
}

/** 
 *  \brief Wait for the workers to be ready and then open the next file.
 *  
 *  Operation carried out by the workers.
 * 
 *  \param id worker identification
 *  \return 1 for Success and 0 for Failure.
 */

int check_for_file(unsigned int id) {

  if ((statusCons[id] = pthread_mutex_lock(&vars_access)) != 0) {                                                     /* enter monitor */
    errno = statusCons[id];                                                                                     /* save error in errno */
    perror("error on entering monitor(CF)");
    statusCons[id] = EXIT_FAILURE;
    pthread_exit(&statusCons[id]);
  }

  pthread_once(&init, initialization);

  /* Increment number of workers ready */
  wait_for_read++;

  int flag_file = 1;

  while (wait_for_read != N) {                                                               /* Wait while there are workers not ready */
    if ((statusCons[id] = pthread_cond_wait(&wait_file_open, &vars_access)) != 0){
      errno = statusCons[id];                                                       
      perror("error on waiting condition to open file");
      statusCons[id] = EXIT_FAILURE;
      pthread_exit(&statusCons[id]);
    }
  }

  if (index_file < num_files) {                                                          /* Check if there are more files to be opened */
    if (!open_file) {                                            /* Check if the file is not already being opened and, if not, open it */
      open_file = 1;
      close_file = 0;
      end_of_file = 0;
      fp = fopen(filenames[index_file], "r");
    }
  }

  else{
    flag_file = 0;
  }

  if ((statusCons[id] = pthread_cond_signal(&wait_file_open)) != 0) {                               /* Signaling the waiting condition */
    errno = statusCons[id];
    perror("error on signaling condition to open file");
    statusCons[id] = EXIT_FAILURE;
    pthread_exit(&statusCons[id]);
  }

  if ((statusCons[id] = pthread_mutex_unlock(&vars_access)) != 0) {                                                    /* exit monitor */
    errno = statusCons[id];                                                                                     /* save error in errno */
    perror("error on exiting monitor(CF)");
    statusCons[id] = EXIT_FAILURE;
    pthread_exit(&statusCons[id]);
  }

  return flag_file;
}

/** 
 *  \brief Wait for the workers to be ready and then close the file.
 *  
 *  Operation carried out by the workers.
 * 
 *  \param id worker identification
 */

void check_close_file(unsigned int id) {
  if ((statusCons[id] = pthread_mutex_lock(&vars_access)) != 0) {                                                     /* enter monitor */
    errno = statusCons[id];                                                                                     /* save error in errno */
    perror("error on entering monitor(CF)");
    statusCons[id] = EXIT_FAILURE;
    pthread_exit(&statusCons[id]);
  }

  /* Increment number of workers ready */
  end_of_file++;

  while (end_of_file != N) {                                                                 /* Wait while there are workers not ready */
    if ((statusCons[id] = pthread_cond_wait(&wait_file_close, &vars_access)) != 0) {
      errno = statusCons[id]; 
      perror("error on waiting condition to close file");
      statusCons[id] = EXIT_FAILURE;
      pthread_exit(&statusCons[id]);
    }
  }

  if (!close_file) {                                            /* Check if the file is not already being closed and, if not, close it */
    fclose(fp);
    index_file++;
    open_file = 0;
    close_file = 1;
    wait_for_read = 0;
    total_num_words = 0;
    num_vowels = 0;
    num_cons = 0;
    partial_results = 0;
  }

  if ((statusCons[id] = pthread_cond_signal(&wait_file_close)) != 0) {                              /* Signaling the waiting condition */
    errno = statusCons[id];
    perror("error on signaling condition to close file");
    statusCons[id] = EXIT_FAILURE;
    pthread_exit(&statusCons[id]);
  }

  if ((statusCons[id] = pthread_mutex_unlock(&vars_access)) != 0) {                                                    /* exit monitor */
    errno = statusCons[id];                                                                                     /* save error in errno */
    perror("error on exiting monitor(CF)");
    statusCons[id] = EXIT_FAILURE;
    pthread_exit(&statusCons[id]);
  }
}

/** 
 *  \brief Calculate the next char and convert it to integer.
 *  
 *  Operation carried out by the workers.
 * 
 *  \param fp pointer to file.
 *  \return value.
 */

int get_int(FILE *fp) {

  int ch_value = fgetc(fp);
  int b = 0;

  if (ch_value == -1) /* if EOF */
    return -1;

  if ((ch_value & 128) == 0) { /* if is only 1 byte char, return it */
    return ch_value;
  }

  /* if contains 226 ('e2'), then it is a 3 byte char */
  if (ch_value == 226) { 
    b = 3;
    ch_value = ch_value & (1 << 4) - 1;
  }

  /* else, is a 2 byte char */
  else {
    b = 2;
    ch_value = ch_value & (1 << 5) - 1;
  }

  /* go through number of the char bytes */
  for (int x = 1; x < b; x++) {

    /* get next byte */
    int next_ch_value = fgetc(fp);

    /* if EOF */
    if (next_ch_value == -1)
      return -1;

    /* calculate int value of the char */
    ch_value = (ch_value << 6) | (next_ch_value & 63);
  }

  return ch_value;
}

/** 
 *  \brief Check if a given char is a vowel.
 *  
 *  Operation carried out by the workers.
 * 
 *  \param char_value character value to be checked.
 *  \return 1 if is a vowel, 0 otherwise.
 */

int is_vowel(int char_value) {
  /* list of all the vowels values */
  int vowels[] = {97, 101, 105, 111, 117, 65, 69, 73, 79, 85, 224, 225, 226, 227, 232, 233, 234, 236, 237, 238,
                  242, 243, 244, 245, 249, 250, 192, 193, 194, 195, 200, 201, 202, 204, 205, 206, 210,
                  211, 212, 213, 217, 218, 219, 251};

  /* go through the list and check if it contains the given char */
  for (int i = 0; i < sizeof(vowels) / sizeof(vowels[0]); i++)
    if (vowels[i] == char_value) {
      return 1;
    }

  return 0;
}

/** 
 *  \brief Check if a given char is a consonant.
 *  
 *  Operation carried out by the workers.
 * 
 *  \param char_value character value to be checked.
 *  \return 1 if is a consonant, 0 otherwise.
 */

int is_consonant(int char_value) {
  /* list of all the consonants values */
  int consonants[] = {98, 99, 100, 102, 103, 104, 106, 107, 108, 109, 110, 112, 113, 114, 115, 116, 118, 119, 120, 121, 122,
                      66, 67, 68, 70, 71, 72, 74, 75, 76, 77, 78, 80, 81, 82, 83, 84, 86, 87, 88, 89, 90,
                      231, 199};

  /* go through the list and check if it contains the given char */
  for (int i = 0; i < sizeof(consonants) / sizeof(consonants[0]); i++)
    if (consonants[i] == char_value)
      return 1;

  return 0;
}

/** 
 *  \brief Check if a given char is a split.
 *  
 *  Operation carried out by the workers.
 * 
 *  \param char_value character value to be checked.
 *  \return 1 if is a split, 0 otherwise.
 */

int is_split(int char_value) {
  /* list of all the split char values */
  int splits[] = {32, 9, 10, 45, 34, 8220, 8221, 91, 93, 123, 125, 40, 41, 46, 44,
                  58, 59, 63, 33, 8211, 8212, 8230, 171, 187, 96};

  /* go through the list and check if it contains the given char */
  for (int i = 0; i < sizeof(splits) / sizeof(splits[0]); i++)
    if (splits[i] == char_value)
      return 1;

  return 0;
}

/**
 *  \brief Reads a specified number of bytes from the file and computes the chunk.
 *
 *  Operation carried out by the workers.
 *
 *  \param consId worker identification.
 *  \return 0 if it reaches EOF, 1 otherwise.
 */

unsigned int getVal(unsigned int consId) {

  if ((statusCons[consId] = pthread_mutex_lock(&vars_access)) != 0) {                                                 /* enter monitor */
    errno = statusCons[consId];                                                                                 /* save error in errno */
    perror("error on entering monitor(CF)");
    statusCons[consId] = EXIT_FAILURE;
    pthread_exit(&statusCons[consId]);
  }

  pthread_once(&init, initialization); 

  /* flag to indicate if the file is over */
  int flag_file_over = 0;

  for (int counter = 0; counter < num_bytes; counter++) {                                          /* read a specified number of bytes */ 

    /* get next char value */
    int ch_value = get_int(fp);

    /* if EOF */
    if (ch_value == -1) {
      flag_file_over = 1;
      break;
    }

    /* check if first char of file is vowel */
    if (flag == 0) {
      if (is_vowel(ch_value) == 1) {
        num_vowels += 1;
      }
      flag = 1;
    }

    /* check if is a lonely apostrophe to avoid counting as word */
    if (ch_value == 39 || ch_value == 8216 || ch_value == 8217) {
      if (is_split(value_before))
        continue;
    }

    /* if is split char */
    if (is_split(ch_value)) {

      /* check if previous char was a consonant */
      if (is_consonant(value_before))
        num_cons += 1;

      end_of_word = 1;
    }

    /* not a split chat */
    else{
      /* check if is end of word to sum total words */
      if (end_of_word == 1) {

        total_num_words += 1;
        end_of_word = 0;

        /* if first char of new word is vowel */
        if (is_vowel(ch_value) == 1)
          num_vowels += 1;
      }
    }

    /* save previous char to check in next iteration */
    value_before = ch_value;
  }

  if ((statusCons[consId] = pthread_mutex_unlock(&vars_access)) != 0) {                                                /* exit monitor */
    errno = statusCons[consId];                                                                                 /* save error in errno */
    perror("error on exiting monitor(CF)");
    statusCons[consId] = EXIT_FAILURE;
    pthread_exit(&statusCons[consId]);
  }

  /* if is EOF */
  if (flag_file_over == 1)
    return 0;

  return 1;
}

/**
 *  \brief Save resuts for a single file.
 *
 *  Operation carried out by the workers.
 *
 *  \param consId worker identification.
 */

void save_file_results(consId) {
  if ((statusCons[consId] = pthread_mutex_lock(&vars_access)) != 0) {                                                 /* enter monitor */
    errno = statusCons[consId];                                                                                 /* save error in errno */
    perror("error on entering monitor(CF)");
    statusCons[consId] = EXIT_FAILURE;
    pthread_exit(&statusCons[consId]);
  }

  /* Ensure that only 1 worker saves the file results */
  if (!partial_results) {
    array_num_words[index_file] = total_num_words;
    array_num_vowels[index_file] = num_vowels;
    array_num_cons[index_file] = num_cons;
    partial_results = 1;
  }

  if ((statusCons[consId] = pthread_mutex_unlock(&vars_access)) != 0) {                                                /* exit monitor */
    errno = statusCons[consId];                                                                                 /* save error in errno */
    perror("error on exiting monitor(CF)");
    statusCons[consId] = EXIT_FAILURE;
    pthread_exit(&statusCons[consId]);
  }
}

/**
 *  \brief print final results.
 *
 *  Operation carried out by the main thread.
 */

void print_final_results() {
  for (int i = 0; i < num_files; i++) {
    printf("File name: %s \n", filenames[i]);
    printf("Total number of words = %d \n", array_num_words[i]);
    printf("N. of words beginning with a vowel = %d \n", array_num_vowels[i]);
    printf("N. of words ending with a consonant = %d \n\n", array_num_cons[i]);
  }
}
