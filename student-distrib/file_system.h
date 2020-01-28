#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#include "types.h"
#include "lib.h"

#define FILENAME_LEN	32
#define DENTRY_RESVD_LEN	24
#define BL_RESVD_LEN	52
#define DENTRY_NUM     63
#define DB_NUM		1023
#define FOURKB     4096
#define INODE_PIECE_LEN 4
#define REGULAR_FILE	2
#define SPACE_33	33
#define SPACE_6		6
#define DECI		10

typedef struct directory_entry {
	char file_name[FILENAME_LEN];
	uint32_t file_type;
	uint32_t inode_num;
	uint8_t reserved[DENTRY_RESVD_LEN];	
} dentry_t;
typedef struct boot_block {
    uint32_t num_dir_entries;
    uint32_t num_inodes;
    uint32_t num_data_blocks;
    uint8_t reserved[BL_RESVD_LEN];
    dentry_t dir_entries[DENTRY_NUM];
} bootblock_t;
typedef struct index_node {
	uint32_t length;
	uint32_t data_block_num[DB_NUM];
} inode_t;
typedef struct data_block {
	uint8_t file_data[FOURKB];
} data_block_t;

bootblock_t* bootblock;
int32_t list_directory();
void print_dentry(dentry_t* dentry);
void clear_dentry(dentry_t* dentry);
void print_file_by_name(const uint8_t* fname);
void print_file_by_idx(int file_idx);
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

int32_t read_directory(int32_t fd, void* buf, int32_t nbytes);
int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes);
int32_t open_directory(const uint8_t* filename);
int32_t close_directory(int32_t fd);

int32_t file_read(int32_t fd, void* buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t file_open(const uint8_t* filename);
int32_t file_close(int32_t fd);

#endif


