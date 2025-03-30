#include <iostream>
#include <vector>
#include <map>
using namespace std;


// ====================== Resource Allocation Graph ======================
class RAG {
    int p, r;
    bool singleInstance;
    vector<int> instances;
    vector<vector<int>> adjMatrix;
    map<int, vector<pair<int, int>>> adjList;

public:
    void input() {
        cout << "Single (1) or Multiple (2) instances: ";
        int choice; cin >> choice;
        singleInstance = (choice == 1);

        cout << "Number of processes: "; cin >> p;
        cout << "Number of resources: "; cin >> r;

        if(!singleInstance) {
            instances.resize(r);
            cout << "Enter instances per resource:\n";
            for(int i=0; i<r; i++) {
                cout << "R" << i << ": ";
                cin >> instances[i];
            }
        }

        int total = p + r;
        adjMatrix.resize(total, vector<int>(total, 0));

        cout << "Enter allocations (process, resource) [ -1 to stop ]:\n";
        while(true) {
            int pr, rs;
            cin >> pr;
            if(pr == -1) break;
            cin >> rs;
            adjMatrix[p + rs][pr] = 1;
            adjList[p + rs].push_back({pr, 1});
        }

        cout << "Enter requests (process, resource) [ -1 to stop ]:\n";
        while(true) {
            int pr, rs;
            cin >> pr;
            if(pr == -1) break;
            cin >> rs;
            adjMatrix[pr][p + rs] = 1;
            adjList[pr].push_back({p + rs, 1});
        }
    }

    void print() {
        cout << "\nAdjacency Matrix:\n";
        for(auto row : adjMatrix) {
            for(int val : row) cout << val << " ";
            cout << endl;
        }

        cout << "\nAdjacency List:\n";
        for(auto& [k, v] : adjList) {
            cout << k << " -> ";
            for(auto [n, w] : v) cout << "(" << n << "," << w << ") ";
            cout << endl;
        }
    }

    friend class WFG;
};

// ====================== Main Function ======================
int main() {

    // Resource Allocation Graph
    RAG rag;
    rag.input();
    rag.print();

    return 0;
}