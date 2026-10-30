#include <iostream>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <string>
using namespace std;

const int MAX_PROCESSES = 1000;
const double LAMBDA_PER_MS = 2.0;
const double T_MS = 1.0;
const double QUANTUM = 2.0;

struct Process {
    int id;
    float arrival_time;
    float service_time;
    float remaining_service_time;
    float start_time;
    float completion_time;

    Process(){
        id = 0;
        arrival_time = 0.0f;
        service_time = 0.0f;
        remaining_service_time = 0.0f;
        start_time = -1.0f;
        completion_time = -1.0f;
    }

    Process(int pid, float arrival, float service){
        id = pid;
        arrival_time = arrival;
        service_time = service;
        remaining_service_time = service;
        start_time = -1.0f;
        completion_time = -1.0f;
    }
};

struct Event {
    enum EventType{
        ARR, DEP, QUANTUM_EXPIRE 
    };
    EventType type;
    float time;
    Process* process;

    Event(){
        type = ARR;
        time = 0.0f;
        process = nullptr;
    }

    Event(EventType t, float et, Process* p){
        type = t;
        time = et;
        process = p;
    }
};

class EventQueue {
    private:
        struct eventNode {
            Event event;
            eventNode* next;
            eventNode(Event e){
                event = e;
                next = nullptr;
            }
        };
        eventNode* head;

    public:
        EventQueue(){ 
            head = nullptr; 
        }

        void add_event(Event e){
            eventNode* neweventNode = new eventNode(e);
            if(head == nullptr || e.time < head->event.time){
                neweventNode->next = head;
                head = neweventNode;
                return;
            }
            eventNode* current = head;
            while(current->next != nullptr && current->next->event.time <= e.time)
                current = current->next;
            neweventNode->next = current->next;
            current->next = neweventNode;
        }

        Event get_next_event(){
            if(head == nullptr){
                return Event();
            }
            eventNode* temp = head;
            Event e = temp->event;
            head = head->next;
            delete temp;
            return e;
        }

        void cancel_event(Process* p){
            eventNode* current = head;
            eventNode* previous = nullptr;
            while(current != nullptr){
                if(current->event.type == Event::DEP && current->event.process == p){
                    if(previous == nullptr){
                        head = current->next;
                    }
                    else{
                        previous->next = current->next;
                    }
                    delete current;
                    return;
                }
                previous = current;
                current = current->next;
            }
        }

        bool is_empty(){
            return head == nullptr;
        }

        ~EventQueue(){
            while(head != nullptr){
                eventNode* temp = head;
                head = head->next;
                delete temp;
            }
        }
};

class ReadyQueue {
private:
    struct readynode {
        Process* process;
        readynode* next;
        readynode(Process* p){
            process = p;
            next = nullptr;
        }
    };
    readynode* head;
    readynode* tail;
    int count;

public:
    ReadyQueue(){
        head = nullptr;
        tail = nullptr;
        count = 0;
    }

    void add_ready(Process* p){
        readynode* newreadynode = new readynode(p);
        if(tail == nullptr){
            head = newreadynode;
        }
        else{
            tail->next = newreadynode;
        }
        tail = newreadynode;
        count++;
    }

    Process* get_ready(int schedType, float clock){
        if(head == nullptr){
            return nullptr;
        }
        readynode* best = head;
        readynode* bestPrevious = nullptr;
        readynode* previous = head;
        readynode* current = head->next;

        while(current != nullptr){
            Process* p = current->process;
            Process* q = best->process;
            bool better = false;

            if(schedType == 1 || schedType == 2){
                float pTime = p->service_time;
                float qTime = q->service_time;
                if(schedType == 2){
                    pTime = p->remaining_service_time;
                    qTime = q->remaining_service_time;
                }
                better = pTime < qTime || (pTime == qTime && p->id < q->id);
            }
            else if(schedType == 4){
                double pRatio = (clock - p->arrival_time + p->service_time) / p->service_time;
                double qRatio = (clock - q->arrival_time + q->service_time) / q->service_time;
                better = (pRatio > qRatio || (pRatio == qRatio && p->id < q->id));
            }

            if(better){
                best = current;
                bestPrevious = previous;
            }
            previous = current;
            current = current->next;
        }

        if(bestPrevious == nullptr){
            head = best->next;
        }
        else{
            bestPrevious->next = best->next;
        }

        if(tail == best){
            tail = bestPrevious;
        }
        Process* result = best->process;
        delete best;
        count--;
        return result;
    }

    int size(){
        return count;
    }
    bool is_empty(){
        return head == nullptr;
    }

    ~ReadyQueue(){
        while(head != nullptr){
            readynode* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

struct Metrics {
    double completion;
    double turnaround;
    double waiting;
    double response;
    double utilization;
    double throughput;
    double ready_length;
};

void generate_processes(Process processes[]){
    srand(static_cast<unsigned int>(time(nullptr)));
    float current_arrival = 0.0f;

    for(int i = 0; i < MAX_PROCESSES; i++){
        if(i > 0){
            double u1 = static_cast<double>(rand()) / RAND_MAX;
            while(u1 == 0.0) u1 = static_cast<double>(rand()) / RAND_MAX;
            current_arrival += static_cast<float>(-log(u1) / LAMBDA_PER_MS);
        }
        double u2 = static_cast<double>(rand()) / RAND_MAX;
        while(u2 == 0.0){
            u2 = static_cast<double>(rand()) / RAND_MAX;
        }
        float service_time = static_cast<float>(-T_MS * log(u2));
        processes[i] = Process(i + 1, current_arrival, service_time);
    }
}

Metrics run_fifo_simulation(Process original[], int schedType){
    Process processes[MAX_PROCESSES];
    EventQueue events;
    ReadyQueue ready;

    for(int i = 0; i < MAX_PROCESSES; i++){
        processes[i] = original[i];
        events.add_event(Event(Event::ARR, processes[i].arrival_time, &processes[i]));
    }

    Process* running = nullptr;
    float clock = 0.0f;
    float last_start_time = 0.0f;
    double busy_time = 0.0;
    double ready_area = 0.0;

    while(!events.is_empty()){
        Event e = events.get_next_event();
        ready_area += ready.size() * static_cast<double>(e.time - clock);
        clock = e.time;

        if(e.type == Event::ARR){
            ready.add_ready(e.process);
            if(schedType == 2 && running != nullptr){
                float elapsed = clock - last_start_time;
                float remaining = running->remaining_service_time - elapsed;
                if(e.process->remaining_service_time < remaining){
                    events.cancel_event(running);
                    running->remaining_service_time = remaining;
                    busy_time += elapsed;
                    ready.add_ready(running);
                    running = nullptr;
                }
            }
        }
        else if(e.type == Event::DEP && e.process == running){
            busy_time += clock - last_start_time;
            running->remaining_service_time = 0.0f;
            running->completion_time = clock;
            running = nullptr;
        }
        else if(e.type == Event::QUANTUM_EXPIRE && e.process == running){
            float elapsed = clock - last_start_time;
            busy_time += elapsed;
            running->remaining_service_time -= elapsed;
            ready.add_ready(running);
            running = nullptr;
        }

        if(running == nullptr && !ready.is_empty()){
            running = ready.get_ready(schedType, clock);
            if(running->start_time < 0.0f){
                running->start_time = clock;
            }
            last_start_time = clock;
            if(schedType == 3 && running->remaining_service_time > QUANTUM){
                events.add_event(Event(Event::QUANTUM_EXPIRE, clock + static_cast<float>(QUANTUM), running));
            }
            else{
                events.add_event(Event(Event::DEP, clock + running->remaining_service_time, running));
            }
        }
    }

    double turnaround = 0.0;
    double waiting = 0.0;
    double response = 0.0;
    float finish = 0.0f;
    
    for(int i = 0; i < MAX_PROCESSES; i++){
        turnaround += processes[i].completion_time - processes[i].arrival_time;
        waiting += processes[i].completion_time - processes[i].arrival_time - processes[i].service_time;
        response += processes[i].start_time - processes[i].arrival_time;
        if(processes[i].completion_time > finish){
            finish = processes[i].completion_time;
        }
    }
    Metrics m;
    m.completion = finish / 1000.0;
    m.turnaround = (turnaround / MAX_PROCESSES) / 1000.0;
    m.waiting = (waiting / MAX_PROCESSES) / 1000;
    m.response = (response / MAX_PROCESSES) / 1000;
    m.utilization = (finish > 0.0f) ? busy_time / finish * 100.0 : 0.0;
    m.throughput = MAX_PROCESSES * 1000.0 / finish;
    m.ready_length = ready_area / finish;
    return m;
}

int main(){
    Process processes[MAX_PROCESSES];
    generate_processes(processes);
    Metrics results[5];
    for(int i = 0; i < 5; i++){
        results[i] = run_fifo_simulation(processes, i);
    }

    cout << fixed << setprecision(3);
    cout << "### CS 3360 Assignment 3 Performance Summary (1000 Processes)" << endl << endl;
    cout << "| Metric | FCFS | SJF | SRTF | RR (q=2.0) | HRRN |" << endl;
    cout << "|---|---:|---:|---:|---:|---:|" << endl;
    const char* labels[7] = {
        "Total Completion Time (s)",
        "Average Turnaround Time (s)",
        "Average Waiting Time (s)",
        "Average Response Time (s)",
        "CPU Utilization (%)",
        "System Throughput (proc/s)",
        "Avg. Ready Queue Length"
    };
    for(int row = 0; row < 7; row++){
        cout << "| " << labels[row] << " |";
        for(int j = 0; j < 5; j++){
            double value = 0.0;
            if(row == 0){
                value = results[j].completion;
            }
            else if(row == 1){
                value = results[j].turnaround;
            }
            else if(row == 2){
                value = results[j].waiting;
            }
            else if(row == 3){
                value = results[j].response;
            }
            else if(row == 4){
                value = results[j].utilization;
            }
            else if(row == 5){
                value = results[j].ready_length;
            }
            else{
                value = results[j].throughput;
            }
            cout << " " << value << " |";
        }
        cout << endl;
    }
    return 0;
}