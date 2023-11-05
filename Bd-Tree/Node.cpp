#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#define DELETE 0
#define INSERT 1
#define BLANK 2


struct Message{
    int key;
    int op;
};

class Node{
        public:
        Node* parent;
        std::vector<Node*> children;
        std::vector<int> keys;
        std::vector<Message> buffer;
        std::vector<int> maxVect, minVect; // Stores the maximum/minimum size of a leaf
        std::vector<int> maxIndex, minIndex; // Indicates which child contains the subtree with max/min leaf


        Node(Node* parentIn, std::vector<Node*> childrenIn, std::vector<int> keysIn): parent(parentIn), children(childrenIn), keys(keysIn){
            //DISK-WRITE
            buffer = std::vector<Message>{};
            maxVect = {0}, minVect = {0}, maxIndex = {0}, minIndex = {0};
        }
        
        Node(Node* parentIn, std::vector<Node*> childrenIn, std::vector<int> keysIn, std::vector<Message> buffIn): parent(parentIn), 
            children(childrenIn), keys(keysIn), buffer(buffIn){
            //DISK-WRITE
            maxVect = {0}, minVect = {0}, maxIndex = {0}, minIndex = {0};
        }
        
        /*
        * Modified code from Stackoverflow. This function prints the tree 
        * horizontaly (easier to read for big trees).
        * Original code: https://stackoverflow.com/questions/36802354/print-binary-tree-in-a-pretty-way-using-c
        */
        void printBT(const std::string& prefix, bool isLeft){
            std::cout << prefix;

            std::cout << (isLeft ? "├──" : "└──" );

            // print the value of the node
            for (int i=0; i<keys.size(); i++){
                std::cout << keys[i] << " ";
            }
            std::cout << "Buffer: [ ";
            for(Message msg:buffer) std::cout << msg.key << " "; 
            
            std::cout << "]" << std::endl;

            // enter the next tree level
            if(!isMicroLeaf()){
                children[0]->printBT(prefix + (isLeft ? "│   " : "    "), true);
                for(int i=1; i<children.size(); i++){
                    children[i]->printBT(prefix + (isLeft ? "│   " : "    "), false);
                }                
            }

        }

        void printKeys(){ for(int key: keys) std::cout << key << " ";
                            std::cout << std::endl;}
        
        void setParent(Node* parentIn){ parent = parentIn;}

        bool isMicroLeaf(){ return children.empty();}

        bool isMicroRoot() {
            if(!isMicroLeaf()){ // Avoid seg fault
                return children[0]->isMicroLeaf();
            }
            return false;
        }

        int getLeafSize(){
            int res = 0;
            for(Node* child : children){
                for(int key : child->keys){
                    bool deleted = false;
                    for(Message msg : buffer) { // Annihilate matching INS/DEL updates
                        if(msg.key == key && msg.op == DELETE){
                            deleted = true;
                            break;
                        }
                    }
                    if(!deleted) ++res;
                }
            }

            for(Message msg : buffer) {
                if(msg.op != DELETE) ++res;
            }
            //printf("Leaf size: %i\n", res);
            return res;
        }

        int getMaxLeafIndex() {
            int maxElement = maxVect[0];
            int maxIndex = 0;

            for (int i = 1; i < maxVect.size(); ++i) {
                if (maxVect[i] > maxElement) {
                    maxElement = maxVect[i];
                    maxIndex = i;
                }
            }
            return maxIndex;
        }

        int getMinLeafIndex() {
            int minElement = minVect[0];
            int minIndex = 0;

            for (int i = 1; i < minVect.size(); ++i) {
                if (minVect[i] < minElement) {
                    minElement = minVect[i];
                    minIndex = i;
                }
            }
            return minIndex;
        }



        void insertChild(Node* child, int index) {children.insert(children.begin()+index, child);}

        void insertKey(int key, int index) {keys.insert(keys.begin()+index, key);}

        double log2B(int a, int B){ return pow(log2(a), 2) / pow(log2(B), 2);}

        double logB(int a, int B) {return log2(a) / log2(B);}

        int findChild(int k){
            int n = keys.size();
            int i=0; 
            while(i<n && keys[i] < k) ++i;
            return i;
        }

        int myIndex(){
            return parent->findChild(keys[0]);
        }

        int findFlushingChild(){
            if(keys.empty()) return 0;
            int resArray[children.size()] = {0};
            for(Message msg : buffer){
                ++resArray[findChild(msg.key)];
            }
            int resIndex = 0;
            int maxCount = 0;
            for(int c = 0; c < children.size(); ++c){
                if(resArray[c] > maxCount){
                    maxCount = resArray[c];
                    resIndex = c;
                }
            }
            return resIndex;
        }

        Node* getLeftSibling(){
            int index = parent->findChild(keys[0]);
            if(index == 0) return this;
            return parent->children[index-1];
        }

        Node* getRightSibling(){
            int index = parent->findChild(keys[0]);
            if(index == parent->keys.size()) return this;
            return parent->children[index+1];
        }

        void annihilateMatching(){
            for(auto msg = buffer.begin(); msg != buffer.end();){
                bool found = false;
                for(auto msg2 = msg+1; msg2 != buffer.end();){
                    if(msg2->key == msg->key && msg2->op != msg->op){
                        msg2 = buffer.erase(msg2);
                        found = true;
                        break;
                    } else msg2++;
                }
                msg = (found) ? buffer.erase(msg) : ++msg;
            }
        }

        void updateAux(){
            int index = myIndex();
            printf("Updating node: %i \n", index);
            if(isMicroRoot()) {    
                int size = getLeafSize();
                if(index > maxVect.size()){
                    maxVect.push_back(size);
                    maxIndex.push_back(index);
                    minVect.push_back(size);
                    minIndex.push_back(index);
                } else {
                    maxVect[index] = size;
                    minVect[index] = size;
                }
            } else if(!isMicroLeaf()) {
            }

        }

        void updateAux1(){
            for(int i=0; i<children.size(); ++i){
                if(i > maxVect.size()){
                    maxIndex.push_back(i);
                    maxVect.push_back(children[i]->getLeafSize());
                } else maxVect[i] = children[i]->getLeafSize();
            }
        }

        /*
         *------------------------------------------------------------------------
                                    INSERTION
         *------------------------------------------------------------------------
        */

        void split(){
            // Idea: keep "this" as new left node and only create new right node + delete appropriate children/keys from "this"
            int half = keys.size()/2;
            int keyUp = keys[half];
            // Push key to parent (or create new root)
            if(parent != NULL){
                int parentIndex = parent->findChild(keyUp);
                parent->insertKey(keyUp, parentIndex);
            } else parent = new Node(NULL, {this}, {keyUp}); // I/O
            std::vector<Node*> rightChildren = {};
            std::vector<int> rightKeys = {keys.begin()+half+1, keys.end()};
            // Handle children
            if(!isMicroLeaf()) {
                rightChildren = {children.begin()+half+1, children.end()};
                children.erase(children.begin()+half+1, children.end()); // Delete from "new left" node
            }
            keys.erase(keys.begin()+half+isMicroLeaf(), keys.end()); // Keep key if node is micro-leaf (since we use duplicates)
            Node* newRight = new Node(parent, rightChildren, rightKeys); // I/O
            // Update children's parent
            for(Node* child : newRight->children){
                if(child != NULL) child->setParent(newRight);
            }
            // Split buffer
            for(auto iter = buffer.begin(); iter != buffer.end();){
                if(iter->key > keyUp){
                    newRight->buffer.push_back(*iter);
                    iter = buffer.erase(iter);
                } else ++iter;
            }

            int rightIndex = parent->findChild(newRight->keys[0]);
            parent->insertChild(newRight, rightIndex);
            //updateAux();
            //newRight->updateAux();
        }

        /*
         *------------------------------------------------------------------------
                                    DELETION
         *------------------------------------------------------------------------
        */

        void merge(int threshold){
            Node* sibling = getLeftSibling();
            int keyIndex = 0, childIndex = 0, buffIndex = 0;
            if(sibling == this){ //|| sibling->keys.size() > threshold
                sibling = getRightSibling();
                keyIndex = keys.size();
                childIndex = children.size();
                buffIndex = buffer.size();
            }
            //if(sibling->keys.size() > threshold) return; // Don't merge if sibling has enough keys
            // Merging keys and children into current node
            keys.insert(keys.begin()+keyIndex, sibling->keys.begin(), sibling->keys.end());
            for(Node* child : sibling->children){
                if(child != NULL) child->setParent(this); // Update parent of merged node
            }
            children.insert(children.begin()+childIndex, sibling->children.begin(), sibling->children.end());

            // Merge buffer into current buffer
            buffer.insert(buffer.begin()+buffIndex, sibling->buffer.begin(), sibling->buffer.end());
            sibling->buffer.clear();

            // Deleting merged node from it's parent's children
            int keyDownIndex = std::max(myIndex()-1, 0);
            if(keyDownIndex == parent->keys.size()) --keyDownIndex;
            int mergedIndex = parent->findChild(sibling->keys[0]);
            parent->children.erase(parent->children.begin()+mergedIndex);

            //Move median key in merged node
            if(!isMicroLeaf()) {    
                int insertIndex = findChild(parent->keys[keyDownIndex]);
                int keyDown = parent->keys[keyDownIndex];
                keys.insert(keys.begin()+insertIndex, keyDown);
            }
            parent->keys.erase(parent->keys.begin()+keyDownIndex);
            //updateAux();   
        }
    };