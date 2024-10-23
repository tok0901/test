//------------------------------------------------------------------------
//Netlist 定義, 外部変数, プロトタイプ宣言
//File name : Nelist.h
//Date : 2006/03/26
//Designer : R.Inoue
//Ver : 3.00（配布Ver）
//------------------------------------------------------------------------
//------------------------------------------------------------------------
// 定義
//------------------------------------------------------------------------
//ゲートタイプ
#define  IN		0
#define  BUF    10
#define  INV	11
#define  AND	12
#define  NAND	13
#define  OR		14
#define  NOR	15
#define  EXOR	16 //2入力のみ可
#define  EXNOR	17 //2入力のみ可
#define  FOUT	18
#define  DFF	19 //D-フリップフロップ
#define  RDFF	21 //D-フリップフロップ（リセット有）
#define  DFFS	23 //D-スキャンフリップフロップ
#define  RDFFS	24 //D-スキャンフリップフロップ（リセット有）
#define  GND	25 //グランド
#define  ACC	26 //電源

//追記
typedef unsigned long long int ULLI;
#define BIT_64	64
//////

//ネットリスト構造体定義
typedef struct _Netlist_Format_ {
	char* name;							//名前
	int n;								//ID（ユニーク）
	int type;							//タイプ
	int n_in;							//入力数
	struct _Netlist_Format_** in;		//入力ポインタ配列
	int n_out;							//出力数
	struct _Netlist_Format_** out;		//出力ポインタ配列
	char* name_port;					//端子名 pin名
	char* name_ins;						//インスタンス名

	//追記///

	//論理シミュレーション
	int level;							//ゲートレベル
	ULLI val;							//格納論理値(64bit)

	//同時故障シミュレーション
	//struct _Netlist_Format_** fault;	//検出故障
	//int n_fault;						//検出故障数
	//int* sa_flag;						//検出故障フラグ
	//int dic_flag;						//検出故障重複フラグ(自身を対象)

	//二重検出
	//int sum_fault_0;						//0縮退故障検出回数
	//int sum_fault_1;						//1縮退故障検出回数
	//int es_fault_0_flag;				//必須0縮退故障フラグ
	//int es_fault_1_flag;				//必須1縮退故障フラグ

	//FFR分割
	int ffr_id;

	//CPT
	ULLI det;

	//PPSFP
	ULLI fault_flag;					//故障伝搬フラグ
	short sim_fault0_flag;				//0縮退故障検出フラグ
	short sim_fault1_flag;				//1縮退故障検出フラグ

	//TPI
	short score_flag;					//TPIスコアフラグ(有効:total_scoreへ加算しない)
	double score[2];					//TPIスコア(score[0]にf1,score[1]に)
	double total_score;					//合計TPIスコア
	short tpi_flag;						//TPIフラグ(有効:Test Pointとして選択)


} NLIST;


//------------------------------------------------------------------------
// 外部変数
//------------------------------------------------------------------------
//ネットリスト
NLIST* nl;

//モジュール名
char* module_name;

//信号数
int n_net;

//外部入力数, 外部入力
NLIST** pi;
int n_pi;

//外部出力数, 外部出力
NLIST** po;
int n_po;

//DFF, DFF数
NLIST** dff;
int n_dff;

//RDFF, RDFF数
NLIST** rdff;
int n_rdff;

//DFFS, DFFS数
NLIST** dffs;
int n_dffs;

//RDFFS, RDFFS数
NLIST** rdffs;
int n_rdffs;

//assign
NLIST** assign;
int n_assign;


//追記//
// 
NLIST** sort_net;//ソート配列

int n_test;//テストパターン数

//int o_test;//圧縮テストパターン数
//
//int d_test;	//最終圧縮テストパターン数

double not_fault; //未検出故障数

char** Str_test;//テストパターン配列

char** Str_pin;//外部入力ピン配列

char** test_tbl;//論理値格納配列

int n_ffr;//ffr数

int n_tpi_po;//TPI後出力応答ビット数

int n_64bit;//64bit列数(出力応答値bit列数:0~64=1 , 65~128=2)

NLIST** tpi_po_net;//観測ポイント配列

int n_tpi;//観測ポイント数

NLIST** tpi_net;

#define _CRTSECURE_NO_WARNINGS
#pragma warning(disable:4996)


//追記終了//

//------------------------------------------------------------------------
// プロトタイプ宣言
//------------------------------------------------------------------------
int read_nl(char*);

//追記//
void printBinary(ULLI n,int sim_test);	//2進数表示関数
int bit_print_N(ULLI n, int N);			//ビット列数値算出関数(Nビット目の値を返す:0 or 1)
ULLI bit_setting_1(ULLI n, int N);	//ビット列数値加算関数(Nビット目の値を1にしてn'を返す)
ULLI bit_setting_0(ULLI n, int N);	//ビット列数値加算関数(Nビット目の値を0にしてn'を返す)
int bit_search_N(ULLI n, int N);		//ビット列数値探索関数(Nビット目の値を返す:0or1)
void bit_count_64(void);				//出力応答値ビット列数更新関数
void print_po_val(ULLI* n);				//出力応答値標準出力関数
int strcmp_po_val(ULLI* x, ULLI* y);	//出力応答値比較関数(0:一致,1:不一致)
int count_number_64(int n);			//64bit列数算出関数

//追記終了//
