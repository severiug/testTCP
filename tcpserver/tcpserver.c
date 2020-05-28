/* Severenchuk Aleksandr, sasha.severen@gmail.com, 26.05.2020 */
#include <stdio.h> 
#include <stdlib.h>
#include <string.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define MAX_SIZE 100000

int Demon( char* arg_port );

int sock_err( const char* function, int s )
{
  int err;
  err = errno;
  fprintf(stderr, "%s: socket error: %d\n", function, err);
  return -1;
}

void s_close( int s )
{
  close(s);
}

/*
* Функция для корректного принятия сообщения
* и записи его в файл *f.
*/
int recv_string( int cs, FILE *f )
{
  char buf[MAX_SIZE];
  int flags = 0;
  int rcv = 0, i,count = 0;
  if ((rcv = recv(cs, buf, MAX_SIZE, flags)) > 0)
  {	
    for (i = 0; i < rcv; i++) {
	if (buf[i] == '\n')
	{		
	  fwrite(buf, sizeof(char), rcv, f);
	  fclose(f);
	  break;
	}
    count = rcv;
    }
  }
  return count;
}

/* функция определения времени в нужном формате */
char* getTime() 
{
	long int ttime;
	ttime = time (NULL);
  char *ret;
  ret = (char*) malloc(100);
  memset(ret, 0, 100);
  ret = ctime (&ttime);
  return (ret);
}

/*
* Функция для работы сервера-демона. 
* Включает функционал для работы с несколькими клиентами
* и записывает данные с передаваемыми сообщения и
* информацией о клиенте в файлы ip_address.txt и ip_address.log.
*/
int Demon( char* arg_port )
{
  int shandle;
  struct sockaddr_in addr;
  unsigned int ip;
  char *NamLog;
  char *RecvFile;
  char host[20];
  FILE* log_file;
  FILE* recv_msg;
  int err;
  int flags = 0;
  int nbytes = 0;
  pid_t pid;

  NamLog = (char *) malloc(25 * sizeof(char));
  RecvFile = (char*) malloc(25 * sizeof(char));
  
  /* Создание TCP сокета */
  shandle = socket(AF_INET, SOCK_STREAM, 0);
  if (shandle < 0) return sock_err("socket", shandle);

   /* Заполнение адреса прослушивания */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(arg_port)); 
  addr.sin_addr.s_addr = htonl(INADDR_ANY); 

  /* Связывание сокета и адреса прослушивания */
  if (bind(shandle, (struct sockaddr*) &addr, sizeof(addr)) < 0) 
    return sock_err("bind", shandle);
  /* Начало прослушивания */
  if (listen(shandle, 1) < 0)
	return sock_err("listen", shandle);
	
  do {
		/* Принятие очередного подключившегося клиента */
	   int addrlen = sizeof(addr);
	   int cs = accept(shandle, (struct sockaddr*) & addr, (socklen_t*)&addrlen);		
	   if (cs < 0)
	   {
	     sock_err("accept", shandle);
		 break;
	   }
       if ((pid=fork()) == 0) 
       {
	       inet_ntop(AF_INET, &addr.sin_addr, host,sizeof(host));
    		 ip = ntohl(addr.sin_addr.s_addr);
    		 memset(NamLog, 0, 25);
    		 strcpy(NamLog, host);
    		 strncat(NamLog, "\0", 1);
    		 strcat(NamLog, ".log");
    		 memset(RecvFile, 0, 25);
         strcpy(RecvFile, host);
         strncat(RecvFile, "\0", 1);
         strcat(RecvFile, ".txt");
         
         /* Cоздаем лог файл подключившегося клиента */
		 if ((log_file = fopen(NamLog, "a+")) != NULL) 
		 {
		   fprintf(log_file, "%s Connected client:%s in port: %s\n", getTime(), host, arg_port);
		   fflush(log_file);
		   fclose(log_file);
		 }                
		 do {
		       /* Прием строки от клиента */
			   if ((recv_msg = fopen(RecvFile,"a+")) != NULL) { 
			     nbytes = recv_string(cs, recv_msg); }

			   if ((log_file = fopen(NamLog,"a+")) != NULL)
               {
                 fprintf(log_file,"%s Client send to Server:%d\n", getTime(), nbytes);
                 fflush(log_file); 
                 fclose(log_file); 
               }
		     } while(nbytes > 0);

		     if((log_file = fopen(NamLog, "a+")) != NULL)
         {
           fprintf(log_file, "%s Close session on client:%s\n", getTime(), host);
           fflush(log_file);
           fclose(log_file);
         }
         s_close(shandle); 
         exit(0);
	   } else if (pid > 0) s_close(shandle);
    } while(1);
  free(NamLog);
  free(RecvFile);
  return 1;
}

int main( int argc, char *argv[] )
{
  int pid;
  char *arg_port;
  if (argc == 2)
  {
    arg_port = argv[1];
  }
  else
  {
    printf("Usage: %s <port of server> \n",argv[0]);
    return -1;
  }
  pid = fork(); 
  /* Если не удалось запустить потомка */
  if (pid == -1) 
  {
       printf("Error: Start Daemon failed (%s)\n", strerror(errno));
       return -1;
  }
  else if (!pid)
  {
  	 
  	umask(0);
  	setsid();
  	Demon(arg_port);
  	chdir("/");
  	close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }
  return 0;
}
