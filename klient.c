#include <asm-generic/socket.h>
#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <sys/time.h>
#include <stdbool.h>

#define MAX_BUFF 1024
#define MAX_SERV 3
#define CHECK_TIME_US 220000
#define CHECK_TIME_S 5

char random_char() {
    // losowy znak a do z lub 0 do 9
    int ran = rand() % 36;
    if (ran < 26) {
        return 'a' + ran;
    } else {
        return '0' + (ran - 26);
    }
}

char* random_string(int length) {
    char* result = (char*)malloc((length + 1) * sizeof(char));
    if (result == NULL) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        exit(-1);
    }
    for (int i = 0; i < length; ++i) {
        result[i] = random_char();
    }
    result[length] = '\0';
    return result;
}

//funkcja sprawdza czy sa serwer jest na liscie
int check(struct sockaddr_in* servers, struct sockaddr_in server, int serv_number){
	int pom = 0;
	for(int i = 0; i < serv_number; i++){
		if(servers[i].sin_addr.s_addr==server.sin_addr.s_addr){
			pom = pom + 1;
		}
	}
	if(pom == 0){
		return 1;
	}else{
		return 0;
	}
}

//funkcja sprawdza na ktorej pozycji na liscie znajduje sie serwer
int which(struct sockaddr_in* servers, struct sockaddr_in server, int serv_number){
	int pom = serv_number;
	for(int i = 0; i < serv_number; i++){
		if(servers[i].sin_addr.s_addr==server.sin_addr.s_addr){
			pom = i;
		}
	}
	return pom;
}

void get_time(char buffer[], double diff){
	time_t c_time;
	time(&c_time);
	char time_str[64];
	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&c_time));
	printf("data: %s, wiadomosc: %s ", time_str, buffer);
	if(diff ==  0){
		printf("\n");
	}
	else{
		printf("czas od wyslania polecenia: %f\n", diff);
	}
}

int main(void){
	//zmienne
	struct addrinfo hints, *res=NULL;
	struct sockaddr_in c, servers[MAX_SERV];
	struct timeval tv;
	int sock, mess, c_len = sizeof(c), serv_number = 0, pos = 0;
	unsigned char buffer[MAX_BUFF];
	struct timespec start, end;
	double diff = 0;
	//deklaracje
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	tv.tv_sec=0;
	tv.tv_usec=0;
	//getaddr
	if(getaddrinfo(NULL, "1505", &hints, &res)!=0){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}
	//socket
	if((sock=socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}
	//bind
	if(bind(sock, res->ai_addr, res->ai_addrlen)!=0){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}
	//zmienne do select
	fd_set fdset;
	int sel;
	//petla
	printf("klient gotowy do dzialania\n");
	while(1){
		//select
		FD_ZERO(&fdset);
		FD_SET(sock, &fdset);
		if(tv.tv_usec == 0 && tv.tv_sec == 0){
			tv.tv_usec = CHECK_TIME_US;
			tv.tv_sec = CHECK_TIME_S;
		}
		sel = select(sock + 1, &fdset, NULL, NULL, &tv);
		if(sel==-1){
			printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
			exit(-1);
		}
		else if(sel>0){
			//odebranie wiadomosci
			mess = recvfrom(sock, buffer, MAX_BUFF, 0, (struct sockaddr*)&c, &c_len);
			clock_gettime(CLOCK_MONOTONIC, &end);
			if (mess == -1) {
     				printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        			exit(-1);
	    		}
			//sprawdzenie czy serwer na liscie
			if(check(servers,c,serv_number)==1 && serv_number < (MAX_SERV + 1)){
				memcpy(&servers[serv_number], &c, c_len);
				serv_number = serv_number + 1;
			}
			buffer[mess]='\0';
			//warunek by program nie liczyl diff dla przywitan serwerow
			if(start.tv_sec != 0 && start.tv_nsec != 0 && end.tv_sec !=0 && end.tv_nsec != 0){
				diff = (end.tv_sec - start.tv_sec)*1000.0;
				diff += (end.tv_nsec - start.tv_nsec)/1000000.0;
			}
			get_time(buffer, diff);
			//random mess
			snprintf(buffer, MAX_BUFF, "%s", random_string(19));
			pos = which(servers,c,serv_number);
			printf("pozycja serwera (wysylajacego wiadomosc) na liscie: %d\n", pos);
			printf("liczba aktywnych serwerow na liscie: %d\n", serv_number);
			//wyslanie wiadomosci
			int ran = rand() % serv_number; // losujemy do ktorego serwera wysylamy
			int ran_delay = rand() % 401 + 1500;
			usleep(ran_delay * 1000);
			mess = sendto(sock, buffer, MAX_BUFF, 0, (const struct sockaddr*)&servers[ran], c_len);
			clock_gettime(CLOCK_MONOTONIC, &start);
			if (mess == -1) {
     				printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        			exit(-1);
			}
        	}
		else{
			//timeout
			snprintf(buffer, MAX_BUFF, "%s", "jestes?");
			int ran = rand() % serv_number; // losujemy do ktorego serwera wysylamy
			mess = sendto(sock, buffer, MAX_BUFF, 0, (const struct sockaddr*)&servers[ran], c_len);
			clock_gettime(CLOCK_MONOTONIC, &start);
			if (mess == -1) {
     				printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        			exit(-1);
        		}
			mess = recvfrom(sock, buffer, MAX_BUFF, 0, (struct sockaddr*)&c, &c_len);
			clock_gettime(CLOCK_MONOTONIC, &end);
			if (mess == -1) {
     				printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        			exit(-1);
	    		}
			buffer[mess]='\0';
			if(start.tv_sec != 0 && start.tv_nsec !=0){
				diff = (end.tv_sec - start.tv_sec)*1000.0;
				diff += (end.tv_nsec - start.tv_nsec)/1000000.0;
			}
			get_time(buffer, diff);
			//czas odebrania i data
			//random mess
			pos = which(servers,c,serv_number);
			printf("pozycja serwera sprawdzanego czy aktywny na liscie: %d\n", pos);
			printf("liczba aktywnych serwerow na liscie: %d\n", serv_number);
		}
	}
	close(sock);
	return 0;
}
