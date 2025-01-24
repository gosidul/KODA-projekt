#include "bitOperations.h"

/** 
 * @brief:  Frees memory allocated for input data buffer
 * @param:  this - address of pointer to buffer structure
 * @retval: None
 */
void freeBaseBuffer(baseBuffer** this)
{
    (*this)->killMe = NULL;
    (*this)->baseBufferSize = 0;
    (*this)->multiplier = 0;
    if ((*this)->dataBuffer) 
        free((*this)->dataBuffer);
    free(*this);
    *this = NULL;
}

void freeByteBuffer(byteBuffer** this)
{
    (*this)->killMe = NULL;
    (*this)->appendByte = NULL;
    (*this)->currentByte = 0;
    if ((*this)->baseBuffer) 
        (*this)->baseBuffer->killMe(&(*this)->baseBuffer);
    free(*this);
    *this = NULL;
}

void freeBitBuffer(bitBuffer** this)
{
    (*this)->lastByte = 0;
    (*this)->killMe = NULL;
    (*this)->popBit = NULL;
    (*this)->popSymbol = NULL;
    (*this)->currentShift = 0;
    (*this)->currentByte = 0;
    if ((*this)->baseBuffer) 
        (*this)->baseBuffer->killMe(&(*this)->baseBuffer);
    free(*this);
    *this = NULL;
}

/**
 * @brief:  Function responsible of reallocating memory for array storing bits from
 *          compressed file.
 * @param:  this - pointer to buffer structure.
 * @retval: 0 if succesfully reallocates memory, 1 in case of memory allocation failure
 */
uint8_t reallocateBuffer(baseBuffer* this)
{
    // Allocate larger memory pool
    uint8_t* newBuffer = (uint8_t*)malloc((this->multiplier + 1) * this->baseBufferSize);
    if (!newBuffer) {
        printf("Błąd podczas alokacji nowej pamięci bufora danych!\n");
        return 1;
    } 
    // Copy old data to new buffer and clear old buffer memory
    if (this->dataBuffer) {
        memcpy(newBuffer, this->dataBuffer, this->multiplier * this->baseBufferSize);
        free(this->dataBuffer);
    }
    // Assign new dataBuffer memory to buffer struct
    this->dataBuffer = newBuffer;
    this->multiplier++;
    return 0;
}

/**
 * @brief Extracts the most significant bit (MSB) from the current byte in the bit buffer,
 *        shifts the byte to prepare for the next bit, and updates the position.
 * @param this Pointer to the bitBuffer instance.
 * @return
 *         - 0: If the extracted bit is 0.
 *         - 1: If the extracted bit is 1.
 *         - 2: If the end of the buffer is reached (no more bits to read).
 */
uint8_t popBit(bitBuffer* this)
{
    if (this->currentByte > this->lastByte) return 2;
    uint8_t bit = 0;
    if ((this->baseBuffer->dataBuffer[this->currentByte] & MSB) == MSB) bit = 1;
    this->currentShift++;
    if (this->currentShift < BITS_IN_BYTE) {
        this->baseBuffer->dataBuffer[this->currentByte] <<= 1;
        return bit;
    }
    this->currentShift = 0;
    this->currentByte++;
    return bit;
}

/**
 * @brief Extracts 8bit symbol value from buffer
 * @param this Pointer to the bitBuffer instance.
 * @return symbol value
 */
uint8_t popSymbol(bitBuffer* this)
{
    uint8_t symbol = 0;
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t bit = this->popBit(this);
        symbol <<= 1;
        symbol += bit;
    }
    return symbol;
}

/**
 * @brief Appends a byte to the data buffer, reallocating memory if necessary.
 * @param this Pointer to the byteBuffer instance.
 * @param byte The byte to append to the data buffer.
 * @return 
 *         - 0: On successful append.
 *         - 1: If memory reallocation fails.
 */
uint8_t appendByte(byteBuffer* this, uint8_t byte)
{
    this->baseBuffer->dataBuffer[this->currentByte] = byte;
    this->currentByte++;
    // Realocate memory if next appendByte() would exceed current buffer size
    if (this->currentByte % this->baseBuffer->baseBufferSize == 0) 
        if (reallocateBuffer(this->baseBuffer)) return 1;   
    return 0;
}

/** 
 * @brief:  Ask user for path to compressed file and loads data from it to buffer
 * @param:  my - pointer to buffer structure
 * @retval: 0 if succesfully loads data to program memory, 1 in case of memory
 *          allocation failure 2 in case of FILE creation error
 */
uint8_t loadDataFromFile(bitBuffer* this)
{
    // Open file storing compressed data
    char filePath[256];
    printf("Podaj ścieżkę do skompresowanego pliku:\n");
    if (scanf("%250s", filePath) != 1) { 
        printf("Błąd podczas odczytu ścieżki!\n");
        return 1;
    }
    FILE* compressed = fopen(filePath, "rb");
    if (!compressed) {
        printf("Błąd podczas otwierania skompresowanego pliku!\n");
        return 1;
    }
    // Read data chunk by chunk up to the end of file
    uint16_t readBytes;
    while (1) {
        readBytes = fread(&this->baseBuffer->dataBuffer[this->lastByte], 1, CHUNK_SIZE, compressed);
        this->lastByte += readBytes;
        if (readBytes != CHUNK_SIZE) break;
        if (this->lastByte >= MAX_BYTE_NUM) {
            printf("Przekroczono maksymalny rozmiar pliku wejściowego!\n");
            return 1;
        }
        // Realocate memory if next fread() would exceed current buffer size
        if (this->lastByte % this->baseBuffer->baseBufferSize == 0) 
            if (reallocateBuffer(this->baseBuffer)) return 1;      
    }
    // Close file with compressed data
    if (fclose(compressed)) {
        printf("Błąd podczas zamykania skompresowanego pliku!\n");
        return 1;
    }
    return 0;
}

/** 
 * @brief:  Creates buffer instance
 * @param:  None
 * @retval: Pointer to newly created buffer, or NULL on error
 */
baseBuffer* createDataBuffer(uint16_t baseBufferSize)
{
    // Create instance of buffer
    baseBuffer* newBuffer = (baseBuffer*)malloc(sizeof(baseBuffer));
    if (!newBuffer) {
        printf("Błąd podczas alokacji pamięci bufora danych!\n");
        return NULL;
    }
    // Initialize buffer
    newBuffer->baseBufferSize = baseBufferSize;
    newBuffer->multiplier = 0;
    newBuffer->dataBuffer = NULL;
    newBuffer->killMe = freeBaseBuffer;
    if (reallocateBuffer(newBuffer)) {
        newBuffer->killMe(&newBuffer);
        return NULL;
    }
    return newBuffer;
}

byteBuffer* createByteBuffer(uint16_t baseBufferSize)
{
    byteBuffer* newByteBuffer = (byteBuffer*)malloc(sizeof(byteBuffer));
    if (!newByteBuffer) {
        printf("Błąd podczas alokacji pamięci bufora pixeli!\n");
        return NULL;
    }
    newByteBuffer->currentByte = 0;
    newByteBuffer->appendByte = appendByte;
    newByteBuffer->killMe = freeByteBuffer;
    newByteBuffer->baseBuffer = createDataBuffer(baseBufferSize);
    if (!newByteBuffer->baseBuffer) {
        newByteBuffer->killMe(&newByteBuffer);
        return NULL;
    }
    return newByteBuffer;
}

bitBuffer* createBitBuffer(uint16_t baseBufferSize)
{
    bitBuffer* newBitBuffer = (bitBuffer*)malloc(sizeof(bitBuffer));
    if (!newBitBuffer) {
        printf("Błąd podczas alokacji pamięci bufora danych wejściowych!\n");
        return NULL;
    }
    newBitBuffer->lastByte = 0;
    newBitBuffer->currentShift = 0;
    newBitBuffer->currentByte = 0;
    newBitBuffer->popSymbol = popSymbol;
    newBitBuffer->popBit = popBit;
    newBitBuffer->killMe = freeBitBuffer;
    newBitBuffer->baseBuffer = createDataBuffer(baseBufferSize);
    if (!newBitBuffer->baseBuffer) {
        newBitBuffer->killMe(&newBitBuffer);
        return NULL;
    }
    // Load created buffer with data
    if (loadDataFromFile(newBitBuffer)) {
        newBitBuffer->killMe(&newBitBuffer);
        return NULL;
    }
    
    return newBitBuffer;
}

uint8_t writeToFile(byteBuffer* this)
{
    uint8_t fileName[256];
    printf("Wprowadź nazwę dla zdekompresowanego pliku:\n");
    if (scanf("%250s", fileName) != 1) { 
        printf("Error reading input\n");
        return 1;
    }
    // Append ".pgm" extension to the provided file name
    snprintf(fileName, sizeof(fileName), "%s.pgm", fileName);
    FILE* newFile = fopen(fileName,"wb");

    if (newFile == NULL) {
        printf("Błąd podczas tworzenia pliku!\n");
        return 1;
    }

    fprintf(newFile, "P5\n");
    fprintf(newFile, "# Created by IrfanView\n");
    fprintf(newFile, "512 512\n");
    fprintf(newFile, "255\n");

    if (fwrite(this->baseBuffer->dataBuffer, 1, this->currentByte, newFile) != this->currentByte) {
        printf("Błąd podczas zapisywania danych do pliku!\n");
        return 1;
    }
    if (fclose(newFile)) {
        printf("Błąd podczas zamykania zdekompresowanego pliku!\n");
        return 1;
    }
    printf("Zdekompresowany plik zapisany poprawnie\n");
    return 0;
}