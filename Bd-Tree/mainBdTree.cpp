#include "BdTree.cpp"
#define DELTA (0.25)

void testCase1(){
    /*
    Tests insertion
    */
    int N = pow(10, 3);
    BeTree t = BeTree(20, DELTA, N);
    for(int i=1; i<=1000; i++) t.insertUpdate(i, INSERT);
    t.printTree();
}

int main(){
    printf("\n------------------------------TEST CASE 1------------------------------\n\n");
    testCase1();
    return 0;
}