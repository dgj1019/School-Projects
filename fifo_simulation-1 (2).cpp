#include <iostream>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <random>
using namespace std;

const int MAX_PROCESSES = 1000;
const double LAMBDA_PER_MS = 2.0;
const double T_MS = 1.0;

struct Process{
    public:
        int pid;
        int arrival_time;
        int service_time;
        int start_time;
        int end_time;
        string state;
        
        Process(){
            pid = 0;
            arrival_time = 0;
            service_time = 0;
            start_time = 0;
            end_time = 0;
            state = "NEW";
        }
        
        Process(int p, int a_t, int s_t){
            this->pid = p;
            this->arrival_time = a_t;
            this->service_time = s_t;
            this->start_time = 0;
            this->end_time = 0;
            this->state = "NEW";
        }
};


void generate_processes(Process arrayProcess[]){
    srand(static_cast<unsigned int>(time(NULL)));
    int current_arrival = 0;
    for(int i = 0; i < MAX_PROCESSES; i++){
        int inter_arrival_ms = 0;
        if (i > 1){
            double u1 = static_cast<double>(rand()) / RAND_MAX;
            while (u1 == 0.0) {
                u1 = static_cast<double>(rand()) / RAND_MAX;
            }
            inter_arrival_ms = static_cast<int>(round(-log(u1) / LAMBDA_PER_MS));
        }
        current_arrival += inter_arrival_ms;
        
        double u2 = static_cast<double>(rand()) / RAND_MAX;
        while(u2 == 0.0){
            u2 = static_cast<double>(rand()) / RAND_MAX;
        }
        int service_time_ms = static_cast<int>(round(-T_MS * log(u2)));
        
        arrayProcess[i] = Process(i + 1, current_arrival, service_time_ms);
    }
    
    cout << "**Process Workload**" << endl << endl;
    cout << "process_id | arrival_time | requested_service_time" << endl;
    
    for(int i = 0; i < MAX_PROCESSES; i++){
        cout << left << setw(10) << arrayProcess[i].pid << " | " 
        << setw(12) << arrayProcess[i].arrival_time << " | "
        << arrayProcess[i].service_time << endl;
        
    }
    cout << endl << endl;
}

void run_fifo_simulation(Process arrayProcess[]){
    cout << endl << "**CPU Simulation Trace**" << endl;
    cout << "Time (in ms)" << right << setw(8) << " | " << "CPU Status" << " | PID " << endl;
    
    int current_time = 0;
    int total_service_time = 0;
    
    for (int i = 0; i < MAX_PROCESSES; i++){
        Process& processes = arrayProcess[i];
        processes.state = "READY";
        total_service_time += processes.service_time;
        
        if (current_time < processes.arrival_time){
            cout << "[" << right << setw(7) << current_time << "," 
            << setw(7) << arrayProcess[i].arrival_time << ") | " << left << setw(10) << "IDLE"
            << " | " << endl;
            current_time = arrayProcess[i].start_time;
        }
        
        processes.start_time = current_time;
        processes.state = "RUNNING";
        processes.end_time = current_time + processes.service_time;
        
        cout << "[" << right << setw(7) << arrayProcess[i].start_time << "," 
        << setw(7) << arrayProcess[i].end_time << ") | " << left << setw(10) << "BUSY" << " | " 
        << arrayProcess[i].pid << right << endl;
        current_time = arrayProcess[i].end_time;
        
        processes.state = "TERMINATED";
        current_time = processes.end_time;
    }
    cout << endl << endl;
    
    double last_arrival = static_cast<double>(arrayProcess[MAX_PROCESSES - 1].arrival_time);
    double avg_arrival_time = (last_arrival > 0) ? (MAX_PROCESSES / last_arrival) : 0.0;
    double avg_service_time = static_cast<double>(total_service_time) / MAX_PROCESSES;
    
    cout << "**Generation Statistics** " << endl;
    cout << "Statistics                    | Results" << endl;
    cout << "------------------------------|-----------------" << endl;
    cout << "Expected Average Arrival Rate |" << fixed << setprecision(1) << LAMBDA_PER_MS << " processes per ms" << endl;
    cout << "Computed Average Arrival Rate |" << fixed << setprecision(2) << avg_arrival_time << endl;
    cout << "Expected Average Service Time |" << fixed << setprecision(1) << T_MS << endl;
    cout << "Computed Average Service Time |" << fixed << setprecision(2) << avg_service_time << endl << endl;
    
    int total_complete_time = arrayProcess[MAX_PROCESSES - 1].end_time;
    
    double total_turnaround = 0.0;
    double total_waiting = 0.0;
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        total_turnaround += (arrayProcess[i].end_time - arrayProcess[i].arrival_time);
        total_waiting += (arrayProcess[i].start_time - arrayProcess[i].arrival_time);
    }

    double avg_turnaround_time = total_turnaround / MAX_PROCESSES;
    double avg_waiting_time = total_waiting / MAX_PROCESSES;
    double cpu_utilization = (total_complete_time > 0) ? (static_cast<double>(total_service_time) / total_complete_time * 100.0) : 0.0;
    double system_throughput = (static_cast<double>(MAX_PROCESSES) / total_complete_time) * 1000.0;
    cout << "**Simulation Statistics**\n\n";
    cout << "Statistics                    | Results\n";
    cout << "------------------------------|-----------------\n";
    cout << "Total Complete Time           | " << total_complete_time << "ms\n";
    cout << "Average Turnaround Time       | " << fixed << setprecision(2) << avg_turnaround_time << "ms\n";
    cout << "Average Waiting Time          | " << fixed << setprecision(2) << avg_waiting_time << "ms\n";
    cout << "Overall CPU utilization       | " << fixed << setprecision(2) << cpu_utilization << "%\n";
    cout << "Overall System Throughput     | " << fixed << setprecision(2) << system_throughput << " processes/sec\n";
    cout << "\n";
}


int main()
{
    Process processes[MAX_PROCESSES];
    generate_processes(processes);
    run_fifo_simulation(processes);
    
    return 0;
}