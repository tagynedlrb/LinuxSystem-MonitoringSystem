#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
//for IP addr
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#define ONE_LINE 80
#define PAST 0
#define PRESENT 1
#define JIFFIES_NUM 4
#define PORT 9001

enum jiffy{USER, USER_NICE, SYSTEM, IDLE} jiffy_enum;

void getIP(char* ipstr){
	struct ifreq ifr;
	//char ipstr[40];
	int s;
	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	strncpy(ifr.ifr_name, "enp0s3", IFNAMSIZ);
	
	if(ioctl(s, SIOCGIFADDR, &ifr) < 0){
		printf("Error");
	} else{
		inet_ntop(AF_INET, ifr.ifr_addr.sa_data+2, ipstr, sizeof(struct sockaddr));
//		printf("My IP : %s\n", ipstr);
	}
}

int main (void){
	char loadDataBuf[ONE_LINE] = {0};
	char cpuId[4] = {0};

	int jiffies[2][JIFFIES_NUM] = {0}, totalJiffies;
	int diffJiffies[JIFFIES_NUM];
	int idx;
	char broker_address[100];
	char hostIP[40];

	FILE* addrFile;
	FILE* statFile;

	//get broker IP address
	addrFile = fopen("./broker_ip.txt", "r"); // open broker_ip
	fscanf(addrFile, "%s", &broker_address[0]);

	//get host IP
	getIP(hostIP);

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
		float cpu_util = 100.0*(1.0-(diffJiffies[IDLE]/
					(double)totalJiffies));

		printf("Cpu usage : %f%%\n", cpu_util);

		char instruct[400] = {0};

		sprintf(instruct, "sudo mosquitto_pub -t 'mon/cpu' -h %s -m '{ \"IP\" : \"%s\", \"timestamp\" : \"%d\", \"cpu_usage\" : \"%f\" }'", broker_address, hostIP, (int)time(NULL), cpu_util);

		system(instruct);	
		memcpy(jiffies[PAST], jiffies[PRESENT], 
			sizeof(int)*JIFFIES_NUM);

		fclose(statFile);

		sleep(1);
	}
	fclose(addrFile);
	//close addrFile

	return 0;
}
