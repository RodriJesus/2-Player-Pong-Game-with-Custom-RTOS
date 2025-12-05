// os.c
// Runs on LM4F120/TM4C123/MSP432
// Lab 3 starter file.
// Daniel Valvano
// March 24, 2016

#include <stdint.h>
#include <stdlib.h> // Allows use of NULL
#include "os.h"
#include "CortexM.h"
#include "BSP.h"
int32_t FifoSemaphore; // counts the number of valid items in the FIFO
// function definitions in osasm.s
void StartOS(void);

#define NUMTHREADS  6        // maximum number of threads
#define NUMPERIODIC 2        // maximum number of periodic threads
#define STACKSIZE   100      // number of 32-bit words in stack per thread
struct tcb{
  int32_t *sp;       // pointer to stack (valid for threads not running
  struct tcb *next;  // linked-list pointer
   // nonzero if blocked on this semaphore
   // nonzero if this thread is sleeping
//*FILL THIS IN****
	int32_t *blocked;
	uint32_t sleep;
};
typedef struct tcb tcbType;
tcbType tcbs[NUMTHREADS];
tcbType *RunPt;
int32_t Stacks[NUMTHREADS][STACKSIZE];

typedef struct{
	void(*Task)(void);
	uint32_t period;
	uint32_t counter;
} periodic_t;
periodic_t Periodic[NUMPERIODIC];

// ******** OS_Init ************
// Initialize operating system, disable interrupts
// Initialize OS controlled I/O: periodic interrupt, bus clock as fast as possible
// Initialize OS global variables
// Inputs:  none
// Outputs: none
void OS_Init(void){
  DisableInterrupts();
  BSP_Clock_InitFastest();// set processor clock to fastest speed
  // perform any initializations needed
		
	RunPt = NULL;
	
	for(int i =0; i < NUMTHREADS; i++){
		tcbs[i].sp = NULL;
		tcbs[i].next = NULL;
	}
}

void SetInitialStack(int i){

  // **Same as Lab 2****
	tcbs[i].sp = &Stacks[i][STACKSIZE-16]; // thread stack pointer
  Stacks[i][STACKSIZE-1] = 0x01000000;   // thumb bit
  Stacks[i][STACKSIZE-3] = 0x14141414;   // R14
  Stacks[i][STACKSIZE-4] = 0x12121212;   // R12
  Stacks[i][STACKSIZE-5] = 0x03030303;   // R3
  Stacks[i][STACKSIZE-6] = 0x02020202;   // R2
  Stacks[i][STACKSIZE-7] = 0x01010101;   // R1
  Stacks[i][STACKSIZE-8] = 0x00000000;   // R0
  Stacks[i][STACKSIZE-9] = 0x11111111;   // R11
  Stacks[i][STACKSIZE-10] = 0x10101010;  // R10
  Stacks[i][STACKSIZE-11] = 0x09090909;  // R9
  Stacks[i][STACKSIZE-12] = 0x08080808;  // R8
  Stacks[i][STACKSIZE-13] = 0x07070707;  // R7
  Stacks[i][STACKSIZE-14] = 0x06060606;  // R6
  Stacks[i][STACKSIZE-15] = 0x05050505;  // R5
  Stacks[i][STACKSIZE-16] = 0x04040404;  // R4
}

//******** OS_AddThreads ***************
// Add six main threads to the scheduler
// Inputs: function pointers to six void/void main threads
// Outputs: 1 if successful, 0 if this thread can not be added
// This function will only be called once, after OS_Init and before OS_Launch
int OS_AddThreads(void(*thread0)(void),
                  void(*thread1)(void),
                  void(*thread2)(void),
                  void(*thread3)(void),
                  void(*thread4)(void),
                  void(*thread5)(void)){
  // **similar to Lab 2. initialize as not blocked, not sleeping****
 int32_t status = StartCritical();
// Link TCBs in a circular list:
  tcbs[0].next = &tcbs[1];
  tcbs[1].next = &tcbs[2];
  tcbs[2].next = &tcbs[3];
  tcbs[3].next = &tcbs[4];
  tcbs[4].next = &tcbs[5];
  tcbs[5].next = &tcbs[0];
  
  // Initialize each thread's stack (set blocked and sleep to 0)
  SetInitialStack(0);Stacks[0][STACKSIZE-2] = (int32_t)(thread0);
  tcbs[0].blocked = 0;
  tcbs[0].sleep = 0;
										
  SetInitialStack(1);Stacks[1][STACKSIZE-2] = (int32_t)(thread1);
  tcbs[1].blocked = 0;
  tcbs[1].sleep = 0;	
										
  SetInitialStack(2);Stacks[2][STACKSIZE-2] = (int32_t)(thread2);
  tcbs[2].blocked = 0;
  tcbs[2].sleep = 0;			

  SetInitialStack(3);Stacks[3][STACKSIZE-2] = (int32_t)(thread3);
  tcbs[3].blocked = 0;
  tcbs[3].sleep = 0;
	
	SetInitialStack(4);Stacks[4][STACKSIZE-2] = (int32_t)(thread4);
  tcbs[4].blocked = 0;
  tcbs[4].sleep = 0;
	
	SetInitialStack(5);Stacks[5][STACKSIZE-2] = (int32_t)(thread5);
  tcbs[5].blocked = 0;
  tcbs[5].sleep = 0;
										
  RunPt = &tcbs[0]; // first thread to run
  EndCritical(status);
  return 1;               // successful
}

//******** OS_AddPeriodicEventThread ***************
// Add one background periodic event thread
// Typically this function receives the highest priority
// Inputs: pointer to a void/void event thread function
//         period given in units of OS_Launch (Lab 3 this will be msec)
// Outputs: 1 if successful, 0 if this thread cannot be added
// It is assumed that the event threads will run to completion and return
// It is assumed the time to run these event threads is short compared to 1 msec
// These threads cannot spin, block, loop, sleep, or kill
// These threads can call OS_Signal
// In Lab 3 this will be called exactly twice
int OS_AddPeriodicEventThread(void(*thread)(void), uint32_t period){
// ****IMPLEMENT THIS****
	static int index = 0;
	if(index >= NUMPERIODIC) return 0;
	Periodic[index].Task = thread; // Store the function pointer
	Periodic[index].period = period; // Set the period
	Periodic[index].counter = period; // Initialize the counter
	index++;	
  return 1; // Succesfully added
}

void static runperiodicevents(void){
// ****IMPLEMENT THIS****
// **RUN PERIODIC THREADS, DECREMENT SLEEP COUNTERS
  tcbType *temp = RunPt;           // Start from the current thread
  for (int i = 0; i < NUMTHREADS; i++){
    if (temp->sleep > 0){          // If the thread is sleeping...
      temp->sleep--;               // ...decrement its sleep counter.
    }
    temp = temp->next;             // Move to the next thread in the circular list.
  }

  // -------------------------------
  // 2. Process periodic event threads.
  // -------------------------------
  for (int i = 0; i < NUMPERIODIC; i++){
    if (Periodic[i].counter > 0){
      Periodic[i].counter--;       // Decrement the periodic counter (assumed to be in ms).
      if (Periodic[i].counter == 0){  // Time for this periodic event to run.
        Periodic[i].Task();          // Call the periodic event function.
        // Reset the counter back to its full period.
        Periodic[i].counter = Periodic[i].period;
      }
    }
  }
}

//******** OS_Launch ***************
// Start the scheduler, enable interrupts
// Inputs: number of clock cycles for each time slice
// Outputs: none (does not return)
// Errors: theTimeSlice must be less than 16,777,216
void OS_Launch(uint32_t theTimeSlice){
  STCTRL = 0;                  // disable SysTick during setup
  STCURRENT = 0;               // any write to current clears it
  SYSPRI3 =(SYSPRI3&0x00FFFFFF)|0xE0000000; // priority 7
  STRELOAD = theTimeSlice - 1; // reload value
  STCTRL = 0x00000007;         // enable, core clock and interrupt arm
  StartOS();                   // start on the first task
}
// runs every ms
void Scheduler(void){ // every time slice
// ROUND ROBIN, skip blocked and sleeping threads
  runperiodicevents();  // Process periodic events and decrement sleep counters

  // Then pick the next thread that is not blocked and not sleeping
  do {
    RunPt = RunPt->next;
  } while( (RunPt->blocked != 0) || (RunPt->sleep > 0) );
}

//******** OS_Suspend ***************
// Called by main thread to cooperatively suspend operation
// Inputs: none
// Outputs: none
// Will be run again depending on sleep/block status
void OS_Suspend(void){
  STCURRENT = 0;        // any write to current clears it
  INTCTRL = 0x04000000; // trigger SysTick
// next thread gets a full time slice
}

// ******** OS_Sleep ************
// place this thread into a dormant state
// input:  number of msec to sleep
// output: none
// OS_Sleep(0) implements cooperative multitasking
void OS_Sleep(uint32_t sleepTime){
// set sleep parameter in TCB
	DisableInterrupts();
	RunPt->sleep = sleepTime;
	EnableInterrupts();
// suspend, stops running
	OS_Suspend();
}

// ******** OS_InitSemaphore ************
// Initialize counting semaphore
// Inputs:  pointer to a semaphore
//          initial value of semaphore
// Outputs: none
void OS_InitSemaphore(int32_t *semaPt, int32_t value){
//***IMPLEMENT THIS***
	*semaPt = value;
}

// ******** OS_Wait ************
// Decrement semaphore and block if less than zero
// Lab2 spinlock (does not suspend while spinning)
// Lab3 block if less than zero
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Wait(int32_t *semaPt){
//***IMPLEMENT THIS***
	DisableInterrupts();
	(*semaPt)=(*semaPt) - 1;
	if ((*semaPt)<0){
		// Mark the current thread as blocked
		RunPt->blocked = semaPt;
		EnableInterrupts();
		OS_Sleep(0); // yield control (calls OS_Suspend internally)
	}
	EnableInterrupts();
	
}

// ******** OS_Signal ************
// Increment semaphore
// Lab2 spinlock
// Lab3 wakeup blocked thread if appropriate
// Inputs:  pointer to a counting semaphore
// Outputs: none
void OS_Signal(int32_t *semaPt){
//***IMPLEMENT THIS***
	DisableInterrupts();
	(*semaPt) = (*semaPt) +1;
	// Search for a thread blocked on this semaphore and unblock it
	tcbType *temp = RunPt;
	for (int i = 0; i < NUMTHREADS; i++){
		if (temp->blocked == semaPt){
			temp->blocked = 0; //unblock the thread
			break; // if you expect only one thread to wait; otherwise, continue the search
		}
		temp = temp->next;
	}
	EnableInterrupts();
}

#define FSIZE 10    // can be any size
uint32_t PutI;      // index of where to put next
uint32_t GetI;      // index of where to get next
uint32_t Fifo[FSIZE];
int32_t CurrentSize;// 0 means FIFO empty, FSIZE means full
uint32_t LostData;  // number of lost pieces of data

// ******** OS_FIFO_Init ************
// Initialize FIFO.  
// One event thread producer, one main thread consumer
// Inputs:  none
// Outputs: none
void OS_FIFO_Init(void){
//***IMPLEMENT THIS***
	PutI = 0;
	GetI = 0;
	CurrentSize = 0;
	LostData = 0;
	OS_InitSemaphore(&FifoSemaphore,0);
	
}

// ******** OS_FIFO_Put ************
// Put an entry in the FIFO.  
// Exactly one event thread puts,
// do not block or spin if full
// Inputs:  data to be stored
// Outputs: 0 if successful, -1 if the FIFO is full
int OS_FIFO_Put(uint32_t data){
//***IMPLEMENT THIS***
	int result;
	DisableInterrupts(); // Start critical section
	if(CurrentSize == FSIZE){
		LostData++;
		result = -1; // FIFO is full
	} else{
	Fifo[PutI] = data;
	PutI = (PutI + 1) % FSIZE;
	CurrentSize++;
	OS_Signal(&FifoSemaphore); // signal that new data is availalble
  result = 0;   // success
	}
	EnableInterrupts(); // end critical section
	return result;

}

// ******** OS_FIFO_Get ************
// Get an entry from the FIFO.   
// Exactly one main thread get,
// do block if empty
// Inputs:  none
// Outputs: data retrieved
uint32_t OS_FIFO_Get(void){uint32_t data;
//***IMPLEMENT THIS***
	OS_Wait(&FifoSemaphore); //block if FIFO is empty
	
	DisableInterrupts(); // Start critical section
	data = Fifo[GetI];
	GetI = (GetI + 1) % FSIZE;
	CurrentSize--;
	EnableInterrupts(); // end critical section

  return data;
}



