#include <fstream>
#include <cstdlib>
#include "Node.cpp"

#define DELETE 0
#define INSERT 1
#define BLANK 2

class BeTree{
    public:
    unsigned B;
    float eps;
    unsigned Beps;
    Node* root;
    unsigned blockTransfers;
    int N;

    BeTree(unsigned BIn, float epsIn): B(BIn), eps(epsIn){
        root = new Node(NULL, {}, {});
        Beps = std::floor(pow(B, eps));
        blockTransfers = 0;
        N = 0;
        // std::cout << "B^e: " << Beps << std::endl;
        // std::cout << "B - B^e: " << B - Beps << std::endl;
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
        std::ofstream dotFile("betree.dot");
        dotFile << "digraph BTree {" << std::endl;
        //dotFile << "rankdir=TB;" << std::endl;
        dotFile << "node [shape = record,height=.5];" << std::endl;
        dotFile << "ranksep = 3.0;" << std::endl;
        dotFile << "splines = false;" << std::endl;
        generateDotNode(root, dotFile);
        dotFile << "}" <<std::endl;

        // Generate SVG file
        gernerateSVGFromDot("betree.dot", "betree.svg");
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

    void fixLeaf(Node* node){
        if(node->tooBig(B, Beps)) fixBigLeaf(node);
        else if (node->tooSmall(B, Beps)) fixSmallLeaf(node);
    }

    void fixBigLeaf(Node* node){
        Node* curr = node;
        while(curr && curr->tooBig(B, Beps)){
            curr->split();
            curr = curr->parent;
            blockTransfers += 2;
        }
        // Fixing root case
        if(root->parent != NULL) root = root->parent;
    }

    void fixSmallLeaf(Node* node){
        Node* curr = node;
        while(curr && curr->tooSmall(B, Beps) && curr->parent){
            Node* leftSibling = curr->getLeftSibling(); //DISK READ
            Node* rightSibling = curr->getRightSibling(); //DISK READ
            blockTransfers += 2;
            // Borror from left sibling
            if(leftSibling != curr && leftSibling && leftSibling->bigEnough(B, Beps)) curr->borrowLeft(leftSibling);
            // Borrow from right sibling
            else if(rightSibling != curr && rightSibling && rightSibling->bigEnough(B, Beps)) curr->borrowRight(rightSibling);
            // Merge with one of the sibling
            else curr->merge();

            // Update parent key if needed
            for(int i = 0; i<curr->keys.size(); ++i){
                if(!curr->isLeaf() && curr->children[0]->isLeaf() && curr->keys[i] != curr->children[i]->keys.back()) 
                                            curr->keys[i] = curr->children[i]->keys.back();
            }
            // Check buffer overflow
            bool cascades = false;
            while(curr->buffer.size() > B-Beps){
                printf("Flushing cascades after merge!\n");
                cascades = true;
                flush(curr);
            }
            curr = curr->parent;
        }
        // Fixing root case
        if(root->keys.size()==0) {
            root->children[0]->buffer.insert(root->children[0]->buffer.begin(), root->buffer.begin(), root->buffer.end());
            root->buffer.clear();
            root = root->children[0];
            root->setParent(NULL);
            for(int i=1; i<root->children.size(); i++) root->children[i]->setParent(root);
            while(root->buffer.size() > B-Beps) flush(root);
        }

    }

    void apply(Message msg, Node* node){
        int index = node->findChild(msg.key);
        switch(msg.op){
            case DELETE: 
                if(node->keys[index] = msg.key) {
                    node->keys.erase(node->keys.begin()+index);
                    --N;
                }
                else printf("Key %i node in tree\n", msg.key);
                break;
            case INSERT: 
                node->insertKey(msg.key, index);
                ++N;
            default: break;
        }
    }

    void flush(Node* node, bool cascades=false){
        // Flushes at least O(B^(1-eps)) updates (pigeonhole principle)
        int childIndex = node->findFlushingChild();
        Node* child = node->children[childIndex];
        ++blockTransfers;
        for(auto it = node->buffer.begin(); it != node->buffer.end();){
            if(node->findChild(it->key) == childIndex){
                Message msg = *it;
                it = node->buffer.erase(it);
                if(child->isLeaf()) apply(msg, child);
                else {
                    auto it = std::upper_bound(child->buffer.cbegin(), child->buffer.cend(), msg);
                    child->buffer.insert(it, msg);
                }
            } else ++it;
        }
        if(!child->isLeaf()) child->annihilateMatching(); // Annihilate matching ins/del operations
        fixLeaf(child);
        if(!cascades && blockTransfers > ceil((double) log2(N)/log2(B))) printf("Height: %f, blocktransfers: %i\n", ceil((double) log2(N)/log2(B)), blockTransfers);
        if(!cascades) blockTransfers=0;
        //printf("Applying %f updates in a Be-tree of height %f required %i block transfers\n", ceil((double) pow(B, 1-eps)), ceil((double) log2(N)/log2(B)), blockTransfers);
        // Flush child if needed
        while(child->buffer.size() > B-Beps) flush(child);

    }

    void insertUpdate(int key, int op){
        Message msg{key, op};
        // Insert in the root buffer
        if(root->isLeaf()) {
            apply(msg, root); // If root is a leaf -> instantly apply update
            fixLeaf(root);
        }
        else{ // If root is not a leaf -> add msg to its buffer
            auto it = std::upper_bound(root->buffer.cbegin(), root->buffer.cend(), msg);
            root->buffer.insert(it, msg);
            if(root->buffer.size() > B-Beps) flush(root);         
        }           
    }

    int predecessor(int key){
        if(root != NULL){
            int index = root->findChild(key);
            Node* curr = root->children[index];
            std::vector<Message> pending; 
            while(!curr->isLeaf()){
                // Store pending updates
                for(std::vector<Message>::iterator it = curr->buffer.begin(); it != curr->buffer.end();){
                    if(it->key == key) {
                        pending.push_back(*it);
                        it = curr->buffer.erase(it);
                    } else it++;
                }
                curr = curr->children[curr->findChild(key)];
                index = curr->findChild(key);
                // Go up in tree if necessary
                if(curr->isLeaf() && index == 0){
                    while(index == 0){
                        curr = curr->parent;
                        index = curr->findChild(key);
                    }
                    curr = curr->children[index-1];
                }
            }
            // Insert pending updates in current buffer
            curr->buffer.insert(curr->buffer.begin(), pending.begin(), pending.end());
            curr->annihilateMatching();
            for(Message msg : pending) apply(msg, curr); // apply pending updates

            return curr->keys[curr->findChild(key)-1];
        }
        return -1;
    }
};