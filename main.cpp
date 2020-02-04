#include <bits/stdc++.h>
#include <semaphore.h>
#include <pthread.h>
#include<unistd.h>
#define monitor_sleep 1
#define collector_sleep 6
#define buffer_size 2

using namespace std;
int cnt, N,free_pos;
vector<pthread_t> threads;
pthread_t monitor_thread, collector_thread;
sem_t empty_spaces, full_spaces, use;
vector<int> buffer;

struct mcounter_data{
    int ind;
    sem_t* semaphore;
    mcounter_data(int ind, sem_t* sem){
        this->ind = ind;
        semaphore = sem;
    }
};

void init_sems(){
    sem_init(&empty_spaces, 1, buffer_size);
    sem_init(&full_spaces, 1, 0);
    sem_init(&use, 1, 1);
}

class mcounter{
    public :
        mcounter(sem_t* sem,int ind){
            mySem = sem;
            this->ind = ind;
        }
        void run(){
            printf("Counter thread %d : waiting to write\n",ind);
            sem_wait(mySem);
            cnt++;
            printf("Counter thread %d : now adding to the counter, counter value = %d\n",ind,cnt);
            sem_post(mySem);
        }
    private :
        sem_t* mySem;
        int ind;
};

class mmonitor{
    public:
        mmonitor(sem_t* semaphore){
            this->semaphore = semaphore;
            pos = 0;
        }
        void run(){
            while(1){
                sleep(monitor_sleep);
                sem_wait(semaphore);
                printf("Monitor thread : reading a count value of %d\n",cnt);
                if(!free_pos) printf("Monitor thread : buffer full!\n");
                sem_wait(&empty_spaces);
                sem_wait(&use);
                printf("Monitor thread : writing to buffer at position %d, value \n",pos+1,cnt);
                free_pos--;
                buffer[pos++] = cnt;
                cnt = 0;
                while(pos>=buffer_size) pos-=buffer_size;
                sem_post(&use);
                sem_post(&full_spaces);
                sem_post(semaphore);
            }
        }
    private:
        sem_t* semaphore;
        int pos;
};

class mcollector{
    public :
        mcollector(){
            pos = 0;
        }
        void run(){
            while(1){
                sleep(collector_sleep);
                if(free_pos == buffer_size) printf("Collector thread : nothing is in the buffer\n");
                sem_wait(&full_spaces);
                sem_wait(&use);
                printf("COLLECTOR thread : reading value %d from buffer at position %d\n",buffer[pos] ,pos+1);
                free_pos++, pos++;
                if(pos == buffer_size) pos = 0;
                sem_post(&use);
                sem_post(&empty_spaces);
            }
        }
    private :
        int pos;
};

void *mcounter_spawn(void *args){
    mcounter_data* data = (mcounter_data*)args;
    int x = 100;
    while(x--){
        sleep(rand()%2);
        printf("Counter thread %d : now received a message\n",data->ind+1);
        mcounter * inst = new mcounter(data->semaphore, data->ind+1);
        inst->run();
        delete inst;
    }
    delete args;
}

void *mmonitor_spawn(void *args){
    mmonitor * monitor = new mmonitor((sem_t*)args);
    monitor->run();
    delete monitor;
    delete args;
}

void *mcollector_spawn(void *args){
    mcollector *collector = new mcollector();
    collector->run();
    delete collector;
    delete args;
}

int main()
{
    cin>>N;
    threads.resize(N);
    buffer.resize(buffer_size);
    free_pos = buffer_size;
    sem_t cnt_sem;
    sem_init(&cnt_sem, 1, 1);
    init_sems();
    for(int i = 0 ; i < N ; i++){
        pthread_create(&threads[i], NULL, mcounter_spawn, new mcounter_data(i,&cnt_sem));
    }
    pthread_create(&monitor_thread, NULL, mmonitor_spawn, &cnt_sem);
    pthread_create(&collector_thread, NULL, mcollector_spawn, NULL);
    for(int i = 0 ; i < N ; i++){
        pthread_join(threads[i], NULL);
    }
    pthread_join(monitor_thread, NULL);
    pthread_join(collector_thread, NULL);
    return 0;
}
