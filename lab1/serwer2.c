#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_BUFF 1024

char random_num() {
    char tab[10] = {'0','1','2','3','4','5','6','7','8','9'};
    int ran = rand() % 10;
	return tab[ran];
}

char* str(int length, char buffer[]) {
    char* result = (char*)malloc((length + 1) * sizeof(char));
    if (result == NULL) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        exit(-1);
    }
    result[0] = random_num();
    for (int i = 1; i < length; ++i) {
    result[i] = buffer[i-1];
    }
    result[length] = '\0';
    return result;
}

int main(void){
	//zmienne
	struct addrinfo hints, *res=NULL;
	struct sockaddr_in c;
	int sock, mess, c_len=sizeof(c);
	unsigned char buffer[MAX_BUFF];
	//deklaracje
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	//getaddr
	if(getaddrinfo("192.168.56.109", "1505", &hints, &res)!=0){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}
	//socket
	if((sock=socket(res->ai_family, res->ai_socktype, res->ai_protocol))==-1){
		printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
		exit(-1);
	}
	//petla
	printf("serwer gotowy do dzialania\n");
	snprintf(buffer, MAX_BUFF, "%s", "witaj, tu serwer 2!");
	mess = sendto(sock, buffer, MAX_BUFF, 0, res->ai_addr, res->ai_addrlen);
	while(1){
		mess = recvfrom(sock, buffer, MAX_BUFF, 0, (struct sockaddr*)&c, &c_len);
		buffer[mess]='\0';
		printf("%s\n", buffer);
		int ran_delay = rand() % 5000;
		usleep(ran_delay*1000);
		if(strlen(buffer)==19){
			snprintf(buffer, MAX_BUFF, "%s", str(20,buffer));
		}
		else{
			snprintf(buffer, MAX_BUFF, "%s", "serwer 2 aktywny");
		}
		mess = sendto(sock, buffer, MAX_BUFF, 0, res->ai_addr, res->ai_addrlen);
	}
	close(sock);
	return 0;
}
