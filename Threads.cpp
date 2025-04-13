
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>

#include <iostream>


// #define NUM_PROCESSES 2
// #define LOOP_NUM 100

using namespace std;

// int total = 0;
// void load_increment(){
//     for(int i=0; i<LOOP_NUM;i++){
//         total++;
//         cout<<total<<endl;
//     }
// }

// int main(int argc, char* argv[]){
//     pid_t pids [NUM_PROCESSES];
//     for (int i=0; i<NUM_PROCESSES; i++){
//         pids[i] = fork();
//         if (pids[i] == 0){
//             load_increment();
//             cout<<"Child process "<<i<<endl;
//             cout<<"PID: "<<getpid()<<endl;
//             cout<<total<<endl;
//             cout<<"Parent PID: "<<getppid()<<endl;
//             exit(0); // this is a critical line becuase if we dont exit then even the child process will continue the for loop and spawn a grndchild.
//         }
//     }

//     for (int i=0; i<NUM_PROCESSES; i++){
//         waitpid(pids[i], NULL, 0);
//     }
//     cout<<"Parent Total: "<<total<<endl;
//     return EXIT_SUCCESS;
// }

// Sample code that demonstrates a data race between threads reading and writing
// from the same shared global variable in Static Data.
// Interleaving of reads and writes will sometimes cause printed sum to be less
// than expected.

////////// USING THREADS//////////////////
constexpr int NUM_THREADS = 50;
constexpr int LOOP_NUM = 100;

static int sum_total = 0;

// increment sum_total LOOP_NUM times
void* thread_main(void* arg) {
  for (int i = 0; i < LOOP_NUM; i++) {
    sum_total++;
  }
  return nullptr;  // return type is a pointer
}


int main(int argc, char** argv) {
  array<pthread_t, NUM_THREADS> thds{};  // array of thread ids

  // create threads to run thread_main()
  for (int i = 0; i < NUM_THREADS; i++) {
    if (pthread_create(&thds.at(i), nullptr, &thread_main, nullptr) != 0) {
      cerr << "pthread_create failed" << endl;
    }
  }

  // wait for all child threads to finish
  // (children may terminate out of order, but cleans up in order)
  for (int i = 0; i < NUM_THREADS; i++) {
    if (pthread_join(thds.at(i), nullptr) != 0) {
      cerr << "pthread_join failed" << endl;
    }
  }

  // print out the final sum (expecting NUM_THREADS * LOOP_NUM)
  cout << "Total: " << sum_total << endl;

  return EXIT_SUCCESS;
}


// -------------------------------------------------------------------------------------------------

// Sample code that demonstrates using pthread mutex synchronization.
// from the same shared global variable in Static Data.
// Interleaving of reads and writes will sometimes cause printed sum to be less
// than expected.
//
// Remember to compile with -pthread

#include <pthread.h>
#include <iostream>
#include <array>

using std::cout;
using std::cerr;
using std::endl;
using std::array;

constexpr int NUM_THREADS = 50;
constexpr int LOOP_NUM = 100;


// NEW: struct to pass arguments to thread_main
struct thd_arg {
  int* sum_ptr;
  pthread_mutex_t* lock_ptr;
  int num;
};

// increment sum_total LOOP_NUM times in an atomic fashion
void* thread_main(void* arg) {
  // NEW: cast arguments back into struct
  struct thd_arg* a = reinterpret_cast<struct thd_arg*>(arg);
  int local_sum = 0;
  for (int i = 0; i < a->num; i++) {
    local_sum++;
  }
  pthread_mutex_lock(a->lock_ptr);
  *a->sum_ptr += local_sum;
  pthread_mutex_unlock(a->lock_ptr);
  // NEW: delete dynamically-allocated struct
  delete a;
  return nullptr;  // return type is a pointer
}


int main(int argc, char** argv) {
  // NEW: made sum_total a local variable
  int sum_total = 0;
  pthread_mutex_t sum_lock;  // will be accessible by all threads
  array<pthread_t, NUM_THREADS> thds{};  // array of thread ids
  pthread_mutex_init(&sum_lock, nullptr);  // initialize mutex to default

  // create threads to run thread_main()
  for (int i = 0; i < NUM_THREADS; i++) {
    struct thd_arg* args = new struct thd_arg;
    args->sum_ptr = &sum_total;
    args->lock_ptr = &sum_lock;
    args->num = LOOP_NUM;
    if (pthread_create(&thds.at(i), nullptr, &thread_main, args) != 0) {
      cerr << "pthread_create failed" << endl;
    }
  }

  // wait for all child threads to finish
  // (children may terminate out of order, but cleans up in order)
  for (int i = 0; i < NUM_THREADS; i++) {
    if (pthread_join(thds.at(i), nullptr) != 0) {
      cerr << "pthread_join failed" << endl;
    }
  }

  // print out the final sum (expecting NUM_THREADS * LOOP_NUM)
  cout << "Total: " << sum_total << endl;

  pthread_mutex_destroy(&sum_lock);  // destroy the mutex to clean up
  return EXIT_SUCCESS;
}