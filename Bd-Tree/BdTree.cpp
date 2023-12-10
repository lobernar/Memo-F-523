#include <fstream>
#include "Node.cpp"

class BdTree{
    public:
    unsigned B;
    float delta;
    unsigned Bdelta;
    unsigned height;
    Node* root;
    unsigned blockTransfers;
    int N; // Number of elements in the tree
    int Nestimate;
    int Nold;
    int leafSize;
    int ci;
    unsigned updatesCounter;
    unsigned currStep, splitMergeStep;
    bool splitPhase;
    bool splitMerged;
    bool mergeLeft;
    int flushChildIndex;
    bool paused;
    Node* newRight;
    Node* mergeSibling;
    Node* tmpParent;
    Node* n1;
    Node* n2;
    Node* n3;
    Node* n4;
    Node* n5;
    Node* n6;

    BdTree(unsigned BIn, float deltaIn, int N): B(BIn), delta(deltaIn), Nestimate(N) {
        root = NULL;
        blockTransfers = 0;
        N = 0;
        Nold = 0;
        ci = 1;
        updatesCounter = 0;
        Bdelta = ceil((double) pow(B, delta));
        leafSize = B*logB(Nestimate);
        currStep = 0;
        splitMergeStep = 1;
        splitPhase = true;  
        splitMerged = false;
        paused = false;
        height = ci*logB(Nestimate);
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

    void generateSVGFile(){
        std::ofstream svgFile("Bdtree.svg");
        svgFile << "<svg width=\"1700\" height=\"1000\" xmlns=\"http://www.w3.org/2000/svg\">\n";
        generateSVGNode(root, svgFile, 500, 50, 400, 60);
        svgFile << "</svg>\n";
    }

    void generateSVGNode(Node* node, std::ostream& out, int x, int y, int xOffset, int yOffset) {
        if (node) {
            int totalWidth = node->keys.size()*40;

            // Draw a single rectangle for the keys of the current node
            out << "<rect x=\"" << x - totalWidth / 2 << "\" y=\"" << y - 20 << "\" width=\"" << totalWidth << "\" height=\"40\" fill=\"white\" stroke=\"black\"/>\n";

            // Write keys inside the rectangle
            int currentX = x - totalWidth / 2 + 10; // Initial x-position inside the rectangle
            for (int i = 0; i < node->keys.size(); ++i) {
                out << "<text x=\"" << currentX << "\" y=\"" << y << "\" text-anchor=\"start\" dominant-baseline=\"middle\">" << node->keys[i] << "</text>\n";
                currentX += 40; // Adjust the spacing between keys as needed
            }

            // Draw lines to the children
            for (int i = 0; i < node->children.size(); ++i) {
                // Calculate the position of the child subtree
                int childX = x - totalWidth / 2 + i * xOffset + totalWidth / 2;
                int childY = y + yOffset;

                // Draw a line from the current node to the child
                out << "<line x1=\"" << x + i * totalWidth / node->children.size() << "\" y1=\"" << y + 20 << "\" x2=\"" << childX << "\" y2=\"" << childY - 20 << "\" stroke=\"black\"/>\n";

                // Recursively draw the child
                generateSVGNode(node->children[i], out, childX, childY, xOffset / 2, yOffset);
            }
        }
    }

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
        // Remove from micro-leaf
        node->keys.erase(node->keys.begin()+index); 
        if(node->keys.size() < B*logB(Nestimate)/2) node->merge((B*logB(Nestimate))/2);
        --N;
    }

    void apply(Message msg, Node* node){
        switch(msg.op){
            case DELETE: 
                std::cout << "Removing " << msg.key << std::endl;
                remove(node, msg.key);
                // Handle empty root case
                if(root->keys.size()==0){
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
        while(!child->isMicroRoot() && child->buffer.size() > ceil((double) B/Bdelta)) flush(child); // Not sure about this
        if(child->isMicroRoot()) {
            while(child->buffer.size() > (B/Bdelta)*logB(Nestimate)) flush(child);
            Node* curr = child;
            while(curr != root) {
                printf("Updating Aux while flushing\n");
                curr->updateParentAux();
                curr = curr->parent;
            }
        }
    }

    void insertUpdate(int key, int op){
        Message msg{key, op};
        root->buffer.push_back(msg);
        while(root->buffer.size() > B/Bdelta) flush(root);
        ++updatesCounter;
        if(updatesCounter == ceil((double)(B/(Bdelta*Bdelta)) / (ci*logB(Nestimate)))){ // pow(logB(Nestimate), 3)
            updatesCounter = 0; // Reset counter
            paused = false;
            //printTree();
            cycleWait();
            //cycle();
        }
    }

    void cycle(){
        // Moving n1 down
        Node* n1 = root;
        bool splitMerged = false;
        while(!n1->isMicroRoot()){
            int nextIndex = (splitPhase) ? n1->getMaxLeafIndex() : n1->getMinLeafIndex();
            n1 = n1->children[nextIndex];
            ++blockTransfers;
        }
        // Split/merge micro-root
        if(splitPhase && !n1->keys.empty() && n1->getLeafSize() >= 4*B*pow(logB(Nestimate), 2)) {
            n1->split();
            splitMerged = true;
        }
        else if(!splitPhase && !root->keys.empty() && n1->getLeafSize() <= 2*B*pow(logB(Nestimate), 2)){
            n1->merge((B*logB(Nestimate))/2);
            // Split if resulting node too big
            if(n1->getLeafSize() > 5*B*pow(logB(Nestimate), 2)) {
                n1->split();
            }
            splitMerged = true;
        }            
        // Moving n1 up
        while(n1->parent && splitMerged){
            n1 = n1->parent;
            ++ blockTransfers;
            if(splitPhase && n1->keys.size() > Bdelta) {
                n1->split();
                fixRoot();
            }
            else if(!splitPhase && n1 != root && n1->keys.size() <= Bdelta/2){
                // Merging
                n1->merge((B*logB(Nestimate))/2);
                fixRoot();
            }
            
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
                    ++blockTransfers;
                }
                // Move n3 up while updating auxiliary information
                while(n3 != root){
                    printf("Moving n3 up and update auxiliary information\n");
                    n3->updateParentAux();
                    n3 = n3->parent;
                    ++blockTransfers;
                }
            }

            // Flush root buffer
            while(root->buffer.size() > B/Bdelta){
                printf("Flushing root buffer in cycle\n");
                Node* n5 = root;
                while(n5->buffer.size() > B/Bdelta){
                    printf("Flushing n5 and moving down\n");
                    int flushChildIndex = n5->findFlushingChild();
                    Node* n6 = n5->children[flushChildIndex];
                    flush(n5);
                    n5 = n6;
                    ++blockTransfers;
                }
                // Move n5 up while updating auxiliary information
                while(n5 != root){
                    printf("Moving n5 up and update auxiliary information\n");
                    n5->updateParentAux();
                    n5 = n5->parent;
                    ++blockTransfers;
                }
            }
        }
        printf("Finished one cycle with %i block transfers\n", blockTransfers);
        splitPhase = !splitPhase;
        blockTransfers = 0;
    }

    void cycleWait() {
        while(!paused){
            switch (currStep) {
            case 0: // Setting n1 to root and initialize variables
                n1 = root;
                splitMerged = false;
                ++currStep;
                break;
            case 1: // Navigating to micro-root
                if(!n1->isMicroRoot()){
                    int nextIndex = (splitPhase) ? n1->getMaxLeafIndex() : n1->getMinLeafIndex();
                    n1 = n1->children[nextIndex];
                    ++blockTransfers;
                    pause();
                } else {
                    ++currStep;
                    printf("Reached micro-root\n");
                }
                break;
            case 2: // Split/Merging micro-root
                if(splitPhase && !n1->keys.empty() && n1->getLeafSize() >= 4*B*pow(logB(Nestimate), 2)) {
                    printf("Splitting micro-root\n");
                    n1->split();
                    splitMerged = true;
                    blockTransfers += 2;
                    ++currStep;
                    pause();
                }
                else if(!splitPhase && !root->keys.empty() && n1->getLeafSize() <= 2*B*pow(logB(Nestimate), 2)){
                    // Merging
                    printf("Merging micro-root\n");
                    n1->merge((B*logB(Nestimate))/2);
                    blockTransfers += 2;
                    // Split if resulting node too big
                    if(n1->getLeafSize() > 5*B*pow(logB(Nestimate), 2)) {
                        n1->split();
                        blockTransfers += 2;
                    }
                    splitMerged = true;
                    ++currStep;
                    pause();
                } else currStep = 14; // Go to last step
                break;
            case 3: // Moving n1 to it's parent
                if(n1->parent && splitMerged){
                    printf("Moving n1 to it's parent\n");
                    n1 = n1->parent;
                    ++currStep;
                    ++blockTransfers;
                    pause();
                } else currStep = 14; // Go to last step
                break;
            case 4: // Split/merging n1 if needed
                if(splitPhase && n1->keys.size() > Bdelta) {
                    printf("Splitting while moving n1 up\n");
                    n1->split();
                    blockTransfers += 2;
                    fixRoot();
                    ++currStep;
                    pause();
                }
                else if(!splitPhase && n1 != root && n1->keys.size() <= Bdelta/2){
                    // Merging
                    printf("Merging while moving n1 up\n");
                    n1->merge((B*logB(Nestimate))/2);
                    fixRoot();
                    blockTransfers += 2;
                    ++currStep;
                    pause();
                } else ++currStep;
                if(n1 != root){
                    n1->updateParentAux();
                } 
                break;
            case 5: // Set n2 to n1
                n2 = n1;
                ++currStep;
                break;
            case 6: // Repeated flushing of split/merged node
                if(!n2->isMicroRoot() && n2->buffer.size() > B/Bdelta){
                    printf("Flushing n2 while overfull\n");
                    n3 = n2;
                    ++currStep;
                } else currStep = 10;
                break;
            case 7: // Propagating flush downward
                if(n3->buffer.size() > B/Bdelta){
                    printf("Progagate flush of n3 down\n");
                    int flushChildIndex = n3->findFlushingChild();
                    n4 = n3->children[flushChildIndex];
                    ++currStep;
                    ++blockTransfers;
                    pause();
                } else currStep = 9;
                break;
            case 8: // Flushing n3
                flush(n3);
                n3 = n4;
                currStep = 7;
                break;
            case 9: // Update auxiliary information
                if(n3 != root){
                    printf("Updating aux in n3\n");
                    n3->updateParentAux();
                    n3 = n3->parent;
                    ++blockTransfers;
                    pause();
                } else currStep = 6;
                break;
            case 10: // Check root buffer
                if(root->buffer.size() > B/Bdelta) {
                    n5 = root;
                    ++currStep;
                } else currStep = 3;
                break;
            case 11: // Flush from root
                if(n5->buffer.size() > B/Bdelta){
                    printf("Flushing n5 and moving down\n");
                    int flushChildIndex = n5->findFlushingChild();
                    n6 = n5->children[flushChildIndex];
                    ++currStep;
                    ++blockTransfers;
                    pause();
                } else currStep = 13;
                break;
            case 12: // Flushing n5
                flush(n5);
                n5 = n6;
                currStep = 11;
                break;
            case 13: // Update auxiliary information of n5
                if(n5 != root){
                    n5->updateParentAux();
                    n5 = n5->parent;
                    ++blockTransfers;
                    pause();
                } else currStep = 10;
                break;
            case 14: // Update variables
            {   splitPhase = !splitPhase;
                currStep = 0;
                splitMerged = false;
                printf("Finished one cycle with %i block transfers in tree of height %f\n", blockTransfers, ceil((double) logB(N)));
                int maxblock = height+height*((Bdelta*(height+Bdelta)) + (Bdelta*(height+Bdelta)));
                printf("Max number of block transfers: %i\n", maxblock);
                blockTransfers = 0;
                break;
            }
            default:
                break;
            }  
        }
    }

    void pause(){
        //printf("I/O occured at step %i, pausing execution...\n", currStep);
        paused = true;
    }

    void predecessor(int q){
        Node* curr = root;
        std::vector<int> L = {};
        // Go down until micro-root is reached
        while(!curr->isMicroRoot()) curr = curr->children[curr->findChild(q)];
        for(Message msg : curr->buffer){
            if(msg.op != DELETE && msg.key <= q) L.push_back(msg.key);
        }
        for(Node* child : curr->children){
            for(int key : child->keys) if(key <= q) L.push_back(key);
        }
        // Visit ancestors and store insertions and deletions <= q
        std::vector<int> L2 = {};
        std::vector<int> Ld = {};
        while(curr->parent){
            curr = curr->parent;
            for(Message msg : curr->buffer){
                if(msg.op == DELETE && msg.key <= q) Ld.push_back(msg.key); // Store deletions
                else if(msg.op != DELETE && msg.key <= q) L2.push_back(msg.key); // Store pending insertions <= q
            }
        }


    }

};