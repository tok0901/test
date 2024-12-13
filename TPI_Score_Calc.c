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


int TPI_Score_Net(NLIST* sim_net,int calc_flag);
int TPI_Score_Total(void);
int TPI_Points_Count(void);
double alpha = 0.9; double main_fault_score = 10.0;

#define	Score_1st  1
#define Score_2nd  2
#define Score_Lost 3
#define Score_ALL  4


//TPIスコア計算関数(未識別故障集合格納配列を対象)
int TPI_Score_Calc(void) {

	int hash_number = 0; int count = 0;
	NLIST* cur_net; NLIST* sim_net; NLIST* signalo;

	//未識別故障集合格納配列内-全未識別故障集合を参照
	while (hash_number < conf_fault_module) {

		if (hash.n_unconf_fault[hash_number] >= 2) {

			//printf("\n\n****************Unconf_Fault_Pair%d*********************************************\n\n", hash_number);
			count = 0;

			//未識別故障集合内-全故障参照
			//cur_net探索
			for (int cur_number = 0; cur_number < hash.n_index[hash_number]; cur_number++) {
				if (hash.unconf_fault[hash_number][cur_number] != NULL) {

					count++;
					if (count == hash.n_unconf_fault[hash_number]) {
						break;
					}

					cur_net = hash.unconf_fault[hash_number][cur_number];

					//if (TPI_Score_Gate(cur_net, 1) != 1) {
					//	return 0;//エラー処理
					//}


					//未識別故障集合内-全故障参照(cur_number以降)
					//sim_net探索
					for (int search_number = cur_number + 1; search_number < hash.n_index[hash_number]; search_number++) {
						if (hash.unconf_fault[hash_number][search_number] != NULL) {
					
							//全信号線スコアリセット
							for (int net_number = 0; net_number < n_net; net_number++) {
								sort_net[net_number]->tpi_score = -1.0;
								sort_net[net_number]->tpi_score_flag = 0;
							}

							sim_net = hash.unconf_fault[hash_number][search_number];

							//cur_netスコア付与(1回目)
							cur_net->tpi_score = main_fault_score;
							cur_net->tpi_score_flag = Score_1st;

							//cur_netスコア計算(1回目)
							for (int net_number = 0; net_number < n_net; net_number++) {

								if (TPI_Score_Net(sort_net[net_number], Score_1st) == 1) {
									sort_net[net_number]->tpi_score_flag = Score_1st;
								}

							}

							//sim_netスコア付与(2回目)
							if (sim_net->tpi_score_flag == 1) {
								sim_net->tpi_score = 0.0;
								sim_net->tpi_score_flag = Score_Lost;
							}
							else {
								sim_net->tpi_score = main_fault_score;
								sim_net->tpi_score_flag = Score_2nd;
							}

							//sim_netスコア計算(2回目)
							for (int net_number = 0; net_number < n_net; net_number++) {

								if (TPI_Score_Net(sort_net[net_number], Score_2nd) == 1) {
									sort_net[net_number]->tpi_score_flag = Score_2nd;
								}
								
							}

							//total_score算出
							//printf("\n***合計スコア算出:{%s,%s}***\n\n", cur_net->name, sim_net->name);
							if (TPI_Score_Total() != 1) {
								return 0;
							}

							//if (TPI_Score_Reset(2) != 1) {
							//	return 0;
							//}

						}
					}

					/*if (TPI_Score_Reset(1) != 1) {
						return 0;
					}*/

				}
			}

		}

		hash_number++;
	}

	//観測ポイント選択関数
	if (TPI_Points_Count() != 1) {
		return 0;
	}

	//観測ポイント挿入後は再度FFR分割-ffrの領域解放
	free(ffr);

	return 1;
}

//TPIスコア計算関数(return 1:スコア計算成功,return Score_Lost:score=0)
int TPI_Score_Net(NLIST* sim_net,int calc_flag) {

	double min_score = 11.0;
	
	//入力線分析(入力線がスコア付与されているか)
	for (int in_number = 0; in_number < sim_net->n_in; in_number++) {

		//(入力線がスコア計算されているか)
		if ((sim_net->in[in_number]->tpi_score < min_score) && (sim_net->in[in_number]->tpi_score > 0.0)) {

			min_score = sim_net->in[in_number]->tpi_score;
		}

		//calc_flag判定(スコア計算1回目or2回目)
		//入力線score_flag確認(score_flag=Score_2nd,Score_Lostがあるならsim_net->score=0,->score_flag=Score_Lost)
		if (((sim_net->in[in_number]->tpi_score_flag == Score_2nd)||(sim_net->in[in_number]->tpi_score_flag == Score_Lost)) && (calc_flag == Score_2nd)) {
			sim_net->tpi_score = 0.0;
			sim_net->tpi_score_flag = Score_Lost;
			return Score_Lost;
		}

	}

	if ((min_score != 11.0)&&(calc_flag==Score_1st)) {

		switch (sim_net->type) {

		case FOUT:
		case INV:
		case BUF:

			sim_net->tpi_score = sim_net->in[0]->tpi_score;
			break;

		case AND:
		case NAND:
		case OR:
		case NOR:
		case EXOR:
		case EXNOR:

			sim_net->tpi_score = min_score * alpha;
			break;

		}

		return 1;

	}

	return 0;

}

//total_TPIスコア計算関数
int TPI_Score_Total(void) {

	//double max_score = 0.0;

	for (int net_number = 0; net_number < n_net; net_number++) {

		if (sort_net[net_number]->tpi_score > 0) {
			sort_net[net_number]->total_score += sort_net[net_number]->tpi_score;
			//printf("%s\ttotal_score:%.10f\n", sort_net[net_number]->name, sort_net[net_number]->total_score);
		}

	}

	return 1;
}

//観測ポイント選択関数
int TPI_Points_Count(void) {

	double max_score = 0.0;

	//最大スコア算出
	for (int net_number = 0; net_number < n_net; net_number++) {

		if ((sort_net[net_number]->total_score > max_score) && (sort_net[net_number]->tpi_flag == 0)) {
			max_score = sort_net[net_number]->total_score;
		}
	}

	if (max_score == 0.0) {
		return 0;
	}

	/*/実験用
	int research_number, count = 0;
	for (int net_number = 0; net_number < n_net; net_number++) {
		if (sort_net[net_number]->ffr_id == 105) {
			count++;
			if (count == 6) {
				printf("\nresearch_number=%d\n", net_number);
				research_number = net_number;
				break;
			}
		}
	}

	int net_number = research_number;		//信号線h
	int second_number = 15;

	sort_net[net_number]->tpi_flag = 1;
	n_tpi_po++;
	n_tpi++;
	tpi_po_net = (NLIST**)realloc(tpi_po_net, sizeof(NLIST*) * (n_tpi_po+1));
	tpi_po_net[n_tpi_po - 1] = sort_net[net_number];
	tpi_net = (NLIST**)realloc(tpi_net, sizeof(NLIST*) * (n_tpi+1));
	tpi_net[n_tpi - 1] = sort_net[net_number];
	printf("\n選択観測ポイント:%s\n\n挿入観測ポイント数:%d\n",sort_net[net_number]->name, n_tpi);
	printf("FFR_ID:%d\n",sort_net[net_number]->ffr_id);
	printf("level=%d\n", sort_net[net_number]->level);
	printf("net_number=%d\n", sort_net[net_number]->sort_n);
	printf("識別状態:%d\t%d\n", sort_net[net_number]->conf_fault0_flag, sort_net[net_number]->conf_fault1_flag);

	////2本目観測ポイント
	//sort_net[second_number]->tpi_flag = 1;
	//n_tpi_po++;
	//n_tpi++;
	//tpi_po_net[n_tpi_po - 1] = sort_net[second_number];
	//tpi_net[n_tpi - 1] = sort_net[second_number];
	//printf("\n選択観測ポイント:%s\n\n挿入観測ポイント数:%d\n", sort_net[second_number]->name, n_tpi);

	//////////////////////////*/

	//観測ポイント選択
	printf("\n\n******************選択観測ポイント********************************\n\n");
	for (int net_number = 0; net_number < n_net; net_number++) {

		if ((sort_net[net_number]->total_score == max_score)&&(sort_net[net_number]->tpi_flag==0)) {
			sort_net[net_number]->tpi_flag = 1;
			n_tpi_po++;
			n_tpi++;
			tpi_po_net = (NLIST**)realloc(tpi_po_net, sizeof(NLIST*) * n_tpi_po);
			tpi_po_net[n_tpi_po-1] = sort_net[net_number];
			tpi_net = (NLIST**)realloc(tpi_net, sizeof(NLIST*) * n_tpi);
			tpi_net[n_tpi - 1] = sort_net[net_number];

			printf("\n最大スコア:%.10f\n\n選択観測ポイント:%s\n\n挿入観測ポイント数:%d\n", max_score, sort_net[net_number]->name, n_tpi);
			//printf("識別状態:%d\t%d\n", sort_net[net_number]->conf_fault0_flag, sort_net[net_number]->conf_fault1_flag);
			//printf("net_number=%d\n", sort_net[net_number]->sort_n);
			//printf("level=%d\n", sort_net[net_number]->level);
			//printf("FFR_ID:%d\n", sort_net[net_number]->ffr_id);
			break;
		}

	}///////////////////////*/

	for (int net_number = 0; net_number < n_net; net_number++) {

		sort_net[net_number]->total_score = 0.0;
	}

	return 1;
}

