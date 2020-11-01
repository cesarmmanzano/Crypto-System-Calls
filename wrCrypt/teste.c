#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio_ext.h>

#define BUFFER_LENGTH 256

void clearScreen();
void clearMessage(char[]);

int main()
{  
	char message[BUFFER_LENGTH], receive[BUFFER_LENGTH];	
	int ret, fd;
	int option, option2;
	int sizeRet;
	void *buf; 

	do
	{
		clearMessage(message);
		clearMessage(receive);

		do
		{
			clearScreen();
			printf("====================\n");
			printf("1.Armazenar mensagem cifrada no arquivo\n");
			printf("2.Ler arquivo cifrado\n");
			printf("0.Sair\n");
			printf("====================\n");
			printf("Digite a opção desejada: ");
			scanf("%d", &option);
			getchar();

		}while(option < 0 || option > 2);

		clearScreen();

		switch(option)
		{

			case 0: return 0;

			case 1: 
				printf("Digite a mensagem para ser cifrada: ");
				scanf("%[^\n]%*c", message);
				printf("Mensagem Enviada: %s", message);
				getchar();
				buf = message;

				if ((fd = open("/home/cesar/Downloads/test.txt", O_WRONLY | O_TRUNC | O_CREAT)) < 0)
				{
					perror("Failed to open the file...");
					return errno;
				}

				sizeRet = syscall(333, fd, buf, strlen(message));
				
				if ((ret = close(fd)) < 0)
				{
					perror("Erro ao fechar o arquivo");
					return errno;
				}

				break;

			case 2: 

				if ((fd = open("/home/cesar/Downloads/test.txt", O_RDONLY)) < 0)
				{
					perror("Failed to open the file...");
					return errno;
				}

				buf = NULL;
				ret = syscall(334, fd, buf, sizeRet);	
				
				printf("Mensagem lida: %s\n", receive);
				getchar();

				if ((ret = close(fd)) < 0)
				{
					perror("Erro ao fechar o arquivo");
					return errno;
				}

				break;
		}

		clearScreen();
		printf("====================\n");
		printf("1.Continuar\n");
		printf("0.Sair\n");
		printf("====================\n");
		printf("Digite a opção desejada: ");
		scanf("%d", &option2);
		getchar();

	}while(option2 != 0);
	

	/*
	printf("Invoking 'listProcessInfo' system call");
         
	long int ret_status = syscall(333); // 333 is the syscall number
         
	if(ret_status == 0) 
		printf("System call 'listProcessInfo' executed correctly. Use dmesg to check processInfo\n");
    
	else 
		printf("System call 'listProcessInfo' did not execute as expected\n");
	*/

	return 0;
}

/* ================================================== */

void clearMessage(char message[])
{
	for (int i = 0; i < strlen(message); i++)
	{
        	message[i] = '\0';
	}
}

/* ================================================== */

void clearScreen()
{
	printf("\033[H\033[J");
}
