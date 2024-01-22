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
        return parent->children[index-1];
    }

    Node* getRightSibling(){
        int index = parent->findChild(keys[0]);
        if(index == keys.size()-1) return this;
        return parent->children[index+1];
    }

    int myIndex(){
        for(int i=0; i<parent->children.size(); ++i){
            if(parent->children[i] == this) return i;
        }
        return -1;
    }

    void moveKeyToChild(Node* child, int index, int insertIndex){
        printf("Moving key %i from parent to child\n", keys[index]);
        child->keys.insert(child->keys.begin() + insertIndex, keys[index]);
        keys.erase(keys.begin() + index); 
    }

    void moveChild(Node* sibling, int eraseChildIndex, int insertIndex){
        children.insert(children.begin()+insertIndex, sibling->children[eraseChildIndex]);
        sibling->children[eraseChildIndex]->setParent(this);
        sibling->children.erase(sibling->children.begin()+eraseChildIndex);
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

    // void merge(Node* other, bool left){
    //     int keyIndex = (left) ? 0 : keys.size();
    //     int childIndex = (left) ? 0 : children.size();
    //     // Merging keys and children into current node
    //     keys.insert(keys.begin()+keyIndex, other->keys.begin(), other->keys.end());
    //     for(Node* child : other->children){
    //         if(child != NULL) child->setParent(this); // Update parent of merged node
    //     }
    //     children.insert(children.begin()+childIndex, other->children.begin(), other->children.end());

    //     // Deleting merged node from it's parent's children
    //     int mergedIndex = other->parent->findChild(other->keys[0]);
    //     other->parent->children.erase(other->parent->children.begin()+mergedIndex);

    //     //Move median key in merged node
    //     int index = parent->findChild(keys[0]);
    //     int insertIndex = findChild(parent->keys[index]);
    //     parent->moveKeyToChild(this, index, insertIndex); 
    // }

    void merge(){
        //printKeys();
        Node* sibling = getLeftSibling();
        int keyIndex = 0, childIndex = 0;
        if(sibling == this){
            sibling = getRightSibling();
            keyIndex = keys.size();
            childIndex = children.size();
        }
        int keyDownIndex = std::max(myIndex()-1, 0);
        if(keyDownIndex == parent->keys.size()) --keyDownIndex;
        //sibling->printKeys();
        // Merging keys and children into current node
        keys.insert(keys.begin()+keyIndex, sibling->keys.begin(), sibling->keys.end());
        for(Node* child : sibling->children){
            if(child != NULL) child->setParent(this); // Update parent of merged node
        }
        children.insert(children.begin()+childIndex, sibling->children.begin(), sibling->children.end());

        // Deleting merged node from it's parent's children
        int mergedIndex = parent->findChild(sibling->keys[0]);
        parent->children.erase(parent->children.begin()+mergedIndex);

        //Move median key in merged node
        int insertIndex = findChild(parent->keys[keyDownIndex]);
        int keyDown = parent->keys[keyDownIndex];
        keys.insert(keys.begin()+insertIndex, keyDown);
        parent->keys.erase(parent->keys.begin()+keyDownIndex);
    }

    void mergePredSucc(Node* successor){
        // Merging keys into predecessor node
        keys.insert(keys.end(), successor->keys.begin(), successor->keys.end());
        // Deleting merged node from it's parent's children
        int mergedIndex = successor->parent->findChild(successor->keys[0]);
        successor->parent->children.erase(successor->parent->children.begin()+mergedIndex);
    }

    void borrowLeft(Node* sibling){
        // Move key from parent down in node
        int keyDownIndex = parent->findChild(sibling->keys[0]);
        int keyDown = parent->keys[keyDownIndex];
        keys.insert(keys.begin(), keyDown);
        parent->keys.erase(parent->keys.begin()+keyDownIndex);

        // Move key from sibling up in parent
        int keyUp = sibling->keys.back();
        int insertIndex = parent->findChild(keyUp);
        parent->keys.insert(parent->keys.begin()+insertIndex, keyUp);
        sibling->keys.pop_back();

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
        int keyDownIndex = parent->findChild(keys[0]);
        int keyDown = parent->keys[keyDownIndex];
        parent->keys.erase(parent->keys.begin()+keyDownIndex);
        insertIndex = findChild(keyDown);
        keys.insert(keys.begin()+insertIndex, keyDown);

        // Handle children
        if(!isLeaf()) moveChild(sibling, 0, children.size());
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