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
int gate_calc_fault(int tst_number,NLIST* sim_net);



int SAF_PPSFP(int test_number, int sim_test, FFR* ffr) {

	int ffr_number, tst_number, cpt_flag;

	for (ffr_number = 0; ffr_number < n_ffr; ffr_number++) {								//外部出力側のFFRからCPT実行

		for (tst_number = 0; tst_number < sim_test; tst_number++) {						//パターン並列(64bit)

			cpt_flag = 0;
			if (ffr[ffr_number].fos->n_out >= 2) {										//対象信号線がFOSである時

				//FoutBlanchのdetectability判定
				for (int out_number = 0; out_number < ffr[ffr_number].fos->n_out; out_number++) {					
					if (ffr[ffr_number].fos->out[out_number]->detec[tst_number] == 1) {			//fosの出力線がdetectability=1であるかどうか(trueならbreak→CPT実行)
						cpt_flag = 1;
						break;
					}
				}
			}

			else {																		//対象信号線が外部出力線である時
				cpt_flag = 2;
			}

			if (cpt_flag = 1) {
				//PPSFP実行


				for (int out_number = 0; out_number < ffr[ffr_number].fos->n_out; out_number++) {

					for (int net_number = 0; net_number < n_net - n_pi; net_number++) {
						sort_net[net_number]->value_fault_flag[tst_number] = 0;					//故障検出フラグ初期化
					}

					if (ffr[ffr_number].fos->out[out_number]->detec[tst_number] == 1) {
						
						if(gate_calc)
					}
				}
			}

			
			if (cpt_flag == 2) {

				//CPT実行
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


//故障値算出関数

int gate_calc_fault(int tst_number,NLIST* sim_net) {

	int flag=0,num,result; int* in_value;
	int in_number, out_number;

	if (sim_net->n_in >= 2) {
		num = sim_net->n_in;
		in_value = (int*)malloc(sizeof(int) * num);
		for (in_number = 0; in_number < num; in_number++) {
			if (sim_net->in[in_number]->value_fault_flag[tst_number] == 1) {

				if (sim_net->in[in_number]->value[tst_number] == 1) {
					in_value[in_number] = 0;
				}
				else {
					in_value[in_number] = 1;
				}

			}
			else if (sim_net->in[in_number]->value_fault_flag[tst_number] == 0) {
				in_value[in_number] = sim_net->in[in_number]->value[tst_number];
			}
		}

	}

	switch (sim_net->type) {

	case IN:
	case BUF:
		sim_net->value_fault_flag[tst_number] = 1;

		for (out_number = 0; out_number < sim_net->n_out; out_number++) {
			if (gate_calc_fault(tst_number, sim_net->out[out_number]) == 1) {
				flag = 1;
			}
		}

		break;

	case INV:
		sim_net->value_fault_flag[tst_number] = 1;

		for (out_number = 0; out_number < sim_net->n_out; out_number++) {
			if (gate_calc_fault(tst_number, sim_net->out[out_number]) == 1) {
				flag = 1;
			}
		}

		break;

	case AND:

		for (in_number = 0; in_number < num; in_number++) {
			if (in_value[in_number] == 0) {
				result = 0;
				break;
			}
			result = 1;
		}

		if (sim_net->value[tst_number] != result) {
			sim_net->value_fault_flag[tst_number] = 1;
			for (out_number = 0; out_number < sim_net->n_out; out_number++) {
				if (gate_calc_fault(tst_number, sim_net->out[out_number]) == 1) {
					flag = 1;
				}
			}
		}

		break;

	case NAND:

		for (in_number = 0; in_number < num; in_number++) {
			if (in_value[in_number] == 0) {
				result = 1;
				break;
			}
			result = 0;
		}

		if (sim_net->value[tst_number] != result) {
			sim_net->value_fault_flag[tst_number] = 1;
			for (out_number = 0; out_number < sim_net->n_out; out_number++) {
				if (gate_calc_fault(tst_number, sim_net->out[out_number]) == 1) {
					flag = 1;
				}
			}
		}

		break;

	case OR:
		for (in_number = 0; in_number < num; in_number++) {
			if (in_value[in_number] == 1) {
				result = 1;
				break;
			}
			result = 0;
		}

		if (sim_net->value[tst_number] != result) {
			sim_net->value_fault_flag[tst_number] = 1;
			for (out_number = 0; out_number < sim_net->n_out; out_number++) {
				if (gate_calc_fault(tst_number, sim_net->out[out_number]) == 1) {
					flag = 1;
				}
			}
		}

		break;

	case NOR:
		for (in_number = 0; in_number < num; in_number++) {
			if (in_value[in_number] == 1) {
				result = 0;
				break;
			}
			result = 1;
		}

		if (sim_net->value[tst_number] != result) {
			sim_net->value_fault_flag[tst_number] = 1;
			for (out_number = 0; out_number < sim_net->n_out; out_number++) {
				if (gate_calc_fault(tst_number, sim_net->out[out_number]) == 1) {
					flag = 1;
				}
			}
		}

		break;

	case EXOR:

		if (in_value[0] != in_value[1]) {
			result = 1;
		}
		else {
			result = 0;
		}

		if (sim_net->value[tst_number] != result) {
			sim_net->value_fault_flag[tst_number] = 1;
			for (out_number = 0; out_number < sim_net->n_out; out_number++) {
				if (gate_calc_fault(tst_number, sim_net->out[out_number]) == 1) {
					flag = 1;
				}
			}
		}

	case EXNOR:

		if (in_value[0] != in_value[1]) {
			result = 0;
		}
		else {
			result = 1;
		}

		if (sim_net->value[tst_number] != result) {
			sim_net->value_fault_flag[tst_number] = 1;
			for (out_number = 0; out_number < sim_net->n_out; out_number++) {
				if (gate_calc_fault(tst_number, sim_net->out[out_number]) == 1) {
					flag = 1;
				}
			}
		}

	case FOUT:
		sim_net->value_fault_flag[tst_number] = 1;

		for (out_number = 0; out_number < sim_net->n_out; out_number++) {
			if (gate_calc_fault(tst_number, sim_net->out[out_number]) == 1) {
				flag = 1;
			}
		}

		break;

	}//switch文終了

	if (flag == 0) {		//故障が伝搬しなかった
		return 0;
	}
	else if (flag == 1) {	//故障が外部出力まで伝搬した
		return 1;
	}

}
