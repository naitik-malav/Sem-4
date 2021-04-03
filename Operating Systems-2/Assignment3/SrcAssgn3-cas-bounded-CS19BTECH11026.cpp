#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <unistd.h>
#include <fstream>
#include <random>
#include <vector>
using namespace std;

class Info {
    public:
        int n;
        int k;
        double lambda1;
        double lambda2;
        
        void getParameter();
}input;

atomic<bool>Flag(false);    //atomic bool variable Flag.. which is setted to false
bool *Waiting;	
default_random_engine Generator(time(NULL));    //generates random number
chrono::duration<double> avgWaitingTime(0), maxWaitingTime(0);     //avg. waiting time and max waiting time													
chrono::duration<double> tmp(0); 								
ofstream Log("CAS-Bounded-Log.txt");
ofstream Stats("CAS-Bounded-Stats.txt");
exponential_distribution<double> *distribution1,*distribution2;     //expon distri.
time_t t;

void Info::getParameter() {
    ifstream input;
    input.open("inp-params.txt");
    input>> n>> k>> lambda1>> lambda2;
    input.close();
}

string getSysTime() {
	time(&t);
	struct tm * Time;
	Time = localtime (&t);
	char String[9];
	sprintf(String, "%.2d:%.2d:%.2d", Time->tm_hour, Time->tm_min, Time->tm_sec);
	string str(String);
	return str;
}

void testCS(int id) {
    chrono::time_point<chrono::system_clock> startTime, endTime;
    for(int i=0; i<input.k; i++) {
        string requestTime = getSysTime();
        Waiting[id]=true;
		bool key=true, tempFlag=false;
    	startTime = chrono::system_clock::now();
        
		while(Waiting[id] && key) {
            if(atomic_compare_exchange_strong(&Flag, &tempFlag, true)) 
                key = false;
			else 
                tempFlag = false;
        } 							

    	endTime = chrono::system_clock::now();
        Waiting[id] = false;

		Log<< i<< "th CS requested at "<< requestTime;
        Log<< " by thread "<< id<< endl;
        Log<< i<< "th CS entered at  "<< getSysTime();
        Log<< " by thread "<< id<< endl;
		tmp = endTime-startTime;
    	avgWaitingTime = avgWaitingTime+tmp;								// Calculating waiting time
    	maxWaitingTime = max(tmp, maxWaitingTime);	// Updating max time taken
		
		usleep((*distribution1)(Generator)*1000000);

		int k = (id+1)%input.n;
        while(!Waiting[k] && k!=i) 
            k = (k+1)%input.n;

		Log<< i<< "th CS Exited at   "<< getSysTime();
        Log<< " by thread "<< id<< endl;

		if(k==i)
            Flag =false;
		else 
            Waiting[k]=false;
		usleep((*distribution2)(Generator)*1000000);
	}
}

int main(void) {
    
    input.getParameter();
    
    distribution1 = new exponential_distribution<double>(1/input.lambda1);
	distribution2 = new exponential_distribution<double>(1/input.lambda2);
    Waiting = new bool[input.n];
    
    vector<thread> Thread;  //n threads
	for(int i=0; i<input.n; i++) {
        Thread.push_back(thread(testCS, i));
    } 
	for(int i=0; i<input.n; i++) {
        Thread[i].join();
    } 
	Log.close();

    Stats<< "Average time = "<< avgWaitingTime.count()/(input.n*input.k)<< "s"<< endl; 
    Stats<< "Max waiting time = "<< maxWaitingTime.count()<< "s"<< endl;
    Stats.close();

    delete distribution1;
    delete distribution2;
}