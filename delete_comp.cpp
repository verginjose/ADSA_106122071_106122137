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
unsigned int seed = 1333;
std::mt19937 gen(seed);   
std::uniform_int_distribution<> dis(1, 100);
vector<priority_queue<int, vector<int>, greater<int>>> pq;
constexpr double CPU_FREQUENCY_GHZ = 4.1;

long throughput_og = 0;
long throughput_half = 0;
long throughput_exact_half = 0;
int randomInt(int min, int max) { return min + dis(gen) % (max - min + 1); }

void *insert(void *arg) {
  while (1) {
    int index = randomInt(0, pk);
    int value;
    sem_getvalue(&mut[index], &value);
    if (value == 1) {
      sem_wait(&mut[index]);
      uint64_t start = __rdtsc();
      for (int i = 0; i < 2000000; i++) {
        pq[index].push(rand() % 1000);
      }
      uint64_t end = __rdtsc();

      double elapsed_cycles = static_cast<double>(end - start);
      double elapsed_seconds =
          elapsed_cycles /
          (CPU_FREQUENCY_GHZ * 1e9); 

      cout << "Elapsed time in seconds11: " << elapsed_seconds << endl;

      sem_post(&mut[index]);
      break;
    }
  }
  return NULL;
}

void *deleteMin(void *arg) {
  int n = 0;
  while (n < 1000000) {
    int index1 = rand() % pk;
    int index2 = rand() % pk;
    if (index1 == index2)
      continue;
    sem_getvalue(&mut[index1], &value1);
    sem_getvalue(&mut[index2], &value2);
    int ops = 0;
    if (value1 && value2 && !pq[index1].empty() && !pq[index2].empty()) {
      uint64_t start = __rdtsc();
      if (pq[index1].top() > pq[index2].top()) {
        sem_wait(&mut[index2]);
        while (!pq[index2].empty() && n + ops < 1000000) {
          pq[index2].pop();
          ops++;
        }
        sem_post(&mut[index2]);
      } else {
        sem_wait(&mut[index1]);
        while (!pq[index1].empty() && n + ops < 1000000) {
          pq[index1].pop();
          ops++;
        }
        sem_post(&mut[index2]);
      }
      uint64_t end = __rdtsc();
      double elapsed_cycles = static_cast<double>(end - start);
      double elapsed_seconds =
          elapsed_cycles /
          (CPU_FREQUENCY_GHZ * 1e9); // Convert cycles to seconds
      cout << "Elapsed time in seconds22: " << elapsed_seconds << endl;
      throughput_og += (ops / elapsed_seconds);
      n += ops;
    }
  }
  return NULL;
}

void *OptHalfDelete(void *arg) {
  int i = *((int *)(arg));
  int check1 = 0;
  while (check1 < 1000000) {
    int index1, index2;
    if (i < p / 2) {
      index1 = randomInt(0, pk / 2);
      index2 = randomInt(0, pk / 2);
    } else {
      index1 = randomInt((pk / 2) + 1, pk - 1);
      index2 = randomInt((pk / 2) + 1, pk - 1);
    }
    index1 = index1 % pk;
    index2 = index2 % pk;
    if (index1 == index2) {
      continue;
    }
    sem_getvalue(&mut[index1], &value1);
    sem_getvalue(&mut[index2], &value2);
    if (pq[index1].top() < pq[index2].top()) {
      if (value1 && !pq[index1].empty()) {
        sem_wait(&mut[index1]);
        pq[index1].pop();
        sem_post(&mut[index1]);
        check1++;
      }
    } else {
      if (value2 && !pq[index2].empty()) {
        sem_wait(&mut[index2]);
        pq[index2].pop();
        sem_post(&mut[index2]);
        check1++;
      }
    }
  }
  return NULL;
}

void *OptExactDelete(void *arg) {
  int i = *((int *)(arg));
  int flag = 1;
  int n = 0;
  while (n < 1000000) {
    int index1, index2;
    if (flag) {
      index1 = randomInt(i, i + k);
      index2 = randomInt(i, i + k);
      index1 = index1 % pk;
      index2 = index2 % pk;
      flag = 0;
      if (index1 == index2) {
        flag = 1;
        continue;
      }

    } else {
      if (i < pk / 2) {
        index1 = randomInt(0, pk / 2);
        index2 = randomInt(0, pk / 2);
      } else {
        index1 = randomInt((pk / 2) + 1, pk - 1);
        index2 = randomInt((pk / 2) + 1, pk - 1);
      }
      if (index1 == index2) {
        continue;
      }
    }
    sem_getvalue(&mut[index1], &value1);
    sem_getvalue(&mut[index2], &value2);
    int ops = 0;
    if (value1 && value2 && !pq[index1].empty() && !pq[index2].empty()) {
      uint64_t start = __rdtsc();
      if (pq[index1].top() > pq[index2].top()) {
        sem_wait(&mut[index2]);
        while (!pq[index2].empty() && n + ops < 1000000) {
          pq[index2].pop();
          ops++;
        }
        sem_post(&mut[index2]);
      } else {
        sem_wait(&mut[index1]);
        while (!pq[index1].empty() && n + ops < 1000000) {
          pq[index2].pop();
          ops++;
        }
        sem_post(&mut[index1]);
      }
      uint64_t end = __rdtsc();
      double elapsed_cycles = static_cast<double>(end - start);
      double elapsed_seconds =
          elapsed_cycles /
          (CPU_FREQUENCY_GHZ * 1e9); // Convert cycles to seconds
      cout << "Elapsed time in seconds44: " << elapsed_seconds << endl;
      throughput_exact_half += (ops / elapsed_seconds);
      n += ops;
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

  pthread_t th1[p];
  for (int i = 0; i < p; i++) {
    pthread_create(&th1[i], NULL, deleteMin, &s);
  }
  for (int i = 0; i < p / 2; i++) {
    pthread_join(th1[i], NULL);
  }
  printf("Original %ld\n", throughput_og);

  // pthread_t thi1[p];
  //  for (int i = 0; i < p; i++) {

  //   pthread_create(&thi1[i], NULL, insert, &s);
  // }
  // for (int i = 0; i < p; i++) {
  //   pthread_join(thi1[i], NULL);
  // }

  // pthread_t th2[p];
  // for (int i = 0; i < p; i++) {
  //   pthread_create(&th2[i], NULL, OptHalfDelete, &i);
  // }
  // for (int i = 0; i < p; i++) {
  //   pthread_join(th2[i], NULL);
  // }
  // printf("OptHalf %ld\n", throughput_half);

  pthread_t thi3[p];
  for (int i = 0; i < p; i++) {
    pthread_create(&thi3[i], NULL, insert, &s);
  }
  for (int i = 0; i < p; i++) {
    pthread_join(thi3[i], NULL);
  }

  pthread_t th3[p];
  for (int i = 0; i < p - 1; i++) {
    pthread_create(&th3[i], NULL, OptExactDelete, &i);
  }
  for (int i = 0; i < p - 1; i++) {
    pthread_join(th3[i], NULL);
  }

  printf("OPtHalfExact %ld\n", throughput_exact_half);
  return 0;
}