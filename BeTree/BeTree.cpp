#include "Node.cpp"

#define DELETE 0
#define INSERT 1
#define UPDATE 2
#define BLANK 3

class BeTree{
    public:
    unsigned B;
    float eps;
    unsigned Beps;
    Node* root;
    std::vector<Message> initBuff;

    BeTree(unsigned BIn, float epsIn): B(BIn), eps(epsIn){
        root = NULL;
        Beps = std::floor(pow(B, eps));
        std::cout << "B^e: " << Beps << std::endl;
        std::cout << "B - B^e: " << B - Beps << std::endl;
    }

    void printTree(){
        if(root != NULL) root->printBT("", false);
    }

    Node* search(int k){
        if(root == NULL){
            std::cout << "The tree is empty" << std::endl;
            return NULL;
        }
        return root->search(k);
    }

    void insert(Node* node, int key){
        int index = node->findChild(key);
        node->insertKey(key, index);
        Node* curr = node;
        // Check if node needs to be split
        while(curr && curr->tooBig(B, Beps)){
            int half = curr->isLeaf() ? B/2 : Beps/2;
            curr->split(half);
            curr = curr->parent;
        }
    }

    void remove(Node* node, int key){
        int index = node->findChild(key);
        // Key not in tree
        if(node->keys[index] != key){
            std::cout << "Key not in tree\n";
            return;
        }
        node->keys.erase(node->keys.begin()+index); // Remove from leaf
        // Update parent key if needed
        if(node->isLeaf() && node != root) node->updateParent(key);
        Node* curr = node;
        // Check if node becomes too small
        while(curr && curr->tooSmall(B, Beps) && curr->parent){
            Node* leftSibling = curr->getLeftSibling(); //DISK READ
            Node* rightSibling = curr->getRightSibling(); //DISK READ
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
                if(leftSibling != curr) curr->merge(leftSibling, 0, 0, 0);
                else curr->merge(rightSibling, curr->keys.size(), curr->children.size(), curr->buffer.size());
            }
            // Check buffer overflow
            //while(curr->buffer.size() > B-Beps) flush(curr);
            curr = curr->parent;
        }
    }

    void apply(Message msg, Node* node){
        switch(msg.op){
            case DELETE: 
                std::cout << "Removing " << msg.key << std::endl;
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

    void flush(Node* node){
        // Flushes O(B^(1-eps)) updates (pigeonhole principle)
        for(int i=0; i<node->buffer.size(); ++i){
            Message msg = node->buffer[i];
            int childIndex = node->findChild(msg.key);
            Node* child = node->children[childIndex];
            node->buffer.erase(node->buffer.begin()+i);
            if(child->isLeaf()) apply(msg, child); // Next child is a leaf -> instantly apply update
            else {
                child->buffer.push_back(msg);
                while(child->buffer.size() >= B-Beps) flush(child);
            }
        }
        // int childIndex = node->findFlushingChild();
        // Node* child = node->children[childIndex];
        // for(int i = 0; i<node->buffer.size(); ++i){
        //     Message msg = node->buffer[i];
        //     if(node->findChild(msg.key) == childIndex){
        //         node->buffer.erase(node->buffer.begin()+i);
        //         if(child->isLeaf()) apply(msg, child);
        //         else {
        //             child->buffer.push_back(msg);
        //         }
        //     }
        // }

        // // Flush child if needed (can cause flushing cascades)
        // while(child->buffer.size() > B-Beps) flush(child);
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
                std::cout << "Flushed init buffer\n";
                initBuff.clear();
            }
        }
        // Insert in the root buffer
        else {
            if(root->isLeaf()) apply(msg, root); // If root is a leaf -> instantly apply update
            else{ // If root is not a leaf -> add msg to its buffer
                root->buffer.push_back(msg);
                if(root->buffer.size() >= B-Beps) flush(root);            
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
        std::vector<int> predecessors{};
        int pred = predecessor(y+1);
        while(pred >= x){
            predecessors.insert(predecessors.begin(), pred);
            pred = predecessor(pred);
        }

        return predecessors;
    }

};