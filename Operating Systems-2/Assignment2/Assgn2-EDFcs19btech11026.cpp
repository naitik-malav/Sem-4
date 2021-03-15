#include <iostream>
#include <fstream>
#include <queue>
#include <iomanip>

using namespace std;

#define ui unsigned int
typedef pair<int, ui> PAIR;
ifstream input;

class ProcessInfo{
    public:
        int pid, k, kctr;		//process id, number of time process gonna execute
        ui period, execTime, deadline;	//period, execution time, next deadline of a process
        double waitingTime = 0;		//total waiting time of a process
        double startTime = 0;	//time when it's execution started
        
        bool Flag=false;
        bool operator() (PAIR, PAIR);	//comparison

        ProcessInfo(){}
        ~ProcessInfo(){}

		void getInput(int, int, ui, ui);	//function for input
};

void ProcessInfo::getInput(int id, int k, ui period, ui execTime) {
	this->pid = id;
	this->k = k;
	this->period = period;
	this->execTime = execTime;
	this->kctr = k;
}

bool ProcessInfo::operator() (PAIR a, PAIR b) {
    return a.second > b.second;
} 

int main(){
	ofstream log, stats;
	input.open("inp-params.txt");
	
	int n, totalProcesses=0, missCTR=0;	//no. of processes; processes came into system; counter for processes whi missed deadline
	input>> n;
	ProcessInfo Process[n+1];					// Array of objects of Class PCB. It is used to provide a mapping between PCB information and process id
	ui timer=0, remainingBurstTime[n+1]={0};

	//priority_queue<PAIR, vector<PAIR>, ProcessInfo> activeQueue;		
	priority_queue<PAIR, vector<PAIR>, ProcessInfo> eventQueue;
	priority_queue<PAIR, vector<PAIR>, ProcessInfo> activeQueue; 
	log.open("EDF-Log.txt");	

	int id;
	for (int i=0; i<n; i++) {	//scanning input from the file inp-params.txt
		int k;
		ui execTime, period;
		input>> id>> execTime>> period>> k;	
		Process[id].getInput(id, k, period, execTime);
	}

	for(int i=0; i<n; i++) {
		if(Process[i+1].k > 0)
            eventQueue.push(PAIR(i+1, 0));	//pushing process id and 0 into eventQueue. It will be sorted according to the id.
	}

	for(int i=0; i<n; i++) {	//printing their joining info into RMS-Log.txt
		totalProcesses = totalProcesses + Process[i+1].k;
		if(Process[i+1].k > 0) {
            log<< "Process P"<< Process[i+1].pid;
			log<< ": execution time="<< Process[i+1].execTime;
			log<< "; deadline="<< Process[i+1].period;
            log<< "; period="<<Process[i+1].period;
			log<< "; joined the system at time: "<< timer<< endl;
        }
	}
	input.close();
	
	while(!activeQueue.empty() || !eventQueue.empty()){	//till both are queue empty, or till all process terminates
		
		if(!eventQueue.empty() && timer == eventQueue.top().second){	//setting up eventQueue and activeQueue
			id = eventQueue.top().first;
			Process[id].deadline = Process[id].period + timer;

				if(remainingBurstTime[id] != 0) {	
					missCTR++;					// process is at the top of the ready priority queue
					log<< "Process P"<< id;
					log<< " missed deadline at time "<< timer<< endl;
					
					Process[id].waitingTime = Process[id].waitingTime + timer - Process[id].startTime;
					Process[id].startTime = timer;
					activeQueue.pop();
					activeQueue.push(PAIR(id, Process[id].deadline));			// process is not present in ready queue	
				}

				else {
					Process[id].startTime = timer;
					activeQueue.push(PAIR(id, Process[id].deadline));
				}
			remainingBurstTime[id] = Process[id].execTime;		// Reseting  parameters related to the current process 
			eventQueue.pop();
			Process[id].k--;

			if(Process[id].k > 0)
				eventQueue.push(PAIR(id, Process[id].deadline));		// Push another instances of current process if it exists
			continue;
		}
		
		if(!activeQueue.empty()){	//activeQueue is non empty
			id = activeQueue.top().first;
			if(Process[id].execTime == remainingBurstTime[id]) {		//execution start
				log<< "Process P"<< id;
				log<< " starts execution at time "<< timer<< endl;
				Process[id].Flag = false;
			}
			else if(Process[id].Flag) {		//execution resumes
				log<< "Process P"<< id;
				log<< " resumes execution at time "<< timer<< endl;
				Process[id].Flag=false;
			}											
			if(eventQueue.empty()) {		//if eventQueue is empty											
				if(remainingBurstTime[id]+timer > Process[id].deadline) {	//successful completion
					missCTR++;
					remainingBurstTime[id] = 0;
					Process[id].waitingTime = Process[id].waitingTime + timer - Process[id].startTime;
					timer = Process[id].deadline;
					Process[id].deadline = Process[id].deadline + Process[id].period;
					activeQueue.pop();

					log<< "Process P"<< id;
					log<< " missed deadline at time "<< timer<< endl;	//misses deadline
				}

				else {
					Process[id].waitingTime = Process[id].waitingTime + timer - Process[id].startTime;
					timer = remainingBurstTime[id] + timer;
					Process[id].deadline = Process[id].deadline + Process[id].period;
					
					log<< "Process P"<< id;
					log<< " finishes execution at time "<< timer<< endl;	//finished execution
					
					remainingBurstTime[id] = 0;
					activeQueue.pop();
				}
			}
			else{
				if(remainingBurstTime[id]+timer <= eventQueue.top().second && remainingBurstTime[id]+timer <= Process[id].deadline) {
					Process[id].waitingTime = Process[id].waitingTime + timer - Process[id].startTime;
					timer = timer + remainingBurstTime[id];														
					remainingBurstTime[id] = 0;
					log<< "Process P"<< id;
					log<< " finishes execution at time "<< timer<< endl;	//execution finished
					Process[id].deadline = Process[id].deadline + Process[id].period;
					activeQueue.pop();
				}																								
				else if(eventQueue.top().second < Process[id].deadline) {	// else if process is preempted
					Process[id].waitingTime = Process[id].waitingTime+timer-Process[id].startTime-Process[id].execTime+remainingBurstTime[id];
					remainingBurstTime[id] = remainingBurstTime[id]-eventQueue.top().second+timer;
					timer = eventQueue.top().second;
					if(Process[eventQueue.top().first].deadline < Process[id].deadline) {
						log<< "Process P"<< id;
						log<< " is preempted by P"<< eventQueue.top().first;
						log<< " at time "<< timer<< endl;
						Process[id].Flag=true;
					}
				}
				else {
					Process[id].waitingTime = timer-(Process[id].startTime-Process[id].waitingTime);
					timer = Process[id].deadline;																
					log<< "Process P"<< id;
					log<< " missed deadline at time "<< timer<< endl;	//process misses its deadline
					missCTR++;
					Process[id].deadline = Process[id].deadline + Process[id].period;
					activeQueue.pop();
					remainingBurstTime[id] = remainingBurstTime[id] - remainingBurstTime[id];
				}
			}
		}
	
		else {		//activeQueue is empty
			if(!eventQueue.empty()){	//if eventQueue is non empty
				timer = eventQueue.top().second;
				log<< "CPU is idle till time "<< timer<< endl;
				continue;
			}
			else 
				break;
		}
	}
	log.close();
	
	stats.open("EDF-Stats.txt");
	stats<< "Number of Processes that came into the system:"<< totalProcesses<< endl;
	stats<< "Number of Processes that successfully completed:"<< totalProcesses-missCTR<< endl;
	stats<< "Number of Process who missed deadline:"<< missCTR<< endl;
	
	double TotalWaitingTime = 0;
	for (int i = 0; i < n; ++i)
	{
		/*
		Average Waiting time of a process = (Sum of waiting time of all the rounds) / (the number of times process repeats)
		Average Waiting time of all the process = (Sum of Average Waiting time of all the Processes)/(Total number of Processes)
		*/
		TotalWaitingTime = TotalWaitingTime + (double)(Process[i+1].waitingTime)/(Process[i+1].kctr) ;
	}
	stats<< "Average waiting time of all processes = "<< setprecision(4)<< TotalWaitingTime/n<<"\n";
	stats.close();
}