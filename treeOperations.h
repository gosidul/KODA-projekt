#ifndef TREE_OPERATIONS_H
#define TREE_OPERATIONS_H

// #include "fileOperations.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * Represents buffer to store data before writing it to file.
 * @buffer: Variable that stores appended bit paths and symbol values
 * @freeBits: Represent number of free bits left in buffer
 */
typedef struct dataBuffer {
    uint64_t buffer;
    uint8_t freeBits;
} dataBuffer;

/**
 * Represents a node in the Huffman tree.
 * @parent: Pointer to the parent node.
 * @link0: Pointer to the child node representing a "0" in the bit stream path.
 * @link1: Pointer to the child node representing a "1" in the bit stream path.
 * @positionInTree: distance from root node in nodes array
 * @count: Number of occurrences if the node represents a symbol,
 *         or the sum of the counts of its child nodes.
 */
typedef struct node {
    struct node* parent;
    struct node* link0;
    struct node* link1;
    uint16_t positionInTree;
    uint32_t count;
} node;

/**
 * Represents a tree structure containing nodes and metadata for memory management.
 * @nodes: Array of pointers to node structs, used to create the tree.
 * @baseNumberOfNodes: Base size of memory chunk for nodes.
 * @memoryBlockMultiplier: Number of memory blocks allocated for nodes.
 * @lastNode: Position of the last node in the array, used for tracking new symbols.
 */
typedef struct tree {
    struct node** nodes;
    uint8_t baseNumberOfNodes;
    uint8_t memoryBlockMultiplier;
    uint16_t lastNode;
} tree;

/**
 * Represents a single entry in the symbol cache.
 * @treeAddress: Pointer to a node in the tree structure representing the symbol.
 * @symbolValue: Value of the symbol associated with this cache entry.
 */
typedef struct cacheEntry {
    struct node* treeAddress;
    uint8_t symbolValue;
} cacheEntry;

/**
 * Represents a cache for storing symbol recorded in data stream.
 * @symbolCache: Array of cacheEntry structs for symbol lookup.
 * @lastSymbolPath: Last processed symbol's path in the tree.
 * @baseNumberOfEntries: Base size for the memory chunk of the cache.
 * @memoryBlockMultiplier: Number of memory blocks allocated for the cache.
 * @lastCacheEntry: Position of the last cache entry in the symbolCache array.
 */
typedef struct cache {
    struct cacheEntry* symbolCache;
    uint8_t baseNumberOfEntries;
    uint8_t memoryBlockMultiplier;
    uint16_t lastCacheEntry;
} cache;

/**
 * Manages a 2D matrix of int8_t records with sequential access capabilities.
 * @matrix: Dynamically allocated 2D array of uint8_t values.
 * @currentRow: Current row index being accessed in the matrix.
 * @currentColumn: Current column index being accessed in the current row.
 * @popRecord: Function pointer for retrieving the next record in sequence.
 */
typedef struct records {
    uint8_t** matrix;
    uint16_t currentDimension[2];
    uint16_t matrixDimension[2];
    uint8_t (*popRecord)(struct records*);
} records;

/**
 * Represents the main structure for managing the Huffman codec, 
 * including input records, symbol cache, and the Huffman tree.
 * @bitBuffer: A `dataBuffer` structure containing bits ready to be written to file
 * @compressedFile: A pointer to `FILE` object containing information used while writing data
 * @records: A `records` structure for managing the 2D matrix of input data.
 * @cache: A `cache` structure for storing symbol information and lookup paths.
 * @tree: A `tree` structure representing the Huffman tree for encoding and decoding.
 */
typedef struct handler {
    dataBuffer bitBuffer;
    FILE* compressedFile;
    records records;
    cache  cache;
    tree tree;
} handler;

/**
 * Allocates and initializes a new `handler` structure, including its internal 
 * components (`records`, `cache`, and `tree`).
 *
 * @return A pointer to the newly created `handler` structure. 
 *         Returns `NULL` if memory allocation fails.
 **/
handler* createHandler();

/**
  * @brief  Initialize tree by loading records to records buffer, allocating memory for tree and cache
  *         and creating base tree consistiong of root, first symbol from stream and NewSymbol node
  * @param  my A pointer to handler struct containing information about tree, cache and records
  * @retval 0 if successfully created tree and buffer, 1 otherwise
  */
uint8_t initialize(handler* my);

uint8_t constructTree(handler* my);

#endif // TREE_OPERATIONS
