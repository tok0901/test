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

int devide_ffr(void) {

	NLIST* signalo;

	int max_id = 0;
	int* n_ffr_in;

	//ffr_id付与
	for (int net_number = n_net - 1; net_number >= 0; net_number--) {								//信号線数分

		//PO,FOS,TPIに該当するかどうか
		if ((sort_net[net_number]->n_out >= 2) || (sort_net[net_number]->n_out == 0)||(sort_net[net_number]->tpi_flag==1)) {				

			sort_net[net_number]->ffr_id = max_id;
			queue_enqueue(sort_net[net_number]);
			max_id++;

		}

		else {
			sort_net[net_number]->ffr_id = sort_net[net_number]->out[0]->ffr_id;				//出力線のffr_idをそのまま付与
		}

	}

	n_ffr = max_id;		//ffr数更新

	//FFR構造体配列の領域確保,メンバの初期化
	ffr = (FFR*)malloc(sizeof(FFR) * n_ffr);

	for (int ffr_number = 0; ffr_number < n_ffr; ffr_number++) {

		ffr[ffr_number].n_pi = 0;
		ffr[ffr_number].n_in_net = 0;
		ffr[ffr_number].po_flag = 0;
		ffr[ffr_number].n_unconf_fault_det = 0;
		ffr[ffr_number].n_unconf_fault = 0;
		ffr[ffr_number].n_net_sim = 0;

		ffr[ffr_number].in = (FFR**)malloc(sizeof(FFR*) * n_ffr);

		ffr[ffr_number].out = (FFR**)malloc(sizeof(FFR*) * n_ffr);
		
	}

	//FFR入力数配列の領域確保
	n_ffr_in = (int*)malloc(sizeof(int) * n_ffr);
	for (int i = 0; i < n_ffr; i++) {
		n_ffr_in[i] = 0;

	}

	//FFR_id接続
	for (int ffr_number = 0; ffr_number < n_ffr; ffr_number++) {
		signalo = queue_dequeue();

		ffr[ffr_number].fos = signalo;
		ffr[ffr_number].ffr_id = ffr_number;
		ffr[ffr_number].po_fault_flag = (ULLI*)malloc(sizeof(ULLI) * n_tpi_po);
		for (int po_number = 0; po_number < n_tpi_po; po_number++) {
			ffr[ffr_number].po_fault_flag[po_number] = 0ULL;
		}

	}

	//外部入力カウント
	for (int net_number = 0; net_number < n_pi; net_number++) {

		ffr[pi[net_number]->ffr_id].n_pi++;

	}

	//外部出力格納フラグ設定
	for (int net_number = 0; net_number < n_po; net_number++) {

		ffr[po[net_number]->ffr_id].po_flag = 1;

	}

	//信号線カウント
	for (int net_number = 0; net_number < n_net; net_number++) {
		ffr[sort_net[net_number]->ffr_id].n_net_sim++;
	}

	int out_ffr_id, in_number;

	//FFRポインタ接続
	for (int ffr_number = 0; ffr_number < n_ffr; ffr_number++) {
																

			for (int net_number = 0; net_number < ffr[ffr_number].fos->n_out; net_number++) {

				out_ffr_id = ffr[ffr_number].fos->out[net_number]->ffr_id;									//対象の信号線の出力線におけるffr_id
				in_number = n_ffr_in[out_ffr_id];															//他入力FFRのffr_idのインデックス番号

				ffr[out_ffr_id].in[in_number] = &ffr[ffr_number];														//他FFRの入力ポインタへ対象FFRポインタを挿入
				ffr[ffr_number].out[net_number] = &ffr[out_ffr_id];													//対象FFRの出力ポインタへ他FFRポインタを挿入
				n_ffr_in[out_ffr_id]++;																		//FFR入力数インクリメント
			}
		
	}

	//FFR入力数,出力数,入力線数算出
	for (int ffr_number = 0; ffr_number < n_ffr; ffr_number++) {

		ffr[ffr_number].n_in = n_ffr_in[ffr_number];
		ffr[ffr_number].n_out = ffr[ffr_number].fos->n_out;
		ffr[ffr_number].n_in_net = ffr[ffr_number].n_pi + ffr[ffr_number].n_in;

	}

	/*/FFR_id表示
	for (int ffr_number = 0; ffr_number < n_ffr; ffr_number++) {

		printf("\nFFR%d\n", ffr_number);
		printf("\n%s\tffr_ID:%d\n", ffr[ffr_number].fos->name,ffr[ffr_number].ffr_id);
		if ((ffr[ffr_number].fos->type != IN) && (ffr[ffr_number].fos->type != FOUT)) {
			queue_enqueue(ffr[ffr_number].fos);
		}

		while (queue_empty() == 1) {

			NLIST* signalo = queue_dequeue();

			if (signalo->ffr_id != ffr_number) {
				printf("%s\ttype:%d\nerror_ID:%d\n", signalo->name,signalo->type, signalo->ffr_id);
			}

			printf("%s\tffr_ID:%d\tnet_number:%d\n", signalo->name, signalo->ffr_id,signalo->sort_n);

			if ((signalo->type != IN) && (signalo->type != FOUT)) {
				for (int in_number = 0; in_number < signalo->n_in; in_number++) {

					queue_enqueue(signalo->in[in_number]);

				}
			}

		}

	}
	///////////////////////*/

	/*/FFR表示
	for (int ffr_number = 0; ffr_number < n_ffr; ffr_number++) {
		printf("FFR%d\n他FFRからの入力数:%d\t他FFRへの出力数:%d\n", ffr[ffr_number].ffr_id, ffr[ffr_number].n_in, ffr[ffr_number].n_out);

		printf("入力数:%d\n", ffr[ffr_number].n_in_net);
		printf("外部出力格納フラグ:%d\n", ffr[ffr_number].po_flag);

		printf("入力FFR:");
		for (int in_num = 0; in_num < ffr[ffr_number].n_in; in_num++) {

			printf("%d,",ffr[ffr_number].in[in_num]->ffr_id);
		}
		printf("\n");
		printf("出力FFR:");
		for (int out_num = 0; out_num < ffr[ffr_number].n_out; out_num++) {

			printf("%d,", ffr[ffr_number].out[out_num]->ffr_id);
		}

		printf("\nFout Stem:%s", ffr[ffr_number].fos->name);
		printf("\n\n");
	}///////////////////////////*/

	free(n_ffr_in);

	//printf("FFR分割完了！\n\n\n");

	return 1;

}

void ffr_reset(void) {

	for (int ffr_number = 0; ffr_number < n_ffr; ffr_number++) {
		free(ffr[ffr_number].in);
		free(ffr[ffr_number].out);
		free(ffr[ffr_number].po_fault_flag);
	}

	free(ffr);

	n_ffr = 0;

}
