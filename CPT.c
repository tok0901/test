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

//int SAF_CPT(int test_number, int sim_test, int ffr_number) {
//
//	NLIST* sim_net = ffr[ffr_number].fos;
//
//	//対象FFR内-FOS-det=ALL1
//	sim_net->det = ~0ULL;
//	sim_net->det_flag = 1;
//
//	//対象FFR内-tpi_FOSのみの場合は処理終了
//	if ((sim_net->type != IN) && (sim_net->type != FOUT)) {
//		queue_enqueue(sim_net);
//	}
//
//	//FFR内-FOS以外の全信号線のdet算出
//	while (queue_empty() == 1) {
//
//		NLIST* signalo = queue_dequeue();
//		int det,
//			val;
//
//		//全入力信号線-det算出
//		for (int tst_number = test_number % BIT_64; tst_number < sim_test; tst_number++) {
//
//			det = bit_search_N(signalo->det, tst_number);
//			val = bit_search_N(signalo->val, tst_number);
//
//			//detectabilityが有効なtpだけ処理
//			if (det == 1) {
//
//				if (val == 1) {
//					calc_detectability_value1(signalo, tst_number);
//				}
//				else if (val == 0) {
//					calc_detectability_value0(signalo, tst_number);
//				}
//
//			}
//
//		}
//
//		//FFR内の入力信号線をエンキュー
//		//tpi_netはスルー
//		for (int in_number = 0; in_number < signalo->n_in; in_number++) {
//
//			if ((signalo->in[in_number]->type != IN) && (signalo->in[in_number]->type != FOUT) && (signalo->in[in_number]->tpi_flag != 1)) {
//				queue_enqueue(signalo->in[in_number]);
//			}
//
//		}
//
//
//	}
//	
//	return 1;
//
//}

//
int SAF_CPT(int test_number, int sim_test, int ffr_number) {

	//int SAF_CPT(NLIST* sim_net){

	NLIST* sim_net = ffr[ffr_number].fos;


	//対象FFR内-FOS-det=ALL1
	sim_net->det = ~0ULL;
	sim_net->det_flag = 1;

	//FFR内FOSエンキュー
	queue_enqueue(sim_net);
	

	//FFR内-FOS以外の全信号線のdet算出
	while (queue_empty() == 1) {

		NLIST* signalo;
		signalo = queue_dequeue();

		//入力信号線のdet算出
		for (int in_number = 0; in_number < signalo->n_in; in_number++) {

			//FFR外入力信号線はスルー
			if (signalo->in[in_number]->ffr_id == ffr_number) {

				if ((signalo->type == BUF) || (signalo->type == INV)) {
					signalo->in[in_number]->det = signalo->det;
					signalo->in[in_number]->det_flag = 1;
				}


				else {
					ULLI temp = 0ULL;
					temp = in_fault_calc(signalo, in_number);			//in[in_number]に故障伝搬した時のsignaloの故障値を算出

					ULLI det_temp = 0ULL;
					det_temp = temp ^ (signalo->val);							//gate範囲でのin[in_number]->det算出

					signalo->in[in_number]->det = det_temp & (signalo->det);	//in[in_number]->det算出
					signalo->in[in_number]->det_flag = 1;

				}

				//入力信号線エンキュー
				queue_enqueue(signalo->in[in_number]);
				

			}

			/*/tpi_FOS(n_out==1)
			else {
				if (signalo->in[in_number]->n_out == 1) {
					printf("%s", signalo->in[in_number]->name);
				}
			}/////////////////////*/

		}

	}

	return 1;

}
////////////////////////*/


//void calc_detectability_value1(NLIST* sim_net,int tst_number) {
//
//	int ctr_count = 0, 
//		ctr_number;
//
//	switch (sim_net->type) {
//
//	//全入力信号線検出可能
//	case BUF:
//	case INV:
//	case AND:
//	case NOR:
//	case EXOR:
//	case EXNOR:
//		 
//		for (int in_number = 0; in_number < sim_net->n_in; in_number++) {
//
//			sim_net->in[in_number]->det = bit_setting_1(sim_net->in[in_number]->det, tst_number);
//		}
//		break;
//
//	//制御値0を持つ入力信号線1本のみ検出可能
//	case NAND:
//
//		for (int in_number = 0; in_number < sim_net->n_in; in_number++) {
//
//			//制御値
//			if (bit_search_N(sim_net->in[in_number]->val, tst_number) == 0) {
//				ctr_count++;
//				ctr_number = in_number;
//			}
//			if (ctr_count >= 2) {
//				break;
//			}
//		}
//		if (ctr_count == 1) {
//			sim_net->in[ctr_number]->det = bit_setting_1(sim_net->in[ctr_number]->det, tst_number);
//		}
//		break;
//
//	//制御値1を持つ入力信号線1本のみ検出可能
//	case OR:
//
//		for (int in_number = 0; in_number < sim_net->n_in; in_number++) {
//
//			//制御値
//			if (bit_search_N(sim_net->in[in_number]->val, tst_number) == 1) {
//				ctr_count++;
//				ctr_number = in_number;
//			}
//			if (ctr_count >= 2) {
//				break;
//			}
//		}
//		if (ctr_count == 1) {
//			sim_net->in[ctr_number]->det = bit_setting_1(sim_net->in[ctr_number]->det, tst_number);
//		}
//		break;
//
//	}
//
//}

//void calc_detectability_value0(NLIST* sim_net,int tst_number) {
//
//	int ctr_count = 0, 
//		ctr_number;
//
//	switch (sim_net->type) {
//
//	//全入力信号線検出可能
//	case BUF:
//	case INV:
//	case NAND:
//	case OR:
//	case EXOR:
//	case EXNOR:
//
//		for (int in_number = 0; in_number < sim_net->n_in; in_number++) {
//
//			sim_net->in[in_number]->det = bit_setting_1(sim_net->in[in_number]->det, tst_number);
//		}
//		break;
//
//	//制御値0を持つ入力信号線1本のみ検出可能
//	case AND:
//
//		for (int in_number = 0; in_number < sim_net->n_in; in_number++) {
//
//			//制御値
//			if (bit_search_N(sim_net->in[in_number]->val, tst_number) == 0) {
//				ctr_count++;
//				ctr_number = in_number;
//			}
//			if (ctr_count >= 2) {
//				break;
//			}
//		}
//		if (ctr_count == 1) {
//			sim_net->in[ctr_number]->det = bit_setting_1(sim_net->in[ctr_number]->det, tst_number);
//		}
//		break;
//
//	//制御値1を持つ入力信号線1本のみ検出可能
//	case NOR:
//
//		for (int in_number = 0; in_number < sim_net->n_in; in_number++) {
//
//			//制御値
//			if (bit_search_N(sim_net->in[in_number]->val, tst_number) == 1) {
//				ctr_count++;
//				ctr_number = in_number;
//			}
//			if (ctr_count >= 2) {
//				break;
//			}
//		}
//		if (ctr_count == 1) {
//			sim_net->in[ctr_number]->det = bit_setting_1(sim_net->in[ctr_number]->det, tst_number);
//		}
//		break;
//
//	}
//
//
//}

ULLI in_fault_calc(NLIST* sim_net, int fault_in) {

	ULLI temp = 0ULL; ULLI temp0 = 0ULL; ULLI temp1 = ~0ULL;

	switch (sim_net->type) {

	case BUF:
	case FOUT:

		temp = ~sim_net->in[fault_in]->val;
		break;

	case INV:
		temp = sim_net->in[fault_in]->val;
		break;

	case AND:
		for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

			if (in_number == fault_in) {
				temp = ~sim_net->in[in_number]->val & temp1;
				temp1 = temp;
			}
			else {
				temp = sim_net->in[in_number]->val & temp1;
				temp1 = temp;
			}
		}
		break;

	case NAND:
		for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

			if (in_number == fault_in) {
				temp = ~sim_net->in[in_number]->val & temp1;
				temp1 = temp;
			}
			else {
				temp = sim_net->in[in_number]->val & temp1;
				temp1 = temp;
			}
		}
		temp = ~temp;
		break;

	case OR:
		for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

			if (in_number == fault_in) {
				temp = ~sim_net->in[in_number]->val | temp0;
				temp0 = temp;
			}
			else {
				temp = sim_net->in[in_number]->val | temp0;
				temp0 = temp;
			}
		}
		break;

	case NOR:
		for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

			if (in_number == fault_in) {
				temp = ~sim_net->in[in_number]->val | temp0;
				temp0 = temp;
			}
			else {
				temp = sim_net->in[in_number]->val | temp0;
				temp0 = temp;
			}
		}
		temp = ~temp;
		break;

	case EXOR:

		if (fault_in == 0) {
			temp = ~sim_net->in[0]->val ^ sim_net->in[1]->val;
		}
		else if (fault_in == 1) {
			temp = sim_net->in[0]->val ^ ~sim_net->in[1]->val;
		}
		break;

	case EXNOR:
		if (fault_in == 0) {
			temp = ~sim_net->in[0]->val ^ sim_net->in[1]->val;
		}
		else if (fault_in == 1) {
			temp = sim_net->in[0]->val ^ ~sim_net->in[1]->val;
		}
		temp = ~temp;
		break;

	default:
		printf("detectability算出はFFR毎に行います\n");
		exit(0);

	}

	return temp;

}
