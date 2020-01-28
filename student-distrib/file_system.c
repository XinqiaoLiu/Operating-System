#include "file_system.h"
#include "system_call.h"
#include "lib.h"

#define EIGHT_MB	0x800000
#define EIGHT_KB	0x2000
bootblock_t* bootblock;

//local helper functions
int filename_compare(char* fname_1, const uint8_t* fname_2);
void filename_copy(char* fname_1, char* fname_2);

/* int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
 * Inputs:  const uint8_t* fname : filename of the file to search
			dentry_t* dentry : the dentry to copy the file info with the filename indicated
 * Return Value: 0 success, -1 failure 
 * Function: copy the file info with the indicated filename to the dentry, called to find and open a file*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
	int i;
	//check empty filename 
	if(fname[0] == '\0')
		return -1;
	for(i=0; i < DENTRY_NUM; i++){
		if(filename_compare((bootblock->dir_entries[i]).file_name,fname)){
			dentry_t target = bootblock->dir_entries[i];
			//copy the file info
			filename_copy(dentry->file_name,target.file_name);
			dentry->file_type = target.file_type;
			dentry->inode_num = target.inode_num;
			return 0;
		}
	}
	return -1;
}

/* int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
 * Inputs:  uint32_t index : directory entry index of the file to search
			dentry_t* dentry : the dentry to copy the file info with the directory entry index indicated
 * Return Value: 0 success, -1 failure 
 * Function: copy the file info with the indicated directory entry index to the dentry, called to list the file in directory*/

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
	//invalid index
	if(index<0 || index>=DENTRY_NUM){
		return -1;
	}
	
	dentry_t target = bootblock->dir_entries[index];
	
	filename_copy(dentry->file_name,target.file_name);
	dentry->file_type = target.file_type;
	dentry->inode_num = target.inode_num;
	return 0;
}


/* int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
 * Inputs:  uint32_t inode : inode index of the file to search
			uint32_t offset : the offset to begin read in the data blocks of this file
			uint8_t* buf : the buffer to copy the data in data blocks with the file indicated of the length indicated
			uint32_t length : the intended length in bytes to read and copy to the buffer
 * Return Value: actual number of bytes read and copy to the buffer 
 * Function: copy the file data of length bytes with the indicated index to the buffer, called to find and open a file*/
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	int i;
	
	int copy_rounds = 1;
	int copy_rounds_counter = 0;
	int rest_bytes;
	uint32_t file_length;
	int start_data_block;
	int start_byte_in_data_block;
	inode_t* target_inode_ptr;
	data_block_t* target_data_block_ptr;
	int target_data_block_index;
	char* byte_ptr;
	//check inode index validity
	//QUESTION HERE : why num_nodes is 64?
	if(inode<0 || inode>= DENTRY_NUM){
		return -1;
	}
	//calculate where to start reading in data blocks of this file
	start_data_block = offset/FOURKB;
	start_byte_in_data_block = offset%FOURKB;
	//calculate the pointers to the inode, data block and start byte; and the data block index in the whole file system
	target_inode_ptr = (inode_t*)((uint32_t)bootblock + (uint32_t)((inode+1)*FOURKB));
	target_data_block_index = *((uint32_t*)((uint32_t)target_inode_ptr + (uint32_t)INODE_PIECE_LEN + (uint32_t)(start_data_block*INODE_PIECE_LEN)));
	target_data_block_ptr = (data_block_t*)((uint32_t)bootblock + (uint32_t)(FOURKB*(bootblock->num_inodes+1)) + (uint32_t)(target_data_block_index*FOURKB));
	byte_ptr =(char*)((uint32_t)target_data_block_ptr + (uint32_t)start_byte_in_data_block);
	
	file_length = target_inode_ptr->length;
	//check offset validity
	if(offset > file_length)
		return -1;
	//check length validity
	if(offset + length > file_length)
		length = file_length - offset;
	
	//calculate the num of data blocks to copy
	if(length == 0)
		return 0;
	//copy to next data block
	rest_bytes = FOURKB - start_byte_in_data_block;
	if(length > rest_bytes){
		copy_rounds += (length - rest_bytes)/FOURKB;
		if((length - rest_bytes)%FOURKB)
			copy_rounds++;
	}
	
	//only one round to copy
	if(copy_rounds==1)
	{
		for(i=0;i<length;i++){
			buf[i] = *(byte_ptr + i);
		}
		return length;
	}
	
	//multiple rounds
	//first round
	for(i=0;i<rest_bytes;i++){
		buf[i]  = *(byte_ptr + i);
	}
	copy_rounds_counter++;
	//copy the middle rounds
	while(copy_rounds_counter!=copy_rounds-1){
		//update 
		start_data_block ++;
		target_data_block_index = *((uint32_t*)((uint32_t)target_inode_ptr + (uint32_t)INODE_PIECE_LEN + (uint32_t)(start_data_block*INODE_PIECE_LEN)));
		target_data_block_ptr = (data_block_t*)((uint32_t)bootblock + (uint32_t)(FOURKB*(bootblock->num_inodes+1)) + (uint32_t)(target_data_block_index*FOURKB));
		byte_ptr =(char*)((uint32_t)target_data_block_ptr + (uint32_t)start_byte_in_data_block);
		//copy the current block
		for(i=0;i<FOURKB;i++){
			buf[rest_bytes + (copy_rounds_counter-1)*FOURKB + i] = *(byte_ptr + i);
		}
		copy_rounds_counter++;
	}
	//the last round
	//update 
	start_data_block ++;
	target_data_block_index = *((uint32_t*)((uint32_t)target_inode_ptr + (uint32_t)INODE_PIECE_LEN + (uint32_t)(start_data_block*INODE_PIECE_LEN)));
	target_data_block_ptr = (data_block_t*)((uint32_t)bootblock + (uint32_t)(FOURKB*(bootblock->num_inodes+1)) + (uint32_t)(target_data_block_index*FOURKB));
	byte_ptr =(char*)((uint32_t)target_data_block_ptr + (uint32_t)start_byte_in_data_block);
	
	for(i=0;i<length-rest_bytes-(copy_rounds_counter-1)*FOURKB;i++){
		buf[rest_bytes + (copy_rounds_counter-1)*FOURKB + i] = *(byte_ptr + i);
		
	}
		
	return length;
}

/* file_open();
 * Inputs:  None
 * Return Value: 0
 * Function: open the file*/
int32_t file_open(const uint8_t* filename){
	return 0;
}

/* file_open();
 * Inputs:  None
 * Return Value: 0
 * Function: close the file*/
int32_t file_close(int32_t fd){
	return 0;
}

/* file_write();
 * Inputs:  None
 * Return Value: -1 (always fail)  
 * Function: None*/
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
	printf("FILE SYSTEM IS UNABLE TO WRITE.");
	return -1;
}

/* file_read();
 * Inputs:  None
 * Return Value: None
 * Function: None */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes){
	if(buf == NULL){
		return -1;
	}
	pcb_t * pcb_ptr;
	
	pcb_ptr = (pcb_t*)(EIGHT_MB - EIGHT_KB * (current_process_idx + 1));
	
	int ret;
	ret = read_data(pcb_ptr->file_descriptor_table[fd].inode, pcb_ptr->file_descriptor_table[fd].file_position, buf, nbytes);
	pcb_ptr->file_descriptor_table[fd].file_position += ret;
	
	return ret;
}
/* print_file_by_name(const uint8_t* fname);
 * Inputs:  const uint8_t* fname : filename of the file to print
 * Return Value: None
 * Function: print the data of the file indicated by the filename*/
void print_file_by_name(const uint8_t* fname){
	
	dentry_t dentry;
	clear_dentry(&dentry);
	int file_length;
	uint32_t inode_index;
	inode_t* inode_ptr;
	int read_data_result;
	int read_file_result = read_dentry_by_name(fname,&dentry);
	if(read_file_result == -1){
		printf("FILE DOES NOT EXIST");
		return;
	}
	//check file type
	if(dentry.file_type != REGULAR_FILE){
		printf("NOT REGULAR FILE");
		return;
	}
	inode_index = dentry.inode_num;
	inode_ptr = (inode_t*)((uint32_t)bootblock + (uint32_t)((inode_index+1)*FOURKB));
	file_length = inode_ptr->length;
	uint8_t buf[file_length];
	read_data_result = read_data(inode_index,0,buf,file_length);
	if(read_data_result == -1){
		printf("INVALID INODE");
		return;
	}
	
	//print the data in file
	printf("%s",buf);
}

/* print_file_by_idx(int file_idx);
 * Inputs:  int file_idx : index of the file to print
 * Return Value: None
 * Function: print the data of the file indicated by the index*/
 
void print_file_by_idx(int file_idx){
	
	dentry_t dentry;
	inode_t* inode_ptr;
	int read_data_result;
	int file_length;
	if(read_dentry_by_index(file_idx, &dentry) == -1){
		printf("INVALID INDEX");
	}
	else{
		inode_ptr = (inode_t*)((uint32_t)bootblock + (uint32_t)((dentry.inode_num+1)*FOURKB));
		file_length = inode_ptr->length;
		uint8_t buf[file_length];
		read_data_result = read_data(dentry.inode_num,0,buf,file_length);
		if(read_data_result == -1){
			printf("INVALID INODE");
			return;
		}
	
		//print the data in file
		printf("%s",buf);
		}
}

/* open_directory();
 * Inputs:  None
 * Return Value: 0
 * Function: open the directory*/
int32_t open_directory(const uint8_t* filename){
	return 0;
}

/* close_directory();
 * Inputs:  None
 * Return Value: 0
 * Function: close the directory*/
int32_t close_directory(int32_t fd){
	return 0;
}

/* list_directory();
 * Inputs:  None
 * Return Value: 0 when success, -1 when fail
 * Function: print the file info of the file in the directory*/

int32_t list_directory(){
	int i;
	for(i=0;i<bootblock->num_dir_entries;i++){
		dentry_t dentry;
		if(read_dentry_by_index(i, &dentry) == -1){
			printf("INVALID INDEX");
			return -1;
		}
		else{
			print_dentry(&dentry);
		}
			
	}
	// Return success.
	return 0;
}


/* read_directory();
 * Inputs:  None
 * Return Value: 0 when reach the end.
 * Function: print the file info of the file in the directory*/
int32_t read_directory(int32_t fd, void* buf, int32_t nbytes){
	if(buf == NULL)
		return -1;
	
	dentry_t dentry;
	int i;
	pcb_t* pcb;
	int file_idx;
	pcb = (pcb_t*)(EIGHT_MB - EIGHT_KB * (current_process_idx + 1));			

	file_idx = pcb->file_descriptor_table[fd].file_position;							
	
	if (read_dentry_by_index(file_idx, &dentry) == -1)	{							
		pcb->file_descriptor_table[fd].file_position = 0;
		return 0;
	}
	uint8_t* ptr;
	ptr = (uint8_t*)buf;
	int len;
	if (nbytes < strlen(dentry.file_name)){
		len = nbytes;
	}
	else{
		len = strlen(dentry.file_name);
	}
	for (i = 0; i < len ; i++) {										
		ptr[i] = dentry.file_name[i];
	}
	pcb->file_descriptor_table[fd].file_position += 1;

	return len;
}

/* write_directory();
 * Inputs:  None
 * Return Value: -1 (always fail)  
 * Function: None*/
int32_t write_directory(int32_t fd, const void* buf, int32_t nbytes){
	printf("FILE SYSTEM IS UNABLE TO WRITE.");
	return -1;
}


/* void print_dentry(dentry_t* dentry);
 * Inputs:  dentry_t* dentry : the dentry containing the file info 
 * Return Value: None
 * Function: print the file info of the file in the dentry*/
void print_dentry(dentry_t* dentry){
	int i;
	int j;
	inode_t* inode_ptr = (inode_t*)((uint32_t)bootblock + (uint32_t)((dentry->inode_num+1)*FOURKB));
	int file_size = inode_ptr->length;
	int file_size_copy = file_size;
	printf("file_name:");
	for(i = 0; i<FILENAME_LEN; i++){
		if((dentry->file_name)[i] == '\0')
			break;
	}
	for(j =0; j < SPACE_33-i; j++){
		printf(" ");
	}
	if(i==FILENAME_LEN){
		char buf[FILENAME_LEN+1];
		filename_copy(buf,dentry->file_name);
		buf[i] = '\0';
		printf("%s",buf);
	}
	else{
		printf("%s",dentry->file_name);
	}
	printf(", file_type: ");
	printf("%d",dentry->file_type);
	printf(", file_size:");
	i = 0;
	while(file_size_copy/DECI != 0){
		i ++;
		file_size_copy /= DECI;
	}
	for(j=0;j<SPACE_6-i;j++){
		printf(" ");
	}
	printf("%d", file_size);

	printf("\n");
	
	

}

/* void clear_dentry(dentry_t* dentry);
 * Inputs:  dentry_t* dentry : the newly created dentry 
 * Return Value: None
 * Function: clears the filename part of the dentry*/
void clear_dentry(dentry_t* dentry){
	int i;
	for(i=0;i<FILENAME_LEN;i++){
		dentry->file_name[i] = '\0';
	}
	
}

/* int filename_compare(char* fname_1, const uint8_t* fname_2);
 * Inputs:  char* fname_1 : the first filename
			char* fname_2 : the second filename
 * Return Value: 1 the same, 0 different
 * Function: compare the two file name if the same*/
int filename_compare(char* fname_1, const uint8_t* fname_2){
	int i;
	for(i=0; i<FILENAME_LEN; i++){
		if(fname_1[i] == '\0' && fname_2[i] == '\0')
			return 1;
		if(fname_1[i] != fname_2[i]){
			return 0;
		}
	}
	
	return 1;	
}
/* void filename_copy(char* fname_1, char* fname_2);
 * Inputs:  char* fname_1 : the first filename ,to copy
			char* fname_2 : the second filename, to be copied
 * Return Value: None
 * Function: copy the second filename to the first filename*/
void filename_copy(char* fname_1, char* fname_2){
	int i;
	for(i = 0; i< FILENAME_LEN; i++){
		
		fname_1[i] = fname_2[i];
		if(fname_2[i] == '\0')
			break;
	}
}

