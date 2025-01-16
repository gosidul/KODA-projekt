#include "decoderOperations.h"

/**
 * @brief  Expands the memory pointers by reallocating memory for the array of node pointers.
 *         It calculates the current array size, allocates memory for the expanded array, and 
 *         copies existing pointers to the new memory. The function ensures that the old memory 
 *         is freed after the expansion.
 * @param  None
 * @retval 0 if successfully allocated new space, 1 if memory allocation fails
 */
uint8_t expandPointersArray(tree* this)
{
    node** newArray = (node**)malloc((this->memoryBlockMultiplier + BASE_ARRAY_ENTRIES) * sizeof(node*));
    if (!newArray) {
        printf("Błąd podczas alokowania pamięci na tablicę wskaźników do węzłów głównych!");
        return 1;
    }
    if (this->memoryPointers) {
        memcpy(newArray, this->memoryPointers, this->memoryBlockMultiplier * sizeof(node*));
        free(this->memoryPointers);
    }
    this->memoryPointers = newArray;
    return 0;
}

/**
 * @brief  Expands the tree by reallocating memory for the array of node pointers and adding a new block of nodes.
 *         It calculates the current tree size, allocates memory for the expanded tree, and copies the existing node 
 *         pointers to the new memory. The function ensures that the old memory is freed after the expansion.
 * @param  None
 * @retval 0 if successfully allocated new space, 1 if memory allocation fails
 */
uint8_t expandNodes(tree* this)
{
    // Expand array of pointers if necesarry 
    if (this->memoryBlockMultiplier % BASE_ARRAY_ENTRIES == 0) 
        if (expandPointersArray(this)) return 1;

    uint16_t currentNumberOfNodes = this->baseNumberOfNodes * this->memoryBlockMultiplier;
    node** newNodes = (node**)malloc(this->baseNumberOfNodes * (this->memoryBlockMultiplier + 1) * sizeof(node*));
    if (!newNodes) {
        printf("Błąd podczas alokowania pamięci na nową tablicę wskaźników do węzłów");
        return 1;
    }
    *(newNodes + currentNumberOfNodes) = (node*)malloc(this->baseNumberOfNodes * sizeof(node));
    if (!newNodes[currentNumberOfNodes]) {
        free(newNodes);
        printf("Błąd podczas alokowania pamięci na tablicę węzłów");
        return 1;
    }

    // Append address of new memory chunk to pointers array for memory tracking
    this->memoryPointers[this->memoryBlockMultiplier] = *(newNodes + currentNumberOfNodes);

    // node* -> &node[0]; node* + 1 -> &node[1] -> Flatten nodes array dimentions 2D -> 1D
    for (uint16_t i = 1; i < this->baseNumberOfNodes; i++)
        *(newNodes + currentNumberOfNodes + i) = newNodes[currentNumberOfNodes] + i;
    if (this->nodes) {
        memcpy(newNodes, this->nodes, currentNumberOfNodes * sizeof(node*));
        free(this->nodes);
    }
    this->nodes = newNodes;
    this->memoryBlockMultiplier++;
    return 0;
}

void freeAlocatedMemory(tree** this)
{
    // Free memory used for nodes array
    if ((*this)->nodes) {
        for (uint16_t i = 0; i < (*this)->memoryBlockMultiplier; i++)
            free((*this)->memoryPointers[i]);
        // Free memory for node struct pointers
        free((*this)->nodes);
        (*this)->nodes = NULL;
    }
    free((*this)->memoryPointers);
    (*this)->memoryPointers = NULL;
    (*this)->input->killMe(&(*this)->input);
    (*this)->output->killMe(&(*this)->output);
    (*this)->baseNumberOfNodes = 0;
    (*this)->lastNode = 0;
    (*this)->memoryBlockMultiplier = 0;
    (*this) = NULL;
}

tree* createTree()
{
    tree* this = malloc(sizeof(tree));
    if (!this) {
        printf("Błąd podczas alokowania pamięci na strukturę drzewa!");
        return NULL;
    }
    this->memoryPointers = NULL;
    this->nodes = NULL;
    this->input = createBitBuffer(BASE_BUFFER_SIZE);
    this->output = createByteBuffer(BASE_BUFFER_SIZE);
    this->baseNumberOfNodes = BASE_NODES_ENTRIES;
    this->memoryBlockMultiplier = 0;
    this->lastNode = 0;
    expandNodes(this);

    node* root =  this->nodes[this->lastNode];
    node* symbol0 = this->nodes[++this->lastNode];
    node* newSymbol = this->nodes[++this->lastNode];

    root->count = 1;
    root->parent = NULL;
    root->link0 = symbol0;
    root->link1 = newSymbol;
    root->positionInTree = 0;
    root->value = 0;

    newSymbol->count = 0;
    newSymbol->parent = root;
    newSymbol->link0 = NULL;
    newSymbol->link1 = NULL;
    newSymbol->positionInTree = this->lastNode;
    newSymbol->value = 0;

    symbol0->count = 1;
    symbol0->parent = root;
    symbol0->link0 = NULL;
    symbol0->link1 = NULL;
    symbol0->positionInTree = 1;

    this->input->popBit(this->input); // Path to first symbol (0)...
    symbol0->value = this->input->popSymbol(this->input); // Followed by bit representation
    this->output->appendByte(this->output, symbol0->value);

    return this;
}

/**
  * @brief  Function checks if it is necesarry to reallocate memory and if so, calls expandTree()
  * @param None
  * @retval 1 if failed to reallocate memory, 0 otherwise
  */
uint8_t memoryCheck(tree* this)
{
    if ((this->lastNode + 2) >= this->baseNumberOfNodes * this->memoryBlockMultiplier) 
        if (expandNodes(this)) return 1;
    return 0;
}

/**
  * @brief  Function search for a node highest in the tree hierarchy on the same "level" that means
  *         with the same "count" value that could be swapped with the node that we will increment 
  *         in this function call, if that is true, performs swap and returns address of next node
  * @param  Node Address of node that we will increment
  * @retval address of "parent" node of newly created node after swap, to further tree reorganization
  */
node* rearrangeTree(tree* this, node* _node)
{
    uint16_t tempAddress = _node->positionInTree;
    node* incrementedNode = this->nodes[tempAddress];

    while (incrementedNode->count == this->nodes[tempAddress - 1]->count) 
        tempAddress--;
    
    incrementedNode->count++;
    node* nodeToSwap = this->nodes[tempAddress];

    if (_node->positionInTree == tempAddress || nodeToSwap == _node->parent) 
        return incrementedNode->parent;

    this->nodes[nodeToSwap->positionInTree] = incrementedNode;
    this->nodes[_node->positionInTree] = nodeToSwap;

    tempAddress = nodeToSwap->positionInTree;
    nodeToSwap->positionInTree = incrementedNode->positionInTree;
    incrementedNode->positionInTree = tempAddress;

    if ((nodeToSwap->parent)->link1 == nodeToSwap)
        (nodeToSwap->parent)->link1 = incrementedNode;
    else (nodeToSwap->parent)->link0 = incrementedNode;

    if ((incrementedNode->parent)->link1 == incrementedNode)
        (incrementedNode->parent)->link1 = nodeToSwap;
    else (incrementedNode->parent)->link0 = nodeToSwap;

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
node* addNewSymbol(tree* this, uint8_t newValue)
{

node* newParentNode = this->nodes[this->lastNode];
node* symbolFromStream = this->nodes[++this->lastNode];

symbolFromStream->count = 1;
symbolFromStream->value = newValue;
symbolFromStream->parent = newParentNode;
symbolFromStream->link0 = NULL;
symbolFromStream->link1 = NULL;
symbolFromStream->positionInTree = this->lastNode;

node* newSymbolNode = this->nodes[++this->lastNode];

newSymbolNode->count = 0;
newSymbolNode->value = 0;
newSymbolNode->parent = newParentNode;
newSymbolNode->link0 = NULL;
newSymbolNode->link1 = NULL;
newSymbolNode->positionInTree = this->lastNode;

newParentNode->count = 1;
newParentNode->link0 = symbolFromStream;
newParentNode->link1 = newSymbolNode;

return newParentNode->parent;
}

node* retrieveSymbol(tree* this)
{
    // Start from root -> nodes[0]
    node* node = this->nodes[0];
    // While node == internal node
    while (node->link0 != NULL) {
        uint8_t bit = this->input->popBit(this->input);
        if (bit)
            node = node->link1;
        else 
            node = node->link0;
    }
    if (node == this->nodes[this->lastNode]) {
        uint8_t newSymbolValue = this->input->popSymbol(this->input);
        this->output->appendByte(this->output, newSymbolValue);
        return addNewSymbol(this, newSymbolValue);
    }
    this->output->appendByte(this->output, node->value);   
    return node;
}

uint8_t constructTree(tree* this)
{
    node* node;
    while (this->input->lastByte > this->input->currentByte) {
        if (memoryCheck(this)) return 1;
        node = retrieveSymbol(this);
        this->nodes[0]->count++;
        while (node->parent != NULL)
            node = rearrangeTree(this, node);
    }
    return 0;
}