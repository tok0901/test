#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"

int devide_ffr(NLIST** sort_net) {
	
	Que.que = (NLIST**)malloc(sizeof(NLIST*) * n_net);
	Que.max = n_net;
	Que.num = 0;
	Que.front = 0;
	Que.rear = 0;

	NLIST* signalo;
	
	int net_number,ffr_number,max_id=0;
	int* n_ffr_in;


	//外部出力への新規ffr_id付与
	for (net_number = n_po-1; net_number >=0; net_number--) {
		
		po[net_number]->ffr_id = max_id;
		queue_enqueue(po[net_number]);
		max_id++;

	}

	//ffr_id付与
	for (net_number = n_net - n_po - 1; net_number >= 0; net_number--) {						//外部出力以外の全信号線

		if (sort_net[net_number]->n_out >= 2) {													//FANOUT STEMに該当するかどうか
												
			sort_net[net_number]->ffr_id = max_id;	
			queue_enqueue(sort_net[net_number]);
			max_id++;

		}

		else {																					
			sort_net[net_number]->ffr_id = sort_net[net_number]->out[0]->ffr_id;				//出力線のffr_idをそのまま付与
		}

	}

	//FFR構造体配列の領域確保
	ffr = (FFR*)malloc(sizeof(FFR) *max_id);
	for (int i = 0; i <max_id; i++) {
		ffr[i].in = (FFR**)malloc(sizeof(FFR*)*max_id);											//FFR入力ポインタの領域確保

		ffr[i].out = (FFR**)malloc(sizeof(FFR*)*max_id);										//FFR出力ポインタの領域確保

		ffr[i].n_pi = 0;
		ffr[i].n_in_net = 0;

	}

	//FFR入力数配列の領域確保
	n_ffr_in = (int*)malloc(sizeof(int)*max_id);												
	for (int i = 0; i < max_id; i++) {															
		n_ffr_in[i] = 0;
		
	}

	//FFR分割
	for (ffr_number = 0; ffr_number < max_id; ffr_number++) {

		signalo = queue_dequeue();

		ffr[ffr_number].fos = signalo;
		ffr[ffr_number].ffr_id = ffr_number;

		if (signalo->n_out >= 2) {																//FANOUT STEMに該当するかどうか

			for (net_number = 0; net_number < signalo->n_out; net_number++) {

				int out_ffr_id, in_number;
				out_ffr_id= signalo->out[net_number]->ffr_id;									//対象の信号線の出力線におけるffr_id
				in_number = n_ffr_in[out_ffr_id];												//他入力FFRのffr_idのインデックス番号

				ffr[out_ffr_id].in[in_number] = &ffr[signalo->ffr_id];							//他FFRの入力ポインタへ対象FFRポインタを挿入
				ffr[signalo->ffr_id].out[net_number] = &ffr[out_ffr_id];						//対象FFRの出力ポインタへ他FFRポインタを挿入
				n_ffr_in[out_ffr_id]++;															//FFR入力数インクリメント
			}
		}
	}

	for (net_number = 0; net_number < n_pi; net_number++) {
		
		ffr[pi[net_number]->ffr_id].n_pi++;
		ffr[pi[net_number]->ffr_id].n_in_net++;

	}

	for (net_number = 0; net_number < n_net; net_number++) {
		printf("%s,\tFFR_id:%d\n", sort_net[net_number]->name, sort_net[net_number]->ffr_id);
	}printf("\n");

	for (ffr_number = 0; ffr_number < max_id; ffr_number++) {

		ffr[ffr_number].n_out = ffr[ffr_number].fos->n_out;										//他FFRへの出力数算出
		ffr[ffr_number].n_in = n_ffr_in[ffr_number];											//他FFRへの入力数算出
		ffr[ffr_number].n_in_net += ffr[ffr_number].n_in;

		printf("FFR%d\n他FFRからの入力数:%d\t他FFRへの出力数:%d\n", ffr[ffr_number].ffr_id, ffr[ffr_number].n_in, ffr[ffr_number].n_out);


		//printf("入力数:%d\n", ffr[ffr_number].n_in_net);
		printf("入力FFR:");
		for (int in_number = 0; in_number < ffr[ffr_number].n_in; in_number++) {
		
			printf("%d,",ffr[ffr_number].in[in_number]->ffr_id);
		}

	}

	printf("FFR分割成功！\n\n\n");

	return 1;

}
