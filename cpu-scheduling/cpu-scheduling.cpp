#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>

struct Process {
    int id;
    int arrivalTime;
    int burstTime;
    int waitingTime;
    int turnaroundTime;
    int endTime;
    bool isQueue = false;
};

struct GanttChart {
	int id;
	int time;
};

void enterProcesses(std::vector<Process>& processes) {
    processes.clear(); // remove old data

    int numberOfProcesses;
    std::cout << "Enter the number of processes: ";
    std::cin >> numberOfProcesses;
    while (numberOfProcesses <= 0) {
		std::cout << "The number of processes must be greater than 0. Enter again: ";
		std::cin >> numberOfProcesses;
	}

    std::cout << "==================================\n";

    processes.resize(numberOfProcesses); // set size for vector

    for (int i = 0; i < numberOfProcesses; ++i) {
        std::cout << "Process " << i + 1 << ":\n";
        processes[i].id = i + 1;

        std::cout << "   Arrival Time: ";
        std::cin >> processes[i].arrivalTime;
        while (processes[i].arrivalTime < 0) {
			std::cout << "   Arrival Time must be greater than or equal to 0. Enter again: ";
			std::cin >> processes[i].arrivalTime;
		}

        std::cout << "   Burst Time: ";
        std::cin >> processes[i].burstTime;
        while (processes[i].burstTime <= 0) {
            std::cout << "   Burst Time must be greater than 0. Enter again: ";
            std::cin >> processes[i].burstTime;
        }

        std::cout << "\n";

        // set default value for waiting time and turnaround time
        processes[i].waitingTime = 0;
        processes[i].turnaroundTime = 0;
    }
}

// FCFS algorithm
void FCFS(std::vector<Process>& processes, std::vector< GanttChart>& ganttChart) {
    // sort processes by arrival time
    std::sort(processes.begin(), processes.end(),
        [](const Process& a, const Process& b) {
            return a.arrivalTime < b.arrivalTime;
        });

    int currentTime = 0;
    for (auto& process : processes) {
        // waiting time = 0 if process arrives after CPU is idle, otherwise waiting time = arrival time - current time
        process.waitingTime = std::max(0, currentTime - process.arrivalTime);
        // update current time when process finishes
        currentTime = std::max(currentTime, process.arrivalTime) + process.burstTime;
        // turnaround time
        process.turnaroundTime = process.waitingTime + process.burstTime;
        // end time
        process.endTime = currentTime;

        // save data to gantt chart
        ganttChart.push_back({ process.id, process.endTime });
    }
}

// SJF algorithm
void SJF(std::vector<Process>& processes, std::vector< GanttChart>& ganttChart) {
    // sort processes by arrival time
	std::sort(processes.begin(), processes.end(),
        [](const Process& a, const Process& b) {
			return a.arrivalTime < b.arrivalTime;
		});

	int currentTime = 0;
	int finishedProcesses = 0;
    while (finishedProcesses < processes.size()) {
		int minBurstTime = INT_MAX;
		int minBurstTimeIndex = -1;
        for (int i = 0; i < processes.size(); ++i) {
            if (!processes[i].isQueue && processes[i].arrivalTime <= currentTime && processes[i].burstTime < minBurstTime) {
				minBurstTime = processes[i].burstTime;
				minBurstTimeIndex = i;
			}
		}

        if (minBurstTimeIndex == -1) {
			++currentTime;
			continue;
		}

		// waiting time = current time - arrival time
		processes[minBurstTimeIndex].waitingTime = currentTime - processes[minBurstTimeIndex].arrivalTime;
		// update current time when process finishes
		currentTime += processes[minBurstTimeIndex].burstTime;
		// turnaround time
		processes[minBurstTimeIndex].turnaroundTime = processes[minBurstTimeIndex].waitingTime + processes[minBurstTimeIndex].burstTime;
		// end time
		processes[minBurstTimeIndex].endTime = currentTime;
		// mark process as finished
		processes[minBurstTimeIndex].isQueue = true;
		++finishedProcesses;

        // save data to gantt chart
        ganttChart.push_back({ processes[minBurstTimeIndex].id, processes[minBurstTimeIndex].endTime });
	}   
}

// SRT algorithm
void SRT(std::vector<Process>& processes, std::vector< GanttChart>& ganttChart) {
    int currentTime = 0;
    int completed = 0;
    std::vector<int> remainingBurstTime(processes.size());

    for (int i = 0; i < processes.size(); i++) {
        remainingBurstTime[i] = processes[i].burstTime;
    }

    while (completed != processes.size()) {
        int shortest = INT_MAX;
        int index = -1;
        bool found = false;

        for (int i = 0; i < processes.size(); i++) {
            if (processes[i].arrivalTime <= currentTime && !processes[i].isQueue && remainingBurstTime[i] < shortest) {
                shortest = remainingBurstTime[i];
                index = i;
                found = true;
            }
        }

        if (!found) {
            currentTime++;
            continue;
        }

        remainingBurstTime[index]--;
        currentTime++;

        if (remainingBurstTime[index] == 0) {
            processes[index].isQueue = true;
            completed++;
            processes[index].endTime = currentTime;
            processes[index].turnaroundTime = processes[index].endTime - processes[index].arrivalTime;
            processes[index].waitingTime = processes[index].turnaroundTime - processes[index].burstTime;
        }

        // save data to gantt chart
        // if the current process is different from the previous one
        if (ganttChart.empty() || ganttChart.back().id != processes[index].id) {
			ganttChart.push_back({ processes[index].id, currentTime});
		}
        // else if the current process is the same as the previous one
        else {
            // update the current time of the previous process
            ganttChart.back().time = currentTime;
        }
    }
}

// RR algorithm
void RR(std::vector<Process>& processes, int quantum, std::vector< GanttChart>& ganttChart) {
    int currentTime = 0;
    std::queue<int> q; // Queue to store the index of the processes

    std::vector<int> remainingBurstTime(processes.size());
    for (int i = 0; i < processes.size(); i++) {
        remainingBurstTime[i] = processes[i].burstTime;
    }

    // Initialize the queue with the first process
    q.push(0);
    processes[0].isQueue = true;

    while (!q.empty()) {
        int index = q.front();
        q.pop();

        // Process the current process in queue
        int timeSlice = std::min(quantum, remainingBurstTime[index]);
        currentTime += timeSlice;
        remainingBurstTime[index] -= timeSlice;

        // Check for new arrivals and add them to the queue before the current process (arrived before the current time)
        for (int i = 0; i < processes.size(); i++) {
            if (processes[i].arrivalTime < currentTime && !processes[i].isQueue) {
                q.push(i);
                processes[i].isQueue = true;
            }
        }

        // If current process is not finished, add it back to the queue
        if (remainingBurstTime[index] > 0) {
            
            q.push(index);
        }
        else {
            processes[index].endTime = currentTime;
            processes[index].turnaroundTime = processes[index].endTime - processes[index].arrivalTime;
            processes[index].waitingTime = processes[index].turnaroundTime - processes[index].burstTime;
        }

        // Check for new arrivals and add them to the queue (arrived after the current time)
        for (int i = 0; i < processes.size(); i++) {
            if (processes[i].arrivalTime == currentTime && !processes[i].isQueue) {
                q.push(i);
                processes[i].isQueue = true;
            }
        }

        // save data to gantt chart
        // if the current process is different from the previous one
        if (ganttChart.empty() || ganttChart.back().id != processes[index].id) {
            ganttChart.push_back({ processes[index].id, currentTime});
        }
        // else if the current process is the same as the previous one
        else {
			// update the current time of the previous process
			ganttChart.back().time = currentTime;
		}
    }
}

// reset processes
void resetProcesses(std::vector<Process>& processes) {
    for (auto& process : processes) {
		process.waitingTime = 0;
		process.turnaroundTime = 0;
		process.endTime = 0;
		process.isQueue = false;
	}
}

// reset gantt chart
void resetGanttChart(std::vector<GanttChart>& ganttChart) {
	ganttChart.clear();
}

// print table of processes
void printProcesses(const std::vector<Process>& processes) {
	std::cout << "\n\tProcess\t\tArrival Time\tBurst Time\tWaiting Time\tTurnaround Time\t   End Time\n";
    for (const auto& process : processes) {
		std::cout << "\tP" << process.id << "\t\t" << process.arrivalTime << "\t\t"
			<< process.burstTime << "\t\t" << process.waitingTime << "\t\t"
			<< process.turnaroundTime << "\t\t   " << process.endTime << "\n";
	}

    // average waiting time
    int totalWaitingTime = 0;
    for (const auto& process : processes) {
		totalWaitingTime += process.waitingTime;
	}
    std::cout << "\n\tAverage waiting time: " << (float)totalWaitingTime / processes.size() << "\n";
}

// print gantt chart
void printGanttChart(const std::vector<GanttChart>& ganttChart) {
	std::cout << "\n\tGantt chart:\n\n\t\t";
    for (int i = 0; i < ganttChart.size(); ++i) {
		std::cout << "P" << ganttChart[i].id << "\t";
	}
	std::cout << "\n\t    0\t    ";
    for (int i = 0; i < ganttChart.size(); ++i) {
		std::cout << ganttChart[i].time << "\t    ";
	}
}

int main() {

    std::vector<Process> processes;
    std::vector<GanttChart> ganttChart;
    int choice;


    do {
        enterProcesses(processes);
        /*processes = {
			{1, 0, 11},
			{2, 3, 7},
			{3, 8, 19},
			{4, 13, 4},
			{5, 17, 9}
		};*/  

        // === FCFS algorithm =========================================
        std::cout << "\n========= FCFS:\n";
        FCFS(processes, ganttChart);
        printProcesses(processes);
        printGanttChart(ganttChart);

        // reset data
        resetProcesses(processes);
        resetGanttChart(ganttChart);

        // === SJF algorithm =========================================
        std::cout << "\n\n\n========= SJF:\n";
        SJF(processes, ganttChart);
        printProcesses(processes);
        printGanttChart(ganttChart);

        // reset data
        resetProcesses(processes);
        resetGanttChart(ganttChart);

        // === SRT algorithm =========================================
        std::cout << "\n\n\n========= SRT:\n";
        SRT(processes, ganttChart);
        printProcesses(processes);
        printGanttChart(ganttChart);
        
        // reset data
        resetProcesses(processes);
        resetGanttChart(ganttChart);

        // === RR algorithm =========================================
        std::cout << "\n\n\n========= RR:\n";
        RR(processes, 3, ganttChart); // quantum = 3
        printProcesses(processes);
        printGanttChart(ganttChart);

        // reset data
        resetProcesses(processes);
        resetGanttChart(ganttChart);

        // ask user to continue or exit
        std::cout << "\n\nEnter 0 to exit, otherwise try with other processes: ";
        std::cin >> choice;
        
        // clear screen
        system("cls");
    } while (choice != 0);

    return 0;
}