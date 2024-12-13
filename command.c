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

int command(int argc, char** argv) {

	//オプション初期化

	strcpy(opt.tp, "NO");					// テストパターンファイル

	strcpy(opt.pin, "NO");					// ピンファイル

	strcpy(opt.v, "NO");						// 組合せ回路ファイル

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

	printf("テストパターンファイル名:%s\n外部入力ピンファイル名:%s\n組合せ回路ファイル名:%s\n\n\n", opt.tp, opt.pin, opt.v);

	printf("コマンド解析完了!\n\n\n");

	return 1;

}
