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

bool test_and_set(atomic<bool> *target) {
    return target->exchange(true);
}

void unlock(atomic<bool> *target) {
    target->store(false);
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

// bool test_and_set(bool *target) {
//     bool rv=*target;
//     *target=true;
//     return rv;
// }

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

        long long reqtime=currentTimeMicro(); // Time before requesting CS
        
        while (test_and_set(&lockVar)) 
            ; // Spin until lock is free

        // Log request **after acquiring the lock** to avoid printing uninitialized task values
        while (test_and_set(&logfilelock));
        outFile<<"Thread "<<thread_index<<" requests to enter CS"<<" at "<<formattime()<<endl;
        unlock(&logfilelock);

        long long entrytime=currentTimeMicro(); // Time after acquiring CS lock
        while (test_and_set(&logfilelock));
        outFile<<"Thread "<<thread_index<<" enters CS"<<" at "<<formattime()<<endl;
        unlock(&logfilelock);
        // **Critical Section**
        task=C;  
        C += taskInc;

        long long csexittime=currentTimeMicro(); // Time after updating C
        
        // Log exit
        while (test_and_set(&logfilelock));
        outFile<<"Thread "<<thread_index<<" exits CS"<<" at "<<formattime()<<endl;
        unlock(&logfilelock);
        
        unlock(&lockVar); // Unlock CS

        // Save CS timing data
        while(test_and_set(&entry_exit_lock));
        csentry.push_back(entrytime - reqtime);
        csexit.push_back(csexittime - entrytime);
        unlock(&entry_exit_lock);

        if(task>=3*N) break;

        for(int i=0; i<taskInc; i++){
            int curr=task+i;
            if(curr>=3*N) break;

            bool valid=true;

            if(curr<N){
                while (test_and_set(&logfilelock));
                outFile<<"Thread "<<thread_index<<" grabs row "<<curr<<" at "<<formattime()<<endl;
                unlock(&logfilelock);

                valid=rowcheck(curr);

                while (test_and_set(&logfilelock));
                if(valid)
                    outFile<<"Thread "<<thread_index<<" completes checking of row "<<curr<<" and found it valid at "<<formattime()<< endl;
                else outFile<<"Thread "<<thread_index<<" completes checking of row "<<curr<<" and found it invalid at "<<formattime()<< endl;
                unlock(&logfilelock);
            } 
            else if(curr<2*N){
                while (test_and_set(&logfilelock));
                outFile<<"Thread "<<thread_index<<" grabs column "<<curr-N<<" at "<<formattime()<<endl;
                unlock(&logfilelock);

                valid=colcheck(curr-N);

                while (test_and_set(&logfilelock));
                if(valid)
                    outFile<<"Thread "<<thread_index<<" completes checking of column "<<curr-N<<" and found it valid at "<<formattime()<< endl;
                else outFile<<"Thread "<<thread_index<<" completes checking of column "<<curr-N<<" and found it invalid at "<<formattime()<< endl;
                unlock(&logfilelock);
            }
            else{
                int grid_index=curr-2*N;
                int row=(grid_index/gridsize)*gridsize;
                int col=(grid_index%gridsize)*gridsize;

                while (test_and_set(&logfilelock));
                outFile<<"Thread "<<thread_index<<" grabs grid "<<grid_index<<" at "<<formattime()<<endl;
                unlock(&logfilelock);

                valid=gridcheck(row, col, gridsize);

                while (test_and_set(&logfilelock));
                if(valid)
                    outFile<<"Thread "<<thread_index<<" completes checking of grid "<<curr-2*N<<" and found it valid at "<<formattime()<< endl;
                else outFile<<"Thread "<<thread_index<<" completes checking of grid "<<curr-2*N<<" and found it invalid at "<<formattime()<< endl;
                unlock(&logfilelock);
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

    auto overallstart=high_resolution_clock::now();

    for(int i=0; i<K; i++){
        int* thread_index=new int(i);
        pthread_create(&threads[i], NULL, validsudoku,(void*)thread_index);
    }

    if(!isvalid){
        for(int i=0; i<K; i++){
            int* thread_index=new int(i);
            pthread_cancel(threads[i]);
        }
    }

    bool invalidfound=false;
    for(int i=0; i < K; i++){
        void* retvalue;
        pthread_join(threads[i], &retvalue);
        if(retvalue == (void*)1){
            invalidfound=true;
        }
    }


    auto overallend=high_resolution_clock::now();
    double totaltime=duration_cast<microseconds>(overallend - overallstart).count();
    
    // Compute CS entry and exit averages and worst-case 
    long long sumentry=0, worstentry=0;
    long long sumexit=0, worstexit=0;
        
    while(test_and_set(&entry_exit_lock));
    for(auto t : csentry){
        sumentry += t;
        worstentry=max(worstentry, t);
    }
    for(auto t : csexit){
        sumexit += t;
        worstexit=max(worstexit, t);
    }
    unlock(&entry_exit_lock);
    
    double avgentry =(csentry.empty() ? 0.0 :(double)sumentry / csentry.size());
    double avgexit  =(csexit.empty() ? 0.0 :(double)sumexit / csexit.size());
    
    // Write summary to output file
    while (test_and_set(&logfilelock));
    outFile<<"Final outcome: Sudoku is " <<(isvalid ? "valid" : "invalid")<<"."<<endl;
    outFile<<"Total time taken: "<<totaltime<<" microseconds."<<endl;
    outFile<<"Average time to enter CS: "<<avgentry<<" microseconds."<<endl;
    outFile<<"Average time to exit CS: "<<avgexit<<" microseconds."<<endl;
    outFile<<"Worst-case time to enter CS: "<<worstentry<<" microseconds."<<endl;
    outFile<<"Worst-case time to exit CS: "<<worstexit<<" microseconds."<<endl;
    unlock(&logfilelock);
    
    outFile.close();
    
    return 0;
}
