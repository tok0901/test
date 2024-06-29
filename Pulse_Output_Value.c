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
	char* po_temp;
	po_temp = (char*)malloc(sizeof(char) * n_po);

	for (net_number = 0; net_number < n_po; net_number++) {
		
		//外部出力線に故障が伝搬する時(故障値の出力応答値を格納)
		if (po[net_number]->value_fault_flag[tst_number] == 1) {

			if (po[net_number]->value[tst_number] == 0) {
				po_temp[net_number] = "1";
			}
			else if (po[net_number]->value[tst_number] == 1) {
				po_temp[net_number] = "0";
			}
		}

		//外部出力線に故障が伝搬しない時(正常値の出力応答値を格納)
		else if (po[net_number]->value_fault_flag[tst_number] == 0) {

			if (po[net_number]->value[tst_number] == 0) {
				po_temp[net_number] = "0";
			}
			else if (po[net_number]->value[tst_number] == 1) {
				po_temp[net_number] = "1";
			}
		}


	}

	return po_temp;

}
