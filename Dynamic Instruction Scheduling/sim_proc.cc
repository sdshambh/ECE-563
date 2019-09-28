// sim_proc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//#include "pch.h"

#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <string>
#include "sim_proc.h"
#include <math.h>
using namespace std;

proc_params params;
execute exe;

void InitialisePipeline(int robSize, int iqSize, int width)
{



	delete[] params.retireD1;
	delete[] params.retireD2;
	delete[] params.wbDest;
	delete[] params.RMT;
	delete[] params.ROB;
	delete[] params.Instruction;
	delete[] params.DE;
	delete[] params.RN;
	delete[] params.RR;
	delete[] params.DI;
	delete[] params.IQ;
	delete[] params.WB;
	delete[] exe.EX;


	/*
	params.retireD1 = (int *)calloc(width, sizeof(int *));
	params.retireD2 = (int *)calloc(width, sizeof(int *));
	params.wbDest = (int *)calloc(width * 5, sizeof(int *));
	params.RMT = (rmtBlock *)calloc(67, sizeof(rmtBlock *));
	params.ROB = (ROBQueue *)calloc(robSize-1, sizeof(ROBQueue *));
	params.Instruction = (inst *)calloc( width, sizeof(inst *));
	params.DE = (decodeRegister *)calloc(width, sizeof(decodeRegister *));
	params.RN = (renameRegister *)calloc(width, sizeof(renameRegister *));
	params.RR = (regReadRegister *)calloc(width, sizeof(regReadRegister *));
	params.DI = (dispatchRegister *)calloc(width, sizeof(dispatchRegister *));
	params.IQ = (issueQueue *)calloc(iqSize, sizeof(issueQueue *));
	params.WB = (writeBackRegister *)calloc(width, sizeof(writeBackRegister *));
	exe.EX = (executeList **)calloc(width, sizeof(executeList **));
*/
//instantiating registers
	params.retireD1 = new int[width * 5];
	params.retireD2 = new int[width * 5];
	params.wbDest = new int[width * 5];
	params.RMT = new rmtBlock[67];
	params.ROB = new ROBQueue[robSize];
	params.Instruction = new inst[width];
	params.DE = new decodeRegister[width];
	params.RN = new renameRegister[width];
	params.RR = new regReadRegister[width];
	params.DI = new dispatchRegister[width];
	params.IQ = new issueQueue[iqSize];
	params.WB = new writeBackRegister[width];
	exe.EX = new executeList*[width];
	for (int i = 0; i < width; i++)
	{
		exe.EX[i] = new executeList[width];
		exe.EX[i]->Instruction = new inst[width * 5];
	}

	for (int i = 0; i < 67; i++)
	{
		params.RMT[i].RMTValidbit = 0;
	}
	for (int i = 0; i < width * 5; i++)
	{
		params.retireD1[i] = -2;
		params.retireD2[i] = -2;
		params.wbDest[i] = -2;
	}
//instatiating all instructions in each registers
	params.DE->DEInstruction = new inst[width];
	params.RN->Instruction = new inst[width];
	params.RR->Instruction = new inst[width];
	params.DI->Instruction = new inst[width];
	params.IQ->Instruction = new inst[width * 5];
	params.IQ->IQ = new iqBlock[iqSize];
	params.IQ->sortedIQ = new iqBlock[iqSize];
	params.WB->Instruction = new inst[width * 5];
	params.IQ->tempInstruction = new inst[width * 5];
	params.ROB->ROB = new robBlock[robSize];

//initialise all valid bits to zero and status as empty.
	params.currentCycle = 0;
	params.change = 1;
	params.stall = 0;
	params.totalCount = 0;
	params.DI->valid = 0;
	params.DI->state = 1;
	params.DI->status = 0;
	params.RN->valid = 0;
	params.RN->status = 0;
	params.RN->state = 1;
	params.RR->valid = 0;
	params.RR->status = 0;
	params.RR->state = 1;
	params.DE->valid = 0;
	params.DE->status = 0;
	params.DE->state = 1;
	params.WB->WBValid = 0;
	params.WB->state = params.WB->WBValid;
	params.WB->valid = new int[width * 5];
	params.ROB->valid = 0;
	for (int i = 0; i <= (robSize - 1); i++)
	{
		params.ROB->ROB[i].valid = 0;
	}
	params.IQ->IQValid = 0;
	params.IQ->state = 1;
	for (int i = 0; i < iqSize; i++)
	{
		params.IQ->IQ[i].valid = 0;
		params.IQ->IQ[i].age = 0;
	}
	for (int i = 0; i < width; i++)
	{
		exe.EX[i]->EXValid = 0;

		for (int j = 0; j < 5; j++)
		{
			exe.EX[i]->wakeup[j] = -2;
			exe.EX[i]->valid[j] = 0;
		}
	}
	for (int i = 0; i < width*5; i++)
	{
		
			params.WB->valid[i] = 0;
		
	}
}
void initialiseInstruction(int k)
{
	//initialise all start and end cycle for each register as zero.
	params.Instruction[k].initFE = 0;
	params.Instruction[k].timeFE = 0;
	params.Instruction[k].initDE = 0;
	params.Instruction[k].timeDE = 0;
	params.Instruction[k].initRN = 0;
	params.Instruction[k].timeRN = 0;
	params.Instruction[k].initRR = 0;
	params.Instruction[k].timeRR = 0;
	params.Instruction[k].initDI = 0;
	params.Instruction[k].timeDI = 0;
	params.Instruction[k].initIS = 0;
	params.Instruction[k].timeIS = 0;
	params.Instruction[k].initEX = 0;
	params.Instruction[k].timeEX = 0;
	params.Instruction[k].initWB = 0;
	params.Instruction[k].timeWB = 0;
	params.Instruction[k].initRT = 0;
	params.Instruction[k].timeRT = 0;
	params.Instruction[k].src1Ready = 0;
	params.Instruction[k].src2Ready = 0;

}

void instructiontype(int type, int k)
{
	//Assigns number of execute cycles according to optype
	if (type == 0)
	{
		params.Instruction[k].no_of_cycle = 1;
	}
	else if (type == 1)
	{
		params.Instruction[k].no_of_cycle = 2;
	}
	else if (type == 2)
	{
		params.Instruction[k].no_of_cycle = 5;
	}
}
void fwdtoDE(inst *Instruction)
{
	//forwards whole bundle of instruction from fetch to decode stage.
	for (int i = 0; i < params.width; i++)
	{
		params.DE->DEInstruction[i] = Instruction[i];
	}
//Make decode stage status as full and set valid bit to 1
	params.DE->status = 1;
	params.DE->valid = 1;
	//delete Instruction;
	return;
}
void fwdtoRN(inst *Instruction)
{
	//forwards whole bundle of instruction from decode to register rename stage.
	for (int i = 0; i < params.width; i++)
	{
		params.RN->Instruction[i] = Instruction[i];
	}
	//Make rename stage status as full and set valid bit to 1
	params.RN->valid = 1;
	params.RN->status = 1;
	//delete Instruction;
}

int enqueue(robBlock *data)
{
	//check if rob is empty
	if (((params.ROB->head == 0) && (params.ROB->tail == (params.rob_size - 1))) || (params.ROB->head == params.ROB->tail + 1))
	{
		//delete data;
		return 0;
	}
	else if ((params.ROB->head == -1) && (params.ROB->tail == -1))
	{
		//initialise head and tail in 1st cycle
		params.ROB->head = 0;
		params.ROB->tail = 0;
		params.ROB->ROB[params.ROB->tail] = *data;
		params.ROB->ROB[params.ROB->tail].valid = 1;
		params.ROB->count++;
	}
	else if (params.ROB->tail == (params.rob_size - 1))
	{
		//applying circular FIFO if tail reaches the robsize
		params.ROB->tail = 0;
		params.ROB->ROB[params.ROB->tail] = *data;
		params.ROB->ROB[params.ROB->tail].valid = 1;
		params.ROB->count++;
	}
	else
	{
		//update data in ROB and increment tail.
		params.ROB->tail++;
		params.ROB->ROB[params.ROB->tail] = *data;
		params.ROB->ROB[params.ROB->tail].valid = 1;
		params.ROB->count++;
	}
	//delete data;
	return 1;
}
void fwdtoRR(inst *Instruction)
{
	//forwards whole bundle of instruction from rename to register read stage.
	for (int i = 0; i < params.width; i++)
	{
		params.RR->Instruction[i] = Instruction[i];
	}
//Make register read stage status as full and set valid bit to 1
	params.RR->status = 1;
	params.RR->valid = 1;
	//delete Instruction;

}
void fwdtoDI(inst * Instruction)
{
	//forwards whole bundle of instruction from register read stage to dispatch stage.
	for (int i = 0; i < params.width; i++)
	{
		params.DI->Instruction[i] = Instruction[i];
	}
//Make dispatch stage status as full and set valid bit to 1
	params.DI->status = 1;
	params.DI->valid = 1;
	//delete Instruction;
}
int IQEntryStatus()
{
	//check if any istruction can be issued
	int count = 0;
	for (int i = 0; i < params.iq_size; i++)
	{
		if (params.IQ->IQ[i].valid == 0)
			count++;
	}
	if (count >= params.width)
		return 0;
	else return 1;

}
void FwdtoIQ(inst * Instruction)
{
	//forwards whole bundle of instruction from dispatch to issue stage.
	int counter = params.width;
	int i = 0;
	params.IQ->IQValid = 1;


	do
	{
		for (int j = 0; j < params.iq_size; j++)
		{
			if (params.IQ->IQ[j].valid == 0)
			{
				params.IQ->cage++;
				params.IQ->IQ[j].valid = 1;
				params.IQ->IQ[j].age = params.IQ->cage;
				params.IQ->IQ[j].Instruction.src1Ready = Instruction[i].src1Ready;
				params.IQ->IQ[j].Instruction.src2Ready = Instruction[i].src2Ready;

				params.IQ->IQ[j].Instruction = *(Instruction + i);
				counter--;
				i++;
			}

			if (counter == 0)
				break;
		}

	} while (counter != 0);
	//delete Instruction;
	return;
}
void dequeueROB()
{
	//remove all the instruction from rob which are moved to arf 
	if (params.ROB->head == -1 && params.ROB->tail == -1) {
		//cout << "ROB is empty\n";
	}
	else {
		if (params.ROB->head == params.ROB->tail) {
			params.ROB->ROB[params.ROB->head].valid = 0;
			params.ROB->ROB[params.ROB->head].ready = 0;
			params.ROB->head = -1;
			params.ROB->tail = -1;
			params.ROB->count--;
		}
		else if (params.ROB->head == (params.rob_size - 1)) {
			params.ROB->ROB[params.ROB->head].valid = 0;
			params.ROB->ROB[params.ROB->head].ready = 0;
			params.ROB->head = 0;
			params.ROB->count--;
		}
		else {

			params.ROB->ROB[params.ROB->head].valid = 0;
			params.ROB->ROB[params.ROB->head].ready = 0;
			params.ROB->head++;
			params.ROB->count--;
		}
	}

	return;
}
void fwdtoWB(inst *Instruction)
{
	for (int i = params.width * 5 - 2; i >= 0; i--)
	{
		params.WB->Instruction[i + 1] = params.WB->Instruction[i];
		//Mark instruction as valid in WB pipeline register.		
		params.WB->valid[i + 1] = params.WB->valid[i];
	}

	params.WB->Instruction[0] = *(Instruction + 0);
	params.WB->valid[0] = 1;

	params.WB->WBValid = 1;
	//delete Instruction;
}

void IssueQueueSorting()
{
	//bubble sort all the issue queue data
	iqBlock temp;
	for (int j = 0; j < params.iq_size; j++)
	{
		params.IQ->sortedIQ[j] = params.IQ->IQ[j];
	}

	for (int j = 0; j < params.iq_size - 1; j++)
	{
		for (int i = 0; i < params.iq_size - j - 1; i++)
		{
			if (params.IQ->sortedIQ[i + 1].age < params.IQ->sortedIQ[i].age)
			{
				temp = params.IQ->sortedIQ[i];
				params.IQ->sortedIQ[i] = params.IQ->sortedIQ[i + 1];
				params.IQ->sortedIQ[i + 1] = temp;
			}
		}
	}

	for (int j = 0; j < params.iq_size; j++)
	{
		params.IQ->IQ[j] = params.IQ->sortedIQ[j];
	}
	return;
}
int ROBstatus()
{
	//return 1 when rob is full else zero.
	int a, b;

	if (((params.ROB->head == 0) && (params.ROB->tail == (params.rob_size - 1))) || (params.ROB->head == params.ROB->tail + 1)) {

		return 1;
	}

	if (params.ROB->head == -1 && params.ROB->tail == -1) {

		return 0;
	}

	if (params.ROB->tail > params.ROB->head)
	{
		a = (params.rob_size - 1) - params.ROB->tail;
		b = params.ROB->head;
		if (a + b >= params.width)
		{

			return 0;
		}
		else
		{

			return 1;
		}
	}

	if (params.ROB->tail < params.ROB->head)
	{
		if ((params.ROB->head - params.ROB->tail - 1) >= params.width)
		{

			return 0;
		}
		else
		{
			return 1;
		}
	}


	return 0;
}
int Advance_Cycle()
{
	params.currentCycle++;
	//delete[] exe.EX;
	//Check if pipeline is empty i.e. ROB has been written to the ARF. (No more instructions in Retire stage)
	if (params.currentCycle > 2 && ROBstatus() == 0)
		return 1;
	else
		return 0;

}

void Retire()
{
	int Counter = 0;
	//check if retire is vaid-i.e has data
	if (params.ROB->valid == 1)
	{


		for (int i = 0; i < (params.rob_size - 1); i++)
		{
			if (params.ROB->ROB[params.ROB->head].valid == 1 && params.ROB->ROB[params.ROB->head].ready == 1)
			{
				//Retire if the valid bit is 1 and it's ready to retire
				params.ROB->ROB[params.ROB->head].Instr.timeRT = params.currentCycle + 1 - params.ROB->ROB[params.ROB->head].Instr.initRT;
				Counter++;

				//if RMT has latest tag then make the RMT vakid bit zero. ARF has updated data
				if (params.ROB->ROB[params.ROB->head].Instr.robTag == params.RMT[params.ROB->ROB[params.ROB->head].Instr.dest].tag)
				{
					params.RMT[params.ROB->ROB[params.ROB->head].Instr.dest].RMTValidbit = 0;
				}
// Then print the data for that instruction
				if (params.ROB->ROB[params.ROB->head].Instr.sequence != -1)
				{
					//Print stats
					cout
						<< params.ROB->ROB[params.ROB->head].Instr.sequence << " "
						<< "fu{" << params.ROB->ROB[params.ROB->head].Instr.type << "} "
						<< "src{" << params.ROB->ROB[params.ROB->head].Instr.src1 << "," << params.ROB->ROB[params.ROB->head].Instr.src2 << "} "
						<< "dst{" << params.ROB->ROB[params.ROB->head].Instr.dest << "} "
						<< "FE{" << params.ROB->ROB[params.ROB->head].Instr.initFE << "," << params.ROB->ROB[params.ROB->head].Instr.timeFE << "} "
						<< "DE{" << params.ROB->ROB[params.ROB->head].Instr.initDE << "," << params.ROB->ROB[params.ROB->head].Instr.timeDE << "} "
						<< "RN{" << params.ROB->ROB[params.ROB->head].Instr.initRN << "," << params.ROB->ROB[params.ROB->head].Instr.timeRN << "} "
						<< "RR{" << params.ROB->ROB[params.ROB->head].Instr.initRR << "," << params.ROB->ROB[params.ROB->head].Instr.timeRR << "} "
						<< "DI{" << params.ROB->ROB[params.ROB->head].Instr.initDI << "," << params.ROB->ROB[params.ROB->head].Instr.timeDI << "} "
						<< "IS{" << params.ROB->ROB[params.ROB->head].Instr.initIS << "," << params.ROB->ROB[params.ROB->head].Instr.timeIS << "} "
						<< "EX{" << params.ROB->ROB[params.ROB->head].Instr.initEX << "," << params.ROB->ROB[params.ROB->head].Instr.timeEX << "} "
						<< "WB{" << params.ROB->ROB[params.ROB->head].Instr.initWB << "," << params.ROB->ROB[params.ROB->head].Instr.timeWB << "} "
						<< "RT{" << params.ROB->ROB[params.ROB->head].Instr.initRT << "," << params.ROB->ROB[params.ROB->head].Instr.timeRT << "} "
						<< endl;
//remove the entry from the rob i.e. make the valid bit a zero in dequeue function
					if (params.ROB->ROB[params.ROB->head].Instr.sequence == 9999)
					{
						params.totalCount = params.ROB->ROB[params.ROB->head].Instr.initRT + params.ROB->ROB[params.ROB->head].Instr.timeRT;
					}
				}

				dequeueROB();

			}
			//retire only width size instructions
			if (Counter == params.width)
				break;
		}
	}

}

void Writeback()
{
	// checks the cycle which it is gonna write back for
	params.change = params.change * (-1);

	if (params.WB->WBValid)
	{
		for (int i = 0; i < params.width * 5; i++)
		{
			if (params.WB->valid[i] == 1)
			{
				// if valid bit is one the send the data to retire and calculate the cycle time in writeback stage
				params.WB->valid[i] = 0;
				params.WB->Instruction[i].initRT = params.currentCycle + 1;
				params.WB->Instruction[i].timeWB = params.currentCycle + 1 - params.WB->Instruction[i].initWB;
				//change the dest register according to the cycle
				if (params.change == 1)
				{
					params.retireD1[i] = params.WB->Instruction[i].destReg;
				}

				if (params.change == -1)
				{
					params.retireD2[i] = params.WB->Instruction[i].destReg;
				}

				params.wbDest[i] = params.WB->Instruction[i].destReg;

				params.ROB->valid = 1;
				//make the ready bit 1 for the retire stage
				params.ROB->ROB[params.WB->Instruction[i].robTag].valid = 1;
				params.ROB->ROB[params.WB->Instruction[i].robTag].ready = 1;
				params.ROB->ROB[params.WB->Instruction[i].robTag].Instr = params.WB->Instruction[i];
			}
			else
				params.wbDest[i] = -2;
		}



	}


}

void Execute()
{
	if (exe.EX[0]->EXValid == 1)
	{
		for (int k = 0; k < params.width; k++)
		{
			
			for (int i = 0; i < 5; i++)
			{

				if (exe.EX[k]->EXValid == 1)
				{

					if (exe.EX[k]->valid[i] == 1)
					{
						//forward instruction to WB stage
						exe.EX[k]->Instruction[i].no_of_cycle--;
						if (exe.EX[k]->Instruction[i].no_of_cycle == 0)     //Send to WB
						{
							exe.EX[k]->valid[i] = 0;
							exe.EX[k]->wakeup[i] = exe.EX[k]->Instruction[i].destReg;
//calculate cycles for execute stage
							exe.EX[k]->Instruction[i].initWB = params.currentCycle + 1;
							exe.EX[k]->Instruction[i].timeEX = params.currentCycle + 1 - exe.EX[k]->Instruction[i].initEX;
							fwdtoWB(&(exe.EX[k]->Instruction[i]));
							//Max width*5 instruction can be woken up.
							//Also send over bypass to wakeup instructions in RR, DI and IQ.
							
							

						}
						else exe.EX[k]->wakeup[i] = -2;
					}
					else exe.EX[k]->wakeup[i] = -2;

				}

			}
		}

	}

}



void Issue()
{
	if (params.IQ->state == 1)
	{

		for (int i = 0; i < params.iq_size; i++)
		{
			if (params.IQ->IQ[i].valid == 1)
			{
				for (int j = 0; j < params.width; j++)
				{
					for (int k = 0; k < 5; k++)
					{
						//if there is data for src1 then make the ready bit 1 else keep it zero and give wakeup when it's ready
						if ((exe.EX[j]->wakeup[k] == params.IQ->IQ[i].Instruction.srcReg1) || (params.IQ->IQ[i].Instruction.srcReg1 == -1))
						{
							params.IQ->IQ[i].Instruction.src1Ready = 1;
						}
					}
				}
			}
		}

		for (int i = 0; i < params.iq_size; i++)
		{
			if (params.IQ->IQ[i].valid == 1)
			{
				for (int j = 0; j < params.width; j++)
				{
					for (int k = 0; k < 5; k++)
					{
						//if there is data for src2 then make the ready bit 1 else keep it zero and give wakeup when it's ready
						if ((exe.EX[j]->wakeup[k] == params.IQ->IQ[i].Instruction.srcReg2) || (params.IQ->IQ[i].Instruction.srcReg2 == -1))
						{
							params.IQ->IQ[i].Instruction.src2Ready = 1;
						}
					}
				}
			}
		}

		for (int i = 0; i < params.iq_size; i++)
		{
			if (params.IQ->IQ[i].valid == 1)
			{
				if (params.ROB->ROB[params.IQ->IQ[i].Instruction.srcReg1].ready == 1)
				{
					params.IQ->IQ[i].Instruction.src1Ready = 1;
				}
			}
		}

		for (int i = 0; i < params.iq_size; i++)
		{
			if (params.IQ->IQ[i].valid == 1)
			{
				if (params.ROB->ROB[params.IQ->IQ[i].Instruction.srcReg2].ready == 1)
				{
					params.IQ->IQ[i].Instruction.src2Ready = 1;
				}
			}
		}

		for (int i = 0; i < params.iq_size; i++)
		{
			for (int k = 0; k < params.width * 5; k++)
			{
				if (params.wbDest[k] == params.IQ->IQ[i].Instruction.srcReg1)
				{
					params.IQ->IQ[i].Instruction.src1Ready = 1;
				}
			}
		}

		for (int i = 0; i < params.iq_size; i++)
		{
			for (int k = 0; k < params.width * 5; k++)
			{
				if (params.wbDest[k] == params.IQ->IQ[i].Instruction.srcReg2)
				{
					params.IQ->IQ[i].Instruction.src2Ready = 1;
				}
			}
		}

//sort issue queue and send the wakeup calls.
		IssueQueueSorting();
		int count = 0;
		int p = 0;
		for (int i = 0; i < params.iq_size; i++)
		{
			if ((params.IQ->IQ[i].valid == 1) && (params.IQ->IQ[i].Instruction.src1Ready == 1) && (params.IQ->IQ[i].Instruction.src2Ready == 1))
			{

				params.IQ->tempInstruction[p] = params.IQ->IQ[i].Instruction;
				p++;
				count++;
				params.IQ->IQ[i].valid = 0;
			}

			if (count == params.width)
				break;
		}

		params.IQ->issueNum = count;


		for (int i = 0; i < params.IQ->issueNum; i++)
		{
			params.IQ->tempInstruction[i].initEX = params.currentCycle + 1;
			params.IQ->tempInstruction[i].timeIS = params.currentCycle + 1 - params.IQ->tempInstruction[i].initIS;
			for (int j = 3; j >= 0; j--)
			{
				exe.EX[i]->Instruction[j + 1] = exe.EX[i]->Instruction[j];
				exe.EX[i]->valid[j + 1] = exe.EX[i]->valid[j];
			}
//frwd the data to execute stage
			exe.EX[i]->Instruction[0] = params.IQ->tempInstruction[i];
			exe.EX[i]->valid[0] = 1;
			exe.EX[i]->EXValid = 1;
			//delete params.IQ->IQ->tempInstruction;
		}


	}
}

void Dispatch()
{
	if (params.DI->state == 1)
	{

		if (params.DI->status == 1)
		{
			for (int i = 0; i < params.width; i++)
			{
				//if disptach has data to it for src1
				if (params.DI->Instruction[i].srcReg1 != -1)
				{
					for (int j = 0; j < params.width; j++)
					{
						for (int k = 0; k < 5; k++)
						{
							if (exe.EX[j]->wakeup[k] == params.DI->Instruction[i].srcReg1)
							{
								params.DI->Instruction[i].src1Ready = 1;
							}
						}
					}
				}

				//if disptach has data to it for src2
				if (params.DI->Instruction[i].srcReg2 != -1)
				{
					for (int j = 0; j < params.width; j++)
					{
						for (int k = 0; k < 5; k++)
						{
							if (exe.EX[j]->wakeup[k] == params.DI->Instruction[i].srcReg2)
							{
								params.DI->Instruction[i].src2Ready = 1;
							}
						}
					}
				}


				//See if it's raedy to dispach in ROB table. if yes make the readybit 1
				if (params.ROB->ROB[params.DI->Instruction[i].srcReg1].ready == 1)
				{
					params.DI->Instruction[i].src1Ready = 1;
				}

				if (params.ROB->ROB[params.DI->Instruction[i].srcReg2].ready == 1)
				{
					params.DI->Instruction[i].src2Ready = 1;
				}

			}


			//For src1 from write back stage give wakeup
			for (int i = 0; i < params.width; i++)
			{
				for (int k = 0; k < params.width * 5; k++)
				{
					if (params.wbDest[k] == params.DI->Instruction[i].srcReg1)
					{
						params.DI->Instruction[i].src1Ready = 1;
					}
				}
			}

			//do same for src2
			for (int i = 0; i < params.width; i++)
			{
				for (int k = 0; k < params.width * 5; k++)
				{
					if (params.wbDest[k] == params.DI->Instruction[i].srcReg2)
					{
						params.DI->Instruction[i].src2Ready = 1;
					}
				}
			}


			//Check number of free entries in IQ before sending.
			if (IQEntryStatus() == 0)
			{

				for (int i = 0; i < params.width; i++)
				{
					params.DI->Instruction[i].initIS = params.currentCycle + 1;
					params.DI->Instruction[i].timeDI = params.currentCycle + 1 - params.DI->Instruction[i].initDI;
				}
				//forward the data to issue queue if theer is space for the bundle.
				FwdtoIQ(params.DI->Instruction);
				params.DI->status = 0;
				params.DI->valid = 0;

			}
		}

	}
}

void RegRead()
{

	if (params.RR->state == 1)
	{
		if (params.RR->status == 1)
		{
			//if the status of RR is empty then set the bits to ready if the source and destination are there
			for (int i = 0; i < params.width; i++)
			{
				if (params.RR->Instruction[i].srcReg1 == -1)
				{
					params.RR->Instruction[i].src1Ready = 1;
				}
				else
				{
					if (params.ROB->ROB[params.RR->Instruction[i].srcReg1].ready == 1)
					{
						params.RR->Instruction[i].src1Ready = 1;
					}
					switch (params.change)
					{
					case -1:
						for (int k = 0; k < params.width * 5; k++)
						{
							if (params.RR->Instruction[i].srcReg1 == params.retireD1[k])
							{
								params.RR->Instruction[i].src1Ready = 1;
							}
						}
						break;
					case 1:
						for (int k = 0; k < params.width * 5; k++)
						{
							if (params.RR->Instruction[i].srcReg1 == params.retireD2[k])
							{
								params.RR->Instruction[i].src1Ready = 1;
							}
						}
						break;
					}
					for (int j = 0; j < params.width; j++)
					{
						for (int k = 0; k < 5; k++)
						{
							if (exe.EX[j]->wakeup[k] == params.RR->Instruction[i].srcReg1)
							{
								params.RR->Instruction[i].src1Ready = 1;
							}
						}
					}
					for (int j = 0; j < params.width; j++)
					{
						for (int k = 0; k < 5; k++)
						{
							if (exe.EX[j]->wakeup[k] == params.RR->Instruction[i].srcReg1)
							{
								params.RR->Instruction[i].src1Ready = 1;
							}
						}
					}

					for (int k = 0; k < params.width * 5; k++)
					{
						if (params.wbDest[k] == params.RR->Instruction[i].srcReg1)
						{
							params.RR->Instruction[i].src1Ready = 1;
						}
					}

					//ROB
					if (params.ROB->ROB[params.RR->Instruction[i].srcReg1].ready == 1)
					{
						params.RR->Instruction[i].src1Ready = 1;
					}
				}

				if (params.RR->Instruction[i].srcReg2 == -1)
				{
					params.RR->Instruction[i].src2Ready = 1;
				}
				else
				{

					if ((params.ROB->ROB[params.RR->Instruction[i].srcReg2].ready == 1))
					{
						params.RR->Instruction[i].src2Ready = 1;
					}

					//Wakeup from Retire stage.
					switch (params.change)
					{
					case 1:
						for (int k = 0; k < params.width * 5; k++)
						{
							if (params.RR->Instruction[i].srcReg2 == params.retireD1[k])
							{
								params.RR->Instruction[i].src2Ready = 1;
							}
						}
						break;
					case -1:
						for (int k = 0; k < params.width * 5; k++)
						{
							if (params.RR->Instruction[i].srcReg2 == params.retireD2[k])
							{
								params.RR->Instruction[i].src2Ready = 1;
							}
						}
						break;
					}

					//Check for wakeup from Execute stage.
					for (int j = 0; j < params.width; j++)
					{
						for (int k = 0; k < 5; k++)
						{
							if (exe.EX[j]->wakeup[k] == params.RR->Instruction[i].srcReg2)
							{
								params.RR->Instruction[i].src2Ready = 1;
							}
						}
					}

					//check wakeup from Writeback stage;
					for (int k = 0; k < params.width * 5; k++)
					{
						if (params.wbDest[k] == params.RR->Instruction[i].srcReg2)
						{
							params.RR->Instruction[i].src2Ready = 1;
						}
					}

				}

			}

		}    
		//if Dispatch is empty send the instruction bundle to dispatch stage.
		if (params.DI->status == 0 || params.DI->state == 0)
		{
			if (params.RR->status == 1)
			{
				//calculate teh cycle time in register read for the instruction
				for (int i = 0; i < params.width; i++)
				{
					params.RR->Instruction[i].initDI = params.currentCycle + 1;
					params.RR->Instruction[i].timeRR = params.currentCycle + 1 - params.RR->Instruction[i].initRR;
				}

				fwdtoDI(params.RR->Instruction);
				params.RR->status = 0;

			}
		}
	}

}


void Rename()
{
	if (params.RN->state == 1)
	{
		if (params.RN->status == 1)
		{
			//if the remname state is valid and is rob has space enqueue data in ROB
			if ((params.RR->status == 0) && (ROBstatus() == 0))
			{
				for (int i = 0; i < params.width; i++)
				{
					robBlock *data;
					data = new robBlock;
					data->ready = 0;
					data->valid = 1;
					data->Instr = *(params.RN->Instruction + i);
					data->tag = (params.ROB->tail + 1);
					enqueue(data);

					if (params.RN->Instruction[i].srcReg1 != -1)
					{
						if (params.RMT[params.RN->Instruction[i].src1].RMTValidbit == 1)
						{
							params.RN->Instruction[i].srcReg1 = params.RMT[params.RN->Instruction[i].src1].tag;
							params.RN->Instruction[i].register_renamed1 = 1;
						}
						else
						{
							params.RN->Instruction[i].src1Ready = 1;
						}

					}
					else
					{
						params.RN->Instruction[i].src1Ready = 1;
					}
					//Rename the RMTtags to the rob tags and registers with RMT tags-src2
					if (params.RN->Instruction[i].srcReg2 != -1)
					{
						if (params.RMT[params.RN->Instruction[i].src2].RMTValidbit == 1)
						{
							params.RN->Instruction[i].srcReg2 = params.RMT[params.RN->Instruction[i].src2].tag;
							params.RN->Instruction[i].register_renamed2 = 1;
						}
						else
						{
							params.RN->Instruction[i].src2Ready = 1;
						}

					}
					else
					{
						params.RN->Instruction[i].src2Ready = 1;
					}
					if (params.RN->Instruction[i].destReg != -1)
					{
						params.RMT[params.RN->Instruction[i].dest].tag = params.ROB->tail;
						params.RMT[params.RN->Instruction[i].dest].RMTValidbit = 1;
						params.RN->Instruction[i].robTag = params.ROB->tail;

					}
					else
					{
						params.RN->Instruction[i].robTag = params.ROB->tail;

					}
					//Rename the RMTtags to the rob tags and registers with RMT tags-destination register
					if (params.RN->Instruction[i].destReg != -1)
					{
						if (params.RMT[params.RN->Instruction[i].dest].RMTValidbit == 1)
						{
							params.RN->Instruction[i].destReg = params.RMT[params.RN->Instruction[i].dest].tag;
						}
					}


					
					if (params.ROB->ROB[params.RN->Instruction[i].srcReg1].ready == 1)
						params.RN->Instruction[i].src1Ready = 1;
					if (params.ROB->ROB[params.RN->Instruction[i].srcReg2].ready == 1)
						params.RN->Instruction[i].src2Ready = 1;
				}
				//calculate the no of cycle instruction stalled in rename stage
				for (int i = 0; i < params.width; i++)
				{
					params.RN->Instruction[i].initRR = params.currentCycle + 1;
					params.RN->Instruction[i].timeRN = params.currentCycle + 1 - params.RN->Instruction[i].initRN;
				}
//forward the data to register rename if RR is valid and empty
				fwdtoRR(params.RN->Instruction);
				params.RN->status = 0;

			}

		}

	}

}

void Decode()
{
	if (params.DE->state == 1)
	{
		if (params.DE->status == 1)
		{
			//if decode is valid and has data, check if RN is valid and empty 
			params.RN->valid = 1;
			if (params.RN->status == 0 && params.RN->valid == 1)
			{
				for (int i = 0; i < params.width; i++)
				{
					params.DE->DEInstruction[i].initRN = params.currentCycle + 1;
					params.DE->DEInstruction[i].timeDE = params.currentCycle + 1 - params.DE->DEInstruction[i].initDE;
				}
//then forward data to rename stage
				fwdtoRN(params.DE->DEInstruction);
				params.DE->status = 0;
			}

		}
	}

}

void Fetch()
{
	int flag = 0;
	for (int i = 0; i < params.width; i++)
	{
		//set current cycle as teh base for calculating duration instruction stayed in fetch
		params.Instruction[i].initFE = params.currentCycle;
	}

	if (params.currentCycle % 2 == 1) 
	{
		
		for (int i = 0; i < params.width * 5; i++)
		{
			params.retireD2[i] = -2;
		}
	}
	else
		
	{
		for (int i = 0; i < params.width * 5; i++)
		{
			params.retireD1[i] = -2;
		}

	}

	if (params.DE->status == 0)
	{
		params.stall = 0;

		for (int i = 0; i < params.width; i++)
		{
			//calculate the duration instruction stalled in fetch
			params.Instruction[i].initDE = params.currentCycle + 1;
			params.Instruction[i].timeFE = params.currentCycle + 1 - params.Instruction[i].initFE;
		}
		for (int k = 0; k < params.width; k++)
		{
			if (params.Instruction[k].sequence == -1)
				flag = flag + 1;
		}
		//if decode is empty and valid forward the instruction bundle to decode stage
		if (flag == 0)
		{
			fwdtoDE(params.Instruction);
		}
		else if (flag < params.width)
		{
			fwdtoDE(params.Instruction);
		}
		else if (flag == params.width)
		{
			//Nothing
		} 
	}
	else if (params.DE->status == 1 && params.currentCycle > 2)
	{
		params.stall = 1;
	}
	else params.stall = 0;

	return;

}

int Simulation()
{

	Retire();

	Writeback();

	Execute();

	Issue();

	Dispatch();

	RegRead();

	Rename();

	Decode();

	Fetch();

	if (params.stall == 1)
		return 1;
	else
		return 0;
}




int main(int argc, char *argv[])
{
	int Stall = 0;//setting invalid
	int complete = 0;//setting invalid
	int sequence = 0;
	string sequenceLine;            // File handler
	ifstream trace_file;       // Variable that holds trace file name;
		 // look at sim_bp.h header file for the the definition of struct proc_params
	int op_type, dest, src1, src2;  // Variables are read from trace file
	unsigned long int pc; // Variable holds the pc read from input file
	//string filename;
	//filename = "val_trace_gcc1.txt";
	if (argc != 5)
	{
		printf("Error: Wrong number of inputs:%d\n", argc - 1);
		exit(EXIT_FAILURE);
	}

	params.rob_size = strtoul(argv[1], NULL, 10);
	params.iq_size = strtoul(argv[2], NULL, 10);
	params.width = strtoul(argv[3], NULL, 10);
	//trace_file = argv[4];
	/*params.rob_size = 16;
	params.iq_size = 8;
	params.width = 1;*/
	InitialisePipeline(params.rob_size, params.iq_size, params.width);
	//const char *filename ="val_trace_gcc1";
	//cout << "Start of simulation";
	/*printf("rob_size:%lu "
		"iq_size:%lu "
		"width:%lu "
		"tracefile:%s\n", params.rob_size, params.iq_size, params.width,filename);
		*/

		// Open trace_file in read mode
		/*FP = fopen(trace_file, "r");
		if (FP == NULL)
		{
			// Throw error and exit if fopen() failed
			printf("Error: Unable to open file %s\n", trace_file);
			exit(EXIT_FAILURE);
		}
		*/
	trace_file.open(argv[4]);


	do {
		if (Stall == 0)
		{
			for (int k = 0; k < params.width; k++)
			{
				if ((!trace_file.eof()) && getline(trace_file, sequenceLine))
				{
					initialiseInstruction(k);

					istringstream trace(sequenceLine);
					trace >> hex >> pc >> dec >> op_type >> dest >> src1 >> src2;
					params.Instruction[k].destReg = params.Instruction[k].dest = dest;
					params.Instruction[k].srcReg1 = params.Instruction[k].src1 = src1;
					params.Instruction[k].srcReg2 = params.Instruction[k].src2 = src2;
					params.Instruction[k].type = op_type;
					params.Instruction[k].pc = pc;
					params.Instruction[k].sequence = sequence;
					if (params.Instruction[k].sequence == 0)
					{
						params.ROB->head = -1;
						params.ROB->tail = -1;
						params.ROB->count = 0;
					}
					instructiontype(params.Instruction[k].type, k);
					sequence++;

				}

				else if (trace_file.eof())
				{
					params.Instruction[k].sequence = -1;
					params.Instruction[k].destReg = -2;
					params.Instruction[k].srcReg1 = -1;
					params.Instruction[k].srcReg2 = -1;

				}
			}

			Stall = Simulation();
			complete = Advance_Cycle();

			if (complete == 1)
			{
				//Retire();
			}
		}

		else
		{
			Stall = Simulation();
			complete = Advance_Cycle();

		}


	} while ((!trace_file.eof()) || (complete == 0)  || (params.ROB->head!=-1));

	trace_file.close();
	params.ROB->head = params.ROB->head - 1;
	//while (trace_file >> hex >> pc >> dec >> op_type >> dest >> src1 >> src2)
	//{

	//	printf("%lx %d %d %d %d\n", pc, op_type, dest, src1, src2); //Print to check if inputs have been read correctly
	//	/*************************************
	//		Add calls to OOO simulator here
	//	**************************************/
	//}
	cout << "# === Simulator Command =========" << '\n';
	cout << "# " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << '\n';
	cout << "# === Processor Configuration ===" << "\n";
	cout << "# ROB_SIZE = " << params.rob_size << "\n"
		<< "# IQ_SIZE  = " << params.iq_size << "\n"
		<< "# WIDTH    = " << params.width << endl;

	streamsize default_prec = cout.precision();
	//float ipc = 0;
	//ipc = (float)(sequence)/(float)(processor.totalCount);

	cout << "# === Simulation Results ========" << '\n';
	cout << "# Dynamic Instruction Count    = " << sequence << '\n';
	cout << "# Cycles                       = " << params.totalCount << '\n';
	//printf("# Instructions per Cycle (IPC) = %0.2f\n", ipc);
	cout.precision(2);
	cout.setf(ios::fixed, ios::floatfield);
	cout << "# Instructions Per Cycle (IPC) = " << (float)(sequence) / (float)(params.totalCount) << '\n';
	cout.unsetf(ios::floatfield);
	cout.precision(default_prec);
	//return 0;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
