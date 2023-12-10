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


        Node(Node* parentIn, std::vector<Node*> childrenIn, std::vector<int> keysIn): parent(parentIn), children(childrenIn), keys(keysIn){
            //DISK-WRITE
            buffer = std::vector<Message>{};
            maxVect = {0}, minVect = {INT16_MAX};
        }
        
        Node(Node* parentIn, std::vector<Node*> childrenIn, std::vector<int> keysIn, std::vector<Message> buffIn): parent(parentIn), 
            children(childrenIn), keys(keysIn), buffer(buffIn){
            //DISK-WRITE
            maxVect = {0}, minVect = {INT16_MAX};
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
            
            std::cout << "] ";

            printf("Max vect: ");
            for(int val : maxVect) printf("%i, ", val);
            printf("\n");

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
            return res;
        }

        int getMaxLeafIndex() {
            return std::max_element(maxVect.begin(), maxVect.end()) - maxVect.begin();
        }

        int getMinLeafIndex() {
            return std::min_element(minVect.begin(), minVect.end()) - minVect.begin();
        }

        void insertChild(Node* child, int index) {children.insert(children.begin()+index, child);}

        void insertKey(int key, int index) {keys.insert(keys.begin()+index, key);}

        int findChild(int k){
            int n = keys.size();
            int i=0; 
            while(i<n && keys[i] < k) ++i;
            return i;
        }

        int myIndex() {return (keys.size() > 0) ? parent->findChild(keys[0]) : 0;}

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

        void updateParentAux(){
            int index = myIndex();
            if(isMicroRoot()) {    
                int size = getLeafSize();
                parent->maxVect[index] = size;
                parent->minVect[index] = size;
            } else if(!isMicroLeaf()) {
                int bigIndex = getMaxLeafIndex();
                int smallIndex = getMinLeafIndex();
                int parentMax = parent->maxVect[index];
                int parentMin = parent->minVect[index];
                parent->maxVect[index] = maxVect[bigIndex];
                parent->minVect[index] = minVect[smallIndex];                        
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

            // Handle min/max vectors
            if(!isMicroLeaf()){
                parent->maxVect.insert(parent->maxVect.begin() + rightIndex, 0);
                parent->minVect.insert(parent->minVect.begin() + rightIndex, INT16_MAX);
                if(!isMicroRoot()){
                    std::vector<int> newMaxVect = {maxVect.begin()+half+1, maxVect.end()};
                    std::vector<int> newMinVect = {minVect.begin()+half+1, minVect.end()};
                    maxVect.erase(maxVect.begin()+half+1, maxVect.end());
                    minVect.erase(minVect.begin()+half+1, minVect.end());
                    newRight->maxVect = newMaxVect;
                    newRight->minVect = newMinVect;
                }    
                updateParentAux();
                newRight->updateParentAux();    
            }
        }

        /*
         *------------------------------------------------------------------------
                                    DELETION
         *------------------------------------------------------------------------
        */

        void merge(int threshold){
            // Maybe replace leaf variable with isMicroRoot()?
            Node* sibling = getLeftSibling();
            int keyIndex = 0, childIndex = 0, buffIndex = 0, auxIndex = 0;
            if(sibling == this || (isMicroRoot() && sibling->keys.size() > threshold)){
                sibling = getRightSibling();
                keyIndex = keys.size();
                childIndex = children.size();
                buffIndex = buffer.size();
                auxIndex = maxVect.size();
            }
            if(isMicroRoot() && sibling->keys.size() > threshold) return; // Don't merge if sibling has enough keys
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
            int siblingIndex = sibling->myIndex();
            int keyDownIndex = std::max(myIndex()-1, 0);
            if(keyDownIndex == parent->keys.size()) --keyDownIndex;
            int mergedIndex = parent->findChild(sibling->keys[0]);
            parent->children.erase(parent->children.begin()+mergedIndex);

            // Move median key in merged node
            if(!isMicroLeaf()) {    
                int insertIndex = findChild(parent->keys[keyDownIndex]);
                int keyDown = parent->keys[keyDownIndex];
                keys.insert(keys.begin()+insertIndex, keyDown);
            }
            parent->keys.erase(parent->keys.begin()+keyDownIndex);

            // Handle min/max vectors
            parent->maxVect.erase(parent->maxVect.begin()+siblingIndex);
            parent->minVect.erase(parent->minVect.begin()+siblingIndex);
            if(!isMicroRoot() && !isMicroLeaf()) {
                maxVect.insert(maxVect.begin()+auxIndex, sibling->maxVect.begin(), sibling->maxVect.end());
                minVect.insert(minVect.begin()+auxIndex, sibling->minVect.begin(), sibling->minVect.end());
            }
            if(!isMicroLeaf()) updateParentAux();
        }
    };