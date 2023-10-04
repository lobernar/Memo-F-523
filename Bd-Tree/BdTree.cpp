#include "Node.cpp"

#define DELETE 0
#define INSERT 1
#define UPDATE 2
#define BLANK 3

class BeTree{
    public:
    unsigned B;
    float delta;
    unsigned Bdelta;
    Node* root;
    std::vector<Message> initBuff;
    int N; // Number of elements in the tree
    int Nold;

    BeTree(unsigned BIn, float deltaIn): B(BIn), delta(deltaIn){
        root = NULL;
        N = 0;
        Nold = 0;
        Bdelta = std::floor(pow(B, delta));
        std::cout << "B^e: " << Bdelta << std::endl;
        std::cout << "B - B^e: " << B - Bdelta << std::endl;
    }

    void printTree(){
        if(root != NULL) root->printBT("", false);
    }

    double log2B(int a){ return pow(log2(a), 2) / pow(log2(B), 2);}

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
        ++N;
        Node* curr = node;
        // Check if node needs to be split
        while(curr && curr->tooBig(B, Bdelta, N)){
            int half = curr->isLeaf() ? B/2 : Bdelta/2;
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
        --N;
        // Update parent key if needed
        if(node->isLeaf() && node != root) node->updateParent(key);
        Node* curr = node;
        // Check if node becomes too small
        while(curr && curr->tooSmall(B, Bdelta, N) && curr->parent){
            Node* leftSibling = curr->getLeftSibling(); //DISK READ
            Node* rightSibling = curr->getRightSibling(); //DISK READ
            // Borror from left sibling
            if(leftSibling != curr && leftSibling && leftSibling->bigEnough(B, Bdelta, N)){
                curr->borrowLeft(leftSibling);
            }
            // Borrow from right sibling
            else if(rightSibling != curr && rightSibling && rightSibling->bigEnough(B, Bdelta, N)){
                curr->borrowRight(rightSibling);
            }
            // Merge with one of the sibling
            else{
                if(leftSibling != curr) curr->merge(leftSibling, 0, 0, 0);
                else curr->merge(rightSibling, curr->keys.size(), curr->children.size(), curr->buffer.size());
                // Check buffer overflow after merge
            }
            // Update parent key if needed
            for(int i = 0; i<curr->keys.size(); ++i){
                if(!curr->isLeaf() && curr->children[0]->isLeaf() && curr->keys[i] != curr->children[i]->keys.back()) 
                                            curr->keys[i] = curr->children[i]->keys.back();
            }
            //while(curr->buffer.size() >= B/Bdelta) flush(curr);
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
                    while(root->buffer.size() > B/Bdelta) flush(root);
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
        for(int i=0; i<node->buffer.size(); ++i){
            Message msg = node->buffer[i];
            int childIndex = node->findChild(msg.key);
            Node* child = node->children[childIndex];
            node->buffer.erase(node->buffer.begin()+i);
            if(child->isLeaf()) apply(msg, child); // Next child is a leaf -> instantly apply update
            else {
                child->buffer.push_back(msg);
                while(child->buffer.size() >= B/Bdelta) flush(child);
            }
        }
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
                if(root->buffer.size() >= B/Bdelta) flush(root);            
            }           
        }
    }

    int backgroundProcess(int step, bool splitPhase){
        // Split/merge phase
        Node* n1 = root;
        if(splitPhase){
            while(!n1->isLeaf()) n1 = n1->children[n1->maxSizeIndex];
            if(n1->keys.size() >= 4*B*log2B(N)) n1->split(n1->keys.size()/2);
            
        } else{
            while(!n1->isLeaf()) n1 = n1->children[n1->minSizeIndex];
            if(n1->keys.size() <= 2*B*log2B(N)){
                Node* sibling = n1->getLeftSibling();
                if(sibling != n1 && sibling) n1->merge(sibling, 0, 0, 0);
                else{
                    sibling = n1->getRightSibling();
                    n1->merge(sibling, n1->keys.size(), n1->children.size(), n1->buffer.size());
                    if(n1->tooBig(B, Bdelta, N)) n1->split(n1->keys.size()/2);
                }
            }
        }

        // Propagate splits/merges up
        while(n1){
            n1 = n1->parent;
            if(splitPhase && n1->tooBig(B, Bdelta, N)) n1->split(n1->keys.size()/2);
            else if(!splitPhase && n1->tooSmall(B, Bdelta, N)){
                Node* sibling = n1->getLeftSibling();
                if(sibling != n1 && sibling) n1->merge(sibling, 0, 0, 0);
                else{
                    sibling = n1->getRightSibling();
                    n1->merge(sibling, n1->keys.size(), n1->children.size(), n1->buffer.size());
                    if(n1->tooBig(B, Bdelta, N)) n1->split(n1->keys.size()/2);
                }
            }
            //TODO: update auxiliary information
            Node* n2 = n1;
            // Repeated flushing of split/merged node
            Node* n3;
            while(!n2->isLeaf() && n2->buffer.size() > B/Bdelta){
                n3 = n2;
                // Propagate flush of split/merged node down
                while(n3->buffer.size() > B/Bdelta){
                    int flushChild = n3->findFlushingChild();
                    Node* n4 = n3->children[flushChild];
                    flush(n3);
                    n3 = n4;
                }
                // Update auxiliary information
                while(n3){
                    //TODO
                    n3 = n3->parent;
                }
            }
        }

        // Flush from root
        Node* n5 = root;
        while(n5->buffer.size() > B/Bdelta){
        int flushChild = n5->findFlushingChild();
        Node* n6 = n5->children[flushChild];
        flush(n5);
        n5 = n6;
        }
        // Update auxiliary information
        while(n5){
            //TODO
            n5 = n5->parent;
        }
        splitPhase = !splitPhase;
        return step;
    }

    void rebuildTree(){
        Nold = N;
    }
};