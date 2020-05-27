/* Severenchuk Aleksandr, sasha.severen@gmail.com, 26.05.2020 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_SIZE 100000

int sock_err( const char* function, int s )
{
  int err;
  err = errno;
  fprintf(stderr, "%s: socket error: %d\n", function, err);
  return -1;
}

/* 
 * Функция возвращает все адреса указанного хоста
 * в виде динамического однонаправленного списка.
 */
unsigned int get_host_ipn( const char* name )
{
  
  struct addrinfo* addr = 0;
  unsigned int ip4addr = 0;
  if (getaddrinfo(name, 0, 0, &addr) == 0)
  {
    struct addrinfo * cur = addr;
    while (cur)
    {
      /* Интересует только IPv4 адрес, если их несколько - то первый */
      if (cur -> ai_family == AF_INET)
      {
        ip4addr = ((struct sockaddr_in *) cur -> ai_addr) -> sin_addr.s_addr;
        break;
      }
      cur = cur->ai_next;
    }
    freeaddrinfo(addr);
  }  
  return ip4addr;
}

/* 
 * Функция отправляет сообщение со строкой request
 * по переданному значению сокета soc.
 */
int send_request( int soc, char* request )
{
  int flags = 0, sent = 0, res = 0;
  int size = strlen(request);
  while (sent < size)
  {	 	
    res = send(soc, request + sent, size - sent, flags);
	if (res < 0) { return sock_err("send", soc); }
    sent += res;
 	printf("%d bytes sent.\n", sent);
  } 
  return 0; 
}

/* 
 * Функция для перевода текста из файла 
 * в строку buf. Возвращает указатель на строку.
 */
char* read_from_txt( FILE* file_to_send )
{
  char *buf = NULL;
  int buf_size = 0;
  int count = 0;
  char c;
 
  while ((c = fgetc(file_to_send)) != EOF)
  {
    if (buf_size == count)
    { 	
      buf_size += 50;
      buf = (char *) realloc(buf, (buf_size + 1) * sizeof(char));
    }
    *(buf + count) = c;
    count++;
  }
  fclose(file_to_send);
  buf[count] = '\0';
  return buf;
}

int main(int argc, char *argv[])
{
  FILE *file_to_send;
  char *text = NULL;
  char filepath[100];
  int shandle;
  int cnct_res, cnct_N = 0, n = 0;
  int command = 0;
  int flags = 0;
  struct sockaddr_in addr;
  char *arg_ip_port;

  if (argc == 2)
  {
    arg_ip_port = argv[1];
  }
  else
  {
    printf("Usage: %s <ip:port of server> \n",argv[0]);
    return -1;
  }
  char *str_ip = strtok(arg_ip_port, ":");
  char *str_port = strtok(NULL, ":");
    
  if (!str_port)
  {
    printf("Usage: %s <ip:port of server> \n",argv[0]);
    return -1;
  }
  shandle = socket(AF_INET, SOCK_STREAM, 0);
  if (shandle < 0) { return sock_err("socket", shandle); }
  
  /* Заполнение структуры с адресом удаленного узла */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(str_port));
  addr.sin_addr.s_addr = get_host_ipn(str_ip);
    
  /* В случае неудачи, пытаемся подключится к серверу 10 раз через каждые 100 мс. */
  printf("Connecting to: %s:%s\n", str_ip, str_port);
  while ((cnct_res = connect(shandle, (struct sockaddr *) &addr, 
  	      sizeof(addr))) != 0 && ++cnct_N <= 10)
  {
    printf("Failed to connect. Wait 100 ms and try again. Attempt %d.\n", cnct_N);
    usleep(100 * 1000);
  }
  /* Если подключится не удалось. */
  if (cnct_res != 0)
  {
    close(shandle);
    return sock_err("connect", shandle);
  } 
  else printf("Connected.\n");
    
  /* Основной цикл с выбором файла для отправки сообщения на сервер */
  while (1) 
  {
    printf("\nOptions:\n"
    	    "0-stop\n"
    	    "1-send text from file\n");
    if (scanf("%d", &command) != 1) { 
      scanf("%*s"); 
      continue; 
    }
    switch (command)
    {
    	case 0:
    	  close(shandle);
    	  return 1;
    	case 1:
	  printf("Input path_to_file: ");
	  scanf("%s", filepath);
	  text = (char*) malloc(MAX_SIZE * sizeof(char));
	  if ((file_to_send = fopen(filepath, "rb")) != 0) { 
	    text = read_from_txt(file_to_send); } 
	  else { 
	    printf("Can't open file %s\n", filepath); 
	    continue; }
	  strncat(text, "\0", 1);
    	  printf("Sending message to server...\n");
    	  send_request(shandle, text);
          free(text);
        default:
          continue;
    }
  }   
  close(shandle);
  return 0;
}
