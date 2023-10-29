#include "BdTree.cpp"
#define DELTA (0.25)

void testCase1(){
    /*
    Tests insertion
    */
    int N = pow(10, 3);
    BdTree t = BdTree(20, DELTA, N);
    for(int i=1; i<=2000; i++) t.insertUpdate(i, INSERT);
    t.printTree();
}

void testCase2(){
    /*
    Tests insertion
    */
    int N = pow(10, 6);
    BdTree t = BdTree(50, DELTA, N);
    for(int i=1; i<=200000; i++) t.insertUpdate(i, INSERT);
    t.printTree();
}

int main(){
    printf("\n------------------------------TEST CASE 1------------------------------\n\n");
    testCase1();
    printf("\n------------------------------TEST CASE 2------------------------------\n\n");
    //testCase2();
    return 0;
}