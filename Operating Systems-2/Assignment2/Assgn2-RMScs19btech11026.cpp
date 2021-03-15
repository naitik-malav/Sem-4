#include <iostream>
#include <fstream>
#include <queue>
#include <utility>
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
	this->deadline = period;
	this->kctr = k;
}

bool ProcessInfo::operator() (PAIR a, PAIR b) {
    return a.second > b.second;
} 

int main(){
	ofstream log, stats;
	input.open("inp-params.txt");
	
	int n, totalProcesses=0, missCTR=0;	//no. of processes; processes came into system; counter for processes whi missed deadline
	input>>n;
	ProcessInfo Process[n+1];					// Array of objects of Class PCB. It is used to provide a mapping between PCB information and process id
	ui timer=0, remainingBurstTime[n+1]={0};

	priority_queue<PAIR, vector<PAIR>, ProcessInfo> activeQueue;		
	priority_queue<PAIR, vector<PAIR>, ProcessInfo> eventQueue;
	
	log.open("RMS-Log.txt");	

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
			log<< ": processing time="<< Process[i+1].execTime;
			log<< "; deadline="<< Process[i+1].deadline;
            log<< "; period="<<Process[i+1].period;
			log<< "; joined the system at time: "<< timer<< endl;
        }
	}

	input.close();
	
	while(!activeQueue.empty() || !eventQueue.empty()){	
	//until both queues are empty
		/* here timer is the arrival time */	
		if(timer == eventQueue.top().second and !eventQueue.empty()){	//setting up eventQueue and activeQueue
			int id = eventQueue.top().first;	//first element of pair is id and process with the higest priority is at top
			
			if(Process[id].k > 0) {
				if(remainingBurstTime[id] != 0){	//time a process will executes or spend in it's period. not more than it is allowed
					Process[id].waitingTime = Process[id].waitingTime + (timer-Process[id].startTime);	//initializing waiting time
					Process[id].startTime = timer;	//setting startime to timer.
					log<<"Process P"<< id;
					log<< " missed deadline at time "<<timer<<endl;	//missed deadline at time = timer
					missCTR++;
				}																			
				else{
						activeQueue.push(PAIR(id, Process[id].period));	//pushing id and period into activeQueue
						Process[id].startTime = timer;	//setting startime to timer
				}
			}
			Process[id].deadline = Process[id].period + timer;
			eventQueue.pop();
			Process[id].k--;

			if(Process[id].k >= 0)		//Pushing another instances of current process if it exists, i.e. k>=0
				eventQueue.push(PAIR(id, Process[id].deadline));	

			remainingBurstTime[id] = Process[id].execTime;		// Reseting  parameters related to the current process
			continue;	
		}
		
		if(!activeQueue.empty()){	//activeQueue is non empty
			int id = activeQueue.top().first;		//this is the process id with least period amongst all process in queue at that time									
			if(Process[id].execTime == remainingBurstTime[id]){		//means process is going to execute
				Process[id].Flag = false;		//setting Flag bool to false
				log<< "Process P"<< id<<" ";
				log<< "starts execution at time "<< timer<< endl;		//execution started at time=timer
				
			}
			else if(Process[id].Flag) {		//Execution resumes
				log<< "Process P"<< id<< " ";
				log<< "resumes execution at time "<< timer<< endl;
				Process[id].Flag = false;
			}

			if(!eventQueue.empty()) {		//if eventQueue is not empty
				//if remainingBurstTime[id]+timer is less than equal to deadline and period
				if(remainingBurstTime[id]+timer <= Process[id].deadline && remainingBurstTime[id]+timer <= eventQueue.top().second) {
					activeQueue.pop();		//popping activeQueue
					Process[id].waitingTime = Process[id].waitingTime + timer-Process[id].startTime;	//calculating waitingTime
					timer = timer + remainingBurstTime[id];		//shifting timer to next position													// Process completely
					remainingBurstTime[id]=0;	//process is executed so it's remainingBurstTime is 0
					log<< "Process P"<< id;
					log<< " finishes execution at time "<< timer<< endl;
					Process[id].deadline = Process[id].period+Process[id].deadline;	//incrementing deadline by 1 period
					
				}

				else if(Process[id].deadline > eventQueue.top().second) {	// means Process is preempted
					remainingBurstTime[id] = timer + (remainingBurstTime[id]- eventQueue.top().second);	//due to preemption process is not executed completely
					timer = eventQueue.top().second;
					if(Process[eventQueue.top().first].period < Process[id].period && Process[eventQueue.top().first].k != 0){
						log<< "Process P"<< id<< " is preempted by P"<< eventQueue.top().first<< " at time "<< timer;
						log<< " Remaining processsing time:"<< remainingBurstTime[id]<< endl;
						Process[id].Flag=true;
					}
					Process[id].waitingTime = timer-(Process[id].startTime+Process[id].execTime)+Process[id].waitingTime+remainingBurstTime[id];
				}

				else {
					Process[id].waitingTime = timer-(Process[id].startTime - Process[id].waitingTime);
					timer=Process[id].deadline;												// Process misses its deadline
					remainingBurstTime[id] = remainingBurstTime[id]-remainingBurstTime[id];
					log<< "Process P"<< id;
					log<< " missed deadline at time="<< timer<< endl;
					missCTR++;
					activeQueue.pop();
				}

			}
			else {		//if eventQueue is empty
				if(remainingBurstTime[id]+timer <= Process[id].deadline) {	//means Process executed completely
					Process[id].waitingTime = timer-(Process[id].startTime-Process[id].waitingTime);
					timer = timer + remainingBurstTime[id];
					remainingBurstTime[id] = 0;
					Process[id].deadline = Process[id].deadline + Process[id].period;

					log<< "Process P"<< id;
					log<< " finishes execution at time "<< timer<< endl;
					eventQueue.pop();	//popping eventQueue
				}

				else {//if process miss it's deadline
					Process[id].waitingTime = timer-(Process[id].startTime-Process[id].waitingTime);
					remainingBurstTime[id] = 0;
					missCTR++;

					log<< "Process P"<< id;
					log<< " missed deadline at time "<< timer<< endl;
					activeQueue.pop();	//popping active queue
					timer = Process[id].deadline;
				}
			}
			
		}

		else{	//activeQueue is empty
			if(!eventQueue.empty()){	//eventQueue is non empty
				if(Process[eventQueue.top().first].k > 0)	//when CPU is idle
					log<< "CPU is idle till time "<< timer<< endl;
				timer = eventQueue.top().second;
				continue;
			}
			else
				break;
		}
	}
	log.close();
	
	stats.open("RM-Stats.txt");
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