#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "FFR.h"

//プロトタイプ宣言////////////////////////////////////////////////////////////////

//ネットリスト読込み
int command(int argc, char** argv);
int input_f(char* tp, char* pin, char* v);
int make_net(NLIST* nl);


//故障シミュレーション
int devide_ffr(NLIST** sort_net);
int losic_simulation(int test_number, int sim_test, NLIST** sort_net);
int SAF_PPSFP(int test_number, int sim_test, FFR* ffr);
//int make_dic(int test_number,int sim_test,NLIST** sort_net);

//////////////////////////////////////////////////////////////////////////////////


int main(int argc, char* argv[]) {

	if (command(argc, argv) !=1) {
		printf("\n\nコマンド解析エラー\n");
		return 0;
	}

	if (input_f(opt.tp, opt.pin, opt.v) !=1) {
		printf("\n\nファイル入力エラー\n");
		return 0;
	}

	if (make_net(nl) !=1) {
		printf("\n\n信号線正規化エラー\n");
		return 0;
	}

	if (devide_ffr(sort_net) != 1) {
		printf("\n\nFFR分割エラー\n");
		return 0;
	}

	int sim_test = 64, max_test;
	for (int test_number = 0; test_number < n_test;test_number+=sim_test) {

		max_test = n_test - test_number;

		if (max_test <= sim_test) {
			sim_test = max_test;
		}

		if (losic_simulation(test_number,sim_test,sort_net) != 1) {
			printf("\n\n論理シミュレーションエラー\n");
			return 0;
		}

		if (SAF_PPSFP(test_number,sim_test,ffr) != 1) {
			printf("\n\nPPSFPエラー\n");
			return 0;
		}

		/*if (make_dic(test_number, sim_test, sort_net) != 1) {
			printf("\n\n故障辞書生成エラー\n");
			return 0;
		}*/

		printf("%d番目から%d番目のテストパターンによる故障検出完了\n", test_number, test_number + sim_test);

	}

	for (int test_number = 0; test_number < n_test; test_number++) {
		printf("tp%d\t検出故障数:%d\n", test_number,dic[test_number].n_fault);

		for (int net_number = 0; net_number < dic[test_number].n_fault; net_number++) {
			printf("s-a-%d\t%s\n", dic[test_number].saf_flag[net_number], dic[test_number].fault[net_number]->name);
		}
	}

	for (int net_number = 0; net_number < n_net; net_number++) {
		if (sort_net[net_number]->sim_fault0_flag != 1) {
			printf("未検出故障:s-a-0\t%s\n", sort_net[net_number]->name);
			not_fault++;
		}
		if (sort_net[net_number]->sim_fault1_flag != 1) {
			printf("未検出故障:s-a-1\t%s\n", sort_net[net_number]->name);
			not_fault++;
		}
	}

	double result,all_fault,n_sim_fault;

	all_fault = n_net * 2;

	n_sim_fault = n_net * 2 - not_fault;

	result = (double)n_sim_fault / all_fault;

	printf("故障検出率:%.2f％\n", result*100);

	return 0;

}
