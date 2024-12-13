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

void printBinary(ULLI n,int sim_test) {
	unsigned long long mask = 1ULL << (sizeof(n) * 8 - 1);

	// 負の整数の場合は補数を使う
	if (n < 0) {
		n = ~n; // 補数
	}

	for (int i = 0; i < sim_test; i++) {
		if (n & mask) {
			putchar('1');
			
		}
		else  {
			putchar('0');
		}
		mask >>= 1ULL;
	}

}
