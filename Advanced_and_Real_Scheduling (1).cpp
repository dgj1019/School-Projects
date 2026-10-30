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

struct Process{
    int id;
    float arrival_time;
    float service_time;
    float remaining_service_time;
    float start_time;
    float completion_time;
    float period;
    float absolute_deadline;
    int priority;
    int current_queue;

    Process(){
        id = 0;
        arrival_time = 0.0f;
        service_time = 0.0f;
        remaining_service_time = 0.0f;
        start_time = -1.0f;
        completion_time = -1.0f;
        period = 0.0f;
        absolute_deadline = 0.0f;
        priority = 0;           
        current_queue = 0;
    }

    Process(int pid, float arrival, float service, int pri = 0, float per = 0.0f){
        id = pid;
        arrival_time = arrival;
        service_time = service;
        remaining_service_time = service;
        start_time = -1.0f;
        completion_time = -1.0f;
        period = per;
        absolute_deadline = arrival + per;
        priority = pri;
        current_queue = 0;
    }
};

struct Event{
    enum EventType{
        ARR, DEP, QUANTUM_EXPIRE, BOOST 
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

class EventQueue{
    private:
        struct eventNode{
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
            if(p == nullptr){
                return;
            }
            eventNode* current = head;
            eventNode* previous = nullptr;
            while(current != nullptr){
                if(current->event.process == p){
                    if(previous == nullptr){
                        head = current->next;
                    }
                    else{
                        previous->next = current->next;
                    }
                    eventNode* temp = current;
                    current = current-> next;
                    delete temp;
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

class ReadyQueue{
    private:
        struct readynode{
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

    void boost_all(){
        readynode* current = head;
        while(current != nullptr){
            current->process->current_queue = 0;
            current = current->next;
        }
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
            Process* p = current-> process;
            Process* q = best-> process;
            bool better = false;

            if(schedType == 1){
                int p_wait_intervals = static_cast<int>((clock - p->arrival_time) / 10.0f);
                int p_effective = p->priority - p_wait_intervals;
                if(p_effective < 1){
                    p_effective = 1;
                }

                int q_wait_intervals = static_cast<int>((clock - q->arrival_time) / 10.0f);
                int q_effective = q->priority - q_wait_intervals;
                if(q_effective < 1){
                    q_effective = 1;
                }

                better =(p_effective < q_effective) ||(p_effective == q_effective && p->id < q->id);
            }
            else if(schedType == 2){
                if(p->current_queue < q->current_queue){
                    better = true;
                }
                else if(p->current_queue == q->current_queue){
                    better =(p->arrival_time < q->arrival_time) ||(p->arrival_time == q->arrival_time && p->id < q->id);
                }
            }
            else if(schedType == 3){
                better =(p->period < q->period) ||(p->period == q->period && p->id < q->id);
            }
            else if(schedType == 4){
                better =(p->absolute_deadline < q->absolute_deadline) ||(p->absolute_deadline == q->absolute_deadline && p->id < q->id);
            }

            if(better){
                best = current;
                bestPrevious = previous;
            }
            previous = current;
            current = current-> next;
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

struct Metrics{
    double completion;
    double turnaround;
    double waiting;
    double max_waiting;
    double response;
    double utilization;
    int deadline_misses;
};

void generate_interactive_processes(Process processes[]){
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
        int priority =(rand() % 10) + 1; 
        processes[i] = Process(i + 1, current_arrival, service_time, priority);

    }
}

void generate_periodic_tasks(Process realtime_tasks[]){
    int count = 0;

    for(float t = 0.0f; t < 12.0f; t += 4.0f){
        realtime_tasks[count++] = Process(1, t, 1.0f, 0, 4.0f);
    }
    for(float t = 0.0f; t < 12.0f; t += 6.0f){
        realtime_tasks[count++] = Process(2, t, 2.0f, 0, 6.0f);
    }
    for(float t = 0.0f; t < 12.0f; t += 12.0f){
        realtime_tasks[count++] = Process(3, t, 3.0f, 0, 12.0f);
    }
}

Metrics run_simulation(Process input_processes[], int count, int policy){
    Process processes[MAX_PROCESSES];
    for(int i = 0; i < count; i++){
        processes[i] = input_processes[i];
    }
    EventQueue events;
    ReadyQueue ready;

    for(int i = 0; i < count; i++){
        events.add_event(Event(Event::ARR, processes[i].arrival_time, &processes[i]));
    }

    if(policy == 2){
        events.add_event(Event(Event::BOOST, 20.0f, nullptr));
    }

    float clock = 0.0f;
    float last_start_time = 0.0f;
    double busy_time = 0.0;
    Process* running = nullptr;

    double total_turnaround = 0.0;
    double total_waiting = 0.0;
    double total_response = 0.0;
    double max_waiting = 0.0;
    int deadline_misses = 0;

    int completed_processes = 0;
    while(!events.is_empty() && completed_processes < count){
        Event e = events.get_next_event();
        clock = e.time;

        if(e.type == Event::ARR){
            e.process->current_queue = 0;
            if((policy == 3 || policy == 4) && running != nullptr){
                bool preempt = false;
                if(policy == 3 && e.process->period < running->period){
                    preempt = true; 
                }
                else if(policy == 4 && e.process->absolute_deadline < running->absolute_deadline){
                    preempt = true; 
                }

                if(preempt){
                    float elapsed = clock - last_start_time;
                    busy_time += elapsed;
                    running->remaining_service_time -= elapsed;
                    events.cancel_event(running);
                    ready.add_ready(running);
                    running = nullptr;
                }
            }
            ready.add_ready(e.process);
        }
        else if(e.type == Event::DEP && e.process == running){
            float elapsed = clock - last_start_time;
            busy_time += elapsed;
            running->remaining_service_time = 0.0f;
            running->completion_time = clock;

            float turnaround = running->completion_time - running->arrival_time;
            float waiting = turnaround - running->service_time;
            float response = running->start_time - running->arrival_time;

            total_turnaround += turnaround;
            total_waiting += waiting;
            total_response += response;

            if(waiting > max_waiting){
                max_waiting = waiting;
            }

            if((policy == 3 || policy == 4) && running->completion_time > running->absolute_deadline){
                deadline_misses++;
            }
            completed_processes++;
            running = nullptr;
        }
        else if(e.type == Event::QUANTUM_EXPIRE){
            float elapsed = clock - last_start_time;
            busy_time += elapsed;
            running->remaining_service_time -= elapsed;
            running->current_queue = 1;
            ready.add_ready(running);
            running = nullptr;
        }
        else if(e.type == Event::BOOST){
            if(policy == 2){
                ready.boost_all();
                if(running != nullptr){
                    running->current_queue = 0;
                }
                if(completed_processes < count){
                    events.add_event(Event(Event::BOOST, clock + 20.0f, nullptr));
                }
            }
        }

        if(running == nullptr && !ready.is_empty()){
            running = ready.get_ready(policy, clock);

            if(running->start_time < 0.0f){
                running->start_time = clock;
            }
            last_start_time = clock;

            if(policy == 2 && running->current_queue == 0 && running->remaining_service_time > 2.0f){
                events.add_event(Event(Event::QUANTUM_EXPIRE, clock + 2.0f, running));
            } 
            else{
                events.add_event(Event(Event::DEP, clock + running->remaining_service_time, running));
            }
        }
    }

    Metrics m;
    m.completion = clock;
    m.turnaround = total_turnaround / count;
    m.waiting = total_waiting / count;
    m.response = total_response / count;
    m.utilization =(busy_time / clock) * 100.0;
    m.max_waiting = max_waiting;
    m.deadline_misses = deadline_misses;

    return m;
}

int main(){
    Process processes[MAX_PROCESSES];
    generate_interactive_processes(processes);

    Metrics m1 = run_simulation(processes, MAX_PROCESSES, 1);
    Metrics m2 = run_simulation(processes, MAX_PROCESSES, 2);

    cout << fixed << setprecision(3);
    cout << "Part 1: Advanced Schedulers Summary(1,000 Processes)" << endl << endl;
    cout << "| " << right << setw(30) << "Metric" << " | Priority w/ Aging| MLFQ(Boost S=20ms)  |" << endl;
    cout << "|--------------------------------|-----------------:|--------------------:|" << endl;
    
    cout << "| " << right << setw(30) << "Total Completion Time(ms)" 
         << " | " << setw(16) << m1.completion 
         << " | " << setw(19) << m2.completion << " |" << endl;

    cout << "| " << right << setw(30) << "Average Turnaround Time(ms)" 
         << " | " << setw(16) << m1.turnaround 
         << " | " << setw(19) << m2.turnaround << " |" << endl;

    cout << "| " << right << setw(30) << "Average Waiting Time(ms)" 
         << " | " << setw(16) << m1.waiting 
         << " | " << setw(19) << m2.waiting << " |" << endl;

    cout << "| " << right << setw(30) << "Average Response Time(ms)" 
         << " | " << setw(16) << m1.response 
         << " | " << setw(19) << m2.response << " |" << endl;

    cout << "| " << right << setw(30) << "Overall CPU Utilization(%)" 
         << " | " << setw(16) << m1.utilization 
         << " | " << setw(19) << m2.utilization << " |" << endl;

    cout << "| " << right << setw(30) << "Maximum Waiting Time(ms)" 
         << " | " << setw(16) << m1.max_waiting 
         << " | " << setw(19) << m2.max_waiting << " |" << endl;

    Process realtime_tasks[MAX_PROCESSES];
    generate_periodic_tasks(realtime_tasks);

    Metrics m3 = run_simulation(realtime_tasks, 6, 3);
    Metrics m4 = run_simulation(realtime_tasks, 6, 4);

    double total_utilization = (1.0/4.0) + (2.0/6.0) + (3.0/12.0);
    double rm_bound = 3.0 *(pow(2.0, 1.0/3.0) - 1.0);

    cout << endl << "Part 2: Real-Time Periodic Schedulers(Hyper-period = 12 ms)" << endl << endl;
    cout << "| " << right << setw(30) << "Scheduler Policy" 
         << " | Total Utilization(U)  | Schedulable Bound | Deadlines Met? | Misses |" << endl;
    cout << "|--------------------------------|-----------------------|-------------------|----------------|--------|" << endl;

    cout << "| " << right << setw(30) << "Rate-Monotonic(RM)" 
         << " | " << setw(21) << total_utilization 
         << " | " << setw(17) << rm_bound 
         << " | " << setw(14) <<(m3.deadline_misses == 0 ? "Yes" : "No")
         << " | " << setw(6) << m3.deadline_misses << " |" << endl;

    cout << "| " << right << setw(30) << "Earliest-Deadline-First(EDF)" 
         << " | " << setw(21) << total_utilization 
         << " | " << setw(17) << 1.000 
         << " | " << setw(14) <<(m4.deadline_misses == 0 ? "Yes" : "No")
         << " | " << setw(6) << m4.deadline_misses << " |" << endl;

    return 0;
}