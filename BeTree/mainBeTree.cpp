#include <stdlib.h>
#include "BeTree.cpp"
#define EPS (0.5)


void testCase1(){
    /*
    Tests insertion
    */
    BeTree t = BeTree(9, EPS);
    for(int i=1; i<=500; i++) t.insertUpdate(i, INSERT);
    t.printTree();
}

void testCase2(){
    /*
    Tests deletion (borrow from left sibling + merges)
    */
    BeTree t = BeTree(9, EPS);
    for(int i=1; i<=10; i++) {
        t.insertUpdate(i, INSERT);
    }
    t.insertUpdate(28, INSERT);
    t.insertUpdate(31, INSERT);
    t.insertUpdate(33, INSERT);
    t.insertUpdate(32, INSERT);
    for(int i=11; i<=21; i++) t.insertUpdate(i, INSERT);
    t.insertUpdate(34, INSERT);
    for(int i=40; i<50; i++) t.insertUpdate(i, INSERT);
    std::cout << "Before DELETE updates\n";
    t.printTree();
    for(int i=1; i<=15; i++) {
        t.insertUpdate(i, DELETE);
    }
    std::cout << "After DELETE updates\n";
    t.printTree();
    for(int i=1; i<100; i++) t.insertUpdate(i, BLANK);
    std::cout << "After BLANK updates\n";
    t.printTree();

}

void testCase3(){
    /*
    Tests deletion (borrow from left + merges)
    */
    // BeTree t = BeTree(9, EPS);
    // for(int i=1; i<=150; i++) t.insertUpdate(i, INSERT);
    // t.printTree();
    // for(int i=1; i<=150; i++) t.insertUpdate(i, DELETE);
    // t.printTree();
    BeTree t = BeTree(9, EPS);
    for(int i=1; i<=10; i++) {
        t.insertUpdate(i, INSERT);
    }
    t.insertUpdate(28, INSERT);
    t.insertUpdate(31, INSERT);
    t.insertUpdate(33, INSERT);
    t.insertUpdate(32, INSERT);
    for(int i=11; i<=21; i++) t.insertUpdate(i, INSERT);
    t.insertUpdate(34, INSERT);
    for(int i=40; i<50; i++) t.insertUpdate(i, INSERT);
    std::cout << "Before DELETE updates\n";
    t.printTree();
    for(int i=15; i>1; i--) {
        t.insertUpdate(i, DELETE);
    }
    std::cout << "After DELETE updates\n";
    t.printTree();
    for(int i=1; i<100; i++) t.insertUpdate(i, BLANK);
    std::cout << "After BLANK updates\n";
    t.printTree();
}

void testCase4(){
    /*
    Tests deletion 
    */
    BeTree t = BeTree(9, EPS);
    for(int i=1; i<=150; i++) t.insertUpdate(i, INSERT);
    t.printTree();
    for(int i=150; i>0; i--) t.insertUpdate(i, DELETE);
    t.printTree();
}

void testCase5(){
    /*
    Tests deletion
    */
    BeTree t = BeTree(16, EPS);
    for(int i=1; i<=150; i++) t.insertUpdate(i, INSERT);
    t.printTree();
    for(int i=150; i>0; i--) t.insertUpdate(i, DELETE);

    t.printTree();
}

void testCase6(){
    /*
    Tests range query
    */
    BeTree t = BeTree(9, EPS);
    for(int i=1; i<=1500; i++) t.insertUpdate(i, INSERT);
    t.printTree();
    int from = 70, to = 140;
    std::vector<int> preds = t.range(from, to);
    for(int key : preds) printf("%i ", key);
    printf("\n");
}


int main(){
    printf("\n------------------------------TEST CASE 1------------------------------\n\n");
    testCase1();
    printf("\n------------------------------TEST CASE 2------------------------------\n\n");
    testCase2();
    printf("\n------------------------------TEST CASE 3------------------------------\n\n");
    testCase3();
    printf("\n------------------------------TEST CASE 4--------------------------\n\n");
    testCase4();
    printf("\n------------------------------TEST CASE 5--------------------------\n\n");
    testCase5();
    printf("\n------------------------------TEST CASE 6--------------------------\n\n");
    testCase6();


    return 0;
}