#include "BdTree.cpp"
#define DELTA (0.25)
#define N (pow(10, 6))

void testCase1(){
    /*
    Tests insertion
    */
    BeTree t = BeTree(10, DELTA, N);
    for(int i=1; i<=500; i++) t.insertUpdate(i, INSERT);
    t.printTree();
}

int main(){
    printf("\n------------------------------TEST CASE 1------------------------------\n\n");
    testCase1();
    return 0;
}