//GNU AFFERO 3.0 Useop GIm April 10. 2023

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <functional>
#include "BaseSocket.h"
#include "Object.h"


int main(int argv, char** argc)
{
	printf("Hello world!");
	//Test for socket
	// TCP::Client client("127.0.0.1", 8888, 3000);
	// printf("connecting start\n");
	// while (client.connecting()) {}
	// for(int i = 0; i++  < 10;)
	// {
	// 	client.sending("hello server!");
	// 	auto recvmsg = client.recving();
	// 	std::cout << "Recv:" << recvmsg << std::endl;
	// }

	printf("new loop0\n");

	World world("simulator", { 100,100,700,400 });

	world.BaseRenderLoop();	

	return 0;
}