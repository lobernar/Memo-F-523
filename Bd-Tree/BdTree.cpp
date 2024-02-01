#include <fstream>
#include "Node.cpp"

class BdTree{
    public:
    unsigned B;
    float delta;
    int Bdelta;
    int maxLeafSize;
    int minLeafSize;
    unsigned height;
    Node* root;
    unsigned blockTransfers;
    int N; // Number of elements in the tree
    int Nestimate;
    int Nold;
    int microLeafSize;
    int ci;
    unsigned updatesCounter;
    unsigned currStep, splitMergeStep;
    bool splitPhase;
    bool splitMerged;
    bool mergeLeft;
    int flushChildIndex;
    bool paused;
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
        microLeafSize = static_cast<int>(ceil((double) B*logB(Nestimate)));
        maxLeafSize = static_cast<int>(ceil((double)4*B*pow(logB(Nestimate), 2)));
        minLeafSize = static_cast<int>(ceil((double)2*B*pow(logB(Nestimate), 2)));
        currStep = 0;
        splitMergeStep = 1;
        splitPhase = true;  
        splitMerged = false;
        paused = false;
        height = ci*logB(Nestimate);
        root = new Node(NULL, {}, {});
        initTree();
        n1 = root;
        std::cout << "B^d: " << Bdelta << std::endl;
        std::cout << "B - B^d: " << B - Bdelta << std::endl;
        printf("logB(N) = %i\n", microLeafSize);
        printf("B/B^d = %i\n", B/Bdelta);
        printf("Max leaf size 4Blog_B(N)² = %f\n", 4*B*pow(logB(Nestimate), 2));
        printf("Min leaf size 2Blog_B(N)² = %f\n", 2*B*pow(logB(Nestimate), 2));
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

    void getMaxNodeWidth(Node* node, int* maxWidth) {
        int nodeWidth  =(node->keys.size()+1)*50;
        if(nodeWidth>*maxWidth) *maxWidth=nodeWidth;
        for(Node* child : node->children){
            int childWidth = (child->keys.size()+1)*50;
            if(childWidth>*maxWidth) *maxWidth = childWidth;
            getMaxNodeWidth(child, maxWidth);
        }
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

    double logB(int a) {return log2(a)/log2(B);}

    void fixRoot(){
        if(root->parent != NULL) root = root->parent;
        else if(root->keys.size()==0 && !root->children.empty()){
            root->children[0]->buffer.insert(root->children[0]->buffer.begin(), root->buffer.begin(), root->buffer.end());
            root->buffer.clear();
            root = root->children[0];
            root->setParent(NULL);
            for(int i=1; i<root->children.size(); i++) root->children[i]->setParent(root);
        }
    }

    void fixMicroLeaf(Node* leaf){
        if(leaf->keys.size() > microLeafSize){
            leaf->split();
        }
        else if(!root->keys.size()==0 && !leaf->parent->keys.empty() && leaf->keys.size() < ceil((double) microLeafSize/2)){
            leaf->merge(microLeafSize);
            fixRoot();
        }
    }

    void insert(Node* node, int key){
        int index = node->findChild(key);
        node->insertKey(key, index);
        ++N;
    }

    void remove(Node* node, int key){
        int index = node->findChild(key);
        // Key not in tree
        if(node->keys[index] != key){
            printf("Key %i not in tree\n", key);
            exit(0);
            return;
        }
        // Remove from micro-leaf
        node->keys.erase(node->keys.begin()+index); 
        --N;
    }

    void apply(Message msg, Node* node){
        switch(msg.op){
            case DELETE: 
                //printf("Removing %i\n", msg.key);
                remove(node, msg.key);
                // Handle empty root case
                if(root->keys.size()==0 && node != root){
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
                else {
                    auto it2 = std::upper_bound(child->buffer.cbegin(), child->buffer.cend(), msg);
                    child->buffer.insert(it2, msg);
                }
            } else ++it;
        }
        if(child->isMicroLeaf()) fixMicroLeaf(child); 
        if(!child->isMicroLeaf()) child->annihilateMatching(); // Annihilate matching ins/del operations
        if(child->isMicroRoot()) {
            Node* curr = child;
            while(curr != root) {
                curr->updateParentAux();
                curr = curr->parent;
            }
        }
        while(!child->isMicroRoot() && child->buffer.size() > B/Bdelta) flush(child); // NOT SURE ABOUT THIS
        while(child->isMicroRoot() && child->buffer.size() > (B/Bdelta)*logB(Nestimate)) flush(child);

    }

    void insertUpdate(int key, int op){
        Message msg{key, op};
        auto it = std::upper_bound(root->buffer.cbegin(), root->buffer.cend(), msg);
        root->buffer.insert(it, msg);
        while(root->buffer.size() > 2*B/Bdelta) {
            if(root->isMicroLeaf()){
                Message msg = root->buffer.front();
                root->buffer.erase(root->buffer.begin());
                apply(msg, root);
                // for(auto it = root->buffer.begin(); it != root->buffer.end();){
                //     Message msg = *it;
                //     it = root->buffer.erase(it);
                //     apply(msg, root);
                // }
            } else flush(root);
        }
        ++updatesCounter;
        if(updatesCounter == ceil((double)(B/(Bdelta*Bdelta)) / (ci*logB(Nestimate)))){ // pow(logB(Nestimate), 3)
            updatesCounter = 0; // Reset counter
            paused = false;
            cycleWait();
        }
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
                if(!n1->isMicroRoot() && !root->children.empty()){
                    int nextIndex = (splitPhase) ? n1->getMaxLeafIndex() : n1->getMinLeafIndex();
                    n1 = n1->children[nextIndex];
                    ++blockTransfers;
                    pause();
                } else {
                    ++currStep;
                    //printf("Reached micro-root\n");
                }
                break;
            case 2: // Split/Merging micro-root
                if(splitPhase && !n1->keys.empty() && n1 != root && n1->getLeafSize() > maxLeafSize) {
                    //printf("Splitting micro-root\n");
                    n1->split();
                    fixRoot();
                    splitMerged = true;
                    blockTransfers += 2;
                    ++currStep;
                    pause();
                    //printTree();
                }
                else if(!splitPhase && n1 != root && !root->keys.empty() && n1->getLeafSize() < minLeafSize){
                    // Merging
                    //printf("Merging micro-root\n");
                    n1->merge(microLeafSize/2);
                    blockTransfers += 2;
                    fixRoot();
                    for(Node* child : n1->children){
                        if(child->keys.empty()){
                            //printf("Merging empty micro-leaf after merge\n");
                            child->merge(microLeafSize);
                            fixRoot();
                        }
                    }
                    // Split if resulting node too big
                    if(n1->getLeafSize() > 5*B*pow(logB(Nestimate), 2)) {
                        n1->split();
                        fixRoot();
                        blockTransfers += 2;
                    }
                    splitMerged = true;
                    ++currStep;
                    pause();
                } else currStep = 14; // Go to last step
                break;
            case 3: // Moving n1 to it's parent
                if(n1 && n1->parent && splitMerged){
                    //printf("Moving n1 to it's parent\n");
                    n1 = n1->parent;
                    ++currStep;
                    ++blockTransfers;
                    pause();
                } else currStep = 14; // Go to last step
                break;
            case 4: // Split/merging n1 if needed
                if(splitPhase && n1->keys.size() > std::max(Bdelta, 2)) {
                    //printf("Splitting while moving n1 up\n");
                    n1->split();
                    blockTransfers += 2;
                    fixRoot();
                    ++currStep;
                    pause();
                }
                else if(!splitPhase && n1 != root && n1->keys.size() < std::max(Bdelta/2, 2)){
                    // Merging
                    //printf("Merging while moving n1 up\n");
                    n1->merge(ceil((double) Bdelta/2));
                    fixRoot();
                    blockTransfers += 2;
                    ++currStep;
                    pause();
                } else ++currStep;
                // TODO: Not sure about this
                if(n1 != root){
                    //n1->updateParentAux();
                    //printTree();
                } 
                break;
            case 5: // Set n2 to n1
                n2 = n1;
                ++currStep;
                break;
            case 6: // Repeated flushing of split/merged node
                if(!n2->isMicroRoot() && n2->buffer.size() > B/Bdelta){
                    //printf("Flushing n2 while overfull\n");
                    n3 = n2;
                    ++currStep;
                } else currStep = 10;
                break;
            case 7: // Propagating flush downward
                if(n3->buffer.size() > B/Bdelta){
                    //printf("Progagate flush of n3 down\n");
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
                    //printf("Updating aux in n3\n");
                    n3->updateParentAux();
                    n3 = n3->parent;
                    ++blockTransfers;
                    pause();
                } else currStep = 6;
                break;
            case 10: // Check root buffer
                if(root->buffer.size() > 2*B/Bdelta) {
                    n5 = root;
                    ++currStep;
                } else currStep = 3;
                break;
            case 11: // Flush from root
                if(n5->buffer.size() > 2*B/Bdelta){
                    //printf("Flushing n5 and moving down\n");
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
                //printf("Finished one cycle with %i block transfers in tree of height %f\n", blockTransfers, ceil((double) logB(N)));
                int maxblock = height+height*((Bdelta*(height+Bdelta)) + (Bdelta*(height+Bdelta)));
                //printf("Max number of block transfers: %i\n", maxblock);
                blockTransfers = 0;
                pause();
                break;
            }
            default:
                break;
            }  
        }
    }

    void pause(){ paused = true; }

    int predecessor(int q){
        Node* curr = root;
        // Go down until micro-root is reached
        std::vector<int> L = {};
        while(!curr->isMicroRoot()) curr = curr->children[curr->findChild(q)];

        // Store B*logBN greatest insertions
        for(Message msg : curr->buffer){
            if(msg.op != DELETE && msg.key <= q) {
                if(L.size() < B*logB(N)) L.push_back(msg.key);
                else{
                    int minIndex = std::min_element(L.begin(), L.end()) - L.begin();
                    if (L[minIndex] < msg.key) L[minIndex] = msg.key;
                }
            }
        }
        for(Node* child : curr->children){
            for(int key : child->keys) {
                if(key <= q){
                    if(L.size() < B*logB(N)) L.push_back(key);
                    else{
                        int minIndex = std::min_element(L.begin(), L.end()) - L.begin();
                        if (L[minIndex] < key) L[minIndex] = key;
                    }                
                } 
            }
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

        // Get BlogBN largest elements in L and L2
        std::vector<int> L3 = {};
        std::vector<int> combined = {};
        for(int i=0; i<L.size(); i++) combined.push_back(L[i]);
        for(int i=0; i<L2.size(); i++) combined.push_back(L2[i]);
        std::sort(combined.begin(), combined.end(), std::greater<int>());
        L3 = {combined.begin(), combined.begin()+B*logB(N)};

        // Find largest element in L3 not in Ld
        int res;
        for(int i=0; i<L3.size(); ++i){
            bool inLd = false;
            for(int j=0; j<Ld.size(); ++j){
                if(Ld[j] == L3[i]){
                    inLd = true;
                    break;
                }
            }
            if(!inLd){
                res = L3[i];
                break;
            }
        }
        return res;
    }

};