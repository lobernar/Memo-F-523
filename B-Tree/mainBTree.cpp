#include <iostream>
#include "BTree.cpp"

void testCase1(){
    BTree t = BTree(6);
    for(int i=5; i<=65; i=i+5) t.insert(i);
    t.insert(28);
    t.insert(31);
    t.insert(33);
    t.insert(32);
    //t.printTree();
    t.printTree();
    t.remove(32);
    t.printTree();
    std::cout << std::endl;

    t.remove(31);
    t.printTree();
    std::cout << std::endl;
    //t.remove(30);
    t.printTree();
}

void testCase2(){
    BTree t = BTree(6);
    for(int i=5; i<=65; i=i+5) t.insert(i);
    t.insert(31);
    t.insert(33);
    t.insert(32);
    t.printTree();
    t.remove(33);
    t.printTree();
    std::cout << std::endl;
    t.remove(30);
    t.printTree();
    t.remove(35);
    t.printTree();
}

void testCase3(){
    BTree t = BTree(6);
    for(int i=5; i<=20; i=i+5) t.insert(i);
    t.insert(30);
    t.insert(35);
    t.insert(70);
    t.printTree();
    t.remove(10);
    t.printTree();
}

void testCase4(){
    BTree t = BTree(6);
    for(int i=10; i<=100; i=i+10) t.insert(i);
    t.insert(65);
    t.printTree();
    t.remove(65);
    t.printTree();
    std::cout << std::endl;
    t.remove(70);
    t.printTree();
    std::cout << std::endl;
    t.remove(100);
    t.printTree();
    std::cout << std::endl;
    //t.search(80)->print();
    t.remove(80);
    t.printTree();
}

void testCase5(){
    BTree t = BTree(10);
    for(int i=1; i<=300; ++i) t.insert(i);
    t.printTree();
    t.remove(3);
    t.printTree();
    for(int i=10; i<= 300; i+=10) t.remove(i);
    t.printTree();
}

void testCase6(){
    // Test predecessor
    BTree t = BTree(10);
    for(int i=1; i<=300; ++i) t.insert(i);
    t.printTree();
    int pred = 3;
    printf("Predecessor of %i: %i\n", pred, t.predecessor(pred));
}

void testCase7(){
    BTree t = BTree(20);
    for(int i=1500; i>1; --i) t.insert(i);
    t.printTree();
    t.remove(3);
    t.printTree();
    for(int i=10; i<= 300; i+=10) t.remove(i);
    t.printTree();
}

void testCase8(){
    BTree t = BTree(20);
    for(int i=1; i<= 1000000; i++) t.insert(i);
}

void testCase9(){
    BTree t = BTree(20);
    for(int i=1; i<= 1000000; i++) t.insert(i);
    for(int i=1; i<50000; ++i) t.remove(i);
}


int main(int argc, char** argv){
    std::cout << "------------------------------TEST CASE 1----------------------------\n";
    testCase1();
    std::cout << "------------------------------TEST CASE 2----------------------------\n";
    testCase2();
    std::cout << "------------------------------TEST CASE 3----------------------------\n";
    testCase3();
    std::cout << "------------------------------TEST CASE 4----------------------------\n";
    testCase4();
    std::cout << "------------------------------TEST CASE 5----------------------------\n";
    testCase5(); 
    std::cout << "------------------------------TEST CASE 6----------------------------\n";
    testCase6();    
    std::cout << "------------------------------TEST CASE 7----------------------------\n";
    testCase7();    
    std::cout << "------------------------------TEST CASE 8----------------------------\n";
    testCase8();
    std::cout << "------------------------------TEST CASE 9----------------------------\n";
    testCase9();  


    return 0;
}