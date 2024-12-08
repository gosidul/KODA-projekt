#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <cstring>

#define IMAGE_ROW_WIDTH     512
#define BASE_SYMBOL_BUFFER  262144
#define BASE_NODE_ENTRIES   128
#define BASE_CACHE_ENTRIES  64

uint8_t cacheMemoryBlockMultiplier   = 1;
uint8_t nodesMemoryBlockMultiplier   = 1;

uint16_t lastSymbolInCache;
uint16_t lastNode;
uint32_t recordsCount;

int debug = 0;

typedef struct node {
    node* parent;
    node* link0;
    node* link1;
    uint16_t positionInNodes;
    uint32_t count;
} node;

typedef struct cacheEntry {
    int8_t symbolValue;
    node* nodesAddress;
} cacheEntry;

typedef struct symbol {
    uint32_t bits;
    uint32_t mask;
} symbol;

symbol* symbols = NULL;
int8_t* records = NULL;
cacheEntry* symbolCache = NULL;
node** nodes = NULL;

FILE *fileToCompress = NULL;
FILE *compressedFile = NULL;

void manageMemory(uint16_t mode);
int openFile();
int readDataFromFile();
void constructTree();
void resolveTree();
void saveCompressedFile();
node* searchCache(int8_t symbol);
node* addNewSymbol(int8_t newSymbolValue);
node* incrementNode (node* incrementedNode);
void memoryCheck();

int main()
{
    manageMemory(0);
    if (openFile())
        return 0;
    if (readDataFromFile())
        return 0;
    constructTree();
    resolveTree();
    saveCompressedFile();
    if (fileToCompress) 
        fclose(fileToCompress);
    if (compressedFile) 
        fclose(fileToCompress);
    return 1;
}

void constructTree()
{
    int8_t record = records[recordsCount];
    node* root = nodes[0];
    while (recordsCount < BASE_SYMBOL_BUFFER) {      // Function reads symbol values until EOF is reached
        memoryCheck();
        node* symbol = NULL;
        symbol = searchCache(record);
        if (symbol == NULL)
            symbol = addNewSymbol(record);
        root->count++;
        while (symbol != root)
            symbol = incrementNode(symbol);
        record = records[++recordsCount];
    }
}

/**
  * @brief  Function increments count value of node passed as argument, then searches if it could swap 
  * this node with node that is higher in tree hierarchy. If it can, performs the swap.
  * @param Node Address of node that we will increment
  * @retval address of "parent" node of newly created node, to further tree reorganization
  */
node* incrementNode (node* incrementedNode)
{
    // Search for a node higher in the tree hierarchy that could be swapped with the node that we will increment
    uint16_t tempAddress = incrementedNode->positionInNodes;

    while (nodes[tempAddress]->count == nodes[tempAddress - 1]->count) 
        tempAddress--;

    node* swapedNode = nodes[tempAddress];

    // Increment count of node
    incrementedNode->count++;
      
    // If we just increment node value without altering tree hierarchy, exit function
    if (tempAddress == incrementedNode->positionInNodes) 
        return incrementedNode->parent;

    //Swap nodes in tree hierarchy...
    nodes[swapedNode->positionInNodes] = incrementedNode;
    nodes[incrementedNode->positionInNodes] = swapedNode;

    tempAddress = swapedNode->positionInNodes;
    swapedNode->positionInNodes = incrementedNode->positionInNodes;
    incrementedNode->positionInNodes = tempAddress;

    // Update link of Parent nodes of swapped symbols
    if ((swapedNode->parent)->link1 == swapedNode)
        (swapedNode->parent)->link1 = incrementedNode;
    else (swapedNode->parent)->link0 = incrementedNode;

    if ((incrementedNode->parent)->link1 == incrementedNode)
        (incrementedNode->parent)->link1 = swapedNode;
    else (incrementedNode->parent)->link0 = swapedNode;

    // Swap their parents so they actually change places in the tree structure
    node* tempNode = swapedNode->parent;
    swapedNode->parent = incrementedNode->parent;
    incrementedNode->parent = tempNode;

    return incrementedNode->parent;
}

/**
  * @brief  Function creates new nodes, one node that represents new  symbol value registered in
  * data stream and one node that will chain together two symbols: one newly added and one selected
  * from the tree (symbol with the lowest "count" value and highest in tree hierarchy)
  * @param newSymbolValue new symbol registered in data stream not present in SymbolCache
  * @retval address of "parent" node of newly created node, to further tree reorganization
  */
node* addNewSymbol(int8_t newSymbolValue)
{
// Search for the highest symbol in tree structure with the lowest "count" value 
uint16_t tempAddress = lastNode;
while ((*nodes[tempAddress]).count == (*nodes[tempAddress - 1]).count) 
    tempAddress--;

node* newParentNode = nodes[++lastNode];    // Copy pointer to newParentNode variable
nodes[lastNode] = nodes[tempAddress];       // Put selected symbol struct address to new place in array 
nodes[tempAddress] = newParentNode;         // In place of previously reallocated node we will put copied pointer
node* selectedSymbolNode = nodes[lastNode]; // selectedSymbolNode is realocated
node* newSymbolNode = nodes[++lastNode];    // Create new node for symbol

// Add newly registered symbol to SymbolCache
symbolCache[lastSymbolInCache].nodesAddress = newSymbolNode;
symbolCache[lastSymbolInCache].symbolValue = newSymbolValue;

// Populate struct fields for new symbol
newSymbolNode->count = 1;
newSymbolNode->parent = newParentNode;
newSymbolNode->link0 = newSymbolNode;
newSymbolNode->link1 = newSymbolNode;
newSymbolNode->positionInNodes = lastNode;

// Create new node in "nodes" array that will have reallocated and new symbol set 
// as its children, and "count" as sum of children "count" values, swap parents of 2 nodes
newParentNode->count = selectedSymbolNode->count + 1;
newParentNode->link0 = newSymbolNode;
newParentNode->link1 = selectedSymbolNode;

// Update link of Parent node of swapped symbol
if ((selectedSymbolNode->parent)->link1 == selectedSymbolNode)
    (selectedSymbolNode->parent)->link1 = newParentNode;
else (selectedSymbolNode->parent)->link0 = newParentNode;

newParentNode->parent = selectedSymbolNode->parent; 
selectedSymbolNode->parent = newParentNode;

// Update hierarchy representation inside nodes
newParentNode->positionInNodes = selectedSymbolNode->positionInNodes;
selectedSymbolNode->positionInNodes = lastNode - 1;


// Return PARENT of added "node" to further tree reorganizatio

return newParentNode->parent;
}

/**
  * @brief  Function searches cache for symbol, if mach is found value of symbol is returned
  * @param symbol value of symbol from data stream
  * @retval NULL if no mach is found and symbol address if symbol was previously registered in cache
  */
node* searchCache(int8_t symbol)
{
    uint16_t temp = 0;

    // Search the cache
    while (temp <= lastSymbolInCache) {
        if (symbolCache[temp].symbolValue == symbol)
            return symbolCache[temp].nodesAddress;
        temp++;
    }
    lastSymbolInCache++;
    return NULL;
}

/**
  * @brief  Function checks if it is necesarry to reallocate memory and calls manageMemory() if so
  * it should be called each iteration inside main loop after we add new entry
  * @param None
  * @retval None
  */
void memoryCheck()
{
    if ((lastSymbolInCache + 1) >= BASE_CACHE_ENTRIES * cacheMemoryBlockMultiplier) 
        manageMemory(1);
    if ((lastNode + 2) >= BASE_NODE_ENTRIES * nodesMemoryBlockMultiplier) 
        manageMemory(2);
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
        case 0: {
            records = (int8_t*)malloc(BASE_SYMBOL_BUFFER * sizeof(int8_t));

            nodes = (node**)malloc(BASE_NODE_ENTRIES * sizeof(node*));
            node* nodesMemoryBlock = (node*)malloc(BASE_NODE_ENTRIES * sizeof(node));
            for (int i = 0; i < BASE_NODE_ENTRIES; i++)
                nodes[i] = &nodesMemoryBlock[i];

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
            root->positionInNodes = 0;

            symbol1->count = 1;
            symbol1->parent = root;
            symbol1->link0 = symbol1;    // Symbol
            symbol1->link1 = symbol1;
            symbol1->positionInNodes = 1;

            symbol2->count = 1;
            symbol2->parent = root;
            symbol2->link0 = symbol2;    // Symbol
            symbol2->link1 = symbol2;
            symbol2->positionInNodes = 2;

            cache0->nodesAddress = symbol1;
            cache0->symbolValue = 0;

            cache1->nodesAddress = symbol2;
            cache1->symbolValue = 1;

            lastSymbolInCache = 1;
            lastNode = 2;
            recordsCount = 0;

            printf("Memory initialized");
            break;
        }
        // Realloc for symbolCache
        case 1: {
            uint16_t oldSizeOfCashe = BASE_CACHE_ENTRIES * cacheMemoryBlockMultiplier * sizeof(cacheEntry);
            cacheMemoryBlockMultiplier++;
            cacheEntry* tempCache = (cacheEntry*)malloc(BASE_CACHE_ENTRIES * cacheMemoryBlockMultiplier * sizeof(cacheEntry));
            memcpy(tempCache, symbolCache, oldSizeOfCashe);
            free(symbolCache);
            symbolCache = tempCache;
            break;
        }
        // Realloc for nodes array
        case 2: {
            uint16_t numberOfNodes = BASE_NODE_ENTRIES * nodesMemoryBlockMultiplier;
            uint16_t oldSizeOfNodes = numberOfNodes * sizeof(node*);
        
            // Increment multiplier for the expanded node array
            nodesMemoryBlockMultiplier++;

            // Allocate new memory blocks
            node** tempNodes = (node**)malloc(BASE_NODE_ENTRIES * nodesMemoryBlockMultiplier * sizeof(node*));
            node* newNodesMemoryBlock = (node*)malloc(BASE_NODE_ENTRIES * sizeof(node));

            // Copy old pointers into the new array and initialize pointers for new nodes
            memcpy(tempNodes, nodes, oldSizeOfNodes);
            for (int i = numberOfNodes; i < BASE_NODE_ENTRIES * nodesMemoryBlockMultiplier; i++)
                tempNodes[i] = &newNodesMemoryBlock[i - numberOfNodes];

            // Free old array and assign new memory
            free(nodes);
            nodes = tempNodes;
            break;
        }
        // Free all data
        case 3: {
            free(nodes[0]); // TODO: Implement free of all data stored
            free(nodes);
            free(symbolCache);
            free(records);
            break;
        }
    }
}

/**
  * @brief  Function ask user for file path and opens file if path is correct
  * @param None
  * @retval 1 if error occured, 0 otherwise
  */
int openFile()
{
    char filePath[512];
    printf("\nPlease enter valid path file: ");
    if (scanf("%511s", filePath) != 1) { 
        printf("\nError reading input.");
        return 1;
    }
    fileToCompress = fopen(filePath,"rb");
    
    if (fileToCompress == NULL) {
        printf("\nError opening file");
        return 1;
    }
    return 0;
}

/**
  * @brief  Function skips header of file 
  * @param None
  * @retval None
  */
void skipHeader()
{
    char line[64];
    uint8_t headerLines = 3;   // Each PGM Image File must consist of 3 header lines:
                                // signature, rows and cols, max grey level
    while (headerLines > 0) {
        fgets(line, sizeof(line), fileToCompress); // Read header line
        if (line[0] == '#') continue;    // Skip all comments
        headerLines--;    
    }
}

/**
  * @brief Function responsible for loading data to records array, that means reading 
  * header and actual data, seting row width (assign value from header, we hardcode value 512 here)
  * @param None
  * @retval Function returns 0 if file is read correctly,
  */
int readDataFromFile()
{
    skipHeader();

    // Read all data from file and store it in records array
    for (int i = 0; i <= IMAGE_ROW_WIDTH; i++) {
        size_t bytesRead = fread(&records[i * IMAGE_ROW_WIDTH], 1, IMAGE_ROW_WIDTH, fileToCompress);
        if (bytesRead != IMAGE_ROW_WIDTH && !feof(fileToCompress)) {
            printf("Error reading data from file\n");
            free(records);
            fclose(fileToCompress);
            return 1;
        }
    }
    // Check if we reached end of file
    if (feof(fileToCompress))
        return 0;
    printf("EOF not reached!\n");
    return 1;
}

/**
  * @brief Function creates bit representation for each symbol present in symbolCache. Representation
  * is saved in array of struct "symbols" when each struct consist of bit representation "bits" and
  * number of bits saved -> "mask" because representation can have leading zeros.
  * @param None
  * @retval None
  */
void resolveTree()
{
    symbols = (symbol*)malloc(lastSymbolInCache * sizeof(symbol));

    for (int i = 0; i < lastSymbolInCache; i++) {
        node* root = nodes[0];
        node* node = symbolCache[i].nodesAddress;

        symbols[i].bits = 0;
        symbols[i].mask = 0;

        while (node != root) {
            if (node == (node->parent)->link1) // If our parent has us as its link1 children save "1" as bit 
                symbols[i].bits = (symbols[i].bits << 1) + 1; // else save "0"
            else
                symbols[i].bits = (symbols[i].bits << 1);
        symbols[i].mask = (symbols[i].mask << 1) + 1;
        node = node->parent;
        }
    }
}

/**
  * @brief Function saves compressed stream to file according to bit representation table
  * @param None
  * @retval 
  */
void saveCompressedFile()
{
    //TODO
}
