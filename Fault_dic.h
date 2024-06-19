typedef struct Fault_dic {
	int tp;						//テストパターン番号
	NLIST** fault;				//検出故障
	int* saf_flag;				//検出故障フラグ(0縮退故障or1縮退故障)
	int n_fault;				//検出故障数
	//int es_flag;				//必須故障検出フラグ		1:必須故障を持つ,0:必須故障を持たない

}DICT;

DICT* dic;
