/*
NAME: Karim Benlghalia
EMAIL: kbenlghalia@ucla.edu
ID: 105179657

*/



#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <mraa.h>
#include <math.h>
#include <poll.h>

#define B 4275
#define R0 100000
#define temp 298.15
#define temp1 273.15


mraa_aio_context DevSensor;
//mraa_gpio_context DevButton;

// Global Variables
int Get_temp = 1;
int log_option = 0;
int period = 1;
char Scale_Option = 'F';
int FdLog;
const int MAX = 20;
struct timeval TimeNow;
struct timeval TimeNext;
FILE* logFile = 0;
int port_number;
char* Hostname;
int sockfd;
int IdNumber;
int option_id;
int host_option;
// Functions prototypes
void CommandFunc(const char *ArgU);
void compare_str(const char *ArgU);
void Get_DevSensor();
void get_Time();
void closing_func();
float get_tempera();
int String_Match( const char * str1, const char *strt2);
void suttDown_func();
void open_connection(int port_number);
void Send_id();
struct sockaddr_in serv_addr;
struct hostent *server;

int main(int argc, char *argv[])
{
	atexit(closing_func);
	
	static const struct option long_options[] =
		{
            {"id", required_argument, NULL, 'i'},
		    {"host", required_argument, NULL, 'h'},
			{"period", required_argument, NULL, 'p'},
			{"scale", required_argument, NULL, 's'},
			{"log", required_argument, NULL, 'l'},
			{0, 0, 0, 0}};

	int c;
	while (1)
	{
		int option_index = 0;										  
		c = getopt_long(argc, argv, "", long_options, &option_index); 

		if (c == -1)
			break;

		switch (c)
		{
        case 'i':
			option_id = 1;
			if (strlen(optarg) != 9) {
			fprintf(stderr, "Error: ID must be a 9 digit number\n");
			exit(1);} 
			IdNumber = atoi(optarg);
			break;  // for --id option
        case 'h':
			host_option = 1;
			Hostname = optarg;
			
			break;  // for -
		case 'p':
			period = atoi(optarg);
			break; // for --period option
		case 'l':
			log_option = 1;
			logFile=fopen(optarg, "w");
			break; // for --log option
		case 's':
			if (optarg[0] == 'F')
			{
				Scale_Option = optarg[0];
				break;
			}
			else if (optarg[0] == 'C')
			{
				Scale_Option = optarg[0];
				break;
			}	// for --scale option
		default: //case '?':
			fprintf(stderr, "Correct usage: ./lab4b [--period=Sampling Interval (in seconds)][--scale= CELCIUS OR FARENHEIT][--log=FilName]\n");
			exit(1);
		}
	}
	if (optind < argc)
	{
	port_number = atoi(argv[optind]);
	}
	else{
		fprintf(stderr, "Error: no port number provided\n");
		exit(1);
	}

   	if (port_number <= -1 || port_number == 0)
	        {
		        fprintf(stderr, "Error: Invalid Port Number\n");
				exit(1);
		    }
	if(option_id!=1){
		fprintf(stderr, "Error: ID is Mandatory\n");
		exit(1);
	}

	if(host_option!=1){
		fprintf(stderr, "Error: Server host name is mantatory\n");
		exit(1);
	}
	if(log_option!=1){
		fprintf(stderr, "Error: Log file is mandatory\n");
		exit(1);
	}
    open_connection(port_number);
        
		
    close(STDIN_FILENO);
	close(STDOUT_FILENO);
	dup(sockfd);
	dup(sockfd);
	close(sockfd);
	
	Send_id();

	DevSensor = mraa_aio_init(1);
	if (DevSensor == NULL)
	{
		fprintf(stderr, "Error in initialization of the Temperature sensure\n");
		// mraa_deinit();
		exit(1);
	}
	//DevButton = mraa_gpio_init(60);
	//mraa_gpio_dir(DevButton, MRAA_GPIO_IN);
	//Set an interrupt on DevButton and call shuttDown_func when interrupt is triggered
	//mraa_gpio_isr(DevButton, MRAA_GPIO_EDGE_RISING, &suttDown_func, NULL);

	struct pollfd Pull_Fds[1];
	Pull_Fds[0].fd = STDIN_FILENO;
	Pull_Fds[0].events = POLLIN | POLLHUP | POLLERR;

	char buffer[MAX];
	char argBuffer[MAX];
	int charct = 0;

	for (;;)
	{
		gettimeofday(&TimeNow, NULL);
		
		get_Time();

		int retPoll = poll(Pull_Fds, 1, 1000);

		if (retPoll < 0)
		{
			fprintf(stderr, "Error in poll function: %s\n", strerror(errno));
			exit(1);
		}

		else if (retPoll > 0)
		{
			if (Pull_Fds[0].revents & POLLIN)
			
			{
				int count = read(STDIN_FILENO, buffer, MAX);
				if (count < 0)
				{
					fprintf(stderr, "Error reading from stdin%s\n", strerror(errno));
					exit(1);
				}

				int i = 0;
				while (i < count)
				{
					char ch = buffer[i];
					if (ch == '\n')
					{
						argBuffer[charct] = '\0';
						if (log_option)
						{
							char arg2[MAX];
							strcpy(arg2, argBuffer);
							strcat(arg2, "\n");	
							fputs(arg2,logFile);
						}

						compare_str(argBuffer);
						charct = 0; // Need to reset it to 0 to process next command
					}
					else
					{
						argBuffer[charct] = ch;
						charct++;
					}
					i++;
				}
			}
		}
	}

	exit(0);
}


void Get_DevSensor()
{
	float temperature = get_tempera();

	struct timespec ts;
	struct tm *TimeNow;
	clock_gettime(CLOCK_REALTIME, &ts);

	TimeNow = localtime((&(ts.tv_sec)));

	char Data[MAX];
	char hours[10];
	char minutes[10];
	char seconds[10];
	char tempe[10];

	sprintf(hours, "%02d", TimeNow->tm_hour);
	sprintf(minutes, "%02d", TimeNow->tm_min);
	sprintf(seconds, "%02d", TimeNow->tm_sec);
	sprintf(tempe, "%.1f", temperature);

	strcat(Data, hours);
	strcat(Data, ":");
	strcat(Data, minutes);
	strcat(Data, ":");
	strcat(Data, seconds);
	strcat(Data, " ");
	strcat(Data, tempe);
	strcat(Data, "\n");

	
	if (write(STDOUT_FILENO, Data, strlen(Data)) == -1)
	{
		fprintf(stderr, "Failed to write to STDOUT: %s\n", strerror(errno));
		exit(1);
	}
		
	if (log_option)
	{
		fputs(Data,logFile);
		
	}
	
}

void compare_str(const char *ArgU)
{

	if (String_Match(ArgU ,"STOP"))
		Get_temp = 0;
	// stop getting the samples
	if (String_Match(ArgU ,"START"))
		Get_temp = 1;
	// continue getting the samples
	if (String_Match(ArgU ,"OFF"))
		suttDown_func();
	// shut down the program
	if (String_Match(ArgU ,"SCALE=F"))
		Scale_Option = 'F';
	if (String_Match(ArgU ,"SCALE=C"))
		Scale_Option = 'C';
	if (String_Match("PERIOD=",ArgU ))
	{
		const char *tempe = ArgU + strlen("PERIOD=");
		period = atoi(tempe);
	}
	if (String_Match(ArgU ,"LOG"))
	{ //do nothing
	}
}
void get_Time()
{
	if ((TimeNow.tv_sec >= TimeNext.tv_sec) && Get_temp == 1)
	{
		Get_DevSensor();
		
		TimeNext = TimeNow;
		TimeNext.tv_sec += period;
		// update the next read
	}
}
float get_tempera()
{
	float temperature;
	int Temp_value = mraa_aio_read(DevSensor);
	float R = 1023.0 / Temp_value - 1.0;
	R = R0 * R;
	float ValueTemp_C = (1.0 / (logf(R / R0) / B + 1.0 / temp)) - temp1;
	float ValueTemp_F = (ValueTemp_C * 9) / 5 + 32;

	switch (Scale_Option)
	{
	case 'F':
		temperature = ValueTemp_F;
		break;
	case 'C':
		temperature = ValueTemp_C;
		break;

	default:
		break;
	}

	return temperature;
}
int String_Match( const char * str1, const char *strt2){

	int value = strncmp(str1, strt2, strlen(str1));

	return !value;
}
void suttDown_func()
{

	// when the button is pressed, prepare to exit
	struct timeval now;
	struct tm *TimeNow;

	gettimeofday(&now, NULL);

	TimeNow = localtime(&now.tv_sec);

	char Data[MAX];
	char hours[10];
	char minutes[10];
	char seconds[10];

	sprintf(hours, "%02d", TimeNow->tm_hour);
	sprintf(minutes, "%02d", TimeNow->tm_min);
	sprintf(seconds, "%02d", TimeNow->tm_sec);

	strcat(Data, hours);
	strcat(Data, ":");
	strcat(Data, minutes);
	strcat(Data, ":");
	strcat(Data, seconds);
	strcat(Data, " ");
	strcat(Data, "SHUTDOWN");
	strcat(Data, "\n");

	
	if (write(STDOUT_FILENO, Data,strlen(Data)) == -1)
	{
		fprintf(stderr, "Failed to write to STDOUT: %s\n", strerror(errno));
		exit(1);
	}

	if (log_option)
	{
		fputs(Data,logFile);
	}

	exit(0);
}

void closing_func()
{
	
	mraa_aio_close(DevSensor);
	//mraa_gpio_close(DevButton);
}


void open_connection(int port_number){
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0)
            {
                fprintf(stderr, "Error when creating socket in the client side: %s\n", strerror(errno));
                exit(1);
            }
            // get the host name of the server
            server = gethostbyname(Hostname);
            if (!server)
            {
                fprintf(stderr, "Error when obtaining server host name: %s\n", strerror(errno));
                exit(1);
            }
            // fill in the server address

            bzero((char *)&serv_addr, sizeof(serv_addr));
            serv_addr.sin_family = AF_INET;
            bcopy((char *)(server->h_addr), (char *)&serv_addr.sin_addr.s_addr, server->h_length);
            serv_addr.sin_port = htons(port_number);
            // connect to the server by its address
            if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                fprintf(stderr, "Error connecting to the socked in the client side: %s\n", strerror(errno));
                exit(1);
            }

}


void Send_id(){
	char identity_char[15];
	sprintf(identity_char, "ID=%d\n", IdNumber);
	if(write(STDOUT_FILENO, identity_char, strlen(identity_char))==-1){
		fprintf(stderr, "Error: Failed to write ID to TCP Server: %s\n", strerror(errno));
        exit(1);

	}
}