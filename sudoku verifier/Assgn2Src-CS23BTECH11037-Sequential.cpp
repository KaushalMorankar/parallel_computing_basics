#include <bits/stdc++.h>
using namespace std;

ofstream output("output.txt"); // Log file
vector<string> outputs; // Vector to store log messages

bool isvalid(const vector<vector<int>>& board, int n) {
    int sub=sqrt(n);
    
    // Check each row.
    for (int i=0; i<n; ++i) {
        vector<bool> seen(n+1, false); // numbers 1 through n
        for (int j=0; j<n; ++j) {
            int val=board[i][j];
            if (val != 0) {
                if (val<1 || val>n || seen[val]) {
                    outputs.push_back("Row "+to_string(i)+" is invalid.");
                    return false;  // Early exit on row violation
                }
                seen[val]=true;
            }
        }
        outputs.push_back("Row "+to_string(i)+" is valid.");
    }
    
    // Check each column.
    for (int j=0; j<n; ++j) {
        vector<bool> seen(n+1, false);
        for (int i=0; i<n; ++i) {
            int val=board[i][j];
            if (val != 0) {
                if (val<1 || val>n || seen[val]) {
                    outputs.push_back("Column "+to_string(j)+" is invalid.");
                    return false;  // Early exit on column violation
                }
                seen[val]=true;
            }
        }
        outputs.push_back("Column "+to_string(j)+" is valid.");
    }
    
    // Check each subgrid.
    // There are n subgrids of size sub x sub.
    for (int box=0; box<n; ++box) {
        vector<bool> seen(n+1, false);
        int sr=(box/sub) * sub;
        int sc=(box%sub) * sub;
        for (int i=0; i<sub; ++i) {
            for (int j=0; j<sub; ++j) {
                int row=sr+i;
                int col=sc+j;
                int val=board[row][col];
                if (val != 0) {
                    if (val<1 || val>n || seen[val]) {
                        outputs.push_back("Grid "+to_string(box)+" is invalid.");
                        return false;  // Early exit on subgrid violation
                    }
                    seen[val]=true;
                }
            }
        }
        outputs.push_back("Grid "+to_string(box)+" is valid.");
    }
    
    return true; // Sudoku is valid if no violations found.
}

int main() {
    int n=0, k1=0, taskInc=0;

    ifstream file("./input.txt");
    if (!file) {
        cerr<<"Error opening input file!"<<endl;
        return 1;
    }
    
    file>>k1>>n>>taskInc;  // First line: number of threads (k1) and sudoku size (n)
    
    vector<vector<int>> sudoku(n, vector<int>(n));
    for (int i=0; i<n; i++) {
        for (int j=0; j<n; j++) {
            file>>sudoku[i][j];
        }
    }
    file.close();
    
    auto start=std::chrono::high_resolution_clock::now();
    bool valid=isvalid(sudoku, n);
    auto elapsed=std::chrono::high_resolution_clock::now()-start;
    long long microseconds=std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    
    // Write log messages.
    for (const auto& msg : outputs) {
        output<<msg<<endl;
    }
    if (valid)
        output<<"Sudoku is valid"<<endl;
    else
        output<<"Sudoku is invalid"<<endl;
    
    output<<"Time taken in microseconds: "<<microseconds<<endl;
    output.close();
    
    return 0;
}
