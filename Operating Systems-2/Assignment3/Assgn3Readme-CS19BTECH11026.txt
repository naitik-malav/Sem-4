1. First go to directory where you have download/saved my source code.
2. Now compile and run using below instructions:

    For TAS:
        1. To compile:
            g++ SrcAssgn3-tas-CS19BTECH11026.cpp -pthread -o ./tas

        2. To run:
            ./tas

    For CAS:
        1. To compile:
            g++ SrcAssgn3-cas-CS19BTECH11026.cpp -pthread -o ./cas
            
        2. To run:
            ./cas
    
    For CAS-Bounded:
        1. To compile:
            g++ SrcAssgn3-cas-bounded-CS19BTECH11026.cpp -pthread -o ./casb
            
        2. To run:
            ./casb

Note: if -pthread won't work try -lpthread
After running these programs generates respective log file and stats file. Check it out.