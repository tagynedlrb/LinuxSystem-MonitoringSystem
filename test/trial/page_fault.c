#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define ONE_LINE 80
#define PAST 0
#define PRESENT 1
#define FAULT_NUM 3
#define PORT 9001

enum page_faults{TOTAL, MAJOR, MINOR} page_enum;

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

	int page_faults[2][FAULT_NUM] = {'\0'};
	int cur_faults[FAULT_NUM] = {0};
	char broker_address[100];

	FILE* addrFile;
	FILE* vmstatFile;

	//get broker IP address
	addrFile = fopen("./broker_ip.txt", "r"); // open broker_ip
	fscanf(addrFile, "%s", &broker_address[0]);


	while(1){
		vmstatFile = fopen("/proc/vmstat", "r"); //open meminfo

		page_faults[PRESENT][TOTAL] = 
			file_scan_certain(vmstatFile, "pgfault");
		page_faults[PRESENT][MAJOR] = 
			file_scan_certain(vmstatFile, "pgmajfault");

		page_faults[PRESENT][MINOR] =
			page_faults[PRESENT][TOTAL]
				-page_faults[PRESENT][MAJOR];

//	for(int i=0;i<3;i++)	//for test
//	printf("%d \n", page_faults[PAST][i]);

		for(int i=0; i<FAULT_NUM;i++){
			if(page_faults[PAST][i]){	// null check
				cur_faults[i] = page_faults[PRESENT][i] 
					- page_faults[PAST][i];
			}
		}

//	for(int i=0;i<3;i++)	//for test
//	printf("%d \n", cur_faults[i]);

		char topic[3][100] = {"mon/page/total",
				"mon/page/major", "mon/page/minor"};
		char pageFault_value[3][100];

		//print value  and send MQTT
		for(int i=0; i<FAULT_NUM;i++){
			sprintf(pageFault_value[i], "%d", cur_faults[i]);
			printf("Page Fault%d : %s\n", i, pageFault_value[i]);
			sprintf(pageFault_value[i], "%d", cur_faults[i]);
			instruct_mqtt(topic[i], broker_address,
				 pageFault_value[i]);
		}

		memcpy(page_faults[PAST], page_faults[PRESENT], 
			sizeof(int)*FAULT_NUM);

		fclose(vmstatFile);
		//close vmstatFile	

		sleep(1);
	}
	fclose(addrFile);
	//close addrFile

	return 0;
}
