#include "treeOperations.h"
#include "fileOperations.h"

#define BASE_NODES_ENTRIES 16
#define BASE_CACHE_ENTRIES 4
#define ROW_SIZE 512
#define COL_SIZE 512
#define BITS_IN_BYTE 8

/**
 * @brief  Retrieves the next record from the `records` matrix in a sequential manner.
 *         If the end of the current row is reached, it moves to the next row. If the 
 *         end of the matrix is reached, the matrix is cleared, and the function stops.
 *
 * @param  my: Pointer to the `records` struct containing the matrix, 
 *                  current row and column positions, and associated metadata.
 *
 * @return The value of the next record in the matrix as a `uint8_t`. 
 *         Returns 0 if the matrix is fully read.
 */
uint8_t popRecord(records* my)
{
    uint8_t record = my->matrix[my->currentDimension[0]][my->currentDimension[1]++];
    if (my->currentDimension[1] >= my->matrixDimension[1]) {
        my->currentDimension[1] = 0;
        my->currentDimension[0]++;
    }
    if (my->currentDimension[0] >= my->matrixDimension[0]) {
        my->currentDimension[0] = 0;
        for (int i = 0; i < my->matrixDimension[0]; i++){
            free(my->matrix[i]);
            my->matrix[i] = NULL;
        }
        free(my->matrix);
        my->matrix = NULL;
    } 
    return record;
}

/**
 * @brief  Expands the tree by reallocating memory for the array of node pointers and adding a new block of nodes.
 *         It calculates the current tree size, allocates memory for the expanded tree, and copies the existing node 
 *         pointers to the new memory. The function ensures that the old memory is freed after the expansion.
 * @param  None
 * @retval 0 if successfully allocated new space, 1 if memory allocation fails
 */
uint8_t expandTree(handler* my)
{
    uint16_t currentNumberOfNodes = my->tree.baseNumberOfNodes * my->tree.memoryBlockMultiplier;
    my->tree.memoryBlockMultiplier++;
    node** newNodes = malloc(my->tree.baseNumberOfNodes * my->tree.memoryBlockMultiplier * sizeof(node*));
    if (!newNodes) {
        printf("Failed expanding tree");
        return 1;
    }
    *(newNodes + currentNumberOfNodes) = (node*)malloc(my->tree.baseNumberOfNodes * sizeof(node));
    if (!newNodes[currentNumberOfNodes]) {
        free(newNodes);
        printf("Failed expanding tree");
        return 1;
    }
    for (uint16_t i = 1; i < my->tree.baseNumberOfNodes; i++)
        *(newNodes + currentNumberOfNodes + i) = *(newNodes + currentNumberOfNodes) + i;
    if (my->tree.nodes) {
        memcpy(newNodes, my->tree.nodes, currentNumberOfNodes * sizeof(node*));
        free(my->tree.nodes);
    }
    my->tree.nodes = newNodes;
    return 0;
}

/**
  * @brief  Expands cache. Function calculates the current cache size, allocates memory for the expanded cache,
  *         copies the existing cache entries to the new memory. The function ensures that the old memory
  *         is freed after the expansion.
  * @param  None
  * @retval 0 if successfully allocated new space, 1 if memory allocation fails
  */
uint8_t expandCache(handler* my)
{
    uint16_t oldSizeOfCache = my->cache.baseNumberOfEntries * my->cache.memoryBlockMultiplier * sizeof(cacheEntry);
    cacheEntry* tempCache = (cacheEntry*)malloc(my->cache.baseNumberOfEntries * ++my->cache.memoryBlockMultiplier * sizeof(cacheEntry));
    if(!tempCache) {
        printf("Failed expanding cache");
        return 1;
    }
    if (my->cache.symbolCache) {
        memcpy(tempCache, my->cache.symbolCache, oldSizeOfCache);
        free(my->cache.symbolCache);
    }
    my->cache.symbolCache = tempCache;
    return 0;
}

/**
  * @brief  Calculates the path as bit sequence from a given node to the root of the tree.
  *         The path is stored in bits, where each bit represents whether the node is a
  *         left (0) or right (1) child in the tree. The path is represented from node to
  *         root. Function register each nodes connection by incrementing mask, so leading
  *         zeros are not omtied. Then modifies the path if the node is the last node in
  *         the tree by adding constant symbol value registered from records, and writes
  *         the final bit sequence to the file.
  * @param  node Pointer to the node for which the bit sequence is appended to the file.
  * @retval None
  */
void appendPathToFile(handler* my, node* _node)
{
    node* node = _node;
    uint32_t bits = 0;
    uint8_t mask = 0;

    while (node->parent) {
        mask++;
        if (node == (node->parent)->link1)
            bits = (bits << 1) + 1;
        else
            bits = bits << 1;
        node = node->parent;
    }

    if (_node->positionInTree == my->tree.lastNode) {
        bits = (bits << BITS_IN_BYTE) + my->records.matrix[my->records.currentDimension[0]][my->records.currentDimension[1]];
        mask += BITS_IN_BYTE;
    }

    if (writeToFile(my->compressedFile, &my->bitBuffer, bits, mask)) printf("ERROR: Cannot write to file!");
}

/**
 * Allocates and initializes a new `handler` structure, including its internal 
 * components (`records`, `cache`, and `tree`).
 *
 * @return A pointer to the newly created `handler` structure. 
 *         Returns `NULL` if memory allocation fails.
 *
 **/
handler* createHandler() {
    // Dynamically allocate memory for the handler
    handler* _handler = malloc(sizeof(handler));
    if (!_handler) {
        return NULL; // Handle allocation failure
    }

    // Initialize internal structs
    _handler->bitBuffer.buffer = 0;
    _handler->bitBuffer.freeBits = sizeof(_handler->bitBuffer.buffer) * 8;

    _handler->compressedFile = NULL;

    _handler->records.matrix = NULL;
    _handler->records.currentDimension[0] = 0; // rows
    _handler->records.currentDimension[1] = 0; // columns
    _handler->records.matrixDimension[0] = 0;
    _handler->records.matrixDimension[1] = 0;
    _handler->records.popRecord = popRecord;

    _handler->cache.symbolCache = NULL;
    _handler->cache.baseNumberOfEntries = BASE_CACHE_ENTRIES;
    _handler->cache.memoryBlockMultiplier = 0;
    _handler->cache.lastCacheEntry = 0;

    _handler->tree.nodes = NULL;
    _handler->tree.baseNumberOfNodes = BASE_NODES_ENTRIES;
    _handler->tree.memoryBlockMultiplier = 0;
    _handler->tree.lastNode = 0;

    return _handler;
}

/**
  * @brief  Initialize tree by loading records to records buffer, allocating memory for tree and cache
  *         and creating base tree consistiong of root, first symbol from stream and NewSymbol node
  * @param  None
  * @retval 0 if successfully created tree and buffer, 1 otherwise
  */
uint8_t initialize(handler* my)
{
    // Create file for compressed data
    my->compressedFile = createCompressedFile();

    // Allocate memory for struct fields
    if (expandTree(my)) return 1;
    if (expandCache(my)) return 1;

    // Allocate memory for records matrix and populate it with data
    if (readDataFromFile(&my->records)) return 1;
    if (!my->records.matrix) return 1;

    // Declare first entries for cache and first nodes in tree
    // and populate cache and nodes entries fields
    
    node* root =  my->tree.nodes[my->tree.lastNode];       // Root -> position in tree = "0"
    node* symbol0 = my->tree.nodes[++my->tree.lastNode];
    node* newSymbol = my->tree.nodes[++my->tree.lastNode];

    cacheEntry* cachedSymbol0 = &my->cache.symbolCache[my->cache.lastCacheEntry];
    cachedSymbol0->symbolValue = my->records.popRecord(&my->records);
    cachedSymbol0->treeAddress = symbol0;

    root->count = 1;
    root->parent = NULL;      // Root -> No parent
    root->link0 = symbol0;
    root->link1 = newSymbol;  // Node -> internal node, links != NULL
    root->positionInTree = 0;

    symbol0->count = 1;
    symbol0->parent = root;
    symbol0->link0 = NULL;    // Symbol -> external node, links == NULL
    symbol0->link1 = NULL;
    symbol0->positionInTree = 1;

    newSymbol->count = 0;
    newSymbol->parent = root;
    newSymbol->link0 = NULL;
    newSymbol->link1 = NULL;
    newSymbol->positionInTree = my->tree.lastNode;   // NewSymbol -> position in tree = tree.lastNode

    if (writeToFile(my->compressedFile, &my->bitBuffer, cachedSymbol0->symbolValue, 9)) printf("ERROR: Cannot write to file!");

    return 0;
}

/**
  * @brief  Function checks if it is necesarry to reallocate memory and if so, calls expandTree() and 
  *         expandCache().
  * @param None
  * @retval 1 if failed to reallocate memory, 0 otherwise
  */
uint8_t memoryCheck(handler* my)
{
    if ((my->cache.lastCacheEntry + 1) >= my->cache.baseNumberOfEntries * my->cache.memoryBlockMultiplier)
        if(expandCache(my)) return 1;
    if ((my->tree.lastNode + 2) >= my->tree.baseNumberOfNodes * my->tree.memoryBlockMultiplier) 
        if (expandTree(my)) return 1;
    return 0;
}

/**
  * @brief  Function search for a node highest in the tree hierarchy on the same "level" that means
  *         with the same "count" value that could be swapped with the node that we will increment 
  *         in this function call, if that is true, performs swap and returns address of next node
  * @param  Node Address of node that we will increment
  * @retval address of "parent" node of newly created node after swap, to further tree reorganization
  */
node* rearrangeTree(handler* my, node* _node)
{

    // Search for a node highest in the tree hierarchy on the same "level" -> with the same "count"
    // value that could be swapped with the node that we will increment in this function call
    uint16_t tempAddress = _node->positionInTree;
    node* incrementedNode = my->tree.nodes[tempAddress];

    while (incrementedNode->count == my->tree.nodes[tempAddress - 1]->count) 
        tempAddress--;
    
    incrementedNode->count++;
    node* nodeToSwap = my->tree.nodes[tempAddress];

    // If we just increment node value without altering tree hierarchy, return parent
    if (_node->positionInTree == tempAddress || nodeToSwap == _node->parent) 
        return incrementedNode->parent;

    // Swap nodes addresses in tree
    my->tree.nodes[nodeToSwap->positionInTree] = incrementedNode;
    my->tree.nodes[_node->positionInTree] = nodeToSwap;

    // And their localizers (position in tree read from node perspective)
    tempAddress = nodeToSwap->positionInTree;
    nodeToSwap->positionInTree = incrementedNode->positionInTree;
    incrementedNode->positionInTree = tempAddress;

    // Update link of Parent nodes of swapped symbols
    if ((nodeToSwap->parent)->link1 == nodeToSwap)
        (nodeToSwap->parent)->link1 = incrementedNode;
    else (nodeToSwap->parent)->link0 = incrementedNode;

    if ((incrementedNode->parent)->link1 == incrementedNode)
        (incrementedNode->parent)->link1 = nodeToSwap;
    else (incrementedNode->parent)->link0 = nodeToSwap;

    // Swap their parents links so they actually change places in the tree structure
    node* tempNode = nodeToSwap->parent;
    nodeToSwap->parent = incrementedNode->parent;
    incrementedNode->parent = tempNode;

    return incrementedNode->parent;
}

/**
  * @brief  Function creates 2 new nodes, one node that represents new symbol for value
  *         registered in data stream and one parent node that will chain together two
  *         symbols: one newly added and one existing symbol that parent node overwrite
  *         in tree array.
  * @param  newValue new symbol registered in data stream (records) not present in SymbolCache
  * @retval address of "parent" node of newly created parent node, to further tree reorganization
  */
node* addNewSymbol(handler* my, uint8_t newValue)
{
// Create new parent node in place of newSymbolNode
node* newParentNode = my->tree.nodes[my->tree.lastNode];

// Create new node for symbol from stream
node* symbolFromStream = my->tree.nodes[++my->tree.lastNode];

// Populate struct fields for new symbolFromStream
symbolFromStream->count = 1;
symbolFromStream->parent = newParentNode;
symbolFromStream->link0 = NULL;
symbolFromStream->link1 = NULL;
symbolFromStream->positionInTree = my->tree.lastNode;

// Create new node for newSymbolNode
node* newSymbolNode = my->tree.nodes[++my->tree.lastNode];

// Populate struct fields for newSymbolNode
newSymbolNode->count = 0;
newSymbolNode->parent = newParentNode;
newSymbolNode->link0 = NULL;
newSymbolNode->link1 = NULL;
newSymbolNode->positionInTree = my->tree.lastNode;

// Populate struct fields for newParentNode,
// parent and positionInTree is already set
newParentNode->count = 1;
newParentNode->link0 = symbolFromStream;
newParentNode->link1 = newSymbolNode;

// Add newly registered symbol to SymbolCache
my->cache.symbolCache[++my->cache.lastCacheEntry].treeAddress = symbolFromStream;
my->cache.symbolCache[my->cache.lastCacheEntry].symbolValue = newValue;

// Return parent of newParentNode for further tree reorganization
return newParentNode->parent;
}

/**
  * @brief   Function searches cache for symbol, if match is found, path of this
  *          symbol is appended to file and address of this symbol is returned.
  *          Otherwise new node for newly registered symbol is created and tree
  *          is reorganized, node returned from addNewSymbol() is returned. Before
  *          adding nodes to tree, function checks if there is available memory to
  *          complete this operation, tries to reallocate memory if necesarry.
  * @param   symbol value of symbol from data stream
  * @retval  symbol address to further tree reorganization. NULL if error
  */
node* searchCache(handler* my, uint8_t symbol)
{
    if (memoryCheck(my)) return NULL;
    for(uint16_t i = 0; i <= my->cache.lastCacheEntry; i++)
        if (my->cache.symbolCache[i].symbolValue == symbol) {
            appendPathToFile(my, my->cache.symbolCache[i].treeAddress);
            return my->cache.symbolCache[i].treeAddress;
        }
    // If no match found append NewSymbol
    appendPathToFile(my, my->tree.nodes[my->tree.lastNode]);
    return addNewSymbol(my, symbol);
}

uint8_t constructTree(handler* my)
{
    node* symbol;
    while (my->records.matrix) {
        symbol = searchCache(my, my->records.popRecord(&my->records));
        if (!symbol) return 1;
        my->tree.nodes[0]->count++;
        while (symbol->parent != NULL)
            symbol = rearrangeTree(my, symbol);
    }
    if (writeRemainingBits(&my->bitBuffer, my->compressedFile)) return 1;
    return 0;
}
