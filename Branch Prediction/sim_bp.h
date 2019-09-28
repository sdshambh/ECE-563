#pragma once
#ifndef SIM_BP_H
#define SIM_BP_H

typedef struct bp_params {
	unsigned long int K;
	unsigned long int M1;
	unsigned long int M2;
	unsigned long int N;
	const char* bp_name;
	int PC_bits;
	int BHR_bits;
	int table_rows;
	int  * table;
	unsigned int BHR_table;
	int predictions ;
	int mispredictions ;
	unsigned int PC_mask;
	unsigned int PCmask_gshare;
	unsigned int BHR_gshare__mask;
	unsigned int address;
	unsigned int index;
	
	
}bp_params;

typedef struct bp_hybrid{
	unsigned long int K;
	unsigned long int M1;
	unsigned long int M2;
	unsigned long int N;
	unsigned int h_address;
	unsigned int h_index;
	int h_index_bits_bimodal;
	int h_index_bits_gshare;
	int h_BHR_bits;
	int h_chooser_bits;
	unsigned int h_BHR_table;

	int tablerows_bimodal;
	int tablerows_gshare;
	int tablerows_hybrid;

	int *table_bimodal;
	int *table_gshare;
	int *table_hybrid;

	unsigned int PCmask_bimodal;		//For bimodal
	unsigned int PCmask_gshare;
	unsigned int PCmask_hybrid;
	unsigned int BHR_gshare_mask;			//For gshare

	int predictions;
	int mispredictions;
}bp_hybrid;

// Put additional data structures here as per your requirement

#endif
