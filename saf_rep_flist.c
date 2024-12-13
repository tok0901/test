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


void saf_rep_flist(void) {

	int		i, j;

	//============================================
	// 代表故障数初期化
	//============================================
	n_rep = n_net * 2;

	//============================================
	// テスト対象フラグ初期化
	//============================================
	for (i = 0; i < n_net; i++) {
		nl[i].test_sf0 = YES;
		nl[i].test_sf1 = YES;
	}

	//============================================
	// 等価故障解析(テスト対象から外していく)
	//============================================
	for (i = 0; i < n_net; i++) {
		switch (nl[i].type) {
			//------------------------------------------
		case BUF:
		case INV:
			//入力信号線の故障は等価
			nl[i].in[0]->test_sf0 = NO;
			nl[i].in[0]->test_sf1 = NO;

			//代表故障数を減らす
			n_rep -= 2;
			break;
			//------------------------------------------
		case AND:
		case NAND:
			//入力信号線の0縮退故障は等価
			for (j = 0; j < nl[i].n_in; j++) {
				nl[i].in[j]->test_sf0 = NO;
			}

			//代表故障数を減らす
			n_rep -= nl[i].n_in;
			break;
			//------------------------------------------
		case OR:
		case NOR:
			//入力信号線の1縮退故障は等価
			for (j = 0; j < nl[i].n_in; j++) {
				nl[i].in[j]->test_sf1 = NO;
			}

			//代表故障数を減らす
			n_rep -= nl[i].n_in;
			break;
			//------------------------------------------
		default:
			break;
			//------------------------------------------
		}

	}

}
