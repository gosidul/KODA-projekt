#ifndef DECODER_OPERATIONS_H
#define DECODER_OPERATIONS_H

#define BASE_NODES_ENTRIES 32
#define BASE_CACHE_ENTRIES 16
#define BASE_ARRAY_ENTRIES 8
#define BITS_IN_BYTE 8

#include "bitOperations.h"

/**
 * @brief:  Represents a node in the Huffman tree.
 * @parent: Pointer to the parent node.
 * @link0: Pointer to the child node representing a "0" in the bit stream path.
 * @link1: Pointer to the child node representing a "1" in the bit stream path.
 * @value: Value of node which is appended to decompressed data.
 * @positionInTree: distance from root node in nodes array.
 * @count: Number of occurrences if the node represents a symbol,
 *         or the sum of the counts of its child nodes if it is internal node.
 */
typedef struct node {
    struct node* parent;
    struct node* link0;
    struct node* link1;
    uint8_t value;
    uint16_t positionInTree;
    uint32_t count;
} node;

/**
 * @brief: Represents a tree structure containing nodes and metadata for memory management.
 * @nodes: Array of pointers to node structs, used to create the tree.
 * @memoryPointers: Array of pointers to node structs that stores information about memory.
 * @input: Struct containing bit value read from compressed file
 * @output: Struct containing byte value of pixels, used for creating output file
 * @baseNumberOfNodes: Base size of memory chunk for nodes.
 * @memoryBlockMultiplier: Number of memory blocks allocated for nodes.
 * @lastNode: Position of the last node in the array, used for tracking new symbols.
 */
typedef struct tree {
    struct node** nodes;
    struct node** memoryPointers;
    struct bitBuffer* input;
    struct byteBuffer* output;
    uint8_t baseNumberOfNodes;
    uint8_t memoryBlockMultiplier;
    uint16_t lastNode;
} tree;

/**
  * @brief: Initialize tree by allocating memory for tree and creating base tree
  *         consisting of root, first symbol from stream and NewSymbol node.
  * @param  pointer to tree struct
  * @retval 0 if successfully created tree, 1 otherwise
  */
tree* createTree();

/**
  * @brief: Constructs the Huffman tree and stores decompressed data in buffer.
  *         Iterates through input bits, updates the tree structure, and decodes data.
  * @param  pointer to the tree struct containing the Huffman tree.
  * @retval 0 if the tree is successfully constructed and data decompressed, 1 on error
  */
uint8_t constructTree(tree*);

/**
  * @brief  Frees memory used by structs
  * @param  pointer to handler struct containing instances of: dataBuffer, records, cache, tree
  *         and compressedFile pointer
  * @retval None
  */
void freeAlocatedMemory(tree**);

#endif