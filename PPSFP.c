#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "module.h"


int SAF_PPSFP(int test_number,int sim_test) {

	int ffr_number;
		
	//CPT実行
	for (ffr_number = n_ffr; ffr_number >= 0; ffr_number--) {						//外部入力側のFFRからCPT実行

		if (ffr[ffr_number].fos->value == 1) {										//対象信号線の正常値が1の時

			if (SAF_CPT1(ffr[ffr_number].fos, test_number,sim_test) != 1) {			//SAF_CPT1呼出し
				return 0;
			}

		}

		else if (ffr[ffr_number].fos->value == 0) {									//対象信号線の正常値が0の時

			if (SAF_CPT0(ffr[ffr_number].fos, test_number, sim_test) != 1) {		//SAF_CPT0呼出し
				return 0;
			}

		}

		printf("FFR%dのCPT完了\n\n", ffr[ffr_number].fos->ffr_id);

	}

	printf("%d番目から%d番目のテストパターンによるPPSFP完了!\n\n", test_number, test_number + sim_test);

	return 1;
}
