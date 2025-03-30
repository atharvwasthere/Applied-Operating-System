#include <iostream>
#include <vector>
#include <map>
using namespace std;

// ====================== Banker's Algorithm ======================
class BankersAlgorithm {
    int n, m;
    vector<int> available;
    vector<vector<int>> max;
    vector<vector<int>> allocated;
    vector<vector<int>> need;

public:
    void input() {
        cout << "Enter number of processes: ";
        cin >> n;
        cout << "Enter number of resources: ";
        cin >> m;

        available.resize(m);
        cout << "Enter available resources: ";
        for(int i=0; i<m; i++) cin >> available[i];

        max.resize(n, vector<int>(m));
        cout << "Enter max matrix:\n";
        for(int i=0; i<n; i++) {
            cout << "Process " << i << ": ";
            for(int j=0; j<m; j++) cin >> max[i][j];
        }

        allocated.resize(n, vector<int>(m));
        cout << "Enter allocated matrix:\n";
        for(int i=0; i<n; i++) {
            cout << "Process " << i << ": ";
            for(int j=0; j<m; j++) cin >> allocated[i][j];
        }

        computeNeed();
    }

    void computeNeed() {
        need.resize(n, vector<int>(m));
        for(int i=0; i<n; i++)
            for(int j=0; j<m; j++)
                need[i][j] = max[i][j] - allocated[i][j];
    }

    bool isSafe() {
        vector<int> work = available;
        vector<bool> finish(n, false);
        int count = 0;

        while(count < n) {
            bool found = false;
            for(int i=0; i<n; i++) {
                if(!finish[i]) {
                    bool possible = true;
                    for(int j=0; j<m; j++) 
                        if(need[i][j] > work[j]) {
                            possible = false;
                            break;
                        }
                    
                    if(possible) {
                        for(int j=0; j<m; j++)
                            work[j] += allocated[i][j];
                        finish[i] = true;
                        count++;
                        found = true;
                    }
                }
            }
            if(!found) return false;
        }
        return true;
    }

    bool request(int pi, vector<int> req) {
        for(int j=0; j<m; j++) {
            if(req[j] > need[pi][j] || req[j] > available[j]) {
                cout << "Request cannot be granted\n";
                return false;
            }
        }

        for(int j=0; j<m; j++) {
            available[j] -= req[j];
            allocated[pi][j] += req[j];
            need[pi][j] -= req[j];
        }

        if(!isSafe()) {
            for(int j=0; j<m; j++) {
                available[j] += req[j];
                allocated[pi][j] -= req[j];
                need[pi][j] += req[j];
            }
            cout << "System would be unsafe. Rolling back\n";
            return false;
        }
        return true;
    }
};




// ====================== Main Function ======================
int main() {
    // Banker's Algorithm
    BankersAlgorithm ba;
    ba.input();
    int pi = 1;
    vector<int> req = {1, 0, 0};
    ba.request(pi, req);


    return 0;
}