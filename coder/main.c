#include "treeOperations.h"
#include "fileOperations.h"

int main()
{
    handler* handler = createHandler();
    if (!handler) return 1;
    if (initialize(handler)) return 1;
    if (constructTree(handler)) return 1;
    return 0;
}
