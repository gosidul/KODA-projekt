#include "decoderOperations.h"

uint8_t run()
{
    tree* this = createTree();
    if (!this) return 1;
    if (constructTree(this)) return 1;
    if (writeToFile(this->output)) return 1;
    freeAlocatedMemory(&this);
    return 0;
}

int main()
{
    run();
    scanf("%d", NULL);
    return 0;
}