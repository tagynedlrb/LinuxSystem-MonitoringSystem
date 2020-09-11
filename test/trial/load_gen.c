#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define ONE_LINE 80
#define PAST 0
#define PRESENT 1
#define JIFFIES_NUM 4
#define PORT 1883

enum jiffy{USER, USER_NICE, SYSTEM, IDLE} jiffy_enum;

int main (void){
	char loadDataBuf[ONE_LINE] = {0};
	char cpuId[4] = {0};

	int jiffies[2][JIFFIES_NUM] = {0}, totalJiffies;
	int diffJiffies[JIFFIES_NUM];
	int idx;
	char* broker_address="223.194.134.76";


	FILE* statFile;

	while(1){
		statFile = fopen("/proc/stat", "r");
		fscanf(statFile, "%s %d %d %d %d",
		cpuId, &jiffies[PRESENT][USER], &jiffies[PRESENT][USER_NICE],
		&jiffies[PRESENT][SYSTEM], &jiffies[PRESENT][IDLE]);
	
		for(idx = 0, totalJiffies = 0; idx< JIFFIES_NUM; ++idx){
			diffJiffies[idx] =
			 jiffies[PRESENT][idx] - jiffies[PAST][idx];
			totalJiffies = totalJiffies + diffJiffies[idx];
		}
		float cpu_util_float = 100.0*(1.0-(diffJiffies[IDLE]/
					(double)totalJiffies));
//		char cpu_util_test[100] = (100.0*(1.0-(diffJiffies[IDLE]/
//					(double)totalJiffies))); 
		char cpu_util[100];
		sprintf(cpu_util, "%f", cpu_util_float);

		
		printf("Cpu usage : %f%%\n", cpu_util_float);

		char temp[100] = "sudo mosquitto_pub -t 'CPU_UTIL' -h ";

		strcat(temp, broker_address);
		strcat(temp, " -m '");
		strcat(temp, (char*)cpu_util);
		strcat(temp, "%'");
		char* instruction = temp;

		system(instruction);	
		memcpy(jiffies[PAST], jiffies[PRESENT], 
			sizeof(int)*JIFFIES_NUM);

		fclose(statFile);

		sleep(0.5);
	}
	return 0;
}
