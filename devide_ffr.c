#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "FFR.h"

int devide_ffr(NLIST** sort_net) {
	
	/*Que.que = (NLIST**)malloc(sizeof(NLIST*) * n_net);
	Que.max = n_net;
	Que.num = 0;
	Que.front = 0;
	Que.rear = 0;*/

	NLIST* signalo;
	
	int net_number, ffr_number, max_id = 0;
	int* n_ffr_in;

	//ffr_id付与
	for (net_number = n_net - 1; net_number >= 0; net_number--) {								//信号線数分

		if ((sort_net[net_number]->n_out >= 2)||(sort_net[net_number]->n_out==0)) {				//PO,FOSに該当するかどうか
												
			sort_net[net_number]->ffr_id = max_id;	
			queue_enqueue(sort_net[net_number]);
			max_id++;

		}

		else {																					
			sort_net[net_number]->ffr_id = sort_net[net_number]->out[0]->ffr_id;				//出力線のffr_idをそのまま付与
		}

	}

	n_ffr = max_id;

	//FFR構造体配列の領域確保,メンバの初期化
	ffr = (FFR*)malloc(sizeof(FFR) * n_ffr);

	for (ffr_number = 0; ffr_number < n_ffr; ffr_number++) {

		ffr[ffr_number].n_pi = 0;
		ffr[ffr_number].n_in_net = 0;
		ffr[ffr_number].po_flag = 0;

		ffr[ffr_number].in = (FFR**)malloc(sizeof(FFR*) * n_ffr);
		for (net_number = 0; net_number < n_ffr; net_number++) {
			ffr[ffr_number].in[net_number] = (FFR*)malloc(sizeof(FFR));
		}

		ffr[ffr_number].out = (FFR**)malloc(sizeof(FFR*) * n_ffr);
		for (net_number = 0; net_number < n_ffr; net_number++) {
			ffr[ffr_number].out[net_number] = NULL;
		}
	}

	//FFR入力数配列の領域確保
	n_ffr_in = (int*)malloc(sizeof(int)*n_ffr);												
	for (int i = 0; i < n_ffr; i++) {															
		n_ffr_in[i] = 0;
		
	}

	//FFR_id接続
	for (ffr_number = 0; ffr_number < n_ffr; ffr_number++) {
		signalo = queue_dequeue();

		ffr[ffr_number].fos = signalo;
		ffr[ffr_number].ffr_id = ffr_number;

	}

	//外部入力カウント
	for (net_number = 0; net_number < n_pi; net_number++) {

		ffr[pi[net_number]->ffr_id].n_pi++;

	}

	//外部出力格納
	for (net_number = 0; net_number < n_po; net_number++) {

		ffr[po[net_number]->ffr_id].po_flag = 1;

	}

	int out_ffr_id, in_number;
	FFR* in_ffr, * out_ffr;

	//FFRポインタ接続
	for (ffr_number = 0; ffr_number < n_ffr; ffr_number++) {

		if (ffr[ffr_number].fos->n_out >= 2) {																//FANOUT STEMに該当するかどうか

			for (net_number = 0; net_number < ffr[ffr_number].fos->n_out; net_number++) {

				out_ffr_id= ffr[ffr_number].fos->out[net_number]->ffr_id;									//対象の信号線の出力線におけるffr_id
				in_number = n_ffr_in[out_ffr_id];															//他入力FFRのffr_idのインデックス番号

				in_ffr = &ffr[ffr_number];
				out_ffr = &ffr[out_ffr_id];

				ffr[out_ffr_id].in[in_number] = in_ffr;														//他FFRの入力ポインタへ対象FFRポインタを挿入
				ffr[ffr_number].out[net_number] = out_ffr;													//対象FFRの出力ポインタへ他FFRポインタを挿入
				n_ffr_in[out_ffr_id]++;																		//FFR入力数インクリメント
			}
		}
	}

	//FFR入力数,出力数,入力線数算出
	for (ffr_number = 0; ffr_number < n_ffr; ffr_number++) {

		ffr[ffr_number].n_in = n_ffr_in[ffr_number];
		ffr[ffr_number].n_out = ffr[ffr_number].fos->n_out;
		ffr[ffr_number].n_in_net = ffr[ffr_number].n_pi + ffr[ffr_number].n_in;

	}

	//FFR_id表示
	for (net_number = n_net - 1; net_number >= 0; net_number--) {
		printf("%s,\tFFR_ID:%d\n", sort_net[net_number]->name, sort_net[net_number]->ffr_id);
	}printf("\n");

	//FFR表示
	for (ffr_number = 0; ffr_number < n_ffr; ffr_number++) {
		printf("FFR%d\n他FFRからの入力数:%d\t他FFRへの出力数:%d\n", ffr[ffr_number].ffr_id, ffr[ffr_number].n_in, ffr[ffr_number].n_out);

		printf("入力数:%d\n", ffr[ffr_number].n_in_net);
		printf("外部出力格納フラグ:%d\n", ffr[ffr_number].po_flag);

		/*printf("入力FFR:");
		for (int in_num = 0; in_num < ffr[ffr_number].n_in; in_num++) {

			printf("%d,",ffr[ffr_number].in[in_num]->ffr_id);
		}
		printf("\n");
		printf("出力FFR:");
		for (int out_num = 0; out_num < ffr[ffr_number].n_out; out_num++) {

			printf("%d,", ffr[ffr_number].out[out_num]->ffr_id);
		}*/

		printf("\nFout Stem:%s", ffr[ffr_number].fos->name);
		printf("\n\n");
	}

	printf("FFR分割完了！\n\n\n");

	return 1;

}
