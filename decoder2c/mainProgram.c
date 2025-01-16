#include "decoderOperations.h"
#include "time.h"

uint8_t main()
{
    clock_t start = clock();
    tree* this = createTree();
    if (!this) return 1;
    if (constructTree(this)) return 1;
    if (writeToFile(this->output)) return 1;
    freeAlocatedMemory(&this);
    printf("Execute time: %dms\n", clock() - start);
    scanf("%1s", NULL);
    return 0;
}