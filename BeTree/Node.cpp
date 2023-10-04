#include <iostream>
#include <vector>
#include <cmath>

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


        Node(Node* parentIn, std::vector<Node*> childrenIn, std::vector<int> keysIn): parent(parentIn), children(childrenIn), keys(keysIn){
            //DISK-WRITE
            buffer = std::vector<Message>{};
        }
        
        Node(Node* parentIn, std::vector<Node*> childrenIn, std::vector<int> keysIn, std::vector<Message> buffIn): parent(parentIn), 
            children(childrenIn), keys(keysIn), buffer(buffIn){
            //DISK-WRITE
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
            if(!isLeaf()){
                children[0]->printBT(prefix + (isLeft ? "│   " : "    "), true);
                for(int i=1; i<children.size(); i++){
                    children[i]->printBT(prefix + (isLeft ? "│   " : "    "), false);
                }                
            }

        }

        void printKeys(){ for(int key: keys) std::cout << key << " ";
                            std::cout << std::endl;}
        
        void setParent(Node* parentIn){ parent = parentIn;}

        bool isLeaf(){ return children.empty();}

        void insertChild(Node* child, int index) {children.insert(children.begin()+index, child);}

        void insertKey(int key, int index) {keys.insert(keys.begin()+index, key);}

        int findChild(int k){
            int n = keys.size();
            int i=0; 
            while(i<n && keys[i] < k) ++i;
            return i;
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

        bool bigEnough(int B, int Beps) {
            return (isLeaf() && keys.size() >= ceil((double) B/2)) || (!isLeaf() && keys.size() >= ceil((double) Beps/2));
        }
        bool tooSmall(int B, int Beps){
            return (isLeaf() & keys.size() <  ceil((double) B/2)-1) || (!isLeaf() && keys.size() <  ceil((double) Beps/2)-1);
        }
        
        bool tooBig(int B, int Beps){
            return (isLeaf() && keys.size() > B) || (!isLeaf() && keys.size() > Beps);
        }

        void annihilateInsDel(){
            for(auto it=buffer.begin(); it!=buffer.end();){
                bool deleted = false;
                for(auto it2=buffer.begin(); it2!=buffer.end();){
                    if(it2->key == it->key && it2->op != it->op){
                        it2 = buffer.erase(it2);
                        deleted = true;
                    } else it2++;
                }
                it = (deleted) ? buffer.erase(it) : it++;
            }
        }

    
        /*
         *------------------------------------------------------------------------
                                    QUERIES
         *------------------------------------------------------------------------
        */

        Node* search(int k){
            int i=0;
            int n=keys.size();
            while(i<n && keys[i]<k) ++i;
            // Is a leaf but doesn't contain desired key
            if(isLeaf()) return NULL;
            // Key was found
            else if(keys[i] == k) return this;
            // Key not found but node is not a leaf -> recurse on appropriate child 
            // DISK-READ
            else if(children[i] != NULL) return children[i]->search(k);
            else return NULL;
        }


        /*
         *------------------------------------------------------------------------
                                    INSERTION
         *------------------------------------------------------------------------
        */

        void split(int half){
            // Idea: keep "this" as new left node and only create new right node + delete appropriate children/keys from "this"
            int keyUp = keys[half];
            std::vector<Node*> rightChildren = {};
            std::vector<int> rightKeys = {keys.begin()+half+1, keys.end()};
            // Push key to parent (or create new root)
            if(parent != NULL){
                int parentIndex = parent->findChild(keyUp);
                parent->insertKey(keyUp, parentIndex);
            } 
            else parent = new Node(NULL, {this}, {keyUp}); 
            // Handle children
            if(!isLeaf()) {
                rightChildren = {children.begin()+half+1, children.end()};
                children.erase(children.begin()+half+1, children.end()); // Delete from "new left" node
            }
            keys.erase(keys.begin()+half+isLeaf(), keys.end()); // Keep key if node is leaf (since we use duplicates)
            Node* newRight = new Node(parent, rightChildren, rightKeys);
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

        void borrowLeft(Node* sibling){
            // Move key from parent down in node
            int keyDownIndex = parent->findChild(sibling->keys[0]);
            int keyDown = parent->keys[keyDownIndex];
            keys.insert(keys.begin(), keyDown);
            parent->keys.erase(parent->keys.begin()+keyDownIndex);

            // Move key from sibling up in parent
            if(isLeaf()) sibling->keys.pop_back();
            int keyUp = sibling->keys.back();
            int insertIndex = parent->findChild(keyUp);
            parent->keys.insert(parent->keys.begin()+insertIndex, keyUp);
            if(!isLeaf()) sibling->keys.pop_back();

            // Handle children
            if(!isLeaf()) moveChild(sibling, sibling->children.size()-1, 0);
        }

        void borrowRight(Node* sibling){
            // Move key from sibling to parent
            int keyUp = sibling->keys[0];
            sibling->keys.erase(sibling->keys.begin());
            int insertIndex = parent->findChild(keys[0])+1;
            parent->keys.insert(parent->keys.begin()+insertIndex, keyUp);

            // Move keyUp down in node
            int keyDownIndex = (isLeaf()) ? insertIndex : parent->findChild(keys[0]);
            int keyDown = parent->keys[keyDownIndex];
            parent->keys.erase(parent->keys.begin()+keyDownIndex-isLeaf());
            insertIndex = findChild(keyDown);
            keys.insert(keys.begin()+insertIndex, keyDown);

            // Handle children
            if(!isLeaf()) moveChild(sibling, 0, children.size());
        }

        void moveChild(Node* sibling, int eraseChildIndex, int insertIndex){
            Node* child = sibling->children[eraseChildIndex];
            int firstKey = child->keys[0];
            int lastKey = child->keys[child->keys.size()-1];
            // Move updates concerning the moved child
            for(auto it=sibling->buffer.begin(); it!=sibling->buffer.end();){
                Message msg = *it;
                if(msg.key >= firstKey && msg.key <= lastKey){
                    buffer.push_back(msg);
                    it = sibling->buffer.erase(it);
                } else it++;
            }
            children.insert(children.begin()+insertIndex, sibling->children[eraseChildIndex]);
            sibling->children[eraseChildIndex]->setParent(this);
            sibling->children.erase(sibling->children.begin()+eraseChildIndex);
        }

        void merge(Node* sibling, int keyIndex, int childIndex, int buffIndex){
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
            int mergedIndex = parent->findChild(sibling->keys[0]);
            parent->children.erase(parent->children.begin()+mergedIndex);

            //Move median key in merged node
            int keyDownIndex = parent->findChild(keys[0]);
            if(!isLeaf()) {    
                if(keyDownIndex > parent->keys.size()-1) keyDownIndex = 0;
                int insertIndex = findChild(parent->keys[keyDownIndex]);
                int keyDown = parent->keys[keyDownIndex];
                parent->keys.erase(parent->keys.begin()+keyDownIndex);
                keys.insert(keys.begin()+insertIndex, keyDown);
            } else parent->keys.erase(parent->keys.begin()+keyDownIndex);                    
        }

        void updateParent(int deletedKey){
            int lastKey = keys.back();
            int parentIndex = parent->findChild(deletedKey);
            if(parent->keys[parentIndex] == deletedKey) parent->keys[parentIndex] = lastKey;
        }
    };