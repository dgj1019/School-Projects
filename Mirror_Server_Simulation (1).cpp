#include <iostream>
#include <random>
#include <iomanip>
#include <cmath>
#include <ctime>
using namespace std;

const int DOWN_TIME = 2;
int totalTime;


class MirrorServer{
    public:
        int MTBF;
        double rate;
        int downCounter;
        int upCounter;
        int totalUptime;

        MirrorServer(){
            MTBF = 0;
            rate = 0.0;
            downCounter = 0;
            upCounter = 0;
            totalUptime = 0;
        }

        MirrorServer(int meanTime){
            this->MTBF = meanTime;
            this->rate = 1.0 / meanTime;
            this->downCounter = 0;
            this->upCounter = 0;
            this->totalUptime = 0;
        }
};


void generate_MTBF(MirrorServer arrayServer[], int amountServers){

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(10, 20);
    for(int i = 0; i < amountServers; i++){
        arrayServer[i].MTBF = dist(gen);
        arrayServer[i].rate = 1.0 / arrayServer[i].MTBF;
    }

    for(int i = 0; i < amountServers; i++){
        cout << "S0" << i << " | " << arrayServer[i].MTBF << " |" << endl;
    }
    cout << endl << endl;
}

void generate_Status(MirrorServer arrayServer[], int amountServers){
    cout << setw(6) << "| " << "S0";
    for(int i = 0; i < amountServers; i++){
        if(i == amountServers - 1){
            cout << i << "  |" << endl;
        }
        else{
            cout << i << "  | " << "S0";
        }
    }

    cout << "-------------------";
    for(int i = 2; i < amountServers; i++){
        cout << "-------";
    }
    cout << endl;

    int upCount[amountServers];
    int downCount[amountServers];
    for(int i = 0; i < amountServers; i++) {
        upCount[i] = 0;
        downCount[i] = 0;
    }

    int currentTime = 0;
    cout << right << setw(4) << currentTime << "| ";
    for(int i = 0; i < amountServers; i++){
        cout << " UP  | ";
        upCount[i]++;
    }
    cout << endl;

    int nextRecoveryTime[amountServers];
    int currentServTime[amountServers];
    for(int i = 0; i < amountServers; i++) {
        nextRecoveryTime[i] = 0;
        currentServTime[i] = 0;
    }

    int nextFailTime[amountServers];
    int nextTimeStamp;
    srand(static_cast<unsigned int>(time(NULL)));
    for(int i = 0; i < amountServers; i++){
        double u1 = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
        while(u1 == 0.0){
            u1 = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
        }
        nextFailTime[i] = static_cast<int>(round(-log(u1) / arrayServer[i].rate));
        arrayServer[i].totalUptime = nextFailTime[i];
    }

    bool allDown = false;
    while(!allDown){
        bool firstEvent = true;
        for(int i = 0; i < amountServers; i++){
            if(currentServTime[i] == 0){
                if(firstEvent || nextFailTime[i] < nextTimeStamp){
                    nextTimeStamp = nextFailTime[i];
                    firstEvent = false;
                }
            }
            else{
                if(firstEvent || nextRecoveryTime[i] < nextTimeStamp){
                    nextTimeStamp = nextRecoveryTime[i];
                    firstEvent = false;
                }
            }
        }

        int finalTime;
        currentTime = nextTimeStamp;
        for(int i = 0; i < amountServers; i++){
            if(currentServTime[i] == 0 && nextFailTime[i] == currentTime){
                currentServTime[i] = 1;
                nextRecoveryTime[i] = currentTime + DOWN_TIME;
            }
            else if(currentServTime[i] == 1 && nextRecoveryTime[i] == currentTime){
                currentServTime[i] = 0;
                double u1 = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
                while(u1 == 0.0){
                    u1 = static_cast<double>(rand()) /
                         static_cast<double>(RAND_MAX);
                }
                nextFailTime[i] = currentTime + static_cast<int>(round(-log(u1) / arrayServer[i].rate));
                arrayServer[i].totalUptime += static_cast<int>(round(-log(u1) / arrayServer[i].rate));
                finalTime = nextFailTime[amountServers - 1];
            }
        }
        totalTime = finalTime;
        

        cout << right << setw(4) << currentTime << "|  ";
        for(int i = 0; i < amountServers; i++){
            if(currentServTime[i] == 0){
                cout << "UP  |  ";
                upCount[i]++;

            }
            else{
                cout << "DOWN|  ";
                downCount[i]++;
            }
        }

        cout << endl;
        allDown = true;
        for(int i = 0; i < amountServers; i++){
            if(currentServTime[i] == 0){
                allDown = false;
            }
        }
    }
    cout << endl << endl;
}

void performance_Metrics(int amountServers, MirrorServer arrayServer[]){
    int avgUptime[amountServers];
    int avgDowntime[amountServers];
    double availability[amountServers];
    for(int i = 0; i < amountServers; i++){
        avgUptime[i] = arrayServer[i].totalUptime;
        avgDowntime[i] = totalTime - arrayServer[i].totalUptime;
        availability[i] = (static_cast<double>(avgUptime[i]) / static_cast<double>(totalTime)) * 100;
    }

    cout << "Server  | Avg Uptime | Avg Downtime | Availability   | MTBF" << endl;
    cout << "------------------------------------------------------------------" << endl;

    for(int i = 0; i < amountServers; i++){
        cout << left << setw(2) << "S0" << i << "     | " << setw(11) << avgUptime[i] << "| " 
        << setw(13)<< avgDowntime[i] << "|" << right << setw(15) << availability[i] << "%"
        << "|" << right << setw(12) << arrayServer[i].MTBF << endl;
    }
}


int main(){
    int numServers;

    cout << "Pick the number of servers between 2 and 5 to test: ";
    cin >> numServers;
    while(numServers < 2 || numServers > 5){
        cout << "Pick again: ";
        cin >> numServers;
    }

    MirrorServer serverSimulation[5];

    generate_MTBF(serverSimulation, numServers);
    generate_Status(serverSimulation, numServers);
    performance_Metrics(numServers, serverSimulation);

    return 0;
}