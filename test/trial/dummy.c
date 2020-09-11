#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(void){
	pid_t pid;
	int i=0, status, sum;
	int n[8000] = {0};

	while(i<100){
		pid = fork();
		if(pid>0){	//parent
			i++;
			if(i<100) continue;
			else{
				i=0;
				sleep(2);
				continue;
			}
			return 0;
		}
		else if(pid==0){	//child
			printf("child : %d exit \n", getpid());
			while(1){
				for(i=0;i<8000;i++){
					sum+=n[i];
				}
			i=0;
			}
			printf("sum = %d\n", sum);	//put it in while
			return 0;
			}
		else{
			fprintf(stderr, "fork error");
			return 1;
		}
	}
}
