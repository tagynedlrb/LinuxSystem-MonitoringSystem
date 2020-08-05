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
	char broker_address[100];

	FILE* addrFile;
	FILE* memFile;

	//get broker IP address
	addrFile = fopen("./broker_ip.txt", "r"); // open broker_ip
	fscanf(addrFile, "%s", &broker_address[0]);


	while(1){
		memFile = fopen("/proc/meminfo", "r"); //open meminfo

		fscanf(memFile, "%*s %d %*s", &memories[PRESENT][TOTAL]); fflush(stdin);
		fscanf(memFile, "%*s %d %*s", &memories[PRESENT][FREE]); fflush(stdin);
		fscanf(memFile, "%*s %d %*s", &memories[PRESENT][AVAILABLE]); fflush(stdin);
//	for(int i=0;i<3;i++)	//for test
//		printf("%d \n", memories[1][i]);

		float mem_util_nom_float = 100.0*(
		(memories[PRESENT][TOTAL]-memories[PRESENT][FREE])*1.0
			/memories[PRESENT][TOTAL]);


		float mem_util_act_float = 100.0*(
		(memories[PRESENT][TOTAL]-memories[PRESENT][AVAILABLE])*1.0
			/memories[PRESENT][TOTAL]);

		char mem_util_nom[100];	//nominal memory use
		char mem_util_act[100];	//actual memory use

		//float to string
		sprintf(mem_util_nom, "%f%%", mem_util_nom_float);
		sprintf(mem_util_act, "%f%%", mem_util_act_float);

		printf("Nominal Memory usage : %f%%\n", mem_util_nom_float);
		printf("Actual Memory usage : %f%%\n", mem_util_act_float);

	//instruct_nom shell instruction
		char instruct_nom[100] = "sudo mosquitto_pub -t 'MEM_NOM_UTIL' -h ";

		strcat(instruct_nom, broker_address);
		strcat(instruct_nom, " -m '");
		strcat(instruct_nom, (char*)mem_util_nom);
		strcat(instruct_nom, "%'");

		system(instruct_nom);

	//instruct_act shell instruction
		char instruct_act[100] = "sudo mosquitto_pub -t 'MEM_ACT_UTIL' -h ";

		strcat(instruct_act, broker_address);
		strcat(instruct_act, " -m '");
		strcat(instruct_act, (char*)mem_util_act);
		strcat(instruct_act, "%'");

		system(instruct_act);
	//end shell instruction

		memcpy(memories[PAST], memories[PRESENT], 
			sizeof(int)*MEMORIES_NUM);

		fclose(memFile);
		//close memFile	

		sleep(1);
	}
	fclose(addrFile);
	//close addrFile

	return 0;
}
