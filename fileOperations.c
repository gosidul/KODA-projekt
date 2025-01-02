#include "fileOperations.h"

/**
  * @brief  Opens a file for binary reading operations.
  * @param  None
  * @retval Pointer to the opened FILE object for read operations,
  *         or NULL if an error occurs.
  */
FILE* openFile()
{
    uint8_t filePath[512];

    printf("\nPlease enter valid path to file to compress:\n");

    //strcpy((char *)filePath, "C:\\Cpp\\projects\\CODEC\\chronometer.pgm"); //              <--- for debug, clear later: #include <string.h> // For strcpy

    if (scanf("%511s", filePath) != 1) { 
        printf("\nError: Invalid input. Please try again.\n");
        return NULL;
    }

    // Open file with provided name on binary read mode
    FILE* fileToCompress = fopen(filePath,"rb");

    if (fileToCompress == NULL) {
        printf("\nError: Could not open file");
        return NULL;
    }
    // Return the FILE pointer
    return fileToCompress;
}

FILE* createCompressedFile()
{
    uint8_t fileName[256];

    printf("\nPlease enter valid file name for compressed data:\n");

    //strcpy((char *)fileName, "Compressed"); //                                              <--- for debug, clear later: #include <string.h> // For strcpy

    if (scanf("%250s", fileName) != 1) { 
        printf("\nError reading input.");
        return NULL;
    }

    // Append ".bin" extension to the provided file name
    snprintf(fileName, sizeof(fileName), "%s.bin", fileName);

    // Open file with provided name on binary write mode
    // fopen allocates memory automatically
    FILE* compressedFile = fopen(fileName,"wb");

    if (compressedFile == NULL) {
        printf("\nError opening file");
        return NULL;
    }
    return compressedFile;
}

uint8_t readDataFromFile(records* my)
{
    uint8_t headerLine[64];
    uint8_t headerLines = 0;
    FILE* file = openFile();
    if (!file) return 1;

    // Each PGM Image File must consist of 3 header lines: signature, rows and cols, max grey level
    while (headerLines < 3) {
        if (!fgets(headerLine, sizeof(headerLine), file)) {
            printf("Error: Unexpected end of file while reading header.\n");
            return 1;
        }
        // Skip all comments
        if (headerLine[0] == '#') 
            continue; 
        // Read and assign number of rows and columns to fields in records
        if (headerLines == 1) {
            uint8_t j = 0;
            for (uint8_t i = 0; headerLine[i] != '\n'; i++) {
                if (headerLine[i] == ' ') { 
                    i++;
                    j++;
                }
                my->matrixDimension[j] = my->matrixDimension[j] * 10 + (headerLine[i] - '0');
            }
        }
        headerLines++;    
    }

    // Allocate memory for the records buffer: IMAGE_ROWS x IMAGE_COLS
    my->matrix = (uint8_t**)malloc(my->matrixDimension[0] * sizeof(uint8_t*));
    for (int i = 0; i < my->matrixDimension[0]; i++) {
        my->matrix[i] = (uint8_t*)malloc(my->matrixDimension[1] * sizeof(uint8_t));
    }

    // Read pixel data row by row
    for (int i = 0; i < my->matrixDimension[0]; i++) {
        size_t bytesRead = fread(my->matrix[i], 1, my->matrixDimension[1], file);

        if (bytesRead != my->matrixDimension[1]) {
            if (feof(file)) {
                printf("Error: Unexpected end of file at row %d.\n", i);
            } else {
                printf("Error: Failed to read data from row %d.\n", i);
            }

            // Free allocated memory if error detected
            for (int row = 0; row < my->matrixDimension[0]; row++) {
                free(my->matrix[row]);
            }
            free(my->matrix);
            return 1;
        }
    }
    printf("File read correctly\n");
    fclose(file);

    return 0;
}

uint8_t writeToFile(FILE* compressedFile, dataBuffer* my, uint32_t input, uint8_t count)
{
    // Move input data to bigger variable to match buffer.buffer size
    // Prepare data to append it to the buffer by calculating bitwise shift to match buffer end
    uint64_t data = input;
    int8_t shift = my->freeBits - count;

    // If overflow happened buffer.buffer can't fit all new bits
    if (shift < 0) {
        // Calculate shift for these bits that will fit in buffer by changing sign of shift
        // only bits that will fit into buffer remain in variable, since we are using right shift
        my->buffer += data >> -shift;

        // Write data to a file from a buffer, return "0" if error occurs
        my->buffer = _byteswap_uint64(my->buffer); // swap endiannes if necesarry
        if (fwrite(&my->buffer, BUFFER_BYTE_LENGTH, 1, compressedFile) != 1) return 1;

        // Prepare buffer for new data clearing its bit fields
        my->buffer = 0;

        // Set freeBits counter to max value and update count 
        // to match number of bits that are not added yet
        count = count - my->freeBits;
        my->freeBits = BUFFER_BIT_LEN;

        // Calculate new shift 
        shift = my->freeBits - count;
    }
    my->buffer += data << shift;
    my->freeBits = my->freeBits - count;
    return 0;
}

uint8_t writeRemainingBits(dataBuffer* my, FILE* compressedFile)
{
    if (my->freeBits == BUFFER_BIT_LEN) {
        if (fclose(compressedFile)) {
            printf("Error: Error during closing file\n");
            return 1;
        }
        printf("File successfully written and closed\n");
        return 0;
    }

    my->buffer = my->buffer >> my->freeBits;
    my->buffer = _byteswap_uint64(my->buffer);

    if (fwrite(&my->buffer, BUFFER_BYTE_LENGTH, 1, compressedFile) != 1) {
        printf("Error: Cannot write remaining bits to file\n");
        return 1;
    }
    if (fclose(compressedFile)) {
        printf("Error: Error during closing file\n");
        return 1;
    }
    printf("File successfully written and closed\n");
    return 0;
}
