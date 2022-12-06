---
title: "図解TLSF"
emoji: "✡️"
topics: [ "メモリアロケータ" ]
published: true
type: "tech"
---

世の中には様々なメモリアロケータがありますが，ここでは某ゲーム開発解説サイトでも紹介されているTLSF(Two Level Segregate Fit)メモリアロケータのソースコードを読み，
メモリ上ではどのようになっているのかを図解してみたいと思います．なお，元々はCのコードですが，いちいちstructをtypedefしないなど省略している部分があります．
また，具体的な数値を出すために，ここでは64bit環境でアドレス長が8バイトである前提としています．

# TLSF ([http://www.gii.upv.es/tlsf/](http://www.gii.upv.es/tlsf/))
ソースコードはTLSFの公式サイトから2.4.6をダウンロードしてきています．

# 基本的な使い方の流れ

TLSFを使う場合の基本的な流れは次のようになります．

1. init_memory_poolであらかじめ確保しているメモリ(これをメモリプールと呼ぶ)にTLSF管理用のデータ構造を構築する
2. malloc_exでメモリ確保，free_exでメモリ解放を行う
3. destroy_memory_poolでメモリプールを破棄する(メモリプールそのものの解放は外部の責任となる)

# メモリプールの初期化

最初にメモリプール上にTLSFの管理を行うデータ構造であるtlsf_tを構築します．
このあと詳しく解説しますが，tlsf_tはアドレス長が8バイトの環境では，
約6KiBのメモリを必要とするため，それよりも小さいサイズのメモリプールの場合，
初期化に失敗します．

また，メモリプールの先頭アドレスがアドレス長の倍数になっていない場合も初期化に失敗します．

この条件を満たしたら，メモリプールの先頭にtlsf_tのデータ構造が構築されます．

では，tlsf_tの構造体を見てみましょう．なお，実際のコードにある排他制御や統計のためのコードは省いてあります．

```cpp
struct tlsf_t
{
    // 初期化済みの場合に署名が入る
    uint32_t tlsf_signature;

    // エリア情報のリンクリストの先頭
    area_info_t *area_head;

    // ファーストレベルのビットマップ
    // そのレベルが空いている場合はクリアされる
    uint32_t fl_bitmap;

    // セカンドレベルのビットマップ
    // REAL_FLI == (MAX_FLI - FLI_OFFSET)
    // MAX_FLI == 30
    // FIL_OFFSET = 6
    // REAL_FLI == 24
    // FLI(First Level Index), SLI(Second Level Index)
    uint32_t sl_bitmap[REAL_FLI];

    // ブロックヘッダのリンクリストの先頭へのポインタを管理する2次元配列
    // MAX_SLI == (1 << MAX_LOG2_SLI)
    // MAX_LOG2_SLI == 5
    // MAS_SLI == 32
    bhdr_t* matrix[REAL_FLI][MAX_SLI];
};
```

メモリプールの先頭を0番地だとすると，次のようになります．

![tlsf_t](/images/tlsf_t.png)
*tlsf_t*

この図を書いてみて，area_headを後ろに持ってくるとパディングが消えるのでは，
とも思いましたが，他にもロックを追加できたり統計情報を追加できたりするので，
数バイトはいったん無視しても良いかなと．

このメモリプールの先頭がTLSF_SIGNATURE(== 0x2A59FA59)になっている場合，
初期化済みとして最初のブロックサイズが返されます．

```c
tlsf_t* tlsf = (tlsf_t *)mem_pool;
if(tlsf->tlsf_signature == TLSF_SIGNATURE){
    bhdr_t* b = GET_NEXT_BLOCK(mem_pool, ROUNDUP_SIZE(sizeof(tlsf_t)));
    return b->size & BLOCK_SIZE;
}
```

まず，ROUNDUP_SIZEについて見ておきましょう．ついでに関連するROUNDDOWN_SIZEについても見ていきます．
書いてある数値はあくまでも64bit環境でのものです．

```c
// BLOCK_ALIGN  == 16 == 0b10000
#define BLOCK_ALIGN (sizeof(void *) * 2)
// MEM_ALIGN == 15 == 0b01111
#define MEM_ALIGN ((BLOCK_ALIGN) = 1)
// 16の倍数に切り上げる
#define ROUNDUP_SIZE(_r) (((_r) + MEM_ALIGN) & ~MEM_ALIGN)
// 16の倍数になるよう切り捨てる
#define ROUNDDOWN_SIZE(_r) ((_r) & ~MEM_ALIGN)
```

ROUNDDOWN_SIZEのようにMEM_ALIGNを反転させたものとの&を取ると，
下位4ビットを削って16の倍数に切り下げることができます．
そして，ROUNDUP_SIZEのようにいったん15を足してから切り下げると
16の倍数に切り上げることができます．

2のべき乗の数Nについて，N - 1をMとして，適当な数X & ~MでNの倍数に切り下げ，
(X + M) & ~Mというのは良く使われるテクニックですね．

つまり上記tlsf_tの図によればsizeof(tlsf_t)が0x1878なので，
ROUNDUP_SIZE(sizeof(tlsf_t))は0x1880を返すわけです．

```c
bhdr_t* b = GET_NEXT_BLOCK(mem_pool, ROUNDUP_SIZE(sizeof(tlsf_t)));
```

では，次にGET_NEXT_BLOCKの実装を見てみましょう．これは非常に単純です．

```c
#define GET_NEXT_BLOCK(_addr, _r) ((bhdr_t *)((char *)(_addr) + (_r)))
```

まず，渡されたポインタをcharポインタにキャストして，渡されたオフセットバイト分進め，
そのアドレスをbhdr_t型へのポインタに変換して返します．
このbhdr_tはブロックヘッダを表す型です．この辺は時代を感じる命名ですね．

returnの部分については，ここではちょっとスキップします．
初期化の最後で同じようなreturnがあるので，bhdr_tの中身を見てから，そこで説明します．

さて，初期化済みのケースが済むと，tlsf_tの部分を0クリアした上で，
tlsf_signatureにTLSF_SiGNATUREを設定します．
これ，初期化終わってから設定した方が良いのでは，という気もしましたが，
マルチスレッドとかじゃなければまぁ良いかと．

初期化済みのケースが終わったので，実際の初期化に入ります．
最初に，process_areaという関数にtlsf_tの直後のアドレス
(ROUNDUPで16の倍数になっている)と
そのアドレスからメモリプールの末端までのサイズを渡します．

```cpp
bhdr_t *ib = process_area(
	GET_NEXT_BLOCK(mem_pool, ROUNDUP_SIZE(sizeof(tlsf_t))),
	ROUNDDOWN_SIZE(mem_pool_size - sizeof(tlsf_t))
);
```

process_areaの処理を見ていく前にbhdr_tの構造を見ておきます．

```cpp
// 未使用ブロックで前後の未使用ブロックを指す
struct free_ptr_t
{
    struct bhdr_t *prev;
    struct bhdr_t *next;
};

// ブロックのヘッダを表す
struct bhdr_t
{
    // 前のブロックへのポインタ
    struct bhdr_t *prev_hdr;
    // 下位2ビットはブロックが使用中かどうかと前のブロックが使用中かどうかに使われる
    size_t size;

    union
    {
        free_ptr_t free_ptr; // 未使用の場合は前後の未使用ブロックを指すのに利用される
        uint8_t buffer[1];   // 可変長領域の先頭を表す
    } ptr;
};

// エリアの情報を表す
strut area_info_t
{
    bhdr_t *end;
    area_info_struct *next;
};
```

area_info_tについては，サイズの計算などで必要なのでここで先に出しています．

メモリ上でのそれぞれの構造体のイメージは次のような感じです．

![free_ptr_t](/images/free_ptr_t.png)
![bhdr_t](/images/bhdr_t.png)
![area_info_t](/images/area_info_t.png)

未使用のブロックと使用中のブロックで，次のように使い分けられるわけです．

![bhdr_t](/images/tlsf_block.png)

uint8_t buffer[1]のようにサイズ1の配列を見ると意味がないように思えるかもしれませんが，
これは可変長の領域を表したい場合によく使われるテクニックです．

ptr.buffer[2]などのように範囲外にアクセスすることで実際のブロック部分にアクセスできるわけです．

では，process_areaの実装を見ていきます．
process_areaのインターフェースは次のようになっています．

```cpp
bhdr_t* process_area(void *area, size_t size);
```

まずはareaの先頭をブロックとみなしてbhdr_t*の変数ibを用意します．
(initial blockの意味でしょうか?)
```cpp
bhdr_t *ib = (bhdr_t *)area;
```

まずはこのibのサイズを求めるのですが，実際のコードは?:を使っていてややこしいので，
if文に置き換えています．

```cpp
// MIN_BLOCK_SIZE == sizeof(free_ptr_t) == 16
// sizeof(area_info_t) == sizeof(void *) * 2 == 16
if(sizeof(area_info_t) < MIN_BLOCK_SIZE)
{
	// 今の実装ではここに来ることは無い
	ib->size = MIN_BLOCK_SIZE;
}
else
{
	// sizeof(area_info_t)が16の倍数なのでROUNDUP_SIZEは何もしない
	ib->size = ROUNDUP_SIZE(sizeof(area_info_t));

}
// この時点でib->size == 16

// ブロックサイズの0ビット目はブロックを利用中かどうかに使用する
#define FREE_BLOCK (0x1)
#define USED_BLOCK (0x0)

// ブロックサイズの1ビット目は前のブロックが利用中かどうかに使用する
#define PREV_FREE (0x1 << 1)
#define PREV_USED (0x0 << 1)

ib->size |= USED_BLOCK;
ib->size |= PREV_USED;

// 0を設定しているだけなのでib->sizeは16のまま
```

このあと出てくるのですが，最初のブロックにはarea_info_tを格納するので，
その分のサイズを確保して使用済みのマークをします．

最初のブロックはエリア情報用のブロックなので，実際に利用するためのブロックを構築します．
```cpp
#define PTR_MASK   (sizeof(void *) - 1)    // 64bit環境では7，つまり0b0111
#define BLOCK_SIZE (0xFFFFFFFF - PTR_MASK) // ブロックサイズを8の倍数に揃えるのに使われる

// BLOCK_SIZEでマスクするとブロックサイズは4GiBまでに制限されるので注意が必要

bhdr_t *b = GET_NEXT_BLOCK(ib->ptr.bufer, ib->size & BLOCK_SIZE);

// ib->size & BLOCK_SIZEは8の倍数への切り捨てだが元々16は8の倍数なので
// 変化無し
```

bはibの直後になります．次にbのサイズを計算します．

```cpp
// BHDR_OVERHEADはbhdr_tから実際にブロックとして使われる部分のサイズを引いたサイズ
// 64bit環境では16
#define BHDR_OVERHEAD (sizeof(bhdr_t) - MIN_BLOCK_SIZE)
b->size = ROUNDDOWN_SIZE(size - 3 * BHDR_OVERHEAD - (ib->size & BLOCK_SIZE));
b->size |= USED_BLOCK;
b->size |= PREV_USED;
```

BHDR_OVERHEADは次の図の緑部分，bhdr_tの先頭部分のサイズです．

![bhdr_t_overhead](/images/bhdr_t_overhead.png)

初期化しようとしているエリアには，ibとbの他に，この後構築するlbを配置するため，
3 * BHDR_OVERHEADとなっています．更に既に使用済みのib->sizeを引くわけです．

ここでちょっと気になるのが，このブロックを使用中として扱うことです．
前のブロックはibなので使用中というのは分かります．
でも，何故このブロックが使用中なのでしょう?
しかもその直後に未使用ブロックとして初期化します．

```cpp
b->ptr.free_ptr.prev = nullptr;
b->ptr.free_ptr.next = nullptr;
```

この辺は，init_memory_poolに戻った後の処理に関係してくるので，
取り合えずそうなっていると覚えておきましょう．

次にlb (last blockでしょうか?)を構築します．

```cpp
bhdr_t *lb = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
```

```cpp
lb->size = 0 | USED_BLOCK | PREV_FREE;
```

最後に，ibのところに作る予定だったarea_info_tを構築します．

```cpp
area_info_t *ai = (area_info_t *)ib->ptr.buffer;
ai->next = nullptr;
ai->end = lb;
```

process_areaの最後ではメモリプールは次のような状態になっています．

![tlsf_area_image](/images/tlsf_area_image.png)

さて，init_memory_poolの処理に戻ります．

```cpp
bhdr_t *ib = process_area(
	GET_NEXT_BLOCK(mem_pool, ROUNDUP_SIZE(sizeof(tlsf_t))),
	ROUNDDOWN_SIZE(mem_pool_size - sizeof(tlsf_t))
);
```

構築したエリアのibが返ってきています．
そして，最初のブロックを取得します．

```cpp
bhdr_t *b = GET_NEXT_BLOCK(ib->ptr.buffer, ib->size & BLOCK_SIZE);
```
USED_BLOCKやPREV_USEDなどのフラグがサイズに入っているので，BLOCK_SIZEでマスクするのを忘れずに．

このブロックは，process_area内で使用中扱いになっているので，
ここでfree_exを呼び出して，解放します．

```cpp
free_ex(b->ptr.buffer, tlsf);
```

では，free_exの中を見ていきましょう．
インターフェースは次のようになっています．

```cpp
void free_ex(void *ptr, void *mem_pool);
```

まずは，渡されたptrからブロックヘッダを特定します．
これは簡単で，BHDR_OVERHEAD分戻ったところがブロックヘッダの先頭です．

```cpp
bhdr_t *b = (bhdr_t *)((char *)ptr - BHDR_OVERHEAD);
```

そして，ブロックが解放済みであることをsizeに設定し，未使用ブロックとして
free_ptrをクリアします．

```cpp
b->size |= FREE_BLOCK;
b->ptr.free_ptr.prev = nullptr;
b->ptr.free_ptr.next = nullptr;
```

そして，次のブロックヘッダを取得します．
```
bhdr_t *tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
// 初期化の場合はlbが得られる
```

ソースコードとしては，このあとのブロックが未使用の場合の処理があるのですが，
初期化の場合はlbが返ってきて，USED_BLOCKであることが分かっているので
スキップします．

次に，bの前が未使用の場合の処理があるのですが，これも初期化の場合は
ibがあって使用中と分かっているのでスキップします．

初期化時には通らない処理をスキップしたあとは，このメモリプールのサイズをもとに，
TLSFに登録するため，MAPPING_INSERTを呼び出してfirst levelとsecond levelを計算します．

この2つの数値を使うのがTLSFがTwo Levelと言う理由です．

```cpp
int fl = 0; // first level
int sl = 0; // second level
MAPPING_INSERT(b->size & BLOCK_SIZE, &fl, &sl);
```

では，MAPPING_INSERTの中を見てみましょう．

```cpp
#define SMALL_BLOCK  (128)
#define FLI_OFFSET   (6)
#define MAX_LOG2_SLI (5)
#define MAX_SLI      (1 << MAX_LOG2_SLI) // 32
VOID MAPPING_INSERT(size_t _r, int *_fl, int *_sl)
{
    if(_r < SMALL_BLOCK)
    {
        // SMALL_BLOCKサイズ以下のブロックはfirst level 0に押し込む
        *_fl = 0;
	*_sl = _r / (SMALL_BLOCK / MAX_SLI);
    }
    else
    {
        *_fl = ms_bit(_r); // 1になっているビットの最大インデックスを取得する
	*_sl = (_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
	*_fl -= FLI_OFFSET;
    }
}
```

ここで，このコードを理解するためにTLSFの論文から式を引用します．
ソースコードに合わせて，SLIの部分をMAX_LOG2_SLIに置き換えています．

$$fl = \lfloor \log_2(size)\rfloor$$
$$sl = (size - 2^{fl})\frac{2^{MAX\_LOG2\_SLI}}{2^{fl}}$$

論文では，例として460をflとslに分解しています．

まず460を次のように2のべき乗の和にします．
$$460 = 0b1\_1100\_1100 = 2^8 + 2^7 + 2^6 + 2^3 + 2^2$$

すると，次のような関係式が成り立ちます．

$$\log_2 2^8 < \log_2 460 < \log_2 2^9$$

このことから，flは次のように求まります．

$$fl = \lfloor \log_2 460 \rfloor = 8$$

つまり，flはsizeを表す2進数の式で最大の$2^N$のNを得れば良い．

都合の良いことにWindowsには_BitScanReverseという最上位ビットから
最下位ビットに向かって1となっているビットを探し，
そのインデックスを最下位ビットからのインデックスで返す関数があります．
clangやgccにも似たような関数があるため，これを求めるのはたやすいことです．

次に，slを求める式を少し変形してみます．

$$sl = (size - 2^{fl})2^{MAX\_LOG2\_SLI - fl}$$

これを展開すると，次の式になります．

$$sl = size \cdot 2^{MAX\_LOG2\_SLI - fl} - 2^{MAX\_LOG2\_SLI - fl + fl}$$
$$sl = size\cdot 2^{MAX\_LOG2\_SLI - fl} - 2^{MAX\_LOG2\_SLI}$$
$$sl = \frac{size}{2^{fl - MAX\_LOG2\_SLI}} - MAX\_SLI$$

2のべき乗による除算は右シフトとみなせるので，
最終的に次のようなコードが出来上がります．

```cpp
*_fl = ms_bit(_r); // 1になっているビットの最大インデックスを取得する
*_sl = (_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
*_fl -= FLI_OFFSET;
```

ただし，下位6ビットで表せる小さいサイズのブロックは0とみなすようになっているので，
最後にFLI_OFFSETを引いています．
FLI_OFFSETを先に引いてしまうと計算が合わないので注意しましょう．

ここで，sizeが128未満の場合，slはSMALL_BLOCK / MAX_SLI == 4で割るようになっています．

とにかく，新しい未使用ブロックを追加する準備が出来たので，
次はINSERT_BLOCKというマクロを呼び出して，実際にブロックを追加します．

```cpp
INSERT_BLOCK(b, tlsf, fl, sl);
// 実際には次のような処理になる
b->ptr.free_ptr.prev = nullptr;
b->ptr.free_ptr.prev = tlsf->matrix[fl][sl]; // 初期化時点ではnullptr
if(tlsf->matrix[fl][sl]) // 初期化時点ではnullptrなので通らない
{
    tlsf->matrx[fl][sl]->ptr.free_ptr.prev = b;
}
tlsf->matrix[fl][sl] = b; // 該当するサイズのフリーブロックとして登録
// sl_bitmapに未使用領域があるとビットを立てる
set_bit(sl, &tlsf->sl_bitmap[fl]);
// fl_bitmapに未使用領域があるとビットを立てる
set_bit(fl, &tlsf->fl_bitmap);
```

これで，tlsfの管理用のデータ構造に用意したブロックが登録されました．

最後にこれは不要なのですが，lbをtmp_bとして取得し，sizeに
PREV_FREEを設定し，bをlbの前のブロックとして登録する処理があります．
これは，free_exを流用しているが故の処理ですね．

init_memory_mapに戻り，最後にibに作ったarea_info_tへのポインタを
tlsfのarea_headに登録したら，ようやくメモリプールの初期化が完了です．

最終的にメモリプールは次のような状態になっています．

![tlsf_init_image](/images/tlsf_init_image.png)

この図を見ていると，いくつかのprev_hdrを初期化していないので若干不安になりますが，
この後の処理を見ていくと問題無いことが分かります．

# malloc_exでメモリを確保してみる

では，早速このTLSFを使ってメモリを確保してみます．
TLSFでメモリを確保するには，次のようなmalloc_ex関数を呼びます．

```cpp
void *malloc_ex(size_t size, void *mem_pool);
// mem_poolはtlsfの先頭アドレス
```

まず，sizeをMIN_BLOCK_SIZE(==16)以上の16の倍数に補正します．

```cpp
if(size < MIN_BLOCK_SIZE)
{
    size = MIN_BLOCK_SIZE;
}
else
{
    size = ROUNDUP_SIZE(size);
}
```

次に，このサイズが入るべきブロックを探すため，MAPPING_SEARCHという関数を
呼び出します．

```cpp
MAPPING_SEARCH(&size, &fl, &sl);
```

この関数を呼び出すと，flとsl以外に，sizeも変化することがあります．
具体的に処理の中を見てみます．

```cpp
void MAPPING_SEARCH(size_t *_r, int *_fl, int *_sl)
{
    if(*r < SMALL_BLOCK)
    {
        *_fl = 0;
	*_sl = *r / (SMALL_BLOCKC / MAX_SLI);
    }
    else
    {
        int _t = (1 << (ms_bit(*_r) - MAX_LOG2_SLI)) - 1;
	*_r = *r + _t;
	*_fl = ms_bit(*_r);
	*_sl = *(_r >> (*_fl - MAX_LOG2_SLI)) - MAX_SLI;
	*_fl -= FLI_OFFSET;
	*_r &= ~_t;
    }
}
```

sizeがSMALL_BLOCKより小さい場合についてはMAPPING_INSERTで同じ処理を見たので
スキップします．

次に_tに関する部分は，次のように書くと何となくどこかで見たことがあるような気がしませんか?

```cpp
int _t = (1 << (ms_bit(*_r) - MAX_LOG2_SLI)) - 1;
*_r = *_r + _t;
*_r &= ~_t;
```

そうROUNDUP_SIZEと同じです．少し具体的な数字で計算してみましょう．
128，つまり$2^7$の場合,(1 << (7 - 5)) - 1 == 3で4の倍数になります．

|size|倍数|
|----|----|
|128|4|
|256|8|
|512|16|
|1024|32|

ただし，ここで注意したいのが，元々sizeは16の倍数だったということです．
そのため実際には，1023までは_tは何も変化しないわけです．
つまり，テーブルは次のようになります．

|size|倍数|
|----|----|
|128|16|
|256|16|
|512|16|
|1024|32|
|2048|64|


残りの式はMAPPING_INSERTのときに見たものと同じです．
つまり，元のサイズに少し倍数に合うようなサイズを足した上で，
flとslを決めるわけです．

flとslが決まったら，次はFIND_SUITABLE_BLOCKを呼んで適切なブロックを見つけます．
もちろん見つからなかったらNULLを返します．

```
bhdr_t *b = FIND_SUITABLE_BLOCK(tlsf, &fl, &sl);
if(!b)
{
    return nullptr;
}
```

では，このFIND_SUITABLE_BLOCKの処理を見てみましょう．

```cpp
bhdr_t *FIND_SUITABLE_BLOCK(tlsf_t *_tlsf, int *_fl, int *_sl)
{
    bhdr_t *_b = nullptr;

    uint32_t _tmp = _tlsf->sl_bitmap[*_fl] & (~0 << *_sl);

    if(_tmp)
    {
        *_sl = ls_bit(_tmp);
        _b = _tlsf->matrix[*_fl][*_sl];
    }
    else
    {
        *_fl = ls_bit(_tlsf->fl_bitmap & (~0 << (*_fl + 1)));
        if(*_fl > 0) /* likely */
        {
	    *_sl = ls_bit(_tlsf->sl_bitmap[*_fl]);
	    _b = _tlsf->matrix[*_fl][*_sl];
        }
    }

    return _b;
}
```

まず，sl_bitmap[fl]からsl番目より上のビット全てを取り出します．

```cpp
uint32_t _tmp = _tlsf->sl_bitmap[*_fl] & (~0 << *_sl);
```

何かしらビットマップにビットが立っていれば，その一番下のビットの部分を使うようにします．

```cpp
if(_tmp)
{
    *_sl = ls_bit(_tmp);
    _b = _tlsf->matrix[*_fl][*_sl];
}
```

空きがない場合，fl_bitmapからflより上のビットを取り出し，その一番下のbitの位置を新しいflにします．
今のflより下はサイズが小さくて足りないことが分かり切っているので対象にはなりません．

```cpp
*_fl = ls_bit(_tlsf->fl_bitmap & (~0 << (*_fl + 1)));
```

さて，このときほとんどの確率で見つかるということでlikelyというコメントが書いています．
コンパイラによっては最適化されて高速に動作することがあります．
fl_bitmapが全て0の場合は，未使用ブロックがないということなので諦めるしかないです．

flを更新したので，slはsl_bitmapで更新しておきます．

```cpp
if(*_fl > 0) /* likely */
{
    *_sl = ls_bit(_tlsf->sl_bitmap[*_fl]);
    _b = _tlsf->matrix[*_fl][*_sl];
}
```

ここまでで，適切なブロックを探す処理は完了です．

気付いたかもしれませんが，ここまでwhileや再帰などはしていないので，
定数時間で適切なブロックを見つけられる，ということです．

ブロックが見つかったので，そのブロックをEXTRACT_BLOCK_HDRマクロで取り出します．

```cpp
_tlsf->matrix[_fl][_sl] = _b->ptr.free_ptr.next;
if(_tlsf->matrix[_fl][_sl])
{
    _tlsf->matrix[_fl][_sl]->ptr.free_ptr.prev = nullptr;
}
```

最初に，未使用ブロックが指している次の未使用ブロックヘッダをtlsf->matrixの先頭として
つなぎかえます．
実際に先が存在している場合は，先のブロックには前の未使用ブロックとして自身を指しているので，
NULLにしておきます．

図で言うと次のような感じです．

![tlsf_extract_block_hdr_01](/images/tlsf_extract_block_hdr_01.png)

この状態から次の図の状態になるわけです．

![tlsf_extract_block_hdr_02](/images/tlsf_extract_block_hdr_02.png)

一方，先がない場合，そのflとslの組み合わせでの未使用ブロックが
無くなったということなので，sl_bitmap[_fl]をclear_bitでクリアしておきます．

そして，sl_bitmapをクリアした結果，0になった場合，そのflにある未使用ブロックが
無くなったということなので，fl_bitmapもクリアしておきます．

```cpp
if(!_tlsf->sl_bitmap[_fl])
{
    clear_bit(_fl, &_tlsf->fl_bitmap);
}
```

最後に取り出したブロックのfree_ptrの部分はnullptrでクリアしておきます．

ブロックを取り出したら，init_mem_poolのときに見たように，そのブロックを
挟んでいるブロックヘッダがあるので，そちらをnext_bとして取り出します．

```cpp
bhdr_t *next_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
```

そして，ブロックのサイズから必要なサイズを引いたサイズが
ブロックヘッダサイズよりも大きいのであれば，
ブロックを分割します．

```cpp
tmp_size = (b->size & BLOCK_SIZE) - size;
if(tmp_size >= sizeof(bhdr_t))
{
    // 新しいヘッダの分はサイズから引いておく
    tmp_size -= BHDR_OVERHEAD;

    // sizeは事前に適当な倍数にしているので
    // アライメントで問題になることはない
    bhdr_t* b2 = GET_NEXT_BLOCK(b->ptr.buffer, size);
    b2->size = tmp_size | FREE_BLOCK | PREV_USED;

    next_b->prev_hdr = b2;

    // 新規ブロックのサイズに合わせて登録しておく
    // つまりb2が該当するfl，slのmatrixの先頭に入る
    MAPPING_INSERT(tmp_size, &fl, &sl);
    INSERT_BLOCK(b2, tlsf, fl, sl);


    // PREV_STATEはPREV_FREEまたはPREV_USEDを取り出すための
    // ビットマスク
    // 本当はUSED_BLOCKも設定した方が分かりやすいとは思うが
    // 元々sizeの下位が0なので設定していないと思われる
    b->size = size | (b->size & PREV_STATE);
}
```

イメージとしては，次のようにブロックの途中にヘッダが出来る感じです．

![tlsf_split_block](/images/tlsf_split_block.png)

そして，ブロックを分割しない場合，そのブロックは余りも含めて使えなくなるので，
終端のブロックのPREV_FREEを外した上で，ブロック全体を使用状態にする．

```
if(tmp_size >= sizeof(bhdr_t))
{
    ...
}
else
{
    // 終端の前のブロックは使用済みになる
    next_b->size &= ~PREV_FREE;

    // ブロックは利用中
    b->size &= ~FREE_BLOCK;
}
```

最後に，blockのbufferの位置を利用できるポインタとして返せばmallocの完了です．

```cpp
return (void *) b->ptr.buffer;
```

# free_exでメモリを解放してみる

init_memory_poolでも軽く見ましたが，今度はスキップした処理も含めて
free_exの実装を見ていきます．

まず，与えられたアドレスからブロックヘッダを取り出し，
未使用ブロックとし，free_ptrをクリアしておく部分は
init_memory_poolで見ました．

```cpp
bhdr_t *b = (bhdr_t *)((char *)ptr - BHDR_OVERHEAD);
b->size |= FREE_BLOCK;

b->ptr.free_ptr.prev = nullptr;
b->ptr.free_ptr.next = nullptr;
```

そして，そのブロックの次のブロックを求めます．

```cpp
bhdr_t *tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
```

そして，次のブロックが未使用ブロックの場合，今のブロックと結合します．

```cpp
if(tmp_b->size & FREE_BLOCK)
{
    // 次のブロックのflとslを求める
    MAPPING_INSERT(tmp_b->size & BLOCK_SIZE, &fl, &sl);
    // 次のブロックをtlsfのmatrixから抜き取っておく
    EXTRACT_BLOCK(tmp_b, tlsf, fl, sl);
    // 次のブロックのサイズとヘッダの部分のサイズを足しておく
    b->size += (tmp_b->size & BLOCK_SIZE) + BHDR_OVERHEAD;
}
```

イメージとしては，次のような感じです．

![tlsf_merge_next_block](/images/tlsf_merge_next_block.png)

tmp_bの次のブロックも未使用ブロックだったなら結合した方が
良いのでは，と思うかもしれませんが，そもそもtmp_bの次の
ブロックが未使用ブロックであれば，tmp_bと結合済みでないとおかしいのです．

さて，malloc_exでEXTRACT_BLOCK_HDRは出てきましたが，EXTRACT_BLOCK
は何が違うのでしょうか?

まずはEXTRACT_BLOCKの実装を見てみましょう．

```cpp
if(_b->ptr.free_ptr.next)
{
    _b->ptr.free_ptr.next->ptr_free_ptr.prev = ->b->ptr.free_ptr.prev;
}
```

まず，bの次の未使用ブロックがある場合，その未使用ブロックの前の
未使用ブロックをbではなくbの前の未使用ブロックに変更します．

イメージとしては次の図の赤い線のようになります．

![tlsf_extract_block_01](/images/tlsf_extract_block_01.png)

同様にbの前の未使用ブロックがあれば，そちらの次の未使用ブロックは
bではなくbの次の未使用ブロックを指すようにします．

イメージとしては次の図の緑の線のようになります．

![tlsf_extract_block_01](/images/tlsf_extract_block_02.png)

これでbはを切り離すことができそうです．

次に，取り出すブロックが`tlsf->matrix[fl][sl]`の先頭の場合を考えます．
この場合，それより前の未使用ブロックは無い状態です．

先頭のブロックを更新し，必要に応じてsl_bitmapやfl_bitmapを更新します．

```cpp
if(_tlsf->matrix[_fl][_sl] == _b)
{
    _tlsf->matrx[_fl][_sl] = _b->ptr.free_ptr.next; // 次の未使用ブロックをmatrixに登録しておく
    if(!_tlsf->matrix[_fl][_sl])
    {
        // 次の未使用ブロックが見つからなかった場合，
	// そのサイズに該当するブロックが無いので
	// sl_bitmapをクリアしておく
        clear_bit(_sl, &_tlsf->sl_bitmap[_fl]);

	if(!_tlsf->sl_bitmap[_fl])
	{
	    // sl_bitmapが0になった場合はそのflにも
	    // 空きが無いのでfl_bitmapもクリアしておく
	    clear_bit(_fl, &_tlsf->fl_bitmap);
	}
    }
}
```

最後に，free_ptrのprevとnextをNULLでクリアして完了です．
これでEXTRACT_BLOCKは観終わりました．

次に，前のブロックが未使用の場合をチェックします．
前のブロックが未使用ブロックかどうかはsizeのビット1を見ればわかります．
そして，その場合はprev_hdrが前のブロックヘッダを指しています．

```cpp
if(b->size & PREV_FREE)
{
    // 前のブロックヘッダをtmp_bとして更新する
    tmp_b = b->prev_hdr;

    // 次のブロックが未使用ブロックだった場合と同様に
    // ブロックを独立させておく
    MAPPING_INSERT(tmp_b->size & BLOCK_SIZE, &fl, &sl);
    EXTRACT_BLOCK(tmp_b, tlsf, fl, sl);

    // 前にある未使用ブロックであるtmp_bの方のサイズを増やす
    tmp_b->size += (b->size & BLOCK_SIZE) + BHDR_OVERHEAD;

    // bを前にあるtmp_bに更新する
    b = tmp_b;
}
```

コメントにあるように前に未使用ブロックがある場合，
ブロックヘッダへのポインタをそちらに更新します．

未使用ブロックの結合が終わったら，tlsfのmatrixに登録します．
ここはinit_memory_poolで見たのでコードを見るだけにします．

```cpp
MAPPING_INSERT(b->size & BLOCK_SIZE, &fl, &sl);
INSERT_BLOCK(b, tlsf, fl, sl);
```

最後に，次のブロックに未使用ブロックになったことを設定しておきます．

```cpp
tmp_b = GET_NEXT_BLOCK(b->ptr.buffer, b->size & BLOCK_SIZE);
tmp_b->size |= PREV_FREE;
tmp_b->prev_hdr = b;
```

これでfree_exの処理は完了です．

ここまで一度もwhileなどのループは出てきていません．
つまり，処理はサイズなどのパラメータによらず定数時間で
処理できるということです．

# まとめ

本当はrealloc_exとかもあるんですが力尽きました．

TLSFはとある事情でちょくちょく名前を聞いていたんですが，
ここまで細かいところまでしっかりと読んだのは初めてです．

これで今後はしっかりと説明ができますね．

ただ，このTLSFにはアライメントを指定したメモリ領域を確保する方法がありません．
また，メモリリークしている場合のチェックもありません．

tlsf_tに6KiB近く必要とするということで，少しのメモリを管理するには向きません．

メモリの管理情報が利用するメモリと連続しているため，簡単に破壊できます．

そういった課題を解決しようとすると，もう少し工夫する必要があるので，
チャレンジしてみるのも楽しいかもしれません．
