#include "pch.h"
#include <stdio.h>
#include <winsock2.h> // Wincosk2.h должен быть раньше windows!
#include <windows.h>
#include <math.h>
#include <time.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define _CRT_SECURE_NO_WARNINGS
#define MAX_CLIENTS 3
#define SA struct sockaddr
#define MY_PORT 127000
// макрос для печати количества активных пользователей
#define PRINTNUSERS if (nclients) printf("%d useronnline\n",nclients);else printf("No User on line\n");
// Прототип функции, обслуживающий подключившихся пользователей
LPTHREAD_START_ROUTINE WINAPI NewClient(PVOID *argt);
double * recv_array(SOCKET sock, double array[], int array_size);
void send_array(SOCKET sock, double z);

int nclients = 0;
double StartUniformSearchMethod(double * array);
double Func(double x);

struct args_struct {
	int client;
	SOCKET sock;
};

int main(int argc, char* argv[])
{
	struct args_struct * args;
	args = (args_struct*)malloc(sizeof(args));
	char buff[1024]; // Буфер для различных нужд
	struct sockaddr_in local_addr;
	printf("TCP SERVER DEMO\n");
	if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
	{
		printf("Error WSAStartup %d\n", WSAGetLastError());
		return 1;
	}
	SOCKET mysocket;
	// AF_INET ­ сокет Интернета
	// SOCK_STREAM ­ потоковый сокет (с установкой соединения)
	// 0 ­ по умолчанию выбирается TCP протокол
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Error socket %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	// Связывание сокета с локальным адресом
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(MY_PORT);
	local_addr.sin_addr.s_addr = 0; // сервер принимаем подключения

	if (bind(mysocket, (SA *)&local_addr, sizeof(local_addr)))
	{
		printf("Error bind %d\n", WSAGetLastError());
		closesocket(mysocket);
		WSACleanup();
		return 1;
	}
	// Ожидание подключений
	// размер очереди – 0x100
	if (listen(mysocket, 0x100))
	{
		printf("Error listen %d\n", WSAGetLastError());
		closesocket(mysocket);
		WSACleanup();
		return 1;
	}

	printf("Waiting connection…\n");

	SOCKET client_socket;
	struct sockaddr_in client_addr; // адрес клиента (заполняется системой)
	// функции accept необходимо передать размер структуры
	int client_addr_size = sizeof(client_addr);
	int clients_sockets[MAX_CLIENTS];
	DWORD thIDs[MAX_CLIENTS];
	// цикл извлечения запросов на подключение из очереди
	while (client_socket = accept(mysocket, (SA *)&client_addr, &client_addr_size))
	{
		client_socket;
		nclients++;
		args->client = nclients;
		args->sock = client_socket;
		PRINTNUSERS

		QueueUserWorkItem((LPTHREAD_START_ROUTINE)NewClient, (PVOID)args, WT_EXECUTEDEFAULT);
	}
	closesocket(mysocket);
	getchar();
	return 1;
}

LPTHREAD_START_ROUTINE WINAPI NewClient(PVOID* argt)
{
	struct args_struct* args = (struct args_struct*) argt;
	SOCKET sock = args->sock;
	BOOL isStart = TRUE;
	while (isStart) {
		char buf[10];
		int array_size;
		recv(sock, buf, sizeof(array_size), 0);
		if (atoi(buf) == 1)
			isStart = atoi(buf);
		else
			isStart = FALSE;
		recv(sock, buf, sizeof(array_size), 0);
		array_size = atoi(buf);
		double* array = NULL;
		array = (double*)malloc(sizeof(double) * (array_size + 1));
		array = recv_array(sock, array, array_size);
		/**/
		double z = StartUniformSearchMethod(array);
		printf("%lf ", z);
		send_array(sock, z);
	}
	return 0;
}

double StartUniformSearchMethod(double* array)
{
	double a = array[0];
	double b = array[1];
	// Количество участков
	double n = 1000;
	// Шаг
	double dx;
	dx = (b - a) / n;
	double yMin = INT_MAX;
	double xMin = a;
	for (double x = a; x <= b; x += dx)
	{
		double y = Func(x);
		if (y < yMin)
		{
			xMin = x;
			yMin = y;
		}
	}
	return ((xMin + yMin) / 2);
}

double Func(double x)
{
	return sqrt(sin(x) + 2);
}

double* recv_array(SOCKET sock, double* array, int array_size) {
	char tmp[2048];
	recv(sock, tmp, sizeof(tmp), 0);
	for (int i = 0; i < array_size; i++) {
		char buf[256];
		recv(sock, buf, 256, 0);
		//printf("%s\n",buf);
		array[i] = strtod(buf, NULL);
		//printf("/%lf/\n ",array[i]);
	}
	return array;
}

void send_array(SOCKET sock, double z) {
	char buf[257];
	sprintf(buf, "%lf", z);
	send(sock, buf, sizeof(buf) - 1, 0);
	fflush(stdout);
}
