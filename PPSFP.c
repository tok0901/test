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
int gate_calc_fault(int tst_number, NLIST* sim_net);



int SAF_PPSFP(int test_number, int sim_test, FFR* ffr) {

	int ffr_number, tst_number, cpt_flag;													//cpt_flag(0:実行無し,1:PPSFP実行,2:CPT実行)
	char* po_temp;
	NLIST* signalo;

	Que.que = (NLIST**)malloc(sizeof(NLIST*) * n_net * 1000);
	Que.max = n_net * 1000;
	Que.num = 0;
	Que.front = 0;
	Que.rear = 0;

	for (ffr_number = 0; ffr_number < n_ffr; ffr_number++) {								//外部出力側のFFRからCPT実行

		for (tst_number = 0; tst_number < sim_test; tst_number++) {							//パターン並列(64bit)

			int test_dic = test_number + tst_number;

			cpt_flag = 0;																	//cpt_flag初期化

			for (int net_number = 0; net_number < n_po; net_number++) {
				po[net_number]->value_fault_flag[tst_number] = 0;						//外部出力線の故障検出フラグ初期化
			}

			if (ffr[ffr_number].fos->n_out >= 2) {											//対象信号線がFoutStemである時

				//FoutBranchのdetectability判定
				for (int out_number = 0; out_number < ffr[ffr_number].fos->n_out; out_number++) {
					if (ffr[ffr_number].fos->out[out_number]->detec[tst_number] == 1) {		//FoutBranchが故障伝搬経路であるかどうか(trueならエンキューしてPPSFP実行)
						cpt_flag = 1;
						queue_enqueue(ffr[ffr_number].fos->out[out_number]);
					}
				}
			}

			else {																			//対象FoutStemが外部出力線である時

				ffr[ffr_number].fos->value_fault_flag[tst_number] = 1;						//対象FoutStem(外部出力線)の故障検出フラグを有効にする

				cpt_flag = 2;
			}

			if (cpt_flag == 1) {

				//PPSFP実行
				while (queue_empty() == 1) {

					signalo = queue_dequeue();

					if (gate_calc_fault(tst_number, signalo) == 1) {
						cpt_flag = 2;
					}

				}
				
			}

			if (cpt_flag == 2) {

				printf("\n****************tp%d********************\n\n", test_number + tst_number);

				printf("***PPSFP完了***\n");
				po_temp = Pulse_Output_Value(tst_number, sort_net);						
				hash_insert(test_number, tst_number, po_temp);							//ハッシュ表挿入

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

		printf("****************FFR%dのCPT完了**************************\n\n", ffr[ffr_number].fos->ffr_id);

	}

	return 1;
}


//故障値算出関数

int gate_calc_fault(int tst_number, NLIST* sim_net) {

	int flag = 0, num, result; int* in_value;
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

				sim_net->in[in_number]->value_fault_flag[tst_number] = 0;			//検出故障フラグ初期化
			}

		}

	switch (sim_net->type) {

	case IN:
	case BUF:
	case FOUT:
		sim_net->value_fault_flag[tst_number] = 1;

		if (sim_net->n_in >= 1) {
			sim_net->in[0]->value_fault_flag[tst_number] = 0;
		}
		
		if (sim_net->n_out == 0) {	//外部出力線である時
			flag = 1;
		}
		else {		//出力線が存在する時
			flag = 2;
		}

		break;

	case INV:
		sim_net->value_fault_flag[tst_number] = 1;

		sim_net->in[0]->value_fault_flag[tst_number] = 0;

		if (sim_net->n_out == 0) {
			flag = 1;
		}
		else {
			flag = 2;
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

			if (sim_net->n_out == 0) {
				flag = 1;
			}
			else {
				flag = 2;
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
			if (sim_net->n_out == 0) {
				flag = 1;
			}
			else {
				flag = 2;
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

			if (sim_net->n_out == 0) {
				flag = 1;
			}
			else {
				flag = 2;
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
			if (sim_net->n_out == 0) {
				flag = 1;
			}
			else {
				flag = 2;
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
			if (sim_net->n_out == 0) {
				flag = 1;
			}
			else {
				flag = 2;
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
			if (sim_net->n_out == 0) {
				flag = 1;
			}
			else {
				flag = 2;
			}
		}


	}//switch文終了

	if (flag == 0) {		//故障が外部出力まで伝搬しない
		return flag;
	}

	else if (flag == 2) {

		if (sim_net->n_out >= 2) {

			sim_net->value_fault_flag[tst_number] = 0;	//故障値算出関数の再起処理が終了するため対象信号線の故障伝搬フラグを初期化

			for (out_number = 0; out_number < sim_net->n_out; out_number++) {
				if (sim_net->out[out_number]->detec[tst_number] == 1) {		//FoutBranchが故障伝搬経路であるかどうか
					queue_enqueue(sim_net->out[out_number]);
				}
			}
		}

		else if (sim_net->n_out == 1) {
			if (gate_calc_fault(tst_number, sim_net->out[0]) == 1) {
				flag = 1;
			}
		}
	}

	return flag;	//flag=1:外部出力まで故障が伝搬した,2:FoutStemまで故障が伝搬した,0:外部出力まで故障が伝搬しなかった

}
