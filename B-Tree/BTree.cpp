#include <fstream>
#include <algorithm>
#include "Node.cpp"

class BTree{

    public:
    int B;
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

    void generateSVG(const std::string& dotFileName, const std::string& svgFile){
        // Generate DOT file
        generateDotFile(dotFileName);
        // Construct the command to run the dot utility
        std::string command = "dot -Tsvg " + dotFileName + " -o " + svgFile;

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
    void generateDotFile(const std::string& dotFileName){
        std::ofstream dotFile(dotFileName);
        dotFile << "digraph BTree {" << std::endl;
        //dotFile << "rankdir=TB;" << std::endl;
        dotFile << "node [shape = record,height=.5];" << std::endl;
        dotFile << "ranksep = 3.0;" << std::endl;
        dotFile << "splines = false;" << std::endl;
        generateDotNode(root, dotFile);
        dotFile << "}" <<std::endl;
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

    int insert(int key){
        int blockTransfers = 0;
        // Empty tree
        if(root == NULL) {
            root = new Node(NULL, std::vector<Node*>{}, std::vector<int>{key});
            return ++blockTransfers;
        } // Handle root overflow
        else if(root->keys.size() > std::max(B, 2)) {
            root->split();
            root = root->parent;
            blockTransfers += 2;
        }
        // Root node is already created -> find appropriate leaf
        Node* curr = root;
        int index = curr->findChild(key);
        // Check for nodes to split
        while(!curr->isLeaf() && curr != NULL){
            Node* next = curr->children[index];
            ++blockTransfers;
            // Check if split is needed + Update variables
            if(next->keys.size() > std::max(B, 2)){
                next->split();
                blockTransfers += 2;
                curr = next->parent;
            } else curr = next;
            index = curr->findChild(key);
        }
        curr->insertKey(key, index); // Will have reached a leaf here
        ++N;
        return blockTransfers;
    }

    void fixSmall(Node* node, int& blockTransfers){
        Node* curr = node;
        while(curr && curr->parent && curr->keys.size() < static_cast<int>(ceil((double) B/2))){
            Node* leftSibling = curr->getLeftSibling();
            Node* rightSibling = curr->getRightSibling();
            // Borrow from left sibling
            if(leftSibling != NULL && leftSibling->keys.size() >= static_cast<int>(ceil((double) B/2))) {
                curr->borrowLeft(leftSibling);
                ++blockTransfers;
            }
            // Borrow from right sibling
            else if(rightSibling != NULL && rightSibling->keys.size() >= static_cast<int>(ceil((double) B/2))) {
                curr->borrowRight(rightSibling);
                ++blockTransfers;
            }
            //Merging node with one of his siblings
            else{
                curr->merge();
                blockTransfers += 2;
                // Tree shrinks
                if(root->keys.size()==0){
                    root = root->children[0];
                    root->setParent(NULL);
                    for(int i=1; i<root->children.size(); i++) root->children[i]->setParent(root);
                }
                if(curr->keys.size() > std::max(B, 2)) {
                    curr->split();
                    blockTransfers += 2;
                }
            }
            curr = curr->parent;
        }        

    }

    int remove(int key){
        int blockTransfers = 0;
        Node* curr = root;
        int keyIndex = curr->findChild(key);
        // Find node containing key to delete
        while(curr->keys[keyIndex] != key && !curr->isLeaf()){
            curr = curr->children[keyIndex];
            keyIndex = curr->findChild(key);
            ++blockTransfers;
        }
        // Reached node containing key to delete
        if(curr->isLeaf() && curr->keys[keyIndex] == key){ // Leaf node case
            curr->keys.erase(curr->keys.begin()+keyIndex);
            fixSmall(curr, blockTransfers);
        } else if(curr->keys[keyIndex] == key){ // Internal node case (Replace with predecessor key)
            Node* predecessor = curr->getPredecessor(keyIndex, blockTransfers);
            int predKey = predecessor->keys[predecessor->keys.size()-1];
            curr->keys[keyIndex] = predKey;
            predecessor->keys.pop_back();
            fixSmall(predecessor, blockTransfers);
        } else{
            printf("Key %i not in tree\n", key);
            exit(0);
        }
        return blockTransfers;
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
        int pred = predecessor(y);
        while(pred >= x){
            predecessors.insert(predecessors.begin(), pred);
            pred = predecessor(pred);
        }

        return predecessors;
    }
};