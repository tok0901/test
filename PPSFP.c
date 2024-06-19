#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "FFR.h"

//プロトタイプ宣言
//クリティカル経路導出関数
int SAF_CPT1(int test_number, int tst_number, NLIST* sim_net);
int SAF_CPT0(int test_number, int tst_number, NLIST* sim_net);



int SAF_PPSFP(int test_number,int sim_test, FFR* ffr) {

	int ffr_number, tst_number, cpt_flag;
		
	//CPT実行
	for (ffr_number = 0; ffr_number <n_ffr; ffr_number++) {								//外部出力側のFFRからCPT実行

		for (tst_number = 0; tst_number < sim_test; tst_number++) {						//パターン並列(64bit)

			cpt_flag = 0;
			if (ffr[ffr_number].fos->n_out >= 2) {										//対象信号線がFOSである時

				for (int i = 0; i < ffr[ffr_number].fos->n_out; i++) {					//FoutBlanchのdetectability判定
					if (ffr[ffr_number].fos->out[i]->detec[tst_number] == 1) {						//fosの出力線がdetectability=1であるかどうか(trueならbreak→CPT実行)
						cpt_flag = 1;
						break;
					}
				}
			}

			else {																		//対象信号線が外部出力線である時
				cpt_flag = 1;
			}

			//CPT実行
			if (cpt_flag == 1) {

				if (ffr[ffr_number].fos->value[tst_number] == 1) {						//対象信号線の正常値が1の時

					if (SAF_CPT1(test_number, tst_number, ffr[ffr_number].fos) != 1) {	//SAF_CPT1呼出し
						return 0;
					}

				}

				else if (ffr[ffr_number].fos->value[tst_number] == 0) {					//対象信号線の正常値が0の時

					if (SAF_CPT0(test_number, tst_number, ffr[ffr_number].fos) != 1) {	//SAF_CPT0呼出し
						return 0;
					}

				}

			}

		}

		printf("FFR%dのCPT完了\n\n", ffr[ffr_number].fos->ffr_id);

	}

	printf("\nPPSFP完了!\n\n");

	return 1;
}
