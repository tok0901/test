//未識別故障ペア配列構造体定義
typedef struct Unconf_Fault_Pair {
	NLIST*** unconf_fault;		//未識別故障集合
	ULLI** unconf_saf_fault;	//縮退故障値
	int* n_unconf_fault;		//未識別故障数
	int* n_index;				//最大要素数
	int n_grp;					//未識別故障集合数
	int insert_number;			//新規未識別故障集合格納要素番号
	int confirm_number;			//細分化未識別故障集合格納要素番号
	short** confirm_flag;			//細分化フラグ(1:識別細分化可能,other:識別細分化不能)
	short* n_confirm_flag;		//細分化可能フラグ数(細分化未識別故障数:n_unconf_fault以上の時は細分化操作不能)


}HASH;

HASH hash;

int Confirm_Fault_Search(int test_number, int hash_number, int fault_number);		//ハッシュ内故障探索関数
void Subdivide_Judge(int test_number, int hash_number, int* devide_hash_number);	//未識別故障ペア細分化判定関数
void hash_reset_DICT(int test_number);												//故障辞書削除関数
