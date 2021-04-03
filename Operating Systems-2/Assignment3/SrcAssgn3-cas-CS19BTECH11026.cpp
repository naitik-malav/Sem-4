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

atomic<bool>Flag(false);    //atomic variable bool Flag, intially false
default_random_engine Generator(time(NULL));        //generates random number
chrono::duration<double> avgWaitingTime(0), maxWaitingTime ;    //avg. waiting time and max. waiting time
chrono::duration<double> tmp(0); 								
ofstream Log("CAS-Log.txt");
ofstream Stats("CAS-Stats.txt");
exponential_distribution<double> *distribution1,*distribution2;     //exponential distri.
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
    for(int i=0; i<input.k; i++) {

		string requestTime = getSysTime();
    	startTime = chrono::system_clock::now();
        bool tempFlag = false;		

		while(!atomic_compare_exchange_strong(&Flag, &tempFlag, true)) {
            tempFlag = false;
        } 							

    	endTime = chrono::system_clock::now();

		Log<< i<< "th CS requested at "<< requestTime<< " by thread "<< id<< endl;
		tmp = (endTime-startTime);
    	avgWaitingTime = avgWaitingTime+tmp;								// Calculating waiting time
    	maxWaitingTime = max(tmp, maxWaitingTime);	// Updating max time taken
		Log<< i<< "th CS entered at  "<< getSysTime()<< " by thread "<< id<< endl;
		usleep((*distribution1)(Generator)*1000000);
		Log<< i<< "th CS exited at   "<< getSysTime()<< " by thread "<< id<< endl;
		Flag = false;
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

	ofstream Stats;
    Stats.open("CAS-Stats.txt");	
    Stats<< "Average time = "<< avgWaitingTime.count()/(input.n*input.k)<< "s"<< endl; 
    Stats<< "Max waiting time = "<< maxWaitingTime.count()<< "s"<< endl;

    delete distribution1;
    delete distribution2;
    Stats.close();
	return 0;
}