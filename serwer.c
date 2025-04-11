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
#define MAX_SERV 2
#define CHECK_TIME_US 0
#define CHECK_TIME_S 1

//generacja bufora gry
void game_buff_gen(unsigned char *game_buff){
	int r;
	game_buff[0] = '0';
	game_buff[1] = '1';
	game_buff[2] = ' ';
	r = rand() % 2;
	if(r == 0){
		game_buff[3] = '0';
	}else{
		game_buff[3] = '1';
	}
	r = rand() % 2;
	if(r == 0){
		game_buff[4] = '0';
	}else{
		game_buff[4] = '1';
	}
	game_buff[5] = '1';
	game_buff[6] = '0';
	game_buff[7] = '1';
}

//rzut moneta
void game(unsigned char *game_buff){
	int r = rand() % 2;
	game_buff[2] = game_buff[3];
	game_buff[3] = game_buff[4];
	if(r == 0){
		game_buff[4] = '0';
	}else{
		game_buff[4] = '1';
	}
}

//wydrukowanie obecnych 3 ostatnich rzutow
void print_pattern(unsigned char *game_buff){
	printf("wzorzec: %c %c %c\n",game_buff[2],game_buff[3],game_buff[4]);
}

//info diagnostyczne
void print_game_sum(unsigned char *buffer, int count, int game_number, int win_node, int bit_node){
	int num;
	if(buffer[2] == '0'){
		num = 1;
	}
	else{
		num = 2;
	}
	printf("INFO DIAGNOSTYCZNE -----------------------------------------------------\n");
	printf("odebrano win od node %d: %s\n",num,buffer);
	printf("liczba gier %d\n",game_number);
	printf("liczba wygranych: %d\n", win_node);
	printf("liczba losowan do wygranej: %d\n", bit_node);
	//k - dlugosc wzorca, n - liczba rzutow, m - liczba wystapien wzorca
	double k = 3;
	double n = (double)count;
	double m = (double)win_node;
	double p = (m / (n - k + 1));
	double e = n / m;
	printf("prawopodobienstwo wystapienia danego wzorca: %.6f\n", p);
	printf("estymowana liczba rzutow do wystapienia danego wzorca: %.6f\n", e);
	printf("------------------------------------------------------------------------\n");
}

//podsumowanie
void print_game_sum_all(int num, int count, int game_number, int win_node){
	printf("PODSUMOWANIE node %d ----------------------------------------------------\n", num);
	printf("liczba gier %d\n",game_number);
	printf("liczba wygranych: %d\n", win_node);
	printf("liczba losowan: %d\n", count);
	//k - dlugosc wzorca, n - liczba rzutow, m - liczba wystapien wzorca
	double k = 3;
	double n = (double)count;
	double m = (double)win_node;
	double p = (m / (n - k + 1));
	double e = n / m;
	printf("prawopodobienstwo wystapienia wzorca: %.6f\n", p);
	printf("estymowana liczba rzutow do wystapienia wzorca: %.6f\n", e);
	printf("------------------------------------------------------------------------\n");
}


int main(void){
	//zmienne ----------------------------------------------------------------------------------------------------------------
	struct addrinfo hints, *res=NULL; //structy potrzebne do funkcji UDP
	struct sockaddr_in c, servers[MAX_SERV]; //structy przechowujace informacje o zgloszonych node
	struct timeval tv;
	fd_set fdset;
	int sock, mess, sel;
	int c_len = sizeof(c);
	int serv_number = 0, game_number = 0, win_node1 = 0, win_node2 = 0, max_game;
	int bit_node1 = 2, bit_node2 = 2, count = 2;
	unsigned char buffer[MAX_BUFF]; //bufor wiadomosci
	unsigned char game_buff[8]; //bufor gry
	//inicjalizacja ----------------------------------------------------------------------------------------------------------
	//alokacja pamieci
	memset(&hints, 0, sizeof(struct addrinfo));
	memset(servers, 0, sizeof(servers));
	hints.ai_family = PF_INET; //socket bedzie uzywal IPv4
	hints.ai_socktype = SOCK_DGRAM; //socket bedzie korzystal z UDP
	hints.ai_flags = AI_PASSIVE; //flaga serwera
	//wyzerowanie czasu
	tv.tv_sec=0;
	tv.tv_usec=0;
	//getaddr ----------------------------------------------------------------------------------------------------------------
	if(getaddrinfo(NULL, "1234", &hints, &res)!=0){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}
	//socket -----------------------------------------------------------------------------------------------------------------
	if((sock=socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}
	//bind -------------------------------------------------------------------------------------------------------------------
	if(bind(sock, res->ai_addr, res->ai_addrlen)!=0){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}
	printf("START NASLUCHIWANIA ----------------------------------------------------\n");
	while(serv_number < MAX_SERV){
		//rec ------------------------------------------------------------------------------------------------------------
		mess = recvfrom(sock, buffer, MAX_BUFF, 0, (struct sockaddr*)&c, &c_len);
		if (mess == -1) {
			printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
			exit(-1);
		}
		buffer[mess]='\0';
		if(buffer[0] == '0' && buffer[1] == '0' && buffer[3] == '1' && buffer[4] == '0' && buffer[5] == '1'){
			if(buffer[2] == '0'){
				memcpy(&servers[0], &c, c_len);
				printf("odebrano hello od node 1: %s\n",buffer);
			}
			else if(buffer[2] == '1'){
				memcpy(&servers[1], &c, c_len);
				printf("odebrano hello od node 2: %s\n",buffer);

			}
			serv_number = serv_number + 1;
		}
	}
	//generacja bufora gry ---------------------------------------------------------------------------------------------------
	game_buff_gen(game_buff);
	printf("PODAJ LICZBE GIER ------------------------------------------------------\n");
	scanf("%d",&max_game);
	printf("START GRY --------------------------------------------------------------\n");
	//gra --------------------------------------------------------------------------------------------------------------------
	while(game_number < max_game){
		//wyzerowanie czasu ----------------------------------------------------------------------------------------------
		FD_ZERO(&fdset);
		FD_SET(sock, &fdset);
		if(tv.tv_usec == 0 && tv.tv_sec == 0){
			tv.tv_usec = CHECK_TIME_US;
			tv.tv_sec = CHECK_TIME_S;
		}
		//sel ------------------------------------------------------------------------------------------------------------
		sel = select(sock + 1, &fdset, NULL, NULL, &tv);
		if(sel==-1){
			printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
			exit(-1);
		}
		else if(sel>0){
			//rec ----------------------------------------------------------------------------------------------------
			mess = recvfrom(sock, buffer, MAX_BUFF, 0, (struct sockaddr*)&c, &c_len);
			if (mess == -1) {
					printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
					exit(-1);
			}
			buffer[mess]='\0';
			if(buffer[0] == '1' && buffer[1] == '0' && buffer[3] == '1' && buffer[4] == '0' && buffer[5] == '1'){
				if(buffer[2] == '0'){
					game_number = game_number + 1;
					win_node1 = win_node1 + 1;
					print_game_sum(buffer,count,game_number,win_node1,bit_node1);
					bit_node1 = 0;
					bit_node2 = 0;
				}
				else if(buffer[2] == '1'){
					game_number = game_number + 1;
					win_node2 = win_node2 + 1;
					print_game_sum(buffer,count,game_number,win_node2,bit_node2);
					bit_node1 = 0;
					bit_node2 = 0;
				}
			}
		}
		else{
			//send -----------------------------------------------------------------------------------------------------------
			game(game_buff);
			print_pattern(game_buff);
			bit_node1 = bit_node1 + 1;
			bit_node2 = bit_node2 + 1;
			count = count + 1;
			snprintf(buffer, MAX_BUFF, "%s", game_buff);
			mess = sendto(sock, buffer, MAX_BUFF, 0, (const struct sockaddr*)&servers[0], c_len);
			if (mess == -1) {
				printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
				exit(-1);
			}
			mess = sendto(sock, buffer, MAX_BUFF, 0, (const struct sockaddr*)&servers[1], c_len);
			if (mess == -1) {
				printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
				exit(-1);
			}
		}
	}
	print_game_sum_all(1,count,game_number,win_node1);
	print_game_sum_all(2,count,game_number,win_node2);
	close(sock);
	return 0;
}
