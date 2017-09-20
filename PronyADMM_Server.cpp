/*
 ============================================================================
 Name        : PronyADMM_Server.cpp
 Author      : Jianhua Zhang
 Description : Initialize the Server to receive local estimated parameters from clients, average them, and then broadcast them back to each client.
 File        : ISO center of power system.  In this version, we only use TCP connection.
 Data        : Feb,14th,2014
 Update	     : April,7th,2015 Basic S-ADMM algorithm with time collection
 Update	     : April,27th,2015 update the random delay model and time collection file name 
 Update	     : April,28th,2015 integrate Tao's delay module.
 Update      : May,5th,2015 fix bug
 CommandLine : ./ADMMServer <TCP_port> <# of PMUs>
 ============================================================================
 */
#include "Prony_common.h"
#define ERROR 0.00001
#define imagError 0.005
#define SamNum 90

int main(int argc, char *argv[])
{
	//========================================= Initialization Phase ========================================//
	if (argc != 3) cout<<"The command format should be ./ADMMServer <TCP_port> <# of PMUs>."<<endl;
	int ClientNum;
	ClientNum = atoi(argv[2]);
	vector<int> delay_array;
	//int pre_delay, min_delay, client_index;
	int i,j;
	srand (time(NULL));
	double RandomDelay;
	int RandomDelay_int;
    char IPset[ClientNum][50];
    strcpy(IPset[0],"172.16.0.1");
    strcpy(IPset[1],"172.16.0.5");
    strcpy(IPset[2],"172.16.0.9");
    strcpy(IPset[3],"172.16.0.13");
    double MU[ClientNum];
    MU[0] = 105;
    MU[1] = 104;
    MU[2] = 51.5;
    MU[3] = 49.5;
    

	struct timeval Start, Result, t34_start, t34_end, t34_result, Tstart, Tend, Tresult;
	
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
	//itoa((now->tm_year+1900), tempbuf1, 10);
	strcpy(filename1, "Serverdata_file_");	
	snprintf(tempbuf1, 5, "%d", tm_info->tm_year+1900);	
	strcat(filename1, tempbuf1);
	strcat(filename1, "-");
	//itoa(now->tm_mon + 1, tempbuf2, 10);
	snprintf(tempbuf2, 5, "%d", tm_info->tm_mon + 1);	
	strcat(filename1, tempbuf2);
	strcat(filename1, "-");
	//itoa(now->tm_mday, tempbuf3, 10);
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

	timing_file.open(filename1);
	timing_file << fixed <<"t34	"<<"Ts-t34	"<<"Ts	";
	for (i=0; i<ClientNum; i++){
		timing_file << fixed <<"t41_"<<i+1<<"	";
	}
	timing_file << fixed <<endl;

	int CountClient;
	unsigned Iteration = 0;
        pthread_t handler_thread[ClientNum];
	string list[ClientNum];
	char * thread_result = (char *)malloc(512);;
	void * exit_status;
	double localpara[ClientNum][ParaNum];
	double avgpara[ParaNum], new_avgpara[ParaNum], error[ParaNum];
	double sum, cal_error;
	char *pht;
	double origeigen_imag[3];
	origeigen_imag[0] = 3.124981;
	origeigen_imag[1] = 5.56238;
	origeigen_imag[2] = 6.095;
	int root_count;

	// Initialize avg vectors
	for (i=0; i<ParaNum; i++){
		avgpara[i] = 0;
		new_avgpara[i] = 0;
	    }

	// Prepare sending message
	char Buffer[DEFAULT_MAX_BUFFER_LEN];
 	char* avgBuffer = (char*)malloc(512*sizeof(char));
  	unsigned size;
	struct timeval tvalStart, currenttime;

	//Set up TCP socket of server side for collection local parameters for each client
	int Server_sockfd;
	struct sockaddr_in LocalAddr;

	int Client_sockfd[ClientNum];
	struct sockaddr_in Client_address[ClientNum];
	//int ipAddr;
	int Client_len = sizeof(Client_address[0]);
	// collecting IP addresses of clients from TCP connection for latter UDP connection
	vector<sockaddr_in> client_list;   
	

	/* Initiate local TCP server socket */
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	LocalAddr.sin_port = htons(atoi(argv[1]));

	/* Create, bind and listen the TCP_socket */
	Server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (bind(Server_sockfd, (struct sockaddr *)&LocalAddr, sizeof(LocalAddr))==-1) {
		perror("Error: Can not bind the TCP socket! \n Please wait a few seconds or change port number.\n");
		exit(EXIT_FAILURE);
	}
	listen(Server_sockfd, DEFAULT_QUEUE_LEN);

	// Calculate roots
	typedef complex<double> dcomp;
   	dcomp z[PoleNum];
   	dcomp root_est_c[PoleNum];

   	int Degree = PoleNum;
   	double op[MDP1], zeroi[MAXDEGREE], zeror[MAXDEGREE]; // Coefficient vectors
   	int index; // vector index
	Mutex_initialization();
	
	// ========================================= The below is the loop =========================================// 
   gettimeofday (&Tstart, NULL);     
   while(Iteration < SamNum){

	//========================Step1: Collect the parameters from all clients===================//
	cout<<"Step1: start collecting localpara ..."<<endl;
	gettimeofday (&Start, NULL); 
	CountClient = 0;
	
	while(CountClient < ClientNum) {
                        
		gettimeofday (&currenttime, NULL);
		cout<<"Current time is "<<currenttime.tv_sec*1000000 + currenttime.tv_usec<<endl;

		// Accept connection		
		if(Iteration == 0){
			Client_sockfd[CountClient] = accept(Server_sockfd,(struct sockaddr *)&Client_address[CountClient], (socklen_t*)&Client_len);
			/*ipAddr = Client_address[CountClient].sin_addr.s_addr;
			inet_ntop(AF_INET, &ipAddr, Client_IP[CountClient], INET_ADDRSTRLEN);
			cout<<"Client_sockfd is "<<Client_sockfd[CountClient]<<endl;
			cout<<"Client_IP["<<CountClient<<"] is "<<Client_IP[CountClient]<<endl;	*/
		   }
							   
		// Handle connection
		pthread_create(&handler_thread[CountClient], NULL, Server_handle, (void*) &Client_sockfd[CountClient]);
                CountClient++;             
	}

        // Wait for all threads finish and return the values
	for (CountClient = 0; CountClient < ClientNum; CountClient++){
		pthread_join(handler_thread[CountClient], &exit_status);
		thread_result = (char *)exit_status;
		list[CountClient] = thread_result;
		//printf("I got paras %s from the thread %d. \n", list[CountClient].c_str(), CountClient);		
	    }	
	cout << "All threads completed."<<endl ;
	gettimeofday (&t34_start, NULL); 
	
	for (i=0; i<ClientNum; i++){
		strcpy(Buffer, list[i].c_str());
		pht = strtok(Buffer, " ");
		pht = strtok(NULL, " ");
		Iteration = strtol(pht, NULL, 10); 
		pht = strtok(NULL, " ");
		//printf("First word in para line is %s\n", pht);
 		for (j=0; j<ParaNum; j++){
			pht = strtok(NULL, " ");
			localpara[i][j]= strtod(pht, NULL);
			//cout<<"local para ["<<i<<"]["<<j<<"] is "<<localpara[i][j]<<endl;
		    }
	    }
	
	//========================== Step2: Average collected parameters =====================//
	cout<<"Step2: Average collected parameters ..."<<endl;
	cal_error = 0;
	for (j=0; j<ParaNum; j++){
		//1. update old avgpara
		avgpara[j] = new_avgpara[j];
		//2. calculate new_avgpara
		sum = 0;
		for (i=0; i<ClientNum; i++){
			sum = sum + localpara[i][j];
		    }	
		new_avgpara[j] = sum/ClientNum;
		cout<<"avgpara ["<<j<<"] is "<<new_avgpara[j]<<endl;
		//3. Calculate the squared error
		error[j] = (new_avgpara[j] - avgpara[j])*(new_avgpara[j] - avgpara[j]);
		//4. max error
		if (error[j] > cal_error) cal_error =  error[j];	
	    }	
	cout<<"Cal_error is "<<cal_error<<endl;

	if (cal_error <= ERROR) {
		//send out the finish package on TCP connection
		size = sprintf(avgBuffer, "Finish! \r\n\r\n");	
        	printf( "Error Server Write:\n %s \n", avgBuffer);        
		for (i=0; i<ClientNum; i++){
			write(Client_sockfd[i], avgBuffer, size); 
	    	    }
		break;
	   }


	// Check the roots of S-function
		//Compute the poles of the polynomial equation of Z function
   	
   	//Input the polynomial coefficients from the file and put them in the op vector
   	op[0] = 1;
   	for (index = 1; index < (Degree+1); index++){
        	op[index] = (-1)*new_avgpara[index-1];
    	    }

   	rpoly_ak1(op, &Degree, zeror, zeroi);
   	cout.precision(DBL_DIG);
/*
   	//cout << "The Z domain roots of Discentralized Prony Algorithm follow:\n";
  	//cout << "\n";
   	for (index = 0; index < Degree; index++){
        	//cout << zeror[index] << " + " << zeroi[index] << "i" << " \n";  
            }
   	cout << "\n";
*/
   	//cout << "Poles of S function: \n";

	//Compute the poles of the polynomial equation of S function
	root_count = 0;
 	for (index = 0; index < Degree; index++){
        	z[index] = dcomp(zeror[index], zeroi[index]);
        	root_est_c[index] = log(z[index])/Ts; 
		for (i=0; i<3; i++){
			if (abs(abs(root_est_c[index].imag()) - origeigen_imag[i]) < imagError) root_count++;
		   }
		
            }
	cout<<"The root_count number is "<<root_count<<endl;

	if (root_count == 6) {
		//send out the finish package on TCP connection
		size = sprintf(avgBuffer, "Finish! \r\n\r\n");	
        	printf( "Count Server Write:\n %s \n", avgBuffer);        
		for (i=0; i<ClientNum; i++){
			write(Client_sockfd[i], avgBuffer, size); 
	    	    }
		break;
	   }

	//======================== Step3: Broadcast the average parameters ==============//
        
	cout<<"Step3: Broadcast avgpara ..."<<endl;  
	// Record sending time
	gettimeofday (&tvalStart, NULL);

	//1: Format the export message
   	size = sprintf(avgBuffer, "Iteration: %d, new_avgpara: %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f\r\n Starttime: %ld \r\n\r\n", Iteration, new_avgpara[0], new_avgpara[1], new_avgpara[2], new_avgpara[3], new_avgpara[4], new_avgpara[5], new_avgpara[6], new_avgpara[7], new_avgpara[8], new_avgpara[9], new_avgpara[10], new_avgpara[11], new_avgpara[12], new_avgpara[13], new_avgpara[14], new_avgpara[15], new_avgpara[16], new_avgpara[17], new_avgpara[18], new_avgpara[19], (tvalStart.tv_sec*1000000 + tvalStart.tv_usec));	
	
	//printf( "Prony_Server Write:\n %s \n", avgBuffer);    

	for (i=0; i<ClientNum; i++){
		RandomDelay = RandomGenerator(MU[i], SIGMA, LAMBDA, P);     // time unit here is ms
  		cout<<"Random Delay is "<<RandomDelay<<endl; 
		RandomDelay_int = int(RandomDelay);
		delay_array.push_back(RandomDelay_int);
        cout<<"delay_array["<<i<<"] is "<<delay_array[i]<<endl;
		//delay_backup.push_back(RandomDelay_int);
	    }
	
	for (i=0; i<ClientNum; i++){
        /*
		client_index = min_element(delay_array.begin(), delay_array.end()) - delay_array.begin();
		min_delay = delay_array[client_index];
        cout<<"min_delay's client index is "<<client_index".  and IP is "<<Client_IP[client_index]<<endl;
		set_delay(Client_IP[client_index], min_delay/100);*/
        set_delay(IPset[i], delay_array[i]);
    }
       
    for (i=0; i<ClientNum; i++){
           write(Client_sockfd[i], avgBuffer, size);
    }
   /*
		if (i==0){
			pre_delay = min_delay;
			//usleep(min_delay);
			set_delay(Client_IP[client_index], min_delay/100);
		} else {
			//usleep(min_delay-pre_delay);
			set_delay(Client_IP[client_index], (min_delay-pre_delay)/100);
			pre_delay = min_delay;
		}
*/
		//delay_array[client_index] = MAX_DELAY;

	gettimeofday (&t34_end, NULL); 
	timer_sub(&t34_start, &t34_end, &t34_result);
	timer_sub(&Start, &t34_end, &Result);
	timing_file << fixed <<t34_result.tv_sec*1000000+t34_result.tv_usec<<"	"<<Result.tv_sec*1000000+Result.tv_usec-t34_result.tv_sec*1000000-t34_result.tv_usec<<"	"<<Result.tv_sec*1000000+Result.tv_usec;
	for (i=0; i<ClientNum; i++){
		timing_file << fixed <<"	"<<delay_array[i];
	}
	timing_file << fixed <<endl;
	delay_array.clear();
    
       for (i=0; i<ClientNum; i++){
           set_delay(IPset[i], 0);
       }

	cout<<"========================= This is end of Iteration "<<Iteration<<" . ^_^ ========================= "<<endl;
    } // end big loop
		
	Mutex_destroy();
	close(Server_sockfd);
	cout<<"Prony algorithm ends happily.  ^_^"<<endl;
	gettimeofday (&Tend, NULL); 
	timer_sub(&Tstart, &Tend, &Tresult);
	timing_file << fixed <<"Total algorithm convergency time is "<<Tresult.tv_sec*1000000+Tresult.tv_usec<<endl;

	// ========================================= Print out the roots =========================================// 
	for (index = 0; index < Degree; index++){
		if ((abs(root_est_c[index].imag())>3 && abs(root_est_c[index].imag())<4) || (abs(root_est_c[index].imag())>5 && abs(root_est_c[index].imag())<7)){
			cout<<root_est_c[index]<<endl;
		   }
            }

	
	
	return 0;

}

