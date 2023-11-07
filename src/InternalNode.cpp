#include "InternalNode.hpp"

//creates internal node pointed to by tree_ptr or creates a new one
InternalNode::InternalNode(const TreePtr &tree_ptr) : TreeNode(INTERNAL, tree_ptr) {
    this->keys.clear();
    this->tree_pointers.clear();
    if (!is_null(tree_ptr))
        this->load();
}

//max element from tree rooted at this node
Key InternalNode::max() {
    Key max_key = DELETE_MARKER;
    TreeNode* last_tree_node = TreeNode::tree_node_factory(this->tree_pointers[this->size - 1]);
    max_key = last_tree_node->max();
    delete last_tree_node;
    return max_key;
}

//if internal node contains a single child, it is returned
TreePtr InternalNode::single_child_ptr() {
    if (this->size == 1)
        return this->tree_pointers[0];
    return NULL_PTR;
}

//inserts <key, record_ptr> into subtree rooted at this node.
//returns pointer to split node if exists
TreePtr InternalNode::insert_key(const Key &key, const RecordPtr &record_ptr) {
    TreePtr new_tree_ptr = NULL_PTR;
    int idx = lower_bound(this->keys.begin(), this->keys.end(), key) - this->keys.begin();
    TreeNode* child_node = TreeNode::tree_node_factory(this->tree_pointers[idx]);
    // recursive call to insert key in child node
    TreePtr potential_split_node_ptr = child_node->insert_key(key, record_ptr);
    if(!is_null(potential_split_node_ptr)){
        TreeNode* left_node = TreeNode::tree_node_factory(this->tree_pointers[idx]); // child node which is split into 2 parts.
        int new_k = left_node->max();
        int idx = lower_bound(this->keys.begin(), this->keys.end(), new_k) - this->keys.begin();
        this->keys.insert(this->keys.begin() + idx, new_k);
        this->tree_pointers.insert(this->tree_pointers.begin() + idx + 1, potential_split_node_ptr);
        this->size += 1;
        delete left_node;
        if(this->overflows()){ // if the node overflows, split it into 2.
            InternalNode* new_node = new InternalNode(); 
            for(auto x=this->keys.begin()+ceil(this->keys.size()/2.0);x!=this->keys.end();x++){ 
                new_node->keys.push_back(*x);
            }
            for(auto x=this->tree_pointers.begin()+ceil(this->tree_pointers.size()/2.0);x!=this->tree_pointers.end();x++){
                new_node->tree_pointers.push_back(*x);
                new_node->size += 1;
            }
            new_node->dump();
            this->keys.erase(this->keys.begin()+ceil(this->keys.size()/2.0)-1,this->keys.end());
            this->tree_pointers.erase(this->tree_pointers.begin()+ceil(this->tree_pointers.size()/2.0)-1,this->tree_pointers.end());
            this->size = this->keys.size()+1;
            new_tree_ptr = new_node->tree_ptr;
        }
    }
    this->dump();
    return new_tree_ptr;
}

void InternalNode::left_redistribute(TreeNode *right_sibling_node) {
    InternalNode* right_node = new InternalNode(right_sibling_node->tree_ptr);
    right_node->tree_pointers.insert(right_node->tree_pointers.begin(), this->tree_pointers[this->size-1]);
    TreeNode* child_node = TreeNode::tree_node_factory(right_node->tree_pointers[0]);
    right_node->keys.insert(right_node->keys.begin(), child_node->max());
    delete child_node;
    right_node->size ++;
    this->tree_pointers.pop_back();
    this->keys.pop_back();
    this->size --;
    right_node->dump();
    this->dump();
    delete right_node;
}

void InternalNode::right_redistribute(TreeNode *left_sibling_node) {
    InternalNode* left_node = new InternalNode(left_sibling_node->tree_ptr);
    left_node->tree_pointers.push_back(this->tree_pointers[0]);
    TreeNode* child_node = TreeNode::tree_node_factory(left_node->tree_pointers[left_node->size-1]);
    left_node->keys.push_back(child_node->max());
    delete child_node;
    left_node->size ++;
    this->tree_pointers.erase(this->tree_pointers.begin());
    this->keys.erase(this->keys.begin());
    this->size --;
    left_node->dump();
    this->dump();
    delete left_node;
}

void InternalNode::merge_nodes(TreeNode *sibling_node) {
    InternalNode* sibling_internal_node = new InternalNode(sibling_node->tree_ptr);
    TreeNode* child_node = TreeNode::tree_node_factory(this->tree_pointers[this->size-1]);
    this->keys.push_back(child_node->max());
    delete child_node;
    this->keys.insert(this->keys.end(), sibling_internal_node->keys.begin(), sibling_internal_node->keys.end());
    this->tree_pointers.insert(this->tree_pointers.end(), sibling_internal_node->tree_pointers.begin(), sibling_internal_node->tree_pointers.end());
    this->size += sibling_internal_node->size;
    sibling_internal_node->delete_node();
    delete sibling_internal_node;
    this->dump();
}

//deletes key from subtree rooted at this if exists
void InternalNode::delete_key(const Key &key) {
    int idx = lower_bound(this->keys.begin(), this->keys.end(), key) - this->keys.begin();
    TreeNode* child_node = TreeNode::tree_node_factory(this->tree_pointers[idx]);
    child_node->delete_key(key);// recursive call to delete key in child node
    if(idx < (this->size-1)) this->keys[idx] = child_node->max();

    if(child_node->underflows()){ // if the node underflows, merge or redistribute.
        if(idx > 0){ // left sibling of child exists
            TreeNode* left_child = TreeNode::tree_node_factory(this->tree_pointers[idx-1]);
            if(left_child->size + child_node->size > FANOUT){
                left_child->left_redistribute(child_node);
                this->keys[idx-1] = left_child->max();
            }
            else{
                left_child->merge_nodes(child_node);
                this->keys.erase(this->keys.begin()+idx-1);
                this->tree_pointers.erase(this->tree_pointers.begin()+idx);
                this->size --;
            }
            delete left_child;
        }
        else if(idx < (this->size-1)){ // right sibling of child exists
            TreeNode* right_child = TreeNode::tree_node_factory(this->tree_pointers[idx+1]);
            if(right_child->size + child_node->size > FANOUT){
                right_child->right_redistribute(child_node);
                this->keys[idx] = child_node->max();
            }
            else{
                child_node->merge_nodes(right_child);
                this->keys.erase(this->keys.begin()+idx);
                this->tree_pointers.erase(this->tree_pointers.begin()+idx+1);
                this->size --;
            }
            delete right_child;
        }
    }
    delete child_node;
    this->dump();
}

//runs range query on subtree rooted at this node
void InternalNode::range(ostream &os, const Key &min_key, const Key &max_key) const {
    BLOCK_ACCESSES++;
    for (int i = 0; i < this->size - 1; i++) {
        if (min_key <= this->keys[i]) {
            auto* child_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
            child_node->range(os, min_key, max_key);
            delete child_node;
            return;
        }
    }
    auto* child_node = TreeNode::tree_node_factory(this->tree_pointers[this->size - 1]);
    child_node->range(os, min_key, max_key);
    delete child_node;
}

//exports node - used for grading
void InternalNode::export_node(ostream &os) {
    TreeNode::export_node(os);
    for (int i = 0; i < this->size - 1; i++)
        os << this->keys[i] << " ";
    os << endl;
    for (int i = 0; i < this->size; i++) {
        auto child_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
        child_node->export_node(os);
        delete child_node;
    }
}

//writes subtree rooted at this node as a mermaid chart
void InternalNode::chart(ostream &os) {
    string chart_node = this->tree_ptr + "[" + this->tree_ptr + BREAK;
    chart_node += "size: " + to_string(this->size) + BREAK;
    chart_node += "]";
    os << chart_node << endl;

    for (int i = 0; i < this->size; i++) {
        auto tree_node = TreeNode::tree_node_factory(this->tree_pointers[i]);
        tree_node->chart(os);
        delete tree_node;
        string link = this->tree_ptr + "-->|";

        if (i == 0)
            link += "x <= " + to_string(this->keys[i]);
        else if (i == this->size - 1) {
            link += to_string(this->keys[i - 1]) + " < x";
        } else {
            link += to_string(this->keys[i - 1]) + " < x <= " + to_string(this->keys[i]);
        }
        link += "|" + this->tree_pointers[i];
        os << link << endl;
    }
}

ostream& InternalNode::write(ostream &os) const {
    TreeNode::write(os);
    for (int i = 0; i < this->size - 1; i++) {
        if (&os == &cout)
            os << "\nP" << i + 1 << ": ";
        os << this->tree_pointers[i] << " ";
        if (&os == &cout)
            os << "\nK" << i + 1 << ": ";
        os << this->keys[i] << " ";
    }
    if (&os == &cout)
        os << "\nP" << this->size << ": ";
    os << this->tree_pointers[this->size - 1];
    return os;
}

istream& InternalNode::read(istream& is) {
    TreeNode::read(is);
    this->keys.assign(this->size - 1, DELETE_MARKER);
    this->tree_pointers.assign(this->size, NULL_PTR);
    for (int i = 0; i < this->size - 1; i++) {
        if (&is == &cin)
            cout << "P" << i + 1 << ": ";
        is >> this->tree_pointers[i];
        if (&is == &cin)
            cout << "K" << i + 1 << ": ";
        is >> this->keys[i];
    }
    if (&is == &cin)
        cout << "P" << this->size;
    is >> this->tree_pointers[this->size - 1];
    return is;
}
