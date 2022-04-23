/**
 *  \file shared.h (interface file)
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


/** \brief Verify if there's files available to open, and open them. */
extern int check_for_file(unsigned int id);

/** \brief Close the opened file, if there's one opened. */
extern void check_close_file(unsigned int id);

/** \brief Reads a specified number of bytes from the file and computes the chunk. */
extern unsigned int getVal (unsigned int consId);

/** \brief Save resuts for a single file. */
extern void save_file_results(unsigned int consId);

/** \brief Print final results. */
extern void print_final_results();




