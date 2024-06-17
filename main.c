#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "module.h"


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

	int test_number, sim_test = 64, max_test;
	for (test_number = 0; test_number < n_test;test_number+=sim_test) {

		max_test = n_test - test_number;

		if (max_test <= sim_test) {
			sim_test = max_test;
		}

		if (losic_simulation(test_number,sim_test,sort_net) != 1) {
			printf("\n\n論理シミュレーションエラー\n");
			return 0;
		}

		if (SAF_PPSFP(test_number,sim_test) != 1) {
			printf("\n\nPPSFPエラー\n");
			return 0;
		}

	}

	return 0;

}
