#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

//未識別故障ペア取得関数(return 0:全検出故障の識別完了,return 1:正常終了→スコア計算)
int make_Confirm_Fault_Pair(int test_number) {

	int  fault_number, search_number;																	//ループ変数
	int n_unconf_fault, insert_number, confirm_number, devide_number, cur_number, saf_flag; int* devide_hash_number;	//操作変数

	dic[test_number].hash_number = (int**)malloc(sizeof(int*) * dic[test_number].n_grp);

	//故障辞書内全未識別故障集合を参照
	for (int hash_number = 0; hash_number < dic[test_number].n_grp; hash_number++) {

		n_unconf_fault = dic[test_number].n_unconf_fault[hash_number];				//故障辞書内未識別故障数
		dic[test_number].hash_number[hash_number] = (int*)malloc(sizeof(int) * n_unconf_fault);
		int flag = 0;																	//細分化操作フラグ初期化
		int count = 0;																	//ハッシュ内故障探索成功回数初期化
		int saf_number = 0;
		int det_number = 0;
		int saf_cur_number = 0;
		int det_cur_number = 0;
		int bit_flag = count_number_64(n_unconf_fault);

		//未識別故障集合内全未識別故障を参照
		for (fault_number = 0; fault_number < n_unconf_fault; fault_number++) {

			//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
			if ((fault_number % BIT_64 == 0) && (det_number == BIT_64)) {
				det_number = 0;
				saf_number++;
			}

			//未識別故障ペア配列内に1つでも対象故障(conf_fault)が探索できたか
			if (Confirm_Fault_Search(test_number,hash_number,fault_number) == 1) {

				flag = 1;
				count++;
			}

			//故障辞書内-未識別故障集合が全て探索できたか
			if (count == n_unconf_fault) {
				flag = 2;
			}

			det_number++;

		}//未識別故障集合内-全未識別故障参照終了

		//新規未識別故障集合格納要素番号代入
		insert_number = hash.insert_number;

		//未識別故障ペア配列内-故障探索失敗時(故障辞書内-全新規未識別故障集合を未識別故障ペア配列に挿入)**************************************************************************************************************************************************************************************************
		if (flag == 0) {

			//新規未識別故障ペア配列領域確保
			hash.unconf_fault[insert_number] = (NLIST**)malloc(sizeof(NLIST*) * n_unconf_fault);
			hash.n_unconf_fault[insert_number] = 0;
			hash.n_index[insert_number] = n_unconf_fault;
			hash.confirm_flag[insert_number] = (short*)malloc(sizeof(short) * n_unconf_fault);
			hash.n_confirm_flag[insert_number] = 0;
			hash.unconf_saf_fault[insert_number] = (ULLI*)malloc(sizeof(ULLI) * bit_flag);
			for (int bit_number = 0; bit_number < bit_flag; bit_number++) {
				hash.unconf_saf_fault[insert_number][bit_number] = 0ULL;
			}

			//新規未識別故障集合挿入
			for (search_number = 0; search_number < n_unconf_fault; search_number++) {
				hash.unconf_fault[insert_number][search_number] = dic[test_number].unconf_fault[hash_number][search_number];
				hash.confirm_flag[insert_number][search_number] = -1;
				hash.n_unconf_fault[insert_number]++;
			}
			for (saf_number = 0; saf_number < bit_flag; saf_number++) {
				hash.unconf_saf_fault[insert_number][saf_number] = dic[test_number].unconf_saf_fault[hash_number][saf_number];
			}

			//未識別故障ペア配列数,細分化故障格納要素番号,新規検出故障格納要素番号インクリメント
			hash.n_grp++;
			hash.confirm_number++;
			hash.insert_number = hash.confirm_number;

		}

		//未識別故障ペア配列内-故障探索成功時(故障辞書内-新規未識別故障集合and細分化未識別故障集合を全て未識別故障ペア配列に挿入)****************************************************************************************************************************************************************************
		else if (flag == 1) {

			bit_flag = count_number_64(n_unconf_fault - count);
			//***************************************************************************************************************************************************
			//新規未識別故障集合のみ挿入************************************************************************************************************************


			//新規未識別故障ペア配列領域確保
			hash.unconf_fault[insert_number] = (NLIST**)malloc(sizeof(NLIST*) * (n_unconf_fault - count));
			hash.n_unconf_fault[insert_number] = 0;
			hash.n_index[insert_number] = n_unconf_fault - count;
			hash.confirm_flag[insert_number] = (short*)malloc(sizeof(short) * (n_unconf_fault - count));
			hash.n_confirm_flag[insert_number] = 0;
			hash.unconf_saf_fault[insert_number] = (ULLI*)malloc(sizeof(ULLI) * bit_flag);
			for (int bit_number = 0; bit_number < bit_flag; bit_number++) {
				hash.unconf_saf_fault[insert_number][bit_number] = 0ULL;
			}

			//操作変数初期化
			cur_number = saf_number = det_number = det_cur_number = saf_cur_number = 0;

			//新規未識別故障集合挿入
			for (search_number = 0; search_number < n_unconf_fault; search_number++) {

				//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
				if ((search_number % BIT_64 == 0) && (det_number == BIT_64)) {
					det_number = 0;
					saf_number++;
				}
				if ((cur_number % BIT_64 == 0) && (det_cur_number == BIT_64)) {
					det_cur_number = 0;
					saf_cur_number++;
				}

				//新規未識別故障かどうか(探索失敗時の故障かどうか)
				if (dic[test_number].hash_number[hash_number][search_number] == -1) {
					hash.unconf_fault[insert_number][cur_number] = dic[test_number].unconf_fault[hash_number][search_number];
					hash.confirm_flag[insert_number][cur_number] = -1;
					hash.n_unconf_fault[insert_number]++;
					cur_number++;

					saf_flag = bit_search_N(dic[test_number].unconf_saf_fault[hash_number][saf_number],det_number);
					if (saf_flag == 0) {
						hash.unconf_saf_fault[insert_number][saf_cur_number] = bit_setting_0(hash.unconf_saf_fault[insert_number][saf_cur_number], det_cur_number);
					}
					else {
						hash.unconf_saf_fault[insert_number][saf_cur_number] = bit_setting_1(hash.unconf_saf_fault[insert_number][saf_cur_number], det_cur_number);
					}
					det_cur_number++;
				}

				det_number++;
			}
			

			//未識別故障ペア配列数,細分化故障格納要素番号,新規検出故障格納要素番号インクリメント
			hash.n_grp++;
			hash.confirm_number++;
			hash.insert_number = hash.confirm_number;

			//***************************************************************************************************************************************************
			//***************************************************************************************************************************************************

			//未識別故障ペア細分化判定関数呼び出し-ハッシュ番号配列生成(終端要素は-1:終了条件)
			devide_hash_number = (int*)malloc(sizeof(int) * (dic[test_number].n_unconf_fault[hash_number] + 1));
			Subdivide_Judge(test_number,hash_number, devide_hash_number);

			//***************************************************************************************************************************************************
			//細分化可能未識別故障集合挿入**********************************************************************************************************************
			int devide_index = 0;
			while (devide_hash_number[devide_index] != -1) {

				//細分化未識別故障集合格納要素番号代入
				confirm_number = hash.confirm_number;

				//ハッシュ番号,細分化未識別故障数代入
				devide_number = devide_hash_number[devide_index];
				count = hash.n_confirm_flag[devide_number];
				bit_flag = count_number_64(count);

				//新規未識別故障ペア配列領域確保
				hash.unconf_fault[confirm_number] = (NLIST**)malloc(sizeof(NLIST*) * count);
				hash.n_unconf_fault[confirm_number] = 0;
				hash.confirm_flag[confirm_number] = (short*)malloc(sizeof(short) * count);
				hash.n_confirm_flag[confirm_number] = 0;
				hash.n_index[confirm_number] = count;
				hash.unconf_saf_fault[confirm_number] = (ULLI*)malloc(sizeof(ULLI) * bit_flag);
				for (int bit_number = 0; bit_number < bit_flag; bit_number++) {
					hash.unconf_saf_fault[confirm_number][bit_number] = 0ULL;
				}

				//操作変数初期化
				cur_number = saf_number = det_number = saf_cur_number = det_cur_number = 0;

				//細分化可能未識別故障集合挿入
				for (search_number = 0; search_number < hash.n_index[devide_number]; search_number++) {

					//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
					if ((search_number % BIT_64 == 0) && (det_number == BIT_64)) {
						det_number = 0;
						saf_number++;
					}
					if ((cur_number % BIT_64 == 0) && (det_cur_number == BIT_64)) {
						det_cur_number = 0;
						saf_cur_number++;
					}

					//対象故障が細分化可能かどうか
					if (hash.confirm_flag[devide_number][search_number] == 1) {
						
						//細分化故障挿入
						hash.unconf_fault[confirm_number][cur_number] = hash.unconf_fault[devide_number][search_number];
						hash.confirm_flag[confirm_number][cur_number] = -1;
						hash.n_unconf_fault[confirm_number]++;
						cur_number++;

						saf_flag = bit_search_N(hash.unconf_saf_fault[devide_number][saf_number], det_number);
						if (saf_flag == 0) {
							hash.unconf_saf_fault[confirm_number][saf_cur_number] = bit_setting_0(hash.unconf_saf_fault[confirm_number][saf_cur_number], det_cur_number);
						}
						else {
							hash.unconf_saf_fault[confirm_number][saf_cur_number] = bit_setting_1(hash.unconf_saf_fault[confirm_number][saf_cur_number], det_cur_number);
						}
						det_cur_number++;

						//被細分箇所削除
						hash.unconf_fault[devide_number][search_number] = NULL;
						hash.confirm_flag[devide_number][search_number] = -1;
						hash.n_unconf_fault[devide_number]--;
					}

					det_number++;
				}

				//未識別故障ペア配列数,細分化故障格納要素番号,新規検出故障格納要素番号インクリメント
				hash.n_grp++;
				hash.confirm_number++;
				hash.insert_number = hash.confirm_number;
				devide_index++;

				//細分化可能フラグ数リセット(hash_numberインクリメント前に必ず!)
				hash.n_confirm_flag[devide_number] = 0;

			}

			free(devide_hash_number);

			//**************************************************************************************************************************************************
			//**************************************************************************************************************************************************

		}

		//未識別故障ペア配列内-全故障探索成功時(故障辞書内-全細分化未識別故障集合を未識別故障ペア配列に挿入)***************************************************************************************************************************************************************************************************
		else if (flag == 2) {

			//未識別故障ペア細分化判定関数呼び出し-ハッシュ番号配列生成(終端要素は-1:終了条件)
			devide_hash_number = (int*)malloc(sizeof(int) * (dic[test_number].n_unconf_fault[hash_number] + 1));
			Subdivide_Judge(test_number,hash_number, devide_hash_number);

			int devide_index = 0;
			while (devide_hash_number[devide_index] != -1) {

				//細分化未識別故障集合格納要素番号代入
				confirm_number = hash.confirm_number;

				//ハッシュ番号,細分化未識別故障数代入
				devide_number = devide_hash_number[devide_index];
				count = hash.n_confirm_flag[devide_number];
				bit_flag = count_number_64(count);

				//新規未識別故障ペア配列領域確保
				hash.unconf_fault[confirm_number] = (NLIST**)malloc(sizeof(NLIST*) * count);
				hash.n_unconf_fault[confirm_number] = 0;
				hash.confirm_flag[confirm_number] = (short*)malloc(sizeof(short) * count);
				hash.n_confirm_flag[confirm_number] = 0;
				hash.n_index[confirm_number] = count;
				hash.unconf_saf_fault[confirm_number] = (ULLI*)malloc(sizeof(ULLI) * bit_flag);
				for (int bit_number = 0; bit_number < bit_flag; bit_number++) {
					hash.unconf_saf_fault[confirm_number][bit_number] = 0ULL;
				}

				//操作変数初期化
				cur_number = det_number = saf_number = det_cur_number = saf_cur_number = 0;

				//細分化可能未識別故障集合挿入
				for (search_number = 0; search_number < hash.n_index[devide_number]; search_number++) {

					//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
					if ((search_number % BIT_64 == 0) && (det_number == BIT_64)) {
						det_number = 0;
						saf_number++;
					}
					if ((cur_number % BIT_64 == 0) && (det_cur_number == BIT_64)) {
						det_cur_number = 0;
						saf_cur_number++;
					}

					//細分化フラグが有効な故障を挿入
					if (hash.confirm_flag[devide_number][search_number] == 1) {

						//細分化故障挿入
						hash.unconf_fault[confirm_number][cur_number] = hash.unconf_fault[devide_number][search_number];
						hash.confirm_flag[confirm_number][cur_number] = -1;
						hash.n_unconf_fault[confirm_number]++;
						cur_number++;

						saf_flag = bit_search_N(hash.unconf_saf_fault[devide_number][saf_number], det_number);
						if (saf_flag == 0) {
							hash.unconf_saf_fault[confirm_number][saf_cur_number] = bit_setting_0(hash.unconf_saf_fault[confirm_number][saf_cur_number], det_cur_number);
						}
						else {
							hash.unconf_saf_fault[confirm_number][saf_cur_number] = bit_setting_1(hash.unconf_saf_fault[confirm_number][saf_cur_number], det_cur_number);
						}
						det_cur_number++;

						//被細分箇所削除
						hash.unconf_fault[devide_number][search_number] = NULL;
						hash.confirm_flag[devide_number][search_number] = -1;
						hash.n_unconf_fault[devide_number]--;
					}

					det_number++;
				}

				//未識別故障ペア配列数,細分化故障格納要素番号,新規検出故障格納要素番号インクリメント
				hash.n_grp++;
				hash.confirm_number++;
				hash.insert_number = hash.confirm_number;
				devide_index++;

				//細分化可能フラグ数リセット(hash_numberインクリメント前に必ず!)
				hash.n_confirm_flag[devide_number] = 0;
			}

			free(devide_hash_number);

		}

		//******************************************************************************************************************************************************************************************************************************************************************************************************

	}//故障辞書内-全未識別故障集合参照終了

	//printf("\n************tp%d**********************************\n", test_number);

	for (int hash_number = 0; hash_number < hash.n_grp; hash_number++) {

		if ((hash.n_unconf_fault[hash_number] == 1) && (hash.n_index[hash_number] >= 2)) {

			NLIST* net_temp; int saf_temp; int saf_number = 0; int det_number = 0;
			for (int reset_number = 0; reset_number < hash.n_index[hash_number]; reset_number++) {
				
				if ((reset_number % BIT_64 == 0) && (det_number == BIT_64)) {
					det_number = 0;
					saf_number++;
				}

				if (hash.unconf_fault[hash_number][reset_number] != NULL) {
					hash.n_index[hash_number] = 1;
					net_temp = hash.unconf_fault[hash_number][reset_number];
					hash.unconf_fault[hash_number][reset_number] = NULL;
					//free(hash.unconf_fault[hash_number]);
					//hash.unconf_fault[hash_number] = (NLIST**)malloc(sizeof(NLIST*)*1);
					hash.unconf_fault[hash_number][0] = net_temp;
					saf_temp = bit_search_N(hash.unconf_saf_fault[hash_number][saf_number], det_number);
					//free(hash.unconf_saf_fault[hash_number]);
					//hash.unconf_saf_fault[hash_number] = (ULLI*)malloc(sizeof(ULLI)*1);
					//hash.unconf_saf_fault[hash_number][0] = 0ULL;
					if (saf_temp == 1) {
						hash.unconf_saf_fault[hash_number][0] = bit_setting_1(hash.unconf_saf_fault[hash_number][0], 0);
						net_temp->conf_fault1_flag = 1;
					}
					else {
						hash.unconf_saf_fault[hash_number][0] = bit_setting_0(hash.unconf_saf_fault[hash_number][0], 0);
						net_temp->conf_fault0_flag = 1;
					}
					//free(hash.confirm_flag[hash_number]);
					//hash.confirm_flag[hash_number] = (short*)malloc(sizeof(short)*1);
					hash.confirm_flag[hash_number][0] = -1;
					break;
				}

				det_number++;
			}
		}

		if (hash.n_unconf_fault[hash_number] == 1) {

			int saf_temp;
			saf_temp = bit_search_N(hash.unconf_saf_fault[hash_number][0], 0);

			if (saf_temp == 1) {
				hash.unconf_fault[hash_number][0]->conf_fault1_flag = 1;

			}
			else if(saf_temp==0){
				hash.unconf_fault[hash_number][0]->conf_fault0_flag = 1;
			}

		}

		/*/確認用
		if (hash.n_unconf_fault[hash_number] >=2) {

			printf("\n***Unconf_Fault_Pair%d***\n", hash_number);

			int det_number = 0; int saf_number = 0;
			for (int fault_number = 0; fault_number < hash.n_index[hash_number]; fault_number++) {

				//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
				if ((fault_number % BIT_64 == 0) && (det_number == BIT_64)) {
					det_number = 0;
					saf_number++;
				}

				if (hash.unconf_fault[hash_number][fault_number] != NULL) {

					saf_flag = bit_search_N(hash.unconf_saf_fault[hash_number][saf_number], det_number);

					if (saf_flag == 0) {
						printf("s-a-0\t%s\n", hash.unconf_fault[hash_number][fault_number]->name);
						//printf("net_number=%d\n", hash.unconf_fault[hash_number][fault_number]->sort_n);
						//printf("level=%d\n", hash.unconf_fault[hash_number][fault_number]->level);
						//printf("FFR_ID=%d\n", hash.unconf_fault[hash_number][fault_number]->ffr_id);

					}
					else {
						printf("s-a-1\t%s\n", hash.unconf_fault[hash_number][fault_number]->name);
						//printf("net_number=%d\n", hash.unconf_fault[hash_number][fault_number]->sort_n);
						//printf("level=%d\n", hash.unconf_fault[hash_number][fault_number]->level);
						//printf("FFR_ID=%d\n", hash.unconf_fault[hash_number][fault_number]->ffr_id);
					}

				}

				det_number++;

			}

		}///////////////////////////////////*/

		

	}

	conf_fault_module = hash.n_grp;

	//対象テストパターンの故障辞書削除
	hash_reset_DICT(test_number);

	//全未識別故障ペア識別完了
	if (n_sim_fault != 0) {
		if (conf_fault_module >= n_sim_fault) {

			return 0;
		}
	}

	return 1;

}

//未識別故障ペア配列内故障探索関数(return 0:探索失敗,1:探索成功)
//機能:未識別故障ペア配列内を対象故障が見つかるまで探索し、故障辞書内-hash_numberに探索した故障のハッシュ番号を代入する(探索失敗時は初期値-1のまま)
int Confirm_Fault_Search(int test_number,int hash_number,int fault_number) {

	//ループ変数,操作変数初期化
	int n_grp, n_index, saf_number, det_number, saf_cur_number, det_cur_number,saf_flag;
	n_grp = hash.n_grp;
	saf_number = fault_number / BIT_64;
	//saf_number = count_number_64(fault_number);
	det_number = fault_number % BIT_64;

	//探索対象故障の定義
	NLIST* sim_net; int sim_saf;
	sim_net = dic[test_number].unconf_fault[hash_number][fault_number];
	sim_saf = bit_search_N(dic[test_number].unconf_saf_fault[hash_number][saf_number], det_number);

	//全未識別故障ペア配列を参照
	for (int devide_number = 0; devide_number < n_grp; devide_number++) {

		//ループ変数初期化
		n_index = hash.n_index[devide_number];
		saf_cur_number = det_cur_number = 0;

		//未識別故障ペア内-全故障を参照
		for (int search_number = 0; search_number < n_index; search_number++) {

			//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
			if ((search_number % BIT_64 == 0) && (det_cur_number == BIT_64)) {
				det_cur_number = 0;
				saf_cur_number++;
			}

			saf_flag = bit_search_N(hash.unconf_saf_fault[devide_number][saf_cur_number], det_cur_number);

			//故障探索成功時
			if ((hash.unconf_fault[devide_number][search_number] == sim_net) && (saf_flag == sim_saf)) {
				dic[test_number].hash_number[hash_number][fault_number] = devide_number;
				hash.confirm_flag[devide_number][search_number] = 1;
				hash.n_confirm_flag[devide_number]++;
				return 1;

			}

			det_cur_number++;

		}

	}

	//故障探索失敗時
	dic[test_number].hash_number[hash_number][fault_number] = -1;
	return 0;

}

//未識別故障ペア細分化判定関数
//機能:細分化可能な未識別故障ペアが格納されているハッシュ番号を生成する
void Subdivide_Judge(int test_number,int hash_number, int* devide_hash_number) {

	int n_devide = 0;					//細分化可能ハッシュ数
	int* delete_hash_number;			//リセットハッシュ番号
	int n_delete = 0;					//リセットハッシュ数
	int n_unconf_fault = dic[test_number].n_unconf_fault[hash_number];
	delete_hash_number = (int*)malloc(sizeof(int) * (n_unconf_fault + 1));

	int search_number, devide_number, flag;	//ループ変数

	for (int fault_number = 0; fault_number < n_unconf_fault; fault_number++) {

		devide_number = dic[test_number].hash_number[hash_number][fault_number];
		flag = 1;

		//参照ハッシュ番号が存在しない時(新規検出故障であるため細分化不能)
		if (devide_number == -1) {
			flag = 0;
		}

		for (int i = 0; i < n_devide; i++) {
			//既存のハッシュ番号かどうか
			if (devide_hash_number[i] == devide_number) {
				flag = 0;
				break;
			}
		}

		for (int i = 0; i < n_delete; i++) {
			if (delete_hash_number[i] == devide_number) {
				flag = 0;
				break;
			}
		}

		//新規ハッシュ番号の時
		if (flag != 0) {

			//細分化可能フラグ数が未識別故障数より少ない時(細分化可能な時)
			if (hash.n_confirm_flag[devide_number] < hash.n_unconf_fault[devide_number]) {
				flag = 1;
			}
			//細分化可能フラグ数が未識別故障数に達している時(細分化不能な時→全フラグをリセット)
			else {
				flag = 0;
			}

			//細分化可能ハッシュ番号生成
			if (flag == 1) {
				devide_hash_number[n_devide] = devide_number;
				n_devide++;
			}
			//細分化不能ハッシュ番号生成,細分化フラグのリセット
			else if (flag == 0) {
				delete_hash_number[n_delete] = devide_number;
				n_delete++;

				//細分化フラグのリセット
				int n_search = hash.n_index[devide_number];
				for (search_number = 0; search_number < n_search; search_number++) {

					hash.confirm_flag[devide_number][search_number] = -1;
				}

				hash.n_confirm_flag[devide_number] = 0;

			}

		}

	}

	free(delete_hash_number);

	//細分化可能ハッシュ番号が得られなかった時
	if (n_devide == 0) {
		devide_hash_number[0] = -1;
	}
	//細分化可能ハッシュ番号格納配列の終端文字生成
	else if (n_devide > 0) {
		devide_hash_number[n_devide] = -1;
	}

}


#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "FFR.h"
#include "HASH.h"

//未識別故障ペア取得関数(return 0:全検出故障の識別完了,return 1:正常終了→スコア計算)
int make_Confirm_Fault_Pair(int test_number) {

	int  fault_number, search_number;																	//ループ変数
	int n_unconf_fault, insert_number, confirm_number, devide_number, cur_number, saf_flag; int* devide_hash_number;	//操作変数

	dic[test_number].hash_number = (int**)malloc(sizeof(int*) * dic[test_number].n_grp);

	//故障辞書内全未識別故障集合を参照
	for (int hash_number = 0; hash_number < dic[test_number].n_grp; hash_number++) {

		n_unconf_fault = dic[test_number].n_unconf_fault[hash_number];				//故障辞書内未識別故障数
		dic[test_number].hash_number[hash_number] = (int*)malloc(sizeof(int) * n_unconf_fault);
		int flag = 0;																	//細分化操作フラグ初期化
		int count = 0;																	//ハッシュ内故障探索成功回数初期化
		int saf_number = 0;
		int det_number = 0;
		int saf_cur_number = 0;
		int det_cur_number = 0;
		int bit_flag = count_number_64(n_unconf_fault);

		//未識別故障集合内全未識別故障を参照
		for (fault_number = 0; fault_number < n_unconf_fault; fault_number++) {

			//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
			if ((fault_number % BIT_64 == 0) && (det_number == BIT_64)) {
				det_number = 0;
				saf_number++;
			}

			//未識別故障ペア配列内に1つでも対象故障(conf_fault)が探索できたか
			if (Confirm_Fault_Search(test_number,hash_number,fault_number) == 1) {

				flag = 1;
				count++;
			}

			//故障辞書内-未識別故障集合が全て探索できたか
			if (count == n_unconf_fault) {
				flag = 2;
			}

			det_number++;

		}//未識別故障集合内-全未識別故障参照終了

		//新規未識別故障集合格納要素番号代入
		insert_number = hash.insert_number;

		//未識別故障ペア配列内-故障探索失敗時(故障辞書内-全新規未識別故障集合を未識別故障ペア配列に挿入)**************************************************************************************************************************************************************************************************
		if (flag == 0) {

			//新規未識別故障ペア配列領域確保
			hash.unconf_fault[insert_number] = (NLIST**)malloc(sizeof(NLIST*) * n_unconf_fault);
			hash.n_unconf_fault[insert_number] = 0;
			hash.n_index[insert_number] = n_unconf_fault;
			hash.confirm_flag[insert_number] = (short*)malloc(sizeof(short) * n_unconf_fault);
			hash.n_confirm_flag[insert_number] = 0;
			hash.unconf_saf_fault[insert_number] = (ULLI*)malloc(sizeof(ULLI) * bit_flag);
			for (int bit_number = 0; bit_number < bit_flag; bit_number++) {
				hash.unconf_saf_fault[insert_number][bit_number] = 0ULL;
			}

			//新規未識別故障集合挿入
			for (search_number = 0; search_number < n_unconf_fault; search_number++) {
				hash.unconf_fault[insert_number][search_number] = dic[test_number].unconf_fault[hash_number][search_number];
				hash.confirm_flag[insert_number][search_number] = -1;
				hash.n_unconf_fault[insert_number]++;
			}
			for (saf_number = 0; saf_number < bit_flag; saf_number++) {
				hash.unconf_saf_fault[insert_number][saf_number] = dic[test_number].unconf_saf_fault[hash_number][saf_number];
			}

			//未識別故障ペア配列数,細分化故障格納要素番号,新規検出故障格納要素番号インクリメント
			hash.n_grp++;
			hash.confirm_number++;
			hash.insert_number = hash.confirm_number;

		}

		//未識別故障ペア配列内-故障探索成功時(故障辞書内-新規未識別故障集合and細分化未識別故障集合を全て未識別故障ペア配列に挿入)****************************************************************************************************************************************************************************
		else if (flag == 1) {

			bit_flag = count_number_64(n_unconf_fault - count);
			//***************************************************************************************************************************************************
			//新規未識別故障集合のみ挿入************************************************************************************************************************


			//新規未識別故障ペア配列領域確保
			hash.unconf_fault[insert_number] = (NLIST**)malloc(sizeof(NLIST*) * (n_unconf_fault - count));
			hash.n_unconf_fault[insert_number] = 0;
			hash.n_index[insert_number] = n_unconf_fault - count;
			hash.confirm_flag[insert_number] = (short*)malloc(sizeof(short) * (n_unconf_fault - count));
			hash.n_confirm_flag[insert_number] = 0;
			hash.unconf_saf_fault[insert_number] = (ULLI*)malloc(sizeof(ULLI) * bit_flag);
			for (int bit_number = 0; bit_number < bit_flag; bit_number++) {
				hash.unconf_saf_fault[insert_number][bit_number] = 0ULL;
			}

			//操作変数初期化
			cur_number = saf_number = det_number = det_cur_number = saf_cur_number = 0;

			//新規未識別故障集合挿入
			for (search_number = 0; search_number < n_unconf_fault; search_number++) {

				//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
				if ((search_number % BIT_64 == 0) && (det_number == BIT_64)) {
					det_number = 0;
					saf_number++;
				}
				if ((cur_number % BIT_64 == 0) && (det_cur_number == BIT_64)) {
					det_cur_number = 0;
					saf_cur_number++;
				}

				//新規未識別故障かどうか(探索失敗時の故障かどうか)
				if (dic[test_number].hash_number[hash_number][search_number] == -1) {
					hash.unconf_fault[insert_number][cur_number] = dic[test_number].unconf_fault[hash_number][search_number];
					hash.confirm_flag[insert_number][cur_number] = -1;
					hash.n_unconf_fault[insert_number]++;
					cur_number++;

					saf_flag = bit_search_N(dic[test_number].unconf_saf_fault[hash_number][saf_number],det_number);
					if (saf_flag == 0) {
						hash.unconf_saf_fault[insert_number][saf_cur_number] = bit_setting_0(hash.unconf_saf_fault[insert_number][saf_cur_number], det_cur_number);
					}
					else {
						hash.unconf_saf_fault[insert_number][saf_cur_number] = bit_setting_1(hash.unconf_saf_fault[insert_number][saf_cur_number], det_cur_number);
					}
					det_cur_number++;
				}

				det_number++;
			}
			

			//未識別故障ペア配列数,細分化故障格納要素番号,新規検出故障格納要素番号インクリメント
			hash.n_grp++;
			hash.confirm_number++;
			hash.insert_number = hash.confirm_number;

			//***************************************************************************************************************************************************
			//***************************************************************************************************************************************************

			//未識別故障ペア細分化判定関数呼び出し-ハッシュ番号配列生成(終端要素は-1:終了条件)
			devide_hash_number = (int*)malloc(sizeof(int) * (dic[test_number].n_unconf_fault[hash_number] + 1));
			Subdivide_Judge(test_number,hash_number, devide_hash_number);

			//***************************************************************************************************************************************************
			//細分化可能未識別故障集合挿入**********************************************************************************************************************
			int devide_index = 0;
			while (devide_hash_number[devide_index] != -1) {

				//細分化未識別故障集合格納要素番号代入
				confirm_number = hash.confirm_number;

				//ハッシュ番号,細分化未識別故障数代入
				devide_number = devide_hash_number[devide_index];
				count = hash.n_confirm_flag[devide_number];
				bit_flag = count_number_64(count);

				//新規未識別故障ペア配列領域確保
				hash.unconf_fault[confirm_number] = (NLIST**)malloc(sizeof(NLIST*) * count);
				hash.n_unconf_fault[confirm_number] = 0;
				hash.confirm_flag[confirm_number] = (short*)malloc(sizeof(short) * count);
				hash.n_confirm_flag[confirm_number] = 0;
				hash.n_index[confirm_number] = count;
				hash.unconf_saf_fault[confirm_number] = (ULLI*)malloc(sizeof(ULLI) * bit_flag);
				for (int bit_number = 0; bit_number < bit_flag; bit_number++) {
					hash.unconf_saf_fault[confirm_number][bit_number] = 0ULL;
				}

				//操作変数初期化
				cur_number = saf_number = det_number = saf_cur_number = det_cur_number = 0;

				//細分化可能未識別故障集合挿入
				for (search_number = 0; search_number < hash.n_index[devide_number]; search_number++) {

					//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
					if ((search_number % BIT_64 == 0) && (det_number == BIT_64)) {
						det_number = 0;
						saf_number++;
					}
					if ((cur_number % BIT_64 == 0) && (det_cur_number == BIT_64)) {
						det_cur_number = 0;
						saf_cur_number++;
					}

					//対象故障が細分化可能かどうか
					if (hash.confirm_flag[devide_number][search_number] == 1) {
						
						//細分化故障挿入
						hash.unconf_fault[confirm_number][cur_number] = hash.unconf_fault[devide_number][search_number];
						hash.confirm_flag[confirm_number][cur_number] = -1;
						hash.n_unconf_fault[confirm_number]++;
						cur_number++;

						saf_flag = bit_search_N(hash.unconf_saf_fault[devide_number][saf_number], det_number);
						if (saf_flag == 0) {
							hash.unconf_saf_fault[confirm_number][saf_cur_number] = bit_setting_0(hash.unconf_saf_fault[confirm_number][saf_cur_number], det_cur_number);
						}
						else {
							hash.unconf_saf_fault[confirm_number][saf_cur_number] = bit_setting_1(hash.unconf_saf_fault[confirm_number][saf_cur_number], det_cur_number);
						}
						det_cur_number++;

						//被細分箇所削除
						hash.unconf_fault[devide_number][search_number] = NULL;
						hash.confirm_flag[devide_number][search_number] = -1;
						hash.n_unconf_fault[devide_number]--;
					}

					det_number++;
				}

				//未識別故障ペア配列数,細分化故障格納要素番号,新規検出故障格納要素番号インクリメント
				hash.n_grp++;
				hash.confirm_number++;
				hash.insert_number = hash.confirm_number;
				devide_index++;

				//細分化可能フラグ数リセット(hash_numberインクリメント前に必ず!)
				hash.n_confirm_flag[devide_number] = 0;

			}

			free(devide_hash_number);

			//**************************************************************************************************************************************************
			//**************************************************************************************************************************************************

		}

		//未識別故障ペア配列内-全故障探索成功時(故障辞書内-全細分化未識別故障集合を未識別故障ペア配列に挿入)***************************************************************************************************************************************************************************************************
		else if (flag == 2) {

			//未識別故障ペア細分化判定関数呼び出し-ハッシュ番号配列生成(終端要素は-1:終了条件)
			devide_hash_number = (int*)malloc(sizeof(int) * (dic[test_number].n_unconf_fault[hash_number] + 1));
			Subdivide_Judge(test_number,hash_number, devide_hash_number);

			int devide_index = 0;
			while (devide_hash_number[devide_index] != -1) {

				//細分化未識別故障集合格納要素番号代入
				confirm_number = hash.confirm_number;

				//ハッシュ番号,細分化未識別故障数代入
				devide_number = devide_hash_number[devide_index];
				count = hash.n_confirm_flag[devide_number];
				bit_flag = count_number_64(count);

				//新規未識別故障ペア配列領域確保
				hash.unconf_fault[confirm_number] = (NLIST**)malloc(sizeof(NLIST*) * count);
				hash.n_unconf_fault[confirm_number] = 0;
				hash.confirm_flag[confirm_number] = (short*)malloc(sizeof(short) * count);
				hash.n_confirm_flag[confirm_number] = 0;
				hash.n_index[confirm_number] = count;
				hash.unconf_saf_fault[confirm_number] = (ULLI*)malloc(sizeof(ULLI) * bit_flag);
				for (int bit_number = 0; bit_number < bit_flag; bit_number++) {
					hash.unconf_saf_fault[confirm_number][bit_number] = 0ULL;
				}

				//操作変数初期化
				cur_number = det_number = saf_number = det_cur_number = saf_cur_number = 0;

				//細分化可能未識別故障集合挿入
				for (search_number = 0; search_number < hash.n_index[devide_number]; search_number++) {

					//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
					if ((search_number % BIT_64 == 0) && (det_number == BIT_64)) {
						det_number = 0;
						saf_number++;
					}
					if ((cur_number % BIT_64 == 0) && (det_cur_number == BIT_64)) {
						det_cur_number = 0;
						saf_cur_number++;
					}

					//細分化フラグが有効な故障を挿入
					if (hash.confirm_flag[devide_number][search_number] == 1) {

						//細分化故障挿入
						hash.unconf_fault[confirm_number][cur_number] = hash.unconf_fault[devide_number][search_number];
						hash.confirm_flag[confirm_number][cur_number] = -1;
						hash.n_unconf_fault[confirm_number]++;
						cur_number++;

						saf_flag = bit_search_N(hash.unconf_saf_fault[devide_number][saf_number], det_number);
						if (saf_flag == 0) {
							hash.unconf_saf_fault[confirm_number][saf_cur_number] = bit_setting_0(hash.unconf_saf_fault[confirm_number][saf_cur_number], det_cur_number);
						}
						else {
							hash.unconf_saf_fault[confirm_number][saf_cur_number] = bit_setting_1(hash.unconf_saf_fault[confirm_number][saf_cur_number], det_cur_number);
						}
						det_cur_number++;

						//被細分箇所削除
						hash.unconf_fault[devide_number][search_number] = NULL;
						hash.confirm_flag[devide_number][search_number] = -1;
						hash.n_unconf_fault[devide_number]--;
					}

					det_number++;
				}

				//未識別故障ペア配列数,細分化故障格納要素番号,新規検出故障格納要素番号インクリメント
				hash.n_grp++;
				hash.confirm_number++;
				hash.insert_number = hash.confirm_number;
				devide_index++;

				//細分化可能フラグ数リセット(hash_numberインクリメント前に必ず!)
				hash.n_confirm_flag[devide_number] = 0;
			}

			free(devide_hash_number);

		}

		//******************************************************************************************************************************************************************************************************************************************************************************************************

	}//故障辞書内-全未識別故障集合参照終了

	//printf("\n************tp%d**********************************\n", test_number);

	for (int hash_number = 0; hash_number < hash.n_grp; hash_number++) {

		if ((hash.n_unconf_fault[hash_number] == 1) && (hash.n_index[hash_number] >= 2)) {

			NLIST* net_temp; int saf_temp; int saf_number = 0; int det_number = 0;
			for (int reset_number = 0; reset_number < hash.n_index[hash_number]; reset_number++) {
				
				if ((reset_number % BIT_64 == 0) && (det_number == BIT_64)) {
					det_number = 0;
					saf_number++;
				}

				if (hash.unconf_fault[hash_number][reset_number] != NULL) {
					hash.n_index[hash_number] = 1;
					net_temp = hash.unconf_fault[hash_number][reset_number];
					free(hash.unconf_fault[hash_number]);
					hash.unconf_fault[hash_number] = (NLIST**)malloc(sizeof(NLIST*));
					hash.unconf_fault[hash_number][0] = net_temp;
					saf_temp = bit_search_N(hash.unconf_saf_fault[hash_number][saf_number], det_number);
					free(hash.unconf_saf_fault[hash_number]);
					hash.unconf_saf_fault[hash_number] = (ULLI*)malloc(sizeof(ULLI));
					hash.unconf_saf_fault[hash_number][0] = 0ULL;
					if (saf_temp == 1) {
						hash.unconf_saf_fault[hash_number][0] = bit_setting_1(hash.unconf_saf_fault[hash_number][0], 0);
						net_temp->conf_fault1_flag = 1;
					}
					else {
						hash.unconf_saf_fault[hash_number][0] = bit_setting_0(hash.unconf_saf_fault[hash_number][0], 0);
						net_temp->conf_fault0_flag = 1;
					}
					free(hash.confirm_flag[hash_number]);
					hash.confirm_flag[hash_number] = (short*)malloc(sizeof(short));
					hash.confirm_flag[hash_number][0] = -1;
					break;
				}

				det_number++;
			}
		}

		if (hash.n_unconf_fault[hash_number] == 1) {

			int saf_temp;
			saf_temp = bit_search_N(hash.unconf_saf_fault[hash_number][0], 0);

			if (saf_temp == 1) {
				hash.unconf_fault[hash_number][0]->conf_fault1_flag = 1;
			}
			else if(saf_temp==0){
				hash.unconf_fault[hash_number][0]->conf_fault0_flag = 1;
			}

		}

		//確認用
		if (hash.n_unconf_fault[hash_number] >=2) {

			//printf("\n***Unconf_Fault_Pair%d***\n", hash_number);

			int det_number = 0; int saf_number = 0;
			for (int fault_number = 0; fault_number < hash.n_index[hash_number]; fault_number++) {

				//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
				if ((fault_number % BIT_64 == 0) && (det_number == BIT_64)) {
					det_number = 0;
					saf_number++;
				}

				if (hash.unconf_fault[hash_number][fault_number] != NULL) {

					saf_flag = bit_search_N(hash.unconf_saf_fault[hash_number][saf_number], det_number);

					if (saf_flag == 0) {
						//printf("s-a-0\t%s\n", hash.unconf_fault[hash_number][fault_number]->name);
						//printf("net_number=%d\n", hash.unconf_fault[hash_number][fault_number]->sort_n);
						//printf("level=%d\n", hash.unconf_fault[hash_number][fault_number]->level);
						//printf("FFR_ID=%d\n", hash.unconf_fault[hash_number][fault_number]->ffr_id);
						hash.unconf_fault[hash_number][fault_number]->conf_fault0_flag = -1;

					}
					else {
						//printf("s-a-1\t%s\n", hash.unconf_fault[hash_number][fault_number]->name);
						//printf("net_number=%d\n", hash.unconf_fault[hash_number][fault_number]->sort_n);
						//printf("level=%d\n", hash.unconf_fault[hash_number][fault_number]->level);
						//printf("FFR_ID=%d\n", hash.unconf_fault[hash_number][fault_number]->ffr_id);
						hash.unconf_fault[hash_number][fault_number]->conf_fault1_flag = -1;
					}

				}

				det_number++;

			}

		}///////////////////////////////////*/

		

	}

	conf_fault_module = hash.n_grp;

	//対象テストパターンの故障辞書削除
	hash_reset_DICT(test_number);

	//全未識別故障ペア識別完了
	if (n_sim_fault != 0) {
		if (conf_fault_module == n_sim_fault) {

			return 0;
		}
	}

	return 1;

}

//未識別故障ペア配列内故障探索関数(return 0:探索失敗,1:探索成功)
//機能:未識別故障ペア配列内を対象故障が見つかるまで探索し、故障辞書内-hash_numberに探索した故障のハッシュ番号を代入する(探索失敗時は初期値-1のまま)
int Confirm_Fault_Search(int test_number,int hash_number,int fault_number) {

	//ループ変数,操作変数初期化
	int n_grp, n_index, saf_number, det_number, saf_cur_number, det_cur_number,saf_flag;
	n_grp = hash.n_grp;
	saf_number = fault_number / BIT_64;
	//saf_number = count_number_64(fault_number);
	det_number = fault_number % BIT_64;

	//探索対象故障の定義
	NLIST* sim_net; int sim_saf;
	sim_net = dic[test_number].unconf_fault[hash_number][fault_number];
	sim_saf = bit_search_N(dic[test_number].unconf_saf_fault[hash_number][saf_number], det_number);

	//全未識別故障ペア配列を参照
	for (int devide_number = 0; devide_number < n_grp; devide_number++) {

		//ループ変数初期化
		n_index = hash.n_index[devide_number];
		saf_cur_number = det_cur_number = 0;

		//未識別故障ペア内-全故障を参照
		for (int search_number = 0; search_number < n_index; search_number++) {

			//ULLI配列(unconf_saf_fault)の末端要素を超えたらループ変数修正
			if ((search_number % BIT_64 == 0) && (det_cur_number == BIT_64)) {
				det_cur_number = 0;
				saf_cur_number++;
			}

			saf_flag = bit_search_N(hash.unconf_saf_fault[devide_number][saf_cur_number], det_cur_number);

			//故障探索成功時
			if ((hash.unconf_fault[devide_number][search_number] == sim_net) && (saf_flag == sim_saf)) {
				dic[test_number].hash_number[hash_number][fault_number] = devide_number;
				hash.confirm_flag[devide_number][search_number] = 1;
				hash.n_confirm_flag[devide_number]++;
				return 1;

			}

			det_cur_number++;

		}

	}

	//故障探索失敗時
	dic[test_number].hash_number[hash_number][fault_number] = -1;
	return 0;

}

//未識別故障ペア細分化判定関数
//機能:細分化可能な未識別故障ペアが格納されているハッシュ番号を生成する
void Subdivide_Judge(int test_number,int hash_number, int* devide_hash_number) {

	int n_devide = 0;					//細分化可能ハッシュ数
	int* delete_hash_number;			//リセットハッシュ番号
	int n_delete = 0;					//リセットハッシュ数
	int n_unconf_fault = dic[test_number].n_unconf_fault[hash_number];
	delete_hash_number = (int*)malloc(sizeof(int) * (n_unconf_fault + 1));

	int search_number, devide_number, flag;	//ループ変数

	for (int fault_number = 0; fault_number < n_unconf_fault; fault_number++) {

		devide_number = dic[test_number].hash_number[hash_number][fault_number];
		flag = 1;

		//参照ハッシュ番号が存在しない時(新規検出故障であるため細分化不能)
		if (devide_number == -1) {
			flag = 0;
		}

		for (int i = 0; i < n_devide; i++) {
			//既存のハッシュ番号かどうか
			if (devide_hash_number[i] == devide_number) {
				flag = 0;
				break;
			}
		}

		for (int i = 0; i < n_delete; i++) {
			if (delete_hash_number[i] == devide_number) {
				flag = 0;
				break;
			}
		}

		//新規ハッシュ番号の時
		if (flag != 0) {

			//細分化可能フラグ数が未識別故障数より少ない時(細分化可能な時)
			if (hash.n_confirm_flag[devide_number] < hash.n_unconf_fault[devide_number]) {
				flag = 1;
			}
			//細分化可能フラグ数が未識別故障数に達している時(細分化不能な時→全フラグをリセット)
			else {
				flag = 0;
			}

			//細分化可能ハッシュ番号生成
			if (flag == 1) {
				devide_hash_number[n_devide] = devide_number;
				n_devide++;
			}
			//細分化不能ハッシュ番号生成,細分化フラグのリセット
			else if (flag == 0) {
				delete_hash_number[n_delete] = devide_number;
				n_delete++;

				//細分化フラグのリセット
				int n_search = hash.n_index[devide_number];
				for (search_number = 0; search_number < n_search; search_number++) {

					hash.confirm_flag[devide_number][search_number] = -1;
				}

				hash.n_confirm_flag[devide_number] = 0;

			}

		}

	}

	free(delete_hash_number);

	//細分化可能ハッシュ番号が得られなかった時
	if (n_devide == 0) {
		devide_hash_number[0] = -1;
	}
	//細分化可能ハッシュ番号格納配列の終端文字生成
	else if (n_devide > 0) {
		devide_hash_number[n_devide] = -1;
	}

}

