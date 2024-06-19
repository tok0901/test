#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "FFR.h"

#define _CRTSECURE_NO_WARNINGS
#pragma warning(disable:4996)

//プロトタイプ宣言
//正常値算出関数
void gate_calc(int tst_number, int net_number, NLIST** sort_net);

//test_number:参照中のテストパターン番号
//tst_number:論理値格納配列の要素番号

int losic_simulation(int test_number, int sim_test, NLIST** sort_net) {

	int i, x, j, m, atoi_value;

	//外部入力線へテストパターン印加
	for (i = 0; i < sim_test; i++) {//テストパターン数分繰り返す

		for (x = 0; x < n_pi; x++) {//格納論理値配列に論理値を格納する

			if (Str_test[test_number+i][x] == '1') {
				test_tbl[x] = "1";
			}
			else if (Str_test[test_number+i][x] == '0') {
				test_tbl[x] = "0";
			}

		}

		for (j = 0; j < n_pi; j++) {//外部入力信号線の数分繰り返す

			for (m = 0; m < n_pi; m++) {//外部入力信号線のピン情報に該当するものを探す
				if (strcmp(Str_pin[j], pi[m]->name) == 0) {
					atoi_value = atoi(test_tbl[j]);//テストパターン文字列を整数型に変換
					pi[m]->value[i] = atoi_value;//該当する名前のピンにテストパターンの値を格納
					printf("%s  %d\n", pi[m]->name, pi[m]->value[i]);
					break;
				}

			}

		}

		printf("\n");

	}

	int tst_number,net_number;
	//論理シミュレーション
	for (tst_number=0; tst_number < sim_test; tst_number++) {		//テストパターン数回

		printf("tp%d\n\n", tst_number+test_number);

		for (net_number = 0; net_number < n_pi; net_number++) {

			pi[net_number]->detec[tst_number] = 0;					//detectability初期化
			printf("%s\t%d\tdetectability:%d\n", pi[net_number]->name, pi[net_number]->value[tst_number],pi[net_number]->detec[tst_number]);

		}

		for (net_number = n_pi; net_number < n_net; net_number++) {	//信号線数回

			gate_calc(tst_number, net_number, sort_net);			//正常値算出

			if (sort_net[net_number]->n_out <= 0){					//外部出力線にはdetectability=1
				sort_net[net_number]->detec[tst_number] = 1; 
			}
			else { 
				sort_net[net_number]->detec[tst_number] = 0;		//detectability初期化
			}			//detectability初期化

			printf("%s\t%d\t detectability:%d\n", sort_net[net_number]->name, sort_net[net_number]->value[tst_number],sort_net[net_number]->detec[tst_number]);
		}

		printf("\n");

	}

	printf("%d番目から%d番目のテストパターンによる論理シミュレーション完了\n",test_number,test_number+sim_test);

	return 1;
}

void gate_calc(int tst_number, int net_number,NLIST** sort_net) {

	int j, flag=-1;

	switch (sort_net[net_number]->type) {

	case IN:
	case BUF:
		flag = 3;
		break;

	case INV:

		if (sort_net[net_number]->in[0]->value[tst_number] == 0) {
			flag = 1;
			break;
		}
		else if (sort_net[net_number]->in[0]->value[tst_number] == 1) {
			flag = 0;
			break;
		}
		else {
			flag = -1;
			break;
		}

	case AND:

		for (j = 0; j < sort_net[net_number]->n_in; j++) {

			if (sort_net[net_number]->in[j]->value[tst_number] == 0) {
				flag = 0;	//入力に制御値0を観測
				break;
			}

			flag = 1;	//全入力が非制御値1を観測

		}

		break;

	case NAND:

		for (j = 0; j < sort_net[net_number]->n_in; j++) {

			if (sort_net[net_number]->in[j]->value[tst_number] == 0) {
				flag = 1;	//入力に制御値0を観測
				break;
			}

			flag = 0;	//全入力が非制御値1を観測

		}

		break;

	case OR:
		for (j = 0; j < sort_net[net_number]->n_in; j++) {

			if (sort_net[net_number]->in[j]->value[tst_number] == 1) {
				flag = 1;	//入力に制御値1を観測
				break;
			}

			flag = 0;	//全入力が非制御値0を観測

		}

		break;

	case NOR:
		for (j = 0; j < sort_net[net_number]->n_in; j++) {

			if (sort_net[net_number]->in[j]->value[tst_number] == 1) {
				flag = 0;	//入力に制御値1を観測
				break;
			}

			flag = 1;	//全入力が非制御値0を観測

		}

		break;

	case EXOR:

		if (sort_net[net_number]->n_in != 2) {
			flag = -1;
			break;
		}

		if (sort_net[net_number]->in[0]->value[tst_number] != sort_net[net_number]->in[1]->value[tst_number]) {
			flag = 1;
			break;
		}
		else if (sort_net[net_number]->in[0]->value[tst_number] == sort_net[net_number]->in[1]->value[tst_number]) {
			flag = 0;
			break;
		}
		else {
			flag = -1;
			break;
		}

	case EXNOR:

		if (sort_net[net_number]->n_in != 2) {
			flag = -1;
			break;
		}

		if (sort_net[net_number]->in[0]->value[tst_number] != sort_net[net_number]->in[1]->value[tst_number]) {
			flag = 0;
			break;
		}
		else if (sort_net[net_number]->in[0]->value[tst_number] == sort_net[net_number]->in[1]->value[tst_number]) {
			flag = 1;
			break;
		}
		else {
			flag = -1;
			break;
		}

	case FOUT:
		flag = 3;
		break;

	default:
		printf("信号線のゲートタイプに誤りがあります\n");
		return -1;
		break;

	}//switch文終了

	if (flag == 0) {
		sort_net[net_number]->value[tst_number]= 0;
	}
	else if (flag == 1) {
		sort_net[net_number]->value[tst_number] = 1;
	}
	else if (flag == 3) {
		sort_net[net_number]->value[tst_number] = sort_net[net_number]->in[0]->value[tst_number];
	}
	else if (flag == -1) {
		printf("信号線上に誤った値が割当てられました\n論理シミュレーション失敗\n");
		exit(1);
	}



}
