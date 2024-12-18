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


#define SELECT 1	//観測ポイント評価段階
#define INSERT 2	//観測ポイント挿入段階

//プロトタイプ宣言////////////////////////////////////////////////////////////////
//
//
//ネットリスト読込み
int command(int argc, char** argv);
int input_f(char* tp, char* pin, char* v);
int make_net(NLIST* nl);
//
//
//FFR分割
int devide_ffr(void);
//
//
//故障シミュレーション
int losic_simulation(int test_number, int sim_test);
int SAF_CPT(int test_number, int sim_test, int ffr_number);
int SAF_PPSFP(int test_number, int sim_test, int ffr_number);
int SAF_make_DICT(int test_number, int sim_test, NLIST* sim_net);
//
//
//未識別故障ペア取得
int make_Confirm_Fault_Pair(int test_number);
//
// 
//観測ポイント挿入
int TPI_Score_Calc(void);
//
// 
//////////////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[]) {

	//制御モジュール操作変数宣言
	double result, all_fault;
	int scan_id;
	int tpi_flag;
	/////////////////////////*/


	//ネットリスト読込み実行/////////////////////////////////////////////////////////////////


	//コマンド解析実行
	if (command(argc, argv) != 1) {
		printf("\n\nコマンド解析エラー\n");
		return 0;
	}/////////////////////////*/


	//ファイル入力実行
	if (input_f(opt.tp, opt.pin, opt.v) != 1) {
		printf("\n\nファイル入力エラー\n");
		return 0;
	}/////////////////////////*/


	//信号線正規化
	if (make_net(nl) != 1) {
		printf("\n\n信号線正規化エラー\n");
		return 0;
	}/////////////////////////*/


	//ネットリスト読込み完了///////////////////////////////////////////////////////////////////////////////////


	//ループ変数、処理変数初期化
	int confirm_flag;
	n_tpi_po = n_po;
	not_fault = 0;
	tpi_flag = INSERT;
	n_64bit_po = count_number_64(n_po);
	char* target = NULL;			//検索する信号線名
	/////////////////////////*/


	//未識別故障ペア数が0になるまで
	do {


		//出力応答値ビット列数更新
		n_64bit = count_number_64(n_tpi_po);
		/////////////////////////*/


			//実行確認
		if (n_tpi == 2) {
			printf("FFR分割を開始しますか？(Yes:1,No:2)\n");
			scanf("%d", &scan_id);
			if (scan_id != 1) {
				return 0;
			}/////////////////////////*/

		}


		//FFR分割
		if (devide_ffr() != 1) {
			printf("\n\nFFR分割エラー\n");
			return 0;
		}/////////////////////////*/


		//故障シミュレーション(論理シミュレーション,PPSFP,CPT)/////////////////////////


		//シミュレーションテストパターン数初期化
		int sim_test = BIT_64,
			max_test;

		for (int test_number = 0; test_number < n_test; test_number += sim_test) {


			//シミュレーションテストパターン数更新
			max_test = n_test - test_number;
			if (max_test <= sim_test) {
				sim_test = max_test;
			}


			//論理シミュレーション実行
			if (losic_simulation(test_number, sim_test) != 1) {
				printf("\n\n論理シミュレーションエラー\n");
				return 0;
			}/////////////////////////*/


			//検出対象故障カウント
			for (int net_number = 0; net_number < n_net; net_number++) {

				if (sort_net[net_number]->test_sf0 == YES) {

					ffr[sort_net[net_number]->ffr_id].n_unconf_fault_det++;

				}

				if (sort_net[net_number]->test_sf1 == YES) {

					ffr[sort_net[net_number]->ffr_id].n_unconf_fault_det++;

				}
			}

			//CPT実行
			for (int ffr_number = 0; ffr_number < n_ffr; ffr_number++) {

				if (ffr[ffr_number].n_unconf_fault_det != 0) {

					if (SAF_CPT(test_number, sim_test, ffr_number) != 1) {
						printf("\n\nCPTエラー\n");
						return 0;
					}

				}
			}/////////////////////////*/


			/*/確認用
			printf("<detectability確認>");
			for (int net_number = 0; net_number < n_net; net_number++) {
				printf("\n%s\t", sort_net[net_number]->name);
				printBinary(sort_net[net_number]->det, sim_test);

				NLIST* sim_net;
				sim_net = sort_net[net_number];

			}/////////////////////////*/


			//故障辞書領域確保
			malloc_DICT(test_number, sim_test);
			/////////////////////////*/


			//PPSFP実行(テストパターンtest_number～sim_testまで)
			for (int ffr_number = 0; ffr_number < n_ffr; ffr_number++) {

				if (ffr[ffr_number].n_unconf_fault_det != 0) {
					if (SAF_PPSFP(test_number, sim_test, ffr_number) != 1) {
						printf("\n\nPPSFPエラー\n");
						return 0;
					}
				}

			}/////////////////////////*/


			//故障辞書生成実行(テストパターンtest_number～sim_testまで)
			for (int net_number = 0; net_number < n_net; net_number++) {

				if ((sort_net[net_number]->test_sf0 == YES) || (sort_net[net_number]->test_sf1 == YES)) {

					if ((sort_net[net_number]->conf_fault0_flag != 1) || (sort_net[net_number]->conf_fault1_flag != 1)) {
						if (SAF_make_DICT(test_number, sim_test, sort_net[net_number]) != 1) {
							printf("\n\n故障辞書生成エラー\n");
							return 0;
						}

					}
				}
			}
			/////////////////////////*/


			printf("\n****************%d番目から%d番目のテストパターンによる故障検出完了********************************\n", test_number, test_number + sim_test - 1);


			/*/確認用(故障シミュレーション)
			for (int tst_number = test_number; tst_number < test_number + sim_test; tst_number++) {

				printf("\n**************tp%d***************\n\n", tst_number);

				int n_result_fault = 0, n_sum_unconf_fault_pair = 0;
				for (int hash_number = 0; hash_number < dic[tst_number].n_grp; hash_number++) {

					int saf_number = 0;
					int det_number = 0;

					printf("出力応答値:");
					for (int bit_number = 0; bit_number < n_64bit; bit_number++) {
						printBinary(dic[tst_number].po_val[hash_number][bit_number], n_tpi_po);
					}
					printf("\n");

					n_result_fault += dic[tst_number].n_unconf_fault[hash_number];

					int n_result_unconf_fault_pair = 0;

					for (int fault_number = 0; fault_number < dic[tst_number].n_unconf_fault[hash_number]; fault_number++) {

						//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
						if ((fault_number % BIT_64 == 0) && (det_number == BIT_64)) {
							det_number = 0;
							saf_number++;
						}

						if (bit_search_N(dic[tst_number].unconf_saf_fault[hash_number][saf_number], det_number) == 0) {
							printf("s-a-0\t%s\n", dic[tst_number].unconf_fault[hash_number][fault_number]->name);
							dic[tst_number].unconf_fault[hash_number][fault_number]->sim_fault0_flag = 1;

							if (dic[tst_number].unconf_fault[hash_number][fault_number]->conf_fault0_flag != 1) {
								n_result_unconf_fault_pair++;
							}
						}
						else {
							printf("s-a-1\t%s\n", dic[tst_number].unconf_fault[hash_number][fault_number]->name);
							dic[tst_number].unconf_fault[hash_number][fault_number]->sim_fault1_flag = 1;

							if (dic[tst_number].unconf_fault[hash_number][fault_number]->conf_fault1_flag != 1) {
								n_result_unconf_fault_pair++;
							}
						}

						det_number++;



						////信号線検索
						//if (strcmp(dic[tst_number].unconf_fault[hash_number][fault_number]->name, target) == 0) {
						//	printf("tp%d", tst_number);
						//}////////////

					}

					n_result_unconf_fault_pair = n_result_unconf_fault_pair * (n_result_unconf_fault_pair - 1) / 2;

					printf("\n\n未識別故障ペア数:%d\n\n", n_result_unconf_fault_pair);

					n_sum_unconf_fault_pair += n_result_unconf_fault_pair;

				}

					//printf("\n\n検出故障数:%d\n", n_result_fault);
					printf("\n合計未識別故障ペア数:%d\n", n_sum_unconf_fault_pair);

			}/////////////////////////*/


			//未識別故障ペア取得実行
			for (int tst_number = test_number; tst_number < test_number + sim_test; tst_number++) {

				confirm_flag = make_Confirm_Fault_Pair(tst_number);
				if (confirm_flag == 0) {
					printf("\n\n全故障の識別完了\n\n");
					break;
				}

			}/////////////////////////*/


		}


		//確認用(故障検出率)
		if (n_tpi == 0) {

			//未検出故障出力
			for (int net_number = 0; net_number < n_net; net_number++) {

				if (sort_net[net_number]->test_sf0 == YES) {
					if (sort_net[net_number]->sim_fault0_flag != 1) {
						printf("\n\n未検出故障:s-a-0\t%s\n", sort_net[net_number]->name);
						printf("n=%d\n", sort_net[net_number]->n);
						printf("val\t"); printBinary(sort_net[net_number]->val, sim_test);
						printf("\ndet\t"); printBinary(sort_net[net_number]->det, sim_test);
						not_fault++;
					}
				}

				if (sort_net[net_number]->test_sf1 == YES) {
					if (sort_net[net_number]->sim_fault1_flag != 1) {
						printf("\n\n未検出故障:s-a-1\t%s\n", sort_net[net_number]->name);
						printf("n=%d\n", sort_net[net_number]->n);
						printf("val\t"); printBinary(sort_net[net_number]->val, sim_test);
						printf("\ndet\t"); printBinary(sort_net[net_number]->det, sim_test);
						not_fault++;
					}
				}
			}


			//検出故障数
			n_sim_fault = n_rep - not_fault;

			//故障検出効率
			result = (double)n_sim_fault / n_rep;

			//全故障ペア数
			all_conf_fault_pair = n_sim_fault * (n_sim_fault - 1) / 2;


			printf("\n検出対象故障数:%d\n", n_rep);

			printf("\n検出故障数:%.0f\n", n_sim_fault);

			printf("\n未検出故障数:%d\n", not_fault);

			printf("\n故障検出効率:%.2f％\n", result * 100);

			printf("\n全故障ペア数:%.0f\n", all_conf_fault_pair);

		}


		//未識別故障ペア数算出
		int n_unconf_fault_grp = 0;
		for (int hash_number = 0; hash_number < hash.n_grp; hash_number++) {

			if (hash.n_unconf_fault[hash_number] >= 2) {

				n_unconf_fault_grp += hash.n_unconf_fault[hash_number] * (hash.n_unconf_fault[hash_number] - 1) / 2;

				for (int fault_number = 0; fault_number < hash.n_index[hash_number]; fault_number++) {

					if (hash.unconf_fault[hash_number][fault_number] != NULL) {

						ffr[hash.unconf_fault[hash_number][fault_number]->ffr_id].n_unconf_fault++;

					}
				}
			}
		}


		/*/確認用
		if (tpi_flag == INSERT) {
			for (int ffr_number = 0; ffr_number < n_ffr; ffr_number++) {
				printf("\nFFR_ID:%d\n信号線数:%d\n未識別故障格納個数:%d\n", ffr_number, ffr[ffr_number].n_net_sim, ffr[ffr_number].n_unconf_fault);
			}
		}
		/////////////////*/

		/*/従来手法観測ポイント挿入法(スコア計算による観測ポイント選択)
		if (confirm_flag!=0) {
			//観測ポイント挿入実行
			if (TPI_Score_Calc() != 1) {
				printf("\n\n観測ポイント挿入エラー\n");
				for (int x = 0; x < n_tpi; x++) {
					printf("%s\n", tpi_net[x]->name);
				}
				printf("\n\n観測ポイント数:%d\n", n_tpi);
				return 0;
			}
		}/////////////////////////*/


		//提案手法観測ポイント挿入法(未識別故障箇所とFFR解析による観測ポイント選択)
		if (confirm_flag != 0) {


			if (tpi_flag == INSERT) {

				//検出対象故障更新(識別故障を対象外に設定)
				for (int net_number = 0; net_number < n_net; net_number++) {
					if (sort_net[net_number]->conf_fault0_flag == 1) {
						sort_net[net_number]->test_sf0 = NO;
					}

					if (sort_net[net_number]->conf_fault1_flag == 1) {
						sort_net[net_number]->test_sf1 = NO;
					}
				}


				//観測ポイント評価段階へ切替え
				tpi_flag = SELECT;

				//未識別故障ペア集合保存(バックアップ)
				hash_backup_save(BACKUP_SELECT);

				//選択候補観測ポイント配列生成
				select_ffr_id = make_tpi_select_net();

				//選択候補観測ポイント挿入
				n_tpi_po++;
				n_tpi++;
				tpi_po_net = (NLIST**)realloc(tpi_po_net, sizeof(NLIST*) * n_tpi_po);
				tpi_po_net[n_tpi_po - 1] = tpi_select_net[select_sim_number].select_net;
				tpi_net = (NLIST**)realloc(tpi_net, sizeof(NLIST*) * n_tpi);
				tpi_net[n_tpi - 1] = tpi_select_net[select_sim_number].select_net;
				tpi_net[n_tpi - 1]->tpi_flag = 1;

				printf("\n評価観測ポイント:%s\n\n評価観測ポイント数:%d\n", tpi_net[n_tpi - 1]->name, n_select_net);

			}

			else if (tpi_flag == SELECT) {

				//未識別故障ペア数保存
				tpi_select_net[select_sim_number].n_unconf_fault_pair = n_unconf_fault_grp;
				tpi_select_net[select_sim_number].select_net->tpi_flag = 0;

				//選択候補観測ポイント配列指定要素番号更新
				select_sim_number++;

				//選択候補観測ポイント評価が残っている場合
				if (select_sim_number < n_select_net) {

					//未識別故障ペア集合(本体)削除
					//hash_backup_reset(HASH_SELECT);

					//未識別故障ペア集合(本体)再構築
					hash_backup_save(HASH_SELECT);

					//選択候補観測ポイント更新,挿入
					tpi_po_net[n_tpi_po - 1] = tpi_select_net[select_sim_number].select_net;
					tpi_net[n_tpi - 1] = tpi_select_net[select_sim_number].select_net;
					tpi_net[n_tpi - 1]->tpi_flag = 1;

					printf("\n\n評価観測ポイント:%s\n\n評価観測ポイント数:%d\n", tpi_net[n_tpi - 1]->name, n_select_net);

				}

				//全選択候補観測ポイント評価が完了した場合
				else if (select_sim_number >= n_select_net) {

					//観測ポイント挿入段階へ切替え
					tpi_flag = INSERT;

					//未識別故障ペア集合(本体)削除
					//hash_backup_reset(HASH_SELECT);

					//未識別故障ペア集合(本体)再構築
					hash_backup_save(HASH_SELECT);

					//観測ポイント挿入
					tpi_insert();

					//未識別故障ペア集合(バックアップ)削除
					//hash_backup_reset(BACKUP_SELECT);

					printf("\n\n**************選択観測ポイント:%s********************\n\n選択FFR:%d\n\n挿入観測ポイント数:%d\n", tpi_net[n_tpi - 1]->name,select_ffr_id, n_tpi);

				}


			}

			//FFR構造体配列の領域解放
			ffr_reset();

		}/////////////////*/

		printf("\n未識別故障ペア数:%d\n", n_unconf_fault_grp);

		printf("\n未識別故障集合配列数:%.0f\n", conf_fault_module);

		printf("\nDC:%.2f％\n", (double)(all_conf_fault_pair - n_unconf_fault_grp) / all_conf_fault_pair * 100);

		if ((((all_conf_fault_pair - n_unconf_fault_grp) / all_conf_fault_pair * 100)) >= 100) {
			printf("計測完了\n");
		}


	} while (n_sim_fault != conf_fault_module);


	printf("\n最終観測ポイント数:%d\n", n_tpi);


	return 0;

}
