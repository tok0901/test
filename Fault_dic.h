//故障辞書構造体定義
typedef struct Fault_dic {
	int tp;						//テストパターン番号
	NLIST** fault;				//検出故障
	int* saf_flag;				//検出故障フラグ(0縮退故障or1縮退故障)
	int n_fault;				//検出故障数
	NLIST*** unconf_fault;		//未識別故障集合
	int** unconf_saf_flag;		//検出未識別故障フラグ(0縮退故障or1縮退故障)
	char** po_value;			//ハッシュ表(外部出力値or剰余値)
	int* n_unconf_fault;		//未識別故障数(ハッシュ表内)
	int n_grp;					//未識別故障集合数
	int insert_number;			//検出故障格納インデックス番号

}DICT;

DICT* dic;


//未識別故障ペア格納ハッシュ配列構造体定義
typedef struct Unconf_Fault_Pair {
	int** saf_flag;				//検出故障フラグ
	NLIST*** unconf_fault;		//未識別故障集合
	int* n_unconf_fault;		//未識別故障ペア数
	int n_grp;					//未識別故障集合数
	int insert_number;			//新規検出故障格納インデックス番号
	int confirm_number;			//識別細分化故障格納インデックス番号


}HASH;

HASH hash;

char* Pulse_Output_Value(int tst_number, NLIST** sort_net);
int hash_insert_number(int test_number, int tst_number, char* temp_po);
