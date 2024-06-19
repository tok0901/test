#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "FFR.h"


int SAF_CPT1(int test_number,int tst_number,NLIST* sim_net) {

	int n_ctr_value = 0, cpt_number, test_dic = test_number + tst_number, net_dic = dic[test_dic].n_fault;

	//n_ctr_value:制御値の数,  cpt_number:CPTを適用する入力信号線id,  test_dic:故障辞書配列の要素番号(テストパターン番号),  net_dic:検出故障の要素番号

	sim_net->detec[tst_number] = 1;			//検出可能な故障としてdetectabilityへフラグ立て

	dic[test_dic].fault[net_dic] = sim_net;	//検出故障信号線保存

	dic[test_dic].saf_flag[net_dic] = 0;				//検出故障値保存

	dic[test_dic].n_fault++;				//検出故障数インクリメント

	switch (sim_net->type) {

		case BUF:	//正常値1の入力線にCPT

			if (SAF_CPT1(test_number, tst_number, sim_net->in[0]) != 1) {
				return 0;
			}
		break;

		case INV:	//正常値0の入力線にCPT

			if (SAF_CPT1(test_number, tst_number, sim_net->in[0]) != 1) {
				return 0;
			}
		break;
		
		case AND:	//全ての非制御値1の入力線にCPT

			for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

				if (SAF_CPT1(test_number, tst_number, sim_net->in[in_number]) != 1) {
					return 0;
				}
			}
		break;
		
		case NAND:	//1本のみの制御値0の入力線にCPT

			for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

				if (sim_net->in[in_number]->value[tst_number] == 0) {
					n_ctr_value++;
					cpt_number = in_number;

					if (n_ctr_value >= 2) {
						break;
					}
				}

			}

			if (n_ctr_value == 1) {

				if (SAF_CPT0(test_number, tst_number, sim_net->in[cpt_number]) != 1) {
					return 0;
				}
			}
		break;

		case OR:	//1本のみの制御値1の入力線にCPT

			for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

				if (sim_net->in[in_number]->value[tst_number] == 1) {
					n_ctr_value++;
					cpt_number = in_number;

					if (n_ctr_value >= 2) {
						break;
					}
				}

			}

			if (n_ctr_value == 1) {

				if (SAF_CPT1(test_number, tst_number, sim_net->in[cpt_number]) != 1) {
					return 0;
				}
			}
		break;

		case NOR:	//全ての非制御値0の入力線CPT

			for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

				if (SAF_CPT0(test_number, tst_number, sim_net->in[in_number]) != 1) {
					return 0;
				}
			}
		break;
		
		case EXOR:	//全ての入力線にCPT

			for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

				if (sim_net->in[in_number]->value[tst_number] == 0) {

					if (SAF_CPT0(test_number, tst_number, sim_net->in[in_number]) != 1) {
						return 0;
					}
				}

				else if (sim_net->in[in_number]->value[tst_number] == 1) {

					if (SAF_CPT1(test_number, tst_number, sim_net->in[in_number]) != 1) {
						return 0;
					}
				}
			}
		break;

		case EXNOR:	//全ての入力線にCPT

			for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

				if (sim_net->in[in_number]->value[tst_number] == 0) {

					if (SAF_CPT0(test_number, tst_number, sim_net->in[in_number]) != 1) {
						return 0;
					}
				}

				else if (sim_net->in[in_number]->value[tst_number] == 1) {

					if (SAF_CPT1(test_number, tst_number, sim_net->in[in_number]) != 1) {
						return 0;
					}
				}
			}
		break;

	}

	return 1;

}



int SAF_CPT0(int test_number, int tst_number, NLIST* sim_net) {

	int n_ctr_value = 0, cpt_number, test_dic = test_number + tst_number, net_dic = dic[test_dic].n_fault;

	//n_ctr_value:制御値の数,  cpt_number:CPTを適用する入力信号線id,  test_dic:故障辞書配列の要素番号(テストパターン番号),  net_dic:検出故障の要素番号

	sim_net->detec[tst_number] = 1;			//検出可能な故障としてdetectabilityへフラグ立て

	dic[test_dic].fault[net_dic] = sim_net;	//検出故障信号線保存

	dic[test_dic].saf_flag[net_dic] = 1;				//検出故障値保存

	dic[test_dic].n_fault++;				//検出故障数インクリメント

	switch (sim_net->type) {

		case BUF:	//正常値0の入力線にCPT

			if (SAF_CPT0(test_number, tst_number, sim_net->in[0]) != 1) {
				return 0;
			}
		break;

		case INV:	//正常値1の入力線にCPT

			if (SAF_CPT1(test_number, tst_number, sim_net->in[0]) != 1) {
				return 0;
			}
		break;

		case AND:	//1本のみの制御値0の入力線にCPT

			for (int in_number=0; in_number < sim_net->n_in; in_number++) {

				if (sim_net->in[in_number]->value[tst_number] == 0) {
					n_ctr_value++;
					cpt_number = in_number;

					if (n_ctr_value >= 2) {
						break;
					}
				}

			}

			if (n_ctr_value == 1) {

				if (SAF_CPT0(test_number, tst_number, sim_net->in[cpt_number]) != 1) {
					return 0;
				}
			}
		break;

		case NAND:	//全ての非制御値1の入力線にCPT

			for (int in_number=0; in_number < sim_net->n_in; in_number++) {

				if (SAF_CPT1(test_number, tst_number, sim_net->in[in_number]) != 1) {
					return 0;
				}
			}
		break;

		case OR:	//全ての非制御値0の入力線にCPT
			for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

				if (SAF_CPT0(test_number, tst_number, sim_net->in[in_number]) != 1) {
					return 0;
				}
			}
			break;

		case NOR:	//1本のみの制御値1の入力戦にCPT
			for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

				if (sim_net->in[in_number]->value[tst_number] == 1) {
					n_ctr_value++;
					cpt_number = in_number;

					if (n_ctr_value >= 2) {
						break;
					}
				}

			}

			if (n_ctr_value == 1) {

				if (SAF_CPT1(test_number, tst_number, sim_net->in[cpt_number]) != 1) {
					return 0;
				}
			}
		break;

		case EXOR:	//全ての入力線にCPT

			for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

				if (sim_net->in[in_number]->value[tst_number] == 0) {

					if (SAF_CPT0(test_number, tst_number, sim_net->in[in_number]) != 1) {
						return 0;
					}
				}

				else if (sim_net->in[in_number]->value[tst_number] == 1) {

					if (SAF_CPT1(test_number, tst_number, sim_net->in[in_number]) != 1) {
						return 0;
					}
				}
			}
		break;	

		case EXNOR:	//全ての入力線にCPT			

			for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

				if (sim_net->in[in_number]->value[tst_number] == 0) {

					if (SAF_CPT0(test_number, tst_number, sim_net->in[in_number]) != 1) {
						return 0;
					}
				}

				else if (sim_net->in[in_number]->value[tst_number] == 1) {

					if (SAF_CPT1(test_number, tst_number, sim_net->in[in_number]) != 1) {
						return 0;
					}
				}
			}
		break;

	}

	return 1;

}
