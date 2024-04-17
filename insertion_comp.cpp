#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <x86intrin.h>

using namespace std;
#define p 12
#define k 2
#define pk 24
sem_t mut[pk];
int value, value1, value2;
vector<priority_queue<int>> pq;
constexpr double CPU_FREQUENCY_GHZ = 2.4; 

long throughput_og;
long throughput_half;

int randomInt(int min, int max) {
    return min + rand() % (max - min);
}

void *insert(void *arg) {
    while (1) {
        int index = rand() % pk;
        int value;
        sem_getvalue(&mut[index], &value);
        if (value == 1) {
            sem_wait(&mut[index]);
            uint64_t start = __rdtsc();
            for (int i = 0; i < 1000; i++) {
                pq[index].push(rand() % 1000);
            }
            uint64_t end = __rdtsc();

            double elapsed_cycles = static_cast<double>(end - start);
            double elapsed_seconds = elapsed_cycles / (CPU_FREQUENCY_GHZ * 1e9); // Convert cycles to seconds

            cout << "Elapsed time in seconds: " << elapsed_seconds << endl;

            throughput_og += (1000000 / elapsed_seconds);
            sem_post(&mut[index]);
            break;
        }
    }
    return NULL;
}

void *OptHalfinsert(void *arg) {
  int i = *((int *)(arg));
  while (1) {
    int index;
    if(i  < pk/2)
      index = randomInt(0, pk/2);
    else{
      index = randomInt((pk/2)+1, pk-1);
    }
    sem_getvalue(&mut[index], &value1);
    if (value1) {
      sem_wait(&mut[index]);
        uint64_t start = __rdtsc();
        for (int i = 0; i < 1000; i++) {
            pq[index].push(rand() % 1000);
        }
        uint64_t end = __rdtsc();

        double elapsed_cycles = static_cast<double>(end - start);
        double elapsed_seconds = elapsed_cycles / (CPU_FREQUENCY_GHZ * 1e9); // Convert cycles to seconds

        cout << "Elapsed time in seconds: " << elapsed_seconds << endl;

        throughput_half += (1000000 / elapsed_seconds);
        sem_post(&mut[index]);
        break;
    }
  }
  return NULL;
}

int main() {
  pq.resize(pk);
  pthread_t th[p];
  for (int i = 0; i < pk; i++) {
    sem_init(&mut[i], 0, 1);
  }
  int s = pk;
  for (int i = 0; i < p; i++) {
    pthread_create(&th[i], NULL, insert, &s);
  }
  for (int i = 0; i < p; i++) {
    pthread_join(th[i], NULL);
  }
  for (int i = 0; i < p; i++) {
    pthread_create(&th[i], NULL, OptHalfinsert, &i);
  }
  for (int i = 0; i < p; i++) {
    pthread_join(th[i], NULL);
  }
  
   printf("Insert %ld\n",throughput_og);
   printf("OptHalfInsert %ld\n",throughput_half);
  // pthread_t th1[p];
  // for (int i = 0; i < p - 3; i++) {
  //   pthread_create(&th1[i], NULL, OptExactDelete, &s);
  // }
  // for (int i = 0; i < p - 3; i++) {
  //   pthread_join(th1[i], NULL);
  // }

  // cout << "Runned succesfully\n" << endl;
  // for (int i = 0; i < pq.size(); i++) {
  //   while (!pq[i].empty()) {
  //     cout << pq[i].top() << " ";
  //     pq[i].pop();
  //   }
  //   cout << endl;
  // }
  // return 0;
}