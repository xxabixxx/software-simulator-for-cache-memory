#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>

using namespace std;

class Access {
public:
		string mem_addr;
		string task;
		string data;
};
class Block {
public:
		string data;
		int tag;
		int addr;
		int valid;
		int dirty;
		int T;
};

int main(int argc, char* argv[]) {
	ifstream infile;
	infile.open(argv[1]);
	string str;

	getline(infile, str);
	int cache_size = atoi(str.c_str());

	getline(infile, str);
	int cache_blk_size = atoi(str.c_str());

	getline(infile, str);
	int cache_assoc = atoi(str.c_str());

	getline(infile, str);
	int T = atoi(str.c_str());

	getline(infile, str);

	string mem_inst[1000];
	int index = 0;

	while(getline(infile, str)) {
		mem_inst[index] = str;
		index++;
	}

	Access mem_accesses[1000];
	int num_reads = 0, num_writes = 0;
	int num_blocks = cache_size/cache_blk_size;												// Number of blocks in the cache
	int num_sets = num_blocks/cache_assoc;													// Number of sets in the cache

	int write_hit = 0, read_hit = 0, write_miss = 0, read_miss = 0;

			// READING ALL ACCESS REQUESTS
	for(int i = 0; i < index; i++) {
		string acc = mem_inst[i];
		int first_comma = acc.find_first_of(",", 0, (acc.length() - 1));
		mem_accesses[i].mem_addr = acc.substr(0,first_comma);
		mem_accesses[i].task = acc.substr((first_comma + 2), 1);
		if(mem_accesses[i].task == "R") {
			mem_accesses[i].data = "_";
			num_reads++;
		}
		else{
			mem_accesses[i].data = acc.substr((first_comma + 5), (acc.length() - first_comma - 5));
			num_writes++;
		}
	}

				// EMPTY BLOCK 
	Block Empty_BLK;
	Empty_BLK.data = "_";
	Empty_BLK.tag = -1;
	Empty_BLK.addr = -1;
	Empty_BLK.valid = 0;
	Empty_BLK.dirty = 0;
	Empty_BLK.T = -2;

			// CACHE INITIALISATION
	Block cache[cache_assoc][num_sets];
	for (int i = 0; i <cache_assoc; i++) {
		for (int j = 0; j<num_sets; j++) {
			cache[i][j] = Empty_BLK;
		}
	}

			// MAIN MEMORY CREATION
	string main_memmory[1024];

			// MAIN MEMORY INITIALISATION
	for (int i = 0; i<1024; i++){
		main_memmory[i] = to_string(-1 * (rand() % 1000));
	}

			// PROCESSING THE ACCESS REQUESTS

	for (int i = 0; i < index; i++) {
		int address = atoi(mem_accesses[i].mem_addr.c_str());								// Calculating the block address
		int cache_addr = address % num_sets;												// Calculating the address in the cache set

			// Processing for a read access
		if (mem_accesses[i].task == "R") {
			// If accessed block is present in the cache
			bool addr_in_set = false;
			int valid_block = -1;
			for (int j = 0; j<cache_assoc; j++) {											// Finding the block in the cache set
				if (cache[j][cache_addr].addr == atoi(mem_accesses[i].mem_addr.c_str())) {
					addr_in_set = true;
					valid_block = j;
				}
			}
			if (addr_in_set) {																// If the data accessed is present in the cache
				read_hit++;																	// HIT
				if (valid_block >= cache_assoc/2) {											// If the data is stored in a block in the low priority level then move that block to the high priority level
					bool has_hpblock = false;
					int hp_block = -1;
					for (int k = 0; k<cache_assoc/2; k++) {
						if (cache[k][cache_addr].valid == 0) {
							has_hpblock = true;
							hp_block = k;
						}
					}
					if(has_hpblock) {														// If the HPL has an empty block, then move the cache block 
						cache[hp_block][cache_addr] = cache[valid_block][cache_addr];
						cache[hp_block][cache_addr].T = -1;
						cache[valid_block][cache_addr] = Empty_BLK;
						// PRINTING
						// cout<<cache[hp_block][cache_addr].data;
						// cout<<", ";
						// cout<<cache[hp_block][cache_addr].tag;
						// cout<<", ";
						// cout<<cache[hp_block][cache_addr].valid;
						// cout<<", ";
						// cout<<cache[hp_block][cache_addr].dirty<<endl;

					}
					else {																	// If no empty block is present the remove the least recently used block from HPL
						int least_recent_block = -1;
						int temp_T = -5;
						for (int k = 0; k< cache_assoc/2; k++) {
							if (cache[k][cache_addr].T > temp_T) {
								least_recent_block = k;
								temp_T = cache[k][cache_addr].T;
							}
						}																	// Store the new cache block 
						main_memmory[cache[least_recent_block][cache_addr].addr] = cache[least_recent_block][cache_addr].data;  // Wrtie back in memory before deleting the cache block
						cache[least_recent_block][cache_addr] = cache[valid_block][cache_addr];
						cache[least_recent_block][cache_addr].T = -1;
						cache[valid_block][cache_addr] = Empty_BLK;
						// PRINTING
						// cout<<cache[least_recent_block][cache_addr].data;
						// cout<<", ";
						// cout<<cache[least_recent_block][cache_addr].tag;
						// cout<<", ";
						// cout<<cache[least_recent_block][cache_addr].valid;
						// cout<<", ";
						// cout<<cache[least_recent_block][cache_addr].dirty<<endl;
					}
				}
				else {																		// If the data is already in a block in the HPL simply access it and set T to -1
					cache[valid_block][cache_addr].T = -1;
					// PRINTING
					// cout<<cache[valid_block][cache_addr].data;
					// cout<<", ";
					// cout<<cache[valid_block][cache_addr].tag;
					// cout<<", ";
					// cout<<cache[valid_block][cache_addr].valid;
					// cout<<", ";
					// cout<<cache[valid_block][cache_addr].dirty<<endl;
				}
			}
			else {
				// If accessed block is not present in the cache
				read_miss++;																// MISS
				bool has_block = false;														// Check for empty block in Low Priority Level
				int empty_block = -1;
				for(int j = cache_assoc/2; j<cache_assoc; j++) {
					if (cache[j][cache_addr].valid == 0) {
						has_block = true;
						empty_block = j;
					}
				}
				if (has_block) {															// If empty block is present then store the data from the main memory into the cache block
					cache[empty_block][cache_addr].data = main_memmory[address];
					cache[empty_block][cache_addr].tag = address/num_sets;
					cache[empty_block][cache_addr].addr = address;
					cache[empty_block][cache_addr].valid = 1;
					cache[empty_block][cache_addr].dirty = 0;
					cache[empty_block][cache_addr].T = -1;
					// PRINTING
					// cout<<cache[empty_block][cache_addr].data;
					// cout<<", ";
					// cout<<cache[empty_block][cache_addr].tag;
					// cout<<", ";
					// cout<<cache[empty_block][cache_addr].valid;
					// cout<<", ";
					// cout<<cache[empty_block][cache_addr].dirty<<endl;
				}
				else {																		// If not present then, find the least recently used block in the Low Priority Level and replace it
					int least_recent_block = -1;
					int temp_T = -5;
					for (int k = cache_assoc/2; k<cache_assoc; k++) {
						if (cache[k][cache_addr].T > temp_T) {
							least_recent_block = k;
							temp_T = cache[k][cache_addr].T;
						}
					}
					main_memmory[cache[least_recent_block][cache_addr].addr] = cache[least_recent_block][cache_addr].data; // Write back in memory before removing the cache block
					cache[least_recent_block][cache_addr].data = main_memmory[address];
					cache[least_recent_block][cache_addr].tag = address/num_sets;
					cache[least_recent_block][cache_addr].addr = address;
					cache[least_recent_block][cache_addr].valid = 1;
					cache[least_recent_block][cache_addr].dirty = 0;
					cache[least_recent_block][cache_addr].T = -1;
					// PRINTING
					// cout<<cache[least_recent_block][cache_addr].data;
					// cout<<", ";
					// cout<<cache[least_recent_block][cache_addr].tag;
					// cout<<", ";
					// cout<<cache[least_recent_block][cache_addr].valid;
					// cout<<", ";
					// cout<<cache[least_recent_block][cache_addr].dirty<<endl;
				}
			}
		}
		else {
			bool addr_in_set = false;
			int valid_block = -1;
			for (int j = 0; j<cache_assoc; j++) {											// Finding the block in the cache set
				if (cache[j][cache_addr].addr == atoi(mem_accesses[i].mem_addr.c_str())) {
					addr_in_set = true;
					valid_block = j;
				}
			}
			if (addr_in_set) {
				write_hit++;
				if (valid_block >= cache_assoc/2) {											// If the data is stored in a block in the low priority level then move that block to the high priority level
					bool has_hpblock = false;
					int hp_block = -1;
					for (int k = 0; k<cache_assoc/2; k++) {
						if (cache[k][cache_addr].valid == 0) {
							has_hpblock = true;
							hp_block = k;
						}
					}
					if (has_hpblock) {
						cache[hp_block][cache_addr] = cache[valid_block][cache_addr];
						cache[hp_block][cache_addr].T = -1;
						cache[valid_block][cache_addr] = Empty_BLK;
						cache[hp_block][cache_addr].data = mem_accesses[i].data;
						cache[hp_block][cache_addr].dirty = 1;
						cache[hp_block][cache_addr].valid = 1;
						// PRINTING
						// cout<<cache[hp_block][cache_addr].data;
						// cout<<", ";
						// cout<<cache[hp_block][cache_addr].tag;
						// cout<<", ";
						// cout<<cache[hp_block][cache_addr].valid;
						// cout<<", ";
						// cout<<cache[hp_block][cache_addr].dirty<<endl;
					}
					else {
						int least_recent_block = -1;
						int temp_T = -5;
						for (int k = 0; k< cache_assoc/2; k++) {
							if (cache[k][cache_addr].T > temp_T) {
								least_recent_block = k;
								temp_T = cache[k][cache_addr].T;
							}
						}																	// Store the new cache block 
						main_memmory[cache[least_recent_block][cache_addr].addr] = cache[least_recent_block][cache_addr].data;  // Wrtie back in memory before deleting the cache block
						cache[least_recent_block][cache_addr] = cache[valid_block][cache_addr];
						cache[least_recent_block][cache_addr].T = -1;
						cache[least_recent_block][cache_addr].data = mem_accesses[i].data;
						cache[least_recent_block][cache_addr].dirty = 1;
						cache[least_recent_block][cache_addr].valid = 1;
						cache[valid_block][cache_addr] = Empty_BLK;
						// PRINTING
						// cout<<cache[least_recent_block][cache_addr].data;
						// cout<<", ";
						// cout<<cache[least_recent_block][cache_addr].tag;
						// cout<<", ";
						// cout<<cache[least_recent_block][cache_addr].valid;
						// cout<<", ";
						// cout<<cache[least_recent_block][cache_addr].dirty<<endl;
					}
				}
				else {
					cache[valid_block][cache_addr].T = -1;
					cache[valid_block][cache_addr].data = mem_accesses[i].data;
					cache[valid_block][cache_addr].dirty = 1;
					cache[valid_block][cache_addr].valid = 1;
					// PRINTING
					// cout<<cache[valid_block][cache_addr].data;
					// cout<<", ";
					// cout<<cache[valid_block][cache_addr].tag;
					// cout<<", ";
					// cout<<cache[valid_block][cache_addr].valid;
					// cout<<", ";
					// cout<<cache[valid_block][cache_addr].dirty<<endl;
				}
			}
			else {
				write_miss++;
				bool has_block = false;														// Check for empty block in Low Priority Level
				int empty_block = -1;
				for(int j = cache_assoc/2; j<cache_assoc; j++) {
					if (cache[j][cache_addr].valid == 0) {
						has_block = true;
						empty_block = j;
					}
				}
				if (has_block) {															// If empty block is present then store the data from the main memory into the cache block
					cache[empty_block][cache_addr].data = mem_accesses[i].data;
					cache[empty_block][cache_addr].tag = address/num_sets;
					cache[empty_block][cache_addr].addr = address;
					cache[empty_block][cache_addr].valid = 1;
					cache[empty_block][cache_addr].dirty = 1;
					cache[empty_block][cache_addr].T = -1;
					// PRINTING
					// cout<<cache[empty_block][cache_addr].data;
					// cout<<", ";
					// cout<<cache[empty_block][cache_addr].tag;
					// cout<<", ";
					// cout<<cache[empty_block][cache_addr].valid;
					// cout<<", ";
					// cout<<cache[empty_block][cache_addr].dirty<<endl;
				}
				else {																		// If not present then, find the least recently used block in the Low Priority Level and replace it
					int least_recent_block = -1;
					int temp_T = -5;
					for (int k = cache_assoc/2; k<cache_assoc; k++) {
						if (cache[k][cache_addr].T > temp_T) {
							least_recent_block = k;
							temp_T = cache[k][cache_addr].T;
						}
					}
					main_memmory[cache[least_recent_block][cache_addr].addr] = cache[least_recent_block][cache_addr].data; // Write back in memory before removing the cache block
					cache[least_recent_block][cache_addr].data = mem_accesses[i].data;
					cache[least_recent_block][cache_addr].tag = address/num_sets;
					cache[least_recent_block][cache_addr].addr = address;
					cache[least_recent_block][cache_addr].valid = 1;
					cache[least_recent_block][cache_addr].dirty = 1;
					cache[least_recent_block][cache_addr].T = -1;
					// PRINTING
					// cout<<cache[least_recent_block][cache_addr].data;
					// cout<<", ";
					// cout<<cache[least_recent_block][cache_addr].tag;
					// cout<<", ";
					// cout<<cache[least_recent_block][cache_addr].valid;
					// cout<<", ";
					// cout<<cache[least_recent_block][cache_addr].dirty<<endl;
				}
			}
		}
					// INCREASES THE COUNT OF "NOT ACCESSED" FOR ALL THE NON-EMPTY CACHE BLOCKS
		for (int jk = 0; jk<cache_assoc; jk++) {
			for(int k = 0; k<num_sets; k++) {
				if(cache[jk][k].data != "_") {
					cache[jk][k].T++;
				}
			}
		}
					// MOVE DOWN THE BLOCKS WHICH HAVE NOT BEEN ACCESSED FOR T ACCESSES
		for (int ii = 0; ii<cache_assoc/2; ii++) {
			for(int k = 0; k<num_sets; k++) {
				if (cache[ii][k].T == T) {
					bool has_block = false;														// Check for empty block in Low Priority Level
					int empty_block = -1;
					for(int l = cache_assoc/2; l<cache_assoc; l++) {
						if (cache[l][k].valid == 0) {
							has_block = true;
							empty_block = l;
						}
					}
					if (has_block) {
						cache[empty_block][k] = cache[ii][k];
						cache[ii][k] = Empty_BLK;
					}
					else{																		// If the Low Priority Block does not an empty block
						int least_recent_block = -1;
						int temp_T = -5;
						for (int l = cache_assoc/2; l<cache_assoc; l++) {
							if (cache[l][k].T > temp_T) {
								least_recent_block = l;
								temp_T = cache[l][k].T;
							}
						}
						main_memmory[cache[least_recent_block][k].addr] = cache[least_recent_block][k].data; // Write back in memory before removing the cache block
						cache[least_recent_block][k] = cache[ii][k];							// Move down the cache block from the higher priority group
						cache[ii][k] = Empty_BLK;												// Empty the location in the Higher Priority group
					}
				}
			}
		}
	}

	for(int a = 0; a< cache_assoc; a++) {
		for(int b = 0; b < num_sets; b++) {
			cout<<cache[a][b].data;
			cout<<", ";
			cout<<cache[a][b].tag;
			cout<<", ";
			cout<<cache[a][b].valid;
			cout<<", ";
			cout<<cache[a][b].dirty<<endl;
		}
	}

	cout<<"Cache Statistics:"<<endl;
	cout<<"Number of Accesses = ";
	cout<<index<<endl;
	cout<<"Number of Reads = ";
	cout<<num_reads<<endl;
	cout<<"Read Hits = ";
	cout<<read_hit<<endl;
	cout<<"Read Misses = ";
	cout<<read_miss<<endl;
	cout<<"Number of Writes = ";
	cout<<num_writes<<endl;
	cout<<"Write Hits = ";
	cout<<write_hit<<endl;
	cout<<"Write Misses = ";
	cout<<write_miss<<endl;
	cout<<"Hit ratio = ";
	cout<<(float(write_hit+read_hit))/(write_hit+read_hit+read_miss+write_miss)<<endl;
}
