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
        Beps =  static_cast<int>(std::ceil(pow(B, eps)));
        blockTransfers = 0;
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
        // Check if node exists
        if (node) {
            // Generate label for the node
            std::string nodeLabel = "node_" + std::to_string(reinterpret_cast<uintptr_t>(node));

            // Start node definition
            dotFile << nodeLabel << " [label=<<table border=\"0\" cellborder=\"1\" cellspacing=\"0\">";

            // Write "Buffer" label
            if (!node->buffer.empty()) {
                dotFile << "<tr><td><b>Buffer:</b></td>";

                // Write buffer content
                for (size_t i = 0; i < node->buffer.size(); ++i) {
                    dotFile << "<td>";
                    if(node->buffer[i].op == 0) dotFile << "DEL: ";
                    else dotFile << "INS: ";
                    dotFile << node->buffer[i].key << "</td>";
                }
                dotFile << "</tr>";
            }

            // Write "Keys" label
            dotFile << "<tr><td><b>Keys:</b></td>";

            // Write keys of the node
            for (size_t i = 0; i < node->keys.size(); ++i) {
                dotFile << "<td>" << node->keys[i] << "</td>";
            }
            dotFile << "</tr>";

            // End node definition
            dotFile << "</table>>, shape=plaintext];" << std::endl;

            // Recursively call generateDotNode for children
            for (size_t i = 0; i < node->children.size(); ++i) {
                generateDotNode(node->children[i], dotFile);
                // Generate edges between parent and children
                dotFile << "\"" << nodeLabel << "\" -> \"node_" << reinterpret_cast<uintptr_t>(node->children[i]) << "\";" << std::endl;
            }
        }
    }


    void fixRootMerge(){
        if(root->keys.size()==0 && !root->isLeaf()) {
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
        if(!root->isLeaf() && root->children[0]->isLeaf()){
            for(Node* child : root->children){
                root->buffer.insert(root->buffer.begin(), child->buffer.begin(), child->buffer.end());
                child->buffer.clear();                
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
        fixRootSplit();
    }

    void fixSmallLeaf(Node* node){
        Node* curr = node;
        while(curr && curr->tooSmall(B, Beps) && curr->parent){
            Node* leftSibling = curr->getLeftSibling(); //DISK READ
            Node* rightSibling = curr->getRightSibling(); //DISK READ
            // Borror from left sibling
            if(leftSibling != curr && leftSibling && leftSibling->bigEnough(B, Beps)) curr->borrowLeft(leftSibling);
            // Borrow from right sibling
            else if(rightSibling != curr && rightSibling && rightSibling->bigEnough(B, Beps)) curr->borrowRight(rightSibling);
            // Merge with one of the sibling
            else curr->merge();

            // Check buffer overflow
            curr->annihilateMatching();
            while(curr->buffer.size() > B-Beps && !curr->isLeaf()){
                flush(curr);
            }
            curr = curr->parent;
            blockTransfers += 2;
        }
        // Fixing root case
        fixRootMerge();
    }

    void apply(Message msg, Node* node){
        int index = node->findChild(msg.key);
        switch(msg.op){
            case DELETE: 
                //printf("Removing %i\n", msg.key);
                if(node->keys[index] == msg.key) {
                    node->keys.erase(node->keys.begin()+index);
                    --N;
                    //printf("Required %i blocktransfers\n", blockTransfers);
                }
                else {
                    // printTree();
                    printf("Key %i not in tree\n", msg.key);
                    node->printKeys();
                    node->parent->printKeys();
                    //exit(0);
                }
                break;
            case INSERT:
                //printf("Inserting %i\n", msg.key); 
                node->insertKey(msg.key, index);
                ++N;
            default: break;
        }
    }

    void flush(Node* node){
        // Flushes at least O(B^(1-eps)) updates (pigeonhole principle)
        int childIndex = node->findFlushingChild();
        Node* child = node->children[childIndex];
        ++blockTransfers;
        bool found = false;
        for(auto it = node->buffer.begin(); it != node->buffer.end();){
            if(node->findChild(it->key) == child->myIndex()){
                Message msg = *it;
                it = node->buffer.erase(it);
                if(child->isLeaf()) {
                    apply(msg, child);
                    fixLeaf(child);
                    // Reset variables (in case of merge/split)
                    it = node->buffer.begin();
                    childIndex = node->findChild(child->keys[0]);
                    child = node->children[childIndex];
                }
                else {
                    auto it2 = std::upper_bound(child->buffer.begin(), child->buffer.end(), msg);
                    child->buffer.insert(it2, msg);
                }
            } else ++it;
        }
        if(!child->isLeaf()) child->annihilateMatching(); // Annihilate matching ins/del operations
        // Flush child if needed
        while(child->buffer.size() > B-Beps) flush(child);
    }

    int insertUpdate(int key, int op){
        Message msg{key, op};
        // Insert in the root buffer
        auto it = std::upper_bound(root->buffer.begin(), root->buffer.end(), msg);
        root->buffer.insert(it, msg);
        root->annihilateMatching();
        while(root->buffer.size() > B-Beps) {
            if(root->isLeaf()){ // Apply update if root is a leaf
                Message msg = root->buffer.front();
                root->buffer.erase(root->buffer.begin());
                apply(msg, root);
                fixLeaf(root);
            } else flush(root);
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