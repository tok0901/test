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

#define SIM_3	3
#define SIM_10	10

int command(int argc, char** argv) {

	//オプション初期化

	strcpy(opt.tp, "NO");					// テストパターンファイル

	strcpy(opt.pin, "NO");					// ピンファイル

	strcpy(opt.v, "NO");						// 組合せ回路ファイル

	opt.n_ffr = SIM_3;

	opt.n_net = SIM_10;

	//コマンド解析

	for (int i = 1; i < argc; i++) {

		//テストパターンファイル
		if (strcmp(argv[i], "-i") == 0) {
			strcpy(opt.tp, argv[i + 1]);
			i++;
		}

		//ピンファイル
		else if (strcmp(argv[i], "-pi") == 0) {
			strcpy(opt.pin, argv[i + 1]);
			i++;
		}

		//組合せ回路ファイル
		else if (strcmp(argv[i], "-v") == 0) {
			strcpy(opt.v, argv[i + 1]);
			i++;
		}

		//評価FFR数
		else if (strcmp(argv[i], "-ffr") == 0) {
			if (atoi(argv[i + 1]) > 0) {
				opt.n_ffr = atoi(argv[i + 1]);
			}
		}


		//FFR内評価信号線数
		else if (strcmp(argv[i], "-net") == 0) {
			if (atoi(argv[i + 1]) > 0) {
				opt.n_net = atoi(argv[i + 1]);
			}
		}

	}

	if (strcmp(opt.tp, "NO") == 0) {
		printf("テストパターンファイル名が入力されていません\n");
		return 0;
	}

	if (strcmp(opt.pin, "NO") == 0) {
		printf("外部入力ピンファイル名が入力されていません\n");
		return 0;
	}

	if (strcmp(opt.v, "NO") == 0) {
		printf("組合せ回路ファイル名が入力されていません\n");
		return 0;
	}

	printf("テストパターンファイル名:%s\n外部入力ピンファイル名:%s\n組合せ回路ファイル名:%s\n評価FFR数:%d個\n評価信号線数:%d本\n\n\n", opt.tp, opt.pin, opt.v,opt.n_ffr,opt.n_net);

	//printf("コマンド解析完了!\n\n\n");

	return 1;

}
