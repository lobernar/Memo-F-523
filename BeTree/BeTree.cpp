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
    int N;

    BeTree(unsigned BIn, float epsIn): B(BIn), eps(epsIn){
        root = NULL;
        Beps = std::floor(pow(B, eps));
        N = 0;
        std::cout << "B^e: " << Beps << std::endl;
        std::cout << "B - B^e: " << B - Beps << std::endl;
    }

    void printTree(){
        if(root != NULL) root->printBT("", false);
    }

    // Function to generate a DOT file for a B-tree
    void generateDotFile(Node* root, std::ofstream& dotFile) {
        if (root) {
            dotFile << "node_" << root << " [label=\"";
            for (size_t i = 0; i < root->keys.size(); ++i) {
                dotFile << root->keys[i];
                if (i < root->keys.size() - 1) {
                    dotFile << " | ";
                }
            }
            dotFile << "\"];" << std::endl;

            for (Node* child : root->children) {
                dotFile << "node_" << root << " -> node_" << child << ";" << std::endl;
                generateDotFile(child, dotFile);
            }
        }
    }

    double logB(int a) {return log2(a)/log2(B);}

    void insert(Node* node, int key, int* blockTransfers=0){
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

    void remove(Node* node, int key, int* blockTransfers=0){
        int index = node->findChild(key);
        // Key not in tree
        if(node->keys[index] != key){
            std::cout << "Key not in tree\n";
            return;
        }
        node->keys.erase(node->keys.begin()+index); // Remove from leaf
        ++blockTransfers;
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
                curr->borrowLeft(leftSibling);
            }
            // Borrow from right sibling
            else if(rightSibling != curr && rightSibling && rightSibling->bigEnough(B, Beps)){
                curr->borrowRight(rightSibling);
            }
            // Merge with one of the sibling
            else{
                curr->merge();
            }

            // Update parent key if needed
            for(int i = 0; i<curr->keys.size(); ++i){
                if(!curr->isLeaf() && curr->children[0]->isLeaf() && curr->keys[i] != curr->children[i]->keys.back()) 
                                            curr->keys[i] = curr->children[i]->keys.back();
            }
            // Check buffer overflow
            while(curr->buffer.size() > B-Beps) flush(curr);
            curr = curr->parent;
        }
        //printf("Deleting in a Be-tree of height %f with %i elements and B = %i required %i block transfers\n", ceil((double) log2(N)/log2(B)), N, B, *blockTransfers);
        --N;
        //printTree();
    }

    void apply(Message msg, Node* node, int* blockTransfers=0){
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
                //printTree();
                break;
            case INSERT: 
                insert(node, msg.key);
                // Handle new root case
                if(root->parent != NULL) root = root->parent;
                break;
            default: break;
        }
    }

    void flush(Node* node, int* blockTransfers=0){
        // Flushes at least O(B^(1-eps)) updates (pigeonhole principle)
        int childIndex = node->findFlushingChild();
        Node* child = node->children[childIndex];
        ++blockTransfers;
        for(auto it = node->buffer.begin(); it != node->buffer.end();){
            if(node->findChild(it->key) == childIndex){
                Message msg = *it;
                it = node->buffer.erase(it);
                if(child->isLeaf()) {
                    apply(msg, child, blockTransfers);
                    // Need to update variables
                    it = node->buffer.begin(); 
                    childIndex = node->findFlushingChild();
                    child = node->children[childIndex];
                } else child->buffer.push_back(msg);
            } else ++it;
        }
        if(!child->isLeaf()) child->annihilateMatching(); // Annihilate matching ins/del operations
        // Flush child if needed (can cause flushing cascades)
        while(child->buffer.size() > B-Beps) flush(child, blockTransfers);
    }

    void insertUpdate(int key, int op){
        Message msg{key, op};
        // Insert message in initial buffer (not bounded to the root node)
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
                root->buffer.push_back(msg);
                if(root->buffer.size() > B-Beps) flush(root);            
            }           
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
            for(Message msg : pending) apply(msg, curr); // apply pending updates TODO: annihilate insert-delete operations

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