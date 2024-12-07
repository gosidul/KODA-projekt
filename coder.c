#include <stdio.h>
#include <stdint.h>

#define SYMBOL_LINK 0xFFFF
#define BASE_SYMBOL_BUFFER  256
#define BASE_NODE_ENTRIES   128
#define BASE_CACHE_ENTRIES  64

uint8_t cacheMemoryBlockMultiplier   = 1;
uint8_t nodesMemoryBlockMultiplier   = 1;
uint8_t recordsMemoryBlockMultiplier = 1;

uint16_t lastSymbolInCache;
uint16_t lastNode;
uint16_t recordsCount;

typedef struct node {
    uint16_t count;
    node* parent;
    node* link0;
    node* link1;
} node;

typedef struct cacheEntry {
    uint16_t value;
    node* nodesAddress;
} cacheEntry;

uint16_t* records = NULL;
cacheEntry* symbolCache = NULL;
node** nodes = NULL;

int main()
{
    manageMemory(0);
    constructTree();
    resolveTree();
    saveCompressedFile();
}

void constructTree() 
{
    uint16_t record = -1;
    while (record == -1) 
        record = readFromStream(); // Functon reads header
    while (record != 0xFFFF) {
        node* symbol = NULL;
        node* root = nodes[0];
        symbol = searchCache(record);
        if (symbol == NULL){
            addNewSymbol(record);
        } 
        root->count++;
        while (symbol != root) {
            symbol = incrementNode(symbol);
        }
        record = readFromStream();
    }
}

/**
  * @brief  Function searches cache for symbol, if mach is found value of symbol is returned
  * @param symbol value of symbol from data stream
  * @retval NULL if no mach is found and symbol address if symbol was previously registered in cache
  */
node* searchCache(uint16_t symbol)
{
    uint16_t temp = 0;

    // Search the cache
    while (temp <= lastSymbolInCache) {
        if (symbolCache[temp].value == symbol)
            return symbolCache[temp].nodesAddress;
        temp++;
    }
    return NULL;
}

/**
  * @brief  Function increments count value of node passed as argument, then searches if it could swap 
  * this node with node that is higher in tree hierarchy. If it can, performs the swap.
  * @param Node Address of node that we will increment
  * @retval address of "parent" node of newly created node, to further tree reorganization
  */
node* incrementNode (node* incrementedNode) 
{
    // Increment count of node
    incrementedNode->count++;

    // Find the index of incrementedNode in the nodes array (for pointer arithmetic)
    node** incrementedNodeAddress = &incrementedNode; 

    // Search for a node higher in the tree hierarchy that could be swapped with the node that was incremented
    node** swapNodeAddress = incrementedNodeAddress;
    while ((*swapNodeAddress)->count > (*swapNodeAddress - 1)->count) 
        swapNodeAddress--;
        
    // If we just increment node value without altering tree hierarchy, exit function
    if (incrementedNodeAddress == swapNodeAddress) 
        return (*swapNodeAddress)->parent;

    // Swap nodes in tree hierarchy...
    node** tempNodeaddress = swapNodeAddress;
    *swapNodeAddress = *incrementedNodeAddress;
    *incrementedNodeAddress = *tempNodeaddress;

    // Swap their parents so they actually change places in the tree structure, not just in the array
    node* swapNode = *swapNodeAddress;
    node* tempParentNode = swapNode->parent;
    swapNode->parent = incrementedNode->parent;
    incrementedNode->parent = tempParentNode;

    return swapNode->parent;
}

/**
  * @brief  Function creates new nodes, one node that represents new  symbol value registered in
  * data stream and one node that will chain together two symbols: one newly added and one selected
  * from the tree (symbol with the lowest "count" value and highest in tree hierarchy)
  * @param newSymbolValue new symbol registered in data stream not present in SymbolCache
  * @retval address of "parent" node of newly created node, to further tree reorganization
  */
node* addNewSymbol(uint16_t newSymbolValue) 
{
// Search for the highest symbol in tree structure with the lowest "count" value 
uint16_t tempAddress = lastNode;
while ((*nodes[tempAddress]).count == (*nodes[tempAddress - 1]).count) 
    tempAddress--;

nodes[++lastNode] = nodes[tempAddress];     // Copy selected symbol struct address to new place in array 
node* selectedSymbolNode = nodes[lastNode]; // Treat its place in array as new address
node* newParentNode = nodes[tempAddress];   // In place of previously reallocated node we will create new one
node* newSymbolNode = nodes[++lastNode];    // Create new node for symbol

// Add newly registered symbol to SymbolCache
symbolCache[lastSymbolInCache].nodesAddress = newSymbolNode;
symbolCache[lastSymbolInCache].value = newSymbolValue;

// Populate struct fields for new symbol
newSymbolNode->count = 1;
newSymbolNode->parent = newParentNode;
newSymbolNode->link0 = SYMBOL_LINK;
newSymbolNode->link1 = SYMBOL_LINK; 

// Create new node in "nodes" array that will have reallocated and new symbol set 
// as its children, and "count" as sum of children "count" values, swap parents of 2 nodes
newParentNode->count = selectedSymbolNode->count + newSymbolNode->count;
newParentNode->link0 = newSymbolNode;
newParentNode->link1 = selectedSymbolNode;
newParentNode->parent = selectedSymbolNode->parent; 
selectedSymbolNode->parent = newParentNode;

// Return PARENT of added "node" to further tree reorganization
return newParentNode->parent;
}

/**
  * @brief  Function checks if it is necesarry to reallocate memory and calls manageMemory() if so
  * it should be called each iteration inside main loop after we add new entry
  * @param None
  * @retval None
  */
void memoryCheck() {
    if (recordsCount + 1 >= BASE_SYMBOL_BUFFER * recordsMemoryBlockMultiplier) 
        manageMemory(1);
    if ((lastSymbolInCache + 1) >= BASE_CACHE_ENTRIES * cacheMemoryBlockMultiplier) 
        manageMemory(2);
    if ((lastNode + 2) >= BASE_NODE_ENTRIES * nodesMemoryBlockMultiplier) 
        manageMemory(3);
}

/**
  * @brief Function responsible for memory management. Each mode represents different arrays to reallocate if necesarry
  * @param None
  * @retval None
  */
void manageMemory(uint16_t mode)
{
    switch (mode) {
    // Initialization    
    case 0:
        nodes = (node**)malloc(BASE_NODE_ENTRIES * sizeof(node*));
        node* nodesMemoryBlock = (node*)malloc(BASE_NODE_ENTRIES * sizeof(node));
        for (int i = 0; i < BASE_NODE_ENTRIES; i++)
            nodes[i] = &nodesMemoryBlock[i];

        records = (uint16_t*)malloc(BASE_SYMBOL_BUFFER * sizeof(uint16_t));
        symbolCache = (cacheEntry*)malloc(BASE_CACHE_ENTRIES * sizeof(cacheEntry));

        node* root = nodes[0];
        node* symbol1 = nodes[1];
        node* symbol2 = nodes[2];
        cacheEntry* cache0 = &symbolCache[0];
        cacheEntry* cache1 = &symbolCache[1];

        root->count = 2;
        root->parent = root;   // Root
        root->link0 = symbol1;
        root->link1 = symbol2;

        symbol1->count = 1;
        symbol1->parent = root;
        symbol1->link0 = SYMBOL_LINK;    // Symbol
        symbol1->link1 = SYMBOL_LINK;

        symbol2->count = 1;
        symbol2->parent = root;
        symbol2->link0 = SYMBOL_LINK;    // Symbol
        symbol2->link1 = SYMBOL_LINK;

        cache0->nodesAddress = symbol1;
        cache0->value = 0;

        cache1->nodesAddress = symbol2;
        cache1->value = 1;

        lastSymbolInCache = 1;
        lastNode = 2;
        recordsCount = 0;
        break;

    // Realloc for records    
    case 1:
        uint16_t oldSizeOfRecords = BASE_SYMBOL_BUFFER * recordsMemoryBlockMultiplier * sizeof(uint16_t);
        recordsMemoryBlockMultiplier++;
        uint16_t *temp = (uint16_t*)malloc(BASE_SYMBOL_BUFFER * recordsMemoryBlockMultiplier * sizeof(uint16_t));
        memcpy(temp, records, oldSizeOfRecords);
        free(records);
        records = temp;
        break;

    // Realloc for symbolCache
    case 2: 
        uint16_t oldSizeOfCashe = BASE_CACHE_ENTRIES * cacheMemoryBlockMultiplier * sizeof(cacheEntry);
        cacheMemoryBlockMultiplier++;
        cacheEntry* temp = (cacheEntry*)malloc(BASE_CACHE_ENTRIES * cacheMemoryBlockMultiplier * sizeof(cacheEntry));
        memcpy(temp, symbolCache, oldSizeOfCashe);
        free(symbolCache);
        symbolCache = temp;
        break;

    // Realloc for nodes array
    case 3:
        uint16_t numberOfNodes = BASE_NODE_ENTRIES * nodesMemoryBlockMultiplier;
        uint16_t oldSizeOfNodes = numberOfNodes * sizeof(node*);
    
        // Increment multiplier for the expanded node array
        nodesMemoryBlockMultiplier++;

        // Allocate new memory blocks
        node** temp = (node**)malloc(BASE_NODE_ENTRIES * nodesMemoryBlockMultiplier * sizeof(node*));
        node* newNodesMemoryBlock = (node*)malloc(BASE_NODE_ENTRIES * sizeof(node));

        // Copy old pointers into the new array and initialize pointers for new nodes
        memcpy(temp, nodes, oldSizeOfNodes);
        for (int i = numberOfNodes; i < BASE_NODE_ENTRIES * nodesMemoryBlockMultiplier; i++)
            temp[i] = &newNodesMemoryBlock[i - numberOfNodes];

        // Free old array and assign new memory
        free(nodes);
        nodes = temp;
        break;

    // Free all data
    case 4:
        for (int i = 0; i < BASE_NODE_ENTRIES * nodesMemoryBlockMultiplier; i++) 
            free(nodes[i]);
        free(nodes);
        free(symbolCache);
        free(records);
        default:
    }
}

/**
  * @brief Function responsible for loading data to records array, that means reading header, 
  * seting record counter value (assign value from header), and making sure that we will end 
  * reading data in right moment (we read all values -> reach counter = 0)
  * @param None
  * @retval Function returns record value if valid record was read from file,
  *         0xFFFF if EOF is reached, -1 -> read parameter is not valid
  */
uint16_t readFromStream() {
    //TODO
}

/**
  * @brief Function creates bit representation for each symbol present in symbolCache
  * @param None
  * @retval 
  */
void resolveTree() {
    //TODO
}

/**
  * @brief Function saves compressed stream to file with bit representation table
  * @param None
  * @retval 
  */
void saveCompressedFile() {
    //TODO
}
