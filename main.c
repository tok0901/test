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

//プロトタイプ宣言////////////////////////////////////////////////////////////////

//ネットリスト読込み
int command(int argc, char** argv);
int input_f(char* tp, char* pin, char* v);
int make_net(NLIST* nl);


//故障シミュレーション
int devide_ffr(NLIST** sort_net);
int losic_simulation(int test_number, int sim_test, NLIST** sort_net);
int SAF_PPSFP(int test_number, int sim_test, FFR* ffr);


//未識別故障ペア取得
int Confirm_Fault_Pair(DICT* dic,int test_number);

//観測ポイント挿入
int TPI_Score_Calc(NLIST** sort_net);

//////////////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[]) {

	double result, all_fault;

	//コマンド解析
	if (command(argc, argv) != 1) {
		printf("\n\nコマンド解析エラー\n");
		return 0;
	}

	//ファイル入力
	if (input_f(opt.tp, opt.pin, opt.v) != 1) {
		printf("\n\nファイル入力エラー\n");
		return 0;
	}

	//信号線正規化
	if (make_net(nl) != 1) {
		printf("\n\n信号線正規化エラー\n");
		return 0;
	}

	//FFR分割
	if (devide_ffr(sort_net) != 1) {
		printf("\n\nFFR分割エラー\n");
		return 0;
	}

	//出力応答値数初期化
	n_tpi_po = n_po;
	//ループ変数、処理変数初期化
	int confirm_flag; int n_unconf_fault_grp;
	not_fault = 0;


	//未識別故障ペア数が0になるまで実行
	do {

		//故障シミュレーション(論理シミュレーション,PPSFP,CPT)
		int sim_test = 64, max_test;
		for (int test_number = 0; test_number < n_test; test_number += sim_test) {

			max_test = n_test - test_number;

			if (max_test <= sim_test) {
				sim_test = max_test;
			}

			if (losic_simulation(test_number, sim_test, sort_net) != 1) {
				printf("\n\n論理シミュレーションエラー\n");
				return 0;
			}

			//故障辞書内-未識別故障集合関連メンバの領域確保
			int sim_number = test_number + sim_test;
			for (int test_dic = test_number; test_dic < sim_number; test_dic++) {
				dic[test_dic].unconf_fault = (NLIST***)malloc(sizeof(NLIST**) * n_ffr);
				dic[test_dic].unconf_saf_flag = (short**)malloc(sizeof(short*) * n_ffr);
				dic[test_dic].po_value = (char**)malloc(sizeof(char*) * n_ffr);
				dic[test_dic].n_unconf_fault = (int*)malloc(sizeof(int) * n_ffr);
			}

			if (SAF_PPSFP(test_number, sim_test, ffr) != 1) {
				printf("\n\nPPSFPエラー\n");
				return 0;
			}

			printf("****************%d番目から%d番目のテストパターンによる故障検出完了********************************\n", test_number, test_number + sim_test);


			//故障シミュレーション出力結果
			for (int confirm_number = 0; confirm_number < sim_test; confirm_number++) {

				printf("\n****************tp%d*********************************************\n", confirm_number);

				for (int hash_number = 0; hash_number < dic[test_number+confirm_number].n_grp; hash_number++) {

					printf("\n出力応答値:%s\n", dic[test_number + confirm_number].po_value[hash_number]);

					for (int fault_number = 0; fault_number < dic[test_number + confirm_number].n_unconf_fault[hash_number]; fault_number++) {

						if (dic[test_number + confirm_number].unconf_saf_flag[hash_number][fault_number] == 0) {
							printf("s-a-0\t%s\n", dic[test_number + confirm_number].unconf_fault[hash_number][fault_number]->name);
						}
						else if (dic[test_number + confirm_number].unconf_saf_flag[hash_number][fault_number] == 1) {
							printf("s-a-1\t%s\n", dic[test_number + confirm_number].unconf_fault[hash_number][fault_number]->name);
						}

					}

				}

			}

			//未識別故障ペア取得
			for (int confirm_number = 0; confirm_number < sim_test; confirm_number++) {

				confirm_flag = Confirm_Fault_Pair(dic, test_number + confirm_number);
				if (confirm_flag == 0) {
					printf("\n\n全故障の識別完了\n\n");
					break;
				}
				hash_reset(test_number + confirm_number);
			}


		}

		if (not_fault == 0) {

			//未検出故障出力
			for (int net_number = 0; net_number < n_net; net_number++) {
				if (sort_net[net_number]->sim_fault0_flag != 1) {
					printf("未検出故障:s-a-0\t%s\n", sort_net[net_number]->name);
					not_fault++;
				}
				if (sort_net[net_number]->sim_fault1_flag != 1) {
					printf("未検出故障:s-a-1\t%s\n", sort_net[net_number]->name);
					not_fault++;
				}
			}


			//全対象故障数
			all_fault = n_net * 2.0;

			//全検出故障数
			n_sim_fault = all_fault - not_fault;

			//故障検出率
			result = (double)n_sim_fault / all_fault;

			//全故障ペア数
			all_conf_fault_pair = n_sim_fault * (n_sim_fault - 1) / 2;

		}

		//未識別故障ペア取得出力結果
		int devide_number = 0; 
		unconf_fault_pair = 0; n_unconf_fault_grp = 0;
		NLIST* temp; short saf_temp;
		while (devide_number < conf_fault_module) {

			//未識別故障集合内-故障が2つ以上の時
			if (hash.n_unconf_fault[devide_number] >= 2) {
				unconf_fault_pair = unconf_fault_pair + hash.n_unconf_fault[devide_number] * (hash.n_unconf_fault[devide_number] - 1) / 2;
				n_unconf_fault_grp++;

				printf("\n\n****************Unconf_Fault_Pair%d*********************************************\n\n", devide_number);
				int search_number = 0, fault_number = 0;

				while (fault_number < hash.n_unconf_fault[devide_number]) {
					if (hash.unconf_fault[devide_number][search_number] != NULL) {
						if (hash.saf_flag[devide_number][search_number] == 0) {
							printf("s-a-0\t%s\n", hash.unconf_fault[devide_number][search_number]->name);
						}
						else if (hash.saf_flag[devide_number][search_number] == 1) {
							printf("s-a-1\t%s\n", hash.unconf_fault[devide_number][search_number]->name);
						}

						fault_number++;
					}

					search_number++;
				}

			}
			//未識別故障集合内-故障が1つのみの時(余領域削除前)
			else if ((hash.n_unconf_fault[devide_number] == 1) && (hash.n_index[devide_number] >= 2)) {
				for (int reset_number = 0; reset_number < hash.n_index[devide_number]; reset_number++) {
					if (hash.unconf_fault[devide_number][reset_number] != NULL) {
						hash.n_index[devide_number] = 1;
						temp = hash.unconf_fault[devide_number][reset_number];
						free(hash.unconf_fault[devide_number]);
						hash.unconf_fault[devide_number] = (NLIST**)malloc(sizeof(NLIST*));
						hash.unconf_fault[devide_number][0] = temp;
						saf_temp = hash.saf_flag[devide_number][reset_number];
						free(hash.saf_flag[devide_number]);
						hash.saf_flag[devide_number] = (short*)malloc(sizeof(short));
						hash.saf_flag[devide_number][0] = saf_temp;
						free(hash.confirm_flag[devide_number]);
						hash.confirm_flag[devide_number] = (short*)malloc(sizeof(short));
						hash.confirm_flag[devide_number][0] = -1;
						break;
					}
				}
			}

			devide_number++;
		}

		//観測ポイント挿入
		if (TPI_Score_Calc(sort_net) != 1) {
			printf("\n\n観測ポイント挿入エラー\n");
			for (int x=0; x < n_tpi; x++) {
				printf("%s\n", tpi_net[x]->name);
			}
			printf("\n\n観測ポイント数:%d\n", n_tpi);
			return 0;
		}


	}while (n_sim_fault!=conf_fault_module);

	printf("\n検出対象故障数:%.0f\n", all_fault);

	printf("\n検出故障数:%.0f\n", n_sim_fault);

	printf("\n故障検出率:%.2f％\n", result * 100);

	printf("\n全故障ペア数:%.0f\n", all_conf_fault_pair);

	printf("\n未識別故障ペア数:%.0f\n", unconf_fault_pair);

	printf("\nDC:%.2f％\n", (double)(all_conf_fault_pair - unconf_fault_pair) / all_conf_fault_pair * 100);

	printf("\n未識別故障集合配列数:%.0f\n", conf_fault_module);

	printf("\n未識別故障集合数:%d\n", n_unconf_fault_grp);

	return 0;

}
