#include "Node.cpp"

class BTree{

    public:
    unsigned B;
    Node* root;

    BTree(unsigned BIn){
        B = BIn;
        root = NULL;
    }

    void printTree(){
        if(root != NULL) root->printBT("", false);
    }

    Node* search(int k){
        if(root == NULL){
            std::cout << "The tree is empty" << std::endl;
            return NULL;
        }
        return root->search(k);
    }

    void insert(int key){
        // Empty tree
        if(root == NULL) {
            root = new Node(NULL, std::vector<Node*>{}, std::vector<int>{key});
            // DISK-WRITE
            return;
        } // Handle root overflow
        else if(root->keys.size() >= B) {
            root->split(0, B/2);
            root = root->parent;
        }
        // Root node is already created -> find appropriate leaf
        Node* curr = root;
        int index = curr->findChild(key);
        // Check for nodes to split
        while(!curr->isLeaf() && curr != NULL){
            Node* next = curr->children[index];
            
            // Check if split is needed + Update variables
            if(next->keys.size() >= B){
                next->split(index, B/2);
                curr = next->parent;
            } else curr = next;
            index = curr->findChild(key);
        }
        curr->insertKey(key, index); // Will have reached a leaf here
    }

    void deleteFromInternal(Node* curr, int index, int& key, Node* next){
        // Find child y/z that preceds/succeds k
        Node* predecessor = curr->getPredecessor(index);
        Node* successor = curr->getSuccessor(index);

        // Find predecessor key and remove it recursively
        if(predecessor != curr && predecessor->keys.size() >= floor((B+1)/2)){
            int predKey = predecessor->keys[predecessor->keys.size()-1];
            curr->keys[index] = predKey;
            key = predKey;
            next = predecessor;
        }
        // Find successor key and remove it recursively
        else if(successor != curr && successor->keys.size() >= floor((B+1)/2)){
            int succKey = successor->keys[0];
            //remove(succKey); //Don't remove since we use duplicates
            curr->keys[index] = succKey;
            key = succKey;
            next = successor;
        }
        // Merging predecessor with successor
        else{
            predecessor->insertKey(key, predecessor->keys.size());
            predecessor->merge(successor, false);
            next = predecessor; // Update variable
        }
    }


    void remove(int key){
        Node* curr = root;
        int index = curr->findChild(key);
        while(curr!=NULL && !curr->isLeaf()){
            Node* next = curr->children[index];
            // Check if next child has enough keys
            if(!next->keys.size() >= floor((B+1)/2)){
                Node* leftSibling = (index > 0) ? curr->children[index - 1] : NULL;
                Node* rightSibling = (index < curr->children.size()-1) ? curr->children[index + 1] : NULL;
                // Borrow from left sibling
                if(leftSibling != NULL && leftSibling->keys.size() >= floor((B+1)/2)) curr->borrow(leftSibling, next, index-1, leftSibling->keys.size()-1, true);
                // Borrow from right sibling
                else if(rightSibling != NULL && rightSibling->keys.size() >= floor((B+1)/2)) curr->borrow(rightSibling, next, index, 0, false);
                //Merging next child with one of his siblings
                else{
                    if(leftSibling != NULL) next->merge(leftSibling, true);
                    else next->merge(rightSibling, false);
                    // Tree shrinks
                    if(curr == root && curr->keys.empty()) {
                        curr->children[0]->setParent(NULL);
                        root = curr->children[0]; // new root
                        curr->children.erase(curr->children.begin());
                        for(int i=1; i<root->children.size(); i++) root->children[i]->setParent(root);
                    }
                }
            }
            //Delete from internal node
            if(curr->keys[index] == key) deleteFromInternal(curr, index, key, next);

            curr = next;
            index = curr->findChild(key);
        }
        // Will have reached a leaf here
        if(curr != NULL && curr->isLeaf() && curr->keys[index] == key){
            curr->keys.erase(curr->keys.begin()+index);
        } else{
            std::cout << "Key " << key << " not in tree!\n";
        }
    }

    int predecessor(int key){
        if(root != NULL){
            int index = root->findChild(key);
            Node* curr = root->children[index];
            // Find leaf
            while(!curr->isLeaf()){
                curr = curr->children[curr->findChild(key)];
                index = curr->findChild(key);
            }
            // Go up in tree if necessary
            if(curr->isLeaf() && index == 0){
                while(index == 0){
                    curr = curr->parent;
                    index = curr->findChild(key);
                }
            }
            return curr->keys[curr->findChild(key)-1];
        }

        return 0;
    }

    std::vector<int> range(int x, int y){
        std::vector<int> predecessors{};
        int pred = predecessor(y+1);
        while(pred >= x){
            predecessors.insert(predecessors.begin(), pred);
            pred = predecessor(pred);
        }

        return predecessors;
    }
};