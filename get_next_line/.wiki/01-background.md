# 01 - 背景知識

この章では、get_next_lineを理解するために必要な全ての背景知識を解説します。
単なる関数の使い方ではなく、「なぜそう設計されているのか」という歴史的・
技術的な理由まで深く掘り下げます。

---

## 1. Unix哲学と"Everything is a file"

### 1.1 Unixが生まれた背景

1969年、AT&Tベル研究所でKen ThompsonとDennis Ritchieによって
Unixオペレーティングシステムが誕生しました。当時のコンピュータは
高価で複雑であり、異なるハードウェアごとにプログラムを書き直す
必要がありました。

Unixの設計者たちは、一つの革命的なアイデアを導入しました。
「全てをファイルとして扱う（Everything is a file）」という哲学です。

### 1.2 "Everything is a file"とは

この哲学の核心は、異なる種類のI/Oリソースに対して
**同じインターフェース（read/write）でアクセスできる**ということです。

```
┌──────────────────────────────────────────────────────────┐
│              "Everything is a file" の世界                 │
│                                                          │
│  通常ファイル ─────→ open() → fd → read(fd, ...) → データ │
│  ディレクトリ ─────→ open() → fd → read(fd, ...) → データ │
│  デバイス ──────────→ open() → fd → read(fd, ...) → データ │
│  パイプ ───────────→ pipe() → fd → read(fd, ...) → データ │
│  ソケット ──────────→ socket()→ fd → read(fd, ...) → データ │
│  ターミナル(stdin) → (fd=0) ──→ read(fd, ...) → データ    │
│                                                          │
│  全て同じ read() 関数で読める!                              │
└──────────────────────────────────────────────────────────┘
```

### 1.3 なぜこれがget_next_lineに関係するのか

get_next_lineは`int fd`を引数に取ります。これはまさにUnixの
"Everything is a file"哲学を体現しています。

- 通常のテキストファイルから読める
- stdin（キーボード入力）から読める
- パイプから読める（`cat file | ./program`）
- ネットワークソケットから読める（発展的な使い方）

fdという統一的なインターフェースのおかげで、get_next_lineは
**データの出所を気にせず**、行単位の読み取りを実現できるのです。

### 1.4 やってみよう: "Everything is a file" の実験

```bash
# /proc はLinuxの仮想ファイルシステム
# プロセス情報が「ファイル」として公開されている

# 自分のプロセスID
cat /proc/self/status | head -5

# CPUの情報（ファイルとしてアクセスできる）
cat /proc/cpuinfo | head -10

# これらは実際にはディスク上のファイルではなく、
# カーネルが「ファイルのふり」をして情報を提供している
# → read() システムコールで読める = "file" として扱える
```

---

## 2. ファイルディスクリプタ（File Descriptor / fd）

### 2.1 ファイルディスクリプタとは

ファイルディスクリプタ（file descriptor、以下fd）は、
Unix/Linuxシステムにおいて開かれたファイルやI/Oリソースを
識別するための**非負整数**です。

ここで重要な問いがあります。
**「なぜポインタやオブジェクトではなく、ただの整数なのか?」**

### 2.2 なぜfdは「整数」なのか - 歴史的理由

1970年代のUnix設計時、コンピュータのリソースは極めて限られていました。
fdを整数にした理由は以下の通りです。

1. **メモリ効率**: 整数はわずか4バイト。ポインタや構造体より格段に小さい
2. **安全性**: ユーザープログラムがカーネル内部のデータ構造に
   直接アクセスできないようにする（間接参照）
3. **シンプルさ**: 配列のインデックスとして使える
4. **移植性**: どのアーキテクチャでも整数は同じように動く

```
なぜ整数が「安全」なのか:

ユーザー空間                    カーネル空間
┌──────────────┐              ┌──────────────────────┐
│              │              │                      │
│  fd = 3     ─┼──────────→  │  fdテーブル[3]        │
│  (ただの数字) │    間接参照   │    ├─ ファイルオフセット │
│              │              │    ├─ アクセス権限      │
│  プログラムは │              │    ├─ inode情報        │
│  カーネル内部を│              │    └─ デバイス情報      │
│  直接触れない  │              │                      │
└──────────────┘              └──────────────────────┘

もしポインタを渡していたら、ユーザープログラムが
カーネルのメモリを直接操作できてしまう（危険！）
```

### 2.3 標準ファイルディスクリプタ

プロセスが起動すると、3つのfdが自動的に開かれます。

| fd | マクロ名 | 説明 | 接続先（通常） |
|----|---------|------|-------------|
| 0 | `STDIN_FILENO` | 標準入力 | キーボード |
| 1 | `STDOUT_FILENO` | 標準出力 | 画面（端末） |
| 2 | `STDERR_FILENO` | 標準エラー出力 | 画面（端末） |

```
プロセス起動時の状態:

プロセスのfdテーブル          接続先
┌─────┬─────────────┐
│  0  │ STDIN  ──────┼──→ キーボード (入力)
│  1  │ STDOUT ──────┼──→ 端末画面   (出力)
│  2  │ STDERR ──────┼──→ 端末画面   (エラー出力)
│  3  │ (未使用)      │
│  4  │ (未使用)      │
│ ... │  ...         │
└─────┴─────────────┘
```

**重要**: get_next_lineはfd=0（stdin）でも動作する必要があります。
stdinは端末からの入力であり、通常のファイルとは異なる振る舞いを
しますが、read()インターフェースは同じです。

### 2.4 fdテーブルの仕組み - カーネル内部の3層構造

実際のLinuxカーネルでは、fdからファイルに至るまで
3層の間接参照が行われています。

```
┌──────────────────────────────────────────────────────────────┐
│ 第1層: プロセスごとのfdテーブル                                 │
│ (各プロセスが独自に持つ)                                        │
│                                                              │
│  プロセスA           プロセスB                                  │
│  ┌────┬───┐          ┌────┬───┐                               │
│  │ 0  │ ──┼─┐        │ 0  │ ──┼──→ ...                       │
│  │ 1  │ ──┼─┼─┐      │ 1  │ ──┼──→ ...                       │
│  │ 2  │ ──┼─┼─┼─┐    │ 2  │ ──┼──→ ...                       │
│  │ 3  │ ──┼─┼─┼─┼─┐  │ 3  │ ──┼──→ ...                       │
│  └────┴───┘ │ │ │ │  └────┴───┘                               │
│             │ │ │ │                                           │
├─────────────┼─┼─┼─┼──────────────────────────────────────────┤
│ 第2層: オープンファイルテーブル（Open File Table）               │
│ (システム全体で共有)                                            │
│             │ │ │ │                                           │
│             v v v v                                           │
│  ┌────────────────┐  ┌────────────────┐                      │
│  │ ファイルオフセット │  │ ファイルオフセット │                      │
│  │ アクセスモード    │  │ アクセスモード    │                      │
│  │ 参照カウント      │  │ 参照カウント      │                      │
│  └───────┬────────┘  └───────┬────────┘                      │
│          │                   │                                │
├──────────┼───────────────────┼────────────────────────────────┤
│ 第3層: inode テーブル                                          │
│          │                   │                                │
│          v                   v                                │
│  ┌──────────────┐    ┌──────────────┐                        │
│  │ ファイルサイズ │    │ ファイルサイズ │                        │
│  │ パーミッション │    │ パーミッション │                        │
│  │ ディスク上位置 │    │ ディスク上位置 │                        │
│  └──────────────┘    └──────────────┘                        │
└──────────────────────────────────────────────────────────────┘
```

**get_next_lineに関係するポイント**:
- **ファイルオフセット**は第2層で管理されている
- read()を呼ぶたびにカーネルがオフセットを自動的に進める
- プログラマがオフセットを手動で管理する必要はない
- **lseek()が禁止**されているのは、オフセットの手動操作が不要だから

### 2.5 open()とfdの割り当て規則

```c
#include <fcntl.h>

int fd = open("test.txt", O_RDONLY);
// fdは「利用可能な最小の非負整数」が割り当てられる
// 通常は3（0,1,2は標準で使用済み）
```

**最小fd番号割り当ての規則**:

```
初期状態:
  fd 0: stdin (使用中)
  fd 1: stdout (使用中)
  fd 2: stderr (使用中)
  fd 3: (空き) ← open()はここを使う

open("file_a.txt", O_RDONLY);  → fd = 3
open("file_b.txt", O_RDONLY);  → fd = 4

close(3);  // fd 3 を解放

open("file_c.txt", O_RDONLY);  → fd = 3 (再利用!)

close(0);  // stdin を閉じた場合(!)

open("file_d.txt", O_RDONLY);  → fd = 0 (最小の空きfd)
// stdinがファイルに置き換わってしまう!
```

この規則は、後のプロジェクト（pipex、minishell）で
**リダイレクト**を実装する際に極めて重要になります。

### 2.6 fdの上限

各プロセスが同時に開けるfdの数には上限があります。

```bash
# 現在のfd上限を確認
ulimit -n
# 典型的な出力: 1024

# システム全体の上限
cat /proc/sys/fs/file-max
# 典型的な出力: 9223372036854775807 (実質無制限)
```

ボーナス課題の`MAX_FD`（デフォルト1024）は、この制限に基づいています。

### 2.7 やってみよう: fdの実験

```c
// 以下のプログラムを書いて、fdの割り当てを観察しよう
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int	main(void)
{
	int	fd1;
	int	fd2;
	int	fd3;

	fd1 = open("test.txt", O_RDONLY);
	fd2 = open("test.txt", O_RDONLY);
	printf("fd1 = %d\n", fd1);  // 3
	printf("fd2 = %d\n", fd2);  // 4
	close(fd1);
	fd3 = open("test.txt", O_RDONLY);
	printf("fd3 = %d\n", fd3);  // 3 (再利用!)
	close(fd2);
	close(fd3);
	return (0);
}
```

**問い**: 上のプログラムで`close(fd1)`の前に`close(0)`を追加したら、
fd3はいくつになるか? 考えてから実行してみよう。

---

## 3. read() システムコール

### 3.1 read()とは何か

`read()`はUnixの基本的なシステムコールで、fdからデータを読み取ります。
「システムコール」とは、ユーザープログラムがカーネル（OSの中核）の
機能を利用するための特別な関数呼び出しです。

### 3.2 プロトタイプと引数

```c
#include <unistd.h>

ssize_t read(int fd, void *buf, size_t count);
```

| パラメータ | 型 | 説明 |
|-----------|-----|------|
| `fd` | `int` | 読み取り元のファイルディスクリプタ |
| `buf` | `void *` | データを格納するバッファへのポインタ |
| `count` | `size_t` | 読み取る**最大**バイト数 |

| 戻り値 | 型 | 意味 |
|--------|-----|------|
| > 0 | `ssize_t` | 実際に読み取ったバイト数 |
| 0 | `ssize_t` | ファイル末尾（EOF）に到達 |
| -1 | `ssize_t` | エラーが発生（errnoにエラーコード） |

**注意**: `size_t`は符号なし、`ssize_t`は符号ありです。
戻り値が-1（エラー）を表現するために`ssize_t`が使われます。

### 3.3 read()の動作を図で理解する

```
ファイル内容: "Hello\nWorld\n"
              H  e  l  l  o  \n  W  o  r  l  d  \n
              0  1  2  3  4  5   6  7  8  9  10 11

=== BUFFER_SIZE=5 の場合 ===

--- read() 1回目 ---
ファイルオフセット: 0
read(fd, buf, 5) → 5バイト読み取り

  ファイル:  [H][e][l][l][o][\n][W][o][r][l][d][\n]
              ^─────────────^
              offset=0       offset=5 (read後)

  buf: ['H']['e']['l']['l']['o']
        0    1    2    3    4

  戻り値: 5

--- read() 2回目 ---
ファイルオフセット: 5（前回の続き）
read(fd, buf, 5) → 5バイト読み取り

  ファイル:  [H][e][l][l][o][\n][W][o][r][l][d][\n]
                                ^─────────────^
                                offset=5       offset=10

  buf: ['\n']['W']['o']['r']['l']
        0     1    2    3    4

  戻り値: 5

--- read() 3回目 ---
ファイルオフセット: 10
read(fd, buf, 5) → 2バイトのみ読み取り（残りが少ない）

  ファイル:  [H][e][l][l][o][\n][W][o][r][l][d][\n]
                                                ^──^
                                             offset=10 offset=12

  buf: ['d']['\n']
        0    1

  戻り値: 2（要求した5より少ない!）

--- read() 4回目 ---
ファイルオフセット: 12（= ファイルサイズ = EOF）
read(fd, buf, 5) → 0バイト

  戻り値: 0（EOF）
```

### 3.4 read()が要求より少なく返す場合

**極めて重要**: read()は要求した`count`バイトより**少ないバイト数**を
返すことがあります。これはエラーではありません。

少なく返す主な理由:

| 状況 | 説明 |
|------|------|
| ファイル末尾付近 | 残りバイト数がcountより少ない |
| stdin（端末入力） | ユーザーがEnterを押した時点でread()が返る |
| パイプ | 書き込み側が送信したデータ量に依存 |
| ネットワーク | パケット単位で届くため |
| シグナル割り込み | シグナルにより中断された場合 |

```
端末からの入力の例:

BUFFER_SIZE=1024 だが、ユーザーが "Hi\n" と入力した場合:

read(0, buf, 1024)
  → ユーザーが "Hi" と打って Enter を押す
  → 戻り値: 3 ("Hi\n" の3バイト)
  → 1024バイト読もうとしたが、3バイトしか返ってこない
  → これはエラーではない!
```

### 3.5 read()のコスト - なぜバッファリングが必要か

read()はシステムコールであり、呼び出すたびに以下が発生します。

```
ユーザーモード → カーネルモード → ユーザーモード
（コンテキストスイッチ）

┌─────────────────────────────────────────────────┐
│ read(fd, buf, n) の実行過程                       │
│                                                  │
│  1. ユーザープログラムが read() を呼ぶ              │
│  2. CPUがユーザーモードからカーネルモードに切り替わる │
│     (特権レベルの変更、レジスタの保存)              │
│  3. カーネルが:                                    │
│     a. fd の有効性を検証                           │
│     b. ファイルオフセットを確認                     │
│     c. ディスクキャッシュを確認                     │
│     d. 必要ならディスクI/Oを実行                   │
│     e. データをユーザーのbufにコピー                │
│     f. ファイルオフセットを更新                     │
│  4. CPUがカーネルモードからユーザーモードに切り替わる │
│     (レジスタの復元)                               │
│  5. 呼び出し元に戻る                               │
│                                                  │
│  この一連の処理に数百〜数千CPUサイクルかかる         │
└─────────────────────────────────────────────────┘
```

**パフォーマンスの比較**:

```
10000バイトのファイルを読む場合:

BUFFER_SIZE=1:
  read()呼び出し: 10000回
  コンテキストスイッチ: 10000回
  → 非常に遅い（数百万CPUサイクルを浪費）

BUFFER_SIZE=10:
  read()呼び出し: 1000回
  コンテキストスイッチ: 1000回
  → まだ遅い

BUFFER_SIZE=1024:
  read()呼び出し: 10回
  コンテキストスイッチ: 10回
  → 実用的な速度

BUFFER_SIZE=10000:
  read()呼び出し: 1回
  コンテキストスイッチ: 1回
  → 最速（ただしメモリ使用量が増える）
```

**結論**: BUFFER_SIZEを大きくすると、read()の呼び出し回数が減り、
パフォーマンスが向上します。ただし、大きすぎるとメモリを浪費します。
このトレードオフの理解が、get_next_lineの設計の核心です。

### 3.6 read()のエラーケース

```c
ssize_t n = read(fd, buf, BUFFER_SIZE);

if (n == -1)
{
    // エラー発生。主な原因:
    // EBADF:  無効なfd（閉じられた、または開かれていない）
    // EINVAL: 無効な引数
    // EIO:    I/Oエラー（ディスク障害など）
    // EISDIR: fdがディレクトリを指している
    // EFAULT: bufが無効なアドレス
    perror("read");  // エラーメッセージを表示
}
```

get_next_lineでは、read()が-1を返した場合:
1. 確保済みの全メモリ（buf、stash）をfreeする
2. NULLを返す

### 3.7 やってみよう: read()の実験

```c
// ファイル内容が "ABCDE" (5バイト) のとき、
// 以下のプログラムの出力を予測してから実行しよう

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int	main(void)
{
	int		fd;
	char	buf[10];
	int		n;

	fd = open("test.txt", O_RDONLY);
	n = read(fd, buf, 3);
	buf[n] = '\0';
	printf("n=%d buf=[%s]\n", n, buf);
	n = read(fd, buf, 3);
	buf[n] = '\0';
	printf("n=%d buf=[%s]\n", n, buf);
	n = read(fd, buf, 3);
	printf("n=%d\n", n);
	close(fd);
	return (0);
}

// 予測:
// n=3 buf=[ABC]
// n=2 buf=[DE]  ← 3バイト要求したが2バイトしかない
// n=0           ← EOF
```

---

## 4. static変数

### 4.1 static変数とは

C言語のstatic変数は、**関数が終了してもその値を保持し続ける**変数です。
通常のローカル変数とは異なり、プログラムの実行開始から終了まで存在します。

### 4.2 通常のローカル変数の問題

なぜget_next_lineにstatic変数が必要なのか、まず通常のローカル変数の
限界を理解しましょう。

```c
// もしstatic変数がなかったら...
char	*get_next_line(int fd)
{
	char	*stash;  // 通常のローカル変数

	stash = ???;
	// 問題: 前回の呼び出しで読みすぎたデータはどこにある?
	// ローカル変数は関数が終了するたびに消滅する!
	// 前回の「残りデータ」を保持できない!
}
```

```
呼び出し1: stash = "Hello\nWorld"
           → "Hello\n" を返す
           → "World" を保持したい...
           → 関数終了 → stash消滅!

呼び出し2: stash = ??? (前回のデータは消えた!)
           → "World" を返せない!
```

### 4.3 static変数による解決

```c
char	*get_next_line(int fd)
{
	static char	*stash;  // static変数!

	// 呼び出し1: stash = NULL（初回、自動的にNULLで初期化）
	//   → read して "Hello\nWorld" を取得
	//   → "Hello\n" を返す
	//   → stash = "World" を保持
	//   → 関数終了 ... stash は消えない!

	// 呼び出し2: stash = "World"（前回の値が残っている!）
	//   → "World" から処理開始
}
```

### 4.4 static変数の特性一覧

| 特性 | ローカル変数 | static変数 | グローバル変数 |
|------|------------|-----------|-------------|
| ライフタイム | 関数実行中のみ | プログラム全体 | プログラム全体 |
| スコープ | 関数内 | **関数内** | 全ファイル |
| 初期化タイミング | 毎回の呼び出し | **一度だけ** | 一度だけ |
| デフォルト値 | 不定（未初期化） | **0/NULL** | 0/NULL |
| 格納場所 | スタック | **データセグメント** | データセグメント |

### 4.5 メモリ上の配置

static変数はスタックではなく、**データセグメント（BSS/Data）**に
配置されます。

```
プロセスのメモリレイアウト:

┌──────────────────────────┐ 高アドレス (例: 0x7fff...)
│                          │
│        スタック            │ ← ローカル変数、関数の引数
│       ↓ (下に伸びる)       │    関数呼び出しごとに確保/解放
│                          │
│                          │
│ (空き領域: スタックとヒープ │
│  が伸びる余地)             │
│                          │
│       ↑ (上に伸びる)       │
│        ヒープ              │ ← malloc()で確保
│                          │    free()で解放
├──────────────────────────┤
│   BSS セグメント           │ ← 初期化されていない static変数
│                          │    (起動時に0/NULLで初期化)
│   static char *stash;    │    ★ get_next_lineのstashはここ!
│                          │
├──────────────────────────┤
│   Data セグメント          │ ← 初期値のある static/global変数
│                          │    (例: static int x = 42;)
├──────────────────────────┤
│   Text セグメント          │ ← プログラムコード（機械語）
│                          │    読み取り専用
└──────────────────────────┘ 低アドレス (例: 0x0040...)
```

**重要な区別**:
- `static char *stash;` ← stashポインタ自体はBSSセグメント（8バイト）
- `stash = malloc(100);` ← stashが指す先のデータはヒープ（100バイト）

```
BSS:  stash [0x00007f1234560000] ─── ポインタ変数（8バイト）
                     │
                     │ ポインタの値 = 0x000055abcdef0000
                     │
                     v
Heap: [H][e][l][l][o][\0] ─── 実際のデータ（6バイト）
      0x000055abcdef0000
```

### 4.6 static変数のライフサイクル図

```
プログラム開始
      │
      v
  static char *stash が BSSセグメントに確保される
  stash = NULL (自動的に0初期化)
      │
      v
┌─── get_next_line() 呼び出し1 ───────────────────┐
│  stash == NULL                                  │
│  → ft_init_stash: stash = malloc(1) = ""        │
│  → read → stash = "Line1\nLine2"                │
│  → line = "Line1\n"                             │
│  → stash = "Line2"                              │
│  return "Line1\n"                               │
└─────────────────────────────────────────────────┘
      │  stash = "Line2" (値が保持される!)
      v
┌─── get_next_line() 呼び出し2 ───────────────────┐
│  stash == "Line2" (前回の値!)                    │
│  → stashに\nがない → read → stash = "Line2" (EOF)│
│  → line = "Line2"                               │
│  → ft_trim_stash: \nがない → free(stash), NULL   │
│  return "Line2"                                 │
└─────────────────────────────────────────────────┘
      │  stash = NULL
      v
┌─── get_next_line() 呼び出し3 ───────────────────┐
│  stash == NULL                                  │
│  → ft_init_stash: stash = malloc(1) = ""        │
│  → read → 0 (EOF)                              │
│  → stash[0] == '\0' → free(stash), NULL         │
│  return NULL                                    │
└─────────────────────────────────────────────────┘
      │  stash = NULL
      v
  プログラム終了
  BSSセグメントのstash変数もOSにより回収される
```

### 4.7 static変数の注意点

1. **スレッドセーフではない**: 複数のスレッドが同時にget_next_lineを
   呼ぶと、stashが壊れる。42の課題ではスレッドは使わないが、
   実際のプログラミングでは重要な考慮事項。

2. **テストの注意**: static変数はプログラム終了まで値を保持するため、
   テスト関数内で複数回テストする場合、前のテストの状態が残る可能性がある。

3. **初期化は一度だけ**:
   ```c
   static int x = 42;  // この初期化は「コンパイル時」に行われる
   // 関数が何回呼ばれても、x = 42 の行は「再実行されない」
   ```

### 4.8 やってみよう: static変数の実験

```c
// 以下の2つの関数を、それぞれ5回呼び出した時の
// 出力を予測してから実行しよう

void	normal_counter(void)
{
	int	count;

	count = 0;
	count++;
	printf("normal: %d\n", count);
}

void	static_counter(void)
{
	static int	count;

	count++;
	printf("static: %d\n", count);
}

// 予測:
// normal: 1, 1, 1, 1, 1  (毎回0にリセット)
// static: 1, 2, 3, 4, 5  (値が保持される)
```

---

## 5. メモリ管理: malloc / free

### 5.1 なぜ動的メモリ確保が必要か

get_next_lineでは、以下の理由で動的メモリ確保（malloc）が不可欠です。

1. **行の長さが不定**: ファイルの行は1文字かもしれないし、
   100万文字かもしれない。コンパイル時にサイズを決められない。
2. **関数間のデータ受け渡し**: get_next_lineが返すlineは
   呼び出し元で使い続ける必要がある。スタック上の配列は
   関数終了時に消えてしまう。
3. **stashの蓄積**: readで読んだデータを蓄積するstashも、
   動的にサイズを変える必要がある。

### 5.2 ヒープメモリの基本

```
┌──────────────────────────────────────────┐
│ malloc(size) の動作                       │
│                                          │
│  1. ヒープ上に size バイトの領域を確保     │
│  2. 確保した領域の先頭アドレスを返す       │
│  3. 確保できない場合は NULL を返す         │
│                                          │
│  確保されたメモリの内容は「不定」          │
│  （ゴミデータが入っている可能性がある）     │
└──────────────────────────────────────────┘
```

```c
// malloc の基本パターン
char *ptr = malloc(sizeof(char) * 10);
if (!ptr)          // 必ずNULLチェック!
    return (NULL);

// ... ptr を使用 ...

free(ptr);         // 使い終わったら必ず解放
ptr = NULL;        // ダングリングポインタ防止
```

### 5.3 get_next_lineでのmalloc/freeの全箇所

get_next_lineの実装で、mallocとfreeが呼ばれる全ての箇所を
把握することは、メモリリーク防止の第一歩です。

```
┌──────────────────────────────────────────────────────────────┐
│ malloc が呼ばれる箇所                                         │
│                                                              │
│ 1. ft_init_stash:   stash = malloc(1)                        │
│    → stashがNULLのとき、空文字列を確保                         │
│                                                              │
│ 2. ft_read_to_stash: buf = malloc(BUFFER_SIZE + 1)           │
│    → read用の一時バッファを確保                                │
│                                                              │
│ 3. ft_strjoin:      joined = malloc(len1 + len2 + 1)        │
│    → stash + buf の結合結果を確保（ループ内で複数回呼ばれる）    │
│                                                              │
│ 4. ft_get_line:     line = malloc(line_len + 1)              │
│    → 返却する行を確保                                         │
│                                                              │
│ 5. ft_trim_stash:   trimmed = malloc(remaining_len)          │
│    → stashの残りを確保                                        │
└──────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────┐
│ free が呼ばれる箇所                                           │
│                                                              │
│ 1. ft_read_to_stash: free(buf)      → ループ後、一時バッファ解放│
│ 2. ft_read_to_stash: free(buf)      → readエラー時            │
│ 3. ft_read_to_stash: free(stash)    → readエラー時            │
│ 4. ft_strjoin:       free(s1)       → 古いstashを解放         │
│ 5. ft_trim_stash:    free(stash)    → 古いstashを解放         │
│ 6. get_next_line:    free(stash)    → EOF時(stash[0]=='\0')   │
│ 7. 呼び出し元:        free(line)     → 返却されたlineを解放     │
└──────────────────────────────────────────────────────────────┘
```

### 5.4 メモリリークとは

malloc()で確保したメモリをfree()せずに、そのポインタを失うことです。

```
メモリリークの発生メカニズム:

  char *s = malloc(10);
  s → [  10バイトの領域  ]    ← ポインタsが参照している
       0x55ab00001000

  s = malloc(20);              ← sを新しいメモリで上書き!
  s → [    20バイトの領域    ]
       0x55ab00002000

  [  10バイトの領域  ]          ← 誰も参照していない!
   0x55ab00001000                 free() できない!
                                  = メモリリーク
```

```
正しいパターン:

  char *s = malloc(10);
  // ... sを使用 ...
  free(s);                // まず古いメモリを解放
  s = malloc(20);         // 次に新しいメモリを確保
```

### 5.5 get_next_lineで最も起きやすいメモリリーク

**パターン1: ft_strjoinでの古いstash解放忘れ**

```c
// 悪い例: 標準的なft_strjoin
char	*ft_strjoin(char *s1, char *s2)
{
	char	*joined;

	joined = malloc(ft_strlen(s1) + ft_strlen(s2) + 1);
	// ... s1とs2をjoinedにコピー ...
	return (joined);
	// s1（古いstash）がリーク!
}

// 呼び出し側:
stash = ft_strjoin(stash, buf);
// stashが新しいポインタに上書きされ、古いstashは解放されない!
```

```c
// 良い例: s1をfreeする特殊版ft_strjoin
char	*ft_strjoin(char *s1, char *s2)
{
	char	*joined;

	joined = malloc(ft_strlen(s1) + ft_strlen(s2) + 1);
	// ... s1とs2をjoinedにコピー ...
	free(s1);          // 古いstash(s1)を解放!
	return (joined);
}
```

**パターン2: read()エラー時のstash解放忘れ**

```c
// 悪い例
if (bytes_read == -1)
{
	free(buf);
	return (NULL);    // stashがリーク!
}

// 良い例
if (bytes_read == -1)
{
	free(buf);
	free(stash);      // stashも忘れずに解放
	return (NULL);
}
```

### 5.6 ダングリングポインタ（Dangling Pointer）

解放済みメモリを指すポインタのことです。アクセスすると未定義動作。

```
char *s = malloc(10);
free(s);

// sはまだ古いアドレスを指している!
// 0x55ab00001000 → [解放済みメモリ]

s[0] = 'A';  // 未定義動作! クラッシュの可能性
printf("%s\n", s);  // 未定義動作! ゴミデータが表示される可能性
```

```
対策: free後にNULLを代入

char *s = malloc(10);
free(s);
s = NULL;    // ポインタをNULLにする

s[0] = 'A';  // セグメンテーションフォルト（検出しやすい!）
```

### 5.7 ダブルフリー（Double Free）

同じポインタを2回freeすると未定義動作です。

```c
char *s = malloc(10);
free(s);
free(s);  // ダブルフリー! 未定義動作!
// クラッシュ、メモリ破壊、セキュリティ脆弱性の原因
```

get_next_lineでダブルフリーが起きやすいケース:

```c
// ft_trim_stash が内部で stash を free する
// → get_next_line で再度 stash を free しようとする

stash = ft_trim_stash(stash);
// ft_trim_stash 内部で stash は既に free されている
// stash には新しいポインタ（またはNULL）が返される
// → stash を再度 free してはいけない!
```

### 5.8 メモリの所有権（Ownership）

get_next_lineでは「誰がfreeする責任を持つか」を明確にすることが重要です。

```
┌──────────────────────────────────────────────────────────────┐
│ メモリ所有権のルール                                           │
│                                                              │
│ buf (読み取りバッファ):                                        │
│   確保: ft_read_to_stash 内の malloc                          │
│   解放: ft_read_to_stash 内の free (同じ関数内で完結)          │
│                                                              │
│ stash (蓄積バッファ):                                          │
│   確保: ft_init_stash, ft_strjoin, ft_trim_stash             │
│   解放: ft_strjoin(古いstash), ft_trim_stash(古いstash),      │
│         get_next_line(EOF時)                                  │
│   ★ stashの所有権は関数間で「移転」する                         │
│                                                              │
│ line (返却する行):                                             │
│   確保: ft_get_line 内の malloc                               │
│   解放: ★ 呼び出し元の責任! (get_next_lineの外)                │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 5.9 やってみよう: メモリリーク検出の練習

```c
// 以下のコードには何箇所のメモリリークがあるか?
// valgrindを使って確認してみよう

void	leaky_function(void)
{
	char	*s1;
	char	*s2;
	char	*s3;

	s1 = malloc(10);
	s2 = malloc(20);
	s3 = malloc(30);
	s1 = s3;        // (1) s1の元の10バイトがリーク
	free(s2);
	free(s1);       // (2) s3の30バイトをfree
	// s3もs1と同じポインタなのでfreeは1回でOK
	// ただしs3をこの後使うとダングリングポインタ
	return ;
	// リーク: s1の元の10バイト (1箇所)
}
```

---

## 6. Unix/LinuxのファイルI/O

### 6.1 ファイルI/Oの基本パターン

```c
// Unixにおけるファイル操作の基本3ステップ
int fd = open("file.txt", O_RDONLY);  // 1. 開く
// ... read(fd, buf, size) ...         // 2. 読む（繰り返し）
close(fd);                             // 3. 閉じる
```

```
ファイルI/Oのライフサイクル:

          open()                    close()
            │                        │
  ┌─────────v────────────────────────v──────┐
  │ ファイル未使用 │    ファイル使用中     │ ファイル未使用 │
  │              │  read()/write()可能   │              │
  │              │  オフセットが進む      │              │
  └──────────────┴───────────────────────┴──────────────┘
                       ↑
                  get_next_lineは
                  この区間で動作する
```

### 6.2 カーネルのバッファキャッシュ

実は、read()で読んだデータは必ずしもディスクから直接読まれるわけでは
ありません。LinuxカーネルはファイルデータをRAMにキャッシュしています。

```
データの流れ:

ディスク (SSD/HDD)
    │
    │ (物理的なI/O - 遅い!)
    v
┌──────────────────────────────┐
│  カーネルのページキャッシュ      │ ← RAMに格納
│  (最近アクセスしたファイルデータ) │
└──────────────┬───────────────┘
               │ (メモリ間コピー - 速い)
               v
┌──────────────────────────────┐
│  ユーザーのバッファ (buf)       │ ← あなたのプログラムのメモリ
└──────────────────────────────┘
```

ただし、read()のシステムコール自体のオーバーヘッド
（コンテキストスイッチ）は毎回発生するため、
呼び出し回数を減らすことは依然として重要です。

### 6.3 バッファリングの3つのレベル

```
レベル1: カーネルバッファ (ページキャッシュ)
  → OSが自動管理、プログラマは意識しなくてよい

レベル2: ライブラリバッファ (stdio)
  → printf(), fgets() などがFILE*内部で管理
  → get_next_lineでは使わない（read()を直接使う）

レベル3: アプリケーションバッファ (BUFFER_SIZE)
  → get_next_lineが自前で管理 ★ これを実装する!
  → stashがこの役割を果たす
```

### 6.4 なぜget_next_lineは自前のバッファリングが必要か

`read()`はバイト単位で動作しますが、get_next_lineは**行単位**で
データを返す必要があります。この不一致を解消するのがバッファリングです。

```
問題の核心:

  read()が返すデータ:  "Hello\nWor"  ← BUFFER_SIZE単位（行の途中で切れる!）
  get_next_lineが返すデータ: "Hello\n"  ← 行単位

  "Wor" はどうする? → stash に保存して次回使う!
```

```
バッファリングの必要性を示す例:

ファイル: "Short\nThis is a very long line\nEnd\n"
BUFFER_SIZE = 10

read #1: "Short\nThis"  → "Short\n" を返す、"This" を保持
read #2: " is a very"   → stash = "This is a very"
read #3: " long line"   → stash = "This is a very long line"
read #4: "\nEnd\n"      → stash = "This is a very long line\nEnd\n"
                         → "This is a very long line\n" を返す
                         → "End\n" を保持
```

---

## 7. バッファの概念

### 7.1 get_next_lineにおける2つのバッファ

| バッファ | 変数名 | ライフタイム | 役割 |
|---------|--------|------------|------|
| 読み取りバッファ | `buf` | ft_read_to_stash関数内のみ | read()の一時受け皿 |
| 蓄積バッファ | `stash` | get_next_line呼び出し間（static） | 行を構成するためのデータ蓄積 |

```
データの流れ:

read(fd, buf, BUFFER_SIZE)
         │
         v
┌─────────────┐
│ buf (一時的) │  BUFFER_SIZEバイトのデータ
│ "Hello Worl" │
└──────┬──────┘
       │ ft_strjoin(stash, buf)
       v
┌──────────────────┐
│ stash (蓄積)      │  前回の残り + 今回のread
│ "...Hello World\n"│
└──────┬───────────┘
       │
       ├── ft_get_line(stash) ──→ line = "...Hello World\n" (返却)
       │
       └── ft_trim_stash(stash) ──→ stash = "次の行のデータ..." (保持)
```

### 7.2 なぜBUFFER_SIZE+1で確保するのか

```c
buf = malloc(sizeof(char) * (BUFFER_SIZE + 1));
```

read()はヌル終端を付けません。C言語の文字列として扱うために、
自分でヌル終端を付ける必要があります。

```
BUFFER_SIZE = 5 の場合:

malloc(6) で確保:
  buf: [ ][ ][ ][ ][ ][ ]
        0   1   2   3   4   5
                              ^
                              ヌル終端用の1バイト

read(fd, buf, 5) → "Hello" (5バイト読む)
  buf: [H][e][l][l][o][ ]
        0   1   2   3   4   5

buf[bytes_read] = '\0';
  buf: [H][e][l][l][o][\0]
        0   1   2   3   4   5
                              ^
                              ここで文字列として完成
```

### 7.3 BUFFER_SIZEごとの動作の違い

同じファイル "AB\nCD\n" (6バイト) に対して:

```
=== BUFFER_SIZE = 1 ===

get_next_line() 呼び出し1:
  read → 'A'    stash = "A"     \nなし, 続行
  read → 'B'    stash = "AB"    \nなし, 続行
  read → '\n'   stash = "AB\n"  \nあり, 停止
  → line = "AB\n", stash = ""

get_next_line() 呼び出し2:
  stash = ""    \nなし
  read → 'C'    stash = "C"     \nなし, 続行
  read → 'D'    stash = "CD"    \nなし, 続行
  read → '\n'   stash = "CD\n"  \nあり, 停止
  → line = "CD\n", stash = ""

get_next_line() 呼び出し3:
  stash = ""    \nなし
  read → 0 (EOF)
  stash[0] == '\0' → return NULL

read()呼び出し回数: 7回（6データ + 1 EOF）
ft_strjoin呼び出し回数: 6回

=== BUFFER_SIZE = 5 ===

get_next_line() 呼び出し1:
  stash = ""    \nなし
  read → "AB\nCD"  stash = "AB\nCD"  \nあり, 停止
  → line = "AB\n", stash = "CD"

get_next_line() 呼び出し2:
  stash = "CD"  \nなし
  read → "\n" (1バイト, 残りのデータ)  stash = "CD\n"  \nあり, 停止
  → line = "CD\n", stash = ""

get_next_line() 呼び出し3:
  stash = ""    \nなし
  read → 0 (EOF)
  stash[0] == '\0' → return NULL

read()呼び出し回数: 3回（2データ + 1 EOF）
ft_strjoin呼び出し回数: 2回

=== BUFFER_SIZE = 100 ===

get_next_line() 呼び出し1:
  stash = ""    \nなし
  read → "AB\nCD\n" (6バイト)  stash = "AB\nCD\n"  \nあり, 停止
  → line = "AB\n", stash = "CD\n"

get_next_line() 呼び出し2:
  stash = "CD\n"  \nあり → readループに入らない!
  → line = "CD\n", stash = ""

get_next_line() 呼び出し3:
  stash = ""    \nなし
  read → 0 (EOF)
  stash[0] == '\0' → return NULL

read()呼び出し回数: 2回（1データ + 1 EOF）
ft_strjoin呼び出し回数: 1回
```

### 7.4 BUFFER_SIZEとパフォーマンスの関係（定量的分析）

```
n = ファイルサイズ（バイト数）
B = BUFFER_SIZE

read() 呼び出し回数 ≈ ceil(n / B) + 1 (EOFの1回を含む)

例: 100,000バイトのファイル

  B=1:       100,001回  ← 非常に遅い
  B=10:      10,001回
  B=100:     1,001回
  B=1024:    98回       ← 実用的
  B=4096:    25回       ← 良好（ページサイズと一致）
  B=10000:   11回
  B=100000:  2回        ← ほぼ最速
  B=10000000: 1回       ← 最速（ただしメモリ10MB消費）

  ※ Linux のメモリページサイズは通常 4096バイト
  ※ カーネルのI/Oも4096バイト単位で行われることが多い
  ※ BUFFER_SIZE=4096は理論的に効率が良い値の一つ
```

### 7.5 やってみよう: BUFFER_SIZEの影響を体感する

```bash
# 大きなテストファイルを作成
python3 -c "
for i in range(10000):
    print(f'This is line number {i:05d} with some padding text')
" > big_test.txt

# 各BUFFER_SIZEでの実行時間を比較
time cc -D BUFFER_SIZE=1 gnl.c gnl_utils.c main.c && time ./a.out > /dev/null
time cc -D BUFFER_SIZE=42 gnl.c gnl_utils.c main.c && time ./a.out > /dev/null
time cc -D BUFFER_SIZE=4096 gnl.c gnl_utils.c main.c && time ./a.out > /dev/null
time cc -D BUFFER_SIZE=1000000 gnl.c gnl_utils.c main.c && time ./a.out > /dev/null
```

---

## 8. Cプリプロセッサと-Dフラグ

### 8.1 BUFFER_SIZEの定義方法

BUFFER_SIZEはコンパイル時に`-D`フラグで定義されるマクロです。

```bash
# コンパイラに「BUFFER_SIZE = 42」を伝える
cc -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c main.c

# これは以下と同じ効果:
# ファイルの先頭に #define BUFFER_SIZE 42 が書かれたのと同じ
```

### 8.2 #ifndefによるデフォルト値設定

```c
// get_next_line.h 内
#ifndef BUFFER_SIZE
# define BUFFER_SIZE 42
#endif
```

この仕組みの動作:

```
ケース1: cc -D BUFFER_SIZE=100 ...
  → コンパイラが BUFFER_SIZE=100 を定義済み
  → #ifndef BUFFER_SIZE は「偽」（定義済みなので）
  → #define BUFFER_SIZE 42 は実行されない
  → BUFFER_SIZE = 100 が使用される

ケース2: cc ...  (BUFFER_SIZEを指定しない)
  → BUFFER_SIZE は未定義
  → #ifndef BUFFER_SIZE は「真」
  → #define BUFFER_SIZE 42 が実行される
  → BUFFER_SIZE = 42 が使用される（デフォルト値）
```

### 8.3 プリプロセスの確認方法

```bash
# プリプロセス結果を確認する（-Eオプション）
cc -E -D BUFFER_SIZE=42 get_next_line.c | less

# BUFFER_SIZE がどこで使われているか確認できる
# malloc(sizeof(char) * (42 + 1))  のように展開される
```

### 8.4 やってみよう: プリプロセスの実験

```bash
# 以下の3つのコンパイルで、BUFFER_SIZEがどう変わるか確認しよう

# 1. 明示的に指定
cc -E -D BUFFER_SIZE=100 get_next_line.h 2>/dev/null | grep "define"

# 2. 指定なし（デフォルト）
cc -E get_next_line.h 2>/dev/null | grep "define"

# 3. 0を指定（エッジケース）
cc -E -D BUFFER_SIZE=0 get_next_line.h 2>/dev/null | grep "define"
```

---

## 9. まとめ: 全ての概念のつながり

この章で学んだ概念が、get_next_lineの中でどう組み合わさるかを
整理しましょう。

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│  Unix哲学 "Everything is a file"                                │
│     └──→ fd（整数）で統一的にアクセス                             │
│           └──→ read(fd, buf, BUFFER_SIZE) でデータを読む         │
│                 │                                               │
│                 │  システムコールのコストが高い                     │
│                 │  → バッファリングで呼び出し回数を減らす            │
│                 │  → BUFFER_SIZE が大きいほど効率的                │
│                 │                                               │
│                 v                                               │
│           buf (一時バッファ) ──→ stash (蓄積バッファ)              │
│           malloc で確保         ft_strjoin で結合                 │
│           関数内で free         static変数で保持                  │
│                                 │                               │
│                                 │  \n を見つけたら行を切り出す     │
│                                 │                               │
│                                 ├──→ line (返却) → 呼び出し元    │
│                                 │    malloc で確保               │
│                                 │    呼び出し元が free            │
│                                 │                               │
│                                 └──→ trimmed stash (次回用)      │
│                                      malloc で確保               │
│                                      次の呼び出しで使用           │
│                                                                 │
│  メモリ管理: 全ての malloc に対応する free が必要                   │
│  所有権: 「誰が free するか」を常に明確にする                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 理解度セルフチェック

この章を読み終えた時点で、以下の質問に答えられるか確認しましょう。

- [ ] fdが整数である理由を2つ挙げられる
- [ ] read()の3つの戻り値の意味を即答できる
- [ ] read()が要求より少ないバイト数を返す理由を3つ挙げられる
- [ ] static変数とローカル変数の違いを4つ挙げられる
- [ ] static変数がメモリ上のどこに配置されるか説明できる
- [ ] メモリリークが起きる仕組みを図で説明できる
- [ ] BUFFER_SIZEがパフォーマンスに影響する理由を説明できる
- [ ] get_next_lineで2種類のバッファがなぜ必要か説明できる
