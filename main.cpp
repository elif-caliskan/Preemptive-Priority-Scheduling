#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <vector>
#include <queue>
using namespace std;
struct Process {
    int priority;
    string processName;
    string processCodeName;
    int arrivalTime;
    int turnaround;
    int burstTime;
    vector<int> instructionTime;
    int processedIndex;
    int indexInArray;
    Process(){};
    //constructor
    Process(string processName_, int priority_,  string processCodeName_, int arrivalTime_){
        priority=priority_;
        processCodeName=processCodeName_;
        processName=processName_;
        arrivalTime=arrivalTime_;
        instructionTime = *new vector<int>;
        burstTime=0;
        indexInArray=0;
        processedIndex=0;
        turnaround=0;
    }
    //assignment operator
    Process& operator =(const Process& a)
    {
        priority = a.priority;
        processCodeName= a.processCodeName;
        processName=a.processName;
        arrivalTime=a.arrivalTime;
        instructionTime=a.instructionTime;
        turnaround=a.turnaround;
        burstTime=a.burstTime;
        processedIndex=a.processedIndex;
        indexInArray=a.indexInArray;
        return *this;
    }
};
//this struct is used for comparing priority in readyqueue
struct ComparePriority {
   bool operator()(Process const  *c1, Process const  *c2) {
       if(c1->priority==c2->priority)
           return c1->arrivalTime>c2->arrivalTime; //emin değilim yalnız
        return c1->priority>c2->priority;
    }
};
//this struct is used for comparing arrival time and finding the first process in readyqueue
struct earlier
{
    inline bool operator() (const Process & process1, const Process & process2)
    {
        if(process1.arrivalTime==process2.arrivalTime){
            return  process1.priority<process2.priority;
        }
        return (process1.arrivalTime < process2.arrivalTime);
    }
};
//this struct is used for comparing the indexes of the initial array in order to go back to first configuration of processes
struct initial
{
    inline bool operator() (const Process & process1, const Process & process2)
    {

        return (process1.indexInArray < process2.indexInArray);
    }
};

//this method is used for splitting a string into words
template <class Container>
void split1(const string& str, Container& cont)
{
    istringstream iss(str);
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter(cont));
}
//this method takes a priorty_queue, an integer and ofstream to write to a file.
//first, I create a vector and I put every element in queue to vector for writing to file.
//then I put these elements back to priority_queue
void writeQueue(priority_queue<Process*,vector<Process*>,ComparePriority>& queue,int time,ofstream* write) {
    *write << time << ":HEAD-";
    vector<Process *> x;
    if (queue.empty()) {
        *write << "-TAIL"<<endl;
    } else {
        while (!queue.empty()) {
            Process *a = queue.top();
            queue.pop();
            x.push_back(a);
            *write << a->processName + "[" << (a->processedIndex + 1) << "]-";
        }
        *write << "TAIL" << endl;
        for (int i = 0; i < x.size(); i++) {
            queue.push(x[i]);
        }
    }

}
int main(int argc, char* argv[]) {
    vector<Process> processes;
    int processCount=0;
    ifstream read;
    ofstream write;
    read.open("./definition.txt");
    write.open("./output.txt");
    if(read.is_open()) {
        string line;
        vector<string> input;
        //reads the definition file and creates processes according to the given information
        //created processes are added to vector
        while (!read.eof()) {
            getline(read, line);
            vector<string> words;
            split1(line, words);
            Process process(words[0], atoi(words[1].c_str()), words[2], atoi(words[3].c_str()));
            process.indexInArray=processCount;
            processes.push_back(process);
            processCount++;
        }
    }
    read.close();
    for(int i=0;i<processCount;i++){
        read.open(("./"+processes[i].processCodeName+".txt").c_str());
        if(read.is_open()) {
            string line;
            int sum=0;
            //reads code files and adds these instructions to relevant process.
            while (!read.eof()) {
                getline(read, line);
                if(line.find("\t")!= string::npos) {
                    line = line.substr(line.find("\t") + 1);
                }
                else{
                    line= line.substr(line.find_last_of(" ")+1);
                }
                processes[i].instructionTime.push_back(atoi(line.c_str()));
                sum+=atoi(line.c_str());
            }
            //calculates the burst time by adding given instructions
            processes[i].burstTime=sum;
        }
        read.close();
    }
    priority_queue<Process*,vector<Process*>,ComparePriority> readyQueue;
    //this sort makes sure the processes are aligned according to their arrival time
    sort(processes.begin(), processes.end(), earlier());
    //if there are no processes that come at time 0, readyQueue is empty
    if(processes[0].arrivalTime!=0){
        write<<"0:HEAD--TAIL"<<endl;
    }
    //this integer is used for keeping track of the next process
    int earliestProcessIndex=1;
    readyQueue.push(&processes[0]);
    int currentTime=processes[0].arrivalTime;
    bool goOn=true;
    //if there is only one process that comes first, it writes the queue
    if(processes[earliestProcessIndex].arrivalTime!=currentTime||earliestProcessIndex>=processes.size()){
        writeQueue(readyQueue, currentTime,&write);
    }
    while(goOn) {
        //if queue is empty, it goes to the next process, current time is increased and earliestprocessIndex is incremented
        if(readyQueue.empty()){
            currentTime=max(processes[earliestProcessIndex].arrivalTime,currentTime);
            readyQueue.push(&processes[earliestProcessIndex]);
            earliestProcessIndex++;
            //if there is not more than one process that comes at current time, it writes the queue
            if(processes[earliestProcessIndex].arrivalTime>currentTime||earliestProcessIndex>=processes.size()){
                writeQueue(readyQueue, currentTime,&write);
            }
        }
        //if the queue is not empty, we take the top element of the queue as current
        Process *current = readyQueue.top();
        readyQueue.pop();
        //if current time is equal to or greater than the next arrival time, first we push the current process to queue,
        //then we push the next process(es) to queue and increment next index. After every prosess is added, it writes the queue
        if (earliestProcessIndex < processes.size() && currentTime >= processes[earliestProcessIndex].arrivalTime) {
            readyQueue.push(current);
           while(earliestProcessIndex < processes.size() && currentTime >= processes[earliestProcessIndex].arrivalTime) {
               readyQueue.push(&processes[earliestProcessIndex]);
               earliestProcessIndex++;
           }
            writeQueue(readyQueue, currentTime,&write);
        }
        else {
            //while current time is less than the arrival of next process or there is no processes left,
            //it increases current time by one instruction
            while ((currentTime < processes[earliestProcessIndex].arrivalTime||earliestProcessIndex>=processes.size())
            && (current->instructionTime.size() > current->processedIndex)) {
                currentTime += current->instructionTime[current->processedIndex];
                current->processedIndex++;
            }
            //after the while loop either another process has arrived or the process is finished.
            //if another process is coming, it pushes the current process to the queue
            if(current->instructionTime.size() > current->processedIndex)
                readyQueue.push(current);
            //if the process is finished, turnaround time is calculated and if there is not a coming process, it writes the queue
            else {
                current->turnaround = currentTime - current->arrivalTime;
                if(currentTime<processes[earliestProcessIndex].arrivalTime||earliestProcessIndex>=processes.size()) {
                    writeQueue(readyQueue, currentTime,&write);
                }
            }
        }
        goOn=false;
        //it checks if every process has its maximum processedIndex
        for(int i=0;i<processCount;i++){
            if(processes[i].processedIndex!=processes[i].instructionTime.size()){
                goOn=true;
                break;
            }
        }
    }
    write<<endl;
    //it sorts the vector according to indexInArray to find the initial vector
    sort(processes.begin(), processes.end(), initial());
    //it writes turnaround and waiting times of every process
    for(int i=0;i<processes.size();i++){
        write<<"Turnaround time for "<<processes[i].processName<<" = "<<processes[i].turnaround<<" ms"<<endl;
        write<<"Waiting time for "<<processes[i].processName<<" = "<<(processes[i].turnaround-processes[i].burstTime)<<endl;
    }
    write.close();
    return 0;
}

	
	
	
