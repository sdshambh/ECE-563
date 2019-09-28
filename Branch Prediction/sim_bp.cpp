
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <cstring>
#include "sim_bp.h"
#include <math.h>
bp_params *bimodal = NULL, *gshare = NULL;
bp_hybrid *hybrid = NULL;
using namespace std;
void print(bp_params * bp,int c)
{
	float miss_rate = (float)((float)(100 * bp->mispredictions) / (float)(bp->predictions));
	cout << "OUTPUT" << endl;
	cout << "number of predictions: " << bp->predictions << endl;
	cout << "number of mispredictions: " << bp->mispredictions << endl;
	cout.precision(2);
	cout.setf(ios::fixed, ios::floatfield);
	cout << "misprediction rate: " << miss_rate << "%" << endl;
	cout.unsetf(ios::floatfield);
	if(c == 0)
	cout << "FINAL\t" << "BIMODAL CONTENTS" << endl;
	else if (c == 1)
		cout << "FINAL\t" << "GSHARE CONTENTS" << endl;

	for (int i = 0; i < bp->table_rows; i++)
	{
		cout << i << '\t' << bp->table[i] << endl;
	}
}


void Hybridprint(bp_hybrid * bp)
{
	float miss_rate;
	miss_rate = (float)((float)(100 * bp->mispredictions) / (float)(bp->predictions));
	cout << "OUTPUT" << endl;
	cout << "number of predictions: " << bp->predictions << endl;
	cout << "number of mispredictions: " << bp->mispredictions << endl;
	cout.precision(2);
	cout.setf(ios::fixed, ios::floatfield);
	cout << "misprediction rate: " << miss_rate << "%" << endl;
	cout.unsetf(ios::floatfield);
	cout << "FINAL\t" << "CHOOSER CONTENTS" << endl;

	for (int i = 0; i < bp->tablerows_hybrid; i++)
	{
		cout << i << '\t' << bp->table_hybrid[i] << endl;
	}

	cout << "FINAL\t" << "GSHARE CONTENTS" << endl;
	for (int i = 0; i < bp->tablerows_gshare; i++)
	{
		cout << i << '\t' << bp->table_gshare[i] << endl;
	}

	cout << "FINAL\t" << "BIMODAL CONTENTS" << endl;
	for (int i = 0; i < bp->tablerows_bimodal; i++)
	{
		cout << i << '\t' << bp->table_bimodal[i] << endl;
	}
}


void initialize_table(bp_params * bp)
{
	for (int i = 0; i < bp->table_rows; i++)
	{
		bp->table[i] = 2;
	}

	//Branch history register init to 0.
	bp->BHR_table = 0;
}

void Hinitialize(bp_hybrid * bp)
{
	for (int i = 0; i < bp->tablerows_bimodal; i++)
	{
		bp->table_bimodal[i] = 2;
	}

	for (int i = 0; i < bp->tablerows_gshare; i++)
	{
		bp->table_gshare[i] = 2;
	}

	for (int i = 0; i < bp->tablerows_hybrid; i++)
	{
		bp->table_hybrid[i] = 1;
	}

	//Branch history register init to 0.
	bp->h_BHR_table = 0;
}

void init_HPredictor(bp_hybrid * bp , int k, int m1, int n, int m2)
{
	bp->h_index_bits_gshare = m1;
	bp->h_BHR_bits = n;
	bp->h_index_bits_bimodal = m2;
	bp->h_chooser_bits = k;

	bp->tablerows_bimodal = pow(2, bp->h_index_bits_bimodal);
	bp->tablerows_gshare = pow(2, bp->h_index_bits_gshare);
	bp->tablerows_hybrid = pow(2, bp->h_chooser_bits);

	bp->table_bimodal = new int[bp->tablerows_bimodal];
	bp->table_gshare = new int[bp->tablerows_gshare];
	bp->table_hybrid = new int[bp->tablerows_hybrid];

	bp->predictions = bp->mispredictions = 0;

	//30 bit 0x11.. Right shift for mask.
	bp->PCmask_bimodal = (1073741823 >> (30 - bp->h_index_bits_bimodal)); //For n=0.

	bp->PCmask_gshare = (1073741823 >> (30 - (bp->h_index_bits_gshare - bp->h_BHR_bits)));		//For m-n bits in gshare
	bp->BHR_gshare_mask = (1073741823 >> (30 - bp->h_BHR_bits));   //For n bits in gshare.

	bp->PCmask_hybrid = (1073741823 >> (30 - bp->h_chooser_bits));
	Hinitialize(bp);
}

void init_predictor(bp_params * bp,int m, int n)
{
	
	bp->PC_bits = m;
	bp->BHR_bits = n;
	bp->table_rows = pow(2, bp->PC_bits);
	bp->table = new int[bp->table_rows];  //Branch predictor table

	bp->predictions = 0;
	bp->mispredictions = 0;
	initialize_table(bp);
	bp->PC_mask = (1073741823 >> (30 - bp->PC_bits));
	bp->PCmask_gshare = (1073741823 >> (30 - (bp->PC_bits - bp->BHR_bits)));
	bp->BHR_gshare__mask = (1073741823 >> (30 - bp->BHR_bits));

}
void Bimodal_prediction(bp_params * bp, unsigned int addr, const char op)
{
	bp->predictions++;
	int predicted;
	int actual=-1;
	bp->address = addr;
	bp->index = (bp->address >> 2) & bp->PC_mask;

	//my predictoion(intal trained)
	if (bp->table[bp->index] > 1)
		predicted = 1;
	else if (bp->table[bp->index] < 2)
		predicted = 0;

	//actual prediction
	if (op == 'n')
		actual = 0;
	else if (op == 't')
		actual = 1;
	if (predicted != actual)
	{
		bp->mispredictions++;
	}

	if (actual == 1)
	{
		bp->table[bp->index] = bp->table[bp->index] + 1;
		if (bp->table[bp->index] > 3)
			bp->table[bp->index] = 3;
	}
	else if (actual == 0)
	{
		bp->table[bp->index] = bp->table[bp->index] - 1;
		if (bp->table[bp->index] < 0)
			bp->table[bp->index] = 0;
	}

}
void gshare_prediction(bp_params * bp, unsigned int addr, const char op)
{
	bp->predictions++;
	int prediction;
	int actual=-1;
	unsigned int p = 0;
	unsigned int x = 0;

	//cout << hex << addr << '\t' << op << endl;
	//cout << "PCmask_gshare " << PCmask_gshare << " Hmask " << Hmask <<  endl;

	bp->address = addr;

	p = bp->BHR_table ^ ((bp->address >> (2 + bp->PC_bits - bp->BHR_bits)) & bp->BHR_gshare__mask);

	bp->index = (p << (bp->PC_bits - bp->BHR_bits)) + ((bp->address >> 2) & bp->PCmask_gshare);

	if (bp->table[bp->index] > 1)
		prediction = 1;
	else if (bp->table[bp->index] < 2)
		prediction = 0;

	if (op == 'n')
		actual = 0;
	else if (op == 't')
		actual = 1;

	if (prediction != actual)
	{
		bp->mispredictions++;
	}

	//Update branch prediction table.
	if (actual == 1)
	{
		bp->table[bp->index] = bp->table[bp->index] + 1;
		if (bp->table[bp->index] > 3)
			bp->table[bp->index] = 3;
	}

	else if (actual == 0)
	{
		bp->table[bp->index] = bp->table[bp->index] - 1;
		if (bp->table[bp->index] < 0)
			bp->table[bp->index] = 0;
	}

	//Update global branch history register.
	//Make MSB equal to actual.
	x = bp->BHR_table >> 1;
	bp->BHR_table = (actual << (bp->BHR_bits - 1)) + x;


}

void hybrid_prediction(bp_hybrid *bp , unsigned int addr, const char op)
{
	int true_bimodal;
	int true_gshare;
	bp->predictions++;
	int actual=-1;
	int prediction_bimodal;
	int prediction_gshare;
	int prediction_overall;
	

	unsigned int p = 0;
	unsigned int x = 0;

	bp->h_address = addr;

	if (op == 'n')
		actual = 0;
	else if (op == 't')
		actual = 1;

	//Get bimodal prediction
	//1 is taken, 0 is not-taken.
	bp->h_index = (bp->h_address >> 2) & bp->PCmask_bimodal;

	if (bp->table_bimodal[bp->h_index] > 1)
		prediction_bimodal = 1;
	else if (bp->table_bimodal[bp->h_index] < 2)
		prediction_bimodal = 0;

	if (prediction_bimodal == actual)
		true_bimodal = 1;
	else true_bimodal = 0;

	//Get gshare prediction
	p = bp->h_BHR_table ^ ((bp->h_address >> (2 + bp->h_index_bits_gshare - bp->h_BHR_bits)) & bp->BHR_gshare_mask);

	bp->h_index = (p << (bp->h_index_bits_gshare - bp->h_BHR_bits)) + ((bp->h_address >> 2) & bp->PCmask_gshare);

	if (bp->table_gshare[bp->h_index] > 1)
		prediction_gshare = 1;
	else if (bp->table_gshare[bp->h_index] < 2)
		prediction_gshare = 0;

	if (prediction_gshare == actual)
		true_gshare = 1;
	else true_gshare = 0;


	//Obtain overall prediction
	bp->h_index = (bp->h_address >> 2) & bp->PCmask_hybrid;

	if (bp->table_hybrid[bp->h_index] > 1)
		prediction_overall = prediction_gshare;
	else if (bp->table_hybrid[bp->h_index] < 2)
		prediction_overall = prediction_bimodal;

	if (prediction_overall != actual)
		bp->mispredictions++;

	//Update choosen branch predictor.
	if (bp->table_hybrid[bp->h_index] > 1)  //gshare
	{
		p = bp->h_BHR_table ^ ((bp->h_address >> (2 + bp->h_index_bits_gshare - bp->h_BHR_bits)) & bp->BHR_gshare_mask);
		bp->h_index = (p << (bp->h_index_bits_gshare - bp->h_BHR_bits)) + ((bp->h_address >> 2) & bp->PCmask_gshare);
		if (actual == 1)
		{
			bp->table_gshare[bp->h_index] = bp->table_gshare[bp->h_index] + 1;
			if (bp->table_gshare[bp->h_index] > 3)
				bp->table_gshare[bp->h_index] = 3;
		}

		else if (actual == 0)
		{
			bp->table_gshare[bp->h_index] = bp->table_gshare[bp->h_index] - 1;
			if (bp->table_gshare[bp->h_index] < 0)
				bp->table_gshare[bp->h_index] = 0;
		}

	}

	else if (bp->table_hybrid[bp->h_index] < 2)		//bimodal
	{
		bp->h_index = (bp->h_address >> 2) & bp->PCmask_bimodal;

		if (actual == 1)
		{
			bp->table_bimodal[bp->h_index] = bp->table_bimodal[bp->h_index] + 1;
			if (bp->table_bimodal[bp->h_index] > 3)
				bp->table_bimodal[bp->h_index] = 3;
		}

		else if (actual == 0)
		{
			bp->table_bimodal[bp->h_index] = bp->table_bimodal[bp->h_index] - 1;
			if (bp->table_bimodal[bp->h_index] < 0)
				bp->table_bimodal[bp->h_index] = 0;
		}

	}

	//Update gshare global branch history register
	x = bp->h_BHR_table >> 1;
	bp->h_BHR_table = (actual << (bp->h_BHR_bits - 1)) + x;

	//Update branch chooser table
	bp->h_index = (bp->h_address >> 2) & bp->PCmask_hybrid;

	if ((true_gshare == 1) & (true_bimodal == 0))
	{
		bp->table_hybrid[bp->h_index] = bp->table_hybrid[bp->h_index] + 1;
		if (bp->table_hybrid[bp->h_index] > 3)
			bp->table_hybrid[bp->h_index] = 3;
	}
	else if ((true_gshare == 0) & (true_bimodal == 1))
	{
		bp->table_hybrid[bp->h_index] = bp->table_hybrid[bp->h_index] - 1;
		if (bp->table_hybrid[bp->h_index] < 0)
			bp->table_hybrid[bp->h_index] = 0;
	}

}



int main(int argc, char* argv[])
{
	
	ifstream trace_file;
	     // Variable that holds trace file name;
	bp_params  params;       // look at sim_bp.h header file for the the definition of struct bp_params
	char outcome;           // Variable holds branch outcome
	unsigned long int addr; // Variable holds the address read from input file
	
	if (!(argc == 4 || argc == 5 || argc == 7))
	{
		printf("Error: Wrong number of inputs:%d\n", argc - 1);
		exit(EXIT_FAILURE);
	}
	
	params.bp_name = argv[1];
	
	// strtoul() converts char* to unsigned long. It is included in <stdlib.h>
	
	if (strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
	{
		if (argc != 4)
		{
			printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc - 1);
			exit(EXIT_FAILURE);
		}
		params.M2 = strtoul(argv[2], NULL, 10);
		trace_file.open(argv[3]);
		cout << "COMMAND" << endl;
		cout << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << endl;
		//printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
		bimodal = (struct bp_params *)calloc(1, sizeof(bp_params));
		init_predictor(bimodal, params.M2,0);
		
		//trace_file = argv[3];
		
		//printf("COMMAND\n %s %lu %s\n", params.bp_name, params.M2, trace_file);
		//cout << "entered here";
		if (!trace_file.is_open())
		{
			// Throw error and exit if fopen() failed
			printf("Error: Unable to open file \n");
			exit(EXIT_FAILURE);
		}
		if (trace_file.is_open())
		{
			while (trace_file >> hex >> addr >> outcome)
			{
				Bimodal_prediction(bimodal, addr, outcome);
			}

		}
		trace_file.close();
		print(bimodal,0);
		free(bimodal);
	}
	else if (strcmp(params.bp_name, "gshare") == 0)          // Gshare
	{
		if (argc != 5)
		{
			printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc - 1);
			exit(EXIT_FAILURE);
		}
		params.M1 = strtoul(argv[2], NULL, 10);
		params.N = strtoul(argv[3], NULL, 10);
		cout << "COMMAND" << endl;
		cout << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << endl;
	//	trace_file = argv[4];
		//printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);
		gshare = (struct bp_params *)calloc(1, sizeof(bp_params));
		init_predictor(gshare, params.M1, params.N);
		trace_file.open(argv[4]);
		if (!trace_file.is_open())
		{
			// Throw error and exit if fopen() failed
			printf("Error: Unable to open file \n");
			exit(EXIT_FAILURE);
		}
		if (trace_file.is_open())
		{
			while (trace_file >> hex >> addr >> outcome)
			{
				gshare_prediction(gshare, addr, outcome);
			}

		}
		trace_file.close();
		print(gshare,1);
		free(gshare);
	}
	else if (strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
	{
		if (argc != 7)
		{
			printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc - 1);
			exit(EXIT_FAILURE);
		}
		params.K = strtoul(argv[2], NULL, 10);
		params.M1 = strtoul(argv[3], NULL, 10);
		params.N = strtoul(argv[4], NULL, 10);
		params.M2 = strtoul(argv[5], NULL, 10);
		trace_file.open(argv[6]);
	//	trace_file = argv[6];
		//printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);
		cout << "COMMAND" << endl;
		cout << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << " " << argv[5] << " " << argv[6] << endl;
		hybrid = (struct bp_hybrid *)calloc(1, sizeof(bp_hybrid));
		init_HPredictor(hybrid, params.K, params.M1, params.N, params.M2);
		if (!trace_file.is_open())
		{
			// Throw error and exit if fopen() failed
			printf("Error: Unable to open file \n");
			exit(EXIT_FAILURE);
		}
		if (trace_file.is_open())
		{
			while (trace_file >> hex >> addr >> outcome)
			{
				hybrid_prediction(hybrid, addr, outcome);
			}

		}
		trace_file.close();
		Hybridprint(hybrid);
		free(hybrid);
	}
	else
	{
		printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
		exit(EXIT_FAILURE);
	}

	//// Open trace_file in read mode
	//FP = fopen(trace_file, "r");
	//if (FP == NULL)
	//{
	//	// Throw error and exit if fopen() failed
	//	printf("Error: Unable to open file %s\n", trace_file);
	//	exit(EXIT_FAILURE);
	//}
	
	
	
	return 0;
}
