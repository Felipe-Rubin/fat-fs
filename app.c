/*
	PUCRS Computer Science 2018/1
	Student: Felipe Pfeifer Rubin
	Email: felipe.rubin@acad.pucrs.br
	--
	Student: Ian Aragon Escobar
	Email: ian.escobar@acad.pucrs.br
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include<stdarg.h>

/* ANSI COLORS */
/* Based in*/
/* https://stackoverflow.com/questions/3219393/stdlib-and-colored-output-in-c */
#define COLORS /* Undefine for standard output */
#ifdef COLORS
#define ACBLACK   "\x1b[30m"
#define ACRED     "\x1b[31m"
#define ACGREEN   "\x1b[32m"
#define ACYELLOW  "\x1b[33m"
#define ACBLUE    "\x1b[34m"
#define ACMAGENTA "\x1b[35m"
#define ACCYAN    "\x1b[36m"
#define ACRESET   "\x1b[0m"
#else
#define ACBLACK   ""
#define ACRED     ""
#define ACGREEN   ""
#define ACYELLOW  ""
#define ACBLUE    ""
#define ACMAGENTA ""
#define ACCYAN    ""
#define ACRESET   ""
#endif

// static char* current_dir = "DIR:/ ";
static char* current_dir = "";

#define USER_LINE() printf("%ssh" ACGREEN "$" ACRESET" ",current_dir);
#define ROOT_LINE()   printf("%sroot" ACBLACK "#" ACRESET" ",current_dir);
#define CUSTOM_PRINTF(AC,fmt_, ...) printf((AC fmt_ ACRESET),##__VA_ARGS__);
#define ERROR(fmt_, ...){printf((ACRED fmt_ ACRESET),##__VA_ARGS__);exit(-1);}
#define INFO(fmt_, ...) printf((ACBLUE fmt_ ACRESET),##__VA_ARGS__);
#define WARNING(fmt_, ...) printf((ACMAGENTA fmt_ ACRESET),##__VA_ARGS__);
// #define WDEBUG
/*WDEBUG  = WITH DEBUG*/
#ifdef DEBUGGING /* Define for printing DEBUG */
#define DEBUG(fmt_, ...)printf((ACCYAN fmt_ ACRESET),##__VA_ARGS__); 
#else
#define DEBUG(fmt_, ...)
#endif
#define SHELL_PRINTF(fmt_, ...) printf((ACBLACK "-shell: " ACRESET" " fmt_) ,##__VA_ARGS__);


/* EXCEPTIONS DEFINITION */
#define fatal_error 0x00
#define permission_denied 0x01
#define filesystem_not_loaded 0x02
#define filesystem_not_found 0x03
#define invalid_directory 0x04
#define file_exists 0x05
#define memory_corrupted 0x06
#define memory_full 0x07
#define directory_full 0x08
#define directory_not_empty 0x09
#define exception(fmt_)\
switch(fmt_){\
	case permission_denied: SHELL_PRINTF("Permission denied\n");break;\
	case fatal_error: ERROR("Fatal Error, halting\n");break;\
	case filesystem_not_loaded: WARNING("No file system loaded\n");break;\
	case filesystem_not_found: WARNING("No file system found\n");break;\
	case invalid_directory: WARNING("Not a valid directory\n");break;\
	case file_exists: SHELL_PRINTF("File exists\n");break;\
	case memory_corrupted: WARNING("File system appears to be corrupted\n");break;\
	case memory_full: WARNING("Not enought free memory\n");break;\
	case directory_full: WARNING("Directory full\n");break;\
	case directory_not_empty: WARNING("Directory not empty\n");break;\
	default: SHELL_PRINTF("Error has occured\n");break;\
\
};

/* First part was based in */
/* https://stackoverflow.com/questions/16870485/how-can-i-read-an-input-string-of-unknown-length */
/*
	Reads from Command Line
	1 - Read all input into a buffer untill new line
	2 - Reallocates in array of arguments
	3 - Free buffer
	4 - return arguments
 */
char** inputscanf(int *argc,FILE *fp)
{
	char *str;
	int ch;
	size_t size = 32; // First time
	size_t len = 0;
	str = realloc(NULL,sizeof(char) * size);
	if(!str){
		if(size == 0) ERROR("Not enought memory to read input\n");
		return NULL;
	}
	int maxlen = 0;
	int currentlen = 0;
	*argc = 1;
	int string_mode = 0;
	while(EOF != (ch=fgetc(fp)) && ch != '\n'){
		if(string_mode){
			if(ch == '\"'){ //turn off now
				string_mode = 0;
			}else{
				currentlen++;
				str[len++] = ch;
				if(len == size){
					str = realloc(str,sizeof(char)*(size+=32));
				}
			}
		}else if(ch == '\"'){
			string_mode = 1;
		}else if(ch == '\\'){ //white space
			currentlen++;
			str[len++] = fgetc(fp);
			if(len == size){
				str = realloc(str,sizeof(char)*(size+=32));
			}
		}else if(ch == ' '){ //new argument
			(*argc)++;
			if(currentlen > maxlen)maxlen = currentlen;
			currentlen = 0;
			str[len++] = '\0';
			if(len == size) str = realloc(str,sizeof(char)*(size+=32));
		}else{
			currentlen++;
			str[len++] = ch;
			if(len == size){
				str = realloc(str,sizeof(char)*(size+=32));
			}
		}
	}
	if(currentlen > maxlen)maxlen = currentlen;
	// DEBUG("MAXLEN = %d\n",maxlen);
	// DEBUG("CURRENTLEN = %d\n",currentlen);
	// DEBUG("ARGC = %d\n",*argc);
	str[len++] = '\0';
	char **ret = malloc(sizeof(char*) * (*argc));
	int i = 0;
	int j = 0;
	for(i = 0; i < *argc; i++){
		ret[i] = malloc(sizeof(char) * (maxlen+1));
		int arg_len  = 0;
		while(str[j] != '\0'){
			ret[i][arg_len] = str[j];
			// DEBUG("{ret[%d][%d]:%c,str[%d]:%c}\n",i,arg_len,ret[i][j],j,str[j]);
			j++;
			arg_len++;
		}
		while(arg_len <= maxlen){
			ret[i][arg_len] = '\0';
			// DEBUG("ret[%d][%d], arg_len = %d\n",i,arg_len,arg_len);
			arg_len++;
		}
		j++;
	}
	free(str);
	for(i = 0; i < *argc; i++){
		DEBUG("arg[%d]: %s\n",i,ret[i]);
	}
	return ret;
}

/* Definition of SIZES in Bytes */ 

#define SECTOR_SIZE 512
#define CLUSTER_SIZE 2 * SECTOR_SIZE

/* directory entry, 32 bytes cada */
/* attributes 0x00 - file , 0x01 - directory */
typedef struct{
    uint8_t filename[18];
    uint8_t attributes;
    uint8_t reserved[7];
    uint16_t first_block;
    uint32_t size;
} dir_entry_t;


/* 8 clusters at FAT table, 4096 entries of 16 bits = 8192 bytes*/

uint16_t fat[4096];
/* directories (ROOT included), 32 directory entries
with 32 bytes each = 1024 bytes or data block of 1024 bytes*/
union{
    dir_entry_t dir[CLUSTER_SIZE / sizeof(dir_entry_t)];
    uint8_t data[CLUSTER_SIZE];
} data_cluster;

/*
	n = how many clusters
	locations[n] = position of each free cluster
	Return code:
	- 0x00: Not enough free memory
	- 0x01: Found n required clusters
 */
uint8_t find_empty_clusters(int n,uint16_t* locations)
{
	uint16_t i = 0x000A;
	uint16_t count = 0x0000;
	for(i = 0x000A; i < 0x1000; i++){
		if(fat[i] == 0x0000){
			locations[count] = i;
			count+= 0x0001;
			DEBUG("find_empty_clusters: found 1: %04x, count: %04x\n",i,count);
			if(count == n)break;
		}
	}
	if(count == n) return 0x01;
	return 0x00;
}

/*
	return
	- 0x00: failure
	- 0x01: success
 */
uint8_t update_fat(uint16_t index) //write_fat
{
	DEBUG("update_fat: starting..\n");
	FILE *file = fopen("./fat.part","rb+");
	if(file == NULL) return 0x00;
	DEBUG("update_fat: fat[%04x]",index);
	uint32_t idx = index;
	DEBUG("update_fat: idx = %d\n",idx);
	DEBUG("update_fat: fat jump is %lu\n",(sizeof(uint16_t)*idx));

	idx = floor(idx/512.0);
	DEBUG("update_fat: true idx = %d\n",idx);
	fseek(file,CLUSTER_SIZE*(1+idx),SEEK_SET);
	DEBUG("update_fat: after fseek %lu\n",ftell(file));
	DEBUG("update_fat: fat[1] = 0x%04x\n",fat[0]);
	if(fwrite((fat+idx),CLUSTER_SIZE,1,file) == 1){
		fclose(file);
		return 0x01;
	}

	DEBUG("update_fat: after fseek %lu\n",ftell(file));

	fclose(file);
	return 0x00;
}
/*
	index: the cluster index, e.g. 0x0009 is the root dir cluster
	return:
	- 0x00: failure
	- 0x01: success

 */
uint8_t update_cluster(uint16_t index) //write_cluster
{
	FILE *file = fopen("./fat.part","rb+");
	if(file == NULL) return 0x00;
	uint32_t idx = index;
	// fseek(file,CLUSTER_SIZE+(sizeof(uint16_t)*idx),SEEK_SET);
	DEBUG("update_cluster: id = %d\n",idx);
	fseek(file,CLUSTER_SIZE*idx,SEEK_SET);
	DEBUG("update_cluster: after fseek %lu\n",ftell(file));

	if(fwrite(&data_cluster.data,sizeof(data_cluster),1,file) == 1){
		fclose(file);
		return 0x01;
	}
	fclose(file);
	return 0x00;
}
/*
	index: the cluster index, e.g. 0x0009 is the root dir cluster
	return:
	- 0x00: failure
	- 0x01: success

 */
uint8_t read_cluster(uint16_t index)
{
	DEBUG("read_cluster: index = 0x%04x\n",index);
	FILE *file = fopen("./fat.part","rb+");
	if(file == NULL) return 0x00;
	uint32_t idx = index;
	DEBUG("read_cluster: id = %d\n",idx);
	fseek(file,CLUSTER_SIZE*idx,SEEK_SET);
	DEBUG("read_cluster: after fseek %lu\n",ftell(file));
	if(fread(&data_cluster.data,sizeof(data_cluster),1,file) == 1){
		fclose(file);
		return 0x01;
	}
	fclose(file);
	return 0x00;
}

/*
	Code:
	- 0x00: Not found
	- 0x01: Found
 */
uint8_t walk_directory(char *path,uint8_t* dir_index)
{
	// FILE *file = fopen("./fat.part","rb");
	// if(file == NULL){
	// 	SHELL_PRINTF("No file system found\n")
	// 	return 0;
	// };
	
	DEBUG("walk_directory(%s)\n",path);
	// uint8_t dir_index = 0x0000;
	// uint8_t *ptr = (char*)data_cluster;
	uint8_t first_empty_dir = 0x20;
	for(*dir_index = 0x0000; *dir_index < CLUSTER_SIZE / sizeof(dir_entry_t); *dir_index+=0x01){
		// DEBUG("walk_directory: dir_index = %02x\n",*dir_index);
		// DEBUG("dir_entry_t %s\n",data_cluster.dir[*dir_index].filename);
		/* Found Directory */
		//uint16_t first_block = ((uint16_t*)&data_cluster.dir[*dir_index])[13];

		if(data_cluster.dir[*dir_index].first_block == 0x0000 && first_empty_dir == 0x20){
			// *dir_index = 0x0000;
			// DEBUG("walk_directory: empty_directory found\n");
			// return 0x00;
			first_empty_dir = *dir_index;
			// break;
		}; /* 32, a.k.a not found */
		// DEBUG("first_block = %04x\n",data_cluster.dir[*dir_index].first_block);
		// if(memcmp(data_cluster.dir[*dir_index].filename,path,sizeof(uint8_t) * 18) == 0){
		if(memcmp(data_cluster.dir[*dir_index].filename,path,sizeof(char)*strlen(path)) == 0){
			DEBUG("walk_directory: found %s at index %02x\n",data_cluster.dir[*dir_index].filename,*dir_index);
			break;
		}
	}
	if(*dir_index == 0x20){ /* Didn't find directory */
		*dir_index = first_empty_dir;
		return 0x00;
	}
	/* Found Directory */
	return 0x01;
}
/*
	Path: /documents/files/notes.txt
	fat_entry: last valid FAT[fat_entry]
	dir_entry: available dir entry(if, it is less than 32);
	returns: 
	8 bits which are the following:
	- 0x00: fat_index is an unknown level above the path
	- 0x01: fat_index is the respective path
	- 0x02: fat_index is one level above the path
	- 0x03: No file system loaded
	- 0x04: No file system found
	- 0x05: corrupted file system
	- 0x06: Permission Denied to access location
 */
uint8_t walk_path(char* path, uint16_t* fat_index,uint16_t* fat_parent_index,uint8_t* dir_index,uint8_t *filename)
{
	
	if(fat[0] != 0xfffd) return 0x03;

	char* token;

	FILE *file = fopen("./fat.part","rb+");

	if(file == NULL) return 0x04;

	/* Starts at root cluster */
	fseek(file,9*CLUSTER_SIZE,SEEK_SET);
	fread(&data_cluster.dir,CLUSTER_SIZE,1,file);
	DEBUG("walk_path: Attributes = %04x\n",data_cluster.dir[0].attributes);
	// *fat_entry = data_cluster.dir[0].first_block;
	*fat_index = 0x0009;
	*fat_parent_index = 0x0009; //First parent is 0x0009
	*dir_index = 0x00;

	token = strtok(path,"/");
	int break_while = 0;
	uint8_t try_dir_index = 0x0000;
	// uint16_t try_fat_index = 0x0000;
	uint16_t try_fat_index = 0x0009; /* root dir index */
	while(token){

		DEBUG("token{len:%lu}: %s\n",strlen(token),token);
		// uint16_t fat_pos = walk_directory(token,dir_index);
		/* rc = result code */
		// uint8_t try_dir_index = 0x0000;
		try_dir_index = 0x00;
		*fat_parent_index = *fat_index;
		uint8_t rc = walk_directory(token,&try_dir_index);
		// if(try_dir_index >= 0x0020){ // >= 32

		if(rc == 0x00){ //Not Found
			*dir_index = try_dir_index; //TESTING
			DEBUG("walk_path: Not found BEFORE {fat_index:0x%04x,try_fat_index:0x%04x}\n",*fat_index,try_fat_index);
			if(try_fat_index != 0x0000){
				// DEBUG("walk_path: 0x%04x != 0, fat_index <- try_fat_index\n",*try_fat_index);
				*fat_index = try_fat_index;
			}
			DEBUG("walk_path: Not found AFTER {fat_index:0x%04x,try_fat_index:0x%04x}\n",*fat_index,try_fat_index);
			DEBUG("walk_path(%s),NOT FOUND\n",token);
			break;
		}else{
			DEBUG("Before else{dir_index:0x%02x,fat_index:0x%04x}\n",*dir_index,*fat_index);
			*dir_index = try_dir_index;
			*fat_index = try_fat_index;
			DEBUG("After else{dir_index:0x%02x,fat_index:0x%04x}\n",*dir_index,*fat_index);
			//tem q testar se eh inicio de arquivo!
			DEBUG("walk_path: else{Name:%s,First_Block:0x%04x,Attributes:0x%02x}\n",data_cluster.dir[*dir_index].filename,data_cluster.dir[*dir_index].first_block,data_cluster.dir[*dir_index].attributes);
			// if(data_cluster.dir[*dir_index].start)
			if(data_cluster.dir[*dir_index].attributes == 0x00){
				// DEBUG("walk_path: MATE, that's a file, won't use try_fat_index = 0x%04x\n",try_fat_index);
				DEBUG("walk_path: MATE, that's a file\n");
				break;
			}
			// *fat_index = try_fat_index;
		}

		////////

		////////
		// uint16_t try_fat_index = data_cluster.dir[*dir_index].first_block;
		try_fat_index = data_cluster.dir[*dir_index].first_block;

		// *fat_index = try_fat_index;

		// uint16_t *fat_entry = fat_pos;
		/* 0x0009 because [0.boot_block,1-7.FAT]*/
		// uint16_t fat_entry = fat[*fat_index] + 0x0009;
		uint16_t fat_entry = fat[try_fat_index];
		switch(fat_entry){
			case 0xfffd: case 0xfffe: fclose(file);return 0x06;
			// case 0xffff: break_while = 1;break; //what to do ?
			case 0x0000: break_while = 2;break;
			default: break;
		}
		/* Need to break, occurs in special cases */
		if(break_while == 1){
		 	
		 	DEBUG("walk_path(%s),SPECIAL CASE #1\n",token);
		 	token = strtok(NULL,"/");
		 	uint32_t fat_idx = *fat_index;
		 	fseek(file,CLUSTER_SIZE * fat_idx,SEEK_SET);
			if(fread(&data_cluster.dir,CLUSTER_SIZE,1,file) != 1){
				fclose(file);
				return 0x05;
			}
		 	break;
		}else if(break_while == 2){
		 	DEBUG("walk_path(%s),SPECIAL CASE #2\n",token);
		 	break;
		}
		// *fat_index = try_fat_entry;
		// fseek(file,CLUSTER_SIZE*((*fat_entry)+0x0009),SEEK_SET);
		// fread_cluster(&data_cluster.dir,(*fat_entry),file);
		
		// uint32_t fat_idx = *fat_index; // 9 - fat[9] = 0xffff
		uint32_t fat_idx = try_fat_index;

		uint32_t fat_ety = fat_entry; //fat[idx] = fat_ety
		// fseek(file,CLUSTER_SIZE*(fat_entry),SEEK_SET);

		DEBUG("walk_path: before fseek %lu\n",ftell(file));
		DEBUG("walk_path: going to seek cluster %d\n",fat_idx);
		// fseek(file,CLUSTER_SIZE*(fat_ety),SEEK_SET);
		fseek(file,CLUSTER_SIZE*(fat_idx),SEEK_SET);
		DEBUG("walk_path: after fseek %lu\n",ftell(file));
		if(fread(&data_cluster.dir,CLUSTER_SIZE,1,file) != 1){
			fclose(file);
			return 0x05;
		}
		/* Ensures that after memcpy(), no trash remains from a previous one */
		memset(filename,0x00,18); 
		memcpy(filename,token,sizeof(char)*strlen(token));
		//
		token = strtok(NULL,"/");
	}
	fclose(file);

	if(!token){ //Token is NULL
		DEBUG("walk_path: ok,found 0x1; {Name:%s,First_Block:0x%04x,Attributes:0x%02x}\n",data_cluster.dir[*dir_index].filename,data_cluster.dir[*dir_index].first_block,data_cluster.dir[*dir_index].attributes);
		return 0x01;
	}else{
		DEBUG("walk_path: ELSE token = %s\n",token);
		DEBUG("walk_path: ELSE filename before = %s\n",(char*)filename);
		memset(filename,0x00,18); 
		memcpy(filename,token,sizeof(char)*strlen(token));
		DEBUG("walk_path: ELSE filename after = %s\n",(char*)filename);
		token = strtok(NULL,"/");
		if(!token){
			return 0x02;
		}else{
			DEBUG("walk_path: remaining token = %s\n",token);
			return 0x00;
		}
	}
}
/*
	All Commands return 1 for success, 0 otherwise.
 */


/* init - inicializar o sistema de arquivos 
com as estruturas de dados, semelhante a 
formatar o sistema de arquivos virtual */
int init(){
	INFO("Creating fat.part\n");
	/* This are required as they may contain trash */
	
	memset(fat,0x0000,4096); //Write 0s

	FILE *file = fopen("./fat.part","wb");
	if(file == NULL) ERROR("Error could not create file\n");

	/* Writes Boot Block (Cluster) */
	memset(data_cluster.data,0xbb,CLUSTER_SIZE);
	uint32_t valid = 0;
	uint32_t i;
	valid = fwrite(&data_cluster.data,CLUSTER_SIZE,1,file);
	if(!valid){
		fclose(file);
	 	ERROR("Error writing boot block\n");
	}

	/* Write First FAT Cluster */
	memset(data_cluster.data,0x00,CLUSTER_SIZE);
	uint16_t fat_boot = 0xfffd;
	memcpy(&data_cluster.data[0],&fat_boot,sizeof(uint16_t));
	uint16_t fat_fat = 0xfffe;
	for(i = 0; i < 8;i++)
		memcpy(&(data_cluster.data[2+(i*2)]),&fat_fat,sizeof(uint16_t));
	data_cluster.data[18] = 0xff; data_cluster.data[19] = 0xff;
	valid = fwrite(&data_cluster.data,CLUSTER_SIZE,1,file);

	if(!valid){
		fclose(file);
		ERROR("Error writing FAT boot\n");
	}
	/*Write Remaining Clusters (7 FAT + 1 ROOT + 4086) */
	memset(&data_cluster.data[0],0x00,20);
	valid = 0;
	for(i = 0; i < 4093; i++) valid += fwrite(&data_cluster.data,CLUSTER_SIZE,1,file);
	if(valid != 4093){
		fclose(file);
		ERROR("Error writing FAT fat\n");
	}
	fclose(file);
	return 1;
}
/*
load - carregar o sistema de arquivos do disco
*/
int load(){
	INFO("Booting File System\n");
	FILE *file = fopen("./fat.part","rb");
	if(file == NULL){
		SHELL_PRINTF("No file system found\n")
		return 0;
	};
	union data_cluster;
	uint32_t valid = 0;
	uint32_t i = 0;
	/* read boot block */
	valid = fread(data_cluster.data,CLUSTER_SIZE,1,file);
	if(valid != 1){
		fclose(file);
		exception(memory_corrupted);
		return 0;
	}
	/* Check for boot block integrity */
	for(i = 0; i < CLUSTER_SIZE; i++) 
		if(data_cluster.data[i] != 0xbb){
			fclose(file);
			exception(memory_corrupted);
			return 0;
		}
	/* read FAT */
	valid = 0;
	for(i = 0; i < 8; i++){
		valid+= fread((fat+(i*SECTOR_SIZE)),CLUSTER_SIZE,1,file);
	}
	DEBUG("update_fat: fat[0] = 0x%04x\n",fat[0]);
	DEBUG("update_fat: fat[1] = 0x%04x\n",fat[1]);

	if(valid != 8){
		fclose(file);
	 	exception(memory_corrupted);
		return 0;
	}

	/* read root dir */
	DEBUG("POS before reading root dir = %lu\n",ftell(file));
	valid = fread(&data_cluster.dir,CLUSTER_SIZE,1,file);
	if(valid != 1){
		fclose(file);
		exception(memory_corrupted);
		return 0;
	}
	fclose(file);

	return 1;
}
/* ls [/caminho/diretorio] - listar diretorio */
int ls(const int arg_count,char** arg_value){
	uint16_t fat_entry = 0x0000;
	uint16_t fat_parent_entry = 0x0000;
	uint8_t dir_index = 0x00;
	uint8_t dir_name[18];
	memset(dir_name,0x0000,18*sizeof(uint8_t)); //Write 0s
	if(arg_count > 1){
		/* rc = result code */
		uint8_t rc = walk_path(arg_value[1],&fat_entry,&fat_parent_entry,&dir_index,dir_name);
		DEBUG("ls: rc = %02x\n",rc);
		switch(rc){
			case 0x00: case 0x02: exception(invalid_directory); return 0;
			case 0x01: break;
			case 0x03: exception(filesystem_not_loaded);return 0;
			case 0x04: exception(filesystem_not_found);return 0;
			case 0x05: exception(memory_corrupted);return 0;
			default: exception(fatal_error);return 0;
		}
		DEBUG("ls: fat_entry = 0x%04x\n",fat_entry);
		DEBUG("ls: dir_index = 0x%02x\n",dir_index);
		 

		// 	If it's here, it is ONE LEVEL ABOVE
		// 	This a special rule

		// 	This only happens if there's no file in the given directory
		 
		
		// if(data_cluster.dir[dir_index].first_block == 0x0000){
		// 	DEBUG("ls: First_block appears to be 0000\n");
		// 	return 0;
		// }
		/* If it is a directory */
		// if(data_cluster.dir[dir_index].attributes == 0x01){
		// 	DEBUG("ls: Was Directory\n");
		// 	read_cluster(data_cluster.dir[dir_index].first_block);
		// }else{
		// 	DEBUG("ls: Wasn't Directory\n");
		// }
		// read_cluster(data_cluster.dir[dir_index].first_block);

		DEBUG("ls: FIRST_BLOCK = 0x%04x\n",data_cluster.dir[dir_index].first_block);
		/* If it got here, means the path was found successfully */
		int i;
		for(i = 0; i < CLUSTER_SIZE / sizeof(dir_entry_t); i++){
			if(((uint16_t*)&data_cluster.dir[i])[13] == 0x0000){
				// break;
				continue;
			}
			CUSTOM_PRINTF(ACBLACK,"%s",data_cluster.dir[i].filename);
			if(data_cluster.dir[i].attributes == 0x01)
				CUSTOM_PRINTF(ACBLACK,"/");
			CUSTOM_PRINTF(ACBLACK,"\n");
		}
	}
	return 1;
}
/* mkdir [/caminho/diretorio] - criar diretorio */
int mkdir(const int arg_count,char** arg_value){
	uint16_t fat_entry = 0x0000;
	uint16_t fat_parent_entry = 0x0000;
	uint8_t dir_index = 0x00;
	uint8_t dir_name[18];
	memset(dir_name,0x0000,18*sizeof(uint8_t)); //Write 0s
	if(arg_count > 1){
		uint8_t rc = walk_path(arg_value[1],&fat_entry,&fat_parent_entry,&dir_index,dir_name);
		DEBUG("mdkdir: rc = 0x%02x\n",rc);
		DEBUG("mkdir: fat_entry = 0x%04x\n",fat_entry);
		DEBUG("mkdir: dir_index = 0x%02x\n",dir_index);
		switch(rc){
			case 0x00: exception(invalid_directory); return 0;
			case 0x01: exception(file_exists);return 0; //already exists
			case 0x02: break; //one level above
			case 0x03: exception(filesystem_not_loaded);return 0;
			case 0x04: exception(filesystem_not_found);return 0;
			case 0x05: exception(memory_corrupted);return 0;
			default: exception(fatal_error);return 0;
		}

		if(data_cluster.dir[dir_index].first_block != 0x0000){
			exception(file_exists);
			return 0;
		}

		uint16_t empty_index = 0x0000;
		rc = find_empty_clusters(1,&empty_index);

		if(rc == 0x00){
			exception(memory_full);
			return 0;
		}
		DEBUG("mkdir: trying to create directory %s\n",(char*)dir_name)
		DEBUG("mkdir: empty_index is 0x%04x\n",empty_index);
		if(dir_index < 0x20){//Empty space still
			dir_entry_t new_dir = {
				.filename = "",
				.attributes=0x01,
				.reserved = 0x00000000000000,
				.first_block = empty_index,
				.size = 0x00000000
			};
			memcpy(new_dir.filename,dir_name,sizeof(char) * 18);
			fat[empty_index] = 0xffff;
			rc = update_fat(empty_index);
			if(rc == 0x00){
				exception(fatal_error);
				return 0;
			}
			data_cluster.dir[dir_index] = new_dir;
			update_cluster(fat_entry);
		}else{
			exception(directory_full);
			return 0;
		}
	}
	return 1;
}
/* create [/caminho/arquivo] - criar arquivo */
int create(const int arg_count,char** arg_value){
	uint16_t fat_entry = 0x0000;
	uint16_t fat_parent_entry = 0x0000;
	uint8_t dir_index = 0x00;
	uint8_t dir_name[18];
	memset(dir_name,0x0000,18*sizeof(uint8_t)); //Write 0s
		if(arg_count > 1){
		uint8_t rc = walk_path(arg_value[1],&fat_entry,&fat_parent_entry,&dir_index,dir_name);
		DEBUG("create: rc = 0x%02x\n",rc);
		DEBUG("create: fat_entry = 0x%04x\n",fat_entry);
		DEBUG("create: dir_index = 0x%02x\n",dir_index);
		switch(rc){
			case 0x00: exception(invalid_directory); return 0;
			case 0x01: exception(file_exists);return 0; //already exists
			case 0x02: break; //one level above
			case 0x03: exception(filesystem_not_loaded);return 0;
			case 0x04: exception(filesystem_not_found);return 0;
			case 0x05: exception(memory_corrupted);return 0;
			default: exception(fatal_error);return 0;
		}

		if(data_cluster.dir[dir_index].first_block != 0x0000){
			exception(file_exists);
			return 0;
		}

		uint16_t empty_index = 0x0000;
		rc = find_empty_clusters(1,&empty_index);

		if(rc == 0x00){
			exception(memory_full);
			return 0;
		}
		DEBUG("create: empty_index is 0x%04x\n",empty_index);
		if(dir_index < 0x20){//Empty space still
			dir_entry_t new_dir = {
				.filename = "",
				.attributes=0x00, //A file
				.reserved = 0x00000000000000,
				.first_block = empty_index,
				.size = 0x00000000
			};
			memcpy(new_dir.filename,dir_name,sizeof(char) * 18);
			fat[empty_index] = 0xffff;
			rc = update_fat(empty_index);
			if(rc == 0x00){
				exception(fatal_error);
				return 0;
			}
			data_cluster.dir[dir_index] = new_dir;
			update_cluster(fat_entry);			

		}else{
			exception(directory_full);
			return 0;
		}
	}

	return 1;
}
/* unlink [/caminho/arquivo] - excluir arquivo 
ou diretorio (o diretorio precisa estar vazio) */
int unlink(const int arg_count,char** arg_value){
	uint16_t fat_entry = 0x0000;
	uint16_t fat_parent_entry = 0x0000;
	uint8_t dir_index = 0x00;
	uint8_t dir_name[18];
	memset(dir_name,0x0000,18*sizeof(uint8_t)); //Write 0s
	if(arg_count > 1){
		/* rc = result code */
		uint8_t rc = walk_path(arg_value[1],&fat_entry,&fat_parent_entry,&dir_index,dir_name);
		DEBUG("unlink: rc = %02x\n",rc);
		switch(rc){
			case 0x00: exception(invalid_directory); return 0;
			case 0x1: case 0x02: break;
			case 0x03: exception(filesystem_not_loaded);return 0;
			case 0x04: exception(filesystem_not_found);return 0;
			case 0x05: exception(memory_corrupted);return 0;
			default: exception(fatal_error);return 0;
		}
		/*
			This a special rule
		
			The way walk_path was implemented, it returns 0x02
			even if the dir_entry does not exists, so we must make sure
			that this dir_entry exists.
			If it's first_block is 0x0000 (boot_block)
			we consider that it doesn't exist
		
		 */
		DEBUG("unlink: after SWITCH dir_index is 0x%02x\n",dir_index);
		DEBUG("unlink: after SWITCH fat_entry is 0x%04x\n",fat_entry);

		// if(data_cluster.dir[dir_index].first_block == 0x0000){
		// 	DEBUG("unlink: First_block appears to be 0000\n");
		// 	exception(invalid_directory);
		// 	return 0;
		// }
		/* If it got here, means the path was found successfully */

		// DEBUG("unlink: trying to delete file #1 %s\n",(char*)dir_name);
		// DEBUG("unlink: trying to delete file #2 %s\n",data_cluster.dir[dir_index].filename);
		if(rc == 0x02 && data_cluster.dir[dir_index].attributes == 0x00){ /*It's a File */
			if(data_cluster.dir[dir_index].first_block == 0x0000){
				DEBUG("unlink: First_block appears to be 0000\n");
				exception(invalid_directory);
				return 0;
			}
			DEBUG("unlink: trying to delete file #1 %s\n",(char*)dir_name);
			DEBUG("unlink: trying to delete file #2 %s\n",data_cluster.dir[dir_index].filename);


			DEBUG("unlink: It's a File\n");
			DEBUG("unlink: dir_index is 0x%02x\n",dir_index);
			DEBUG("unlink: fat_entry is 0x%04x\n",fat_entry);
			/* Need to read cluster again */
			read_cluster(fat_entry);
			//
			// uint16_t curr_block = data_cluster.dir[dir_index].first_block;
			// uint16_t curr_block = fat_entry;
			DEBUG("unlink: first_block is 0x%04x\n",data_cluster.dir[dir_index].first_block);
			uint16_t curr_block = data_cluster.dir[dir_index].first_block;
			// memset(&data_cluster.dir[dir_index],0x0000,32*sizeof(uint8_t));
			/* Remove directory entry for this file */
			memset(&data_cluster.dir[dir_index],0x00,32);
			DEBUG("unlink: Before #while#\n");
			while(fat[curr_block] != 0xffff){
				DEBUG("unlink: #while# curr_block = 0x%04x\n",curr_block);
				uint16_t next_block = fat[curr_block];
				DEBUG("unlink: #while# next_block = 0x%04x\n",next_block);
				fat[curr_block] = 0x0000; //update fat in memory
				update_fat(curr_block); //update fat in disk
				curr_block = next_block;
			}
			if(curr_block >= 0x000a){
				DEBUG("unlink: curr_block >= 0x000a\n");
				fat[curr_block] = 0x0000;
				update_fat(curr_block);
			}
			//OBS!!!! SE DER ERRO TA AQUI!!!!!!!
			update_cluster(fat_entry);
			//ACIMA !!!!!
		}else if(rc == 0x01){
			DEBUG("unlink: It's a Directory\n");
			DEBUG("unlink: dir_index is 0x%02x\n",dir_index);
			DEBUG("unlink: fat_entry is 0x%04x\n",fat_entry);
			DEBUG("unlink: fat_parent_entry is 0x%04x\n",fat_parent_entry);
			/* Test if Directory is empty */
			uint32_t i;
			for(i = 0; i < 32; i++){ //For each possible dir_entry_t
				if(data_cluster.dir[i].first_block != 0x0000){
					/* There is a file in there !*/
					exception(directory_not_empty);
					return 0;
				}
			}
			DEBUG("unlink: Possible to delete\n");
			/* If it is here, the directory is empty and can be deleted 
			*/
			/* Go one directory_entry above */
			// read_cluster(fat_parent_entry);
			// //1) Clear FAT[index]
			// //2) Clear directory_entry_t
			// uint16_t curr_block = data_cluster.dir[dir_index].first_block;
			// /* That means someone tried to boot e_e */
			// if(curr_block == 0x0000){
			// 	DEBUG("unlink: Shouldn't do that\n");
			// 	return 0;
			// }
			// memset(&data_cluster.dir[dir_index],0x00,32);
			// fat[curr_block] = 0x0000;
			// update_fat(curr_block);
			// update_cluster(fat_parent_entry);

			/* Go one directory_entry above */
			read_cluster(fat_entry);
			//1) Clear FAT[index]
			//2) Clear directory_entry_t
			uint16_t curr_block = data_cluster.dir[dir_index].first_block;
			/* That means someone tried to boot e_e */
			if(curr_block == 0x0000){
				DEBUG("unlink: Shouldn't do that\n");
				return 0;
			}
			DEBUG("unlink: fat[0x%04x] = 0x%04x\n",curr_block,fat[curr_block]);
			memset(&data_cluster.dir[dir_index],0x00,32);
			fat[curr_block] = 0x0000;
			update_fat(curr_block);
			update_cluster(fat_entry);


		}
		// else if(data_cluster.dir[dir_index].attributes == 0x01){ /* It's a directory */
		// 	/* Check if it's empty */
		// }
	}
	return 1;
}
/* write ”string” [/caminho/arquivo] - escrever 
dados em um arquivo (sobrescrever dados) */
int write(const int arg_count,char** arg_value){

	/* Won't consider '\0' as part of the string ?*/
	if(arg_count > 2){
		uint32_t string_size = sizeof(char)*strlen(arg_value[1]);
		// string_size+=1; //It's the '\0'
		DEBUG("write: argv[0] = {string:%s,size:%d}\n",arg_value[1],string_size);

		uint16_t fat_entry = 0x0000;
		uint16_t fat_parent_entry = 0x0000;
		uint8_t dir_index = 0x00;
		uint8_t dir_name[18];
		memset(dir_name,0x0000,18*sizeof(uint8_t)); //Write 0s
		uint8_t rc = walk_path(arg_value[2],&fat_entry,&fat_parent_entry,&dir_index,dir_name);
		DEBUG("write: rc = %02x\n",rc);
		switch(rc){
			case 0x00: case 0x1: exception(invalid_directory); return 0;
			case 0x02: break;
			case 0x03: exception(filesystem_not_loaded);return 0;
			case 0x04: exception(filesystem_not_found);return 0;
			case 0x05: exception(memory_corrupted);return 0;
			default: exception(fatal_error);return 0;
		}
		if(data_cluster.dir[dir_index].first_block == 0x0000){
			DEBUG("write: First_block appears to be 0000\n");
			exception(invalid_directory);
			return 0;
		}

		if(data_cluster.dir[dir_index].attributes == 0x00){
			DEBUG("write: It's a File\n");
			DEBUG("write: trying to write to file #1 %s\n",(char*)dir_name);
			DEBUG("write: trying to write to file #2 %s\n",data_cluster.dir[dir_index].filename);
			DEBUG("write: dir_index is 0x%02x\n",dir_index);
			DEBUG("write: fat_entry is 0x%04x\n",fat_entry);

			uint32_t required_clusters = ceil(string_size/1024.0);
			uint32_t current_clusters = ceil(data_cluster.dir[dir_index].size/1024.0);
			//Just gonna change it's size and write to disk
			if(string_size > data_cluster.dir[dir_index].size){
				data_cluster.dir[dir_index].size = string_size;
				update_cluster(fat_entry);
			}
			if(current_clusters == 0) current_clusters = 1;
			//1) Check for size
			DEBUG("write: Current file size = %d\n",data_cluster.dir[dir_index].size);
			DEBUG("write: currently allocated clusters  = %d\n",current_clusters);
			DEBUG("write: required clusters = %d\n",required_clusters);

			uint32_t missing_clusters = required_clusters - current_clusters;
			if(missing_clusters > 0){ //Need to check for available space

				DEBUG("Write: Space not enought\n");
				
				uint16_t empty_indexes[missing_clusters];
				//memset(empty_indexes,0x0000,required_)
				rc = find_empty_clusters(missing_clusters,empty_indexes);
				if(rc == 0x00){
					exception(memory_full);
					return 0;
				}
				/* Adjust new resources */
				int i;
				uint16_t current_block = data_cluster.dir[dir_index].first_block;
				while(fat[current_block] != 0xffff){
					current_block = fat[current_block];
				}
				for(i = 0; i < missing_clusters; i++){
					fat[current_block] = empty_indexes[i];
					update_fat(current_block);
					current_block = empty_indexes[i];
				}
				fat[current_block] = 0xffff;
			}else{ 
				DEBUG("write: Has enought Space\n");
			}
			/* First use the existing resources */
			// uint32_t currentPos = 0;
			uint32_t i;
			uint16_t current_block = data_cluster.dir[dir_index].first_block;
			DEBUG("write: first_block = 0x%04x\n",current_block);
			uint32_t current_pos = 0;
			//writes first block
			read_cluster(current_block);
			if(string_size >= 1024){

				// memset(data_cluster.data,0x00,CLUSTER_SIZE); //Write 0s
				memcpy(data_cluster.data,(arg_value[1]+(current_pos*CLUSTER_SIZE)),CLUSTER_SIZE);
			}else{
				// memset(data_cluster.data,0x00,CLUSTER_SIZE); //Write 0s
				memcpy(data_cluster.data,(arg_value[1]+(current_pos*CLUSTER_SIZE)),string_size);
			}
			update_cluster(current_block);

		}

	}
	return 0;
}
/* append ”string” [/caminho/arquivo] - anexar
dados em um arquivo */
int append(const int arg_count,char** arg_value){

	/* Won't consider '\0' as part of the string ?*/
	if(arg_count > 2){
		uint32_t string_size = sizeof(char)*strlen(arg_value[1]);
		// string_size+=1; //It's the '\0'
		DEBUG("write: argv[0] = {string:%s,size:%d}\n",arg_value[1],string_size);

		uint16_t fat_entry = 0x0000;
		uint16_t fat_parent_entry = 0x0000;
		uint8_t dir_index = 0x00;
		uint8_t dir_name[18];
		memset(dir_name,0x0000,18*sizeof(uint8_t)); //Write 0s
		uint8_t rc = walk_path(arg_value[2],&fat_entry,&fat_parent_entry,&dir_index,dir_name);
		DEBUG("write: rc = %02x\n",rc);
		switch(rc){
			case 0x00: case 0x1: exception(invalid_directory); return 0;
			case 0x02: break;
			case 0x03: exception(filesystem_not_loaded);return 0;
			case 0x04: exception(filesystem_not_found);return 0;
			case 0x05: exception(memory_corrupted);return 0;
			default: exception(fatal_error);return 0;
		}
		if(data_cluster.dir[dir_index].first_block == 0x0000){
			DEBUG("write: First_block appears to be 0000\n");
			exception(invalid_directory);
			return 0;
		}

		if(data_cluster.dir[dir_index].attributes == 0x00){
			DEBUG("write: It's a File\n");
			DEBUG("write: trying to write to file #1 %s\n",(char*)dir_name);
			DEBUG("write: trying to write to file #2 %s\n",data_cluster.dir[dir_index].filename);
			DEBUG("write: dir_index is 0x%02x\n",dir_index);
			DEBUG("write: fat_entry is 0x%04x\n",fat_entry);

			uint32_t required_clusters = ceil(string_size/1024.0);
			uint32_t current_clusters = ceil(data_cluster.dir[dir_index].size/1024.0);
			uint32_t original_size = data_cluster.dir[dir_index].size;
			//Just gonna change it's size and write to disk
			data_cluster.dir[dir_index].size += string_size;
			update_cluster(fat_entry);
			

			if(current_clusters == 0) current_clusters = 1;
			//1) Check for size
			DEBUG("write: Current file size = %d\n",data_cluster.dir[dir_index].size);

			DEBUG("write: currently allocated clusters  = %d\n",current_clusters);
			DEBUG("write: required clusters = %d\n",required_clusters);
			//RECALCULAR ISSO
			uint32_t missing_clusters = required_clusters - current_clusters;
			if(missing_clusters > 0){ //Need to check for available space
				DEBUG("Write: Space not enought\n");
				
				uint16_t empty_indexes[missing_clusters];
				//memset(empty_indexes,0x0000,required_)
				rc = find_empty_clusters(missing_clusters,empty_indexes);
				if(rc == 0x00){
					exception(memory_full);
					return 0;
				}
				/* Adjust new resources */
				int i;
				uint16_t current_block = data_cluster.dir[dir_index].first_block;
				while(fat[current_block] != 0xffff){
					current_block = fat[current_block];
				}
				for(i = 0; i < missing_clusters; i++){
					fat[current_block] = empty_indexes[i];
					update_fat(current_block);
					current_block = empty_indexes[i];
				}
				fat[current_block] = 0xffff;
			}else{ 
				DEBUG("write: Has enought Space\n");
			}
			/* First use the existing resources */
			// uint32_t currentPos = 0;
			uint32_t i;
			uint16_t current_block = data_cluster.dir[dir_index].first_block;
			DEBUG("write: first_block = 0x%04x\n",current_block);
			uint32_t current_pos = 0;

			//writes first block
			// original_size
			read_cluster(current_block);
			if(string_size >= 1024){

				// memset(data_cluster.data,0x00,CLUSTER_SIZE); //Write 0s
				memcpy((data_cluster.data + original_size),(arg_value[1]+(current_pos*CLUSTER_SIZE)),CLUSTER_SIZE);
			}else{
				// memset(data_cluster.data,0x00,CLUSTER_SIZE); //Write 0s
				memcpy((data_cluster.data + original_size),(arg_value[1]+(current_pos*CLUSTER_SIZE)),string_size);
			}
			update_cluster(current_block);

		}

	}
	return 0;

	return 0;
}
/* read [/caminho/arquivo] - ler o conteudo de um arquivo */
int read(const int arg_count,char** arg_value){
	/* Won't consider '\0' as part of the string ?*/
	if(arg_count > 1){
		// string_size+=1; //It's the '\0'
		DEBUG("read: argv[1] = {string:%s}\n",arg_value[1]);

		uint16_t fat_entry = 0x0000;
		uint16_t fat_parent_entry = 0x0000;
		uint8_t dir_index = 0x00;
		uint8_t dir_name[18];
		memset(dir_name,0x0000,18*sizeof(uint8_t)); //Write 0s
		uint8_t rc = walk_path(arg_value[1],&fat_entry,&fat_parent_entry,&dir_index,dir_name);
		DEBUG("read: rc = %02x\n",rc);
		switch(rc){
			case 0x00: case 0x1: exception(invalid_directory); return 0;
			case 0x02: break;
			case 0x03: exception(filesystem_not_loaded);return 0;
			case 0x04: exception(filesystem_not_found);return 0;
			case 0x05: exception(memory_corrupted);return 0;
			default: exception(fatal_error);return 0;
		}
		if(data_cluster.dir[dir_index].first_block == 0x0000){
			DEBUG("read: First_block appears to be 0000\n");
			exception(invalid_directory);
			return 0;
		}

		if(data_cluster.dir[dir_index].attributes == 0x00){
			DEBUG("read: It's a File\n");
			DEBUG("read: trying to write to file #1 %s\n",(char*)dir_name);
			DEBUG("read: trying to write to file #2 %s\n",data_cluster.dir[dir_index].filename);
			DEBUG("read: dir_index is 0x%02x\n",dir_index);
			DEBUG("read: fat_entry is 0x%04x\n",fat_entry);

			uint32_t original_size = data_cluster.dir[dir_index].size;
			uint16_t current_block = data_cluster.dir[dir_index].first_block;
			read_cluster(current_block);
			uint32_t cont = 0;
			while(cont < original_size){
				CUSTOM_PRINTF(ACBLACK,"%c",data_cluster.data[cont]);
				cont++;
				if(cont != 0 && cont%1024 == 0){
					current_block = fat[current_block];
					read_cluster(current_block);
				}
			}
			CUSTOM_PRINTF(ACBLACK,"\n");
		}

	}
	return 0;
}
void help()
{
 printf("\
 init - initialize file system(overwrite existing one)\n\
 load - loads file system from disk\n\
 ls [/path/directory] - list directory\n\
 mkdir [/path/directory] - create directory\n\
 create [/path/file] - create file\n\
 unlink [/path/file] - delete file or directory (directory must be empty)\n\
 write \"string\" [/path/file] - write data to a file(overwrite existing data)\n\
 append \"string\" [/path/file] - append data to a file\n\
 read [/path/file] - read file contents\n\
 help - print help information\n\
 exit - exit from shell\n\
 \n");
}

static const char* str_init = "init";
static const char* str_load = "load";
static const char* str_ls = "ls";
static const char* str_mkdir = "mkdir";
static const char* str_create = "create";
static const char* str_unlink = "unlink";
static const char* str_write = "write";
static const char* str_append = "append";
static const char* str_read = "read";
static const char* str_help = "help";
static const char* str_exit = "exit";
/* Execute Commands */
void run(int arg_count,char** arg_value)
{
	char* command = arg_value[0];
	if(strcmp(command,str_exit) == 0) exit(0);

	if(strcmp(command,str_init) == 0){
		init();
	}else if(strcmp(command,str_load) == 0){
		load();
	}else if(strcmp(command,str_ls) == 0){
		ls(arg_count,arg_value);
	}else if(strcmp(command,str_mkdir) == 0){
		mkdir(arg_count,arg_value);
	}else if(strcmp(command,str_create) == 0){
		create(arg_count,arg_value);
	}else if(strcmp(command,str_unlink) == 0){
		unlink(arg_count,arg_value);
	}else if(strcmp(command,str_write) == 0){
		write(arg_count,arg_value);
	}else if(strcmp(command,str_append) == 0){
		append(arg_count,arg_value);
	}else if(strcmp(command,str_read) == 0){
		read(arg_count,arg_value);
	}else if(strcmp(command,str_help) == 0){
		help();
	}else{
		SHELL_PRINTF("%s: command not found\n",command);
	}
}
/* Shell Environment Commands */
void shell_env()
{
	// char *input = malloc(sizeof(char) * 128);
	
	int arg_count;
	uint32_t i = 0;
	while(1){
		// printf(NORMAL_USER);
		USER_LINE();
		// scanf("%s",input);
		char** arg_value = inputscanf(&arg_count,stdin);
		if(!(arg_count == 1 && (strcmp(arg_value[0],"\0") == 0))){
			run(arg_count,arg_value);
		}
		for(i = 0; i < arg_count; i++) free(arg_value[i]);
		free(arg_value);
		// run(input);
	}
}




int main(int argc, const char** argv)
{
	memset(fat,0x0000,4096); //Write 0s
	shell_env();
	return 0;
}













