#ifndef BIT_OPERATIONS_H
#define BIT_OPERATIONS_H

#define MAX_BYTE_NUM 393216
#define BASE_BUFFER_SIZE 1024
#define CHUNK_SIZE 128
#define BITS_IN_BYTE 8
#define MSB 128

#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"

/**
 * @brief: Represents base instance of buffer
 * @dataBuffer: Array storing bits in 8bit variables
 * @multiplier: Tells us how many times buffer was expanded
 * @baseBufferSize: Base size of chunk of data buffer
 * @killMe: destructor
 */
typedef struct baseBuffer {
    uint8_t* dataBuffer;
    uint16_t  multiplier;
    uint16_t baseBufferSize;
    void (*killMe)(struct baseBuffer**);
} baseBuffer;

/**
 * @brief: Represents instance of buffer storing byte data
 * @baseBuffer: Pointer to buffer struct storing data
 * @currentByte: Tells us possition of last added byte
 * @appendByte: Add byte to baseBuffer
 * @killMe: destructor
 */
typedef struct byteBuffer {
    baseBuffer* baseBuffer;
    uint64_t currentByte;
    uint8_t (*appendByte)(struct byteBuffer*, uint8_t);
    void (*killMe)(struct byteBuffer**);
} byteBuffer;

/**
 * @brief: Represents instance of bit buffer (each bit
 *         in variable should be considered separately)
 * @baseBuffer: Pointer to buffer struct storing data
 * @lastByte: Tells us how many bytes buffer has
 * @currentByte: Tells us possition of currently read byte
 * @currentShift: Tells us possition of currently read bit
 * @popSymbol: retrieve 8 bits from baseBuffer
 * @popBit: retrieve single bit from baseBuffer
 * @killMe: destructor
 */
typedef struct bitBuffer {
    baseBuffer* baseBuffer;
    uint64_t lastByte;
    uint64_t currentByte;
    uint8_t currentShift;
    uint8_t (*popSymbol)(struct bitBuffer*);
    uint8_t (*popBit)(struct bitBuffer*);
    void (*killMe)(struct bitBuffer**);
} bitBuffer;

/** 
 * @brief:  Creates byte buffer instance
 * @param:  None
 * @retval: Pointer to newly created buffer, or NULL on error
 */
byteBuffer* createByteBuffer(uint16_t baseBufferSize);

/** 
 * @brief:  Creates bit buffer instance
 * @param:  None
 * @retval: Pointer to newly created buffer, or NULL on error
 */
bitBuffer* createBitBuffer(uint16_t baseBufferSize);

/** 
 * @brief:  Ask user for name for decopressed file and loads it with decompressed data
 * @param:  this - pointer to buffer structure
 * @retval: 0 if succesfully loads data to file, 1 otherwise
 */
uint8_t writeToFile(byteBuffer* this);

/** 
 * @brief:  Frees memory allocated for input data buffer
 * @param:  this - address of pointer to buffer structure
 * @retval: None
 */
void freeBitBuffer(bitBuffer** this);

/** 
 * @brief:  Frees memory allocated for byte data buffer
 * @param:  this - address of pointer to buffer structure
 * @retval: None
 */
void freeByteBuffer(byteBuffer** this);

#endif