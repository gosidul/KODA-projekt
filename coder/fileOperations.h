#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H
#define BUFFER_BYTE_LENGTH 8
#define BUFFER_BIT_LEN 64 

#include "treeOperations.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
  * @brief  Creates and opens a file for binary writing operations
  *         This function prompts the user to input a valid file name,
  *         appends a ".bin" extension to it, and opens the file in binary
  *         write mode. If the operation fails, it returns NULL.
  * @param  my Pointer to struct containing data.
  * @retval FILE pointer if file created succesfully, or NULL if an error occurs.
  */
FILE* createCompressedFile();

/**
  * @brief  Reads data from a file and stores it in a 2D array.
  *         This function prompts the user to input a valid file path
  *         and attempts to open the file in binary read mode. If the operation
  *         is unsuccessful (for example the file does not exist or cannot be accessed),
  *         it returns 1.
  * @param  None
  * @retval 0 if read was succesfull, or 1 if an error occurs.
  */
uint8_t readDataFromFile(records* my);

/**
  * @brief  Writes data to buffer and to file if buffer is full.
  * @param  compressedFile Pointer to FILE object.
  * @param  my Pointer to struct containing data.
  * @param  data Variable which holds data we want write to file.
  * @param  count Variable which holds number of bits we want write to file.
  *         Set as "0" to append remaining bits in buffer
  * @return 0 if write was succesfull, or 1 if an error occurs.
  */
uint8_t writeToFile(dataBuffer* my, FILE* compressedFile, uint32_t input, uint8_t count);

#endif // FILE_OPERATIONS_H
