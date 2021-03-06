/*
 ============================================================================
 Name        : PronyADMM_client.cpp
 Author      : Jianhua Zhang
 Description : Streaming data from PMU, Construct and update matrix H, C, w, update the a iteratively locally, 
		send it to server, and receive average a from server. 
 File        : Initiating and running the Server to receive data from PMU first.
 Data        : Feb,14th,2014
 Update      : S-ADMM algorithm on Mar,5th,2015
 Update	     : April,27th,2015 update the random delay model and time collection file name 
 Update	     : April,28th,2015 integrate Tao's delay module.
 CommandLine : ./PronyADMM <ip of server> <TCP_port> <sourcefile>
 ============================================================================
 */
#include "Prony_common.h"
#define rho 0.0001  // try smaller values, e.g. 1e-3, 1e-4, 1e-5;  0.06
#define Height_H 45
#define IniHeight 20
#define SamNum 90
#define MU 5.3

int main(int argc, char *argv[])
{
	//=================================== Initialization Phase ======================================//
	
	if (argc != 4) cout<<"The command format should be Prony Server_IP port# file-name."<<endl;
	unsigned Iteration = 1;
	int i = 0;
	struct timeval StartTime, EndTime, SendingTime, currenttime, Start, End, Result, t12, ReceivingTime, Ts_t12, t12_partII;
	gettimeofday (&Start, NULL); 

	srand (time(NULL));
	double RandomDelay;
	int RandomDelay_int;
    char Client_IP[50];
    char * tmp_IP;
    tmp_IP = GetHostIP();
    strcpy(Client_IP, tmp_IP);
    cout<<"Main Function gets CLientIP is "<<Client_IP<<endl;
    
	// For collecting timing
	ofstream timing_file;
	char filename1[50];
	char tempbuf1[5];
	char tempbuf2[5];
	char tempbuf3[5];

	time_t now_time;
	time(&now_time);
	struct tm * tm_info;
	tm_info = localtime(&now_time);
	strcpy(filename1, "PMUdata_file_");	
	snprintf(tempbuf1, 5, "%d", tm_info->tm_year+1900);	
	strcat(filename1, tempbuf1);
	strcat(filename1, "-");
	snprintf(tempbuf2, 5, "%d", tm_info->tm_mon + 1);	
	strcat(filename1, tempbuf2);
	strcat(filename1, "-");
	snprintf(tempbuf3, 5, "%d", tm_info->tm_mday);	
	strcat(filename1, tempbuf3);
	strcat(filename1, "-");
	snprintf(tempbuf1, 5, "%d", tm_info->tm_hour);	
	strcat(filename1, tempbuf1);
	strcat(filename1, ":");	
	snprintf(tempbuf2, 5, "%d", tm_info->tm_min);	
	strcat(filename1, tempbuf2);
	strcat(filename1, ":");
	snprintf(tempbuf3, 5, "%d", tm_info->tm_sec);	
	strcat(filename1, tempbuf3);
	strcat(filename1, "_");
	strcat(filename1, argv[3]);

	timing_file.open(filename1);
	timing_file << fixed << "t12	" <<"t23	"<<"Ts-t12-t23	" << "Ts	"<<"t41"<<endl;

	//1. Setup a TCP connection of client for sending localpara to Prony_server
  	int sockfd = 0;
   	struct sockaddr_in serv_addr;       
    	memset(&serv_addr, '0', sizeof(serv_addr));   
    	serv_addr.sin_family = AF_INET;
    	serv_addr.sin_port = htons(atoi(argv[2])); 
   	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0){
        	printf("\n inet_pton error occured\n");
        	return 1;
    	  } 
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)   {
            printf("\n Error : Could not create socket \n");
            return 1;
        } 
        if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)   {
            perror("\n Error : Connect Failed \n");
            return 1;
        } else {
             printf("\n TCP Connection has been set up\n");
             }	
		

	// form localpara message 
	char* buffer = (char*)malloc(512*sizeof(char));
   	int size, avgBufferLen;
	
	// Prepare receive buffer for avgpara
	char *avgBuffer = (char*)malloc(512*sizeof(char));
	long int SendTime;

	// For calculate avg parameters
	char *pht;
	mat error(ParaNum,1);
	//double cal_error;
   	//cal_error = double(1000000000);

	//======================= Read Data to output vector y ==========================//

	/* Note: The complete process is that 1) stream data from PMU with original sampling rate 2) implement the lowpass
                 filter and resampling the data with lower sampling rate.
           Note: Here we assume we are reading the resampling data to construct H and C, where y is delta delta
        */

	vec y(SamNum);
	ifstream inputFile(argv[3]);
  	string line;
  
  	while (getline(inputFile, line))
  	     {
       		istringstream ss(line);
       		ss >> y(i);
     	        i++;
  	     }
	
	//======================= Calculate initial parameter vector a ==================//
		
	// Construct Intermedia Matrix H, C, a 
   	//mat H(Height_H,ParaNum);
      	//mat C(Height_H,1); 
	mat I(ParaNum, ParaNum);
	mat a(ParaNum,1);
	mat new_a(ParaNum,1);
	mat avgpara(ParaNum,1);
	mat new_avgpara(ParaNum,1);
	mat w(ParaNum,1);
	mat new_w(ParaNum,1);
	int j, TT;
/*
 	// Initialize Intermedia Matrix H, C, a
   	for (i=0; i<Height_H; i++){
       		C(i,0) = y(ParaNum+i);
       	     }
	//C.print("C is");

   	for (i=0; i<Height_H; i++){
	    for (j=0; j<ParaNum; j++){
		H(i,j) = y(i+ParaNum-1-j);
		}
	    }
*/
	for (i=0; i<ParaNum; i++){
		for (j=0; j<ParaNum; j++){
			if (i==j){
				I(i,j) = 1;
			   } else {
					I(i,j) = 0;
			          }
		    }
	    }
	
	for (i=0; i<ParaNum; i++){ 
		a(i,0) = 1;
		w(i,0) = 0;
		avgpara(i,0) = 1;
	    }

	// ======================= The below is the loop =========================// 

    while(1){

	cout<<"====================== Iteration  "<<Iteration<<"  ======================"<<endl;
	//a.print("current a is");

	// Step 1: Update matrice  H and C, and Calculate new_a
	gettimeofday (&StartTime, NULL);
	//cout<<"Iteration "<<Iteration<<" start time is "<<StartTime.tv_sec*1000000 + StartTime.tv_usec<<endl;

	if (IniHeight+floor(Iteration/5)<Height_H){
		TT = IniHeight + floor(Iteration/5);
	   } else {
			TT = Height_H;
		  }

	mat H(TT,ParaNum); 

      	mat C(TT,1); 

	for (i=0; i<TT; i++){
       		C(i,0) = y(ParaNum+i);
       	     }
	//C.print("C is");

   	for (i=0; i<TT; i++){
	    for (j=0; j<ParaNum; j++){
		H(i,j) = y(i+ParaNum-1-j);
		}
	    }	

	new_a = inv(trans(H)*H + rho*I)*(trans(H) * C - w + rho*avgpara);
	//new_a.print("new_a is");

	// Step 2: Send out local parameter vector new_a

	// Record sending time
	gettimeofday (&SendingTime, NULL); 
	timer_sub(&StartTime, &SendingTime, &t12);
	//cout<<"Local computation time-part 1 is "<<t12.tv_sec*1000000 + t12.tv_usec<<endl;

	//cout<<"Local estimate's sending time is "<<SendingTime.tv_sec*1000000 + SendingTime.tv_usec<<endl;
  	// Format the export message
   	size = sprintf(buffer, "Iteration: %d, %s: %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\r\n Starttime: %ld \r\n\r\n", Iteration, argv[3], new_a(0), new_a(1), new_a(2), new_a(3), new_a(4), new_a(5), new_a(6), new_a(7), new_a(8), new_a(9), new_a(10), new_a(11), new_a(12), new_a(13), new_a(14), new_a(15), new_a(16), new_a(17), new_a(18), new_a(19), (SendingTime.tv_sec*1000000 + SendingTime.tv_usec));	

        //printf( "Client Write:\n %s \n", buffer);  
	//Here we let the computer sleep for a mount of time of delay
	RandomDelay = RandomGenerator(MU, SIGMA, LAMBDA, P);     // time unit here is ms
  	cout<<"Random Delay is "<<RandomDelay<<endl; 
	RandomDelay_int = int(RandomDelay);		 // time unit here is ms*10
	gettimeofday (&currenttime, NULL); 
	//cout<<"Pratical sending time is "<<currenttime.tv_sec*1000000 + currenttime.tv_usec<<endl;
	//usleep(RandomDelay_int);
	set_delay(Client_IP, RandomDelay_int);
    write(sockfd, buffer, size);
        
	

	// Step 3: Receive new_avg para from Prony_Server                  
	avgBufferLen = read(sockfd, avgBuffer, DEFAULT_MAX_BUFFER_LEN);
	gettimeofday(&ReceivingTime, NULL);
	//cout<<"Receiving avg paras from PronyDSM_Server at "<<ReceivingTime.tv_sec*1000000 + ReceivingTime.tv_usec<<" ... "<<endl;
	timer_sub(&currenttime, &ReceivingTime, &Ts_t12);
	cout<<"Ts_t12 is "<<Ts_t12.tv_sec*1000000 + Ts_t12.tv_usec<<" ... "<<endl;
    
    //set_delay(Client_IP, 0);


	avgBuffer[avgBufferLen]=0;

	if (avgBufferLen < 50) break;
	
	//cout<<"avgBuffer contains "<<avgBuffer<<endl;
	//Parse new_avgpara
	pht = strtok(avgBuffer, " ");
	pht = strtok(NULL, " ");
	pht = strtok(NULL, " ");
	//printf("This should be %s\n", pht);
 	for (j=0; j<ParaNum; j++){
		pht = strtok(NULL, " ");
		new_avgpara(j,0) = strtod(pht, NULL);
		//cout<<"new_avgpara ("<<j<<") is "<<new_avgpara(j,0)<<endl;
	    }
	pht = strtok(NULL, " ");
	pht = strtok(NULL, " ");
	SendTime = strtol(pht, NULL, 10);
	//printf("SendTime of z message is %ld\n", SendTime); 
	//cout<<"t41 is "<<ReceivingTime.tv_sec*1000000 + ReceivingTime.tv_usec-SendTime<<endl;

	// Step 4: Update newer matrix C, H, and local para, Iteration ====================/

	new_w = w + rho*(new_a - new_avgpara);

	a = new_a;
	avgpara = new_avgpara;
	w = new_w;

	gettimeofday (&EndTime, NULL);
	//cout<<"Iteration "<<Iteration<<" start time is "<<EndTime.tv_sec*1000000 + EndTime.tv_usec<<endl;	
	timer_sub(&ReceivingTime, &EndTime, &t12_partII);
	//cout<<"Local computation time-part II is "<<t12_partII.tv_sec*1000000 + t12_partII.tv_usec<<endl;
	//cout<<"Local computation t12 is "<<t12.tv_sec*1000000+t12_partII.tv_sec*1000000 + t12.tv_usec+t12_partII.tv_usec<<endl;
	timer_sub(&StartTime, &EndTime, &Result);
	cout<<"Ts is "<<Result.tv_sec*1000000+Result.tv_usec<<endl;

	timing_file << fixed <<t12.tv_sec*1000000+t12_partII.tv_sec*1000000 + t12.tv_usec+t12_partII.tv_usec<<"	"<<RandomDelay_int<<"	"<<Ts_t12.tv_sec*1000000 + Ts_t12.tv_usec<<"	"<<Result.tv_sec*1000000+Result.tv_usec<<"	"<<ReceivingTime.tv_sec*1000000 + ReceivingTime.tv_usec-SendTime<<endl;
	

/*	
	error = square(new_a - a);
	//error.print("square error is ");
	cal_error = error.max();
	cout<<"Max error value is "<<cal_error<<endl;
*/
     set_delay(Client_IP, 0);
	cout<<"========================= This is end of Iteration "<<Iteration<<" . ^_^ ========================= "<<endl;

/*
	gettimeofday (&currenttime, NULL);
	cout<<"Current time is "<<currenttime.tv_sec*1000000 + currenttime.tv_usec<<endl;
	//sleep(0.05);
	gettimeofday (&currenttime, NULL);
	cout<<"Current time is "<<currenttime.tv_sec*1000000 + currenttime.tv_usec<<endl;
*/
	Iteration++;	
	
    } //end big loop


	close(sockfd);
    set_delay(Client_IP, 0);
	cout<<"Prony algorithm ends happily.  ^_^"<<endl;
	gettimeofday (&End, NULL); 
 	timer_sub(&Start, &End, &Result);
	cout<<"Total time of PronyDSM algorithm at Client side is "<<Result.tv_sec*1000000 + Result.tv_usec<<endl;
	cout<<"The size of H matrix is "<<Height_H<<endl;

    	return 0;

}

