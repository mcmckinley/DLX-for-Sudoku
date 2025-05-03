class Node {
    public:
        Node* up;
        Node* down;
        Node* right;
        Node* left;
        Node* head;
        Node* row;

        // Only used by headers
        int index;
        int numNodesInColumn;

        // Only used by row markers
        int row_id;
        int column_id;
        int value = -1;
        int row_index = -1;
        
        Node(){
            this->up = this;
            this->down = this;
            this->left = this;
            this->right = this;
            this->head = nullptr;
        }
};