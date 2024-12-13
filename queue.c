#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "FFR.h"
#include "HASH.h"

void queue_enqueue(NLIST* signali) {

	if (queue_overflow() == 1) {

		Que.que[Que.rear] = signali;
		if (Que.rear != Que.max - 1) {
			Que.rear++;
			Que.num++;
		}
		else {
			Que.rear = 0;
			Que.num++;
		}
	}
	else {
		printf("\nキューオーバーフロー\n");
		//exit(0);
	}

}




NLIST* queue_dequeue(void) {
	NLIST* signalo;
	signalo = Que.que[Que.front];
	Que.que[Que.front] = NULL;
	if (Que.front != Que.max - 1) {
		Que.front++;
		Que.num--;
	}
	else {
		Que.front = 0;
		Que.num--;
	}

	//printf("デキューされた信号線:%s\n", signalo->name);

	return signalo;
}



int queue_empty(void)	//空:0
{
	if (Que.num == 0) {
		return 0;
	}
	return 1;
}



int queue_overflow() {
	if (Que.num >= Que.max) {
		return 0;
	}
	else {
		return 1;
	}
}
