#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define ONE_LINE 80
#define PAST 0
#define PRESENT 1
#define DATA_NUM 200
#define PORT 9001

enum page_faults{TOTAL, MAJOR, MINOR} page_enum;
enum datas{PID, MIN_FLT, MAJ_FLT, RSS} data_enum;

typedef struct{
	int pid;
	int min_flt;
	int maj_flt;
	int rss;
} listData; 

listData make_cur_data(listData present, listData past){

	listData result;

	result.pid = present.pid;
	result.min_flt = present.min_flt - past.min_flt;
	result.maj_flt = present.maj_flt - past.maj_flt;
	result.rss = present.rss;

	return result;
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

	listData arrayData[2][DATA_NUM] = {0}; //data array (PAST,PRESENT)
	listData cur_Data[DATA_NUM] = {0};
	char broker_address[30] = {0};

	FILE* addrFile;
	FILE* listFile;

	//get broker IP address
	addrFile = fopen("./broker_ip.txt", "r"); // open broker_ip
	fscanf(addrFile, "%s", &broker_address[0]);


	while(1){

		system("ps -eo pid,min_flt,maj_flt,rss | cat > list_present.txt");
		listFile = fopen("./list_present.txt", "r");


		//scan all items

			//delete first line
		fscanf(listFile, "%*s %*s %*s %*s");
		fflush(stdin);
		for(int i=0; !feof(listFile);i++){
			fscanf(listFile, "%d %d %d %d",
				&arrayData[PRESENT][i].pid,
				&arrayData[PRESENT][i].min_flt,
				&arrayData[PRESENT][i].maj_flt,
				&arrayData[PRESENT][i].rss);
			fflush(stdin);
		}	//end while

		//each for counting => aliased
		int cnt_past, cnt_pres, cnt_cur = 0;
		int *a=&cnt_past, *b=&cnt_pres, *c=&cnt_cur;
		int flag = 1;
		//flag check using //int type default = 0
		while(flag){
			if(arrayData[PAST][*a].pid == 
				arrayData[PRESENT][*b].pid){
				if(arrayData[PAST][*a].maj_flt<arrayData[PRESENT][*b].maj_flt){
					cur_Data[*c] = make_cur_data(
					arrayData[PRESENT][*b], arrayData[PAST][*a]);
					*c=*c+1;
				} //end inner if
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
//printf("^^%d %d %d^^\n", *a, *b, *c);
//printf("== %d %d==\n", arrayData[PAST][*a].pid, arrayData[PRESENT][*b].pid);
		} //end while
//printf("out\n");
		char topic[4][100] = {"mon/list/pid",
				"mon/list/min_flt", "mon/list/maj_flt",
				"mon/list/rss"};

	printf("gonna in!!!\n");
		//print value and send MQTT
		for(int i=0; cur_Data[i].pid != 0;i++){
	printf("%d", cur_Data[i].pid);	
                	char instruct[4][200];
			sprintf( instruct[PID], "sudo mosquitto_pub -t '%s' -h %s -m '%d'", topic[PID], broker_address, cur_Data[i].pid);
			sprintf( instruct[MIN_FLT], "sudo mosquitto_pub -t '%s' -h %s -m '%d'", topic[MIN_FLT], broker_address, cur_Data[i].min_flt);
			sprintf( instruct[MAJ_FLT], "sudo mosquitto_pub -t '%s' -h %s -m '%d'", topic[MAJ_FLT], broker_address, cur_Data[i].maj_flt);
			sprintf( instruct[RSS], "sudo mosquitto_pub -t '%s' -h %s -m '%d'", topic[RSS], broker_address, cur_Data[i].rss);

printf("%s", instruct[0]); //for test

			//shell instruction : mqtt publish
			for(int j=0; j<4; j++)
				system(instruct[j]);
		} //end for

		memcpy(arrayData[PAST], arrayData[PRESENT], 
			sizeof(listData)*DATA_NUM);

		sleep(1);
	}
	fclose(addrFile);
	//close addrFile

	fclose(listFile);
	//close listFile	

	return 0;
}
