#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Netlist.h"
#include "Queue.h"
#include "Command.h"


//ネットリスト読込み
int command(int argc, char** argv);
int input_f(char* tp, char* pin, char* v);
int make_net(NLIST* nl);


//キュー操作
int queue_empty(void);
void queue_enqueue(NLIST* signali);
NLIST* queue_dequeue(void);
int queue_overflow(void);


//ネットリスト_レベルソート
void quick_sort(NLIST** sort_net, int left, int right);
void swap(NLIST** sort_net, int i, int j);


//故障シミュレーション
int devide_ffr(NLIST** sort_net);
int losic_simulation(int test_number,int sim_test, NLIST** sort_net);
void gate_calc(int tst_number, int net_number, NLIST** sort_net);
int SAF_PPSFP(int test_number,int sim_test);
int SAF_CPT1(NLIST* sim_fos,int test_number,int sim_test);
int SAF_CPT0(NLIST* sim_fos,int test_number,int sim_test);
