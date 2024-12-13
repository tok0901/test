//FFR構造体定義
typedef struct Fanout_FreeRegion {

	int							ffr_id;				//FFRのID
	int							n_out;				//他FFRへの出力数
	struct Fanout_FreeRegion**	out;				//他出力FFRへのポインタ
	int							n_in;				//他FFRへの入力数
	struct Fanout_FreeRegion**	in;					//他入力FFRへのポインタ
	int							n_pi;				//FFR内の外部入力数
	int							n_in_net;			//FFR内の入力数(入力はINとFOBの2種類　※他FFRからの入力では無い)
	int							po_flag;			//外部出力格納フラグ(1:FFR内に外部出力線を持つ,0:持たない)
	NLIST*						fos;				//FOUT_STEMへのポインタ
	unsigned long long int*		po_fault_flag;		//出力応答値(po,tpiの故障値を格納)
	int							n_unconf_fault_det;	//故障検出回数
	int							n_unconf_fault;		//未識別故障格納個数
	int							n_net_sim;			//格納信号線数

} FFR;

FFR* ffr;

void ffr_reset(void);
