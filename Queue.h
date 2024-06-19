 struct queue
{
	int max; //キューのサイズ
	int num; //現在の要素数
	int front; //先頭要素のインデックス
	int rear; //末尾要素のインデックス
	NLIST** que;//キュー
 }; struct queue Que;

 //キュー操作
 int queue_empty(void);
 void queue_enqueue(NLIST* signali);
 NLIST* queue_dequeue(void);
 int queue_overflow(void);

#define _CRTSECURE_NO_WARNINGS
#pragma warning(disable:4996)
