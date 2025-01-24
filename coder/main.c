#include "fileOperations.h"

uint8_t run()
{
    handler* handler = createHandler();
    if (!handler) return 1;
    if (initialize(handler)) return 1;
    if (constructTree(handler)) return 1;
    freeAlocatedMemory(handler);
    scanf("%d", NULL);
    return 0;
}

int main()
{
    run();
    scanf("%d", NULL);
    return 0;
}