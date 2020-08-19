#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#define ONE_LINE 80
#define PAST 0
#define PRESENT 1
#define DATA_NUM 5
#define MAX_ROW 200
#define PORT 9001

enum datas{PID, PATH, CPU, MAJ_FLT, RSS} data_enum;

typedef struct{
	int pid;
	char path[6 + 256 + 5]; // /proc/ + d_name + /stat;
	float cpu;
	unsigned long maj_flt;
	long rss;
} listData; 

listData make_cur_data(listData present, listData past){

	listData result;

	result.pid = present.pid;
	strcpy(result.path, present.path);
	result.cpu = present.cpu;
	result.maj_flt = present.maj_flt - past.maj_flt;
	result.rss = present.rss;

	return result;
}

//function to check if a struct dirent from /proc is a PID folder
int is_pid_folder(const struct dirent *entry){
	const char *p;

	for (p = entry->d_name; *p; p++){
		if(!isdigit(*p))
			return 0;
	}
	
	return 1;
}

int file_scan_certain(FILE* targetFile, char target[]){
	
	int fscanf_flag = 0;	// flag for file scan
	int result = 9999999;
	char temp[100];

	while(!fscanf_flag){

		fscanf(targetFile, "%s", temp);

		if(!strcmp(temp, target)){
			fscanf(targetFile, "%d", &result);
			fflush(stdin);	fscanf_flag = 1;
		}
		else{
			fscanf(targetFile, "%*d");
			fflush(stdin);
		}
	}	//end while
	
	return result;
}


void instruct_mqtt(char topic[], char broker_address[], char value[]){
      //start shell instruction
                char instruct[100] = "sudo mosquitto_pub -t '";

                strcat(instruct, topic);
                strcat(instruct, "' -h ");
                strcat(instruct, broker_address);
                strcat(instruct, " -m '");
                strcat(instruct, value);
                strcat(instruct, "'");

                system(instruct);
        //end shell instruction
}

int main (void){
	char loadDataBuf[ONE_LINE] = {0};

	listData arrayData[2][MAX_ROW] = {0}; //data array (PAST,PRESENT)
	listData cur_Data[MAX_ROW] = {0};
	char broker_address[30] = {0};

	FILE* addrFile;
	FILE* pidFile;
	FILE* uptimeFile;
	DIR *procDir;
	struct dirent *entry;
	char path[6 + 256 + 5]; // /proc/ + d_name + /stat
	unsigned long utime = 0, stime = 0;
	unsigned long long starttime = 0;
	float uptime = 0;
	

	//get broker IP address
	addrFile = fopen("./broker_ip.txt", "r"); // open broker_ip
	fscanf(addrFile, "%s", &broker_address[0]);


	while(1){
		
		//Open /proc directory.
		procDir = opendir("/proc");
		if(!procDir){
			perror("opendir failed");
			return 1;
		}

		int cnt = 0;
		//Iterate through all files and folders of /proc.
		while((entry = readdir(procDir))){
			//Skip anything that is not a PID folder
			if(!is_pid_folder(entry))
				continue;

			//Try to open /proc/[PID]/stat.	
			snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
			pidFile = fopen(path, "r");

			if(!pidFile){
				perror(path);
				continue;
			}

			//Get PID, process name and number of faults.
/*By gnu_scanf format, if using wild card(*), skip long || %*lu => %*u */
			fscanf(pidFile, "%d %s %*c %*d %*d %*d %*d %*d %*u %*u %*u %lu %*u %lu %lu %*d %*d %*d %*d %*d %*d %llu %*u %ld", &arrayData[PRESENT][cnt].pid, &arrayData[PRESENT][cnt].path[0], &arrayData[PRESENT][cnt].maj_flt, &utime, &stime, &starttime, &arrayData[PRESENT][cnt].rss);

			//uptime from /proc/uptime, first element
			uptimeFile = fopen("/proc/uptime", "r");
			fscanf(uptimeFile, "%f", &uptime);

			//Hertz => sysconf(_SC_CLK_TCK) <sys.param.h>
			//make Cpu Util
			arrayData[PRESENT][cnt].cpu =
		 (utime + stime)/(uptime-(starttime/sysconf(_SC_CLK_TCK)));	

			//Pretty print.
			printf("%5d %-20s: %.2f | %lu | %ld\n", arrayData[PRESENT][cnt].pid, arrayData[PRESENT][cnt].path, arrayData[PRESENT][cnt].cpu, arrayData[PRESENT][cnt].maj_flt, arrayData[PRESENT][cnt].rss);
			fclose(pidFile);
			cnt++;
		} //end while 

		closedir(procDir);
		//close procDir

/* This lines are not used in this version
*		system("ps -eo pid,min_flt,maj_flt,rss | cat > list_present.txt");
*		listFile = fopen("./list_present.txt", "r");
*
*
*		//scan all items
*
*			//delete first line
*		fscanf(listFile, "%*s %*s %*s %*s");
*		fflush(stdin);
*		for(int i=0; !feof(listFile);i++){
*			fscanf(listFile, "%d %d %d %d",
*				&arrayData[PRESENT][i].pid,
*				&arrayData[PRESENT][i].min_flt,
*				&arrayData[PRESENT][i].maj_flt,
*				&arrayData[PRESENT][i].rss);
*			fflush(stdin);
*		}	//end for
*/
		//each for counting => aliased
		int cnt_past, cnt_pres, cnt_cur = 0;
		int *a=&cnt_past, *b=&cnt_pres, *c=&cnt_cur;
		int flag = 1;
		//flag check using //int type default = 0
		while(flag){
			if(arrayData[PAST][*a].pid == 
				arrayData[PRESENT][*b].pid){
	//			if(arrayData[PAST][*a].maj_flt<arrayData[PRESENT][*b].maj_flt){
					cur_Data[*c] = make_cur_data(
					arrayData[PRESENT][*b], arrayData[PAST][*a]);
					*c=*c+1;
	//			} //end inner if
				*a=*a+1; *b=*b+1;
			} else if(arrayData[PAST][*a].pid > 
					arrayData[PRESENT][*b].pid){
				*b=*b+1;
			} else if(arrayData[PAST][*a].pid <
					arrayData[PRESENT][*b].pid){
				*a=*a+1;
			}

			//check end of arrayData	
			if((arrayData[PAST][*a].pid == 0)
				|| (arrayData[PRESENT][*b].pid == 0)){
				flag = 0; *a=0; *b=0; *c=0;
			}
		} //end while

		char topic[DATA_NUM][100] = {"mon/list/pid",
				"mon/list/path", "mon/list/maj_flt",
				"mon/list/cpu", "mon/list/rss"};

		//print value and send MQTT
		for(int i=0; cur_Data[i].pid != 0;i++){

                	char instruct[DATA_NUM][200];
			sprintf( instruct[PID], "sudo mosquitto_pub -t '%s' -h %s -m '%d'", topic[PID], broker_address, cur_Data[i].pid);
			sprintf( instruct[PATH], "sudo mosquitto_pub -t '%s' -h %s -m '%s'", topic[PATH], broker_address, cur_Data[i].path);
			sprintf( instruct[MAJ_FLT], "sudo mosquitto_pub -t '%s' -h %s -m '%lu'", topic[MAJ_FLT], broker_address, cur_Data[i].maj_flt);
			sprintf( instruct[CPU], "sudo mosquitto_pub -t '%s' -h %s -m '%f'", topic[CPU], broker_address, cur_Data[i].cpu);
			sprintf( instruct[RSS], "sudo mosquitto_pub -t '%s' -h %s -m '%ld'", topic[RSS], broker_address, cur_Data[i].rss);


			//shell instruction : mqtt publish
//			for(int j=0; j<DATA_NUM; j++)
//				system(instruct[j]);
	/*for test*/			system(instruct[0]);
		} //end for

		memcpy(arrayData[PAST], arrayData[PRESENT], 
			sizeof(listData)*MAX_ROW);

		sleep(1);
	}
	fclose(addrFile);
	//close addrFile

	return 0;
}
