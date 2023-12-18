#include "BdTree.cpp"
#define DELTA (0.25)

void testCase1(){
    /*
    Tests insertion
    */
    int N = pow(10, 3);
    BdTree t = BdTree(10, DELTA, N);
    for(int i=1; i<=2000; i++) t.insertUpdate(i, INSERT);
    t.generateSVGFile();
    t.printTree();
    printf("%i\n", t.predecessor(1999));
}

void testCase2(){
    /*
    Tests insertion
    */
    int N = pow(10, 6);
    BdTree t = BdTree(50, DELTA, N);
    for(int i=1; i<=200000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
}

void testCase3(){
    /*
    Tests deletion (Flushing cascades in BeTree)
    */
    int N = pow(10, 3);
    BdTree t = BdTree(10, DELTA, N);
    for(int i=1; i<=2000; i++) t.insertUpdate(i, INSERT);
    t.printTree();
    for(int i=200; i>0; i--) t.insertUpdate(i, DELETE);
    t.printTree();
}

void testCase4(){
    /*
    Tests deletion
    */
    int N = pow(10, 3);
    BdTree t = BdTree(10, DELTA, N);
    for(int i=1; i<=2000; i++) t.insertUpdate(i, INSERT);
    t.printTree();
    for(int i=1; i<200; i++) t.insertUpdate(i, DELETE);
    t.printTree();
}

int main(){
    printf("\n------------------------------TEST CASE 1------------------------------\n\n");
    testCase1();
    printf("\n------------------------------TEST CASE 2------------------------------\n\n");
    //testCase2();
    printf("\n------------------------------TEST CASE 3------------------------------\n\n");
    testCase3();
    printf("\n------------------------------TEST CASE 4------------------------------\n\n");
    testCase4();
    return 0;
}