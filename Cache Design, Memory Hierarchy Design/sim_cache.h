#pragma once
#ifndef SIM_CACHE_H
#define SIM_CACHE_H

typedef struct cache_p {
	unsigned long int block_size;
	unsigned long int l1_size;
	unsigned long int l1_assoc;
	unsigned long int vc_num_blocks;
	unsigned long int l2_size;
	unsigned long int l2_assoc;
	unsigned long int no_of_sets;
	unsigned long int assoc;
	unsigned long int i_blocksize;
	unsigned long int **tag_matrix;
	unsigned long int **regenadd;
	 int **validity_flag_matrix;
	 int **dirty_flag_matrix;
	unsigned long int **LRU;
	unsigned long int read_hit_counter;
	unsigned long int rcount;
	unsigned long int read_miss_counter;
	unsigned long int wcount;
	unsigned long int write_hit_counter;
	unsigned long int write_miss_counter;
	unsigned long int WBcounter;
	struct cache_p * next_level_cache;
	struct cache_p * victim_cache;
}cache_size;

// Put additional data structures here as per your requirement

#endif