/**
 *  \file fifo.h (interface file)
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

#ifndef FIFO_H
#define FIFO_H

/** \brief verify if there's files available to open, and open them. */
extern int file_available(unsigned int id);

/** \brief close the opened file, if there's one opened. */
extern void closeFile(unsigned int id);

/**
 *  \brief Get a value from the data transfer region.
 *
 *  Operation carried out by the consumers.
 *
 *  \param consId consumer identification
 *
 *  \return value
 */

extern unsigned int getVal (unsigned int consId);

extern void writeFinal();

extern void write_file_results(unsigned int consId);

#endif /* FIFO_H */
