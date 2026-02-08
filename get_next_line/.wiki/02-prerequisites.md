# 02 - 前提知識

この章では、get_next_lineの実装に必要なC言語の前提知識を
網羅的に解説します。特にポインタとメモリの理解を、
バイトレベルのASCII図を使って徹底的に深掘りします。

---

## 1. ポインタの完全理解

### 1.1 ポインタとは何か

ポインタは**メモリアドレスを格納する変数**です。
64ビットシステムでは、ポインタは8バイト（64ビット）の変数で、
メモリ上の特定のバイトの位置を指し示します。

```
メモリ空間（一部を抜粋）:

  アドレス         内容
  0x7ffc00001000   [0x42]  ← 'B'のASCIIコード
  0x7ffc00001001   [0x00]  ← '\0'（ヌル終端）
  0x7ffc00001002   [0x00]
  ...
  0x7ffc00002000   [0x00]  ┐
  0x7ffc00002001   [0x10]  │
  0x7ffc00002002   [0x00]  │ char *ptr の中身
  0x7ffc00002003   [0x00]  │ = 0x7ffc00001000
  0x7ffc00002004   [0xfc]  │ （8バイトのアドレス値）
  0x7ffc00002005   [0x7f]  │
  0x7ffc00002006   [0x00]  │
  0x7ffc00002007   [0x00]  ┘

  ptr が 0x7ffc00001000 を指している
  → *ptr は 0x42 = 'B'
```

### 1.2 ポインタの矢印図（get_next_lineに直結する理解）

get_next_lineで最も重要なポインタの操作は、
`char *stash`と`char *line`です。

```
=== ft_strjoin(stash, buf) の前後 ===

【Before】
  stash (BSS)            ヒープ
  ┌──────────┐          ┌─────────────────┐
  │0x55ab0100│ ────────→│ 'H' 'e' 'l' '\0'│
  └──────────┘          └─────────────────┘
                         アドレス: 0x55ab0100

  buf (スタック)          ヒープ
  ┌──────────┐          ┌──────────────────────┐
  │0x55ab0200│ ────────→│ 'l' 'o' '\n' 'W' '\0'│
  └──────────┘          └──────────────────────┘
                         アドレス: 0x55ab0200

【ft_strjoin実行中】
  joined = malloc(8):
                         ヒープ
                        ┌──────────────────────────────────────┐
                        │ 'H' 'e' 'l' 'l' 'o' '\n' 'W' '\0'  │
                        └──────────────────────────────────────┘
                         アドレス: 0x55ab0300

  free(s1):  0x55ab0100 の領域が解放される

【After】
  stash (BSS)            ヒープ
  ┌──────────┐          ┌──────────────────────────────────────┐
  │0x55ab0300│ ────────→│ 'H' 'e' 'l' 'l' 'o' '\n' 'W' '\0'  │
  └──────────┘          └──────────────────────────────────────┘
                         アドレス: 0x55ab0300

  ※ 0x55ab0100 は解放済み（もう使えない）
```

### 1.3 NULLポインタの重要性

NULLは「何も指していない」状態を表す特別な値（通常は0）です。

```c
char *ptr = NULL;

// NULLポインタをデリファレンスすると:
*ptr;      // セグメンテーションフォルト!
ptr[0];    // セグメンテーションフォルト!
strlen(ptr); // セグメンテーションフォルト! (標準のstrlen)

// get_next_lineでは全てNULLセーフに実装:
ft_strlen(NULL);  // 0を返す（安全）
ft_strchr(NULL, 'a'); // NULLを返す（安全）
```

**get_next_lineでNULLチェックが必要な全箇所**:

```
1. stash がNULLか → ft_init_stash で確認
2. malloc の戻り値がNULLか → 全てのmalloc後に確認
3. ft_init_stash の戻り値がNULLか → get_next_line で確認
4. ft_read_to_stash の戻り値がNULLか → get_next_line で確認
5. ft_strjoin の引数 s1, s2 がNULLか → ft_strjoin 冒頭で確認
6. ft_get_line の引数 stash がNULLか → ft_get_line 冒頭で確認
7. ft_trim_stash の引数 stash がNULLか → ft_trim_stash 冒頭で確認
```

### 1.4 ポインタの代入とメモリリークの関係

```c
char *s = malloc(10);  // s → [10バイトの領域A]

// 以下のどちらかをやるとリーク:
s = malloc(20);        // s → [20バイトの領域B]
                       // 領域A は誰も参照していない → リーク!

s = some_other_ptr;    // s → [他の領域]
                       // 領域A は誰も参照していない → リーク!
```

```
リークの図解:

BEFORE:
  s ──→ [AAAAAAAAAA]  (10バイト, malloc済み)

AFTER (s = malloc(20)):
  s ──→ [BBBBBBBBBBBBBBBBBBBB]  (20バイト, 新しいmalloc)

        [AAAAAAAAAA]  ← 誰も指していない! free不可能!
                         ↑ メモリリーク
```

### 1.5 やってみよう: ポインタの基礎

```c
// 以下のコードの各行での変数の値を予測しよう

char	*s1;
char	*s2;

s1 = malloc(6);         // s1 = ? (ヒープアドレス)
strcpy(s1, "Hello");    // s1 → "Hello\0"

s2 = s1;                // s2 = ?  (s1と同じアドレス)
                        // s1とs2は同じメモリを指す!

s2[0] = 'J';            // s1 → "Jello\0"
                        // s2 → "Jello\0" (同じメモリ!)

printf("%s\n", s1);     // "Jello" (s2経由で変更された!)

free(s1);               // メモリ解放
// s2 はダングリングポインタに!
// s2[0] はもう使えない!
```

---

## 2. C言語の文字列

### 2.1 文字列の内部表現

C言語の文字列は、**ヌル終端（'\0'）付きのchar配列**です。

```
文字列 "Hello\n" のメモリ表現:

  アドレス   バイト値  文字  インデックス
  0x1000     0x48     'H'     [0]
  0x1001     0x65     'e'     [1]
  0x1002     0x6C     'l'     [2]
  0x1003     0x6C     'l'     [3]
  0x1004     0x6F     'o'     [4]
  0x1005     0x0A     '\n'    [5]
  0x1006     0x00     '\0'    [6]  ← ヌル終端（文字列の終わり）

  ft_strlen("Hello\n") = 6  （'\0'は含まない）
  malloc に必要なサイズ = 7  （'\0'の1バイトを含む）
```

### 2.2 空文字列 "" の表現

```
空文字列 "" のメモリ表現:

  アドレス   バイト値  文字  インデックス
  0x2000     0x00     '\0'    [0]  ← これだけ!

  ft_strlen("") = 0
  malloc に必要なサイズ = 1  （'\0'の1バイト）
```

**get_next_lineでの重要性**: ft_init_stashで`malloc(1)`して
`stash[0] = '\0'`とするのは、空文字列を作るためです。

### 2.3 '\n' (改行) と '\0' (ヌル終端) の違い

```
'\n' = 0x0A = 改行文字 (Line Feed)
  → テキストの行区切りを示す
  → 文字列の途中に存在できる
  → ft_strlen で数えられる

'\0' = 0x00 = ヌル文字
  → 文字列の終端を示す
  → これ以降は文字列として認識されない
  → ft_strlen で数えない
```

```
"AB\nCD\n" のメモリ:

  [A][B][\n][C][D][\n][\0]
   0   1   2   3   4   5   6

  ft_strlen = 6
  \n の位置: インデックス 2 と 5
  \0 の位置: インデックス 6

get_next_line が返す1行目: "AB\n" (\n を含む!)
get_next_line が返す2行目: "CD\n" (\n を含む!)
```

### 2.4 文字列操作の基本パターン

get_next_lineで使う文字列操作パターン:

```c
// パターン1: 文字列の長さを求める
size_t	i = 0;
while (s[i])     // '\0'が見つかるまで
	i++;
// i = 文字列の長さ

// パターン2: 特定の文字を探す
size_t	i = 0;
while (s[i] && s[i] != '\n')  // '\0'か'\n'が見つかるまで
	i++;
// s[i] == '\n' → 改行が見つかった
// s[i] == '\0' → 改行なし（文字列末尾まで到達）

// パターン3: 文字列のコピー
size_t	i = 0;
while (s[i])
{
	dst[i] = s[i];
	i++;
}
dst[i] = '\0';  // ヌル終端を忘れずに!
```

---

## 3. スタックメモリとヒープメモリの詳細

### 3.1 スタックメモリ

```c
void	func(int n)
{
	char	buf[100];    // スタック上に100バイト
	int		x;           // スタック上に4バイト
	char	*ptr;        // スタック上に8バイト(ポインタ)
}
```

```
関数呼び出し時のスタック:

高アドレス
┌──────────────────────────┐
│  呼び出し元の関数の        │
│  ローカル変数               │
├──────────────────────────┤
│  戻りアドレス (8バイト)     │ ← func()から戻る先
├──────────────────────────┤
│  引数 n (4バイト)          │
├──────────────────────────┤
│  buf[0..99] (100バイト)   │ ← char buf[100]
├──────────────────────────┤
│  x (4バイト)              │ ← int x
├──────────────────────────┤
│  ptr (8バイト)            │ ← char *ptr
├──────────────────────────┤ ← スタックポインタ (SP)
│                          │
│  (空き領域)               │
│                          │
低アドレス

func() が終了すると:
  → SP が元に戻り、buf, x, ptr は全て「消滅」
  → メモリ自体は残っているが、他の関数が上書きする
```

スタックの特徴:
- 確保・解放が超高速（SPの移動だけ）
- サイズはコンパイル時に決定（VLAを除く）
- サイズ制限あり（通常8MB程度）
- 関数を抜けると自動的に無効になる

### 3.2 ヒープメモリ

```c
void	func(void)
{
	char	*buf;

	buf = malloc(100);  // ヒープ上に100バイト確保
	// buf はスタック上（8バイトのポインタ）
	// buf が指す先はヒープ上（100バイトのデータ）
	free(buf);          // 明示的に解放が必要
}
```

```
スタックとヒープの関係:

高アドレス
┌──────────────────────┐
│  スタック              │
│  ┌──────────────┐     │
│  │ buf [0x55ab..]│────┼──┐
│  └──────────────┘     │  │ ポインタが
│                      │  │ ヒープを指す
│  (空き領域)           │  │
│                      │  │
│  ┌──────────────┐     │  │
│  │ [100バイト]    │←───┘  │
│  │ ヒープ領域     │        │
│  └──────────────┘        │
│  ヒープ                    │
低アドレス

funcが終了すると:
  → スタック上のbuf(ポインタ変数)は消滅
  → ヒープ上の100バイトは残ったまま!
  → free(buf)していないと → メモリリーク!
```

ヒープの特徴:
- 確保・解放はスタックより遅い（管理コストがある）
- サイズは実行時に動的に決定できる
- 明示的にfree()しないと解放されない
- 関数間でデータを受け渡せる

### 3.3 get_next_lineでの使い分け

```
┌──────────────────────────────────────────────────────────────┐
│ 変数             格納場所        理由                          │
├──────────────────────────────────────────────────────────────┤
│ static char *stash  BSS (ポインタ自体)   関数間で値を保持する必要 │
│                     ヒープ (データ)     行の長さが不定           │
│                                                              │
│ char *buf          スタック (ポインタ)   関数内で完結            │
│                     ヒープ (データ)     BUFFER_SIZEが大きい可能性│
│                                                              │
│ char *line         スタック (ポインタ)   一時的な参照            │
│                     ヒープ (データ)     呼び出し元に返す必要     │
│                                                              │
│ int bytes_read     スタック             小さな固定サイズ        │
│ size_t i, j        スタック             小さな固定サイズ        │
└──────────────────────────────────────────────────────────────┘
```

### 3.4 なぜ buf をスタック配列ではなくmallocで確保するのか

```c
// 方法A: スタック配列（使わない理由がある）
char buf[BUFFER_SIZE + 1];

// 方法B: ヒープ確保（こちらを使う）
char *buf = malloc(sizeof(char) * (BUFFER_SIZE + 1));
```

方法Aを使わない理由:

1. **BUFFER_SIZEが巨大な場合**: `-D BUFFER_SIZE=10000000`のとき、
   スタック上に10MBの配列を確保するとスタックオーバーフローの危険
   （通常のスタックサイズ上限は8MB程度）

2. **VLA（Variable Length Array）の問題**: BUFFER_SIZEはマクロなので
   定数扱いだが、概念的にVLAに近い使い方になり、移植性の懸念がある

---

## 4. システムコールとライブラリ関数

### 4.1 2つの世界

```
┌──────────────────────────────────────────────────────────────┐
│ ユーザー空間 (User Space)                                      │
│                                                              │
│  あなたのプログラム                                             │
│  ┌──────────────────────────────────────────┐                │
│  │ printf("Hello\n");                        │                │
│  │   → 内部で write() を呼ぶ                  │                │
│  │   → ライブラリがバッファリングする           │                │
│  │                                           │                │
│  │ read(fd, buf, n);                          │                │
│  │   → 直接システムコールを発行                │                │
│  └────────────────────────────┬──────────────┘                │
│                              │                                │
│ ───── システムコール境界 ───── │ ──────────────────────────────│
│                              │                                │
│ カーネル空間 (Kernel Space)    v                                │
│  ┌──────────────────────────────────────────┐                │
│  │ read() の処理:                             │                │
│  │  1. fd の有効性を検証                       │                │
│  │  2. ファイルシステムにアクセス                │                │
│  │  3. データをユーザー空間にコピー              │                │
│  │  4. ファイルオフセットを更新                  │                │
│  └──────────────────────────────────────────┘                │
└──────────────────────────────────────────────────────────────┘
```

### 4.2 get_next_lineで使える関数

| 関数 | 種類 | ヘッダ | 用途 |
|------|------|--------|------|
| `read()` | システムコール | `<unistd.h>` | fdからデータ読み取り |
| `malloc()` | ライブラリ関数 | `<stdlib.h>` | ヒープメモリ確保 |
| `free()` | ライブラリ関数 | `<stdlib.h>` | ヒープメモリ解放 |

**使用禁止**:
- `lseek()` - ファイルオフセットの手動操作
- libftの関数 - 自前で実装する必要
- グローバル変数 - Normで禁止
- `fgets()`, `getline()` 等のstdioバッファリング関数

### 4.3 なぜread()を直接使うのか（fgets()との比較）

```
fgets() を使う場合（禁止されている）:
  → stdio がバッファリングを管理
  → FILE* 構造体の中に内部バッファがある
  → プログラマはバッファリングを意識しなくてよい

read() を直接使う場合（get_next_lineの課題）:
  → バッファリングを自分で実装する必要がある
  → BUFFER_SIZE 分のデータを読み、stash に蓄積
  → \n を見つけたら行を切り出す
  → 残りを次回に持ち越す

  → これがまさにget_next_lineの課題の核心!
  → 「fgets()が内部でやっていることを自分で実装する」
```

---

## 5. valgrindによるメモリデバッグ

### 5.1 valgrindとは

valgrindはメモリ関連のバグを検出するツールです。
以下の問題を検出できます。

| 検出できるバグ | 説明 |
|-------------|------|
| メモリリーク | malloc後にfreeされていないメモリ |
| 未初期化メモリの使用 | mallocした直後の不定な値の参照 |
| 範囲外アクセス | malloc(10)した領域の11バイト目にアクセス |
| ダブルフリー | 同じポインタを2回free |
| use-after-free | free後のメモリアクセス |
| 不正なfree | mallocしていないアドレスのfree |

### 5.2 基本的な使い方

```bash
# 1. デバッグ情報付きでコンパイル（-gオプション必須!）
cc -Wall -Wextra -Werror -g -D BUFFER_SIZE=42 \
    get_next_line.c get_next_line_utils.c main.c -o gnl

# 2. valgrindで実行
valgrind --leak-check=full ./gnl

# 3. より詳細な情報
valgrind --leak-check=full --show-leak-kinds=all \
    --track-origins=yes ./gnl

# 4. 最大限の情報
valgrind --leak-check=full --show-leak-kinds=all \
    --track-origins=yes --verbose ./gnl
```

### 5.3 出力の読み方 - リークなしの場合

```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks    ← 全て解放済み!
==12345==   total heap usage: 19 allocs, 19 frees, 1,234 bytes allocated
                              ^^^^^^^^  ^^^^^^^^^
                              確保回数 = 解放回数 → OK!

==12345== All heap blocks were freed -- no leaks are possible
                                        ^^^^^^^^^^^^^^^^^^^^
                                        リークなし!

==12345== ERROR SUMMARY: 0 errors from 0 contexts
                         ^^^^^^^^
                         エラーなし!
```

### 5.4 出力の読み方 - リークがある場合

```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 42 bytes in 1 blocks   ← 42バイト未解放!
==12345==   total heap usage: 19 allocs, 18 frees, 1,234 bytes allocated
                              ^^^^^^^^  ^^^^^^^^^
                              19回確保、18回解放 → 1回分のfreeが足りない!

==12345== 42 bytes in 1 blocks are definitely lost in loss record 1 of 1
                                   ^^^^^^^^^^^^^^^^
                                   確実にリーク!
==12345==    at 0x4C2FB55: malloc (in /usr/lib/...)
==12345==    by 0x401234: ft_strjoin (get_next_line_utils.c:50)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                          ft_strjoin の50行目でmallocした領域がリーク
==12345==    by 0x401567: ft_read_to_stash (get_next_line.c:30)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                          ft_read_to_stashから呼ばれた
==12345==    by 0x401789: get_next_line (get_next_line.c:60)
```

### 5.5 リークの種類

```
definitely lost:  確実にリーク。ポインタが完全に失われた。
                  → 必ず修正すべき!

indirectly lost:  directly lost なポインタ経由でしか到達できない。
                  → directly lost を修正すれば解消される

possibly lost:    ポインタが配列の途中を指しているなど。
                  → 要確認

still reachable:  プログラム終了時にまだポインタが残っている。
                  → static変数の場合は許容されることが多い
                  → ただし、最後のget_next_line(EOF)後に
                    stashが解放されていれば発生しない
```

### 5.6 valgrindのオプション一覧

| オプション | 説明 |
|-----------|------|
| `--leak-check=full` | 全てのリーク詳細を表示 |
| `--show-leak-kinds=all` | 全種類のリークを表示 |
| `--track-origins=yes` | 未初期化値の出所を追跡 |
| `--verbose` | 詳細な情報を表示 |
| `--log-file=output.txt` | 結果をファイルに出力 |
| `--num-callers=20` | コールスタックの深さ |

### 5.7 やってみよう: valgrindの実践

```c
// 以下のプログラムをvalgrindで実行し、
// 出力から問題を特定してみよう

#include <stdlib.h>

int	main(void)
{
	char	*s1;
	char	*s2;

	s1 = malloc(10);
	s2 = malloc(20);
	s1 = s2;
	free(s1);
	return (0);
}

// 問題:
// 1. s1の元の10バイトがリーク (definitely lost)
// 2. s2(= 新しいs1)は正しくfreeされている
// → "10 bytes in 1 blocks are definitely lost" が表示される
```

---

## 6. Makefileと-Dプリプロセッサフラグ

### 6.1 -D フラグの仕組み

```bash
cc -D BUFFER_SIZE=42 get_next_line.c
```

これは、ソースファイルの先頭に以下を追加するのと等価です:

```c
#define BUFFER_SIZE 42
// ... 以下、元のソースコード ...
```

### 6.2 ヘッダでのデフォルト値設定

```c
// get_next_line.h
#ifndef BUFFER_SIZE
# define BUFFER_SIZE 42
#endif
```

```
処理の流れ:

ケース1: cc -D BUFFER_SIZE=100 ...
  コンパイラ: "BUFFER_SIZE は 100 として定義済み"
  #ifndef BUFFER_SIZE → 偽(定義済み) → スキップ
  結果: BUFFER_SIZE = 100

ケース2: cc ...  (BUFFER_SIZE未指定)
  コンパイラ: "BUFFER_SIZE は未定義"
  #ifndef BUFFER_SIZE → 真(未定義) → 実行
  #define BUFFER_SIZE 42
  結果: BUFFER_SIZE = 42
```

### 6.3 テストすべきBUFFER_SIZE値

```bash
# 最小値テスト: 1バイトずつ読む極端なケース
cc -Wall -Wextra -Werror -D BUFFER_SIZE=1 \
    get_next_line.c get_next_line_utils.c main.c

# 標準テスト: 一般的なサイズ
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 \
    get_next_line.c get_next_line_utils.c main.c

# 大きめテスト: 実用的な大きいサイズ
cc -Wall -Wextra -Werror -D BUFFER_SIZE=9999 \
    get_next_line.c get_next_line_utils.c main.c

# 極大テスト: 10MB
cc -Wall -Wextra -Werror -D BUFFER_SIZE=10000000 \
    get_next_line.c get_next_line_utils.c main.c

# デフォルト値テスト: -Dなし
cc -Wall -Wextra -Werror \
    get_next_line.c get_next_line_utils.c main.c

# エッジケース: 0（NULLを返すべき）
cc -Wall -Wextra -Werror -D BUFFER_SIZE=0 \
    get_next_line.c get_next_line_utils.c main.c
```

---

## 7. ssize_t と size_t の違い

### 7.1 型の定義

```
size_t:
  → 符号なし整数型 (unsigned)
  → 0 以上の値のみ
  → sizeof の戻り値、malloc の引数
  → 文字列の長さ、配列のインデックスに使用
  → 64ビットシステムでは通常 unsigned long (8バイト)

ssize_t:
  → 符号あり整数型 (signed)
  → 負の値も表現可能
  → read() の戻り値（-1 を返す可能性があるため）
  → 64ビットシステムでは通常 long (8バイト)
```

### 7.2 get_next_lineでの使い分け

```c
// read()の戻り値: ssize_t（-1の可能性があるため）
// ただし、Normの制約でintを使うこともある
int bytes_read = read(fd, buf, BUFFER_SIZE);

// 文字列のインデックス: size_t（負になることはない）
size_t i = 0;
while (s[i])
    i++;

// mallocのサイズ指定: size_t
char *buf = malloc(sizeof(char) * (BUFFER_SIZE + 1));
```

**注意: read()の戻り値をintで受ける場合**

実装コードでは`int bytes_read`を使っていますが、
read()の正式な戻り値型は`ssize_t`です。
BUFFER_SIZEがINT_MAXを超える場合、intでは
正しく受け取れない可能性があります。
ただし、42の評価ではintで問題ないケースがほとんどです。

---

## 8. コンパイルフラグの意味

### 8.1 必須フラグの解説

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 \
    get_next_line.c get_next_line_utils.c main.c
```

| フラグ | 意味 | 具体例 |
|-------|------|--------|
| `-Wall` | 一般的な警告を有効化 | 未使用変数、暗黙の型変換 |
| `-Wextra` | 追加の警告を有効化 | 未使用パラメータ、符号の比較 |
| `-Werror` | 全ての警告をエラーとして扱う | 警告があるとコンパイル失敗 |
| `-D BUFFER_SIZE=42` | マクロ定義 | BUFFER_SIZEを42に設定 |
| `-g` | デバッグ情報を含める | valgrind/gdbで行番号表示 |

### 8.2 -Wallと-Wextraが検出する問題の例

```c
// -Wall で検出される例:
int x;
printf("%d\n", x);  // 警告: 'x' is used uninitialized

char *s = "hello";
if (s = NULL)        // 警告: suggest parentheses (代入と比較の混同)

// -Wextra で検出される例:
void func(int unused_param)
{
    // 警告: unused parameter 'unused_param'
}

unsigned int a = 5;
int b = -1;
if (a > b)           // 警告: comparison between signed and unsigned
```

---

## 9. まとめ: 前提知識チェックリスト

get_next_lineの実装を始める前に、以下を確認しましょう。

### 必須理解

- [ ] `char *ptr = malloc(n);`の後にNULLチェックが必要な理由
- [ ] `free(ptr); ptr = NULL;`のパターンとその目的
- [ ] 文字列のヌル終端（'\0'）の役割
- [ ] '\n'（改行）と'\0'（ヌル終端）の違い
- [ ] スタック変数とヒープ変数の違い（5つ以上挙げられるか）
- [ ] メモリリークが発生する条件と検出方法
- [ ] システムコール（read）とライブラリ関数（printf）の違い
- [ ] `-D BUFFER_SIZE=42`の意味と`#ifndef`パターン

### 推奨理解

- [ ] valgrindの基本的な使い方と出力の読み方
- [ ] ダングリングポインタとダブルフリーの違い
- [ ] size_tとssize_tの違い
- [ ] `-Wall -Wextra -Werror`が何を検出するか

この前提知識が身についていれば、
get_next_lineの実装にスムーズに取り組めるはずです。

---

## 10. メモリの可視化: プロセスのメモリマップ

### 10.1 プロセスの完全なメモリレイアウト

```
高アドレス (0xFFFFFFFFFFFF...)
┌──────────────────────────────────────────────┐
│                                              │
│  カーネル空間（ユーザーからアクセス不可）        │
│                                              │
├──────────────────────────────────────────────┤ <- 0x7FFF...
│                                              │
│  スタック (Stack)                              │
│  → ローカル変数、関数引数、戻りアドレス          │
│  → 下方向に成長する                            │
│  → 例: int bytes_read; char *buf;             │
│                                              │
│          |  (空き領域)  |                       │
│          v              v                     │
│                                              │
│  ヒープ (Heap)                                │
│  → malloc() で確保される                       │
│  → 上方向に成長する                            │
│  → 例: buf = malloc(BUFFER_SIZE + 1);         │
│                                              │
├──────────────────────────────────────────────┤
│                                              │
│  BSS セグメント                                │
│  → 初期化されていないstatic/グローバル変数       │
│  → 0に初期化される                             │
│  → 例: static char *stash; → NULL             │
│                                              │
├──────────────────────────────────────────────┤
│                                              │
│  データセグメント (Data)                        │
│  → 初期化済みのstatic/グローバル変数            │
│  → 例: static int count = 42;                │
│                                              │
├──────────────────────────────────────────────┤
│                                              │
│  テキストセグメント (Text/Code)                │
│  → プログラムの機械語コード                     │
│  → 読み取り専用                                │
│  → 例: get_next_line() の命令列               │
│                                              │
└──────────────────────────────────────────────┘
低アドレス (0x000000...)
```

### 10.2 get_next_lineの各変数の配置場所

```
┌─────────────────────────────────────────────────────┐
│ 変数               │ セグメント  │ サイズ  │ 寿命      │
├─────────────────────┼──────────┼────────┼──────────┤
│ static char *stash  │ BSS      │ 8B     │ プログラム │
│  (ポインタ自体)      │          │        │ 全体     │
│                     │          │        │          │
│ stash が指すデータ   │ ヒープ    │ 可変   │ free まで │
│                     │          │        │          │
│ char *buf           │ スタック  │ 8B     │ 関数内   │
│  (ポインタ自体)      │          │        │          │
│                     │          │        │          │
│ buf が指すデータ     │ ヒープ    │ BS+1   │ free まで │
│                     │          │        │          │
│ char *line          │ スタック  │ 8B     │ 関数内   │
│  (ポインタ自体)      │          │        │          │
│                     │          │        │          │
│ line が指すデータ    │ ヒープ    │ 可変   │ 呼出元が │
│                     │          │        │ free     │
│                     │          │        │          │
│ int bytes_read      │ スタック  │ 4B     │ 関数内   │
│ size_t i, j         │ スタック  │ 各8B   │ 関数内   │
│ int fd              │ スタック  │ 4B     │ 関数内   │
└─────────────────────┴──────────┴────────┴──────────┘
```

### 10.3 ポインタの二重構造の理解

get_next_lineで最も理解が難しいのは、
「ポインタ変数自体」と「ポインタが指すデータ」が
異なる場所にあるという二重構造です。

```
static char *stash の場合:

  BSS セグメント                  ヒープ
  ┌───────────────┐              ┌───────────────────────┐
  │ stash         │              │ 'H' 'e' 'l' 'l' 'o'  │
  │ [0x55ab0100]  │─────────────→│ '\n' 'W' 'o' 'r' 'l'  │
  │               │              │ 'd' '\0'              │
  │ 8バイト        │              │ 12バイト (malloc済み)   │
  └───────────────┘              └───────────────────────┘
  プログラム開始時                  ft_strjoin で確保
  から存在                        free するまで存在

  stash 自体は BSS にあり、常に同じアドレスにある
  stash が指す先は ヒープ にあり、ft_strjoin のたびに変わる
```

---

## 11. 実践: 前提知識の確認テスト

### 11.1 ポインタテスト

以下のコードの出力を予測してください（コンパイルして確認）:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int	main(void)
{
	char	*s1;
	char	*s2;

	s1 = malloc(6);
	strcpy(s1, "Hello");
	s2 = s1;
	printf("s1 = %s\n", s1);   // Q1: 何が出力される?
	printf("s2 = %s\n", s2);   // Q2: 何が出力される?
	printf("s1 == s2: %d\n", s1 == s2);  // Q3: 1 or 0?

	s2 = malloc(6);
	strcpy(s2, "World");
	printf("s1 = %s\n", s1);   // Q4: 何が出力される?
	printf("s2 = %s\n", s2);   // Q5: 何が出力される?
	printf("s1 == s2: %d\n", s1 == s2);  // Q6: 1 or 0?

	free(s1);
	free(s2);
	return (0);
}
```

**答え**:
```
Q1: Hello  (s1はmallocした領域を指す)
Q2: Hello  (s2もs1と同じ領域を指す)
Q3: 1      (同じアドレス)
Q4: Hello  (s1は最初のmallocを指したまま)
Q5: World  (s2は新しいmallocを指す)
Q6: 0      (異なるアドレス)
```

### 11.2 メモリリークテスト

以下のコードにはメモリリークがあります。何バイトのリークがありますか?

```c
#include <stdlib.h>
#include <string.h>

int	main(void)
{
	char	*s1;
	char	*s2;
	char	*s3;

	s1 = malloc(10);    // 10バイト確保
	strcpy(s1, "AAAA");
	s2 = malloc(20);    // 20バイト確保
	strcpy(s2, "BBBB");
	s3 = malloc(30);    // 30バイト確保
	strcpy(s3, "CCCC");

	s1 = s2;            // Q: ここでリーク? 何バイト?
	free(s1);           // s2と同じアドレスをfree
	free(s3);           // s3をfree

	return (0);
}
```

**答え**:
- `s1 = s2;` の時点で、元のs1が指していた10バイトのリーク
- `free(s1)` は実質 `free(s2)` と同じ（同じアドレス）
- s2は`free(s1)`で解放済みだが、元のs2のfreeはない
  （既にfree(s1)で解放されている）
- s3は正しくfreeされている
- 結論: 10バイトのメモリリーク

### 11.3 static変数テスト

以下の関数を3回呼んだときの出力を予測してください:

```c
void	counter(void)
{
	static int	count;
	int			local;

	local = 0;
	count++;
	local++;
	printf("count=%d, local=%d\n", count, local);
}

int	main(void)
{
	counter();    // Q1: 出力は?
	counter();    // Q2: 出力は?
	counter();    // Q3: 出力は?
	return (0);
}
```

**答え**:
```
Q1: count=1, local=1
Q2: count=2, local=1  (countは保持される、localは毎回0に初期化)
Q3: count=3, local=1
```

---

## 12. read()の実験プログラム

get_next_lineの実装に入る前に、read()の動作を体感しましょう。

### 12.1 基本的なread()の実験

```c
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int	main(void)
{
	int		fd;
	char	buf[10];
	int		bytes;
	int		call_num;

	fd = open("test.txt", O_RDONLY);
	if (fd == -1)
	{
		printf("Error: cannot open file\n");
		return (1);
	}
	call_num = 1;
	while (1)
	{
		bytes = read(fd, buf, 5);
		if (bytes <= 0)
			break ;
		buf[bytes] = '\0';
		printf("Call %d: read returned %d, buf=[%s]\n",
			call_num, bytes, buf);
		call_num++;
	}
	printf("Final: read returned %d (%s)\n",
		bytes, bytes == 0 ? "EOF" : "Error");
	close(fd);
	return (0);
}
```

この実験で確認できること:
- read()は要求したバイト数より少なく返すことがある
- ファイル末尾に達すると0を返す
- read()はヌル終端を付けない（自分で付ける必要がある）
- 連続呼び出しでファイルオフセットが進む

### 12.2 無効なfdの実験

```c
#include <stdio.h>
#include <unistd.h>

int	main(void)
{
	char	buf[10];
	int		bytes;

	// 無効なfd
	bytes = read(-1, buf, 5);
	printf("read(-1): returned %d\n", bytes);  // -1

	bytes = read(999, buf, 5);
	printf("read(999): returned %d\n", bytes);  // -1

	// 閉じたfd
	int fd = open("test.txt", 0);
	close(fd);
	bytes = read(fd, buf, 5);
	printf("read(closed fd): returned %d\n", bytes);  // -1

	return (0);
}
```

---

## 13. mallocとfreeの実験

### 13.1 基本的なmalloc/freeの実験

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int	main(void)
{
	char	*ptr;

	// 基本的なmalloc
	ptr = malloc(10);
	if (!ptr)
	{
		printf("malloc failed!\n");
		return (1);
	}
	printf("ptr = %p\n", (void *)ptr);
	strcpy(ptr, "Hello");
	printf("*ptr = %s\n", ptr);

	// free後の状態
	free(ptr);
	// ptr はまだアドレスを持っているが、
	// そのアドレスは無効（ダングリングポインタ）
	printf("after free: ptr = %p\n", (void *)ptr);
	// ptr[0] にアクセスするのは未定義動作!

	// NULLに設定
	ptr = NULL;
	printf("after NULL: ptr = %p\n", (void *)ptr);

	return (0);
}
```

### 13.2 所有権移転パターンの実験

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char	*my_strjoin(char *s1, char *s2)
{
	char	*result;
	size_t	len1;
	size_t	len2;

	len1 = strlen(s1);
	len2 = strlen(s2);
	result = malloc(len1 + len2 + 1);
	if (!result)
		return (NULL);
	memcpy(result, s1, len1);
	memcpy(result + len1, s2, len2 + 1);
	free(s1);  // s1の所有権を消滅させる
	return (result);  // resultの所有権を呼び出し元に移転
}

int	main(void)
{
	char	*str;

	str = malloc(1);
	str[0] = '\0';
	printf("Initial: [%s]\n", str);

	str = my_strjoin(str, "Hello");
	printf("After join 1: [%s]\n", str);

	str = my_strjoin(str, " World");
	printf("After join 2: [%s]\n", str);

	str = my_strjoin(str, "!");
	printf("After join 3: [%s]\n", str);

	free(str);
	return (0);
}
```

この実験で確認できること:
- my_strjoinが古いs1をfreeし、新しい結合文字列を返す
- strには常に最新の結合結果のアドレスが入る
- 古いアドレスは自動的に解放される（リークなし）
- 最後のfree(str)で全てのメモリが解放される

---

## 14. やってみよう: 前提知識の総合演習

### 14.1 演習: get_next_lineの簡易版を作る

以下の仕様で、get_next_lineの超簡易版を作ってみましょう:
- ファイルから1文字ずつ読み取り、\nまたはEOFまでの1行を返す
- BUFFER_SIZE やstashの概念は使わない
- まずread()とmalloc()の基本的な使い方を練習する目的

```c
// ヒント:
// 1. 1文字ずつread()で読む
// 2. 毎回reallocまたはmy_strjoin()で文字を追加
// 3. \nが見つかったら行を返す
// 4. read()が0を返したらNULLを返す
```

この演習により、get_next_lineの本実装に入る前に
read()、malloc()、free()、文字列操作の基礎が身につきます。
