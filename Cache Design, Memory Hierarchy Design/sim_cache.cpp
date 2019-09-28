#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <math.h>
#include "sim_cache.h"
using namespace std;


const int WordSize = 32;
int memory_traffic;
cache_p *l1_cache = NULL, *l2_cache = NULL;
double swap_request, swaps;
// print function to print cache contents from MRU to LRU

void free_mem(cache_p *cache)
{
	int i;

	for (i = 0; (unsigned)i < cache->no_of_sets; i++)
	{
		free(cache->tag_matrix[i]);
	}
	free(cache->tag_matrix);



	for (i = 0; (unsigned)i < cache->no_of_sets; i++)
	{
		free(cache->LRU[i]);
	}
	free(cache->LRU);


	for (i = 0; (unsigned)i < cache->no_of_sets; i++)
	{
		free(cache->validity_flag_matrix[i]);
	}
	free(cache->validity_flag_matrix);


	for (i = 0; (unsigned)i < cache->no_of_sets; i++)
	{
		free(cache->dirty_flag_matrix[i]);
	}
	free(cache->dirty_flag_matrix);

	free(cache);
}

void print_data(cache_p *cache)
{
	unsigned long int count;
	int i, j;

	for (j = 0; (unsigned)j < cache->no_of_sets; j++)
	{
		printf("  set%4d:  ", j);
		count = 0;
		while (count < cache->assoc)
		{
			for (i = 0; (unsigned)i < cache->assoc; i++)
			{
				if (cache->validity_flag_matrix[j][i] && (cache->LRU[j][i] == count))
				{
					if (cache->tag_matrix[j][i])
						printf("%lx ", cache->tag_matrix[j][i]);
					if (cache->dirty_flag_matrix[j][i])
						printf("D ");
					else
						printf("  ");

					break;
				}
			}
			count++;
		}

		printf("\n");

	}
}
int empty_block(cache_p *cache, unsigned long int row)
{
	int i = 0;

	for (i = 0; (unsigned)i < cache->assoc; i++)
	{
		if (cache->validity_flag_matrix[row][i] == 0)
			return i;
	}

	return -1;
}
unsigned long int gettagvalue(cache_p *cache, unsigned long int add)
{
	unsigned long int tag, tagbits, mask;
	tagbits = WordSize - log2(cache->i_blocksize) - log2(cache->no_of_sets);
	mask = pow(2, tagbits) - 1;
	tag = ((add >> (WordSize - tagbits)) & mask);
	return tag;
}

//returns index of address for the given cache
unsigned long int getindexvalue(cache_p *cache, unsigned long int add)
{
	unsigned long int index, indexbits, mask;
	//check
	indexbits = log2(cache->no_of_sets);
	//cout << "This is the index bit" << indexbits << "\n";
	mask = pow(2, indexbits) - 1;
	//cout << mask ;
	//cout << "\n"<< (int)log2(cache->i_blocksize);
	index = ((add >> (int)log2(cache->i_blocksize)) & mask);

	return index;
}


unsigned long int LRU_location(cache_p *cache, unsigned long int index)
{

	int col;
	//highest value of LRU will be assoc-1
	for (col = 0; (unsigned)col < cache->assoc; col++)
	{
		if (cache->LRU[index][col] == (cache->assoc - 1))
			return col;
	}

	return cache->assoc;

}

void update_LRU(cache_p *cache, unsigned long int row, int col)
{
	int j;

	for (j = 0; (unsigned)j < cache->assoc; j++)
	{
		if (cache->LRU[row][j] < cache->LRU[row][col])
		{
			cache->LRU[row][j] = cache->LRU[row][j] + 1;
		}
			
	}
	//set most recently used to zero
	cache->LRU[row][col] = 0;
}
void cache_read(cache_p *cache, unsigned long int add);//prototype declaration to avoid identifier not found error
void cache_write(cache_p *cache, unsigned long int add)
{
		unsigned long int index, tag;
		int i, block_empty;
		index = getindexvalue(cache, add);
		tag = gettagvalue(cache, add);
		//cout << "index value=" << hex << index << "\n  tag value=" << hex << tag;
		//write request
		cache->wcount++;

		for (i = 0; (unsigned)i < cache->assoc; i++)
		{
			if (cache->validity_flag_matrix[index][i] && cache->tag_matrix[index][i] == tag)
			{
				//update LRU and set most recently used block
				update_LRU(cache, index, i);
				cache->write_hit_counter++;
				cache->dirty_flag_matrix[index][i] = 1;
				return;
			}
		}
		//if not hit,update miss
		cache->write_miss_counter++;
		//	check if the block is empty and update the location in block_empty variable
		block_empty = empty_block(cache, index);
		//if there is empty block, read from next level and update in empty block,skip VC
		//cout << block_empty;
		if (block_empty != -1)
		{
			if (cache->next_level_cache)
			{
				cache_read(cache->next_level_cache, add);
				//if empty block is there set the tag matrix,validity and dirty flag
				cache->tag_matrix[index][block_empty] = tag;
				cache->validity_flag_matrix[index][block_empty] = 1;
				cache->dirty_flag_matrix[index][block_empty] = 1;
				update_LRU(cache, index, block_empty);
				return;
			}
			else
			{
				memory_traffic++;
				cache->tag_matrix[index][block_empty] = tag;
				cache->validity_flag_matrix[index][block_empty] = 1;
				cache->dirty_flag_matrix[index][block_empty] = 1;
				update_LRU(cache, index, block_empty);
				return;
			}
		}
		//No empty block,eviction
		else
		{
			block_empty = LRU_location(cache, index);

			if (cache->victim_cache)
			{
				//cout << "entered victim";
				int k, v_offset, v_col;
				unsigned long int v_tag;
				int temp_dirty;
				unsigned long int temp_address, temp_tag, cache_tag, victim_tag;

				v_offset = (int)log2(cache->i_blocksize);
				v_tag = add >> v_offset;
				swap_request++;
				v_col = -1;
				for (k = 0; (unsigned)k < cache->victim_cache->assoc; k++)
				{
					if (cache->victim_cache->tag_matrix[0][k] == v_tag)
					{
						v_col = k;

					}

				}

				if (v_col != -1)
				{

					//store evicted block adresss and dirty bit in temp
					temp_dirty = cache->dirty_flag_matrix[index][block_empty];
					temp_tag = cache->tag_matrix[index][block_empty];
					//temp_index_bits = log2(cache->no_of_sets);
					temp_address = ((temp_tag << (int)(log2(cache->no_of_sets))) + (int)index);
					temp_address = temp_address << (int)log2(cache->i_blocksize);

					//swap dirty bits
					cache->dirty_flag_matrix[index][block_empty] = cache->victim_cache->dirty_flag_matrix[0][v_col];
					cache->victim_cache->dirty_flag_matrix[0][v_col] = temp_dirty;

					//swap address and update LRU in cache
					cache_tag = gettagvalue(cache, add);
					cache->tag_matrix[index][block_empty] = cache_tag;
					cache->validity_flag_matrix[index][block_empty] = 1;
					update_LRU(cache, index, block_empty);
					cache->dirty_flag_matrix[index][block_empty] = 1;

					//swap address and update LRU in victim
					victim_tag = gettagvalue(cache->victim_cache, temp_address);
					cache->victim_cache->tag_matrix[0][v_col] = victim_tag;
					cache->victim_cache->validity_flag_matrix[0][v_col] = 1;
					update_LRU(cache->victim_cache, 0, v_col);

					swaps++;
					return;
				}

				else {
					v_col = empty_block(cache->victim_cache, 0);

					if (v_col < 0)
					{
						v_col = LRU_location(cache->victim_cache, 0);

						if (cache->victim_cache->dirty_flag_matrix[0][v_col])
						{
							unsigned long int victim_LRU_address;
							victim_LRU_address = cache->victim_cache->tag_matrix[0][v_col] << v_offset;
							if (cache->next_level_cache)
							{
								cache_write(cache->next_level_cache, victim_LRU_address);
							}
							else
								memory_traffic++;

							cache->WBcounter++;
							cache->victim_cache->dirty_flag_matrix[0][v_col] = 0;
							cache->victim_cache->validity_flag_matrix[0][v_col] = 0;
						}
					}
					unsigned long int cache_to_victim_address, cache_to_victim_tag;
					cache_to_victim_tag = cache->tag_matrix[index][block_empty];
					//temp_index_bits = log2(cache->no_of_sets);
					cache_to_victim_address = ((cache_to_victim_tag << (int)(log2(cache->no_of_sets))) + (int)index);
					cache->victim_cache->tag_matrix[0][v_col] = cache_to_victim_address;
					cache->victim_cache->validity_flag_matrix[0][v_col] = 1;
					cache->victim_cache->dirty_flag_matrix[0][v_col] = cache->dirty_flag_matrix[index][block_empty];
					update_LRU(cache->victim_cache, 0, v_col);

					if (cache->next_level_cache)
						cache_read(cache->next_level_cache, add);
					else
						memory_traffic++;
					cache->tag_matrix[index][block_empty] = tag;
					cache->validity_flag_matrix[index][block_empty] = 1;
					cache->dirty_flag_matrix[index][block_empty] = 1;
					update_LRU(cache, index, block_empty);
					return;
				}
			}
			else
			{
				block_empty = LRU_location(cache, index);
				if (cache->next_level_cache)
				{
					if (cache->dirty_flag_matrix[index][block_empty])
					{
						//int lru_dirty;
						unsigned long int lru_address, lru_tag;
						//lru_dirty = cache->dirty_flag_matrix[index][block_empty];
						lru_tag = cache->tag_matrix[index][block_empty];
						//lru_index_bits = log2(cache->no_of_sets);
						//lru_offset_bit = (int)log2(cache->block_size);
						lru_address = ((lru_tag << (int)(log2(cache->no_of_sets))) + (int)index);
						lru_address = lru_address << (int)log2(cache->i_blocksize);
						//write into next memory level 
						//cache_write(cache->next_level_cache, add);
						cache_write(cache->next_level_cache, lru_address);//this address is wrong(you have to put the lru addrsss and not incoming address)
						cache->dirty_flag_matrix[index][block_empty] = 0;
						//cache->validity_flag_matrix = 0;
						cache->WBcounter++;

					}
					cache_read(cache->next_level_cache, add);
					cache->tag_matrix[index][block_empty] = gettagvalue(cache, add);
					cache->validity_flag_matrix[index][block_empty] = 1;
					cache->dirty_flag_matrix[index][block_empty] = 1;
					update_LRU(cache, index, block_empty);
					return;

				}
			
				else {
					if (cache->dirty_flag_matrix[index][block_empty])
					{
						//write into next memory level 
						memory_traffic++;//not sure about this
						cache->WBcounter++;
						cache->dirty_flag_matrix[index][block_empty] = 0;
						//cache->validity_flag_matrix = 0;

					}
					memory_traffic++;
					cache->tag_matrix[index][block_empty] = gettagvalue(cache, add);
					cache->validity_flag_matrix[index][block_empty] = 1;
					cache->dirty_flag_matrix[index][block_empty] = 1;
					update_LRU(cache, index, block_empty);
					return;
				}
			}




		}

	}

void cache_read(cache_p *cache, unsigned long int add) {
	unsigned long int index, tag;
	int i, block_empty;
	index = getindexvalue(cache, add);
	tag = gettagvalue(cache, add);
	
	//cout << "index value=" << hex << index << "\n  tag value=" << hex << tag;
	//read request
	
	cache->rcount++;

	for (i = 0; (unsigned)i < cache->assoc; i++)
	{
		if (cache->validity_flag_matrix[index][i] && cache->tag_matrix[index][i] == tag)
		{
			//update LRU and set most recently used block
			update_LRU(cache, index, i);
			cache->read_hit_counter++;
			return;
		}
	}
	//if not hit,update miss
	cache->read_miss_counter++;
	//	check if the block is empty and update the location in block_empty variable
	block_empty = empty_block(cache, index);
	//if there is empty block, read from next level and update in empty block,skip VC
	//cout << block_empty;
	if (block_empty != -1)
	{
		if (cache->next_level_cache)
		{
			cache_read(cache->next_level_cache, add);
			//if empty block is there set the tag matrix,validity and dirty flag
			cache->tag_matrix[index][block_empty] = tag;
			cache->validity_flag_matrix[index][block_empty] = 1;
			cache->dirty_flag_matrix[index][block_empty] = 0;
			update_LRU(cache, index, block_empty);
			return;
		}
		else
		{
			
			memory_traffic++;
			cache->tag_matrix[index][block_empty] = tag;
			cache->validity_flag_matrix[index][block_empty] = 1;
			cache->dirty_flag_matrix[index][block_empty] = 0;
			update_LRU(cache, index, block_empty);
			return;
		}
	}
	//No empty block,eviction
	else
	{
		block_empty = LRU_location(cache, index);

		if (cache->victim_cache)
		{
			//cout << "entered victim";
			int k, v_offset, v_col;
			unsigned long int v_tag;
			int temp_dirty;
			unsigned long int temp_address, temp_tag, cache_tag, victim_tag;

			v_offset = (int)log2(cache->i_blocksize);
			v_tag = add >> v_offset;
			swap_request++;
			v_col = -1;
			for (k = 0; (unsigned)k < cache->victim_cache->assoc; k++)
			{
				if (cache->victim_cache->tag_matrix[0][k] == v_tag)
				{
					v_col = k;

				}

			}

			if (v_col != -1)
			{

				//store evicted block adresss and dirty bit in temp
				temp_dirty = cache->dirty_flag_matrix[index][block_empty];
				temp_tag = cache->tag_matrix[index][block_empty];
				//temp_index_bits = log2(cache->no_of_sets);
				temp_address = ((temp_tag << (int)(log2(cache->no_of_sets))) + (int)index);
				temp_address = temp_address << (int)log2(cache->i_blocksize);

				//swap dirty bits
				cache->dirty_flag_matrix[index][block_empty] = cache->victim_cache->dirty_flag_matrix[0][v_col];
				cache->victim_cache->dirty_flag_matrix[0][v_col] = temp_dirty;

				//swap address and update LRU in cache
				cache_tag = gettagvalue(cache, add);
				cache->tag_matrix[index][block_empty] = cache_tag;
				cache->validity_flag_matrix[index][block_empty] = 1;
				update_LRU(cache, index, block_empty);
				

				//swap address and update LRU in victim
				victim_tag = gettagvalue(cache->victim_cache, temp_address);
				cache->victim_cache->tag_matrix[0][v_col] = victim_tag;
				cache->victim_cache->validity_flag_matrix[0][v_col] = 1;
				update_LRU(cache->victim_cache, 0, v_col);

				swaps++;
				return;
			}

			else {
				v_col = empty_block(cache->victim_cache, 0);

				if (v_col < 0)
				{
					v_col = LRU_location(cache->victim_cache, 0);

					if (cache->victim_cache->dirty_flag_matrix[0][v_col])
					{
						unsigned long int victim_LRU_address;
						victim_LRU_address = cache->victim_cache->tag_matrix[0][v_col] << v_offset;
						if (cache->next_level_cache)
						{
							cache_write(cache->next_level_cache, victim_LRU_address);
						}
						else
							memory_traffic++;

						cache->WBcounter++;
						cache->victim_cache->dirty_flag_matrix[0][v_col] = 0;
						cache->victim_cache->validity_flag_matrix[0][v_col] = 0;
					}
				}
				unsigned long int cache_to_victim_address, cache_to_victim_tag;
				cache_to_victim_tag = cache->tag_matrix[index][block_empty];
				//temp_index_bits = log2(cache->no_of_sets);
				cache_to_victim_address = ((cache_to_victim_tag << (int)(log2(cache->no_of_sets))) + (int)index);
				cache->victim_cache->tag_matrix[0][v_col] = cache_to_victim_address;
				cache->victim_cache->validity_flag_matrix[0][v_col] = 1;
				cache->victim_cache->dirty_flag_matrix[0][v_col] = cache->dirty_flag_matrix[index][block_empty];
				update_LRU(cache->victim_cache, 0, v_col);

				if (cache->next_level_cache)
					cache_read(cache->next_level_cache, add);
				else
					memory_traffic++;
				cache->tag_matrix[index][block_empty] = tag;
				cache->validity_flag_matrix[index][block_empty] = 1;
				cache->dirty_flag_matrix[index][block_empty] = 0;
				update_LRU(cache, index, block_empty);
				return;
			}
		}
		else {
			block_empty = LRU_location(cache, index);
			if (cache->next_level_cache)
			{
				if (cache->dirty_flag_matrix[index][block_empty])
				{
					unsigned long int lru_address, lru_tag;
					//lru_dirty = cache->dirty_flag_matrix[index][block_empty];
					lru_tag = cache->tag_matrix[index][block_empty];
					//lru_index_bits = log2(cache->no_of_sets);
					//lru_offset_bit = (int)log2(cache->block_size);
					lru_address = ((lru_tag << (int)(log2(cache->no_of_sets))) + (int)index);
					lru_address =  lru_address<< (int)log2(cache->i_blocksize);
					//cout <<"\n"<< lru_address;
					//write into next memory level 
					//cache_write(cache->next_level_cache, add);
					cache_write(cache->next_level_cache, lru_address);//this address is wrong(you have to put the lru addrsss and not incoming address)
					cache->dirty_flag_matrix[index][block_empty] = 0;
					//cache->validity_flag_matrix = 0;
					cache->WBcounter++;
					
				}
				cache_read(cache->next_level_cache, add);
				cache->tag_matrix[index][block_empty] = gettagvalue(cache, add);
				cache->validity_flag_matrix[index][block_empty] = 1;
				update_LRU(cache,index,block_empty);
				return;

			}
			else {
				if (cache->dirty_flag_matrix[index][block_empty])
				{
					//write into next memory level 
					memory_traffic++;//not sure about this
					cache->WBcounter++;
					cache->dirty_flag_matrix[index][block_empty] = 0;
					//cache->validity_flag_matrix = 0;
					
				}
				memory_traffic++;
				cache->tag_matrix[index][block_empty] = gettagvalue(cache, add);
				cache->validity_flag_matrix[index][block_empty] = 1;
				update_LRU(cache, index, block_empty);
				return;
			}
		}
			

		}




	}

void init_caches(cache_p * i_cache, unsigned long int blocksize, unsigned long int size, unsigned long int assoc) 
{
	int i,j;
	//cout << "cache initialisation starts";
	i_cache->no_of_sets = size / (blocksize * assoc);
	//cout << "\n sets=" << i_cache->no_of_sets;
	i_cache->assoc = assoc;
	//cout << "\n assoc=" << i_cache->assoc;
	i_cache->i_blocksize = blocksize;
	//cout << "\n i_blocksize=" << i_cache->i_blocksize;
	
	//create tag matrix
	//row space
	i_cache->tag_matrix = (unsigned long int **)calloc(i_cache->no_of_sets, sizeof(unsigned long int *));
	//columns space
	for (i = 0; (unsigned)i < i_cache->no_of_sets; i++)
	{
		i_cache->tag_matrix[i] = (unsigned long int *)calloc(assoc, sizeof(unsigned long int));

	}
	//int count = 0;
	//for (i = 0; i < i_cache->no_of_sets; i++)
	//{
	//	for (j = 0; j < i_cache->assoc; j++)
	//	{
	//		//cout << count << i_cache->tag_matrix[i][j]<<"\n";
	//		
	//	}
	//	count++;
	//}

	i_cache->LRU = (unsigned long int **)calloc(i_cache->no_of_sets, sizeof(unsigned long int *));
	for (i = 0; (unsigned)i < i_cache->no_of_sets; i++)
	{
		i_cache->LRU[i] = (unsigned long int *)calloc(assoc, sizeof(unsigned long int));
		for (j = 0; (unsigned)j < assoc; j++)
		{
			i_cache->LRU[i][j] = j;
		}
	}
	
	i_cache->validity_flag_matrix = (int **)calloc(i_cache->no_of_sets, sizeof(int *));
	for (i = 0; (unsigned)i < i_cache->no_of_sets; i++)
	{
		i_cache->validity_flag_matrix[i] = (int *)calloc(assoc, sizeof(int));
	}

	i_cache->dirty_flag_matrix = (int **)calloc(i_cache->no_of_sets, sizeof(int *));
	for (i = 0; (unsigned)i < i_cache->no_of_sets; i++)
	{
		i_cache->dirty_flag_matrix[i] = (int *)calloc(assoc, sizeof(int));
	}


}




int main(int argc, char *argv[])
{	
	//cout << "TEST";

	ifstream trace_file(argv[7]); // File handler
	     // Variable that holds trace file name;
	cache_p params;    // look at sim_cache.h header file for the the definition of struct cache_params
	char rw;                // variable holds read/write type read from input file. The array size is 2 because it holds 'r' or 'w' and '\0'. Make sure to adapt in future projects
	 unsigned long int addr; // Variable holds the address read from input file

	

	// strtoul() converts char* to unsigned long. It is included in <stdlib.h>
	params.block_size = atoi(argv[1]);
	params.l1_size = atoi(argv[2]);
	params.l1_assoc = atoi(argv[3]);
	params.vc_num_blocks = atoi(argv[4]);
	params.l2_size = atoi(argv[5]);
	params.l2_assoc = atoi(argv[6]);
	char *file = argv[7];

	// Open trace_file in read mode
	
	if (!trace_file.is_open())
	{
		// Throw error and exit if fopen() failed
		printf("Error: Unable to open file\n");
		exit(EXIT_FAILURE);
	}

	// Print params
	printf("  ===== Simulator configuration =====\n"
		"  BLOCKSIZE:                     %lu\n"
		"  L1_SIZE:                          %lu\n"
		"  L1_ASSOC:                         %lu\n"
		"  VC_NUM_BLOCKS:                    %lu\n"
		"  L2_SIZE:                          %lu\n"
		"  L2_ASSOC:                         %lu\n"
		"  trace_file:                      %15s\n\n", params.block_size, params.l1_size, params.l1_assoc, params.vc_num_blocks, params.l2_size, params.l2_assoc, file);

	// create l1 cache
	l1_cache = (struct cache_p *)calloc(1, sizeof(cache_size));
	init_caches(l1_cache , params.block_size, params.l1_size, params.l1_assoc);

	// if l2 cache entered , create l2 cache
	if (params.l2_size!=0)
	{
		l2_cache = (struct cache_p *)calloc(1, sizeof(cache_size));
		init_caches(l2_cache, params.block_size, params.l2_size, params.l2_assoc);
		l1_cache->next_level_cache = l2_cache;
		l2_cache->next_level_cache = NULL;
		l2_cache->victim_cache = NULL;
	}
	else
	{
		l1_cache->next_level_cache = NULL;
	}

	if (params.vc_num_blocks)
	{
		l1_cache->victim_cache = (struct cache_p *)calloc(1, sizeof(cache_size));
		init_caches(l1_cache->victim_cache, params.block_size, (params.block_size*params.vc_num_blocks), params.vc_num_blocks);
	}
	else
		l1_cache->victim_cache = NULL;

	char str[2];
	int c = 0;
	while (trace_file>>str[0]>>hex >>addr)
	{
		if (c>-1)
		{
			rw = str[0];
			if (rw == 'r')
			{
				//cout << "read" << addr<<"\n";

			//printf("%s %s\n", "read", addr.c_str());
				//printf("%s %lx\n", "read", addr);
			cache_read(l1_cache, addr);

			}
			// Print and test if file is read correctly
			else if (rw == 'w') {
				//cout << "write" << addr<<"\n";
				//printf("%s %lx\n", "write", addr);

			//printf("%s %s\n", "write", addr.c_str());
				cache_write(l1_cache, addr);

			}
		}// Print and test if file is read correctly
		/*************************************
				  Add cache code here
		**************************************/
		else
		{
			;
		}
		c++;
	}
	if (l1_cache)
	{
		cout <<"===== L1 contents =====" <<endl;
		print_data(l1_cache);
		printf("\n");
	}

	if (l1_cache && l1_cache->victim_cache)
	{
		cout <<"===== VC contents =====" <<endl;
		print_data(l1_cache->victim_cache);
		printf("\n");
	}

	if (l2_cache)
	{
		cout<<"===== L2 contents ====="<<endl;
		print_data(l2_cache);
		printf("\n");
	}

	cout <<"===== Simulation results =====" << endl;

	int l1read = 0, l1rmiss = 0, l1write = 0, l1wmiss = 0, l1vcwb = 0;
	int l2read = 0, l2rmiss = 0, l2write = 0, l2wmiss = 0, l2wb = 0;
	double srr = 0.0, l1_vc_missrate = 0.0, l2_missrate = 0.0;

	if (l1_cache)
	{
		l1read = l1_cache->rcount;
		l1rmiss = l1_cache->read_miss_counter;
		l1write = l1_cache->wcount;
		l1wmiss = l1_cache->write_miss_counter;

		if (l1read || l1write)
			srr = swap_request / (l1read + l1write);

		if (l1read || l1write)
			l1_vc_missrate = (l1rmiss + l1wmiss - swaps) / (l1read + l1write);

		if (l1_cache->victim_cache)
			l1vcwb = l1_cache->WBcounter + l1_cache->victim_cache->WBcounter;
		else
			l1vcwb = l1_cache->WBcounter;
	}

	if (l2_cache)
	{
		l2read = l2_cache->rcount;
		l2rmiss = l2_cache->read_miss_counter;
		l2write = l2_cache->wcount;
		l2wmiss = l2_cache->write_miss_counter;
		l2wb = l2_cache->WBcounter;
		if (l2read)
			l2_missrate = (double)l2rmiss / l2read;
	}


	cout<< "a. number of L1 reads:       	   	" << l1read << endl;
	cout << "b. number of L1 read misses:  	   	" << l1rmiss << endl;
	cout << "c. number of L1 writes:        	   	" << l1write << endl;
	cout << "d. number of L1 write misses:  	   	" << l1wmiss << endl;
	cout << "e. number of swap requests:    	   	"<< (int)swap_request<< endl;
	cout << "f. swap request rate:                   "<< setprecision(4) <<fixed<< srr << endl;
	cout << "g. number of swaps:                   	"<< (int)swaps<< endl;
	cout << "h. combined L1+VC miss rate:            "<<setprecision(4) <<fixed<< l1_vc_missrate << endl;
	cout << "i. number writebacks from L1/VC:   	"<< l1vcwb << endl;
	cout << "j. number of L2 reads:       	   	"<< l2read << endl;
	cout << "k. number of L2 read misses:  	   	"<< l2rmiss << endl;
	cout << "l. number of L2 writes:        	   	"<< l2write << endl;
	cout << "m. number of L2 write misses:  	   	"<< l2wmiss << endl;
	cout << "n. L2 miss rate:                        "<< setprecision(4) <<fixed<< l2_missrate << endl;
	cout << "o. number of writebacks from L2:      	"<< l2wb << endl;
	cout << "p. total memory traffic:              	"<< memory_traffic << endl;

if (l2_cache)
		free_mem(l2_cache);
	if (l1_cache && l1_cache->victim_cache)
		free_mem(l1_cache->victim_cache);
	if (l1_cache)
		free_mem(l1_cache);
	return 0;
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
