#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <stdbool.h>

#define PARTITION_CNT_MAX		50
#define PARTITION_LEN_SIZE		5

#define SLEEP_TIME_US			1000*500

#define BUFF_SIZE				1024*4
#define CMD_SIZE				100

#define BUFF_SIZE_MINOR			10

#define LAST_DISK_POINT  		"sda1"
#define NORMAL_DISK_NUM			4
#define DEBUG

char g_disk_mount[PARTITION_CNT_MAX][PARTITION_LEN_SIZE] = {0};  	//	record the mounted disk
int g_disk_mount_cnt = 0;											//	record the mounted disk count

bool run_System_Cmd(char *cmd)
{
	if (cmd == NULL)
		return false;
		
	#ifdef DEBUG
	printf("run shell: %s\n", cmd);
	#endif
	bool ret = system(cmd);
	//printf("ret = %d\n", ret);
    return ret;
}

int check_Disk_Num()
{
	int disk_num;
	char buff[BUFF_SIZE_MINOR] = {0};
	char cmd[CMD_SIZE] = {0};
	sprintf(cmd, "cat /proc/partitions | wc -l");
	FILE *fp = popen(cmd, "r");
	while( fgets(buff, BUFF_SIZE_MINOR-1, fp) != NULL)
    {
		continue;
    }

	disk_num = atoi(buff);
	#ifdef DEBUG
	printf("check disk num:  %d\n", disk_num);
	#endif
	pclose(fp);
	return disk_num;	
}

bool disk_Is_Single_Partition(char *disk)
{
	if (disk == NULL)
		return false;

	unsigned char buff[BUFF_SIZE_MINOR] = {0};
	unsigned char cmd[CMD_SIZE] = {0};
	sprintf(cmd, "cat /proc/partitions | grep %s | wc -l", disk);
	FILE *fp = popen(cmd, "r");
	while( fgets(buff, BUFF_SIZE_MINOR-1, fp) != NULL)
    {
		continue;
    }
	
	pclose(fp);

	if ( 1 == atoi(buff))
	{
		#ifdef DEBUG
		printf("disk: %s is single partition\n",disk);
		#endif
		return true;
	}
	
	return false;
}


int get_Partition_Info(char *info)
{
	char buff[BUFF_SIZE] = {0};
	char cmd[CMD_SIZE] = {0};
	sprintf(cmd, "cat /proc/partitions");
	FILE *fp = popen(cmd, "r");

	if (fp == NULL)
		return -1;

	while( fgets(buff, BUFF_SIZE-1, fp) != NULL)
    {
    	strncat(info, buff, strlen(buff));
		continue;
    }
    
    #ifdef DEBUG
	printf("\n----partitions info----:\n %s\n", info);
	#endif
	
	pclose(fp);
	return 0;
}

void run_Mount(int add)
{
	unsigned char cmd[CMD_SIZE] = {0};
	for (int i = g_disk_mount_cnt - add; i < g_disk_mount_cnt; i++)
	{
		memset(cmd, 0, CMD_SIZE);
		sprintf(cmd, "mkdir /media/%s", g_disk_mount[i]);
		run_System_Cmd(cmd);

		memset(cmd, 0, CMD_SIZE);
		sprintf(cmd,"mount -t auto /dev/%s /media/%s",g_disk_mount[i],g_disk_mount[i]);  
		run_System_Cmd(cmd);
	}
}

bool disk_Is_Exit(char *disk)
{
	for (int i = 0; i < g_disk_mount_cnt; i++)
	{
		if (strcmp(disk , g_disk_mount[i]) == 0)
		{
			return true;
		}
	}
	return false;
}

void change_Enter(char *temp)
{
	if (temp == NULL)
		return ;	
	
	for (int i = 0; i < strlen(temp); i++)
	{
		if (temp[i] == '\n')
		{
			temp[i] = '\0';
		}
	}
}

bool check_Disk_Mount(char * buff)
{
	if (buff == NULL)
		return false;
		
	for (int i = 0; i < g_disk_mount_cnt; i++)
	{
		//not in file buff
		if (strstr(buff,g_disk_mount[i]) == NULL)
		{
			return false;
		}
	}
	
	return true;
}

void main()
{
	int disk_partition_cnt = 0;
	int disk_num_temp;
	int disk_num = NORMAL_DISK_NUM;
	int disk_mount_cnt_temp;
	
	char buff[BUFF_SIZE] = {0};
	char disk[PARTITION_CNT_MAX][PARTITION_LEN_SIZE] = {0};
	char cmd[CMD_SIZE] = {0};
	
	int disk_umount_cnt = 0;
	int disk_umount_pos[PARTITION_CNT_MAX];
	for (int i = 0; i < PARTITION_CNT_MAX; i++)
	{
		disk_umount_pos[i] = -1;
	}

	while(1)
	{
		//check the /proc/partition
		disk_num_temp = check_Disk_Num();
		
		if (g_disk_mount_cnt != 0)
			run_System_Cmd("sync");
		
		if (disk_num != disk_num_temp)		
		{
			#ifdef DEBUG
			printf("different disk info\n");
			#endif

			//get partition buff
			memset(buff,0,BUFF_SIZE);			
			if (0 != get_Partition_Info(buff))
			{
				usleep(SLEEP_TIME_US);
				continue;
			}

			//disk add mount
			if (disk_num < disk_num_temp)
			{
				#ifdef DEBUG
				printf("disk add action\n");
				#endif	
				
				char *p = strstr(buff,LAST_DISK_POINT);
				if(p != NULL)
				{
					disk_partition_cnt = 0;
					while(1)
					{
						if (disk_partition_cnt >= PARTITION_CNT_MAX)
							break;
							
						p = strstr(p + 4,"sd");
						if(p != NULL)
						{
							//get mount disk list
							memset(disk[disk_partition_cnt], 0, PARTITION_LEN_SIZE);
							memcpy(disk[disk_partition_cnt], p, 4); 
							change_Enter(disk[disk_partition_cnt]);
							disk_partition_cnt++;
						}
						else
						{
							break;
						}
					}
				}
				
				//judge and add to global
				disk_mount_cnt_temp = g_disk_mount_cnt;
				for (int i = 0; i < disk_partition_cnt; i++)
				{
					if (disk_Is_Exit(disk[i]))
						continue;

					//if len=3 disk have no partition
					if (strlen(disk[i]) == 3)
					{
						if ( !disk_Is_Single_Partition(disk[i]))
							continue;
					}
						
					memcpy(g_disk_mount[g_disk_mount_cnt], disk[i], strlen(disk[i]));
					g_disk_mount_cnt++;
				}
				run_Mount(g_disk_mount_cnt - disk_mount_cnt_temp);
			}
			else
			{
				//find the umount disk list and umount
				for (int i = 0; i < PARTITION_CNT_MAX; i++)
				{
					disk_umount_pos[i] = -1;
				}
				disk_umount_cnt = 0;
				for (int i = 0; i < g_disk_mount_cnt; i++)
				{
					if (strstr(buff,g_disk_mount[i]) == NULL)
					{	
						memset(cmd, 0, CMD_SIZE);
						sprintf(cmd,"umount /media/%s",g_disk_mount[i]);
						if ( 0 != run_System_Cmd(cmd))
						{
							memset(cmd, 0, CMD_SIZE);
							sprintf(cmd,"umount /media/%s -l",g_disk_mount[i]);
							run_System_Cmd(cmd);
						}
						
						memset(cmd, 0, CMD_SIZE);
						sprintf(cmd, "rm /media/%s -rf", g_disk_mount[i]);
						run_System_Cmd(cmd);
	
						disk_umount_pos[disk_umount_cnt] = i;
						disk_umount_cnt++;
					}
				}
				
				#ifdef DEBUG
				printf("get g_disk_umount_cnt: %d\n", disk_umount_cnt);
				#endif
				
				//clear the umount disk from global
				for (int i = 0; i < disk_umount_cnt; i++)
				{
					if (disk_umount_pos[i] == -1)
						continue;
						
					memset(g_disk_mount[disk_umount_pos[i]], 0, PARTITION_LEN_SIZE);
				}
				
				
				//reset the global
				g_disk_mount_cnt = g_disk_mount_cnt - disk_umount_cnt;
				int temp_cnt = 0;
				for (int i = 0; i < PARTITION_CNT_MAX; i++)
				{
					if (temp_cnt == g_disk_mount_cnt)
						break;
						
					if (strlen(g_disk_mount[i]) != 0)
					{
						memcpy(g_disk_mount[temp_cnt], g_disk_mount[i], PARTITION_LEN_SIZE);
						temp_cnt++;
					}
				}
				for (int i = temp_cnt; i < PARTITION_CNT_MAX; i++)
				{
					memset(g_disk_mount[i], 0, PARTITION_LEN_SIZE);
				}	
			}
			disk_num = disk_num_temp;
		}
		else if (disk_num_temp != NORMAL_DISK_NUM)
		{
			if (true != check_Disk_Mount(buff))
			{
				//refresh
				disk_num = 0;
			}
		}
		
		#ifdef DEBUG
		printf("get g_disk_mount_cnt: %d\n", g_disk_mount_cnt);
		for (int i = 0; i < g_disk_mount_cnt; i++)
		{
			printf("mount disk[%d]: %s\n", i, g_disk_mount[i]);
		}
		#endif	
		
		usleep(SLEEP_TIME_US);
	}
}
