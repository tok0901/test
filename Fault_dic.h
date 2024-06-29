//故障辞書構造体定義
typedef struct Fault_dic {
	int tp;						//テストパターン番号
	NLIST** fault;				//検出故障
	int* saf_flag;				//検出故障フラグ(0縮退故障or1縮退故障)
	int n_fault;				//検出故障数
	NLIST*** unconf_fault;		//未識別故障集合
	int** unconf_saf_flag;		//検出未識別故障フラグ(0縮退故障or1縮退故障)
	int n_unconf_fault;			//未識別故障数
	//int es_flag;				//必須故障検出フラグ		1:必須故障を持つ,0:必須故障を持たない

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

