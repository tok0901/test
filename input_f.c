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

int input_f(char* testfile, char* pinfile, char* vfile) {

	FILE* fp_test, * fp_pin;
	fp_test = fopen(testfile, "r");
	fp_pin = fopen(pinfile, "r");
	int i, sim_test = 64;

	if (fp_test == NULL) {
		printf("テスト集合ファイルが読み込めません\n");
		return 0;
	}

	if (fp_pin == NULL) {
		printf("外部入力ピンファイルが読み込めません\n");
		return 0;
	}

	if (read_nl(opt.v) != 0) {
		printf("組合せ回路ファイルが読み込めません\n");
		return 0;
	}



	//ピン情報格納配列の領域確保
	Str_pin = (char**)malloc(sizeof(char*) * n_pi);
	for (i = 0; i < n_pi; i++) {
		Str_pin[i] = (char*)malloc(sizeof(char) * 50);
	}

	//外部入力ピンファイル読み込み
	char buf_pin[256];
	char* find;
	i = 0;

	while (fgets(buf_pin, 256, fp_pin) != NULL) {

		strcpy(Str_pin[i], buf_pin);
		if ((find = strchr(Str_pin[i], '\n')) != NULL) {
			*find = '\0';
		}

		i++;
	}

	//テストパターン数カウント
	n_test = 0;
	char* buf_test;
	buf_test = (char*)malloc(sizeof(char) * n_pi);

	while (fgets(buf_test, n_pi * 2, fp_test) != NULL) {
		if (strcmp(buf_test, "\n") != 0) {	//改行のみの記述をカウントしないように制御
			n_test++;
		}
	}rewind(fp_test);

	//テストパターン配列の領域確保
	Str_test = (char**)malloc(sizeof(char*) * n_test);
	for (i = 0; i < n_test; i++) {
		Str_test[i] = (char*)malloc(sizeof(char) * n_pi);
	}

	//テストパターンファイル読み込み
	i = 0;
	while (fgets(buf_test, n_pi * 2, fp_test) != NULL) {

		if (strcmp(buf_test, "\n") != 0) {
			strcpy(Str_test[i], buf_test);
			i++;
		}
	}

	//論理値格納配列の領域確保
	test_tbl = (char**)malloc(sizeof(char*) * n_pi);
	for (i = 0; i < n_pi; i++) {
		test_tbl[i] = (char*)malloc(sizeof(char) * 50);
	}

	//信号線の論理値配列(64bit)の領域確保
	//信号線のDetectability配列(64bit)の領域確保→long long intとして修正,ビット演算として処理
	for (i = 0; i < n_net; i++) {
		nl[i].val = 0ULL;
		nl[i].fval = 0ULL;
		nl[i].score_flag = 0;
		nl[i].total_score = 0.0;
		nl[i].tpi_flag = 0;
	}

	for (i = 0; i < n_po; i++) {
		po[i]->tpi_flag = -1;
	}

	//故障辞書配列の領域確保,初期化
	dic = (DICT*)malloc(sizeof(DICT) * n_test);
	for (int test_number = 0; test_number < n_test; test_number++) {
		dic[test_number].tp = test_number;
		dic[test_number].n_grp = 0;
		dic[test_number].insert_number = 0;
		
	}

	//未識別故障集合格納配列の領域確保,初期化
	hash.unconf_fault = (NLIST***)malloc(sizeof(NLIST**) * n_net * 2);
	hash.unconf_saf_fault = (ULLI**)malloc(sizeof(ULLI*) * n_net * 2);
	hash.n_unconf_fault = (int*)malloc(sizeof(int) * n_net * 2);
	hash.n_index = (int*)malloc(sizeof(int) * n_net * 2);
	hash.n_grp = 0;
	hash.insert_number = 0;
	hash.confirm_number = 0;
	hash.confirm_flag = (short**)malloc(sizeof(short*) * n_net * 2);
	hash.n_confirm_flag = (short*)malloc(sizeof(short) * n_net * 2);

	//観測ポイント配列の領域確保,観測ポイント数初期化
	n_tpi_po = n_po;
	tpi_po_net = (NLIST**)malloc(sizeof(NLIST*) * n_tpi_po);
	n_tpi = 0;

	for (int po_number = 0; po_number < n_po; po_number++) {
		tpi_po_net[po_number] = po[po_number];
	}

	fclose(fp_test);

	fclose(fp_pin);

	//printf("\n\n\nファイル入力完了!\n\n\n");

	return 1;

}
