#include "Prony_common.h"

using namespace std;



/* declare mutex */
	pthread_mutex_t mymutex;

/* function to intilize mutex */
void Mutex_initialization(){
	pthread_mutex_init(&mymutex, NULL);
     }
/* destroy mutext */
void Mutex_destroy(){
	pthread_mutex_destroy(&mymutex);
     }

int timer_sub(const struct timeval *start_time, const struct timeval *end_time, struct timeval *result){

	if (end_time->tv_usec >= start_time->tv_usec){
		result->tv_sec = end_time->tv_sec - start_time->tv_sec;
                result->tv_usec =  end_time->tv_usec - start_time->tv_usec;          
           } else {
			result->tv_sec = end_time->tv_sec - 1 - start_time->tv_sec;
                	result->tv_usec =  end_time->tv_usec + 1000000 - start_time->tv_usec;       			
                  }
        return 0;
}


/*************************************************************
 * Server_handle. When a client connects, the handler reads
 * the request and responds with appropriate files
 *************************************************************/
void *Server_handle(void * parmPtr)
{
        #define PARMPTR ((int *) parmPtr)
        int Client_sockfd = *PARMPTR;
	/* Claim receiving and transmitting buffer */
	char Buffer[DEFAULT_MAX_BUFFER_LEN];
	int BufferLen;
        int length;
	long int SendTime; 
	char ph2[512], pht[512];
	char *ph1;
        struct timeval tvalRecieve, tvalParse, result;
	
	/* lock mutex */
	pthread_mutex_lock (&mymutex);

	/* Read data from client, we here only read once.*/
	BufferLen=read(Client_sockfd, Buffer, DEFAULT_MAX_BUFFER_LEN);
	Buffer[BufferLen]=0;

 	gettimeofday (&tvalRecieve, NULL);
                   
        printf("Buffer message with size %d is \n %s", BufferLen, Buffer);
	
        // Parse messge 
	        	
	ph1 = strtok(Buffer, "\r\n");
	//strcpy(ph1, strtok(Buffer, "\r\n"));
        
        length = strlen(ph1);       
        //printf("Parsed First line message is %s with length %d. \n", ph1, length);	
	//ph2 = strtok(ph1+(length+1)*sizeof(char), "\r\n");
	strcpy(ph2, strtok(ph1+(length+1)*sizeof(char), "\r\n"));
        printf("Second Parsed message is %s\n", ph2);
        //pht = strtok(ph2, " ");
	//num = rand()%2;
	//if (num) sleep(1);
	strcpy(pht, strtok(ph2, " "));
        printf("First word in second line is %s\n", pht);
        //pht = strtok(NULL, " ");
	strcpy(pht, strtok(NULL, " "));
        printf("Second word in second line is %s\n", pht);      
        SendTime = strtol(pht, NULL, 10);
        printf("SendTime of message is %ld\n", SendTime); 

	/* unlock mutex */
	pthread_mutex_unlock (&mymutex);

        printf("Total communication time in microseconds is %ld \n", (tvalRecieve.tv_sec)*1000000+tvalRecieve.tv_usec - SendTime);      
	//free(parmPtr);
	gettimeofday (&tvalParse, NULL);
	timer_sub(&tvalRecieve, &tvalParse, &result);
	printf("Message Processing time is %ld \n", result.tv_sec*1000000+result.tv_usec);
        printf("this thread exits hahhaa. *********************\n\n");
    	pthread_exit(ph1);       
}


double RandomGenerator(double mu, double sigma, double lambda, double p)
{
	double P_x, RANDSEED, step_size, x, error, RandomDelay, criteria=1000000.0;
	int i, t_length=200;
	step_size = 0.1;

	//Step1: Generate a random number between 0 and 1;	
	RANDSEED = double(random()%1000000)/1000000;
	//printf ("First number: %f\n", RANDSEED);
	
	//Step2: Search the random delay which corresponds to generated random number
	for(i=0; i<(int(t_length/step_size)+1); i++){
		x =  i*step_size;
		P_x =  0.5*(erf(mu/sqrt(2)/sigma) + erf((x-mu)/sqrt(2)/sigma)) + 0.5*(p-1)*exp(0.5*lambda*lambda*sigma*sigma+mu*lambda)*exp(-lambda*x)*(erf((lambda*sigma*sigma+mu)/sqrt(2)/sigma)+erf((x-lambda*sigma*sigma-mu)/sqrt(2)/sigma));
		error = P_x - RANDSEED;
		if (error < 0) error = -error;
		if (error < criteria){
			RandomDelay = x;
			criteria = error;
		}
	} 	 
	return RandomDelay;
} 

int set_delay(char ip[], int delay){
	char command[64];
	snprintf(command, sizeof(command),"/bin/sh -c 'nsdelay %s %d'",ip, delay);
	if(system(command)<0){
		printf("Error: failed to set the delay.\n");
		return -1;	
	}
	return 0;
}

char* GetHostIP(){
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char addr[50];
    char* getClientIP=(char *)malloc(50);
    
    getifaddrs (&ifap);
    
    for(ifa=ifap; ifa; ifa = ifa->ifa_next){
        if(ifa->ifa_addr->sa_family==AF_INET){
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            strcpy(addr, inet_ntoa(sa->sin_addr));
            cout<<"addr is "<<addr<<endl;
            if(strncmp(addr, "172.16.0",6) == 0){
                cout<<"The Client_IP is "<<addr<<endl;
                getClientIP = &addr[0];
                break;
            }
        }
    }
    return getClientIP;
}





