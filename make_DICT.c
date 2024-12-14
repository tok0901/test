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

#define BIT_20 20

//引数をtest_number,sim_test,sim_netに修正
int SAF_make_DICT(int test_number, int sim_test,NLIST* sim_net) {

	/*/確認用
	if (sim_net->n == 1858) {
		printf("\n%s", sim_net->name);
	}///////*/

	//printf("\n\n***********FFR%d**********\n", ffr_number);
	for (int test_dic = test_number; test_dic < test_number + sim_test; test_dic++) {

		int tst_number = test_dic % BIT_64;

		if (bit_search_N(sim_net->det, tst_number) == 1) {


			//出力応答正常値算出
			ULLI* temp;
			temp = observate_po_val(test_dic, PO_VAL);

			//出力応答異常値算出
			ULLI* f_temp;
			f_temp = calc_po_val(test_dic, sim_net, PO_VAL);

			//両出力応答値比較
			int po_val_flag;
			po_val_flag = strcmp_po_val(temp, f_temp, PO_VAL);

			/*/確認用
			int tst_number = test_dic % BIT_64;
			if ((bit_search_N(sim_net->det, tst_number) == 1) && (po_val_flag == 1)) {

				printf("\ntp%d\t%s", test_dic, sim_net->name);
				printf("\nval:"); printBinary(sim_net->val, sim_test); printf("\n");
				printf("det:"); printBinary(sim_net->det, sim_test); printf("\n");

				printf("\n正常値:");
				for (int bit_number = 0; bit_number < n_64bit_po; bit_number++) {
					printBinary(temp[bit_number], BIT_64);
				}
				printf("\n故障値:");
				for (int bit_number = 0; bit_number < n_64bit_po; bit_number++) {
					printBinary(f_temp[bit_number], BIT_64);
				}
			}//////////////////////////*/


			free(temp);
			free(f_temp);

			//sim_net-検出可能な場合
			if (po_val_flag == 1) {

				ULLI* f_temp = calc_po_val(test_dic, sim_net, TPI_PO_VAL);

				/*/確認用
				printf("\ntp%d", test_dic);
				printf("\n出力応答値:");
				print_po_val(f_temp); printf("\n");
				//////////////////////////*/


				//出力応答値探索及び故障辞書内-故障挿入場所設定
				hash_insert_DICT(test_dic, f_temp);

				//FFR内-全検出故障挿入及び故障辞書生成
				fault_detect_DICT(test_dic, sim_net);

			}

		}

	}

	return 1;

}

void malloc_DICT(int test_number, int sim_test) {

	for (int test_dic = test_number; test_dic < test_number+sim_test; test_dic++) {
		dic[test_dic].unconf_fault = (NLIST***)malloc(sizeof(NLIST**) * n_ffr);
		dic[test_dic].unconf_saf_fault = (ULLI**)malloc(sizeof(ULLI*) * n_ffr);
		dic[test_dic].po_val = (ULLI**)malloc(sizeof(ULLI*) * n_ffr);
		dic[test_dic].n_unconf_fault = (int*)malloc(sizeof(int) * n_ffr);
	}

}

//PO_VAL(TPI前),TPI_PO_VAL(TPI後)
ULLI* calc_po_val(int test_dic,NLIST* sim_net,int tpi_flag) {

	int tst_number = test_dic % BIT_64;
	int ffr_id = sim_net->ffr_id;
	ULLI* temp = NULL;

	int fval, calc_flag,
		po_number = 0, net_cur_number = 0, bit_number = 0;


	//po_val
	if (tpi_flag == PO_VAL) {

		calc_flag = n_po;


		temp = (ULLI*)malloc(sizeof(ULLI) * n_64bit_po);
		for (int bit_number = 0; bit_number < n_64bit_po; bit_number++) {
			temp[bit_number] = 0ULL;
		}

	}

	//tpi_po_val
	else if (tpi_flag == TPI_PO_VAL) {

		calc_flag = n_tpi_po;


		temp = (ULLI*)malloc(sizeof(ULLI) * n_64bit);
		for (int bit_number = 0; bit_number < n_64bit; bit_number++) {
			temp[bit_number] = 0ULL;
		}

	}

	else {
		printf("出力応答値算出エラー\n");
		exit(0);
	}

	while (po_number < calc_flag) {

		//PO->fval確認
		fval = bit_search_N(ffr[ffr_id].po_fault_flag[po_number], tst_number);


		if (fval == 1) {
			temp[bit_number] = bit_setting_1(temp[bit_number], net_cur_number);
		}
		else if (fval == 0) {
			temp[bit_number] = bit_setting_0(temp[bit_number], net_cur_number);
		}


		po_number++;
		net_cur_number++;
		if ((po_number % BIT_64 == 0) && (net_cur_number == BIT_64)) {
			bit_number++;
			net_cur_number = 0;
		}

	}

	return temp;

}

ULLI* observate_po_val(int test_dic,int flag) {

	ULLI* temp;
	temp = (ULLI*)malloc(sizeof(ULLI) * n_64bit);
	for (int bit_number = 0; bit_number < n_64bit; bit_number++) {
		temp[bit_number] = 0ULL;
	}

	int tst_number = test_dic % BIT_64;	
	int net_cur_number = 0, bit_number = 0, po_number = 0;
	int val;

	while (po_number < n_po) {

		if (flag == PO_VAL) {
			val = bit_search_N(po[po_number]->val, tst_number);
		}

		else if (flag == PO_FVAL) {
			val = bit_search_N(po[po_number]->fval, tst_number);
		}

		if (val == 1) {

			temp[bit_number] = bit_setting_1(temp[bit_number],net_cur_number);
		}
		else if (val == 0) {
			temp[bit_number] = bit_setting_0(temp[bit_number], net_cur_number);
		}
		else {
			printf("\n出力応答値観測エラー\n");
			exit(0);
		}

		po_number++;
		net_cur_number++;

		if ((po_number % BIT_64 == 0) && (net_cur_number == BIT_64)) {

			bit_number++;
			net_cur_number = 0;
		}
	}

	return temp;
	
}

void hash_insert_DICT(int test_dic, ULLI* temp) {

	int insert_flag = 1;
	
	for (int hash_number = 0; hash_number < dic[test_dic].n_grp; hash_number++) {

		if (strcmp_po_val(dic[test_dic].po_val[hash_number],temp,TPI_PO_VAL)==0) {

			insert_flag = 0;
			dic[test_dic].insert_number = hash_number;
			break;
		}

	}

	if (insert_flag == 1) {

		/*int bit_flag = n_net % BIT_64;
		if (bit_flag == 0) {
			bit_flag = n_net / BIT_64;
		}
		else {
			bit_flag = n_net / BIT_64 + 1;
		}*/

		int n_hash, insert_number;
		dic[test_dic].n_grp++;
		n_hash = dic[test_dic].n_grp;
		insert_number = n_hash - 1;
		dic[test_dic].insert_number = insert_number;
		dic[test_dic].unconf_fault[insert_number] = (NLIST**)malloc(sizeof(NLIST*) * 1000);
		dic[test_dic].unconf_saf_fault[insert_number] = (ULLI*)malloc(sizeof(ULLI) * BIT_20);
		for (int bit_number = 0; bit_number < BIT_20; bit_number++) {
			dic[test_dic].unconf_saf_fault[insert_number][bit_number] = 0ULL;
		}
		dic[test_dic].po_val[insert_number] = (ULLI*)malloc(sizeof(ULLI) * n_64bit);
		for (int bit_number = 0; bit_number < n_64bit; bit_number++) {
			dic[test_dic].po_val[insert_number][bit_number] = 0ULL;
			dic[test_dic].po_val[insert_number][bit_number] = temp[bit_number];
		}
		dic[test_dic].n_unconf_fault[insert_number] = 0;

	}

}

void fault_detect_DICT(int test_dic, NLIST* sim_net) {

	int tst_number = test_dic % BIT_64;
	int hash_number = dic[test_dic].insert_number;
	int fault_number = dic[test_dic].n_unconf_fault[hash_number];
	int saf_number = fault_number / BIT_64;
	int det_number = fault_number % BIT_64;


		//1縮退故障検出
		if (bit_search_N(sim_net->val, tst_number) == 0) {

			if (sim_net->test_sf1 == YES) {
				dic[test_dic].unconf_fault[hash_number][fault_number] = sim_net;
				dic[test_dic].unconf_saf_fault[hash_number][saf_number] = bit_setting_1(dic[test_dic].unconf_saf_fault[hash_number][saf_number], det_number);
				dic[test_dic].n_unconf_fault[hash_number]++;
				sim_net->sim_fault1_flag = 1;	//1縮退故障検出フラグを有効
				//printf("s-a-1\t%s\n", sim_net->name);
			}
		}

		//0縮退故障検出
		else if(bit_search_N(sim_net->val, tst_number) == 1) {

			if (sim_net->test_sf0 == YES) {
				dic[test_dic].unconf_fault[hash_number][fault_number] = sim_net;
				dic[test_dic].unconf_saf_fault[hash_number][saf_number] = bit_setting_0(dic[test_dic].unconf_saf_fault[hash_number][saf_number], det_number);
				dic[test_dic].n_unconf_fault[hash_number]++;
				sim_net->sim_fault0_flag = 1;	//1縮退故障検出フラグを有効
				//printf("s-a-0\t%s\n", sim_net->name);
			}
		}


	////故障挿入失敗
	//if (dic[test_dic].n_unconf_fault[hash_number] == 0) {

	//	free(dic[test_dic].unconf_fault[hash_number]);
	//	free(dic[test_dic].unconf_saf_fault[hash_number]);
	//	free(dic[test_dic].po_val[hash_number]);
	//	dic[test_dic].n_grp--;

	//}

}

//void fault_insert_DICT(int test_dic, int ffr_number) {
//
//	int tst_number = test_dic % BIT_64;
//	int hash_number = dic[test_dic].insert_number;
//	int fault_number = dic[test_dic].n_unconf_fault[hash_number];
//	int saf_number = fault_number / BIT_64;
//	int det_number = fault_number % BIT_64;
//	int ffr_id;
//
//	queue_enqueue(ffr[ffr_number].fos);
//	NLIST* signalo;
//
//	while (queue_empty() == 1) {
//
//		signalo = queue_dequeue();
//		//ffr_id = signalo->ffr_id;
//
//		//detectability-テストパターン目を確認(1なら故障挿入)
//		if (bit_search_N(signalo->det, tst_number) == 1) {
//
//			//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
//			if ((fault_number % BIT_64 == 0) && (det_number == BIT_64)) {
//				det_number = 0;
//				saf_number++;
//			}
//
//			//検出可能故障挿入
//			dic[test_dic].unconf_fault[hash_number][fault_number] = signalo;
//
//			//signalo->val-テストパターン目を確認(0なら1縮退,1なら0縮退)
//			//1縮退故障値挿入
//			if (bit_search_N(signalo->val, tst_number) == 0) {
//
//				//saf_fault[hash_number][saf_number]-det_number番目に1を設定
//				dic[test_dic].unconf_saf_fault[hash_number][saf_number] = bit_setting_1(dic[test_dic].unconf_saf_fault[hash_number][saf_number], det_number);
//				signalo->sim_fault1_flag = 1;	//1縮退故障検出フラグを有効
//				printf("s-a-1\t%s\n", signalo->name);
//			}
//
//			//0縮退故障値挿入
//			else if(bit_search_N(signalo->val, tst_number) == 1) {
//
//				//saf_fault[hash_number][saf_number]-det_number番目に0を設定
//				dic[test_dic].unconf_saf_fault[hash_number][saf_number] = bit_setting_0(dic[test_dic].unconf_saf_fault[hash_number][saf_number], det_number);
//				signalo->sim_fault0_flag = 1;	//0縮退故障検出フラグを有効
//				printf("s-a-0\t%s\n", signalo->name);
//			}
//
//			else {
//				printf("故障検出失敗\ndetectabilityに誤りがあります\n");
//				exit(0);
//			}
//
//			dic[test_dic].n_unconf_fault[hash_number]++;
//
//			fault_number++;
//			det_number++;
//
//		}
//
//		//入力線エンキュー(範囲はIN,FOUTまで)
//		if ((signalo->type != IN) && (signalo->type != FOUT)) {
//
//			for (int in_number = 0; in_number < signalo->n_in; in_number++) {
//
//				//tpi以外の入力線を選択
//				if (signalo->in[in_number]->tpi_flag != 1) {
//
//					queue_enqueue(signalo->in[in_number]);
//					
//				}
//			}
//
//		}
//
//	}
//
//	//故障挿入失敗
//	if (dic[test_dic].n_unconf_fault[hash_number] == 0) {
//
//		free(dic[test_dic].unconf_fault[hash_number]);
//		free(dic[test_dic].unconf_saf_fault[hash_number]);
//		free(dic[test_dic].po_val[hash_number]);
//		dic[test_dic].n_grp--;
//
//	}
//
//}

void hash_reset_DICT(int test_number) {

	for (int hash_number = 0; hash_number < dic[test_number].n_grp; hash_number++) {

		free(dic[test_number].unconf_fault[hash_number]);
		free(dic[test_number].unconf_saf_fault[hash_number]);
		free(dic[test_number].po_val[hash_number]);
		free(dic[test_number].hash_number[hash_number]);
		dic[test_number].n_unconf_fault[hash_number] = 0;

	}

	free(dic[test_number].unconf_fault);
	free(dic[test_number].unconf_saf_fault);
	free(dic[test_number].po_val);
	dic[test_number].insert_number = 0;
	dic[test_number].n_grp = 0;
	free(dic[test_number].n_unconf_fault);
	free(dic[test_number].hash_number);

}
