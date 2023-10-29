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
        int maxSize, minSize; // Stores the maximum/minimum size of a leaf
        long int maxSizeIndex, minSizeIndex; // Indicates which child contains the subtree with max/min leaf
        std::vector<int> maxVect, minVect;
        std::vector<int> maxIndex, minIndex;


        Node(Node* parentIn, std::vector<Node*> childrenIn, std::vector<int> keysIn): parent(parentIn), children(childrenIn), keys(keysIn){
            //DISK-WRITE
            buffer = std::vector<Message>{};
            maxSize = 0, maxSizeIndex = 0, minSizeIndex = 0;
            minSize = INT16_MAX;
        }
        
        Node(Node* parentIn, std::vector<Node*> childrenIn, std::vector<int> keysIn, std::vector<Message> buffIn): parent(parentIn), 
            children(childrenIn), keys(keysIn), buffer(buffIn){
            //DISK-WRITE
            maxSize = 0, maxSizeIndex = 0, minSizeIndex = 0;
            minSize = INT16_MAX;
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
            //TODO: Delete matching INS/DEL updates
            int res = buffer.size();
            for(Node* child : children) res += child->keys.size();
            return res;
        }

        int getSize() {return keys.size();}

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
            for(int index = 0; index<children.size(); ++index){
                Node* child = children[index];
                int big = (child->isMicroRoot()) ? child->getLeafSize() : child->maxSize;
                int small = (child->isMicroRoot()) ? child->getLeafSize() : child->minSize;
                if(big > maxSize){
                    maxSize = big;
                    maxSizeIndex = index;
                }
                if(small < minSize){
                    minSize = small;
                    minSizeIndex = index;
                }
            }
        }

        void updateParentAux(){
            if(isMicroRoot()){
                int size = getLeafSize();
                if(size > parent->maxSize){
                    parent->maxSize = size;
                    parent->maxSizeIndex = myIndex();
                }
                if(size < minSize){
                    parent->minSize = size;
                    parent->minSizeIndex = myIndex();
                }
            } else {
                for(int c = 0; c<children.size(); ++c){
                    if(children[c]->maxSize > parent->maxSize){
                        this->maxSize = children[c]->maxSize;
                        parent->maxSize = children[c]->maxSize;
                        parent->maxSizeIndex = c;
                    }
                    if(children[c]->minSize < parent->minSize){
                        this->minSize = children[c]->minSize;
                        parent->minSize = children[c]->minSize;
                        parent->minSizeIndex = c;
                    }
                }
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
            for(std::vector<Message>::iterator iter = buffer.begin(); iter != buffer.end();){
                if(iter->key > keyUp){
                    newRight->buffer.push_back(*iter);
                    iter = buffer.erase(iter);
                } else ++iter;
            }

            int rightIndex = parent->findChild(newRight->keys[0]);
            parent->insertChild(newRight, rightIndex);
        }

        /*
         *------------------------------------------------------------------------
                                    DELETION
         *------------------------------------------------------------------------
        */

        void merge(int threshold){
            Node* sibling = getLeftSibling();
            int keyIndex = 0, childIndex = 0, buffIndex = 0;
            if(sibling == this){
                sibling = getRightSibling();
                keyIndex = keys.size();
                childIndex = children.size();
                buffIndex = buffer.size();
            }
            if(sibling->keys.size() > threshold) return; // Don't merge if sibling has enough keys
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
        }
    };