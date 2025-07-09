#include <bits/stdc++.h>
#include <pthread.h>
#include <atomic>
#include <chrono>

using namespace std;
using namespace chrono;

queue<int> q;
int threads, vertices;
vector<int> vis;
vector<vector<int>> adjlist;
pthread_mutex_t mutex1;
vector<vector<int>> store_arr;
atomic<int> ct{0};

ofstream otpfile("otp.txt");

long long currentTimeMicro()
{
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

string formattime()
{
    auto now = system_clock::now();

    time_t now_time = system_clock::to_time_t(now);
    tm *localTime = localtime(&now_time);

    auto duration = now.time_since_epoch();
    auto microsecondsPart = duration_cast<microseconds>(duration).count() % 1000000; // Extract microseconds

    stringstream ss;
    ss << setfill('0') << setw(2) << localTime->tm_hour << ":" << setfill('0') << setw(2) << localTime->tm_min << "." << setfill('0') << setw(6) << microsecondsPart << " hrs";

    return ss.str();
}


void *start(void *arg)
{
    int thread_index = *(int *)arg;
    delete (int *)arg;
    while (!q.empty())
    {
        // cout<<"hi "<<thread_index<<endl;
        pthread_mutex_lock(&mutex1);
        int hello = q.front();
        q.pop();
        // cout<<"Vertex "<<hello<<" discovered by thread "<<thread_index + 1<<endl;
        otpfile << "Vertex " << hello << " discovered by thread " << thread_index + 1 << endl;
        // cout<<ct<<endl;
        store_arr[ct].push_back(hello);
        // cout<<store_arr[ct][0]<<endl;
        for (int adj : adjlist[hello])
        {
            if (!vis[adj])
            {
                pthread_mutex_lock(&mutex1);
                q.push(adj);
                vis[adj] = 1;
                store_arr[ct].push_back(adj);
                pthread_mutex_unlock(&mutex1);
            }
        }
        pthread_mutex_lock(&mutex1);
        ct++;
        pthread_mutex_unlock(&mutex1);
        // usleep(100);
        pthread_mutex_unlock(&mutex1);
    }

    return nullptr;
}

int main()
{
    ifstream file("./inp.txt");
    if (!file)
    {
        cerr << "Error opening input file!" << endl;
        return 1;
    }

    file >> threads >> vertices;

    adjlist.resize(vertices + 1);
    vis.resize(vertices + 1);

    string hi = "";
    while (getline(file, hi))
    {
        string z = "";
        vector<int> store;
        for (int i = 0; i < hi.size(); ++i)
        {
            if (hi[i] == ' ')
            {
                store.push_back(stoi(z));
                z = "";
            }
            else
                z += hi[i];
        }
        if (!z.empty())
            store.push_back(stoi(z));

        for (int i = 1; i < store.size(); ++i)
            adjlist[store[0]].push_back(store[i]);
    }
    ct=0;

    file.close();
    int ct = 0;
    for (int i = 1; i < adjlist.size(); i++)
    {
        if (adjlist[i].size() > 0)
        {
            ct = i;
            break;
        }
    }
    q.push(ct);
    vis[ct] = 1;
    vector<pthread_t> threads_arr(threads);

    auto overallstart = high_resolution_clock::now();

    for (int i = 0; i < threads; ++i)
    {
        int *thread_index = new int(i);
        pthread_create(&threads_arr[i], NULL, start, (void *)thread_index);
    }

    for (int i = 0; i < threads; ++i)
        pthread_join(threads_arr[i], NULL);

    auto overallend = high_resolution_clock::now();
    double totalTime = duration_cast<microseconds>(overallend - overallstart).count();
    otpfile << endl;
    //     for(int i=0;i<store_arr.size();i++)
    //    {
    //         for(int j=0;j<store_arr[i].size();j++)
    //        {
    //             otpfile<<store_arr[i][j]<<" ";
    //         }
    //         otpfile<<endl;
    //     }
    //     otpfile<<endl;
    // cout<<ct<<endl;
    otpfile << "total time is: " << totalTime << " microseconds" << endl;

    return 0;
}