#include "Node.cpp"

class BeTree{
    public:
    unsigned B;
    float delta;
    unsigned Bdelta;
    Node* root;
    std::vector<Message> initBuff;
    unsigned blockTransfers;
    int N; // Number of elements in the tree
    int Nestimate;
    int Nold;
    int leafSize;
    int ci;
    unsigned updatesCounter;
    unsigned currStep, splitMergeStep;
    bool splitPhase;
    bool mergeLeft;
    int flushChildIndex;
    Node* newRight;
    Node* mergeSibling;
    Node* tmpParent;
    Node* n1;
    Node* n2;
    Node* n3;
    Node* n4;
    Node* n5;
    Node* n6;

    BeTree(unsigned BIn, float deltaIn, int N): B(BIn), delta(deltaIn), Nestimate(N) {
        root = NULL;
        blockTransfers = 0;
        N = 0;
        Nold = 0;
        ci = 1;
        updatesCounter = 0;
        Bdelta = std::floor(pow(B, delta));
        leafSize = B*logB(Nestimate);
        currStep = 0;
        splitMergeStep = 1;
        splitPhase = true;  
        root = new Node(NULL, {}, {});
        initTree();
        printTree();
        n1 = root;
        std::cout << "B^d: " << Bdelta << std::endl;
        std::cout << "B - B^d: " << B - Bdelta << std::endl;
        printf("logB(N) = %i\n", leafSize);
        printf("B/B^d = %i\n", B/Bdelta);
        printf("B^1-2d / logBN = %i\n", (B/Bdelta*Bdelta) / leafSize);
    }

    void initTree(){
        Node* microRoot = new Node(root, {}, {});
        Node* microLeaf = new Node(microRoot, {}, {});
        microRoot->children.push_back(microLeaf);
        root->children.push_back(microRoot);
    }

    void printTree(){
        if(root != NULL) root->printBT("", false);
    }

    double log2B(int a){ return pow(log2(a), 2) / pow(log2(B), 2);}

    double logB(int a) {return log2(a)/log2(B);}

    void fixRoot(){
        if(root->parent != NULL) root = root->parent;
        else if(root->keys.size()==0){
            root->children[0]->buffer.insert(root->children[0]->buffer.begin(), root->buffer.begin(), root->buffer.end());
            root->buffer.clear();
            root = root->children[0];
            root->setParent(NULL);
            for(int i=1; i<root->children.size(); i++) root->children[i]->setParent(root);
        }
    }

    void insert(Node* node, int key){
        int index = node->findChild(key);
        node->insertKey(key, index);
        // Only micro-leafs call this method
        if(node->keys.size() >= B*logB(Nestimate)) {
            node->split();
        }
        ++N;
    }

    void remove(Node* node, int key){
        int index = node->findChild(key);
        // Key not in tree
        if(node->keys[index] != key){
            std::cout << "Key not in tree\n";
            return;
        }
        node->keys.erase(node->keys.begin()+index); // Remove from micro-leaf
        --N;
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
                }
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
        // Flushes at least O(B^(1-2delta)) updates (pigeonhole principle)
        int childIndex = node->findFlushingChild();
        Node* child = node->children[childIndex];
        for(auto it = node->buffer.begin(); it != node->buffer.end();){
            if(node->findChild(it->key) == childIndex){
                Message msg = *it;
                it = node->buffer.erase(it);
                if(child->isMicroLeaf()) apply(msg, child);
                else child->buffer.push_back(msg);
            } else ++it;
        }
        if(!child->isMicroLeaf()) child->annihilateMatching(); // Annihilate matching ins/del operations
        if(child->isMicroRoot() && child->buffer.size() > (B/Bdelta)*logB(Nestimate)) {
            flush(child);
            //printTree();
        }
    }

    void insertUpdate(int key, int op){
        Message msg{key, op};
        root->buffer.push_back(msg);
        if(root->keys.size() == 0 && root->buffer.size() > B/Bdelta){
            flush(root);
            printf("Flushing root buffer\n");
        } 
        ++updatesCounter;
        if(updatesCounter == ceil((B/(Bdelta*Bdelta)) / (ci*pow(logB(Nestimate), 3)))){
            updatesCounter = 0; // Reset counter
            //printTree();
            continueCycle();
        }
    }

    void continueCycle(){
        // Moving n1 down
        Node* n1 = root;
        while(!n1->isMicroRoot()){
            int nextIndex = (splitPhase) ? n1->maxSizeIndex : n1->minSizeIndex;
            n1 = n1->children[nextIndex];
            //printf("Max index of n1: %i", nextIndex);
        }
        printf("Moved n1 to microroot: ");
        //n1->printKeys();
        //Split merge micro-root
        if(splitPhase && n1->getLeafSize() >= 4*B*pow(logB(Nestimate), 2)) {
            printf("SPLIT CYCLE\n");
            printTree();
            n1->split();
        }
        else if(!splitPhase && !root->keys.empty() && n1->getLeafSize() <= 2*B*pow(logB(Nestimate), 2)){
            // Merging
            printf("MERGE CYCLE\n");
            Node* sibling = n1->getLeftSibling();
            mergeLeft = true;
            if(sibling == n1){
                sibling = n1->getRightSibling();
                mergeLeft = false;
            }
            if(mergeLeft) n1->merge(sibling, 0, 0, 0);
            else n1->merge(sibling, n1->keys.size(), n1->children.size(), n1->buffer.size());
            // Split if resulting node too big
            if(n1->getLeafSize() > 5*B*pow(logB(Nestimate), 2)) n1->split();
        }            
        printTree();
        // Moving n1 up
        while(n1->parent){
            n1 = n1->parent;
            if(n1->keys.size() > Bdelta) {
                printf("Splitting while moving n1 up\n");
                //n1->printKeys();
                n1->split();
                fixRoot();
            }
            else if(n1 != root && n1->keys.size() <= Bdelta/2){
                // Merging
                printf("Merging while moving n1 up\n");
                printTree();
                Node* sibling = n1->getLeftSibling();
                mergeLeft = true;
                if(sibling == n1){
                    sibling = n1->getRightSibling();
                    mergeLeft = false;
                }
                if(mergeLeft) n1->merge(sibling, 0, 0, 0);
                else n1->merge(sibling, n1->keys.size(), n1->children.size(), n1->buffer.size());
                fixRoot();
            }
            n1->updateParentAux();
            Node* n2 = n1;

            // Flush while n2 overfull
            while(n2->buffer.size() > B/Bdelta){
                printf("Flushing n2 while overfull\n");
                Node* n3 = n2;
                // Propagate flush of n3 down
                while(n3->buffer.size() > B/Bdelta){
                    printf("Progagate flush of n3 down\n");
                    int flushChildIndex = n3->findFlushingChild();
                    Node* n4 = n3->children[flushChildIndex];
                    flush(n3);
                    n3 = n4;
                }
                // Move n3 up while updating auxiliary information
                while(n3->parent){
                    printf("Moving n3 up and update auxiliary information\n");
                    n3->updateParentAux();
                    n3 = n3->parent;
                }
            }

            // Flush root buffer
            while(root->buffer.size() > B/Bdelta){
                printf("Flushing root buffer in cycle\n");
                Node* n5 = root;
                while(n5->buffer.size() > B/Bdelta){
                    printf("Flushing n5\n");
                    int flushChildIndex = n5->findFlushingChild();
                    Node* n6 = n5->children[flushChildIndex];
                    flush(n5);
                    n5 = n6;
                }
                // Move n5 up while updating auxiliary information
                while(n5->parent){
                    printf("Moving n5 up and update auxiliary information\n");
                    n5->updateParentAux();
                    n5 = n5->parent;
                }
            }
        }
        printf("Inversting splitPhase\n");
        splitPhase = !splitPhase;
    }

    void rebuildTree(){
        Nold = N;
    }
};