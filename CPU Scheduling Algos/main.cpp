#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <deque>
#include <map>

using namespace std;

// ANSI color codes for Windows
#define COLOR_RESET "\033[0m"
#define COLOR_CYAN "\033[36m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_GREEN "\033[32m"
#define COLOR_RED "\033[31m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_BLUE "\033[34m"

struct Process {
    int id;
    int arrival;
    int burst;
    int priority;
    int remaining;
    int start = -1;
    int completion = 0;
    int waiting = 0;
    int turnaround = 0;
    int response = -1;
    int last_executed = 0;
};

struct SchedulerResult {
    vector<Process> processes;
    vector<pair<int, pair<int, int>>> gantt;
    vector<vector<int>> readyQueueHistory;
    int contextSwitches = 0;
    double avgWaiting = 0;
    double avgTurnaround = 0;
    double avgResponse = 0;
    double throughput = 0;
};

// Utility functions
void printHeader(string text) {
    system("cls");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 11);
    
    cout << COLOR_CYAN << "\n  " << string(text.length() + 4, '=') << endl;
    cout << "  | " << COLOR_YELLOW << text << COLOR_CYAN << " |" << endl;
    cout << "  " << string(text.length() + 4, '=') << COLOR_RESET << endl;
}

vector<Process> generateProcesses(int n) {
    vector<Process> processes(n);
    srand(time(0));
    
    for(int i = 0; i < n; i++) {
        processes[i] = {i + 1, rand() % 5, 1 + rand() % 10, 1 + rand() % 100};
        processes[i].remaining = processes[i].burst;
        processes[i].last_executed = processes[i].arrival;
    }
    return processes;
}

void displayTable(const vector<Process>& processes) {
    cout << COLOR_GREEN << "\n  |" << string(90, '-') << "|" << endl;
    cout << "  | " << left << setw(5) << "PID" << "| " << setw(8) << "Arrival" << "| " << setw(7) << "Burst"
         << "| " << setw(8) << "Priority" << "| " << setw(11) << "Completion" << "| " << setw(7) << "TAT"
         << "| " << setw(7) << "WT" << "| " << setw(7) << "RT" << " |" << endl;
    cout << "  |" << string(90, '-') << "|" << endl;

    for(const auto& p : processes) {
        cout << "  | " << setw(5) << p.id << "| " << setw(8) << p.arrival << "| " << setw(7) << p.burst
             << "| " << setw(8) << p.priority << "| " << setw(11) << p.completion << "| " << setw(7) << p.turnaround
             << "| " << setw(7) << p.waiting << "| " << setw(7) << p.response << " |" << endl;
    }
    cout << "  |" << string(90, '-') << "|" << COLOR_RESET << endl;
}

void printGanttChart(const vector<pair<int, pair<int, int>>>& gantt, int contextSwitches) {
    // First line: Top border
    cout << COLOR_BLUE << "\n  +";
    for (size_t i = 0; i < gantt.size(); i++) {
        cout << "----+";
    }
    cout << "\n  |";

    // Second line: Process IDs
    for (const auto& entry : gantt) {
        cout << " " << COLOR_YELLOW << "P" << entry.first << COLOR_BLUE << " |";
    }

    // Third line: Bottom border
    cout << "\n  +";
    for (size_t i = 0; i < gantt.size(); i++) {
        cout << "----+";
    }

    // Fourth line: Time markers
    cout << "\n  ";
    int prev_time = 0;
    for (size_t i = 0; i < gantt.size(); i++) {
        if (i == 0) {
            cout << setw(2) << gantt[i].second.first;
        } else {
            cout << setw(5) << gantt[i].second.first;
        }
    }
    // Print last end time if needed
    if (!gantt.empty()) {
        cout << setw(5) << gantt.back().second.second;
    }

    cout << "\n  Context switches: " << contextSwitches << COLOR_RESET << endl;
}

void printReadyQueueRR(const vector<vector<int>>& history) {
    cout << COLOR_MAGENTA << "\n  Ready Queue History:\n";
    for(size_t t = 0; t < history.size(); t++) {
        cout << "  Time " << setw(2) << t << ": ";
        if(history[t].empty()) {
            cout << "[empty]";
        } else {
            for(size_t i = 0; i < history[t].size(); i++) {
                cout << COLOR_YELLOW << "P" << history[t][i] << COLOR_MAGENTA;
                if(i != history[t].size()-1) cout << ", ";
            }
        }
        cout << endl;
    }
    cout << COLOR_RESET;
}

map<int, string> theories = {
    {1, 
        "--> First-Come, First-Served (FCFS):\n"
        "--> Simplest scheduling algorithm\n"
        "--> Non-preemptive\n"
        "--> Processes executed in arrival order\n\n"
        "Formulas:\n"
        "--> Turnaround Time (TAT) = Completion - Arrival\n"
        "--> Waiting Time (WT) = TAT - Burst\n"
        "--> Response Time (RT) = Start - Arrival"},

    {2, 
        "--> Shortest Job First (SJF) Non-Preemptive:\n"
        "--> Selects process with smallest burst time\n"
        "--> Non-preemptive version\n"
        "--> Uses FCFS for tie-breaking\n\n"
        "Formulas:\n"
        "--> WT = Start Time - Arrival\n"
        "--> RT = WT (non-preemptive)"},
        
    {3,
        "--> SJF Preemptive (SRTF):\n"
        "--> Executes process with shortest remaining time\n"
        "--> Preemptive version of SJF\n"
        "--> Can cause context switches on new arrivals\n\n"
        "Formulas:\n"
        "--> WT = TAT - BT\n"
        "--> RT = First Start - Arrival\n"
        "--> TAT = CT - Arrival"},
        
    {4,
        "--> Priority (Preemptive):\n"
        "--> Executes highest priority process\n"
        "--> Implements priority aging\n"
        "--> Preemptive when higher priority arrives\n\n"
        "Formulas:\n"
        "--> Priority = initial ± aging\n"
        "--> WT = TAT - BT\n"
        "--> RT = ST - AT"},
        
    {5,
        "--> Priority (Non-Preemptive):\n"
        "--> Executes highest priority process\n"
        "--> No preemption\n"
        "--> May cause starvation\n\n"
        "Formulas:\n"
        "--> WT = CT - AT - BT\n"
        "--> RT = ST - AT"},
        
    {6, 
        "--> Round Robin (RR) Scheduling:\n"
        "--> Uses time quantum (2 units)\n"
        "--> Preemptive algorithm\n"
        "--> Ready queue maintained in FIFO order\n\n"
        "Formulas:\n"
        "--> WT = CT - AT - BT\n"
        "--> RT = ST - AT\n"
        "--> TQ = 2 (time quantum)"}
};

SchedulerResult FCFS(vector<Process> processes) {
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival < b.arrival || (a.arrival == b.arrival && a.id < b.id);
    });

    int currentTime = 0;
    SchedulerResult result;
    vector<int> readyQueue;

    for(auto& p : processes) {
        readyQueue.push_back(p.id);
        result.readyQueueHistory.push_back(readyQueue);
        
        if(currentTime < p.arrival) currentTime = p.arrival;
        
        p.start = currentTime;
        p.completion = currentTime + p.burst;
        p.turnaround = p.completion - p.arrival;
        p.waiting = p.start - p.arrival;
        p.response = p.start - p.arrival;
        
        result.gantt.push_back({p.id, {currentTime, p.completion}});
        currentTime = p.completion;
        readyQueue.erase(readyQueue.begin());
    }

    result.processes = processes;
    result.contextSwitches = processes.size() - 1;
    return result;
}

SchedulerResult SJFNonPreemptive(vector<Process> processes) {
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival < b.arrival;
    });

    vector<Process> resultList;
    int currentTime = 0;
    int n = processes.size();
    vector<bool> completed(n, false);
    SchedulerResult result;
    vector<int> readyQueue;

    for(int i = 0; i < n; i++) {
        vector<Process> arrived;
        for(int j = 0; j < n; j++) {
            if(!completed[j] && processes[j].arrival <= currentTime) {
                arrived.push_back(processes[j]);
            }
        }

        if(arrived.empty()) {
            currentTime = processes[i].arrival;
            i--;
            continue;
        }

        sort(arrived.begin(), arrived.end(), [](const Process& a, const Process& b) {
            return a.burst < b.burst || (a.burst == b.burst && a.arrival < b.arrival);
        });

        auto& p = arrived[0];
        readyQueue.push_back(p.id);
        result.readyQueueHistory.push_back(readyQueue);
        
        p.start = currentTime;
        p.completion = currentTime + p.burst;
        p.turnaround = p.completion - p.arrival;
        p.waiting = p.start - p.arrival;
        p.response = p.waiting;
        currentTime = p.completion;
        completed[p.id - 1] = true;
        result.gantt.push_back({p.id, {p.start, p.completion}});
        readyQueue.erase(readyQueue.begin());
    }

    result.processes = processes;
    result.contextSwitches = processes.size() - 1;
    return result;
}

SchedulerResult SJFPreemptive(vector<Process> processes) {
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival < b.arrival;
    });

    int currentTime = 0;
    int completed = 0;
    int n = processes.size();
    vector<bool> done(n, false);
    SchedulerResult result;
    int lastPID = -1;
    int currentIdx = -1;

    while(completed < n) {
        int idx = -1;
        int shortestRemaining = INT_MAX;
        
        // Find process with shortest remaining time
        for(int i = 0; i < n; i++) {
            if(!done[i] && processes[i].arrival <= currentTime) {
                if(processes[i].remaining < shortestRemaining || 
                   (processes[i].remaining == shortestRemaining && 
                    processes[i].arrival < processes[idx].arrival)) {
                    shortestRemaining = processes[i].remaining;
                    idx = i;
                }
            }
        }

        if(idx == -1) {
            currentTime++;
            continue;
        }

        // Track context switch
        if(lastPID != -1 && lastPID != processes[idx].id) {
            result.contextSwitches++;
        }

        // Update process execution
        auto& p = processes[idx];
        if(p.start == -1) {
            p.start = currentTime;
            p.response = p.start - p.arrival;
        }

        // Execute for 1 time unit
        p.remaining--;
        currentTime++;
        lastPID = p.id;

        // Update Gantt chart
        if(!result.gantt.empty() && result.gantt.back().first == p.id) {
            result.gantt.back().second.second = currentTime;
        } else {
            result.gantt.push_back({p.id, {currentTime - 1, currentTime}});
        }

        // Check if process completed
        if(p.remaining == 0) {
            done[idx] = true;
            completed++;
            p.completion = currentTime;
            p.turnaround = p.completion - p.arrival;
            p.waiting = p.turnaround - p.burst;
        }
    }

    result.processes = processes;
    return result;
}

SchedulerResult PriorityPreemptive(vector<Process> processes, bool preemptive) {
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival < b.arrival;
    });

    int currentTime = 0;
    int completed = 0;
    int n = processes.size();
    vector<bool> done(n, false);
    SchedulerResult result;
    vector<int> readyQueue;
    int lastPID = -1;
    int agingCounter = 0;

    while(completed < n) {
        // Priority aging
        if(currentTime % 5 == 0 && currentTime > 0) {
            for(auto& p : processes) {
                if(!done[p.id-1] && p.arrival <= currentTime && p.id != lastPID) {
                    p.priority = max(1, p.priority - 2);
                }
            }
        }

        // Find highest priority process
        int idx = -1;
        int highestPriority = INT_MAX;
        for(int i = 0; i < n; i++) {
            if(!done[i] && processes[i].arrival <= currentTime) {
                if(processes[i].priority < highestPriority || 
                   (processes[i].priority == highestPriority && processes[i].arrival < processes[idx].arrival)) {
                    highestPriority = processes[i].priority;
                    idx = i;
                }
            }
        }

        if(idx == -1) {
            currentTime++;
            continue;
        }

        auto& p = processes[idx];
        readyQueue.push_back(p.id);

        if(p.start == -1) {
            p.start = currentTime;
            p.response = p.start - p.arrival;
        }

        int execTime = 1;
        if(!preemptive) {
            execTime = p.remaining;
        }

        // Check for preemption
        if(preemptive && lastPID != -1 && lastPID != p.id) {
            result.contextSwitches++;
        }

        p.remaining -= execTime;
        currentTime += execTime;
        
        if(p.remaining == 0) {
            done[idx] = true;
            completed++;
            p.completion = currentTime;
            p.turnaround = p.completion - p.arrival;
            p.waiting = p.turnaround - p.burst;
        }

        result.gantt.push_back({p.id, {currentTime - execTime, currentTime}});
        lastPID = p.id;
    }

    result.processes = processes;
    return result;
}

SchedulerResult RoundRobin(vector<Process> processes, int quantum = 2) {
    sort(processes.begin(), processes.end(), [](const Process& a, const Process& b) {
        return a.arrival < b.arrival || (a.arrival == b.arrival && a.id < b.id);
    });

    queue<int> rq;
    vector<int> readyQueue;
    int currentTime = 0;
    int n = processes.size();
    SchedulerResult result;
    int lastPID = -1;
    vector<bool> inQueue(n, false);

    // Initial population
    for(int i = 0; i < n; i++) {
        if(processes[i].arrival <= currentTime) {
            rq.push(i);
            inQueue[i] = true;
            readyQueue.push_back(processes[i].id);
        }
    }
    result.readyQueueHistory.push_back(readyQueue);

    while(!rq.empty()) {
        int idx = rq.front();
        rq.pop();
        inQueue[idx] = false;
        auto& p = processes[idx];

        if(p.start == -1) {
            p.start = currentTime;
            p.response = p.start - p.arrival;
        }

        int execTime = min(quantum, p.remaining);
        p.remaining -= execTime;
        
        // Track context switches
        if(lastPID != p.id && lastPID != -1) {
            result.contextSwitches++;
        }
        lastPID = p.id;

        // Update Gantt chart
        result.gantt.push_back({p.id, {currentTime, currentTime + execTime}});
        currentTime += execTime;

        // Update ready queue
        readyQueue.clear();
        queue<int> temp = rq;
        while(!temp.empty()) {
            readyQueue.push_back(processes[temp.front()].id);
            temp.pop();
        }

        // Add newly arrived processes
        for(int i = 0; i < n; i++) {
            if(!inQueue[i] && processes[i].arrival <= currentTime && 
               processes[i].remaining > 0 && i != idx) {
                rq.push(i);
                inQueue[i] = true;
                readyQueue.push_back(processes[i].id);
            }
        }

        // Re-add current process if not finished
        if(p.remaining > 0) {
            rq.push(idx);
            inQueue[idx] = true;
            readyQueue.push_back(p.id);
        }
        else {
            p.completion = currentTime;
            p.turnaround = p.completion - p.arrival;
            p.waiting = p.turnaround - p.burst;
        }

        result.readyQueueHistory.push_back(readyQueue);
    }

    result.processes = processes;
    return result;
}

int main() {
    while(true) {
        printHeader("CPU Scheduling Algorithms Simulator");
        cout << COLOR_MAGENTA 
             << "\n  1. FCFS Scheduling       5. Priority Non-Preemptive\n"
             << "  2. SJF Non-Preemptive    6. Round Robin\n"
             << "  3. SJF Preemptive        7. Exit\n"
             << "  4. Priority Preemptive   \n" << COLOR_RESET;
        cout << COLOR_YELLOW << "\n  Enter your choice: ";

        int choice;
        cin >> choice;
        if(choice == 7) break;

        int n = 5 + rand() % 6; // 5-10 processes
        auto processes = generateProcesses(n);
        SchedulerResult result;

        switch(choice) {
            case 1:
                printHeader("FCFS Scheduling");
                cout << COLOR_GREEN << "\n  Theory:\n" << theories[1] << COLOR_RESET << endl;
                result = FCFS(processes);
                break;
            case 2:
                printHeader("SJF Non-Preemptive");
                cout << COLOR_GREEN << "\n  Theory:\n" << theories[2] << COLOR_RESET << endl;
                result = SJFNonPreemptive(processes);
                break;
            case 3:
                printHeader("SJF Preemptive (SRTF)");
                cout << COLOR_GREEN << "\n  Theory:\n" << theories[3] << COLOR_RESET << endl;
                result = SJFPreemptive(processes);
                break;
            case 4:
                printHeader("Priority Preemptive");
                cout << COLOR_GREEN << "\n  Theory:\n" << theories[4] << COLOR_RESET << endl;
                result = PriorityPreemptive(processes, true);
                break;
            case 5:
                printHeader("Priority Non-Preemptive");
                cout << COLOR_GREEN << "\n  Theory:\n" << theories[5] << COLOR_RESET << endl;
                result = PriorityPreemptive(processes, false);
                break;
            case 6:
                printHeader("Round Robin");
                cout << COLOR_GREEN << "\n  Theory:\n" << theories[6] << COLOR_RESET << endl;
                result = RoundRobin(processes);
                break;
        }

        // Calculate metrics
        double totalWT = 0, totalTAT = 0, totalRT = 0;
        for(const auto& p : result.processes) {
            totalWT += p.waiting;
            totalTAT += p.turnaround;
            totalRT += p.response;
        }

        result.avgWaiting = totalWT / n;
        result.avgTurnaround = totalTAT / n;
        result.avgResponse = totalRT / n;
        result.throughput = n / (double)result.processes.back().completion;

        // Display results
        displayTable(result.processes);
        printGanttChart(result.gantt, result.contextSwitches);
        
        if(choice == 5 || choice == 6) { // Show ready queue only for RR 
            printReadyQueueRR(result.readyQueueHistory);
        }


        cout << COLOR_CYAN << "\n  Average Waiting Time: " << result.avgWaiting
             << "\n  Average Turnaround Time: " << result.avgTurnaround
             << "\n  Average Response Time: " << result.avgResponse
             << "\n  Throughput: " << fixed << setprecision(2) << result.throughput << "/time unit"
             << "\n  Context Switches: " << result.contextSwitches << COLOR_RESET << endl;

        cout << COLOR_YELLOW << "\n  Press enter to continue...";
        cin.ignore();
        cin.get();
    }
    return 0;
}
