#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "module.h"


int make_net(NLIST* nl) {

	Que.que = (NLIST**)malloc(sizeof(NLIST*) * n_net * 1000);
	Que.max = n_net * 1000;
	Que.num = 0;
	Que.front = 0;
	Que.rear = 0;
	NLIST* signalo;
	
	//レベル初期化

	int i;

	for (i = 0; i < n_net; i++) {
		nl[i].level = -1;
	}

	//初期レベル付与

	for (i = 0; i < n_pi; i++) {

		pi[i]->level = 1;
		queue_enqueue(pi[i]);
		printf("外部入力信号線:%s  level:%d\n", pi[i]->name, pi[i]->level);

	}

	//レベル付与

	int max = 0;
	int level_flag = 1;//level_flagが1の時に最大レベルの算出をし、0の時は処理を行わない
	int j;

	while (queue_empty() == 1) {

		if (queue_overflow() == 0) {
			printf("オーバーフロー\n");
			return 0;
		}
		
		signalo = queue_dequeue();//信号線をデキュー

		if (signalo->level < 1) {//デキューした信号線レベルが－1である

			for (j = 0; j < signalo->n_in; j++) {

				if (signalo->in[j]->level < 1) {//入力信号線レベルが－1である

					queue_enqueue(signalo->in[j]);//入力線をエンキューする
					//printf("エンキューされた入力線:%s\n", signalo->in[j]->name);
					level_flag = 0;//最大レベルの算出を行わないように設定
					max = 0;//比較する最大レベルをリセット
					break;

				}

				else {

					if (max < signalo->in[j]->level) {//入力最大レベルの算出
						max = signalo->in[j]->level;
						level_flag = 1;
					}

				}

			}

			if (level_flag == 1) {

				signalo->level = max + 1;	//現信号線レベルの算出
				printf("レベル付与成功 %s Level:%d\n", signalo->name, signalo->level);
				max = 0;//比較する最大レベルをリセット

			}

		}

		if ((signalo->n_out >= 1) && (level_flag == 1)) {//出力信号線をエンキューする

			for (j = 0; j < signalo->n_out; j++) {

				if (signalo->out[j]->level < 1) {//出力信号線レベルが－1で、エンキュー可能か判定

					//&& (signalo->out[j]->que_flag != 0)
					queue_enqueue(signalo->out[j]);
					//signalo->out[j]->que_flag = 0;

					//printf("エンキューされた出力線:%s\n", signalo->out[j]->name);

				}
			}

		}


	}

	printf("レベル付け完了\n");

	//信号線ソート

	//ソート配列の領域確保

	sort_net = (NLIST**)malloc(sizeof(NLIST*) * n_net);
	for (i = 0; i < n_net; i++) {
		sort_net[i] = &nl[i];
	}

	//クイックソート

	quick_sort(sort_net, 0, n_net - 1);

	//ソート確認

	for (i = 0; i < n_net; i++) {
		printf("レベル：%d, 名前:%6s, type:%d 入力数:%d, 出力数:%d\n", sort_net[i]->level, sort_net[i]->name, sort_net[i]->type,sort_net[i]->n_in,sort_net[i]->n_out);
	}

	printf("信号線正規化完了!\n\n\n");

	return 1;

}

void quick_sort(NLIST** sort_net, int left, int right) {

	int i, j;
	NLIST* pivot;
	i = left;
	j = right;
	pivot = sort_net[(i + j) / 2];

	while (1) {
		while (sort_net[i]->level < (pivot->level)) i++;
		while (pivot->level < sort_net[j]->level) j--;
		if (i >= j) break;
		swap(sort_net, i, j);
		i++;
		j--;
	}
	if (left < i - 1) quick_sort(sort_net, left, i - 1);
	if (j + 1 < right) quick_sort(sort_net, j + 1, right);

}


void swap(NLIST** sort_net, int i, int j) {

	NLIST* temp;
	temp = sort_net[i];
	sort_net[i] = sort_net[j];
	sort_net[j] = temp;

}
