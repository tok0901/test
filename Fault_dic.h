//故障辞書構造体定義
typedef struct Fault_dic {
	int tp;						//テストパターン番号
	NLIST*** unconf_fault;		//未識別故障集合
	int* n_unconf_fault;		//未識別故障数
	short** unconf_saf_flag;		//検出故障フラグ(0縮退故障or1縮退故障)
	char** po_value;			//ハッシュ表(外部出力値or剰余値)
	int n_grp;					//未識別故障集合数
	int insert_number;			//検出故障格納要素番号
	int** hash_number;			//未識別故障格納ハッシュ番号(対象故障-未識別故障ペア配列の要素番号)

}DICT;

DICT* dic;


//未識別故障ペア配列構造体定義
typedef struct Unconf_Fault_Pair {
	short** saf_flag;				//検出故障フラグ
	NLIST*** unconf_fault;		//未識別故障集合
	int* n_unconf_fault;		//未識別故障数
	int* n_index;				//最大要素数
	int n_grp;					//未識別故障集合数
	int insert_number;			//新規未識別故障集合格納要素番号
	int confirm_number;			//細分化未識別故障集合格納要素番号
	short** confirm_flag;			//細分化フラグ(1:識別細分化可能,other:識別細分化不能)
	short* n_confirm_flag;		//細分化可能フラグ数(細分化未識別故障数:n_unconf_fault以上の時は細分化操作不能)


}HASH;

HASH hash;

//make_hash_tbl内関数プロトタイプ宣言
char* Pulse_Output_Value(int tst_number, NLIST** sort_net);
void hash_insert(int test_number, int tst_number, char* temp_po);
void hash_reset(int test_dic);

//外部変数

double all_conf_fault_pair;		//全識別故障ペア数
double unconf_fault_pair;		//未識別故障ペア数
double conf_fault_module;		//識別故障集合数
double n_sim_fault;				//全検出故障数
