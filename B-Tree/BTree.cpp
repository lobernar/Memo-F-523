#include <fstream>
#include "Node.cpp"

class BTree{

    public:
    unsigned B;
    Node* root;
    int N;

    BTree(unsigned BIn){
        B = BIn;
        root = NULL;
        N = 0;
    }

    void printTree(){
        if(root != NULL) root->printBT("", false);
    }

    void gernerateSVGFromDot(const std::string& dotFile, const std::string& svgFile){
        // Construct the command to run the dot utility
        std::string command = "dot -Tsvg " + dotFile + " -o " + svgFile;

        // Run the dot command using system()
        int result = system(command.c_str());

        // Check if the command was successful
        if (result != 0) {
            std::cerr << "Error: Failed to generate SVG file. Make sure Graphviz is installed." << std::endl;
        } else {
            std::cout << "SVG file generated successfully: " << svgFile << std::endl;
        }
    }

    // Function to generate a DOT file for a B-tree
    void generateDotFile(){
        std::ofstream dotFile("btree.dot");
        dotFile << "digraph BTree {" << std::endl;
        //dotFile << "rankdir=TB;" << std::endl;
        dotFile << "node [shape = record,height=.5];" << std::endl;
        dotFile << "ranksep = 3.0;" << std::endl;
        dotFile << "splines = false;" << std::endl;
        generateDotNode(root, dotFile);
        dotFile << "}" <<std::endl;

        // Generate SVG file
        gernerateSVGFromDot("btree.dot", "btree.svg");
    }
    
    void generateDotNode(Node* node, std::ofstream& dotFile) {
        // Generates DOT code for each node
        if (node) {
            std::string nodeLabel = "node_" + std::to_string(reinterpret_cast<uintptr_t>(node));
            dotFile << nodeLabel << "[label = \"<f0>";
            
            for (size_t i = 0; i < node->keys.size(); ++i) {
                dotFile << " |" << node->keys[i] << "|<f" << (i + 1) << ">";
            }

            dotFile << "\"];" << std::endl;

            for (size_t i = 0; i < node->children.size(); ++i) {
                generateDotNode(node->children[i], dotFile);
                dotFile << "\"" << nodeLabel << "\":f" << i << " -> \"node_" << reinterpret_cast<uintptr_t>(node->children[i]) << "\";" << std::endl;
            }
        }
    }

    Node* search(int k){
        if(root == NULL){
            std::cout << "The tree is empty" << std::endl;
            return NULL;
        }
        return root->search(k);
    }

    void insert(int key){
        // Empty tree
        if(root == NULL) {
            root = new Node(NULL, std::vector<Node*>{}, std::vector<int>{key});
            return;
        } // Handle root overflow
        else if(root->keys.size() >= B) {
            root->split(0, B/2);
            root = root->parent;
        }
        // Root node is already created -> find appropriate leaf
        Node* curr = root;
        int blockTransfers = 0;
        int index = curr->findChild(key);
        // Check for nodes to split
        while(!curr->isLeaf() && curr != NULL){
            Node* next = curr->children[index];
            ++blockTransfers;
            // Check if split is needed + Update variables
            if(next->keys.size() >= B){
                next->split(index, B/2);
                blockTransfers += 2;
                curr = next->parent;
            } else curr = next;
            index = curr->findChild(key);
        }
        curr->insertKey(key, index); // Will have reached a leaf here
        //printf("Inserting in a B-tree of height %f with %i elements and B = %i required %i block transfers\n", ceil((double)log2(N)/log2(B)), N, B, blockTransfers);
        ++N;
    }

    void deleteFromInternal(Node* curr, int index, int& key, Node* next, int* blockTransfers = 0){
        // Find child y/z that preceds/succeds k
        Node* predecessor = curr->getPredecessor(index);
        Node* successor = curr->getSuccessor(index);

        // Find predecessor key and remove it recursively
        if(predecessor != curr && predecessor->keys.size() >= floor((B)/2)+1){
            int predKey = predecessor->keys[predecessor->keys.size()-1];
            curr->keys[index] = predKey;
            key = predKey;
            next = predecessor;
            ++blockTransfers;
        }
        // Find successor key and remove it recursively
        else if(successor != curr && successor->keys.size() >= floor((B)/2)+1){
            int succKey = successor->keys[0];
            curr->keys[index] = succKey;
            key = succKey;
            next = successor;
            ++blockTransfers;
        }
        // Merging predecessor with successor
        else{
            predecessor->insertKey(key, predecessor->keys.size());
            predecessor->mergePredSucc(successor);
            int keyIndex = curr->findChild(key);
            curr->keys.erase(curr->keys.begin()+keyIndex);
            // NOT SURE ABOUT THIS
            if(predecessor->parent != successor->parent){
                int insertIndex = curr->findChild(successor->parent->keys[0]);
                curr->keys.insert(curr->keys.begin()+insertIndex, successor->parent->keys[0]);
                successor->parent->keys.erase(successor->parent->keys.begin());
            }
            next = predecessor; // Update variable
            blockTransfers += 2;
        }
    }


    void remove(int key){
        int blockTransfers = 0;
        Node* curr = root;
        int index = curr->findChild(key);
        while(curr!=NULL && !curr->isLeaf()){
            Node* next = curr->children[index];
            ++blockTransfers;
            // Check if next child has enough keys
            if(next->keys.size() < (B/2)){
                Node* leftSibling = (index > 0) ? curr->children[index - 1] : NULL;
                Node* rightSibling = (index < curr->children.size()-1) ? curr->children[index + 1] : NULL;
                // Borrow from left sibling
                if(leftSibling != NULL && leftSibling->keys.size() >= (B/2)) {
                    next->borrowLeft(leftSibling);
                    ++blockTransfers;
                }
                // Borrow from right sibling
                else if(rightSibling != NULL && rightSibling->keys.size() >= (B/2)) {
                    next->borrowRight(rightSibling);
                    ++blockTransfers;
                }
                //Merging next child with one of his siblings
                else{
                    next->merge();
                    blockTransfers += 2;
                    // Tree shrinks
                    if(curr == root && curr->keys.empty()) {
                        curr->children[0]->setParent(NULL);
                        root = curr->children[0]; // new root
                        curr->children.erase(curr->children.begin());
                        for(int i=1; i<root->children.size(); i++) root->children[i]->setParent(root);
                    }
                }
            }
            //Delete from internal node
            if(curr->keys[index] == key) deleteFromInternal(curr, index, key, next, &blockTransfers);

            curr = next;
            index = curr->findChild(key);
        }
        // Will have reached a leaf here
        if(curr != NULL && curr->isLeaf() && curr->keys[index] == key){
            curr->keys.erase(curr->keys.begin()+index);
            //printf("Deleting in a B-tree of height %f with %i elements and B = %i required %i block transfers\n", ceil((double) log2(N)/log2(B)), N, B, blockTransfers);
            --N;
        } else {
            std::cout << "Key " << key << " not in tree!\n";
            exit(0);
        }
    }

    int predecessor(int key){
        if(root != NULL){
            int index = root->findChild(key);
            Node* curr = root->children[index];
            // Find leaf
            while(!curr->isLeaf()){
                curr = curr->children[curr->findChild(key)];
                index = curr->findChild(key);
            }
            // Go up in tree if necessary
            if(curr->isLeaf() && index == 0){
                while(index == 0){
                    curr = curr->parent;
                    index = curr->findChild(key);
                }
            }
            return curr->keys[curr->findChild(key)-1];
        }

        return 0;
    }

    std::vector<int> range(int x, int y){
        std::vector<int> predecessors{};
        int pred = predecessor(y+1);
        while(pred >= x){
            predecessors.insert(predecessors.begin(), pred);
            pred = predecessor(pred);
        }

        return predecessors;
    }
};