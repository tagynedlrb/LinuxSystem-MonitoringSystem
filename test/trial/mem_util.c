#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define ONE_LINE 80
#define PAST 0
#define PRESENT 1
#define MEMORIES_NUM 4
#define PORT 9001

enum memories{TOTAL, FREE, AVAILABLE} mem_enum;

int main (void){
	char loadDataBuf[ONE_LINE] = {0};

	int memories[2][MEMORIES_NUM] = {0};
	char* broker_address="223.194.128.191";


	FILE* memFile;

	while(1){
		memFile = fopen("/proc/meminfo", "r"); //open meminfo

		fscanf(memFile, "%*s %d %*s", &memories[PRESENT][TOTAL]); fflush(stdin);
		fscanf(memFile, "%*s %d %*s", &memories[PRESENT][FREE]); fflush(stdin);
		fscanf(memFile, "%*s %d %*s", &memories[PRESENT][AVAILABLE]); fflush(stdin);
//	for(int i=0;i<3;i++)	//for test
//		printf("%d \n", memories[1][i]);

		float mem_util_float = 100.0*(
		(memories[PRESENT][TOTAL]-memories[PRESENT][FREE])*1.0
			/memories[PRESENT][TOTAL]);
//		printf("%f\n", mem_util_float);

		char mem_util[100];
		sprintf(mem_util, "%f%%", mem_util_float);

		printf("Memory usage : %f%%\n", mem_util_float);

		char temp[100] = "sudo mosquitto_pub -t 'mon/mem' -h ";

		strcat(temp, broker_address);
		strcat(temp, " -m '");
		strcat(temp, (char*)mem_util);
//		strcat(temp, "%'");
strcat(temp, "'");
		char* instruction = temp;

		system(instruction);	
		memcpy(memories[PAST], memories[PRESENT], 
			sizeof(int)*MEMORIES_NUM);

		fclose(memFile);

		sleep(1);
	}
	return 0;
}
