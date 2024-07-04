#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "FFR.h"

//ハッシュ内故障探索関数
int Confirm_Fault_Search(HASH hash,DICT sim_dic,int hash_number,int fault_number);
//未識別故障ペア細分化判定関数
int* Subdivide_Judge(HASH hash,DICT sim_dic,int hash_number);


//未識別故障ペア取得関数(return 0:全検出故障の識別完了,return 1:正常終了→スコア計算)
int Confirm_Fault_Pair(HASH hash, DICT* dic) {

	int test_number, hash_number, fault_number, search_number,i;																	//ループ変数
	int n_grp, n_unconf_fault,count,insert_number, confirm_number,devide_number,cur_number,saf_flag, flag; int* devide_hash_number;	//操作変数
	NLIST* conf_fault;

	//全故障辞書配列を参照
	for (test_number = 0; test_number < n_test; test_number++) {
		
		n_grp = dic[test_number].n_grp;													//故障辞書内未識別故障集合数
		dic[test_number].hash_number = (int**)malloc(sizeof(int*)*n_grp);	

		//故障辞書内全未識別故障集合を参照
		for (hash_number = 0; hash_number < n_grp; hash_number++) {

			n_unconf_fault = dic[test_number].n_unconf_fault[hash_number];				//故障辞書内未識別故障数
			dic[test_number].hash_number[hash_number] = (int*)malloc(sizeof(int) * n_unconf_fault);
			flag = 0;																	//細分化操作フラグ初期化
			count = 0;																	//ハッシュ内故障探索成功回数初期化

			//未識別故障集合内全未識別故障を参照
			for (fault_number = 0; fault_number < n_unconf_fault; fault_number++) {

				conf_fault = dic[test_number].unconf_fault[hash_number][fault_number];
				saf_flag = dic[test_number].unconf_saf_flag[hash_number][fault_number];

				//未識別故障ペア配列内に1つでも対象故障(conf_fault)が探索できたか
				if (Confirm_Fault_Search(hash,dic[test_number],hash_number,fault_number) == 1) {

					flag = 1;
					count++;
				}

				//故障辞書内-未識別故障集合が全て探索できたか
				if (count == n_unconf_fault) {
					flag = 2;
				}

			}//未識別故障集合内-全未識別故障参照終了

			//新規未識別故障集合格納要素番号代入
			insert_number = hash.insert_number;		

			//未識別故障ペア配列内-故障探索失敗時(故障辞書内-全新規未識別故障集合を未識別故障ペア配列に挿入)**************************************************************************************************************************************************************************************************
			if (flag == 0) {

				//新規未識別故障ペア配列領域確保
				hash.unconf_fault[insert_number] = (NLIST**)malloc(sizeof(NLIST*) * n_unconf_fault);
				hash.n_unconf_fault[insert_number] = 0;
				hash.saf_flag[insert_number] = (int*)malloc(sizeof(int) * n_unconf_fault);
				hash.confirm_flag[insert_number] = (int*)malloc(sizeof(int) * n_unconf_fault);
				hash.n_confirm_flag[insert_number] = 0;

				//新規未識別故障集合挿入
				for (search_number = 0; search_number < n_unconf_fault; search_number++) {
					hash.unconf_fault[insert_number][search_number] = dic[test_number].unconf_fault[hash_number][search_number];
					hash.saf_flag[insert_number][search_number] = dic[test_number].unconf_saf_flag[hash_number][search_number];
					hash.confirm_flag[insert_number][search_number] = -1;
					hash.n_unconf_fault[insert_number]++;
				}

				//未識別故障ペア配列数,細分化故障格納要素番号,新規検出故障格納要素番号インクリメント
				hash.n_grp++;
				hash.confirm_number++;							
				hash.insert_number = hash.confirm_number;		

			}

			//未識別故障ペア配列内-故障探索成功時(故障辞書内-新規未識別故障集合and細分化未識別故障集合を全て未識別故障ペア配列に挿入)****************************************************************************************************************************************************************************
			else if (flag == 1) {

				//***************************************************************************************************************************************************
				//新規未識別故障集合のみ挿入************************************************************************************************************************
				

				//新規未識別故障ペア配列領域確保
				hash.unconf_fault[insert_number] = (NLIST**)malloc(sizeof(NLIST*) * (n_unconf_fault-count));
				hash.n_unconf_fault[insert_number] = n_unconf_fault - count;
				hash.saf_flag[insert_number] = (int*)malloc(sizeof(int) * (n_unconf_fault-count));
				hash.confirm_flag[insert_number] = (int*)malloc(sizeof(int) * (n_unconf_fault-count));
				hash.n_confirm_flag[insert_number] = 0;

				//操作変数初期化
				cur_number = 0;

				//新規未識別故障集合挿入
				for (search_number = 0; search_number < n_unconf_fault; search_number++) {

					//新規未識別故障かどうか(探索失敗時の故障かどうか)
					if (dic[test_number].hash_number[hash_number][search_number] == -1) {
						hash.unconf_fault[insert_number][cur_number] = dic[test_number].unconf_fault[hash_number][search_number];
						hash.saf_flag[insert_number][cur_number] = dic[test_number].unconf_saf_flag[hash_number][search_number];
						hash.confirm_flag[insert_number][cur_number] = -1;
						cur_number++;
					}
				}

				//未識別故障ペア配列数,細分化故障格納要素番号,新規検出故障格納要素番号インクリメント
				hash.n_grp++;
				hash.confirm_number++;
				hash.insert_number = hash.confirm_number;

				//***************************************************************************************************************************************************
				//***************************************************************************************************************************************************

				//未識別故障ペア細分化判定関数呼び出し-ハッシュ番号配列生成(終端要素は-1:終了条件)
				devide_hash_number = Subdivide_Judge(hash, dic[test_number], hash_number);


				//***************************************************************************************************************************************************
				//細分化可能未識別故障集合挿入**********************************************************************************************************************
				i = 0;
				while (devide_hash_number!=NULL) {

					//終了条件
					if (devide_hash_number[i] == -1) {
						break;
					}

					//細分化未識別故障集合格納要素番号代入
					confirm_number = hash.confirm_number;

					//ハッシュ番号,細分化未識別故障数代入
					devide_number = devide_hash_number[i];
					count = hash.n_confirm_flag[devide_number];

					//新規未識別故障ペア配列領域確保
					hash.unconf_fault[confirm_number] = (NLIST**)malloc(sizeof(NLIST*) * count);
					hash.n_unconf_fault[confirm_number] = count;
					hash.saf_flag[confirm_number] = (int*)malloc(sizeof(int) * count);
					hash.confirm_flag[confirm_number] = (int*)malloc(sizeof(int) * count);
					hash.n_confirm_flag[confirm_number] = 0;

					//操作変数初期化
					cur_number = 0;

					//細分化可能未識別故障集合挿入
					for (search_number = 0; search_number < hash.n_unconf_fault[devide_number]; search_number++) {

						//対象故障が細分化可能かどうか
						if (hash.confirm_flag[devide_number][search_number] == 1) {
							hash.unconf_fault[confirm_number][cur_number] = hash.unconf_fault[devide_number][search_number];
							hash.saf_flag[confirm_number][cur_number] = hash.saf_flag[devide_number][search_number];
							hash.confirm_flag[confirm_number][cur_number] = -1;
							hash.n_unconf_fault[confirm_number]++;
							cur_number++;

							hash.unconf_fault[devide_number][search_number] = NULL;
							hash.n_unconf_fault[devide_number]--;
						}
					}

					//未識別故障ペア配列数,細分化故障格納要素番号,新規検出故障格納要素番号インクリメント
					hash.n_grp++;
					hash.confirm_number++;
					hash.insert_number = hash.confirm_number;
					i++;
				}

				//**************************************************************************************************************************************************
				//**************************************************************************************************************************************************

			}

			//未識別故障ペア配列内-全故障探索成功時(故障辞書内-全細分化未識別故障集合を未識別故障ペア配列に挿入)***************************************************************************************************************************************************************************************************
			else if (flag == 2) {

				//未識別故障ペア細分化判定関数呼び出し-ハッシュ番号配列生成(終端要素は-1:終了条件)
				devide_hash_number = Subdivide_Judge(hash, dic[test_number], hash_number);

				i = 0;
				while (devide_hash_number != NULL) {

					//終了条件
					if (devide_hash_number[i] == -1) {
						break;
					}

					//細分化未識別故障集合格納要素番号代入
					confirm_number = hash.confirm_number;

					//ハッシュ番号,細分化未識別故障数代入
					devide_number = devide_hash_number[i];
					count = hash.n_confirm_flag[devide_number];

					//新規未識別故障ペア配列領域確保
					hash.unconf_fault[confirm_number] = (NLIST**)malloc(sizeof(NLIST*) * count);
					hash.n_unconf_fault[confirm_number] = count;
					hash.saf_flag[confirm_number] = (int*)malloc(sizeof(int) * count);
					hash.confirm_flag[confirm_number] = (int*)malloc(sizeof(int) * count);
					hash.n_confirm_flag[confirm_number] = 0;

					//操作変数初期化
					cur_number = 0;

					//細分化可能未識別故障集合挿入
					for (search_number = 0; search_number < hash.n_unconf_fault[devide_number]; search_number++) {

						//細分化フラグが有効な故障を挿入
						if (hash.confirm_flag[devide_number][search_number] == 1) {
							hash.unconf_fault[confirm_number][cur_number] = hash.unconf_fault[devide_number][search_number];
							hash.saf_flag[confirm_number][cur_number] = hash.saf_flag[devide_number][search_number];
							hash.confirm_flag[confirm_number][cur_number] = -1;
							cur_number++;

							hash.unconf_fault[devide_number][search_number] = NULL;
							hash.n_unconf_fault[devide_number]--;
						}
					}

					//未識別故障ペア配列数,細分化故障格納要素番号,新規検出故障格納要素番号インクリメント
					hash.n_grp++;
					hash.confirm_number++;
					hash.insert_number = hash.confirm_number;
					i++;
				}

			}
			
			//******************************************************************************************************************************************************************************************************************************************************************************************************
		
		}//故障辞書内-全未識別故障集合参照終了

	}//全故障辞書配列参照終了

	conf_fault_module = hash.n_grp;
	if (conf_fault_module == n_net * 2) {

		printf("\n\n全故障の識別完了\n\n");
		return 0;
	}

	printf("\n\n未識別故障ペア取得完了\n\n");
	return 1;

}


//未識別故障ペア配列内故障探索関数(return 0:探索失敗,1:探索成功)
//機能:未識別故障ペア配列内を対象故障が見つかるまで探索し、故障辞書内-hash_numberに探索した故障のハッシュ番号を代入する(探索失敗時は初期値-1のまま)
int Confirm_Fault_Search(HASH hash, DICT sim_dic, int hash_number, int fault_number) {

	//ループ変数,操作変数初期化
	int devide_number, search_number, n_grp, n_unconf_fault;
	n_grp = hash.n_grp;

	//探索対象故障の定義
	NLIST* sim_net;
	sim_net = sim_dic.unconf_fault[hash_number][fault_number];
	int sim_saf_flag = sim_dic.unconf_saf_flag[hash_number][fault_number];

	//全未識別故障ペア配列を参照
	for (devide_number = 0; devide_number < n_grp; devide_number++) {

		//ループ変数初期化
		n_unconf_fault = hash.n_unconf_fault[devide_number];

		//未識別故障ペア内-全故障を参照
		for (search_number = 0; search_number < n_unconf_fault; search_number++) {

			//故障探索成功時
			if ((hash.unconf_fault[devide_number][search_number] == sim_net) && (hash.saf_flag[devide_number][search_number] == sim_saf_flag)) {
				sim_dic.hash_number[hash_number][fault_number] = devide_number;
				hash.confirm_flag[devide_number][search_number] = 1;
				hash.n_confirm_flag[devide_number]++;
				return 1;

			}

		}

	}

	//故障探索失敗時
	sim_dic.hash_number[hash_number][fault_number] = -1;
	return 0;

}

//未識別故障ペア細分化判定関数(細分化不能:return NULL,細分化可能:return int* devide_hash_number)
//細分化不能ハッシュ番号は対象(delete_hash_number)の未識別故障ペアの細分化フラグ,細分化可能フラグ数を初期化すること。必ず！
int* Subdivide_Judge(HASH hash,DICT sim_dic,int hash_number) {

	int* devide_hash_number;			//細部化可能ハッシュ番号
	int n_devide = 0;					//細分化可能ハッシュ数
	int* delete_hash_number;			//リセットハッシュ番号
	int n_delete = 0;					//リセットハッシュ数
	int n_unconf_fault = sim_dic.n_unconf_fault[hash_number];

	int fault_number, search_number, devide_number, flag, i;	//ループ変数
	
	for (fault_number = 0; fault_number < n_unconf_fault; fault_number++) {

		devide_number = sim_dic.hash_number[hash_number][fault_number];
		flag = 1;

		for (i = 0; i < n_devide; i++) {
			//既存のハッシュ番号かどうか
			if (devide_hash_number[i] == devide_number) {
				flag = 0;
				break;
			}
		}

		//既存ハッシュ番号の時
		if (flag == 0) {
			break;
		}

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
			i = n_devide;
			n_devide++;
			devide_hash_number = (int*)realloc(devide_hash_number, sizeof(int) * n_devide);
			devide_hash_number[i] = devide_number;
		}
		//細分化不能ハッシュ番号生成,細分化フラグのリセット
		else if (flag == 0) {
			i = n_delete;
			n_delete++;
			delete_hash_number = (int*)realloc(delete_hash_number, sizeof(int) * n_delete);
			delete_hash_number[i] = devide_number;

			//細分化フラグのリセット
			int n_search = hash.n_unconf_fault[devide_number];
			for (search_number = 0; search_number < n_search; search_number++) {

				hash.confirm_flag[devide_number][search_number] = -1;
			}

			hash.n_confirm_flag[devide_number] = 0;

		}

	}

	//細分化可能ハッシュ番号が得られなかった時
	if (n_devide == 0) {
		return NULL;
	}
	//細分化可能ハッシュ番号格納配列の終端文字生成
	else if (n_devide > 0) {
		i = n_devide;
		n_devide++;
		devide_hash_number = (int*)realloc(devide_hash_number, sizeof(int) * n_devide);
		devide_hash_number[i] = -1;
		return devide_hash_number;
	}

}
