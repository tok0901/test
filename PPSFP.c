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


int SAF_PPSFP(int test_number, int sim_test, int ffr_number) {


	//全信号線-故障値初期化(正常値を設定)
	for (int net_number = 0; net_number < n_net; net_number++) {
		sort_net[net_number]->fval = sort_net[net_number]->val;
	}


	ffr[ffr_number].fos->fval = ~ffr[ffr_number].fos->val;			//対象ffr-fosの故障値を設定


	//論理シミュレーション(故障箇所-外部出力まで)
	for (int net_number = ffr[ffr_number].fos->sort_n + 1; net_number < n_net; net_number++) {

		fault_value_calc(net_number);
	}

	/*/確認用
	NLIST* sim_net = ffr[ffr_number].fos;
	for (int net_number = 0; net_number < n_pi; net_number++) {
		if (sort_net[net_number] == sim_net) {
			sort_net[net_number]->fval = ~sort_net[net_number]->val;
		}
		else {
			sort_net[net_number]->fval = sort_net[net_number]->val;
		}
	}
	for (int net_number = n_pi; net_number < n_net; net_number++) {
		if (sort_net[net_number] == sim_net) {
			sort_net[net_number]->fval = ~sort_net[net_number]->val;
		}
		else {
			fault_value_calc(net_number);
		}
	}
	///////////////////////*/


	//POの故障値保存(出力応答値保存)
	for (int net_number = 0; net_number < n_tpi_po; net_number++) {
		ffr[ffr_number].po_fault_flag[net_number] = tpi_po_net[net_number]->fval;
	}


	return 1;


	///*/修正前

	////対象FFR-FOS
	//if (signali->n_out >= 2) {

	//	//FOB->fault_flag算出,エンキュー
	//	for (int out_number = 0; out_number < signali->n_out; out_number++) {

	//		signali->out[out_number]->fault_flag = signali->fault_flag;

	//		queue_enqueue(signali->out[out_number]);

	//	}
	//}

	////対象FFR-tpi_FOS
	//else if ((signali->n_out == 1)&&(signali->tpi_flag==1)) {

	//	//tpi_FOS->out[0]->fault_flag算出,エンキュー
	//	signalo = signali->out[0];

	//	int in_number;
	//	for (in_number = 0; in_number < signalo->n_in; in_number++) {

	//		if (signali == signalo->in[in_number]) {
	//			break;
	//		}

	//	}

	//	temp = fault_value_calc(signalo, in_number);

	//	det_temp = temp ^ signalo->val;

	//	signalo->fault_flag = signalo->fault_flag | (signali->fault_flag & det_temp);

	//	queue_enqueue(signalo);

	//}

	////queue_enqueue(signali);

	////全出力側FFRに故障伝搬するまで
	//while (queue_empty() == 1) {

	//	signali = queue_dequeue();

	///*	if (signali->n_out >= 2) {

	//		for (int out_number = 0; out_number < signali->n_out; out_number++) {

	//			signali->out[out_number]->fault_flag = signali->fault_flag;

	//			queue_enqueue(signali->out[out_number]);
	//		}

	//	}*/

	//	//修正前
	//	//対象外FFR-FOS
	//	if (signali->n_out >= 2) {

	//		int ffr_id = signali->ffr_id;

	//		temp = signali->fault_flag;

	//		//対象外FFRの出力応答フラグ継承
	//		for (int net_number = 0; net_number < n_tpi_po; net_number++) {

	//			tpi_po_net[net_number]->fault_flag = tpi_po_net[net_number]->fault_flag | (temp & ffr[ffr_id].po_fault_flag[net_number]);

	//		}

	//	}

	//	/////////////////*/

	//	//修正前
	//	//対象外FFR-tpi_FOS
	//	else if ((signali->n_out == 1) && (signali->tpi_flag == 1)) {

	//		int ffr_id = signali->ffr_id;

	//		temp = signali->fault_flag;

	//		//対象外FFRの出力応答フラグ継承
	//		for (int net_number = 0; net_number < n_tpi_po; net_number++) {

	//			tpi_po_net[net_number]->fault_flag = tpi_po_net[net_number]->fault_flag | (temp & ffr[ffr_id].po_fault_flag[net_number]);

	//		}
	//	}
	//	////////////////////////*/

	//	/*/修正後
	//	//FOS
	//	if (signali->n_out >= 2) {
	//		//FOB->fault_flag算出,エンキュー
	//		for (int out_number = 0; out_number < signali->n_out; out_number++) {

	//			signali->out[out_number]->fault_flag = signali->fault_flag;

	//			queue_enqueue(signali->out[out_number]);

	//		}
	//	}

	//	/////////////////////////////////*/

	//	//FOS,tpi_FOS以外(基本的にはここに分岐)
	//	else if (signali->n_out == 1) {

	//		signalo = signali->out[0];

	//		int in_number;
	//		for (in_number = 0; in_number < signalo->n_in; in_number++) {
	//			
	//			if (signali == signalo->in[in_number]) {
	//				break;
	//			}

	//		}

	//		temp = fault_value_calc(signalo, in_number);

	//		det_temp = temp ^ signalo->val;

	//		signalo->fault_flag = signalo->fault_flag | (signali->fault_flag & det_temp);

	//		queue_enqueue(signalo);

	//	}

	//	////tpi_FOS(分岐元信号線以外)
	//	////(出力線にfault_flagを印加)
	//	//if ((signalo->tpi_flag == 1)&&(signalo->n_out==1)) {

	//	//	//out->FOS->flag = out->FOS->flag | (tpi_FOS->flag & tpi_FOS->det)
	//	//	ffr[signalo->out[0]->ffr_id].fos->fault_flag = ffr[signalo->out[0]->ffr_id].fos->fault_flag | (signalo->fault_flag & signalo->det);
	//	//	
	//	//	//出力FFR内-FOSエンキュー
	//	//	queue_enqueue(ffr[signalo->out[0]->ffr_id].fos);

	//	//}

	//	////FOS
	//	//else if (signalo->n_out >= 2) {

	//	//	for (int out_number = 0; out_number < signalo->n_out; out_number++) {

	//	//		//FOB->flag= FOS->flag
	//	//		signalo->out[out_number]->fault_flag = signalo->fault_flag;

	//	//		//FOBエンキュー
	//	//		queue_enqueue(signalo->out[out_number]);
	//	//		
	//	//	}

	//	//}

	//	////FOB
	//	//else if (signalo->n_out == 1) {

	//	//	//FOS->flag = FOS->flag | (FOB->flag & FOB->det)
	//	//	ffr[signalo->ffr_id].fos->fault_flag = ffr[signalo->ffr_id].fos->fault_flag | (signalo->fault_flag & signalo->det);

	//	//	//FOSエンキュー
	//	//	queue_enqueue(ffr[signalo->ffr_id].fos);

	//	//}

	//}

	//


	/*/確認用
	printf("\nFFR%d\n", ffr_number);
	for (int po_number = 0; po_number < n_tpi_po; po_number++) {
		printf("%s-fval\t", tpi_po_net[po_number]->name);
		printBinary(ffr[ffr_number].po_fault_flag[po_number], sim_test); printf("\n");
	}///////////////////////*/


}
