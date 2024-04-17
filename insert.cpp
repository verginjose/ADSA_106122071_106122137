#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <x86intrin.h>

using namespace std;
#define p 2
#define k 2
#define pk 4
sem_t mut[pk];
int value, value1, value2;
vector<priority_queue<int, vector<int>, greater<int>>> pq;
unsigned int seed = 1333; 
std::mt19937 gen(seed);  
std::uniform_int_distribution<> dis(1, 100);
int randomInt(int min, int max) { return min + dis(gen) % (max - min + 1); }

constexpr double CPU_FREQUENCY_GHZ = 2.4;
long long throughput_og = 0;
long long throughput_half = 0;

void *insert(void *arg) {
  while (1) {
    int index = randomInt(0, pk);
    int value;
    sem_getvalue(&mut[index], &value);
    if (value == 1) {
      sem_wait(&mut[index]);
      uint64_t start = __rdtsc();
      for (int i = 0; i < 1000000; i++) {
        pq[index].push(randomInt(0, 100));
      }
      uint64_t end = __rdtsc();

      double elapsed_cycles = static_cast<double>(end - start);
      double elapsed_seconds = elapsed_cycles / (CPU_FREQUENCY_GHZ * 1e9);

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
    if (i < p / 2)
      index = randomInt(0, (pk / 2) - 1);
    else {
      index = randomInt(pk / 2, pk - 1);
    }
    sem_getvalue(&mut[index], &value1);
    if (value1) {
      sem_wait(&mut[index]);
      uint64_t start = __rdtsc();
      for (int i = 0; i < 1000000; i++) {
        pq[index].push(randomInt(0, 100));
      }
      uint64_t end = __rdtsc();

      double elapsed_cycles = static_cast<double>(end - start);
      double elapsed_seconds = elapsed_cycles / (CPU_FREQUENCY_GHZ * 1e9);

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

  printf("Originial: %lld\n", throughput_og);
  pthread_t th1[p];
  for (int i = 0; i < p; i++) {
    pthread_create(&th1[i], NULL, OptHalfinsert, &i);
  }
  for (int i = 0; i < p; i++) {
    pthread_join(th1[i], NULL);
  }
  printf("Half Insert %lld\n", throughput_half);

  cout << "Runned succesfully\n" << endl;
}
