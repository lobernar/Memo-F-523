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
    t.generateSVG("bdtree.dot", "bdtree.svg");
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

void testCase9(){
    /*
    Tests different value for DELTA
    */
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

void testCase11(){
    int n=pow(10, 7);
    BdTree t = BdTree(50, 0.5, n);
    for(int i=1; i<=n; ++i) t.insertUpdate(i, INSERT);
    for(int i=1; i<n; i+=2) t.insertUpdate(i, DELETE); // Remove odd numbers
    for(int i=2; i<n; i+=2) t.insertUpdate(i, DELETE); // Remove even numbers
    t.printTree();
}

void varB(){
    for(int b=5; b<=100; b += 45){
        BdTree t = BdTree(b, DELTA, 1000000);
        std::vector<int> blockTransfers;
        for(int i=1; i<1000000; i++){
            t.insertUpdate(i, INSERT);
        }
        blockTransfers = t.resTransfers;
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
    BdTree t = BdTree(40, DELTA, 1000);
    std::vector<int> blockTransfers;
    for(int i=1; i<1000; i++){
        int tr = t.insertUpdate(i, INSERT);
        //printf("%i\n", tr);
        blockTransfers.push_back(tr);
    }
    blockTransfers = t.resTransfers;
    // Create a temporary file to store the data
    const std::string name = "data_cycle_ins_40_0.25.dat";
    std::ofstream file(name);

    // Write the data to the temporary file
    for (int j = 0; j < blockTransfers.size(); ++j) {
        //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
        file << j+1 << " " << blockTransfers[j] << "\n";
    }
}

void del1000(){
    BdTree t = BdTree(40, DELTA, 1000);
    std::vector<int> blockTransfers;
    for(int i=1; i<=1000; i++) t.insertUpdate(i, INSERT);
    for(int i=1; i<=1000; i++) blockTransfers.push_back(t.insertUpdate(i, DELETE));
    //blockTransfers = t.resTransfers;
    // Create a temporary file to store the data
    const std::string name = "data_del_40_0.25.dat";
    std::ofstream file(name);

    // Write the data to the temporary file
    for (int j = 0; j < blockTransfers.size(); ++j) {
        //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
        file << j+1 << " " << blockTransfers[j] << "\n";
    }
}

void ins1000Cycle(){
    BdTree t = BdTree(40, DELTA, 1000);
    std::vector<int> blockTransfers;
    for(int i=1; i<1000; i++){
        t.insertUpdate(i, INSERT);
    }
    t.printTree();
    blockTransfers = t.resTransfers;
    // Create a temporary file to store the data
    const std::string name = "data_cycle_ins_40_0.25.dat";
    std::ofstream file(name);

    // Write the data to the temporary file
    for (int j = 0; j < blockTransfers.size(); ++j) {
        //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
        file << j+1 << " " << blockTransfers[j] << "\n";
    }
}

void flushingCascades2(){
    std::vector<int> blockTransfers;
    int n = pow(10, 7);
    BdTree t = BdTree(50, DELTA, n);
    for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove odd numbers
    for(int i=2; i<=n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove even numbers
    t.printTree();
    // Create a temporary file to store the data
    const std::string name = "data_flushCasc2_" + std::to_string(50) + "_" + std::to_string(DELTA) + ".dat";
    std::ofstream file(name);

    // Write the data to the temporary file
    for (int j = 0; j < blockTransfers.size(); ++j) {
        //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
        file << j+1 << " " << blockTransfers[j] << "\n";
    }
}

void varDELTAIns(){
    int n = pow(10, 7);
    for(float delta=0.25; delta<1; delta += 0.25){
        BdTree t = BdTree(50, delta, n);
        std::vector<int> blockTransfers;
        for(int i=1; i<=n; i++){
            int tr = t.insertUpdate(i, INSERT);
            blockTransfers.push_back(tr);
        }
        // Create a temporary file to store the data
        const std::string name = "data_VarDelta_ins_" + std::to_string(50) + "_" + std::to_string(delta) + "_10mil_" ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }
    }
}

void varDeltaDel(){
    for(float delta=0.25; delta<1; delta += 0.25){
        std::vector<int> blockTransfers;
        int n = pow(10, 7);
        BdTree t = BdTree(50, delta, n);
        for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
        for(int i=1; i<n; ++i) blockTransfers.push_back(t.insertUpdate(i, DELETE));
        // Create a temporary file to store the data
        const std::string name = "data_del_" + std::to_string(50) + "_" + std::to_string(delta) + "_10mil_" ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }
    }
}

void varDELTAflushCasc2(){
    int n = pow(10, 7);
    for(float delta=0.25; delta<1; delta += 0.25){
        BdTree t = BdTree(50, delta, n);
        std::vector<int> blockTransfers;
        for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
        for(int i=1; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove odd numbers
        for(int i=2; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove even numbers

        // Create a temporary file to store the data
        const std::string name = "data_VarDelta_del_" + std::to_string(50) + "_" + std::to_string(delta) + "_10mil_" ".dat";
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
        std::vector<int> blockTransfers;
        int n = pow(10, 7);
        BdTree t = BdTree(b, DELTA, n);
        for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
        //t.printTree();
        for(int i=1; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove odd numbers
        for(int i=2; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove even numbers
        //t.printTree();
        // Create a file to store the data
        const std::string name = "data_flushCasc2_VarB_" + std::to_string(b) + "_" + std::to_string(DELTA) + ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }
    }
}

void varB2FlushCasc2(){
    std::vector<int> blockTransfers;
    int n = pow(10, 7);
    BdTree t = BdTree(1000, DELTA, n);
    for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
    //t.printTree();
    for(int i=1; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove odd numbers
    for(int i=2; i<n; i+=2) blockTransfers.push_back(t.insertUpdate(i, DELETE)); // Remove even numbers
    //t.printTree();
    // Create a file to store the data
    const std::string name = "data_flushCasc2_VarB_" + std::to_string(1000) + "_" + std::to_string(DELTA) + ".dat";
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
        BdTree t = BdTree(b, DELTA, n);
        for(int i=1; i<=n; i++) blockTransfers.push_back(t.insertUpdate(i, INSERT));
        //t.printTree();
        // Create a file to store the data
        const std::string name = "data_ins_10mil_" + std::to_string(b) + "_" + std::to_string(0.25) + ".dat";
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
        BdTree t = BdTree(b, DELTA, n);
        for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
        //t.printTree();
        for(int i=1; i<n; i++) blockTransfers.push_back(t.insertUpdate(i, DELETE));
        //t.printTree();
        // Create a file to store the data
        const std::string name = "data_del_10mil_" + std::to_string(b) + "_" + std::to_string(0.25) + ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }
    }
}

void varDELTAIns10mil(){
    int n = pow(10, 7);
    for(float delta=0.25; delta<1; delta += 0.25){
        BdTree t = BdTree(50, delta, n);
        std::vector<int> blockTransfers;
        for(int i=1; i<=n; i++) blockTransfers.push_back(t.insertUpdate(i, INSERT));

        // Create a temporary file to store the data
        const std::string name = "data_ins_10mil_" + std::to_string(50) + "_" + std::to_string(delta) + ".dat";
        std::ofstream file(name);

        // Write the data to the temporary file
        for (int j = 0; j < blockTransfers.size(); ++j) {
            //fprintf(file, "%i %i\n", j + 1, blockTransfers[j]);
            file << j+1 << " " << blockTransfers[j] << "\n";
        }
    }
}

void varDELTADel10mil(){
    int n = pow(10, 7);
    for(float delta=0.25; delta<1; delta += 0.25){
        BdTree t = BdTree(50, delta, n);
        std::vector<int> blockTransfers;
        for(int i=1; i<=n; i++) t.insertUpdate(i, INSERT);
        for(int i=1; i<n; i++) blockTransfers.push_back(t.insertUpdate(i, DELETE));

        // Create a temporary file to store the data
        const std::string name = "data_del_10mil_" + std::to_string(50) + "_" + std::to_string(delta) + ".dat";
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
    // printf("\n------------------------------TEST CASE 4------------------------------\n\n");
    // testCase4();
    // printf("\n------------------------------TEST CASE 5------------------------------\n\n");
    // testCase5();
    // printf("\n------------------------------TEST CASE 6------------------------------\n\n");
    // testCase6();
    // printf("\n------------------------------TEST CASE 7------------------------------\n\n");
    // testCase7();
    // printf("\n------------------------------TEST CASE 8------------------------------\n\n");
    // testCase8();
    // printf("\n------------------------------TEST CASE 9------------------------------\n\n");
    // testCase9();
    // printf("\n------------------------------TEST CASE 10------------------------------\n\n");
    // testCase10();
    // printf("\n------------------------------TEST CASE 11------------------------------\n\n");
    // testCase11();
    // varB();
    // ins1000();
    // ins1000Cycle();
    // flushingCascades2();
    // varDELTAIns();
    // varDELTAflushCasc2();
    // varBFlushCasc2();
    // varB2FlushCasc2();
    // del1000();
    // varDeltaDel();
    varBIns10mil();
    varBDel10mil();

    return 0;
}