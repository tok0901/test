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


int TPI_Score_Gate(NLIST* sim_fault,int flag);
int TPI_Score_Total(void);
int TPI_Score_Reset(int flag);
int TPI_Points_Count(void);
double alpha = 0.9; double main_fault_score = 10.0;


//TPIスコア計算関数(未識別故障集合格納配列を対象)
int TPI_Score_Calc(NLIST** sort_net) {

	int devide_number = 0; int count = 0;
	NLIST* cur_net; NLIST* sim_net;

	//未識別故障集合格納配列内-全未識別故障集合を参照
	while (devide_number < conf_fault_module) {

		if (hash.n_unconf_fault[devide_number] >= 2) {

			printf("\n\n****************Unconf_Fault_Pair%d*********************************************\n\n", devide_number);
			count = 0;

			//未識別故障集合内-全故障参照
			for (int cur_number = 0; cur_number < hash.n_index[devide_number]; cur_number++) {
				if (hash.unconf_fault[devide_number][cur_number] != NULL) {

					count++;
					if (count == hash.n_unconf_fault[devide_number]) {
						break;
					}

					//故障f1のスコア計算
					cur_net = hash.unconf_fault[devide_number][cur_number];
					if (TPI_Score_Gate(cur_net, 1) != 1) {
						return 0;//エラー処理
					}


					//未識別故障集合内-全故障参照(cur_number以降)
					for (int search_number = cur_number + 1; search_number < hash.n_index[devide_number]; search_number++) {
						if (hash.unconf_fault[devide_number][search_number] != NULL) {

							//故障f2のスコア計算
							sim_net = hash.unconf_fault[devide_number][search_number];
							if (TPI_Score_Gate(sim_net,2) != 1) {
								return 0;//エラー処理
							}

							printf("\n***合計スコア算出:{%s,%s}***\n\n", cur_net->name, sim_net->name);
							if (TPI_Score_Total() != 1) {
								return 0;
							}

							if (TPI_Score_Reset(2) != 1) {
								return 0;
							}

						}
					}

					if (TPI_Score_Reset(1) != 1) {
						return 0;
					}

				}
			}

		}

		devide_number++;
	}

	if (TPI_Points_Count() != 1) {
		return 0;
	}

	return 1;
}

//TPIスコア計算関数(未識別故障ペアを対象)
int TPI_Score_Gate(NLIST* sim_fault,int flag) {

	short main_number = flag - 1; 
	short sub_number; 
	short stop_flag = 3;
	NLIST* signalo;

	//インデックス番号格納
	if (main_number == 0) {
		sub_number = 1;
	}
	else if (main_number == 1) {
		sub_number = 0;
	}
	else {
		return 0;//エラー処理
	}

	//対象故障スコア付与
	if(sim_fault->score_flag >= 1){			//1回以上スコア計算した(2回目のスコア計算は0)

			sim_fault->score[main_number] = 0.0;
			sim_fault->score_flag = stop_flag;
	}

	else if (sim_fault->score[sub_number] == 0.0) {	//まだスコア計算をしていない
		sim_fault->score[main_number] = main_fault_score;
		sim_fault->score_flag = flag;
	}

	//printf("%s\tscore:%.10f\n", sim_fault->name, sim_fault->score[main_number]);

	//出力線エンキュー
	for (int out_number = 0; out_number < sim_fault->n_out; out_number++) {
		queue_enqueue(sim_fault->out[out_number]);
	}

	//外部出力までスコア計算(キューが空になるまで)
	while (queue_empty() == 1) {

		signalo = queue_dequeue();

		if ((signalo->score_flag != flag)&&(signalo->score_flag>=1)) {				//1回以上スコア計算した(2回目のスコア計算は0)

			signalo->score[main_number] = 0.0;
			signalo->score_flag = stop_flag;
		}

		else if (signalo->score[sub_number] == 0.0) {	//まだスコア計算をしていない

			switch (signalo->type) {

				case BUF:
				case INV:
				case FOUT:

					//入力線スコア付与
					signalo->score[main_number] = signalo->in[0]->score[main_number];
					//printf("%s\tscore:%.10f\n", signalo->name, signalo->score[main_number]);
					break;

				case AND:
				case NAND:
				case OR:
				case NOR:
				case EXOR:
				case EXNOR:
					
					//入力線スコアをalpha倍して付与(スコアを下げる)
					for (int in_number = 0; in_number < signalo->n_in; in_number++) {
						if (signalo->in[in_number]->score[main_number] > 0.0) {
							signalo->score[main_number] = signalo->in[in_number]->score[main_number] * alpha;
							//printf("%s\tscore:%.10f\n", signalo->name, signalo->score[main_number]);
							break;
						}
					}
					break;
			}

			signalo->score_flag = flag;

		}

		//出力線エンキュー
		for (int out_number = 0; out_number < signalo->n_out; out_number++) {
			queue_enqueue(signalo->out[out_number]);
		}

	}

	return 1;

}

//total_TPIスコア計算関数
int TPI_Score_Total(void) {

	double temp_score;

	for (int net_number = 0; net_number < n_net; net_number++) {

		//f1のスコア計算
		if (sort_net[net_number]->score_flag == 1) {

			if (sort_net[net_number]->score[0] > 0.0) {
				temp_score = sort_net[net_number]->score[0];
				sort_net[net_number]->total_score += temp_score;
				printf("%s\ttotal_score:%.10f\n", sort_net[net_number]->name, sort_net[net_number]->total_score);
			}
		}
		//f2のスコア計算
		else if (sort_net[net_number]->score_flag == 2) {

			if (sort_net[net_number]->score[1] > 0.0) {
				temp_score = sort_net[net_number]->score[1];
				sort_net[net_number]->total_score += temp_score;
				printf("%s\ttotal_score:%.10f\n", sort_net[net_number]->name, sort_net[net_number]->total_score);
			}
		}
		//エラー処理(score_flag:0,1,2,3以外の値はエラー)
		else if ((sort_net[net_number]->score_flag != 0) && (sort_net[net_number]->score_flag != 3)) {
			return 0;
		}

	}

	return 1;
}


//TPIスコアリセット関数
int TPI_Score_Reset(int flag) {

	int main_number = flag - 1;

	if ((main_number != 0) && (main_number != 1)) {
		return 0;
	}

	for (int net_number = 0; net_number < n_net; net_number++) {

		//f1,f2が伝搬している
		if (sort_net[net_number]->score_flag == 3) {
			sort_net[net_number]->score[main_number] = 0.0;
			sort_net[net_number]->score_flag = 1;
		}
		//f2が伝搬している
		else if (sort_net[net_number]->score_flag == 2) {
			sort_net[net_number]->score[main_number] = 0.0;
			sort_net[net_number]->score_flag = 0;
		}
		//f1が伝搬している
		else if (sort_net[net_number]->score_flag == 1) {
			//f1スコアリセット時(f2スコアリセット時は実行無)
			if (flag == 1) {
				sort_net[net_number]->score[main_number] = 0.0;
				sort_net[net_number]->score_flag = 0;
			}
		}
		//エラー処理
		else if (sort_net[net_number]->score_flag != 0) {
			return 0;
		}
	
	}

	return 1;
}

int TPI_Points_Count(void) {

	double max_score = 0.0;

	//最大スコア算出
	for (int net_number = 0; net_number < n_net; net_number++) {

		if ((sort_net[net_number]->total_score > max_score)&&(sort_net[net_number]->tpi_flag==0)) {
			max_score = sort_net[net_number]->total_score;
		}
	}

	if (max_score == 0.0) {
		return 0;
	}

	//観測ポイント列挙
	printf("\n\n******************観測ポイント列挙********************************\n\n");
	for (int net_number = 0; net_number < n_net; net_number++) {

		if (sort_net[net_number]->total_score == max_score) {
			printf("%s\n", sort_net[net_number]->name);
			sort_net[net_number]->tpi_flag = 1;
			tpi_net[n_tpi] = sort_net[net_number];
			n_tpi++;
		}

	}
	printf("\n挿入観測ポイント数:%d\n", n_tpi);
	printf("\n最大スコア:%.10f\n", max_score);

	return 1;
}
