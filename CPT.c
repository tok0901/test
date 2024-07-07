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

	int n_ctr_value = 0, cpt_number, test_dic = test_number + tst_number, hash_number = dic[test_dic].insert_number, net_dic = dic[test_dic].n_unconf_fault[hash_number];

	//n_ctr_value:制御値の数,
	//cpt_number:CPTを適用する入力信号線id
	//test_dic:故障辞書配列の要素番号(テストパターン番号)
	//hash_number:故障挿入箇所(対象ハッシュ表の要素番号)
	//net_dic:検出故障の要素番号

	int n_fault;
	NLIST** fault_temp;
	int* saf_temp;

	sim_net->detec[tst_number] = 1;			//検出可能な故障としてdetectabilityへフラグ立て

	n_fault = dic[test_dic].n_unconf_fault[hash_number];

	dic[test_dic].n_unconf_fault[hash_number]++;

	dic[test_dic].unconf_fault[hash_number][n_fault] = sim_net;	//検出故障信号線保存

	dic[test_dic].unconf_saf_flag[hash_number][n_fault] = 0;	//検出故障値保存

	sim_net->sim_fault0_flag = 1;			//0縮退故障検出フラグ立て

	printf("s-a-0\t%s\n", dic[test_dic].unconf_fault[hash_number][n_fault]->name);

	switch (sim_net->type) {

		case BUF:	//正常値1の入力線にCPT

			if (SAF_CPT1(test_number, tst_number, sim_net->in[0]) != 1) {
				return 0;
			}
		break;

		case INV:	//正常値0の入力線にCPT

			if (SAF_CPT0(test_number, tst_number, sim_net->in[0]) != 1) {
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

	int n_ctr_value = 0, cpt_number, test_dic = test_number + tst_number, hash_number = dic[test_dic].insert_number, net_dic = dic[test_dic].n_unconf_fault[hash_number];

	//n_ctr_value:制御値の数,
	//cpt_number:CPTを適用する入力信号線id
	//test_dic:故障辞書配列の要素番号(テストパターン番号)
	//hash_number:故障挿入箇所(対象ハッシュ表の要素番号)
	//net_dic:検出故障の要素番号

	int n_fault;

	sim_net->detec[tst_number] = 1;			//検出可能な故障としてdetectabilityへフラグ立て

	n_fault = dic[test_dic].n_unconf_fault[hash_number];

	dic[test_dic].n_unconf_fault[hash_number]++;

	dic[test_dic].unconf_fault[hash_number][n_fault] = sim_net;	//検出故障信号線保存
		
	dic[test_dic].unconf_saf_flag[hash_number][n_fault] = 1;	//検出故障値保存

	sim_net->sim_fault1_flag = 1;			//1縮退故障検出フラグ立て

	printf("s-a-1\t%s\n", dic[test_dic].unconf_fault[hash_number][n_fault]->name);

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
