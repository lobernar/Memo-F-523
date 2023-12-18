#include <fstream>
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
    std::vector<Message> initBuff;
    unsigned blockTransfers;
    int N;

    BeTree(unsigned BIn, float epsIn): B(BIn), eps(epsIn){
        root = NULL;
        Beps = std::floor(pow(B, eps));
        blockTransfers = 0;
        N = 0;
        std::cout << "B^e: " << Beps << std::endl;
        std::cout << "B - B^e: " << B - Beps << std::endl;
    }

    void printTree(){
        if(root != NULL) root->printBT("", false);
    }

    // Function to generate a DOT file for a B-tree
    void generateDotFile(){
        std::ofstream dotFile("betree.dot");
        dotFile << "digraph BTree {" << std::endl;
        //dotFile << "rankdir=TB;" << std::endl;
        dotFile << "node [shape = record,height=.5];" << std::endl;
        generateDotNode(root, dotFile);
        dotFile << "}" <<std::endl;
    }
    
    void generateDotNode(Node* node, std::ofstream& dotFile) {
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

    double logB(int a) {return log2(a)/log2(B);}

    void fixNode(Node* node){
        bool split = false;
        Node* curr = node;
        while(curr && curr->tooBig(B, Beps)){
            split = true;
            curr->split();
            curr = curr->parent;
            blockTransfers += 2;
        }
        if(root->parent != NULL) root = root->parent;
        printf("Fixing node too small\n");
        curr = node;
        while(!split && curr && curr->tooSmall(B, Beps) && curr->parent){
            Node* leftSibling = curr->getLeftSibling(); //DISK READ
            Node* rightSibling = curr->getRightSibling(); //DISK READ
            blockTransfers += 2;
            // Borror from left sibling
            if(leftSibling != curr && leftSibling && leftSibling->bigEnough(B, Beps)){
                printf("Borrow left\n");
                curr->borrowLeft(leftSibling);
            }
            // Borrow from right sibling
            else if(rightSibling != curr && rightSibling && rightSibling->bigEnough(B, Beps)){
                printf("Borrow right\n");
                curr->borrowRight(rightSibling);
            }
            // Merge with one of the sibling
            else{
                printf("Merge\n");
                curr->merge();
            }

            // Update parent key if needed
            for(int i = 0; i<curr->keys.size(); ++i){
                if(!curr->isLeaf() && curr->children[0]->isLeaf() && curr->keys[i] != curr->children[i]->keys.back()) 
                                            curr->keys[i] = curr->children[i]->keys.back();
            }
            // Check buffer overflow
            bool cascades = false;
            while(curr->buffer.size() > B-Beps){
                printf("Flushing cascades!\n");
                Node* tmp = curr;
                cascades = true;
                flush(tmp);
                printf("After flushing cascades\n");
            } 
            if(cascades){
                printf("After flushing cascades\n");
                curr->printKeys();
            }
            curr = curr->parent;
        }
        if(root->keys.size()==0) {
            root->children[0]->buffer.insert(root->children[0]->buffer.begin(), root->buffer.begin(), root->buffer.end());
            root->buffer.clear();
            root = root->children[0];
            root->setParent(NULL);
            for(int i=1; i<root->children.size(); i++) root->children[i]->setParent(root);
            while(root->buffer.size() > B-Beps) flush(root);
        }
    }

    void insert(Node* node, int key){
        int index = node->findChild(key);
        node->insertKey(key, index);
        ++blockTransfers; // DISK-WRITE
        Node* curr = node;
        // Check if node needs to be split
        while(curr && curr->tooBig(B, Beps)){
            curr->split();
            curr = curr->parent;
            blockTransfers += 2;
        }
        //printf("Inserting in a Be-tree of height %f with %i elements and B = %i required %i block transfers\n", ceil((double)log2(N)/log2(B)), N, B, *blockTransfers);
        ++N;
    }

    void remove(Node* node, int key){
        int index = node->findChild(key);
        printf("Deleting %i\n", key);
        // Key not in tree
        if(node->keys[index] != key){
            std::cout << "Key not in tree\n";
            return;
        }
        node->keys.erase(node->keys.begin()+index); // Remove from leaf
        ++blockTransfers; // DISK-WRITE
        // Update parent key if needed
        if(node->isLeaf() && node != root) node->updateParent(key);
        Node* curr = node;
        // Check if node becomes too small
        while(curr && curr->tooSmall(B, Beps) && curr->parent){
            Node* leftSibling = curr->getLeftSibling(); //DISK READ
            Node* rightSibling = curr->getRightSibling(); //DISK READ
            blockTransfers += 2;
            // Borror from left sibling
            if(leftSibling != curr && leftSibling && leftSibling->bigEnough(B, Beps)){
                printf("Borrow left\n");
                curr->borrowLeft(leftSibling);
            }
            // Borrow from right sibling
            else if(rightSibling != curr && rightSibling && rightSibling->bigEnough(B, Beps)){
                printf("Borrow right\n");
                curr->borrowRight(rightSibling);
            }
            // Merge with one of the sibling
            else{
                printf("Merge\n");
                curr->merge();
            }

            // Update parent key if needed
            for(int i = 0; i<curr->keys.size(); ++i){
                if(!curr->isLeaf() && curr->children[0]->isLeaf() && curr->keys[i] != curr->children[i]->keys.back()) 
                                            curr->keys[i] = curr->children[i]->keys.back();
            }
            // Check buffer overflow
            if(key < 10) {
                printTree();
                curr->printKeys();
            }
            bool cascades = false;
            while(curr->buffer.size() > B-Beps){
                printf("Flushing cascades!\n");
                Node* tmp = curr;
                cascades = true;
                flush(tmp);
                printf("After flushing cascades\n");
            } 
            if(cascades){
                printf("After flushing cascades\n");
                curr->printKeys();
            }
            curr = curr->parent;
        }
        --N;

    }

    void apply(Message msg, Node* node){
        switch(msg.op){
            case DELETE: 
                remove(node, msg.key);
                // Handle empty root case
                if(root->keys.size()==0) {
                    root->children[0]->buffer.insert(root->children[0]->buffer.begin(), root->buffer.begin(), root->buffer.end());
                    root->buffer.clear();
                    root = root->children[0];
                    root->setParent(NULL);
                    for(int i=1; i<root->children.size(); i++) root->children[i]->setParent(root);
                    while(root->buffer.size() > B-Beps) flush(root);
                }
                printf("Deleting in a Be-tree of height %f with %i elements and B = %i required %i block transfers\n", ceil((double) log2(N)/log2(B)), N, B, blockTransfers);
                break;
            case INSERT: 
                insert(node, msg.key);
                // Handle new root case
                if(root->parent != NULL) root = root->parent;
                printf("Inserting in a Be-tree of height %f with %i elements and B = %i required %i block transfers\n", ceil((double) log2(N)/log2(B)), N, B, blockTransfers);
                break;
            default: break;
        }
        blockTransfers = 0;
    }

    void flush(Node* node){
        // First apply all the updates to a node and the split/merge
        // OR Split/merge while flushing?
        // Flushes at least O(B^(1-eps)) updates (pigeonhole principle)
        int childIndex = node->findFlushingChild();
        Node* child = node->children[childIndex];
        ++blockTransfers;
        for(auto it = node->buffer.begin(); it != node->buffer.end();){
            if(node->findChild(it->key) == childIndex){
                Message msg = *it;
                it = node->buffer.erase(it);
                if(child->isLeaf()) {
                    // apply(msg, child);
                    // // Need to update variables
                    // it = node->buffer.begin(); 
                    // childIndex = node->findFlushingChild();
                    // child = node->children[childIndex];
                    int index = child->findChild(msg.key);
                    if(msg.op==INSERT) {
                        printf("Inserting key: %i\n", msg.key);
                        child->insertKey(msg.key, index);
                    }
                    else if(msg.op==DELETE) {
                        printf("Deleting %i\n", msg.key);
                        child->keys.erase(child->keys.begin()+index);
                    }
                } else {
                    auto it = std::upper_bound(child->buffer.cbegin(), child->buffer.cend(), msg);
                    child->buffer.insert(it, msg);
                }
            } else ++it;
        }
        if(!child->isLeaf()) child->annihilateMatching(); // Annihilate matching ins/del operations
        fixNode(child);
        // Flush child if needed
        while(child->buffer.size() > B-Beps){
            printf("Flushing while flushing\n");
            flush(child);
        } 
    }

    void insertUpdate(int key, int op){
        Message msg{key, op};
        // Insert message in initial buffer (not bounded to the root node)
        int blockTransfers = 0;
        if(root == NULL) {
            initBuff.push_back(msg);
            // Apply messages of the initial buffer and empty it
            if(initBuff.size() >= B) {
                root = new Node(NULL, std::vector<Node*>{}, std::vector<int>{});
                for(Message msg : initBuff) apply(msg, root);
                initBuff.clear();
            }
        }
        // Insert in the root buffer
        else {
            if(root->isLeaf()) apply(msg, root); // If root is a leaf -> instantly apply update
            else{ // If root is not a leaf -> add msg to its buffer
                auto it = std::upper_bound(root->buffer.cbegin(), root->buffer.cend(), msg);
                root->buffer.insert(it, msg);
                if(root->buffer.size() > B-Beps) flush(root);         
            }           
        }
        blockTransfers = 0;
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

    std::vector<int> range(int x, int y){
        std::vector<int> res{};
        Node* curr = root;
        while(!curr->isLeaf()){
            // Check buffer for updates to push down
            for(auto it = curr->buffer.begin(); it != curr->buffer.end();){
                Message msg = *it;
                if(msg.key >= x && msg.key <= y){ // Push update down appropriate node
                    int pushIndex = curr->findChild(msg.key);
                    curr->children[pushIndex]->buffer.push_back(msg);
                    it = curr->buffer.erase(it);
                    curr->children[pushIndex]->annihilateMatching();
                    //TODO: Flush? 
                } else ++it;
            }
            int nextIndex = curr->findChild(x);
            curr = curr->children[nextIndex];
        }
        // Will have reached the leaf containing x (if present)
        bool finished = false;
        while(!finished && curr->parent){
            if(curr->isLeaf()){ // Add keys to solution if currently in leaf
                for(int key : curr->keys){
                    if(key >= x && key <= y) res.push_back(key);
                    else if(key > y) finished = true;
                }
                curr = curr->parent;
            } else {
                bool down = false;
                for(int i = 0; i<curr->children.size(); ++i){
                    if(curr->children[i]->keys[0] > res[res.size()-1] && curr->children[i]->keys[0] <=y){
                        down = true;
                        curr = curr->children[i];
                        break;
                    }
                }
                if(!down) curr = curr->parent;
            }
        }

        return res;
    }

};