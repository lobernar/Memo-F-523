#include "BdTree.cpp"
#define DELTA (0.25)

void testCase1(){
    /*
    Tests insertion
    */
    int N = pow(10, 3);
    BdTree t = BdTree(10, DELTA, N);
    for(int i=1; i<=2000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    //printf("%i\n", t.predecessor(1999));
    t.gernerateSVG("bdtree.dot", "bdtree.svg");
}

void testCase2(){
    /*
    Tests insertion
    */
    int N = pow(10, 5);
    BdTree t = BdTree(50, DELTA, N);
    for(int i=1; i<=200000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
}

void testCase3(){
    /*
    Tests deletion
    */
    int N = pow(10, 3);
    BdTree t = BdTree(10, DELTA, N);
    for(int i=1; i<=2000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
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
    //t.printTree();
    for(int i=1; i<200; i++) t.insertUpdate(i, DELETE);
    t.printTree();
}

void testCase5(){
    /*
    Tests deletion
    */
    int N = pow(10, 3);
    BdTree t = BdTree(10, DELTA, N);
    for(int i=1; i<=2000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1; i<2000; i++) t.insertUpdate(i, DELETE);
    t.printTree();
}

void testCase6(){
    /*
    Tests deletion (Flushing cascades in Be-Tree)
    */
    int N = pow(10, 6);
    BdTree t = BdTree(20, DELTA, N);
    for(int i=1; i<=1000000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1; i<1000000; i++) t.insertUpdate(i, DELETE);
    t.printTree();
}

void testCase7(){
    /*
    Tests deletion (Flushing cascades in Be-Tree)
    */
    int N = pow(10, 6);
    BdTree t = BdTree(20, DELTA, N);
    for(int i=1; i<=1000000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1000000; i>0; i--) t.insertUpdate(i, DELETE);
    t.printTree();
}

void testCase8(){
    /*
    Tests deletion (Flushing cascades in Be-Tree)
    */
    int N = pow(10, 6);
    BdTree t = BdTree(50, DELTA, N);
    for(int i=1; i<=9000000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1000000; i>0; i--) t.insertUpdate(i, DELETE);
    //t.printTree();
}

//TODO: Test different values of DELTA

void testCase9(){
    int N = pow(10, 6);
    BdTree t = BdTree(20, 0.5, N);
    for(int i=1; i<=1000000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1000000; i>0; i--) t.insertUpdate(i, DELETE);
    t.printTree();
}

void testCase10(){
    int N = pow(10, 6);
    BdTree t = BdTree(20, 0.75, N);
    for(int i=1; i<=1000000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1000000; i>0; i--) t.insertUpdate(i, DELETE);
    t.printTree();
}

int main(){
    printf("\n------------------------------TEST CASE 1------------------------------\n\n");
    testCase1();
    printf("\n------------------------------TEST CASE 2------------------------------\n\n");
    testCase2();
    printf("\n------------------------------TEST CASE 3------------------------------\n\n");
    testCase3();
    printf("\n------------------------------TEST CASE 4------------------------------\n\n");
    testCase4();
    printf("\n------------------------------TEST CASE 5------------------------------\n\n");
    testCase5();
    printf("\n------------------------------TEST CASE 6------------------------------\n\n");
    testCase6();
    printf("\n------------------------------TEST CASE 7------------------------------\n\n");
    testCase7();
    printf("\n------------------------------TEST CASE 8------------------------------\n\n");
    testCase8();
    printf("\n------------------------------TEST CASE 9------------------------------\n\n");
    testCase9();
    printf("\n------------------------------TEST CASE 10------------------------------\n\n");
    testCase10();
    return 0;
}