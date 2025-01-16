#include "treeOperations.h"
#include "fileOperations.h"
#include "time.h"

int main()
{
    clock_t start = clock();

    handler* handler = createHandler();
    if (!handler) return 1;
    if (initialize(handler)) return 1;
    if (constructTree(handler)) return 1;
    freeAlocatedMemory(handler);

    printf("Execute time: %dms\n", clock() - start);

    return 0;
}