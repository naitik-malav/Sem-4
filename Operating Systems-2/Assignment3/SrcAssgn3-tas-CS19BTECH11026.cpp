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

atomic_flag Flag=ATOMIC_FLAG_INIT;	//atomic variable flag set to false
default_random_engine Generator(time(NULL));	//gemerates random number
exponential_distribution<double> *distribution1,*distribution2; // exp distri
chrono::duration<double> avgWaitingTime(0), maxWaitingTime(0); 	// avg. and max waiting time
chrono::duration<double> tmp(0); 								
ofstream Log("TAS-Log.txt");
ofstream Stats("TAS-Stats.txt");
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
    chrono::time_point<chrono::system_clock> startTime,endTime;
    for(int i=0; i<input.k; i++){
        
		string requestTime = getSysTime();
    	startTime = chrono::system_clock::now();			
		while(atomic_flag_test_and_set(&Flag));									// Test and Set implementation

    	endTime = chrono::system_clock::now();

		Log<< i<< "th CS requested at "<< requestTime<< " by thread "<< id<< endl;
		tmp = (endTime-startTime);
    	avgWaitingTime = avgWaitingTime+tmp;								// Calculating waiting time
    	maxWaitingTime = max(tmp, maxWaitingTime);	// Updating max time taken
		Log<< i<< "th CS entered at  "<< getSysTime()<< " by thread "<< id<< endl;
		usleep((*distribution1)(Generator)*1000000);
		Log<< i<< "th CS exited at   "<< getSysTime()<< " by thread "<< id<< endl;
		Flag.clear();
		usleep((*distribution2)(Generator)*1000000);
	}
}

int main(void) {
    
    input.getParameter();
    
    distribution1 = new exponential_distribution<double>(1/input.lambda1);
	distribution2 = new exponential_distribution<double>(1/input.lambda2);

    vector<thread> Thread;														// Array of n threads
	for(int i=0; i<input.n; i++) {
        Thread.push_back(thread(testCS, i+1));
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