// dlx.cpp

#include <iostream>
#include <vector>
#include <stack>
#include <cmath>
#include <cassert>
#include <climits>
#include <fstream>
#include <ostream>
#include <sstream>
#include "dlx.h"

using namespace std;

/**
 * Map rows (possibilties) to their columns (constraints). 
 * Each 'index' represents a row in the exact cover matrix.
 * The following 4 functions tell you which constraints the rows
 * satisfy.
 *
 * @param   index   The index of the row
 * @param   n       Sidelength of a single box of the sudoku
*
 * @return  
 */
int indexToCellConstraint(int index, int n){
    return floor(index / (n*n));
}
int indexToRowConstraint(int index, int n){
    int value = index % (n*n);
    int counter = floor(index / (n*n*n*n)); // 0 for 0-80, 1 for 81-161.. 
    return counter * (n*n) + value + (n*n*n*n);
}
int indexToColumnConstraint(int index, int n){
    return index % (n*n*n*n) + (n*n*n*n) * 2;
}
int indexToBoxConstraint(int index, int n){
    int vd1 = floor(index / (n*n*n*n*n)); // 0-2
    int vd3 = floor((index % (n*n*n*n)) / (n*n*n)); // 0-2
    int vd5 = index % (n*n); // 0-9

    int vd1_increment = vd1 * (n*n*n); // max: 54
    int vd3_increment = vd3 * (n*n); // max: 18

    return vd1_increment + vd3_increment + vd5 + (n*n*n*n) * 3; // max: 81
}

/**
 * Construct an exact cover matrix representing an EMPTY sudoku
 * of size n * n.
 *
 * @param   n           Sidelength of a single box of the sudoku
 * @param   allNodes    Vector to store references to all nodes created
 * @param   heads       Vector to store references to column header nodes
 * @param   rowNodes    Vector to store references to row nodes
*
 * @return  pointer to the root of the matrix (top left corner, just before first header)
 */
Node* constructMatrix(
    int n, 
    vector<Node*>& allNodes,
    vector<Node*>& heads,
    vector<Node*>& rowNodes
){
    Node* matrix = new Node();
    allNodes.push_back(matrix);

    // 1. Create the header

    // n*n boxes, 4 types of constraints
    int numColumns = n * n * n * n * 4; 
    for (int i = 0; i < numColumns; i++){
        Node* head = new Node();

        allNodes.push_back(head);
        heads.push_back(head);

        head->index = i;
        head->numNodesInColumn = 0;

        head->right = matrix;
        head->left = matrix->left;
        matrix->left = head;
        head->left->right = head;
        head->head = head; // bruh (is this even needed tho?)
        head->index = i;
    } 

    matrix->right = heads[0];

    // 2. Create each row, and define its constraints.
    //    This requires mapping the index of the row to 
    //    Each respective constraint: cell, row, column, box

    int numRowsInMatrix = n * n * n * n * n * n;

    rowNodes = vector<Node*>(numRowsInMatrix, nullptr);
    for (int i = 0; i < numRowsInMatrix; i++){
        Node* rowNode = new Node();
        rowNodes[i] = rowNode;
        allNodes.push_back(rowNode);

        // Assign the rowNode its values; this is needed in order to interpret the solution
        rowNode->value = i % (n * n);
        rowNode->column_id = floor(i % (n*n*n*n) / (n*n));
        rowNode->row_id = floor(i / (n*n*n*n));
        rowNode->row_index = i;

        // Add each constraint to this row
        int constraints[4] = {
            indexToCellConstraint(i, n),
            indexToRowConstraint(i, n),
            indexToColumnConstraint(i, n),
            indexToBoxConstraint(i, n)
        };

        for (int c = 0; c < 4; c++){
            // Insert a node
            Node* node = new Node();
            allNodes.push_back(node);

            node->row = rowNode;

            // Insert into row
            node->right = rowNode;
            node->left = rowNode->left;
            rowNode->left = node;
            node->left->right = node;

            // Insert into column
            node->up = heads[constraints[c]]->up;
            heads[constraints[c]]->up = node;
            node->down = heads[constraints[c]];
            node->up->down = node;

            node->head = heads[constraints[c]];
            node->head->numNodesInColumn++;
        }
    }

    return matrix;
};

/**
 * Get the header of the column with the least number of values. 
 * Ties go to the first column of the lowest value.
 *
 * @param   matrix      pointer to the root of the matrix
 *
 * @return  the header of the smallest column
 */
Node* getSmallestColumn(Node* matrix){
    assert(matrix != matrix->right);
    int smallest = INT_MAX;
    Node* smallestColumn = nullptr;

    Node* tmp = matrix->right;
    while(tmp != matrix){
        if (tmp->numNodesInColumn < smallest){
            smallest = tmp->numNodesInColumn;
            smallestColumn = tmp;
        }
        tmp = tmp->right;
    }

    return smallestColumn;
}

/**
 * 'Remove' a column from the matrix by 'removing' all of its rows.
 * This does so by making its neighbors point to each other instead.
 *
 * @param   columnHead      pointer to the head of the column to be removed
 */
void removeColumn(Node* columnHead){
    // We must remove the column header, to tell the matrixwhen it's solved.
    columnHead->right->left = columnHead->left;
    columnHead->left->right = columnHead->right;

    for (Node* tmp = columnHead->down; tmp != columnHead; tmp = tmp->down){
        for (Node* row_tmp = tmp->right; row_tmp != tmp; row_tmp = row_tmp->right){
            if (row_tmp->head == nullptr) continue;
            row_tmp->up->down = row_tmp->down;
            row_tmp->down->up = row_tmp->up;
            row_tmp->head->numNodesInColumn-= 1;
        }
    }
}

/**
 * 'Replace' a column from the matrix by 'replacing' all of its rows.
 * This does so by making its neighbors point back to itself.
 *
 * @param   columnHead      pointer to the head of the column to be restored
 */
void restoreColumn(Node* columnHead){
    for (Node* tmp = columnHead->down; tmp != columnHead; tmp = tmp->down){
        for (Node* row_tmp = tmp->right; row_tmp != tmp; row_tmp = row_tmp->right){
            if (row_tmp->head == nullptr) continue;
            row_tmp->up->down = row_tmp;
            row_tmp->down->up = row_tmp;
            row_tmp->head->numNodesInColumn += 1;
        }
    }

    // why do these last?
    columnHead->right->left = columnHead;
    columnHead->left->right = columnHead;
}



/**
 * 
 * Preform DLX on an exact cover matrix.
 *
 * @param   matrix      Pointer to the root of the matrix
 * @param   heads       Vector to store references to column header nodes
 * @param   rowNodes    Vector to store references to row nodes
 * @param   solution    Stack of row markers (nodes with extra properties) passed by reference. These will contain the solution. 
 *
 * @return  True if a solution is found, otherwise false
 */
bool dlx(
    Node* matrix,
    vector<Node*>& heads,
    vector<Node*>& rowNodes,
    stack<Node*>& solution
){
    if (matrix->right == matrix){
        return true;
    }

    // Choose a column deterministically
    Node* head = getSmallestColumn(matrix);
    // Before searching for rows, we must first take this column out 
    // of the matrix so the algorithm doesn't loop back around to it.
    removeColumn(head);

    for (Node* tmp = head->down; tmp != head; tmp = tmp->down){
        solution.push(tmp->row);

        for (Node* row_element = tmp->right; row_element != tmp; row_element = row_element->right){
            if (row_element->head == nullptr) continue;
            removeColumn(row_element->head);
        }

        if (dlx(matrix, heads,rowNodes, solution)) 
            return true;

        solution.pop();

        for (Node* row_element = tmp->left; row_element != tmp; row_element = row_element->left){
            if (row_element->head == nullptr) continue;
            restoreColumn(row_element->head);
        }
    }

    restoreColumn(head);
    return false;
}

/**
 * Find out if a number if a perfect square
 *
 * @param   x      The number to check
 *
 * @return  True if x is a perfect square, otherwise false
 */
bool isPerfectSquare(long long x) {
    if (x >= 0) {
        long long sr = sqrt(x);
        return (sr * sr == x);
    }
    return false;
}

int main(int argc, char* argv[]){
    if (argc == 1){
        cout << "No filename found." << endl;
        cout << "Usage: ./solve [FILENAME]" << endl;
        return 0;
    }
    if (argc > 2){
        cout << "Too many arguments specified." << endl;
        cout << "Usage: ./solve [FILENAME]" << endl;
        return 1;
    }

    // Read the file
    ifstream inputFile;
    inputFile.open(argv[1]);
    if (!inputFile.good()){
        cout << "Error: could not read file." << endl;
        return 1;
    }
    

    string line;
    getline(inputFile, line);
    istringstream os(line);
    
    int i;
    vector<int> topRow;
    while(os >> i)
        topRow.push_back(i);

    if (!isPerfectSquare(topRow.size())){
        cout << "Error: Invalid input found on first line." << endl;
        cout << "The sudoku size must be N*N, where N is a perfect square." << endl;
        return 1;
    }

    // The sidelength of a box of the sudoku
    int n = int(sqrt(topRow.size()));

    // Now that we've determined the dimensions of the input sudoku,
    // we can define it
    vector<vector<int>> sudoku(n*n, vector<int>(n*n, 0));
    sudoku[0] = topRow;

    // Read the rest of the sudoku
    for (int row = 1; row < n * n; row++) {
        if (!getline(inputFile, line)) {
            cout << "Error: Not enough rows in the input file." << endl;
            return 1;
        }
        istringstream rowStream(line);
        // Ignore lines that are just '\n'. This allows the input
        //  to be formatted and more clearly interpretable.
        if (rowStream.peek() == -1){
            row--;
            continue;
        }
        for (int col = 0; col < n * n; col++) {
            if (!(rowStream >> sudoku[row][col])) {
                cout << "Error: Invalid input in row " << row + 1 << "." << endl;
                return 1;
            }
        }
        int dummy;
        if (rowStream >> dummy) {
            cout << "Error: Too many inputs in row " << row + 1 << "." << endl;
            return 1;
        }
    }

    vector<Node*> allNodes;
    vector<Node*> heads;
    vector<Node*> rowNodes;
    stack<Node*> solution;

    // Build the exact cover matrix
    Node* matrix = constructMatrix(n, allNodes, heads, rowNodes);

    // Fill in the matrix with known values.
    for (int i = 0; i < sudoku.size(); i++){
        for (int j = 0; j < sudoku[0].size(); j++){
            if (sudoku[i][j] > 0){
                int val = sudoku[i][j] - 1; // the numerical value stored in the cell
                int colOffset = j * (n*n); // the column 
                int rowOffset = i * (n*n*n*n);

                // find the row representing this possibility
                int choiceIndex = rowOffset + colOffset + val;
                Node* known = rowNodes[choiceIndex];
                solution.push(known);

                // Remove all columns associated with this row
                for (Node* tmp = known->right; tmp != known; tmp = tmp->right)
                    removeColumn(tmp->head);
            }
        }
    }

    // Solve the sudoku
    if (!dlx(matrix, heads, rowNodes, solution)){
        cout << "The input sudoku has no valid solution." << endl;
        return 0;
    };
    
    // Interpret the solution results and fill the sudoku matrix
    while (!solution.empty()){
        Node* r = solution.top(); 
        solution.pop();
        sudoku[r->row_id][r->column_id] = r->value + 1;
    }

    // Print the sudoku matrix
    for (int i = 0; i < sudoku.size(); i++){
        if (i % n == 0 && i > 0) cout << endl;
        for (int j = 0; j < sudoku[0].size(); j++){
            if (j % n == 0 && j > 0) cout << ' ';
            cout << sudoku[i][j] << ' ';
            if (sudoku[i][j] < 10) cout << ' ';
        }   
        cout << endl;
    }

    // Clean up
    for (int i = 0; i < allNodes.size(); i++){
        delete allNodes[i];
    }
}