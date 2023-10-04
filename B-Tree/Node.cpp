#include <iostream>
#include <vector>
#include <math.h>

class Node{
    public:
    Node* parent;
    std::vector<Node*> children;
    std::vector<int> keys;

    Node(Node* parentIn){
        parent = parentIn;
    }

    Node(Node* parentIn, std::vector<Node*> childrenIn, std::vector<int> keysIn){
        parent = parentIn;
        children = childrenIn;
        keys = keysIn;
    }

    void setParent(Node* parentIn){ parent = parentIn;}

    bool isLeaf(){return children.empty();}

    void insertKey(int key, int index) {keys.insert(keys.begin()+index, key);}

    void insertChild(Node* newChild, int index) {children.insert(children.begin()+index, newChild);}

    void printKeys(){
        std::cout << "Keys: ";
        for(int key:keys) std::cout << key << " ";
        std::cout << std::endl;
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

        std::cout << std::endl;

        // enter the next tree level
        if(!isLeaf()){
            children[0]->printBT(prefix + (isLeft ? "│   " : "    "), true);
            for(int i=1; i<children.size(); i++){
                children[i]->printBT(prefix + (isLeft ? "│   " : "    "), false);
            }                
        }
    }

    int findChild(int k){
        int n = keys.size();
        unsigned i=0; 
        while(i<n && keys[i] < k) ++i;
        return i;
    }

    Node* findLeaf(int key){
        int index = this->findChild(key);
        if(isLeaf()) return this;
        return children[index]->findLeaf(key);
    }
    
    Node* getPredecessor(int index){
        Node* curr = children[index];
        while(!curr->isLeaf()){
            curr = curr->children[curr->children.size()-1];
        }
        return curr;
    }
    Node* getSuccessor(int index){
        Node* curr = children[index+1];
        while(!curr->isLeaf()) curr = curr->children[0];
        return curr;
    }

    Node* getLeftSibling(){
        int index = parent->findChild(keys[0]);
        if(index == 0) return this;
        return parent->children[index];
    }

    Node* getRightSibling(){
        int index = parent->findChild(keys[0]);
        if(index == keys.size()-1) return this;
        return parent->children[index+1];
    }

    void moveKeyToChild(Node* child, int index, int insertIndex){
        child->keys.insert(child->keys.begin() + insertIndex, std::move(keys[index]));
        keys.erase(keys.begin() + index); 
    }

    void split(int index, int half){
        // Idea: keep "this" as new left node and only create new right node + delete children/keys from "this"
        int keyUp = keys[half];
        Node* newRight;
        std::vector<Node*> rightChildren = {};
        std::vector<int> rightKeys = {keys.begin()+half+1, keys.end()};
        // Push key to parent (or create new root)
        if(parent != NULL) parent->insertKey(keyUp, index);
        else parent = new Node(NULL, {this}, {keyUp}); 
        // Handle children
        if(!isLeaf()) {
            rightChildren = {children.begin()+half+1, children.end()};
            children.erase(children.begin()+half+1, children.end()); // Delete from "new left" node
        }
        keys.erase(keys.begin()+half, keys.end());
        newRight = new Node(parent, rightChildren, rightKeys);
        // Update children's parent
        for(Node* child : newRight->children){
            if(child != NULL) child->setParent(newRight);
        }

        int rightIndex = parent->findChild(newRight->keys[0]);
        parent->insertChild(newRight, rightIndex);
    }

    void merge(Node* other, bool left){
        int keyIndex = (left) ? 0 : keys.size();
        int childIndex = (left) ? 0 : children.size();
        // Merging keys and children into current node
        keys.insert(keys.begin()+keyIndex, other->keys.begin(), other->keys.end());
        for(Node* child : other->children){
            if(child != NULL) child->setParent(this); // Update parent of merged node
        }
        children.insert(children.begin()+childIndex, other->children.begin(), other->children.end());

        // Deleting merged node from it's parent's children
        int mergedIndex = parent->findChild(other->keys[0]);
        parent->children.erase(parent->children.begin()+mergedIndex);

        //Move median key in merged node
        int index = parent->findChild(keys[0]);
        if(!isLeaf()) {    
            int insertIndex = findChild(parent->keys[index]);
            parent->moveKeyToChild(this, index, insertIndex); 
        } else {
            parent->keys.erase(parent->keys.begin()+index);                    
        }
    }

    void borrow(Node* sibling, Node* next, int index, int n, bool left){
        int keyDown = keys[index];
        int newKeyIndex = (left) ? 0 : next->keys.size();
        int otherKey = (left) ? sibling->keys.size()-1-sibling->isLeaf() : 0;
        int eraseChildIndex = (left) ? sibling->children.size() : 0;
        int eraseKeyIndex = (left) ? sibling->keys.size()-1 : 0;

        keys[index] = sibling->keys[otherKey]; // move key from sibling up in parent
        sibling->keys.erase(sibling->keys.begin()+eraseKeyIndex);  

        if(!next->isLeaf()){
            next->insertChild(sibling->children[n], next->children.size());
            sibling->children[n]->setParent(next); 
            sibling->children.erase(sibling->children.begin()+eraseChildIndex);
            next->insertKey(keyDown, newKeyIndex); // Move key from parent/current to next
        } else{
            if(left) next->keys.insert(next->keys.begin() + newKeyIndex, keyDown); 
            else next->insertKey(keys[index], newKeyIndex);
        }
    }

    Node* search(int k){
        int i=0;
        int n=keys.size();
        while(i<n && keys[i]<k) ++i;
        // Is a leaf but doesn't contain desired key
        if(keys.size()==0 || children.size()==0) return NULL;
        // Key was found
        else if(keys[i] == k) return this;
        // Key not found but node is not a leaf -> recurse on appropriate child 
        // Needs a DISK-READ
        else if(children[i] != NULL) return children[i]->search(k);
        else return NULL;
    }
};