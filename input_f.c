#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Netlist.h"
#include "Queue.h"
#include "Command.h"
#include "Fault_dic.h"
#include "FFR.h"

int input_f(char* testfile, char* pinfile, char* vfile) {

	FILE* fp_test, * fp_pin;
	fp_test = fopen(testfile, "r");
	fp_pin = fopen(pinfile, "r");
	int i,sim_test = 64;

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

	while (fgets(buf_test, n_pi*2, fp_test) != NULL) {
		if (strcmp(buf_test, "\n") != 0) {
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
	while (fgets(buf_test, n_pi*2, fp_test) != NULL) {

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
	//信号線のDetectability配列(64bit)の領域確保
	for (i = 0; i < n_net; i++) {
		nl[i].value = (int*)malloc(sizeof(int) * sim_test);
		nl[i].detec = (int*)malloc(sizeof(int) * sim_test);
		nl[i].value_fault_flag = (int*)malloc(sizeof(int) * sim_test);
	}

	//故障辞書配列の領域確保,初期化
	dic = (DICT*)malloc(sizeof(DICT) * n_test);
	for (int test_number = 0; test_number < n_test; test_number++) {
		dic[test_number].tp = test_number;
		dic[test_number].fault = (NLIST**)malloc(sizeof(NLIST*) * n_net);
		dic[test_number].saf_flag = (int*)malloc(sizeof(int) * n_net);
		dic[test_number].n_fault = 0;
	}

	//未識別故障ペア格納ハッシュ配列の領域確保
	hash.unconf_fault = (NLIST***)malloc(sizeof(NLIST**) * n_net * 2);
	hash.saf_flag = (int**)malloc(sizeof(int*) * n_net * 2);


	fclose(fp_test);

	fclose(fp_pin);

	printf("ファイル入力完了!\n\n\n");

	return 1;

}
