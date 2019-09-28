#ifndef SIM_PROC_H
#define SIM_PROC_H

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <assert.h>
#include <iomanip>
enum Status {
	FULL = 1,
	EMPTY,
	VALID, //3
	INVALID //4
};
typedef struct {
	int sequence;
	unsigned int pc;
	int type;

	//Original values
	int dest;
	int src1;
	int src2;

	int destReg;
	int srcReg1;
	int srcReg2;

	int src1Ready;
	int src2Ready;

	int register_renamed1;
	int register_renamed2;

	int RMTtag1;
	int RMTtag2;

	int initFE;
	int timeFE;
	int initDE;
	int timeDE;
	int initRN;
	int timeRN;
	int initRR;
	int timeRR;
	int initDI;
	int timeDI;
	int initIS;
	int timeIS;
	int initEX;
	int timeEX;
	int initWB;
	int timeWB;
	int initRT;
	int timeRT;

	int robTag;

	int no_of_cycle;

} inst;

typedef struct rmtBlock {
	int RMTValidbit;
	int tag;
} rmtBlock;

//Block in Issue Queue.
typedef struct issueQueueBlock {
	int valid;
	long int age;
	
	inst Instruction;
} iqBlock;

typedef struct robBlock {

	int valid;
	int tag;
	int ready;
	inst Instr;

} robBlock;

typedef struct ROBQueue {
	int head, tail, size, count;
	int valid;
	robBlock *ROB;

}ROBQueue;




typedef struct decodeRegister {

	int valid;
	int status;
	bool state;
	int width;
	inst *DEInstruction;

}decodeRegister;

typedef struct renameRegister {

	int valid;
	int status;
	int state;
	int width;
	inst *Instruction;

}renameRegister;

typedef struct regReadRegister {

	int valid;
	int status;
	int state;
	int width;
	inst *Instruction;
}regReadRegister;

typedef struct dispatchRegister {

	int valid;
	int status;
	int state;
	int width;
	inst *Instruction;
}dispatchRegister;

typedef struct issueQueue {

	int size;
	int width;
	int IQValid;
	int issueNum;
	long int cage;
	int state;
	iqBlock *IQ;
	iqBlock *sortedIQ;
	inst *tempInstruction;
	inst *Instruction;

}issueQueue;

typedef struct writeBackRegister {

	int width;		//Use width*5 in functions.
	int WBValid;
	int *valid;		//width*5
	int state;
	inst * Instruction;

	
}writeBackRegister;

typedef struct executeList {

	int width;
	int EXValid;
	int valid[5];
	int wakeup[5];

	//Need to hold 5 instructions concurrently.
	//So Instruction[0], Instruction[1], Instruction[2]...
	inst * Instruction;

}executeList;



typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
	int change;
	int stall;
	int totalCount;
	int currentCycle;
	int *retireD1;
	int *retireD2;
	int *wbDest;
	rmtBlock *RMT;
	ROBQueue *ROB;
	decodeRegister *DE;
	renameRegister *RN;
	regReadRegister *RR;
	dispatchRegister *DI;
	issueQueue *IQ;
	writeBackRegister *WB;
	

	inst *Instruction;
	

	

}proc_params;

typedef struct execute
{
	executeList **EX;

} execute;


// Put additional data structures here as per your requirement

#endif
