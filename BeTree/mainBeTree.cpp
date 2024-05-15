#include <stdlib.h>
#include "BeTree.cpp"
#define EPS (0.5)


void testCase1(){
    /*
    Tests insertion
    */
    BeTree t = BeTree(9, EPS);
    for(int i=1; i<=500; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
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
    //t.printTree();
    for(int i=1; i<=15; i++) {
        t.insertUpdate(i, DELETE);
    }
    //t.printTree();

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
    //t.printTree();
    for(int i=15; i>1; i--) {
        t.insertUpdate(i, DELETE);
    }
    //t.printTree();
    //for(int i=1; i<100; i++) t.insertUpdate(i, BLANK);
    //t.printTree();
}

void testCase4(){
    /*
    Tests deletion 
    */
    BeTree t = BeTree(9, EPS);
    for(int i=1; i<=5000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=5000; i>0; i--) t.insertUpdate(i, DELETE);
    //t.printTree();
}

void testCase5(){
    /*
    Tests deletion
    */
    BeTree t = BeTree(16, EPS);
    for(int i=1; i<=150; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=150; i>0; i--) t.insertUpdate(i, DELETE);

    //t.printTree();
}

void testCase6(){
    BeTree t = BeTree(20, EPS);
    for(int i=0; i<pow(10, 6); ++i) t.insertUpdate(i, INSERT);
    for(int i=5000; i<100000; ++i) t.insertUpdate(i, DELETE);
    //t.printTree();
}

void testCase7(){
    BeTree t = BeTree(10, EPS);
    for(int i=1; i<=2000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=200; i>0; i--) t.insertUpdate(i, DELETE);
    //t.printTree();
    t.generateSVG("betree.dot", "betree.svg");
}

void testCase8(){
    // Flushing cascades! (EPS = 0.5, B=50, insert 9'000'000, delete 1'000'000)
    BeTree t = BeTree(50, EPS);
    for(int i=1; i<=9000000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1000000; i>0; i--) t.insertUpdate(i, DELETE);
    //t.printTree();
}

void testCase9(){
    /*
    Tests insertion
    */
    BeTree t = BeTree(10, 0.25);
    for(int i=1; i<=2000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    //printf("%i\n", t.predecessor(1999));
}

void testCase10(){
    // Flushing cascades
    BeTree t = BeTree(50, EPS);
    for(int i=1; i<=1000000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1000000; i>0; i--) t.insertUpdate(i, DELETE);
    //t.printTree();
}

void testCase11(){
    // Flushing cascades
    BeTree t = BeTree(50, EPS);
    for(int i=1; i<=1000000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1000000; i>500000; i--) t.insertUpdate(i, DELETE);
    for(int i=1; i<500000; ++i) t.insertUpdate(i, DELETE);
    //t.printTree();
}

void testCase12(){
    // Test insertion in different order
    BeTree t = BeTree(50,EPS);
    for(int i=500000; i>1; i--) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=500001; i<1000000; ++i) t.insertUpdate(i, INSERT);
    //t.printTree();
}

void testCase13(){
    BeTree t = BeTree(10, EPS);
    int n = 20000;
    for(int i=1; i<=n; ++i) t.insertUpdate(i, INSERT);
    for(int i=1; i<n; i+=2){
        t.insertUpdate(i, DELETE);
        //t.insertUpdate(n-i, DELETE);
    }
}

void varB(){
    for(int b=5; b<=100; b += 45){
        BeTree t = BeTree(b, EPS);
        std::vector<int> blockTransfers;
        for(int i=1; i<1000000; i++){
            int tr = t.insertUpdate(i, INSERT);
            blockTransfers.push_back(tr);
        }
        // Create a temporary file to store the data
        const std::string name = "data" + std::to_string(b) + ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }
    }
}

void ins1000(){
    BeTree t = BeTree(40, EPS);
    std::vector<int> blockTransfers;
    for(int i=1; i<1000; i++){
        int tr = t.insertUpdate(i, INSERT);
        blockTransfers.push_back(tr);
    }
    // Create a temporary file to store the data
    const std::string name = "data" + std::to_string(40) + ".dat";
    std::ofstream file(name);

    // Write the data to the temporary file
    for (int j = 0; j < blockTransfers.size(); ++j) {
        //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
        file << j+1 << " " << blockTransfers[j] << "\n";
    }
}

void del1000(){
    BeTree t = BeTree(40, EPS);
    std::vector<int> blockTransfers;
    for(int i=1; i<=1000; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1; i<1000; i++){
        int tr = t.insertUpdate(i, DELETE);
        blockTransfers.push_back(tr);
    }
    t.printTree();
    // Create a temporary file to store the data
    const std::string name = "data_del" + std::to_string(40) + ".dat";
    std::ofstream file(name);

    // Write the data to the temporary file
    for (int j = 0; j < blockTransfers.size(); ++j) {
        //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
        file << j+1 << " " << blockTransfers[j] << "\n";
    }
}

void drawTree(){
    BeTree t = BeTree(6, EPS);
    for(int i=1; i<=20; ++i) t.insertUpdate(i, INSERT);
    for(int i=1; i<=10; ++i) t.insertUpdate(i, DELETE);
    t.generateSVG("testBe.dot", "testBe.svg");
    t.printTree();
}

void varEPSIns(){
    for(float eps=0.25; eps<1; eps += 0.25){
        BeTree t = BeTree(50, eps);
        std::vector<int> blockTransfers;
        int n = pow(10, 7);
        for(int i=1; i<=n; i++){
            int tr = t.insertUpdate(i, INSERT);
            blockTransfers.push_back(tr);
        }
        // Create a temporary file to store the data
        const std::string name = "data_ins_" + std::to_string(50) + "_" + std::to_string(eps) + "_10mil_" ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }
    }
}

void varEPSDel(){
    for(float eps=0.25; eps<1; eps += 0.25){
        BeTree t = BeTree(50, eps);
        std::vector<int> blockTransfers;
        int n = pow(10, 7);
        for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
        for(int i=1; i<n; ++i) blockTransfers.push_back(t.insertUpdate(i, DELETE));
        // Create a temporary file to store the data
        const std::string name = "data_del_" + std::to_string(50) + "_" + std::to_string(eps) + "_10mil_" ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }
    }
}

void flushingCascades(){
    BeTree t = BeTree(50, EPS);
    std::vector<int> blockTransfers;
    int n = pow(10, 7);
    for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1; i<n/2; i++){
        int tr = t.insertUpdate(i, DELETE);
        blockTransfers.push_back(tr);
        blockTransfers.push_back(t.insertUpdate(n-i, DELETE));
    }
    t.printTree();
    // Create a temporary file to store the data
    const std::string name = "data_flushCasc" + std::to_string(50) + "_" + std::to_string(EPS) + ".dat";
    std::ofstream file(name);

    // Write the data to the temporary file
    for (int j = 0; j < blockTransfers.size(); ++j) {
        //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
        file << j+1 << " " << blockTransfers[j] << "\n";
    }
}

void flushingCascades1(){
    BeTree t = BeTree(50, EPS);
    std::vector<int> blockTransfers;
    int n = pow(10, 7);
    for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1; i<n; i+=2){
        int tr = t.insertUpdate(i, DELETE);
        blockTransfers.push_back(tr);
        //blockTransfers.push_back(t.insertUpdate(n-i, DELETE));
    }
    //t.printTree();
    // Create a temporary file to store the data
    const std::string name = "data_flushCasc1_" + std::to_string(50) + "_" + std::to_string(EPS) + ".dat";
    std::ofstream file(name);

    // Write the data to the temporary file
    for (int j = 0; j < blockTransfers.size(); ++j) {
        //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
        file << j+1 << " " << blockTransfers[j] << "\n";
    }
}

void flushingCascades2(){
    BeTree t = BeTree(50, EPS);
    std::vector<int> blockTransfers;
    int n = pow(10, 7);
    for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove odd numbers
    for(int i=2; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove even numbers
    //t.printTree();
    // Create a temporary file to store the data
    const std::string name = "data_flushCasc2_" + std::to_string(50) + "_" + std::to_string(EPS) + ".dat";
    std::ofstream file(name);

    // Write the data to the temporary file
    for (int j = 0; j < blockTransfers.size(); ++j) {
        //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
        file << j+1 << " " << blockTransfers[j] << "\n";
    }
}

void varEPSFlushCasc2(){
    for(float eps=0.25; eps<1; eps += 0.25){
        BeTree t = BeTree(50, eps);
        std::vector<int> blockTransfers;
        int n = pow(10, 7);
        for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
        //t.printTree();
        for(int i=1; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove odd numbers
        for(int i=2; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove even numbers
        //t.printTree();
        // Create a temporary file to store the data
        const std::string name = "data_flushCasc2_" + std::to_string(50) + "_" + std::to_string(eps) + ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }
    }
}

void varBFlushCasc2(){
    for(float b=100; b<=1000; b += 300){
        BeTree t = BeTree(b, EPS);
        std::vector<int> blockTransfers;
        int n = pow(10, 7);
        for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
        //t.printTree();
        for(int i=1; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove odd numbers
        for(int i=2; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove even numbers
        //t.printTree();
        // Create a temporary file to store the data
        const std::string name = "data_flushCasc2_VarB_" + std::to_string(b) + "_" + std::to_string(EPS) + ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }
    }
}

void varB2FlushCasc2(){
    BeTree t = BeTree(1000, EPS);
    std::vector<int> blockTransfers;
    int n = pow(10, 7);
    for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove odd numbers
    for(int i=2; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove even numbers
    //t.printTree();
    // Create a temporary file to store the data
    const std::string name = "data_flushCasc2_VarB_" + std::to_string(1000) + "_" + std::to_string(EPS) + ".dat";
    std::ofstream file(name);

    // Write the data to the temporary file
    for (int j = 0; j < blockTransfers.size(); ++j) {
        //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
        file << j+1 << " " << blockTransfers[j] << "\n";
    }
}

void varBIns10mil(){
    for(float b=100; b<=1000; b += 300){
        std::vector<int> blockTransfers;
        int n = pow(10, 7);
        BeTree t = BeTree(b, EPS);
        for(int i=1; i<=n; i++) blockTransfers.push_back(t.insertUpdate(i, INSERT));
        //t.printTree();
        // Create a file to store the data
        const std::string name = "data_ins_10mil_" + std::to_string(b) + "_" + std::to_string(0.5) + ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }
    }
}

void varBDel10mil(){
    for(float b=100; b<=1000; b += 300){
        std::vector<int> blockTransfers;
        int n = pow(10, 7);
        BeTree t = BeTree(b, EPS);
        for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
        //t.printTree();
        for(int i=1; i<n; i++) blockTransfers.push_back(t.insertUpdate(i, DELETE));
        //t.printTree();
        // Create a file to store the data
        const std::string name = "data_del_10mil_" + std::to_string(b) + "_" + std::to_string(0.5) + ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }
    }
}


int main(){
    // printf("\n------------------------------TEST CASE 1------------------------------\n\n");
    // testCase1();
    // printf("\n------------------------------TEST CASE 2------------------------------\n\n");
    // testCase2();
    // printf("\n------------------------------TEST CASE 3------------------------------\n\n");
    // testCase3();
    // printf("\n------------------------------TEST CASE 4--------------------------\n\n");
    // testCase4();
    // printf("\n------------------------------TEST CASE 5--------------------------\n\n");
    // testCase5();
    // printf("\n------------------------------TEST CASE 6--------------------------\n\n");
    // testCase6();
    // printf("\n------------------------------TEST CASE 7--------------------------\n\n");
    // testCase7();
    // printf("\n------------------------------TEST CASE 8--------------------------\n\n");
    // testCase8();
    // printf("\n------------------------------TEST CASE 9--------------------------\n\n");
    // testCase9();
    // printf("\n------------------------------TEST CASE 10--------------------------\n\n");
    // testCase10();
    // printf("\n------------------------------TEST CASE 11--------------------------\n\n");
    // testCase11();
    // printf("\n------------------------------TEST CASE 12--------------------------\n\n");
    // testCase12();
    // printf("\n------------------------------TEST CASE 13--------------------------\n\n");
    // testCase13();
    // varB();
    // ins1000();
    // del1000();
    // drawTree();
    // varEPSIns();
    // flushingCascades();
    // flushingCascades1();
    // flushingCascades2();
    // varEPSFlushCasc2();
    // varBFlushCasc2();
    // varB2FlushCasc2();
    // varEPSDel();
    // varBIns10mil();
    // printf("VAR B DELETE 10 Million\n");
    // varBDel10mil();
    drawTree();

    return 0;
}