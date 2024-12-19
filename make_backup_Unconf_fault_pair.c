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


//未識別故障ペア集合保存関数
void hash_backup_save(int flag) {

	if (flag == BACKUP_SELECT) {

		int n_grp = hash.n_grp;

		//未識別故障ペア集合数が増大した場合領域確保
		if (n_grp != backup.n_grp) {

			for (int hash_number = backup.n_grp; hash_number < n_grp; hash_number++) {

				int n_unconf_fault = hash.n_unconf_fault[hash_number];
				int bit_flag = count_number_64(n_unconf_fault);

				backup.unconf_fault[hash_number] = (NLIST**)malloc(sizeof(NLIST*) * n_unconf_fault);
				backup.n_unconf_fault[hash_number] = 0;
				backup.n_index[hash_number] = 0;
				backup.confirm_flag[hash_number] = (short*)malloc(sizeof(short) * n_unconf_fault);
				backup.n_confirm_flag[hash_number] = 0;
				backup.unconf_saf_fault[hash_number] = (ULLI*)malloc(sizeof(ULLI) * bit_flag);
				for (int bit_number = 0; bit_number < bit_flag; bit_number++) {
					backup.unconf_saf_fault[hash_number][bit_number] = 0ULL;
				}
			}

		}

		backup.n_grp = n_grp;

		//全ハッシュ配列を保存(行ごとに保存)
		for (int hash_number = 0; hash_number < n_grp; hash_number++) {

			int n_unconf_fault = hash.n_unconf_fault[hash_number];
			int n_index = hash.n_index[hash_number];
			int bit_flag = count_number_64(n_unconf_fault);

			//未識別故障ペア数削減箇所の集合のみを対象に処理
			if (n_unconf_fault != backup.n_unconf_fault[hash_number]) {

				//未識別故障ペア数更新
				backup.n_unconf_fault[hash_number] = n_unconf_fault;
				backup.n_index[hash_number] = n_unconf_fault;

				int cur_number = 0, saf_number = 0, det_number = 0, saf_cur_number = 0, det_cur_number = 0;

				//ハッシュ配列内-全故障を保存(列ごとに保存)
				for (int search_number = 0; search_number < n_index; search_number++) {

					//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
					if ((search_number % BIT_64 == 0) && (det_number == BIT_64)) {
						det_number = 0;
						saf_number++;
					}
					if ((cur_number % BIT_64 == 0) && (det_cur_number == BIT_64)) {
						det_cur_number = 0;
						saf_cur_number++;
					}

					//故障保存
					if (hash.unconf_fault[hash_number][search_number] != NULL) {

						backup.unconf_fault[hash_number][cur_number] = hash.unconf_fault[hash_number][search_number];
						backup.confirm_flag[hash_number][cur_number] = -1;
						cur_number++;

						int saf_flag = bit_search_N(hash.unconf_saf_fault[hash_number][saf_number], det_number);
						if (saf_flag == 0) {
							backup.unconf_saf_fault[hash_number][saf_cur_number] = bit_setting_0(backup.unconf_saf_fault[hash_number][saf_cur_number], det_cur_number);
						}
						else if (saf_flag == 1) {
							backup.unconf_saf_fault[hash_number][saf_cur_number] = bit_setting_1(backup.unconf_saf_fault[hash_number][saf_cur_number], det_cur_number);
						}
						det_cur_number++;

					}
					det_number++;


				}
			}



		}

	}

	else if (flag == HASH_SELECT) {

		int n_grp = backup.n_grp;

		//未識別故障ペア数削減時のみ処理
		if (n_grp != hash.n_grp) {

			//hash.n_grp = n_grp;

			//未識別故障ペア数削減箇所のみ再構築
			for (int hash_number = 0; hash_number < n_grp; hash_number++) {

				int n_unconf_fault = backup.n_unconf_fault[hash_number];

				//未識別故障ペア数削減時のみ処理
				if (n_unconf_fault != hash.n_unconf_fault[hash_number]) {

					int n_index = backup.n_index[hash_number];
					int bit_flag = count_number_64(n_unconf_fault);

					hash.n_unconf_fault[hash_number] = n_unconf_fault;
					hash.n_index[hash_number] = n_unconf_fault;
					hash.n_confirm_flag[hash_number] = 0;

					int cur_number = 0, saf_number = 0, det_number = 0, saf_cur_number = 0, det_cur_number = 0;

					//ハッシュ配列内-全故障を保存(列ごとに保存)
					for (int search_number = 0; search_number < n_index; search_number++) {

						//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
						if ((search_number % BIT_64 == 0) && (det_number == BIT_64)) {
							det_number = 0;
							saf_number++;
						}
						if ((cur_number % BIT_64 == 0) && (det_cur_number == BIT_64)) {
							det_cur_number = 0;
							saf_cur_number++;
						}

						//故障保存
						hash.unconf_fault[hash_number][cur_number] = backup.unconf_fault[hash_number][search_number];
						hash.confirm_flag[hash_number][cur_number] = -1;

						int saf_flag = bit_search_N(backup.unconf_saf_fault[hash_number][saf_number], det_number);
						if (saf_flag == 0) {
							hash.unconf_saf_fault[hash_number][saf_cur_number] = bit_setting_0(hash.unconf_saf_fault[hash_number][saf_cur_number], det_cur_number);
							hash.unconf_fault[hash_number][cur_number]->conf_fault0_flag = -1;
						}
						else if (saf_flag == 1) {
							hash.unconf_saf_fault[hash_number][saf_cur_number] = bit_setting_1(hash.unconf_saf_fault[hash_number][saf_cur_number], det_cur_number);
							hash.unconf_fault[hash_number][cur_number]->conf_fault1_flag = -1;
						}
						cur_number++;
						det_cur_number++;
						det_number++;

					}

				}

			}

			//観測ポイント挿入後-新規未識別故障ペア集合を削除
			for (int hash_number = n_grp; hash_number < hash.n_grp - n_grp; hash_number++) {
				free(hash.unconf_fault[hash_number]);
				free(hash.confirm_flag[hash_number]);
				free(hash.unconf_saf_fault[hash_number]);
			}

			hash.n_grp = n_grp;
			hash.insert_number = n_grp;
			hash.confirm_number = n_grp;

		}

	}
}


//選択候補観測ポイント配列生成関数(戻り値:選択候補観測ポイント数)
int make_tpi_select_net(void) {

	n_select_net = 0;


	//評価FFR選択
	for (int sim_ffr_number = 0; sim_ffr_number < opt.n_ffr; sim_ffr_number++) {


		int n_max_unconf_fault = 0,
			n_sim_net=0,
			max_ffr_number = -1;


		//全FFR-最大未識別故障格納個数算出
		for (int ffr_number = 0; ffr_number < n_ffr; ffr_number++) {

			if ((n_max_unconf_fault < ffr[ffr_number].n_unconf_fault)&&(ffr[ffr_number].sim_flag!=1)) {

				n_max_unconf_fault = ffr[ffr_number].n_unconf_fault;
				max_ffr_number = ffr_number;

			}

		}


		//対象FFR内未識別故障信号線保存
		for (int net_number = n_net - 1 ; net_number >= 0; net_number--) {

			if (sort_net[net_number]->ffr_id == max_ffr_number) {

				//PO,tpi_netはスルー
				if ((sort_net[net_number]->tpi_flag != 1 )&&(sort_net[net_number]->n_out!=0)) {
					queue_enqueue(sort_net[net_number]);
					n_sim_net++;
					n_select_net++;
				}

				if (n_sim_net >= opt.n_net) {
					break;
				}

			}

		}

		ffr[max_ffr_number].sim_flag = 1;

	}
	

	

	/*/実験用
	max_ffr_number = 360;
	//////////////*/



	//選択候補観測ポイント配列生成
	//tpi_select_net = (SELECT*)malloc(sizeof(SELECT) * n_select_net);
	int select_number = 0;

	while (queue_empty() == 1) {

		tpi_select_net[select_number].select_net= queue_dequeue();
		tpi_select_net[select_number].n_unconf_fault_pair = 0;

		select_number++;
	}

	if (n_select_net == 0) {
		printf("確認");
	}

	//選択候補観測ポイント配列指定要素番号初期化
	select_sim_number = 0;

	//return max_ffr_number;

	return 1;

}


//未識別故障ペア集合削除関数
void hash_backup_reset(int flag) {

	if (flag == BACKUP_SELECT) {

		int n_grp = backup.n_grp;

		for (int hash_number = 0; hash_number < n_grp; hash_number++) {

			free(backup.unconf_fault[hash_number]);
			free(backup.confirm_flag[hash_number]);
			free(backup.unconf_saf_fault[hash_number]);

		}

	}

	/*else if (flag == HASH_SELECT) {

		int n_grp = hash.n_grp;

		for (int hash_number = 0; hash_number < n_grp; hash_number++) {

			if (hash.n_unconf_fault[hash_number] != backup.n_unconf_fault[hash_number]) {

				free(hash.unconf_fault[hash_number]);
				free(hash.confirm_flag[hash_number]);
				free(hash.unconf_saf_fault[hash_number]);

			}

		}

	}*/

}


//観測ポイント挿入関数
void tpi_insert(void) {

	//最低未識別故障ペア数初期化(代入値は正の∞でも可)
	int min_unconf_fault_pair = tpi_select_net[0].n_unconf_fault_pair + 1 ,
		min_number = -1;

	//最高評価観測ポイント選択
	for (int select_number = 0; select_number < n_select_net; select_number++) {

		//最低未識別故障ペア数算出
		if (min_unconf_fault_pair > tpi_select_net[select_number].n_unconf_fault_pair) {

			min_unconf_fault_pair = tpi_select_net[select_number].n_unconf_fault_pair;
			min_number = select_number;

		}

	}

	//観測ポイント挿入
	tpi_po_net[n_tpi_po - 1] = tpi_select_net[min_number].select_net;
	tpi_net[n_tpi - 1] = tpi_select_net[min_number].select_net;
	tpi_net[n_tpi - 1]->tpi_flag = 1;

	//評価観測ポイントリセット
	for (int select_number = 0; select_number < n_select_net; select_number++) {

		tpi_select_net[select_number].select_net = NULL;
	}///////////////*/

	//free(tpi_select_net);
	//n_select_net = 0;


}
