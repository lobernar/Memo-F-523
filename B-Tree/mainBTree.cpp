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
    t.remove(32);
    t.remove(31);
    //t.printTree();
    t.remove(30);
    t.printTree();
}

void testCase2(){
    BTree t = BTree(6);
    for(int i=5; i<=65; i=i+5) t.insert(i);
    t.insert(31);
    t.insert(33);
    t.insert(32);
    //t.printTree();
    t.remove(33);
    //t.printTree();
    std::cout << std::endl;
    t.remove(30);
    //t.printTree();
    t.remove(35);
    t.printTree();
}

void testCase3(){
    BTree t = BTree(6);
    for(int i=5; i<=20; i=i+5) t.insert(i);
    t.insert(30);
    t.insert(35);
    t.insert(70);
    //t.printTree();
    t.remove(10);
    t.printTree();
}

void testCase4(){
    BTree t = BTree(6);
    for(int i=10; i<=100; i=i+10) t.insert(i);
    t.insert(65);
    t.remove(65);
    t.remove(70);
    t.remove(100);
    t.remove(80);
    t.printTree();
}

void testCase5(){
    BTree t = BTree(10);
    for(int i=1; i<=300; ++i) t.insert(i);
    t.remove(3);
    for(int i=10; i<= 300; i+=10) t.remove(i);
    t.printTree();
}

void testCase6(){
    // Test predecessor
    BTree t = BTree(10);
    for(int i=1; i<=300; ++i) t.insert(i);
    //t.printTree();
    //int pred = 3;
    //printf("Predecessor of %i: %i\n", pred, t.predecessor(pred));
}

void testCase7(){
    BTree t = BTree(20);
    for(int i=1500; i>1; --i) t.insert(i);
    //t.printTree();
    t.remove(3);
    for(int i=10; i<= 300; ++i) t.remove(i);
    //t.printTree();
    t.gernerateSVG("btree.dot", "btree.svg");
}

void testCase8(){
    BTree t = BTree(20);
    for(int i=1; i<= 1000000; i++) t.insert(i);
}

void testCase9(){
    BTree t = BTree(20);
    for(int i=1; i<= 100000; i++) t.insert(i);
    for(int i=1; i<50000; ++i) t.remove(i);
    //t.printTree();
}

void testCase10(){
    // Flushing cascades! (EPS = 0.5, B=50, insert 9'000'000, delete 1'000'000)
    BTree t = BTree(50);
    for(int i=1; i<=9000000; i++) t.insert(i);
    //t.printTree();
    for(int i=1000000; i>0; i--) t.remove(i);
    //t.printTree();
}

void testCase11(){
    // Flushing cascades! (EPS = 0.5, B=50, insert 9'000'000, delete 1'000'000)
    BTree t = BTree(50);
    for(int i=1; i<=1000000; i++) t.insert(i);
    for(int i=1000000; i>0; i--) t.remove(i);
    //t.printTree();
}

void testCase12(){
    BTree t = BTree(50);
    for(int i=1; i<=1000000; i++) t.insert(i);
    //t.printTree();
    for(int i=1000000; i>500000; i--) t.remove(i);
    for(int i=1; i<500000; ++i) t.remove(i);
    //t.printTree();
}

void testCase13(){
    // Test insertion in different order
    BTree t = BTree(50);
    for(int i=500000; i>1; i--) t.insert(i);
    //t.printTree();
    for(int i=500001; i<1000000; ++i) t.insert(i);
    //t.printTree();
}

void varB(){
    // FILE* gnuplotPipe = popen("gnuplot -persistent", "w");

    // fprintf(gnuplotPipe, "set xlabel 'Insertion'\n");
    // fprintf(gnuplotPipe, "set ylabel 'Block Transfers'\n");
    // fprintf(gnuplotPipe, "set title 'Block Transfers vs. Insertion for diffrent B values'\n");
    // fprintf(gnuplotPipe, "plot ");

    for(int b=5; b<=100; b+=45){
        BTree t = BTree(b);
        std::vector<int> blockTransfers;
        for(int i=1; i<=1000000; ++i){
            int block = t.insert(i);
            blockTransfers.push_back(block);
        }

        for(int k : blockTransfers) printf("%i, ", k);
        printf("\n");

        // Create a temporary file to store the data
        const std::string name = "data" + std::to_string(b) + ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }

        // Rewind the file pointer to the beginning

        // Send commands to gnuplot to plot data from the temporary file
        //fprintf(gnuplotPipe, "'-' with lines title 'B = %i' ", b);
        // Send the data from the temporary file to gnuplot
        //fprintf(gnuplotPipe, "'%s' with lines title 'B = %i' ", name.c_str(), b);
        // Close the temporary file
        // file.close();
        // if(b<100) fprintf(gnuplotPipe, ", "); //fprintf(gnuplotPipe, "\n replot ");
        // else fprintf(gnuplotPipe, "\n");
    }
    // fprintf(gnuplotPipe, "\n");
    // pclose(gnuplotPipe);
    // printf("Plot generated successfully.\n");
}

void ins1000(){
    BTree t = BTree(10);
    std::vector<int> blockTransfers;
    for(int i=1; i<=1000; ++i){
        int block = t.insert(i);
        blockTransfers.push_back(block);
    }

    for(int k : blockTransfers) printf("%i, ", k);
    printf("\n");

    // Create a temporary file to store the data
    const std::string name = "data" + std::to_string(10) + ".dat";
    std::ofstream file(name);

    // Write the data to the temporary file
    for (int j = 0; j < blockTransfers.size(); ++j) {
        //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
        file << j+1 << " " << blockTransfers[j] << "\n";
    }
    file.close();
}


int main(int argc, char** argv){
    // std::cout << "------------------------------TEST CASE 1----------------------------\n";
    // testCase1();
    // std::cout << "------------------------------TEST CASE 2----------------------------\n";
    // testCase2();
    // std::cout << "------------------------------TEST CASE 3----------------------------\n";
    // testCase3();
    // std::cout << "------------------------------TEST CASE 4----------------------------\n";
    // testCase4();
    // std::cout << "------------------------------TEST CASE 5----------------------------\n";
    // testCase5(); 
    // std::cout << "------------------------------TEST CASE 6----------------------------\n";
    // testCase6();    
    // std::cout << "------------------------------TEST CASE 7----------------------------\n";
    // testCase7();    
    // std::cout << "------------------------------TEST CASE 8----------------------------\n";
    // testCase8();
    // std::cout << "------------------------------TEST CASE 9----------------------------\n";
    // testCase9();  
    // std::cout << "------------------------------TEST CASE 10----------------------------\n";
    // testCase10();
    // std::cout << "------------------------------TEST CASE 11----------------------------\n";
    // testCase11();
    // std::cout << "------------------------------TEST CASE 12----------------------------\n";
    // testCase12();
    // std::cout << "------------------------------TEST CASE 13----------------------------\n";
    // testCase13();  
    printf("----------------------------------Testing Different Values of B----------------------------------\n");
    varB();
    ins1000();


    return 0;
}