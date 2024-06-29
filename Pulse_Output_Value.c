#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "FFR.h"

//出力応答値算出関数
char* Pulse_Output_Value(int tst_number, NLIST** sort_net) {

	int net_number;

	for (net_number = 0; net_number < n_pi; net_number++) {
		
		if (po[net_number]->value_fault_flag[tst_number] == 1) {

			if (po[net_number]->value[tst_number] == 0) {

			}
		}


	}


}
