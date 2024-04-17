#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <x86intrin.h>

using namespace std;
#define p 12
#define k 2
#define pk 24
sem_t mut[pk];
float globalmin = 1000000;
int temp = 1000;          // You can use any value here
unsigned int seed = 1333; // You can use any value here
std::mt19937 gen(seed);   
std::uniform_int_distribution<> dis(1, 100);
int value, value1, value2, count1 = 0, count2 = 0, count3 = 0;
float globalError1 = 0, globalError2 = 0, globalError3 = 0, count;
vector<priority_queue<int, vector<int>, greater<int>>> pq;

int randomInt(int min, int max) { return min + dis(gen) % (max - min + 1); }

constexpr double CPU_FREQUENCY_GHZ = 2.4;

void minofAllqueues(void) {
  globalmin = 1000000;
  for (int i = 0; i < pk; i++) {
    if (!pq[i].empty()) {
      globalmin = min(pq[i].top(), (int)globalmin);
    }
  }
}

void *insert(void *arg) {
  while (1) {
    int index = dis(gen) % pk;
    int value;
    sem_getvalue(&mut[index], &value);
    if (value == 1) {
      sem_wait(&mut[index]);
      uint64_t start = __rdtsc();
      for (int i = 0; i < 100000; i++) {
        int t = 1 + rand() % 1000;
        pq[index].push(t);
      }
      uint64_t end = __rdtsc();
      sem_post(&mut[index]);
      break;
    }
  }
  return NULL;
}
void *deleteMin(void *arg) {
  int n = 0;
  while (n < 10000) {
    int index1 = randomInt(0, pk);
    int index2 = randomInt(0, pk);
    if (index1 == index2)
      continue;
    sem_getvalue(&mut[index1], &value1);
    sem_getvalue(&mut[index2], &value2);
    int ops = 0;
    if (value1 && value2 && !pq[index1].empty() && !pq[index2].empty()) {
      if (pq[index1].top() > pq[index2].top()) {
        if (!pq[index2].empty() && n + ops < 10000) {
          sem_wait(&mut[index2]);
          minofAllqueues();
             if (!pq[index2].empty()) {
            globalError3 += (abs(globalmin - pq[index2].top()) / globalmin);
            cout << globalmin << " " << pq[index2].top() << " "
                 << abs(globalmin - pq[index2].top()) << " "
                 << (abs(globalmin - pq[index2].top()) / globalmin) << endl;
            if (!!pq[index2].empty())
              pq[index2].pop();
            n++;
            ops++;
          }
        }
        sem_post(&mut[index2]);
      } else {
        sem_wait(&mut[index1]);
        minofAllqueues();
        if (!pq[index1].empty() && n + ops < 10000) {
          if (!pq[index1].empty()) {
            globalError3 += (abs(globalmin - pq[index1].top()) / globalmin);
            cout << globalmin << " " << pq[index1].top() << " "
                 << abs(globalmin - pq[index1].top()) << " "
                 << (abs(globalmin - pq[index1].top()) / globalmin) << endl;
            pq[index1].pop();
            n++;
            ops++;
          }
        }
        sem_post(&mut[index1]);
      }
    }
    n++;
  }
  return NULL;
}
void *OptHalfinsert(void *arg) {
  int i = *((int *)(arg));
  while (1) {
    int index;
    if (i < pk / 2)
      index = randomInt(0, pk / 2);
    else {
      index = randomInt((pk / 2) + 1, pk - 1);
    }
    sem_getvalue(&mut[index], &value1);
    if (value1) {
      sem_wait(&mut[index]);
      int c = randomInt(0, 1000);
      pq[index].push(c);
      sem_post(&mut[index]);
      break;
    }
  }
  return NULL;
}

void *OptHalfDelete(void *arg) {
  int i = *((int *)(arg));
  int check1 = 0;
  while (check1 < 10000) {
    int index1, index2;
    if (i < p / 2) {
      index1 = randomInt(0, pk / 2);
      index2 = randomInt(0, pk / 2);
    } else {
      index1 = randomInt((pk / 2) + 1, pk - 1);
      index2 = randomInt((pk / 2) + 1, pk - 1);
    }
    if (index1 == index2) {
      continue;
    }
    sem_getvalue(&mut[index1], &value1);
    sem_getvalue(&mut[index2], &value2);
    if (value1 && value2 && !pq[index1].empty() && !pq[index2].empty()) {
      if (pq[index1].top() > pq[index2].top()) {
        sem_wait(&mut[index2]);
        minofAllqueues();
        if (globalmin <= 0) {
          pq[index2].pop();
          continue;
        }
        globalError1 += (abs(globalmin - pq[index2].top()) / globalmin);
        count1++;
        pq[index2].pop();
        sem_post(&mut[index2]);
        check1++;
      } else {
        sem_wait(&mut[index1]);
        minofAllqueues();
        if (globalmin <= 0) {
          pq[index1].pop();
          continue;
        }
        globalError1 += (abs(globalmin - pq[index1].top()) / globalmin);
        count1++;
        pq[index1].pop();
        sem_post(&mut[index1]);
        check1++;
      }
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
  cout << "Insert successfull" << endl;

  pthread_t th1[p];
  for (int i = 0; i < 1; i++) {
    pthread_create(&th1[i], NULL, deleteMin, &i);
  }
  for (int i = 0; i < 1; i++) {
    pthread_join(th1[i], NULL);
  }
  cout << "Optexact delete Succesfull" << endl;
  //
  cout << "Runned succesfully\n" << endl;
  cout << " " << globalError2 << endl;
  cout << " " << count2 << endl;

  return 0;
}
