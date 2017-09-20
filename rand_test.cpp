/* srand example */
#include <cstdio>      /* printf, NULL */
#include <cstdlib>     /* srand, rand */
#include <time.h>       /* time */
#include <cmath> 
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <algorithm>
#include <vector>
//#include "Prony_common.h"

using namespace std;
/*
double RandomGenerator(double mu, double sigma, double lambda, double p)
{
	double P_x, RANDSEED, step_size, x, error, RandomDelay, criteria=1000000.0;
	int i, t_length=250;
	step_size = 0.1;

	//Step1: Generate a random number between 0 and 1;	
	//srand (time(NULL));
	RANDSEED = double(random()%1000000)/1000000;
	printf ("First number: %f\n", RANDSEED);
	
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
*/

int main ()
{
	
	struct timeval currenttime, currenttime_1, sleeptime;
	double mu, sigma, lambda, p, RandomDelay;
	int RandomDelay_int, i;
	vector<int> delay_array;
	int pre_delay, min_delay, client_index, size;
	int ClientNum = 4;
	mu = 105;
	sigma = 0.078;
	lambda = 1.39;
	p = 0.58;
    srand (time(NULL));
    char IPset[4][50];
    strcpy(IPset[0],"172.16.0.1");
    strcpy(IPset[1],"172.16.0.5");
    strcpy(IPset[2],"172.16.0.9");
    strcpy(IPset[3],"172.16.0.13");
    
    char* Buffer = (char*)malloc(512*sizeof(char));
    
    size = sprintf(Buffer, "Iteration:, new_avgpara: \r\nStarttime:  \r\n\r\n");
    cout<<"Buffer contains "<<"\r\n"<<Buffer<<endl;
    
    long int t23 = 105000, REC_t23;
    char tempbuf[25];
    char* pht;
    snprintf(tempbuf, 25,"t23: %ld\r", t23);
    strcat(Buffer,tempbuf);
    cout<<"Again Buffer contains "<<"\r\n"<<Buffer<<endl;
    pht = strtok(Buffer, "\r\n");
    cout<<pht<<endl;
    pht = strtok(NULL, "\r\n");
    cout<<pht<<endl;

    pht = strtok(NULL, " ");
    pht = strtok(NULL, " ");
    REC_t23 = strtol(pht, NULL, 10);
    cout<<"t23 is "<<REC_t23<<endl;
    

/*
	cout<<"Artifical Delay is ";
	for (i=0; i<ClientNum; i++){
		cout<<"t41_"<<i+1<<"	";
	}
	cout<<endl;
*/
/*	for (i=0; i<ClientNum; i++){
		RandomDelay = RandomGenerator(mu, sigma, lambda, p);     // time unit here is ms
  		cout<<"Random Delay is "<<RandomDelay<<endl;
		RandomDelay_int = int(RandomDelay);
		delay_array.push_back(RandomDelay_int);
        cout<<"delay_array["<<i<<"] is "<<delay_array[i]<<endl;
		//usleep(100);
	    }
	
	for (i=0; i<ClientNum; i++){
        //set(IPset[i], delay_array[i]);
*/
/*
		client_index = min_element(delay_array.begin(), delay_array.end()) - delay_array.begin();
		cout<<"client_index is "<<client_index<<endl;
		min_delay = delay_array[client_index];
		cout<<"min_delay is "<<min_delay<<endl;
		if (i==0){
			pre_delay = min_delay;
			usleep(min_delay);
			cout<<"if i==0, min_delay is "<<min_delay<<endl;
		} else {
			usleep(min_delay-pre_delay);			
			cout<<"if i!=0, sleep time is "<<min_delay-pre_delay<<endl;
			pre_delay = min_delay;
		}
		delay_array[client_index] = 1000000000
		cout<<"Update delay_array is "<<delay_array[client_index]<<endl;
	}
    
    sleep(1000);
    
    for (i=0; i<ClientNum; i++){
        //set(IPset[i], 0);
    }
*/
/*
	RandomDelay = RandomGenerator(mu, sigma, lambda, p);
	RandomDelay_int = int(RandomDelay*1000);
  	cout<<"Random Delay is "<<RandomDelay_int<<endl;
	gettimeofday (&currenttime, NULL);
	cout<<"Current time is "<<currenttime.tv_sec*1000000+currenttime.tv_usec<<endl;
	usleep(RandomDelay_int);
 	gettimeofday (&currenttime_1, NULL);
	cout<<"After sleep Current time is "<<currenttime_1.tv_sec*1000000+currenttime_1.tv_usec<<endl; */
  	return 0;

}
