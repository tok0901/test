//故障辞書構造体定義
typedef struct Fault_dic {
	int tp;						//テストパターン番号
	NLIST*** unconf_fault;		//未識別故障集合
	int* n_unconf_fault;		//未識別故障数
	ULLI** unconf_saf_fault;	//縮退故障値(0縮退故障:0, 1縮退故障:1)
	ULLI** po_val;				//ハッシュ表(出力応答値64bit×64テストパターン×剰余分)
	int n_grp;					//未識別故障集合数
	int insert_number;			//検出故障格納要素番号
	int** hash_number;			//未識別故障格納ハッシュ番号(対象故障-未識別故障ペア配列の要素番号)

}DICT;

DICT* dic;

//make_DICT
void malloc_DICT(int test_number, int sim_test);
ULLI* calc_po_val(int test_dic,NLIST* sim_net,int tpi_flag);
ULLI* observate_po_val(int test_dic,int flag);
void hash_insert_DICT(int test_dic, ULLI* temp);
void fault_detect_DICT(int test_dic, NLIST* sim_net);

//外部変数

double all_conf_fault_pair;		//全識別故障ペア数
double unconf_fault_pair;		//未識別故障ペア数
double conf_fault_module;		//識別故障集合数
double n_sim_fault;				//全検出故障数
