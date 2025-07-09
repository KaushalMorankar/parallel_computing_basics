#include <bits/stdc++.h>
#include <pthread.h>
#include <atomic>
#include <chrono>

using namespace std;
using namespace chrono;

vector<vector<int>> sudoku;
atomic<int> C(0);
atomic<bool> isvalid(true); //atomic to prevent race conditions
int N, K, taskInc;

ofstream outFile("output.txt");

vector<long long> csentry; 
vector<long long> csexit;

// TAS Locks
atomic<bool> entry_exit_lock(false);   // For protecting stats updates
atomic<bool> logfilelock(false); // For protecting log writes
atomic<bool> lockVar(false); // for counter

void lock(atomic<bool> &lockVar) {
    bool expected=false;
    while (!lockVar.compare_exchange_weak(expected, true)) {
        expected=false; // Reset expected for next iteration
    }
}

void unlock(atomic<bool> &lockVar) {
    lockVar.store(false);
}

long long currentTimeMicro(){
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

string formattime() {
    auto now=system_clock::now();

    time_t now_time=system_clock::to_time_t(now);
    tm* localTime=localtime(&now_time);

    auto duration=now.time_since_epoch();
    auto microsecondsPart=duration_cast<microseconds>(duration).count() % 1000000;  // Extract microseconds

    stringstream ss;
    ss<<setfill('0')<<setw(2)<<localTime->tm_hour<<":"<< setfill('0')<<setw(2)<<localTime->tm_min<<"."<< setfill('0')<<setw(6)<<microsecondsPart<<" hrs";

    return ss.str();
}

//Critical section function
void increment(atomic<int>& v, int inc, int& task, int thread_index) {
    long long reqtime = currentTimeMicro();

    // Logging the request to enter CS
    lock(logfilelock);
    outFile << "Thread " << thread_index << " requests to enter CS at " << formattime() << endl;
    unlock(logfilelock);

    // Actual CS starts here
    lock(lockVar);  // Ensure only one thread enters at a time

    long long entrytime = currentTimeMicro();

    // Update CS entry time stats
    lock(entry_exit_lock);
    csentry.push_back(entrytime - reqtime);
    unlock(entry_exit_lock);

    // Logging entry into CS
    lock(logfilelock);
    outFile << "Thread " << thread_index << " enters CS at " << formattime() << endl;
    unlock(logfilelock);

    // Atomic increment using CAS
    int temp;
    do {
        temp = v.load();
    } while (!v.compare_exchange_weak(temp, temp + inc));

    task = temp; // Assign the task value after CAS succeeds

    long long csexittime = currentTimeMicro();

    // Logging exit from CS
    lock(logfilelock);
    outFile << "Thread " << thread_index << " exits CS at " << formattime() << endl;
    unlock(logfilelock);

    unlock(lockVar); // Release lock after updating the value

    // Update CS exit time stats
    lock(entry_exit_lock);
    csexit.push_back(csexittime - entrytime);
    unlock(entry_exit_lock);
}

bool rowcheck(int row){
    vector<bool> vis(N+1, false);
    for(int j=0; j<N; j++){
        int num=sudoku[row][j];
        if (num<1) { 
            return false;
        }
        if (num>N) { 
            return false;
        }
        if (vis[num]) { 
            return false;
        }
        vis[num]=true;
    }
    return true;
}

bool colcheck(int col){
    vector<bool> vis(N+1, false);
    for(int i=0; i<N; i++){
        int num=sudoku[i][col];
        if (num<1) { 
            return false;
        }
        if (num>N) { 
            return false;
        }
        if (vis[num]) { 
            return false;
        }
        vis[num]=true;
    }
    return true;
}

bool gridcheck(int sr, int sc, int gridsize){
    vector<bool> vis(N+1, false);
    for(int i=0; i<gridsize; i++){
        for(int j=0; j<gridsize; j++){
            int num=sudoku[sr+i][sc+j];
            if (num<1) { 
                return false;
            }
            if (num>N) { 
                return false;
            }
            if (vis[num]) { 
                return false;
            }
            vis[num]=true;
        }
    }
    return true;
}

// Thread function
// Dynamic allocation of rows then columns and then grids according to taskInc and thread calling it
void* validsudoku(void* arg){
    int thread_index=*(int*)arg;
    delete(int*)arg;
    int gridsize=sqrt(N);

    while(true){
        if(!isvalid){
            pthread_exit((void*)0);
        }
        int task;
        increment(C, taskInc, task, thread_index);

        if(task>=3*N) break;

        for(int i=0; i<taskInc; i++){
            int curr=task+i;
            if(curr>=3*N) break;

            bool valid=true;

            if(curr<N){
                lock(logfilelock);
                outFile<<"Thread "<<thread_index<<" grabs row "<<curr<<" at "<<formattime()<<endl;
                unlock(logfilelock);

                valid=rowcheck(curr);

                lock(logfilelock);
                if(valid)
                    outFile<<"Thread "<<thread_index<<" completes checking of row "<<curr<<" and found it valid at "<<formattime()<< endl;
                else outFile<<"Thread "<<thread_index<<" completes checking of row "<<curr<<" and found it invalid at "<<formattime()<< endl;
                unlock(logfilelock);
            } 
            else if(curr<2*N){
                lock(logfilelock);
                outFile<<"Thread "<<thread_index<<" grabs column "<<curr-N<<" at "<<formattime()<<endl;
                unlock(logfilelock);

                valid=colcheck(curr-N);

                lock(logfilelock);
                if(valid)
                    outFile<<"Thread "<<thread_index<<" completes checking of column "<<curr-N<<" and found it valid at "<<formattime()<< endl;
                else outFile<<"Thread "<<thread_index<<" completes checking of column "<<curr-N<<" and found it invalid at "<<formattime()<< endl;
                unlock(logfilelock);
            }
            else{
                int grid_index=curr-2*N;
                int row=(grid_index/gridsize)*gridsize;
                int col=(grid_index%gridsize)*gridsize;

                lock(logfilelock);
                outFile<<"Thread "<<thread_index<<" grabs grid "<<grid_index<<" at "<<formattime()<<endl;
                unlock(logfilelock);

                valid=gridcheck(row, col, gridsize);

                lock(logfilelock);
                if(valid)
                    outFile<<"Thread "<<thread_index<<" completes checking of grid "<<curr-2*N<<" and found it valid at "<<formattime()<< endl;
                else outFile<<"Thread "<<thread_index<<" completes checking of grid "<<curr-2*N<<" and found it invalid at "<<formattime()<< endl;
                unlock(logfilelock);
            }

            if(!valid){
                isvalid=false;
                pthread_exit((void*)1);
            }
        }
    }

    pthread_exit((void*)0);
}

int main(){
    ifstream file("input.txt");
    file >> K >> N >> taskInc;
    sudoku.resize(N, vector<int>(N));
    for(int i=0; i<N; i++)
        for(int j=0; j<N; j++)
            file >> sudoku[i][j];

    vector<pthread_t> threads(K);

    auto overallStart=high_resolution_clock::now();

    for(int i=0; i<K; i++){
        int* thread_index=new int(i);
        pthread_create(&threads[i], NULL, validsudoku,(void*)thread_index);
    }

    bool invalidfound = false;
    for(int i = 0; i<K; i++){
        void* retVal;
        pthread_join(threads[i], &retVal);
        if(retVal == (void*)1){
            invalidfound = true;
        }
    }


    auto overallend=high_resolution_clock::now();
    double totalTime=duration_cast<microseconds>(overallend - overallStart).count();
    
    // Compute CS entry and exit averages and worst-case 
    long long sumentry=0, worstentry=0;
    long long sumexit=0, worstexit=0;
        
    lock(entry_exit_lock);
    for(auto t : csentry){
        sumentry += t;
        worstentry=max(worstentry, t);
    }
    for(auto t : csexit){
        sumexit += t;
        worstexit=max(worstexit, t);
    }
    unlock(entry_exit_lock);
    
    double avgentry =(csentry.empty() ? 0.0 :(double)sumentry / csentry.size());
    double avgexit  =(csexit.empty() ? 0.0 :(double)sumexit / csexit.size());
    
    // Write summary to output file
    lock(logfilelock);
    outFile << "Final outcome: Sudoku is " <<(isvalid ? "valid" : "invalid") << "." << endl;
    outFile << "Total time taken: " << totalTime << " microseconds." << endl;
    outFile << "Average time to enter CS: " << avgentry << " microseconds." << endl;
    outFile << "Average time to exit CS: " << avgexit << " microseconds." << endl;
    outFile << "Worst-case time to enter CS: " << worstentry << " microseconds." << endl;
    outFile << "Worst-case time to exit CS: " << worstexit << " microseconds." << endl;
    unlock(logfilelock);
    
    outFile.close();
    
    return 0;
}
