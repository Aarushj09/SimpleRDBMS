#include "RecordPtr.hpp"
#include "LeafNode.hpp"

LeafNode::LeafNode(const TreePtr &tree_ptr) : TreeNode(LEAF, tree_ptr) {
    this->data_pointers.clear();
    this->next_leaf_ptr = NULL_PTR;
    if(!is_null(tree_ptr))
        this->load();
}

//returns max key within this leaf
Key LeafNode::max() {
    auto it = this->data_pointers.rbegin();
    return it->first;
}

//inserts <key, record_ptr> to leaf. If overflow occurs, leaf is split
//split node is returned
TreePtr LeafNode::insert_key(const Key &key, const RecordPtr &record_ptr) {
    TreePtr new_leaf = NULL_PTR; //if leaf is split, new_leaf = ptr to new split node ptr
    this->data_pointers[key] = record_ptr;
    this->size++;
    if(this->overflows()){
        LeafNode* new_node = new LeafNode();
        auto data_it = this->data_pointers.begin();
        for(int x=0;x<ceil(this->data_pointers.size()/2.0);x++) data_it++;
        for(auto x=data_it;x!=this->data_pointers.end();x++){
            new_node->data_pointers.insert(*x);
            new_node->size++;
            this->size--;
        }
        this->data_pointers.erase(data_it,this->data_pointers.end());
        new_node->next_leaf_ptr = this->next_leaf_ptr;
        this->next_leaf_ptr = new_node->tree_ptr;
        new_node->dump();
        new_leaf = new_node->tree_ptr;
    }
    this->dump();
    return new_leaf;
}

void LeafNode::left_redistribute(TreeNode *right_sibling_node) {
    right_sibling_node->insert_key(this->data_pointers.rbegin()->first, this->data_pointers.rbegin()->second);
    this->data_pointers.erase(this->data_pointers.rbegin()->first);
    this->size--;
    this->dump();
    right_sibling_node->dump();
}

void LeafNode::right_redistribute(TreeNode *left_sibling_node) {
    left_sibling_node->insert_key(this->data_pointers.begin()->first, this->data_pointers.begin()->second);
    this->data_pointers.erase(this->data_pointers.begin()->first);
    this->size--;
    this->dump();
    left_sibling_node->dump();
}

void LeafNode::merge_nodes(TreeNode *sibling_node) {
    LeafNode* sibling_leaf_node = new LeafNode(sibling_node->tree_ptr);
    for(auto x=sibling_leaf_node->data_pointers.begin();x!=sibling_leaf_node->data_pointers.end();x++){
        this->data_pointers.insert(*x);
        this->size++;
    }
    this->next_leaf_ptr = sibling_leaf_node->next_leaf_ptr;
    sibling_leaf_node->delete_node();
    delete sibling_leaf_node;
    this->dump();
}


//key is deleted from leaf if exists
void LeafNode::delete_key(const Key &key) {
    if(this->data_pointers.find(key) == this->data_pointers.end()) return;
    this->data_pointers.erase(key);
    this->size--;
    this->dump();
}

//runs range query on leaf
void LeafNode::range(ostream &os, const Key &min_key, const Key &max_key) const {
    BLOCK_ACCESSES++;
    for(const auto& data_pointer : this->data_pointers){
        if(data_pointer.first >= min_key && data_pointer.first <= max_key)
            data_pointer.second.write_data(os);
        if(data_pointer.first > max_key)
            return;
    }
    if(!is_null(this->next_leaf_ptr)){
        auto next_leaf_node = new LeafNode(this->next_leaf_ptr);
        next_leaf_node->range(os, min_key, max_key);
        delete next_leaf_node;
    }
}

//exports node - used for grading
void LeafNode::export_node(ostream &os) {
    TreeNode::export_node(os);
    for(const auto& data_pointer : this->data_pointers){
        os << data_pointer.first << " ";
    }
    os << endl;
}

//writes leaf as a mermaid chart
void LeafNode::chart(ostream &os) {
    string chart_node = this->tree_ptr + "[" + this->tree_ptr + BREAK;
    chart_node += "size: " + to_string(this->size) + BREAK;
    for(const auto& data_pointer: this->data_pointers) {
        chart_node += to_string(data_pointer.first) + " ";
    }
    chart_node += "]";
    os << chart_node << endl;
}

ostream& LeafNode::write(ostream &os) const {
    TreeNode::write(os);
    for(const auto & data_pointer : this->data_pointers){
        if(&os == &cout)
            os << "\n" << data_pointer.first << ": ";
        else
            os << "\n" << data_pointer.first << " ";
        os << data_pointer.second;
    }
    os << endl;
    os << this->next_leaf_ptr << endl;
    return os;
}

istream& LeafNode::read(istream& is){
    TreeNode::read(is);
    this->data_pointers.clear();
    for(int i = 0; i < this->size; i++){
        Key key = DELETE_MARKER;
        RecordPtr record_ptr;
        if(&is == &cin)
            cout << "K: ";
        is >> key;
        if(&is == &cin)
            cout << "P: ";
        is >> record_ptr;
        this->data_pointers.insert(pair<Key,RecordPtr>(key, record_ptr));
    }
    is >> this->next_leaf_ptr;
    return is;
}