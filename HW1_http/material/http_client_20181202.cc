// http_client_20181202.cc

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

// buf의 크기로 사용할 상수
#define MAXDATASIZE 1000

int main(int argc, char *argv[]) {
	int sockfd;
	int numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo;
	int rv;
	char s[INET_ADDRSTRLEN];

	// 1. 인자를 받는다
	if(argc != 2) {
	  fprintf(stderr, "usage: http://hostname[:port][/path/to/file]\n");
	  exit(1);
	}
	
	// 2. 받은 인자 parsing 작업
	char* hostname; char* path = NULL; char* port = NULL;
	char http[] = "http://";
	char default_port[] = "80";
	char *a = argv[1];

	for (int i = 0; i < 7; i++) {
		if (http[i] != a[i]) {
			fprintf(stderr, "usage: http_client http://hostname[:port][/paht/to/file]\n");
			exit(1);
		}
	}
	
	hostname = strtok_r(a, "/", &path);
	hostname = strtok_r(NULL, "/", &path);
	hostname = strtok_r(hostname, ":", &port);
	if (*port == 0) port = default_port;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// 3. Find IP Address
	if((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0){
	    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	    return 1;
	}
	
	// 4. Create socket
	if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol))==-1){
	    perror("client: socket");
	    return 2;
	}

	// 5. Connect to server
	if(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
	    close(sockfd);
	    perror("connect");
	    exit(1);
	}
	inet_ntop(servinfo->ai_family, &((struct sockaddr_in*)servinfo->ai_addr)->sin_addr, s, sizeof s);

	// 6. servinfo 구조체 저장공간 해제
	freeaddrinfo(servinfo);
	   
	// 7. Send message
	strcpy(buf, "GET /");
	strcat(buf, path);
	strcat(buf, " HTTP/1.1");
	strcat(buf, "\r\n");

	strcat(buf, "Host: ");
	strcat(buf, hostname);
	strcat(buf, ":");
	strcat(buf, port);
	strcat(buf, "\r\n\r\n");

	if(send(sockfd, buf, strlen(buf), 0) == -1){
	    perror("send");
	    close(sockfd);
	    exit(1);
	}
	
	// 8. Receive message
	char* temp1;
	char* temp2;
	char* remaining;
	int content_length;
	int length_flag = 0;
	int overflow_flag = 0;
	int data_flag = 0;
	int sum = 0;
	FILE *out;
	
	if((numbytes = recv(sockfd, buf, sizeof buf, 0)) == -1){
			perror("recv");
			close(sockfd);
			exit(1);
		} buf[numbytes] = '\0';

	char buf_temp[strlen(buf)];
	strcpy(buf_temp, buf);// buf의 원본을 따로 저장해두기

	// 8.1 첫번째 메시지에서 content-length를 조사
	temp1 = strtok_r(buf, "\r", &remaining);
	printf("%s\n",temp1);
	while (temp1 != NULL){
		if(strcasestr(temp1, "content-length")){
			length_flag = 1;
			temp2 = strtok(temp1, " ");
			temp2 = strtok(NULL, " ");
			printf("%s bytes written to 20181202.out\n", temp2);
			content_length = atoi(temp2);
		}
		temp1 = strtok_r(NULL, "\r", &remaining);
	}
	if(length_flag == 0) { // out 파일 작성 X. Connection을 끊고 프로그램 종료
		printf("Content-Length not specified\n");
		close(sockfd);
		return 0;
	}

	char data[content_length+2]; // 데이터를 모담아 둘 배열
	if(content_length>1000) overflow_flag = 1;

	// 8.1 첫번째 메시지에 데이터 파일이 왔는지 조사
	strcpy(buf, buf_temp); // buf 다시 원본으로 복구
	temp1 = strtok_r(buf, "\r", &remaining);
	while (temp1 != NULL){
		if(strcmp(temp1, "\n") == 0){
			data_flag = 1;
			strcpy(data, remaining);
			sum += (strlen(remaining)-1);
			break;
		}
		temp1 = strtok_r(NULL, "\r", &remaining);
	}

	while(content_length > sum){
		if((numbytes = recv(sockfd, buf, sizeof buf, 0)) == -1){
			perror("recv");
			close(sockfd);
			exit(1);
		} buf[numbytes] = '\0';

		// 이미 데이터 기록이 시작됨 : 그대로 뒤에 붙이기만 하면 되는 경우
		if(data_flag == 1) {
			strcat(data, buf);
			sum += (strlen(buf));
		}

		else { // 아직 데이터 기록 시작X. 헤더가 끝나는 지점을 찾아야 함
			temp1 = strtok_r(buf, "\r", &remaining);
			while (temp1 != NULL){
				if(strcmp(temp1, "\n") == 0){
					data_flag = 1;
					strcpy(data, remaining);
					printf("%s\n", data);
					sum += (strlen(remaining)-1);
					break;
				}
				temp1 = strtok_r(NULL, "\r", &remaining); } }
	}

	// 맨 앞의 공백 삭제 시발! 에휴 이 한줄이면 되는 것을..
	for(int i = 0 ;i<strlen(data);i++) data[i] = data[i+1];

	// 모든 데이터 collect완료. 파일에 출력(파일 맨 앞에 '\n'이 붙어 본래 사이즈 +1 byte로 저장된다)
	out = fopen("20181202.out", "wt");
	if (out == NULL) { fprintf(stderr, "file open error \n"); exit(1); }
	fprintf(out, "%s", data);
	fclose(out);

	close(sockfd);
	return 0;
}
