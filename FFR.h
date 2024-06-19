//FFR構造体定義
typedef struct Fanout_FreeRegion {

	int						ffr_id;				//FFRのID
	int						n_out;				//他FFRへの出力数
	struct FoutFreeRegion** out;				//他出力FFRへのポインタ
	int						n_in;				//他FFRへの入力数
	struct FoutFreeRegion** in;					//他入力FFRへのポインタ
	int						n_pi;				//FFR内の外部入力数
	int						n_in_net;			//FFR内の入力数(入力はINとFOUTブランチの2種類　※他FFRからの入力では無い)
	NLIST* fos;				//FOUT_STEMへのポインタ

}FFR;

FFR* ffr;
