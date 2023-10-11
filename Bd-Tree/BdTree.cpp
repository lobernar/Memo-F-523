#include "Node.cpp"

#define DELETE 0
#define INSERT 1
#define BLANK 2

class BeTree{
    public:
    unsigned B;
    float delta;
    unsigned Bdelta;
    Node* root;
    std::vector<Message> initBuff;
    int N; // Number of elements in the tree
    int Nestimate;
    int Nold;
    int leafSize;
    unsigned nbrUpdates;
    unsigned currStep;
    bool splitPhase;
    Node* n1;
    Node* n2;
    Node* n3;
    Node* n4;
    Node* n5;
    Node* n6;

    BeTree(unsigned BIn, float deltaIn, int N): B(BIn), delta(deltaIn), Nestimate(N) {
        root = NULL;
        N = 0;
        Nold = 0;
        nbrUpdates = 0;
        Bdelta = std::floor(pow(B, delta));
        leafSize = logB(Nestimate);
        currStep = 0;
        splitPhase = true;
        root = new Node(NULL, {}, {});
        n1 = root;
        std::cout << "B^d: " << Bdelta << std::endl;
        std::cout << "B - B^d: " << B - Bdelta << std::endl;
        printf("logB(N) = %i\n", leafSize);
        printf("B^1-2d / logBN = %i\n", (B/pow(Bdelta,2)) / leafSize);
    }

    void printTree(){
        if(root != NULL) root->printBT("", false);
    }

    double log2B(int a){ return pow(log2(a), 2) / pow(log2(B), 2);}

    double logB(int a) {return log2(a)/log2(B);}

    void insert(Node* node, int key){
        int index = node->findChild(key);
        node->insertKey(key, index);
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
    }

    void insertUpdate(int key, int op){
        Message msg{key, op};
        if(root->isMicroLeaf()) apply(msg, root);
        else root->buffer.push_back(msg);
        ++nbrUpdates;
        if(nbrUpdates == ((B/pow(Bdelta,2)) / leafSize)){
            nbrUpdates = 0; // Reset counter
            continueCycle();
        }
    }

    void continueCycle(){
        switch (currStep) {
        case 0: // Move n1 to micro-leaf
            findMicroLeaf();
            printf("Step1\n");
            break;
        case 1: // Start split/merge of n1
            printf("Starting split/merge\n");
            if(splitPhase && n1->keys.size() >= 4*B*pow(leafSize, 2)) n1->split(n1->keys.size()/2, 1);
            else if (!splitPhase && n1->keys.size() <= 2*B*pow(leafSize, 2)){
                //TODO: Merge
            }
            ++currStep;
            break;
        case 2: // Finish split/merge of n1
            printf("Finish split/merge\n");
            if(splitPhase && n1->keys.size() >= 4*B*pow(leafSize, 2)) n1->split(n1->keys.size()/2, 2);
            else if (!splitPhase && n1->keys.size() <= 2*B*pow(leafSize, 2)){
                //TODO: Merge
            }
            ++currStep;
            break;
        case 3: // Propagate splits/merges up
            printf("Propagate up\n");
            if(n1->parent) n1 = n1->parent;
            ++currStep;
            break;
        case 4:
            if(splitPhase && n1->tooBig(B, Bdelta, Nestimate)){
                n1->split(n1->keys.size()/2, 1);
            } else if(!splitPhase && n1->tooSmall(B, Bdelta, Nestimate)){
                //TODO: Merge
            }
            ++currStep;
            break;
        case 5:
            if(splitPhase && n1->tooBig(B, Bdelta, Nestimate)){
                n1->split(n1->keys.size()/2, 2);
            } else if(!splitPhase && n1->tooSmall(B, Bdelta, Nestimate)){
                //TODO: Merge
            }
            ++currStep;
            break;
        case 6: // Update auxiliary information
            n1->updateAux();
            n2 = n1;
            ++currStep;
            break;
        case 7: // Find child to flush to
            if(!n2->isMicroLeaf() && n2->buffer.size() > B/Bdelta){
                n3 = n2;
                int childIndex = n3->findFlushingChild();
                n4 = n3->children[childIndex];
            }
            ++currStep;
            break;
        case 8: // Flush to child
            flush(n3);
            n3 = n4;
            if(!n3->isMicroLeaf() && n3->buffer.size() > B/Bdelta) currStep = 7;
            else ++currStep;
        case 9: // Move n3 up and update auxiliary information until the root is reached
            moveToParentAndUpdateAux(n3);
        case 10:
            n5 = root;
            ++currStep;
        case 11:
            if(!n5->isMicroLeaf() && n5->buffer.size() > B/Bdelta){
                int childIndex = n5->findFlushingChild();
                n6 = n5->children[childIndex];
            }
            ++currStep;
            break;
        case 12:
            flush(n5);
            n5 = n6;
            if(!n5->isMicroLeaf() && n5->buffer.size() > B/Bdelta) {
                currStep = 11;
                continueCycle();
            }
            else moveToParentAndUpdateAux(n5);
        case 13:
            if(root->buffer.size() > B/Bdelta) {
                currStep = 10;
                continueCycle();
            } else {
                splitPhase = !splitPhase;
                currStep = 0;
            }
        default:
            break;
        }
    }

    void findMicroLeaf(){
        while(!n1->isMicroLeaf()){
            int nextIndex = (splitPhase) ? n1->maxSizeIndex : n1->minSizeIndex;
            n1 = n1->children[nextIndex];
            return; // I/O ->stop
        }
        ++currStep;
    }

    void moveToParentAndUpdateAux(Node* node){
        while(node != root){
            node = node->parent;
            node->updateAux();
            return;
        }
        ++currStep;
    }

    void backgroundProcess(){
        // Split/merge phase
        if(splitPhase){
            while(!n1->isMicroLeaf()) {
                n1 = n1->children[n1->maxSizeIndex]; //I/O -> wait for updates
            }
            //if(n1->keys.size() >= 4*B*pow(leafSize, 2)) n1->split(n1->keys.size()/2);
        } else{
            while(!n1->isMicroLeaf()) n1 = n1->children[n1->minSizeIndex];
            if(n1->keys.size() <= 2*B*pow(leafSize, 2)){
                Node* sibling = n1->getLeftSibling();
                if (sibling == n1) sibling = n1->getRightSibling();
                if(sibling != n1 && sibling) n1->merge(sibling, 0, 0, 0);
                else{
                    sibling = n1->getRightSibling();
                    n1->merge(sibling, n1->keys.size(), n1->children.size(), n1->buffer.size());
                    //if(n1->tooBig(B, Bdelta, N)) n1->split(n1->keys.size()/2);
                }
            }
        }

        // Propagate splits/merges up
        while(n1->parent){
            n1 = n1->parent;
            //if(splitPhase && n1->tooBig(B, Bdelta, N)) n1->split(n1->keys.size()/2);
            if(!splitPhase && n1->tooSmall(B, Bdelta, N)){
                Node* sibling = n1->getLeftSibling();
                if(sibling != n1 && sibling) n1->merge(sibling, 0, 0, 0);
                else{
                    sibling = n1->getRightSibling();
                    n1->merge(sibling, n1->keys.size(), n1->children.size(), n1->buffer.size());
                    //if(n1->tooBig(B, Bdelta, N)) n1->split(n1->keys.size()/2);
                }
            }
            //TODO: update auxiliary information
            Node* n2 = n1;
            // Repeated flushing of split/merged node
            Node* n3;
            while(!n2->isMicroLeaf() && n2->buffer.size() > B/Bdelta){
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
    }

    void rebuildTree(){
        Nold = N;
    }
};