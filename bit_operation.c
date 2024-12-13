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

void bit_count_64(void) {

	int count = 0, bit_number;

	bit_number = n_tpi_po;

	while (bit_number > BIT_64) {
		count++;
		bit_number -= BIT_64;
	}

	count++;

	n_64bit = count;	//64bit列数更新

}

int bit_print_N(ULLI n, int N) {

	ULLI mask = 1ULL << (63 - N);

	if ((n & mask) == 0ULL) {
		return 0;
	}
	else {
		return 1;
	}
}

ULLI bit_setting_1(ULLI n, int N) {

	if (N >= 64) {
		printf("対応ビット列にずれが生じています\n");
		exit(0);
	}

	ULLI set = 0ULL;
	ULLI mask = 1ULL << (63 - N);
	set = n | mask;

	return set;

}

ULLI bit_setting_0(ULLI n, int N) {

	if (N >= 64) {
		printf("対応ビット列にずれが生じています\n");
		exit(0);
	}

	ULLI set = 0ULL;
	ULLI mask = 1ULL << (63 - N);
	mask = ~mask;
	set = n & mask;

	return set;

}

int bit_search_N(ULLI n, int N) {

	if (N >= 64) {
		printf("対応ビット列にずれが生じています\n");
		exit(0);
		//return 0;
	}

	ULLI mask = 1ULL << (63 - N);
	//return (n >> N) & 1;

	if ((n & mask) == 0ULL) {
		return 0;
	}
	else {
		return 1;
	}

}

void print_po_val(ULLI* n) {

	int print_number = BIT_64;
	for (int bit_number = 0; bit_number < n_64bit; bit_number++) {

		if (bit_number == n_64bit - 1) {
			
			if (n_tpi_po % BIT_64 != 0) {
				print_number = n_tpi_po % BIT_64;
			}
		}

		printBinary(n[bit_number], print_number);
	}
}

int strcmp_po_val(ULLI* x, ULLI* y,int flag) //一致:0
{

	int str_flag = 0;
	int x_val, y_val, calc_flag;

	if (flag == PO_VAL) {
		calc_flag = n_64bit_po;
	}
	else if (flag == TPI_PO_VAL) {
		calc_flag = n_64bit;
	}

	//64bitずつ
	for (int bit_number = 0; bit_number < calc_flag; bit_number++) {

		//先頭bitから末尾bitまで値を探索
		for (int net_number = 0; net_number < BIT_64; net_number++) {

			x_val = bit_search_N(x[bit_number], net_number);

			y_val = bit_search_N(y[bit_number], net_number);

			//x,yの値が異なる時点で処理終了
			if (x_val != y_val) {

				str_flag = 1;
				break;
			}

		}

	}

	return str_flag;

}

//nにはループ変数等は選択できない(配列宣言時の要素数等)
int count_number_64(int n) {

	int bit_number;

	bit_number = n % BIT_64;

	if (bit_number == 0) {

		bit_number = n / BIT_64;
	}
	else {
		bit_number = n / BIT_64 + 1;
	}

	return bit_number;
}
