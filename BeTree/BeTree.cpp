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
    int blockTransfers;
    int N;

    BeTree(unsigned BIn, float epsIn): B(BIn), eps(epsIn){
        root = new Node(NULL, {}, {});
        Beps =  static_cast<int>(std::floor(pow(B, eps)));
        blockTransfers = 0;
        N = 0;
        std::cout << "B^e: " << Beps << std::endl;
        std::cout << "B - B^e: " << B - Beps << std::endl;
        printf("B/2-1 = %i\n", static_cast<int>(ceil((double) B/2)-1));
        printf("Beps/2-1 = %i\n", static_cast<int>(ceil((double) Beps/2)-1));
    }

    void printTree(){
        if(root != NULL) root->printBT("", false);
    }

    void gernerateSVG(const std::string& dotFileName, const std::string& svgFile){
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

    void fixEmptyRoot(){
        if(root->keys.size()==0) {
            root->children[0]->buffer.insert(root->children[0]->buffer.begin(), root->buffer.begin(), root->buffer.end());
            root->buffer.clear();
            root = root->children[0];
            root->setParent(NULL);
            for(int i=1; i<root->children.size(); i++) root->children[i]->setParent(root);
            while(root->buffer.size() > B-Beps){
                if(root->isLeaf()){
                    Message msg = root->buffer.front();
                    root->buffer.erase(root->buffer.begin());
                    apply(msg, root);
                } else flush(root);
            }
        }
    }

    void fixRootSplit(){
        if(root->parent != NULL) root = root->parent;
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
        fixRootSplit();
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

            // Check buffer overflow
            while(curr->buffer.size() > B-Beps){
                printf("Flushing cascades after merge!\n");
                flush(curr, true);
            }
            curr = curr->parent;
            //++blockTransfers;
        }
        // Fixing root case
        fixEmptyRoot();
    }

    void apply(Message msg, Node* node){
        int index = node->findChild(msg.key);
        switch(msg.op){
            case DELETE: 
                //printf("Removing %i\n", msg.key);
                if(node->keys[index] == msg.key) {
                    node->keys.erase(node->keys.begin()+index);
                    --N;
                }
                else {
                    printf("Key %i not in tree\n", msg.key);
                    exit(0);
                }
                break;
            case INSERT:
                //printf("Inserting %i\n", msg.key); 
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
                if(child->isLeaf()) {
                    apply(msg, child);
                    fixLeaf(child);
                    // Reset variables (in case of merge)
                    it = node->buffer.begin();
                    childIndex = node->findChild(child->keys[0]);
                }
                else {
                    auto it2 = std::upper_bound(child->buffer.begin(), child->buffer.end(), msg);
                    child->buffer.insert(it2, msg);
                }
            } else ++it;
        }
        if(!child->isLeaf()) child->annihilateMatching(); // Annihilate matching ins/del operations
        // Flush child if needed
        while(child->buffer.size() > B-Beps) flush(child, true);
        //if(!cascades) blockTransfers=0;

    }

    int insertUpdate(int key, int op){
        Message msg{key, op};
        // Insert in the root buffer
        if(root->isLeaf()) {
            apply(msg, root); // If root is a leaf -> instantly apply update
            fixLeaf(root);
        }
        else{ // If root is not a leaf -> add msg to its buffer
            auto it = std::upper_bound(root->buffer.begin(), root->buffer.end(), msg);
            root->buffer.insert(it, msg);
            while(root->buffer.size() > B) flush(root);         
        }
        int tr = blockTransfers;
        blockTransfers = 0;
        return tr;        
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