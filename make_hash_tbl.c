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
	po_temp = (char*)malloc(sizeof(char) * (n_po+1));

	for (net_number = 0; net_number < n_po; net_number++) {
		
		//外部出力線に故障が伝搬する時(故障値の出力応答値を格納)
		if (po[net_number]->value_fault_flag[tst_number] == 1) {

			if (po[net_number]->value[tst_number] == 0) {
				po_temp[net_number] = '1';
			}
			else if (po[net_number]->value[tst_number] == 1) {
				po_temp[net_number] = '0';
			}
		}

		//外部出力線に故障が伝搬しない時(正常値の出力応答値を格納)
		else if (po[net_number]->value_fault_flag[tst_number] == 0) {

			if (po[net_number]->value[tst_number] == 0) {
				po_temp[net_number] = '0';
			}
			else if (po[net_number]->value[tst_number] == 1) {
				po_temp[net_number] = '1';
			}
		}


	}

	po_temp[net_number] = '\0';

	return po_temp;

}

//ハッシュ表挿入関数(po_value,insert_numberを更新,その他を生成)
void hash_insert(int test_number, int tst_number, char* po_temp) {

	int hash_number, test_dic,insert_flag,n_hash;
	test_dic = test_number + tst_number;

	insert_flag = 1;
	for (hash_number = 0; hash_number < dic[test_dic].n_grp; hash_number++) {

		if (strcmp(po_temp, dic[test_dic].po_value[hash_number]) == 0) {	//既存の出力応答値が得られた時
			insert_flag = 0;
			dic[test_dic].insert_number = hash_number;						//対象故障辞書のハッシュ表挿入箇所を更新
			break;
		}

	}

	if (insert_flag == 1) {		//ハッシュ表を追加する場合(新たな出力応答値が得られた時)

		dic[test_dic].n_grp++;
		n_hash = dic[test_dic].n_grp;
		//dic[test_dic].unconf_fault = (NLIST***)realloc(dic[test_dic].unconf_fault, sizeof(NLIST**) * n_hash);	//未識別故障集合配列拡張
		dic[test_dic].unconf_fault[n_hash - 1] = (NLIST**)malloc(sizeof(NLIST*) * n_net);						//n_net→ffr_net
		//dic[test_dic].unconf_saf_flag = (int**)realloc(dic[test_dic].unconf_saf_flag, sizeof(int*) * n_hash);	//検出故障フラグ配列拡張
		dic[test_dic].unconf_saf_flag[n_hash - 1] = (int*)malloc(sizeof(int) * n_net);
		//dic[test_dic].po_value = (char**)realloc(dic[test_dic].po_value, sizeof(char*) * n_hash);				//ハッシュ表配列拡張
		dic[test_dic].po_value[n_hash - 1] = po_temp;															//新規ハッシュ表(出力応答値)挿入
		//dic[test_dic].n_unconf_fault = (int*)realloc(dic[test_dic].n_unconf_fault, sizeof(int) * n_hash);		//未検出故障数配列拡張
		dic[test_dic].n_unconf_fault[n_hash - 1] = 0;															//未検出故障数初期化
		dic[test_dic].insert_number = n_hash - 1;																//対象故障辞書のハッシュ表挿入箇所を更新
	}


}
