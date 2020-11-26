#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio_ext.h>
#include <stdbool.h>

#define BUFFER_LENGTH 256

int writeCrypt(char[]);
int readCrypt(char[]);
int readFile(char[]);

void clearScreen();
void clearMessage(char[]);
void printHexDump(const void *, int);

int ret, fd;
char *path = "./test.txt";
bool wasWritten = false;

int main()
{  
	char message[BUFFER_LENGTH], receive[BUFFER_LENGTH];		
	int option, option2;

	do
	{
		clearMessage(message);
		clearMessage(receive);

		do
		{
			clearScreen();
			printf("====================\n");
			printf("1.Cifrar mensagem para armazenar no arquivo\n");
			printf("2.Decifrar mensagem do arquivo\n");
			printf("3.Ler mensagem cifrada do arquivo\n");
			printf("0.Sair\n");
			printf("====================\n");
			printf("Digite a opção desejada: ");
			scanf("%d", &option);
			getchar();

		}while(option < 0 || option > 3);

		clearScreen();

		switch(option)
		{

			case 0: return 0;

			case 1: 
				wasWritten = true;
				writeCrypt(message);

				break;

			case 2: 
				if(wasWritten)
					readCrypt(receive);
				else{
					printf("Voce precisa escrever algo no arquivo antes de ler");
					getchar();
				}

				break;

			case 3:
				if(wasWritten)
					readFile(receive);
				else{
					printf("Voce precisa escrever algo no arquivo antes de ler");
					getchar();
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

	return 0;
}

/* ================================================== */

int writeCrypt(char message[])
{
	void *buf = NULL;

	if ((fd = open(path, O_WRONLY | O_CREAT | O_TRUNC)) < 0)
	{
		perror("Failed to open the file...");
		return errno;
	}

	printf("Digite a mensagem para ser cifrada: ");
	scanf("%[^\n]%*c", message);
	printf("Mensagem Enviada: %s", message);								
	buf = message;
	getchar();				

	syscall(333, fd, buf, strlen(message));

	if ((ret = close(fd)) < 0)
	{
		perror("Erro ao fechar o arquivo");
		return errno;
	}
}

/* ================================================== */

int readCrypt(char receive[])
{
	char buf[BUFFER_LENGTH];
	
	if ((fd = open(path, O_RDONLY)) < 0)
	{
		perror("Failed to open the file...");
		return errno;
	}

	ret = syscall(334, fd, &buf, BUFFER_LENGTH);

	printf("Mensagem lida: %s\n", buf);
	printf("RET: %d\n", ret);
	printHexDump(receive, strlen(receive));
	getchar();
				
	if ((ret = close(fd)) < 0)
	{
		perror("Erro ao fechar o arquivo");
		return errno;
	}
}

/* ================================================== */

int readFile(char receive[])
{
	if ((fd = open(path, O_RDONLY)) < 0)
	{
		perror("Failed to open the file...");
		return errno;
	}
							
	if ((fd = read(fd, receive, BUFFER_LENGTH)) < 0)
	{
		perror("Failed to open the file...");
		return errno;
	}

	printf("Mensagem cifrada no arquivo: ");
	printHexDump(receive, strlen(receive));
	getchar();

	close(fd);
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

/* ================================================== */

void printHexDump(const void *message, int length)
{
    char ascii[50];
    int i;
    ascii[50] = '\0';

    for (i = 0; i < length; ++i)
    {
        printf("%02X", ((unsigned char *)message)[i]);
        if (((unsigned char *)message)[i] >= ' ' && ((unsigned char *)message)[i] <= '~')
        {
            ascii[i % 16] = ((unsigned char *)message)[i];
        }
        else
        {
            ascii[i % 16] = '\0';
        }
        if ((i + 1) % 8 == 0 || i + 1 == length)
        {
            if ((i + 1) % 16 == 0)
            {
		printf("%s", ascii);
            }
            else if (i + 1 == length)
            {
                ascii[(i + 1) % 16] = '\0';
                printf("%s", ascii);
            }
        }
    }
}
