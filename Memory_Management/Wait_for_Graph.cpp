#include <iostream>
#include <vector>
#include <map>
using namespace std;

// ====================== Wait-for Graph ======================
class WFG {
    vector<vector<int>> adj;
    map<int, vector<int>> adjList;

public:
    void buildFromRAG(RAG& rag) {
        adj.resize(rag.p, vector<int>(rag.p, 0));
        
        for(int pi=0; pi<rag.p; pi++) {
            for(auto& [res, w] : rag.adjList[pi]) {
                int resource = res - rag.p;
                for(auto& [pr, alloc] : rag.adjList[res]) {
                    adj[pi][pr] = 1;
                    adjList[pi].push_back(pr);
                }
            }
        }
    }

    bool hasCycle() {
        vector<bool> visited(adj.size(), false);
        vector<bool> recStack(adj.size(), false);

        for(int i=0; i<adj.size(); i++)
            if(!visited[i] && dfs(i, visited, recStack))
                return true;
        return false;
    }

    bool dfs(int v, vector<bool>& visited, vector<bool>& recStack) {
        if(!visited[v]) {
            visited[v] = true;
            recStack[v] = true;

            for(int neighbor : adjList[v]) {
                if(!visited[neighbor] && dfs(neighbor, visited, recStack))
                    return true;
                else if(recStack[neighbor])
                    return true;
            }
        }
        recStack[v] = false;
        return false;
    }

    void print() {
        cout << "\nWFG Adjacency List:\n";
        for(auto& [k, v] : adjList) {
            cout << k << " -> ";
            for(int n : v) cout << n << " ";
            cout << endl;
        }
    }
};

// ====================== Main Function ======================
int main() {

    // Wait-for Graph
    WFG wfg;
    wfg.buildFromRAG(rag);
    wfg.print();

    cout << "\nDeadlock detected: " << (wfg.hasCycle() ? "Yes" : "No") << endl;

    return 0;
}