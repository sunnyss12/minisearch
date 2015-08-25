#include "MakeInvertedIndex.h"


int main(int argc, char const *argv[])
{
    MakeInvertedIndex foo;
    foo.readPageLibIndex();
    foo.readDocuments();
    foo.computeFrequency();
    foo.computeWeight();
    foo.normalizeWordWeight();
    foo.addWeightToIndex();

    foo.saveToDisk();

    return 0;
}
