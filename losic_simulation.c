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

#define _CRTSECURE_NO_WARNINGS
#pragma warning(disable:4996)

//test_number:参照中のテストパターン番号
//tst_number:論理値格納配列の要素番号

int losic_simulation(int test_number, int sim_test) {

	int tst_number = test_number % BIT_64;

	//外部入力信号線の論理値初期化
	for (int net_number = 0; net_number < n_pi; net_number++) {
		pi[net_number]->val = 0ULL;
	}

	//外部入力線にテストパターン印加
	for (int j = 0; j < n_pi; j++) {								//j:テストパターンファイル列番号
		for (int m = 0; m < n_pi; m++) {							//m:外部入力ネットリスト要素番号
			
			if (strcmp(Str_pin[j], pi[m]->name) == 0) {

				for (int i = 0; i < sim_test; i++) {				//i:テストパターンファイル行番号
					
					if (Str_test[test_number + i][j] == '1') {		//論理値1の場合
						pi[m]->val = bit_setting_1(pi[m]->val, i); 	//対象ビット列立上げ
					}
																	//論理値0の場合(初期値0なので省略)
				}
				break;
			}
		}
	}

	/*/確認用
	for (int net_number = 0; net_number < n_pi; net_number++) {
		printf("\n%s\t", pi[net_number]->name);
		printBinary(pi[net_number]->val,sim_test);
	}/////////////////////////*/

	//全信号線のフラグ初期化
	for (int net_number=0; net_number < n_net; net_number++) {
		sort_net[net_number]->det = 0ULL;									//detectability初期化
		sort_net[net_number]->det_flag = 0;
	}

	//論理シミュレーション
	for (int net_number = n_pi; net_number < n_net; net_number++) {
		value_calc(net_number);
		//printf("\n%20s type:%d\t", sort_net[net_number]->name,sort_net[net_number]->type);
		//printBinary(sort_net[net_number]->val, sim_test);

	}


	//printf("\n%d番目から%d番目のテストパターンによる論理シミュレーション完了\n\n", test_number, test_number + sim_test);

	return 1;
}


//正常値算出関数
void value_calc(int net_number) {

	ULLI temp = 0ULL; ULLI temp0 = 0ULL; ULLI temp1 = ~0ULL;


		switch (sort_net[net_number]->type) {

		case BUF:
			sort_net[net_number]->val = sort_net[net_number]->in[0]->val;
			break;

		case INV:
			sort_net[net_number]->val = ~sort_net[net_number]->in[0]->val;
			break;

		case AND:
			for (int in_number = 0; in_number < sort_net[net_number]->n_in; in_number++) {
				temp = sort_net[net_number]->in[in_number]->val & temp1;
				temp1 = temp;
			}
			sort_net[net_number]->val = temp;
			break;

		case NAND:
			for (int in_number = 0; in_number < sort_net[net_number]->n_in; in_number++) {
				temp = sort_net[net_number]->in[in_number]->val & temp1;
				temp1 = temp;
			}
			sort_net[net_number]->val = ~temp;
			break;

		case OR:
			for (int in_number = 0; in_number < sort_net[net_number]->n_in; in_number++) {
				temp = sort_net[net_number]->in[in_number]->val | temp0;
				temp0 = temp;
			}
			sort_net[net_number]->val = temp;
			break;

		case NOR:
			for (int in_number = 0; in_number < sort_net[net_number]->n_in; in_number++) {
				temp = sort_net[net_number]->in[in_number]->val | temp0;
				temp0 = temp;
			}
			sort_net[net_number]->val = ~temp;
			break;

		case EXOR:
			sort_net[net_number]->val = sort_net[net_number]->in[0]->val ^ sort_net[net_number]->in[1]->val;
			break;

		case EXNOR:
			temp = sort_net[net_number]->in[0]->val ^ sort_net[net_number]->in[1]->val;
			sort_net[net_number]->val = ~temp;
			break;

		case FOUT:
			sort_net[net_number]->val = sort_net[net_number]->in[0]->val;
			break;

		}


}



//故障値算出関数
void fault_value_calc(int net_number) {

	ULLI temp = 0ULL; ULLI temp0 = 0ULL; ULLI temp1 = ~0ULL;


	switch (sort_net[net_number]->type) {

	case BUF:
		sort_net[net_number]->fval = sort_net[net_number]->in[0]->fval;
		break;

	case INV:
		sort_net[net_number]->fval = ~sort_net[net_number]->in[0]->fval;
		break;

	case AND:
		for (int in_number = 0; in_number < sort_net[net_number]->n_in; in_number++) {
			temp = sort_net[net_number]->in[in_number]->fval & temp1;
			temp1 = temp;
		}
		sort_net[net_number]->fval = temp;
		break;

	case NAND:
		for (int in_number = 0; in_number < sort_net[net_number]->n_in; in_number++) {
			temp = sort_net[net_number]->in[in_number]->fval & temp1;
			temp1 = temp;
		}
		sort_net[net_number]->fval = ~temp;
		break;

	case OR:
		for (int in_number = 0; in_number < sort_net[net_number]->n_in; in_number++) {
			temp = sort_net[net_number]->in[in_number]->fval | temp0;
			temp0 = temp;
		}
		sort_net[net_number]->fval = temp;
		break;

	case NOR:
		for (int in_number = 0; in_number < sort_net[net_number]->n_in; in_number++) {
			temp = sort_net[net_number]->in[in_number]->fval | temp0;
			temp0 = temp;
		}
		sort_net[net_number]->fval = ~temp;
		break;

	case EXOR:
		sort_net[net_number]->fval = sort_net[net_number]->in[0]->fval ^ sort_net[net_number]->in[1]->fval;
		break;

	case EXNOR:
		temp = sort_net[net_number]->in[0]->fval ^ sort_net[net_number]->in[1]->fval;
		sort_net[net_number]->fval = ~temp;
		break;

	case FOUT:
		sort_net[net_number]->fval = sort_net[net_number]->in[0]->fval;
		break;

	}


}
