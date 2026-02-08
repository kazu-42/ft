# 08 - 学習目標

## このプロジェクト完了後に理解すべきこと

get_next_lineプロジェクトは、C言語における低レベルI/Oとメモリ管理の
基礎を固めるための重要なマイルストーンです。42のカリキュラムの中で、
libftで学んだ基本的なC言語スキルを「実際のシステムプログラミング」に
応用する最初の機会となります。

このプロジェクトを通じて、以下の4つの柱となるスキルを習得します。
これらは全て、後の42プロジェクト（pipex, minishell, cub3Dなど）で
直接的に活用される基盤知識です。

1. **static変数（Static Variables）** — 関数呼び出し間の状態管理
2. **ファイルI/O（File I/O）** — システムコールによる低レベル入出力
3. **バッファ管理（Buffer Management）** — 効率的なデータ読み込み設計
4. **メモリ管理（Memory Management）** — 動的メモリの安全な運用

---

## 習得スキル一覧

---

### 1. static変数マスタリー（Static Variable Mastery）

#### 1.1 理解すべき概念

##### ライフタイム（Lifetime）とスコープ（Scope）の違い

static変数の最も重要な特徴は、**スコープはローカル**だが
**ライフタイムはプログラム全体**という二面性です。

```
┌──────────────────────────────────────────────────┐
│               プログラムの実行期間                    │
│                                                  │
│  ┌─── main() ───────────────────────────────┐    │
│  │                                          │    │
│  │  get_next_line() 呼び出し1               │    │
│  │    static char *stash  ← 初回: NULL      │    │
│  │    stash = "残りデータ"                    │    │
│  │  ← 関数終了。stash は消えない！            │    │
│  │                                          │    │
│  │  get_next_line() 呼び出し2               │    │
│  │    static char *stash  ← "残りデータ"     │    │
│  │    前回の値が残っている！                   │    │
│  │  ← 関数終了                              │    │
│  │                                          │    │
│  │  get_next_line() 呼び出し3               │    │
│  │    static char *stash  ← 前回の残り       │    │
│  │    ...                                   │    │
│  │                                          │    │
│  └──────────────────────────────────────────┘    │
│                                                  │
│  stash のライフタイム: ←――――――――――――――――――→        │
│  stash のスコープ:     get_next_line() 内のみ      │
└──────────────────────────────────────────────────┘
```

**ローカル変数との対比:**

| 特性 | ローカル変数 | static変数 |
|------|------------|-----------|
| スコープ | 宣言された関数内 | 宣言された関数内（同じ） |
| ライフタイム | 関数呼び出し中のみ | プログラム全体 |
| 格納場所 | スタック（Stack） | データセグメント（Data Segment） |
| 初期化 | 毎回（明示的に初期化しないとゴミ値） | 1回のみ（暗黙的にゼロ初期化） |
| 関数間のデータ共有 | 不可能 | 同じ関数の複数回呼び出し間で可能 |

##### データセグメント（Data Segment）への格納

C言語のメモリレイアウトを理解することは、static変数の動作を
正しく理解するために不可欠です。

```
高アドレス ┌─────────────────────┐
           │      スタック        │ ← ローカル変数、関数引数
           │    （Stack）         │   関数呼び出しごとに伸縮
           │         ↓           │
           │                     │
           │         ↑           │
           │    ヒープ（Heap）     │ ← malloc/freeで管理
           │                     │   stashが指す先はここ
           ├─────────────────────┤
           │    BSS セグメント     │ ← 未初期化のstatic変数
           │   （初期値ゼロ）      │   static char *stash はここ
           ├─────────────────────┤
           │   データセグメント     │ ← 初期化済みstatic変数
           │  （Data Segment）    │   static int x = 42; はここ
           ├─────────────────────┤
           │   テキストセグメント   │ ← プログラムコード
低アドレス └─────────────────────┘
```

`static char *stash;` はBSSセグメントに配置されます。
ポインタ変数自体（8バイト、64bitシステム）がBSSにあり、
初期値は自動的にNULL（0）になります。
stashが **指す先のデータ**（mallocで確保した文字列）は
ヒープ上にあることに注意してください。

##### ゼロ初期化（Zero Initialization）の保証

C言語の仕様（C99 6.7.8p10）により、static変数は明示的な
初期化子がない場合、ゼロで初期化されます。

```c
// 以下の2つは等価
static char *stash;          // 暗黙的にNULLで初期化
static char *stash = NULL;   // 明示的にNULLで初期化

// ローカル変数は初期化されない（ゴミ値）
char *local_ptr;             // 何が入っているか不明！危険！
```

この保証があるからこそ、`get_next_line`の初回呼び出し時に
`ft_init_stash`で`stash == NULL`を確認して空文字列で初期化する
パターンが成立します。

##### get_next_lineにおけるstatic変数の具体的な役割

`get_next_line`では、`static char *stash`が以下の役割を担います:

1. **前回の読み残しデータの保持**: `read()`で読んだデータのうち、
   改行より後の部分を次回呼び出しのために保存
2. **状態フラグとしての機能**: `stash == NULL`で「初回呼び出し」
   または「EOF到達済み」を判定
3. **複数回のread()結果の蓄積**: 改行が見つかるまで複数回の
   `read()`結果を連結して蓄積

```c
// 1回目: ファイル内容 "Hello\nWorld\n", BUFFER_SIZE=10
// read() → "Hello\nWor"  → stash = "Hello\nWor"
// \n発見 → line = "Hello\n", stash = "Wor"（残り）

// 2回目: stashに "Wor" が残っている
// \nなし → read() → "ld\n" → stash = "World\n"
// \n発見 → line = "World\n", stash = ""→NULL
```

#### 1.2 到達目標

> 「static変数を使って、関数の呼び出し間で状態を保持する設計パターンを
>   実装できる。そのメリットとデメリットを説明できる。」

##### 具体的な到達基準

**レベル1（最低限）:**
- static変数のライフタイムとスコープの違いを正確に説明できる
- get_next_lineでstatic変数がなぜ必要か、1文で説明できる
- static変数の初期値がNULL（ゼロ）であることを知っている

**レベル2（合格水準）:**
- static変数がデータセグメントに格納されることを説明できる
- スタック・ヒープ・データセグメントの違いを図示できる
- static変数を使った状態保持パターンのメリット・デメリットを
  3つ以上挙げられる

**レベル3（優秀）:**
- 複数fd対応（ボーナス）で`static char *stash[MAX_FD]`を
  使う理由を、配列のインデックスとfdの対応関係で説明できる
- static変数の代替手段（構造体のポインタ渡しなど）を提案し、
  トレードオフを議論できる
- スレッドセーフの問題点を指摘し、解決策を提案できる

##### メリットとデメリットの整理

**メリット:**
1. シンプルなAPI: `get_next_line(fd)`だけで呼べる（状態管理が内部に隠蔽）
2. 呼び出し側の負担が少ない: ユーザーは状態管理を意識しなくてよい
3. 実装が直感的: 「前回の続き」を変数に残すだけ

**デメリット:**
1. スレッドセーフでない: 複数スレッドから同時に呼ぶとデータ競合が発生
2. テストしにくい: 状態がリセットできない（関数外からstashにアクセス不可）
3. 再入可能（reentrant）でない: シグナルハンドラから安全に呼べない
4. メモリリーク: プログラム終了時にstashが指すメモリが"still reachable"になる
5. 単一目的: 1つの関数に1つのstatic状態が紐づくため、汎用性が低い

##### 実世界での使用例

static変数による状態保持パターンは、C言語の標準ライブラリにも見られます:

- `strtok()`: 文字列のトークン分割で、次のトークン位置をstatic変数で保持
- `localtime()`: 結果を内部のstatic構造体に格納して返す
- `rand()`: 乱数のシードをstatic変数で保持

これらは全てスレッドセーフでないという同じ問題を抱えており、
後にスレッドセーフ版（`strtok_r()`, `localtime_r()`, `rand_r()`）が
追加されました。get_next_lineのボーナス（複数fd対応）は、
この問題の一部を解決するアプローチと見なせます。

---

### 2. ファイルI/O（File I/O）

#### 2.1 理解すべき概念

##### ファイルディスクリプタ（File Descriptor）の全体像

ファイルディスクリプタ（fd）は、Unix/Linuxシステムの
「全てはファイルである」という設計哲学の中核概念です。

```
┌─────────────────────────────────────────────────────┐
│                   プロセス                            │
│                                                     │
│  fdテーブル                                          │
│  ┌─────┬────────────────────┐                       │
│  │ fd  │ 参照先              │                       │
│  ├─────┼────────────────────┤     カーネル空間       │
│  │  0  │ stdin  ─────────────┼──→ 端末デバイス       │
│  │  1  │ stdout ─────────────┼──→ 端末デバイス       │
│  │  2  │ stderr ─────────────┼──→ 端末デバイス       │
│  │  3  │ file.txt ───────────┼──→ ディスク上のファイル │
│  │  4  │ pipe[0] ────────────┼──→ パイプの読み取り端  │
│  │  5  │ socket ─────────────┼──→ ネットワーク接続    │
│  └─────┴────────────────────┘                       │
│                                                     │
└─────────────────────────────────────────────────────┘
```

**重要なポイント:**
- fdは単なる非負整数（0以上の`int`値）
- プロセスごとに独立したfdテーブルを持つ
- `open()`は常に使用可能な最小のfd番号を返す
- fd 0, 1, 2はプロセス開始時に自動的に開かれている
- fdの上限はシステム設定で決まる（通常1024〜65536）

##### カーネルが管理するファイルオフセット（File Offset）

`read()`を呼ぶたびに、カーネルは**ファイルオフセット**を
自動的に進めます。これが「続きから読む」動作の仕組みです。

```
ファイル内容: "Hello\nWorld\n" (12バイト)
BUFFER_SIZE = 5

1回目のread(fd, buf, 5):
  ファイル: [H][e][l][l][o][\n][W][o][r][l][d][\n]
            ^^^^^^^^^^^^^^^^^
            ↑ offset=0 から 5バイト読む
            buf = "Hello"
            offset → 5

2回目のread(fd, buf, 5):
  ファイル: [H][e][l][l][o][\n][W][o][r][l][d][\n]
                                ^^^^^^^^^^^^^^^^^
                                ↑ offset=5 から 5バイト読む
                                buf = "\nWorl"
                                offset → 10

3回目のread(fd, buf, 5):
  ファイル: [H][e][l][l][o][\n][W][o][r][l][d][\n]
                                                    ^^
                                                    ↑ offset=10 から2バイトのみ読める
                                                    buf = "d\n"
                                                    offset → 12
                                                    戻り値 = 2（要求5に対して2）

4回目のread(fd, buf, 5):
  ファイル: [H][e][l][l][o][\n][W][o][r][l][d][\n]
                                                      ↑ offset=12（末尾）
                                                      戻り値 = 0（EOF）
```

**`read()`はファイルオフセットを記憶しているのではなく、
カーネルがプロセスに代わって管理している**ことを理解してください。
この仕組みが、get_next_lineで同じfdに対して複数回`read()`を
呼べる理由です。

##### `read()`の3つの戻り値パターン

```c
ssize_t bytes_read = read(fd, buf, BUFFER_SIZE);
```

| 戻り値 | 意味 | get_next_lineでの処理 |
|--------|------|---------------------|
| `> 0` | 正常読み取り。読み取ったバイト数 | bufをstashに結合して継続 |
| `== 0` | EOF到達。読み取るデータなし | ループ終了。stashの残りを処理 |
| `== -1` | エラー。`errno`にエラーコード設定 | buf・stashをfreeしてNULL返却 |

**注意**: `read()`は要求バイト数より**少ない**バイト数を返すことがあります。
これはエラーではありません。特に以下のケースで発生します:

- ファイル末尾に近い場合（残りバイト < BUFFER_SIZE）
- 端末（stdin）からの読み取り（ユーザーが入力した分だけ）
- パイプやソケットからの読み取り（利用可能なデータ分だけ）
- シグナルによる割り込み（EINTRエラー）

##### システムコールのコスト

`read()`はシステムコール（system call）です。通常の関数呼び出しとは
異なり、**ユーザー空間からカーネル空間への切り替え（コンテキストスイッチ）**
が発生します。

```
ユーザー空間          │  カーネル空間
                     │
get_next_line()      │
  │                  │
  ├→ read(fd,buf,n) ─┼──→ sys_read()
  │   [コンテキスト     │     ディスクI/O
  │    スイッチ発生]    │     バッファキャッシュ確認
  │                  │     データコピー
  │  ← 戻り値 ───────┼──← 完了
  │                  │
  ├→ ft_strjoin()    │  ← ユーザー空間内で完結
  │   [高速]         │
  │                  │
  ├→ read(fd,buf,n) ─┼──→ sys_read()
  │   [再びコスト発生]  │     ...
```

**コストの比較:**
- 通常の関数呼び出し: 数ナノ秒
- システムコール: 数マイクロ秒（約1000倍遅い）

これが**バッファリングが重要な理由**です。1バイトずつ読む（BUFFER_SIZE=1）
と、n文字の行を読むのにn回のシステムコールが発生します。
BUFFER_SIZE=4096なら、多くの場合1〜2回で済みます。

#### 2.2 到達目標

> 「Unix/Linuxのファイルシステムにおけるfdの役割を理解し、
>   `read()`を使ったバッファ付き読み込みを実装できる。」

##### 具体的な到達基準

**レベル1（最低限）:**
- `open()`, `read()`, `close()`の基本的な使い方を説明できる
- fd 0, 1, 2が何を意味するか即答できる
- `read()`の戻り値3パターンを説明できる

**レベル2（合格水準）:**
- ファイルオフセットの仕組みを図で説明できる
- `read()`がBUFFER_SIZE未満のバイト数を返すケースを列挙できる
- システムコールのコストとバッファリングの関係を説明できる
- `errno`の役割と確認方法を知っている

**レベル3（優秀）:**
- `/proc/self/fd/`を使ってfdを確認する方法を知っている
- `lseek()`によるファイルオフセット操作を理解している
- ブロッキングI/OとノンブロッキングI/Oの違いを説明できる
- `open()`のフラグ（`O_RDONLY`, `O_CREAT`等）を複数挙げられる

---

### 3. バッファ管理（Buffer Management）

#### 3.1 理解すべき概念

##### なぜ1バイトずつ読むのが非効率か

BUFFER_SIZE=1の場合と4096の場合の比較を見てみましょう。

```
ファイル: 4096バイトの1行テキスト

■ BUFFER_SIZE=1 の場合:
  read()呼び出し: 4096回
  システムコール: 4096回
  ft_strjoin呼び出し: 4096回
  コピー総バイト数: 1+2+3+...+4096 = 約840万バイト（O(n²)）

■ BUFFER_SIZE=4096 の場合:
  read()呼び出し: 1回
  システムコール: 1回
  ft_strjoin呼び出し: 1回
  コピー総バイト数: 4096バイト（O(n)）

→ 約2000倍の差！
```

##### バッファサイズとパフォーマンスの関係

バッファサイズの選択は、以下のトレードオフです:

```
パフォーマンス
    ↑
    │     ┌────────────────────────────
    │    /
    │   /
    │  /
    │ /
    │/
    └─────────────────────────────────→ BUFFER_SIZE
    1   32  128  512  4K  16K  64K

    小さすぎる: システムコール多発、O(n²)コピー
    適切（4K-8K）: システムのページサイズと一致、最も効率的
    大きすぎる: メモリ浪費、キャッシュ効率低下
```

一般的に、4096（4KB）はLinuxのメモリページサイズと一致するため、
良いデフォルト値です。42の課題でBUFFER_SIZEが42なのは、
テスト目的であり、実用的な値ではありません。

##### 読みすぎたデータの保持と再利用（stashパターン）

get_next_lineの核心的な課題は、**`read()`の読み取り単位（BUFFER_SIZE）と
データの処理単位（1行）が一致しない**ことです。

```
ファイル内容: "AB\nCD\nEF\n"  (9バイト)
BUFFER_SIZE = 5

1回目のread(): "AB\nCD"
  → 行の境界（\n）の後にもデータがある！
  → "AB\n"を返して、"CD"をstashに保存

  stashの状態遷移:
  "" → "AB\nCD" → (行を切り出し) → "CD"

2回目のget_next_line():
  stashに"CD"があるが\nがない → read()必要
  read(): "\nEF\n"
  stash: "CD\nEF\n"
  → "CD\n"を返して、"EF\n"をstashに保存

  stashの状態遷移:
  "CD" → "CD\nEF\n" → (行を切り出し) → "EF\n"

3回目のget_next_line():
  stashに"EF\n"がある → \nあり → read()不要！
  → "EF\n"を返して、stash=""→NULL

  stashの状態遷移:
  "EF\n" → (行を切り出し) → NULL
```

**stashの管理ルール:**
1. 初回はNULL → `ft_init_stash`で空文字列に初期化
2. `read()`結果を`ft_strjoin`でstashに連結
3. `\n`が見つかるまで`read()`を繰り返す
4. `ft_get_line`で先頭〜`\n`までを切り出し
5. `ft_trim_stash`で`\n`以降を新しいstashに

##### バッファとデータ処理単位の不一致への対応パターン

この「読み取り単位と処理単位の不一致」問題は、実世界のプログラミングで
頻繁に登場します:

| アプリケーション | 読み取り単位 | 処理単位 |
|----------------|------------|---------|
| get_next_line | BUFFER_SIZEバイト | 1行 |
| HTTPサーバー | TCPパケット（〜1500バイト） | HTTPリクエスト |
| JSONパーサー | バッファチャンク | 1つのJSON値 |
| ログ解析 | ファイルブロック | 1ログエントリ |

全てに共通するパターン: **「読みすぎた分を保存して次回に繰り越す」**

#### 3.2 到達目標

> 「任意のBUFFER_SIZEで正しく動作するバッファ管理ロジックを設計し、
>   バッファサイズの選択がパフォーマンスに与える影響を定量的に説明できる。」

##### 具体的な到達基準

**レベル1（最低限）:**
- `BUFFER_SIZE=1`と`BUFFER_SIZE=9999`の両方で正しく動作することを確認できる
- なぜstashが必要か、「読みすぎ」の概念を説明できる
- `ft_read_to_stash`のwhileループの終了条件を説明できる

**レベル2（合格水準）:**
- BUFFER_SIZE=1でのread()呼び出し回数を正確に計算できる
- O(n²)コピー問題を説明でき、なぜ発生するか図示できる
- stashの状態遷移を任意の入力に対してトレースできる
- バッファサイズの選択基準（ページサイズとの関係）を説明できる

**レベル3（優秀）:**
- 連結リストベースの代替実装を設計し、O(n)コピーを達成する方法を説明できる
- 標準ライブラリのbuffered I/O（`stdio.h`の`FILE`構造体）との
  設計比較ができる
- 実際にBUFFER_SIZEを変えてベンチマークを取り、結果を考察できる

---

### 4. メモリ管理（Memory Management）

#### 4.1 理解すべき概念

##### malloc/freeのライフサイクル管理

get_next_lineでは、複数のmalloc/freeが複雑に絡み合います。
全てのメモリの「所有者」と「解放責任者」を把握することが重要です。

```
┌─────────────────────────────────────────────────────────────┐
│                  メモリ所有権マップ                            │
│                                                             │
│  確保場所          │ 解放場所           │ 所有者              │
│  ─────────────────┼───────────────────┼──────────────────── │
│  ft_init_stash     │ ft_strjoin内      │ get_next_line      │
│  (stash = "")      │ (s1をfree)        │                    │
│                    │                   │                    │
│  ft_read_to_stash  │ ft_read_to_stash  │ ft_read_to_stash   │
│  (buf = malloc)    │ (free(buf))       │                    │
│                    │                   │                    │
│  ft_strjoin        │ ft_trim_stash内   │ get_next_line      │
│  (joined = malloc) │ (free(stash))     │ (via stash)        │
│                    │                   │                    │
│  ft_get_line       │ 呼び出し側        │ 呼び出し側          │
│  (line = malloc)   │ (free(line))      │ (ユーザーコード)     │
│                    │                   │                    │
│  ft_trim_stash     │ 次のft_strjoinか  │ get_next_line      │
│  (trimmed = malloc)│ 次のft_trim_stash │ (via stash)        │
└─────────────────────────────────────────────────────────────┘
```

##### メモリ所有権（Ownership）の概念

get_next_lineで最も重要な設計パターンが**所有権移転**（ownership transfer）です。

```c
// ft_strjoin: s1の所有権を受け取り、joinedの所有権を返す
char *ft_strjoin(char *s1, char *s2)
{
    // ...
    free(s1);      // s1の所有権を消費（freeする責任を果たす）
    return (joined); // joinedの所有権を呼び出し元に移転
}
```

**所有権ルール:**
1. `malloc()`した者が最初の所有者
2. 所有者は、自分でfreeするか、所有権を別の関数に渡す
3. 所有権を渡したら、元の所有者はそのポインタを使ってはいけない
4. 誰も所有権を持たないメモリ → メモリリーク
5. 2人以上が所有権を主張 → ダブルフリー

```
ft_strjoinの所有権フロー:

呼び出し前:
  stash ──→ [H][e][l][l][o][\0]    所有者: get_next_line
  buf   ──→ [\n][W][\0]            所有者: ft_read_to_stash

ft_strjoin(stash, buf) 呼び出し:
  s1 = stash  ← 所有権がft_strjoinに移転
  joined ──→ [H][e][l][l][o][\n][W][\0]  新しいメモリ確保
  free(s1)   ← 古いstashを解放
  return joined  ← 所有権がget_next_lineに移転

呼び出し後:
  stash ──→ [H][e][l][l][o][\n][W][\0]    所有者: get_next_line
  buf   ──→ [\n][W][\0]                    所有者: ft_read_to_stash（変更なし）
```

##### メモリリークの原因パターン

get_next_lineで発生しうるメモリリークのパターン:

```
パターン1: ft_strjoinでs1をfreeし忘れ
  stash → [old data]  ← 誰もfreeしない → リーク！
  stash = ft_strjoin_without_free(stash, buf)
  stash → [new data]

パターン2: エラー時のクリーンアップ漏れ
  buf = malloc(...)
  read() → -1（エラー）
  return NULL  ← bufとstashをfreeし忘れ → リーク！

パターン3: 呼び出し側でlineをfree忘れ
  line = get_next_line(fd);
  line = get_next_line(fd);  ← 最初のlineがリーク！

パターン4: EOF後のstash残留
  // 最後の行に\nがない場合
  // ft_trim_stashがstashをfreeしてNULLにするので通常は問題ない
  // ただし実装ミスでNULLにし忘れるとリーク
```

##### valgrindによるメモリリーク検出

```bash
# 基本的な使用法
valgrind --leak-check=full ./a.out

# 詳細な出力
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./a.out

# 出力例（リークがある場合）:
# ==12345== HEAP SUMMARY:
# ==12345==     in use at exit: 42 bytes in 1 blocks
# ==12345==   total heap usage: 10 allocs, 9 frees, 520 bytes allocated
#                               ↑ allocsとfreesの差がリーク数
#
# ==12345== 42 bytes in 1 blocks are definitely lost in loss record 1 of 1
# ==12345==    at 0x4C2AB80: malloc (vg_replace_malloc.c:299)
# ==12345==    by 0x400789: ft_strjoin (get_next_line_utils.c:50)
# ==12345==    by 0x400654: ft_read_to_stash (get_next_line.c:34)
#                           ↑ リークの発生元（mallocした場所）

# 出力例（リークなし）:
# ==12345== HEAP SUMMARY:
# ==12345==     in use at exit: 0 bytes in 0 blocks
# ==12345==   total heap usage: 10 allocs, 10 frees, 520 bytes allocated
# ==12345== All heap blocks were freed -- no leaks are possible
```

##### ダングリングポインタ（Dangling Pointer）の危険性

freeしたメモリを指すポインタを「ダングリングポインタ」と呼びます。

```c
// 危険な例
char *ptr = malloc(10);
free(ptr);
ptr[0] = 'A';  // 未定義動作！クラッシュする可能性

// get_next_lineでの対策
free(stash);
stash = NULL;  // NULLにすることでダングリング防止
```

get_next_lineの実装では、`ft_trim_stash`が古いstashをfreeした後に
新しいtrimmedを返し、`get_next_line`側で`stash = ft_trim_stash(stash)`
として即座に上書きすることで、ダングリングポインタを防いでいます。

##### エラー発生時のリソース解放パターン

`ft_read_to_stash`のエラーハンドリングを見てみましょう:

```c
static char *ft_read_to_stash(int fd, char *stash)
{
    char *buf;
    int  bytes_read;

    buf = malloc(sizeof(char) * (BUFFER_SIZE + 1));
    if (!buf)
        return (NULL);       // ①stashの解放は呼び出し側の責任
    bytes_read = 1;
    while (!ft_strchr(stash, '\n') && bytes_read > 0)
    {
        bytes_read = read(fd, buf, BUFFER_SIZE);
        if (bytes_read == -1)
        {
            free(buf);       // ②bufを解放
            free(stash);     // ③stashも解放
            return (NULL);   // NULLを返す → get_next_lineでstash=NULLに
        }
        buf[bytes_read] = '\0';
        stash = ft_strjoin(stash, buf);  // stashの所有権はft_strjoinに移転
    }
    free(buf);               // ④正常終了時もbufを解放
    return (stash);
}
```

**エラー時の解放フロー:**
```
malloc失敗時（①）:
  buf未確保、stashは呼び出し側がまだ所有 → NULLだけ返す
  get_next_line側でstash==NULLをチェックするが、
  この場合stashは前の値のまま → ここに注意が必要

read()エラー時（②③）:
  buf確保済み → free(buf)
  stash確保済み → free(stash)
  NULLを返す → get_next_line側でstash=NULL

正常終了時（④）:
  buf不要 → free(buf)
  stashは戻り値として返す（所有権移転）
```

#### 4.2 到達目標

> 「メモリリークゼロの実装を作成できる。valgrindの出力を読み取り、
>   問題の原因を特定して修正できる。」

##### 具体的な到達基準

**レベル1（最低限）:**
- malloc/freeの対応関係を全ての関数で追跡できる
- valgrindの基本的な使い方を知っている
- 「メモリリーク」「ダングリングポインタ」「ダブルフリー」を区別できる

**レベル2（合格水準）:**
- get_next_lineの全エラーパスでメモリリークがないことを説明できる
- valgrindの出力を読んでリークの発生箇所を特定できる
- メモリ所有権の概念を使って、誰がfreeすべきかを即答できる
- "still reachable"と"definitely lost"の違いを説明できる

**レベル3（優秀）:**
- Address Sanitizer（`-fsanitize=address`）を使ったデバッグができる
- メモリ管理のデザインパターン（RAII、所有権移転）を他の言語と比較できる
- RustのOwnershipシステムとの類似性を議論できる

---

## 今後の42プロジェクトとの関連

### libftとの関係

get_next_lineで実装したユーティリティ関数は、libftと類似しています。
ただし、以下の重要な違いがあります。

| 関数 | libft版 | get_next_line版 | 違いの理由 |
|------|---------|----------------|-----------|
| `ft_strjoin` | s1, s2とも解放しない | s1をfree（所有権移転パターン） | stashの連続的な更新に必要 |
| `ft_strlen` | NULLを渡すとクラッシュ | NULLセーフ（0を返す） | stash=NULLの場合に安全に動作させるため |
| `ft_strchr` | 標準準拠 | NULLセーフ（NULLを返す） | stash=NULLの場合に安全にチェックするため |

**学びのポイント:** 同じ名前の関数でも、使用コンテキストに応じて
インターフェース（特にエラー処理）を変える必要があることがあります。
libftに`get_next_line`を統合する際は、このAPI設計の違いに注意してください。

### Born2beroot

- ファイルシステムの概念（fdの背景知識）
- パーティション、マウントポイント、ファイルの物理的な配置
- `/dev/`ディレクトリのデバイスファイル（fdの参照先）
- システムの仕組みの理解（カーネル空間とユーザー空間）

### pipex

get_next_lineで学んだ知識が**最も直接的に**活用されるプロジェクトです。

| get_next_lineの知識 | pipexでの活用 | 具体例 |
|-------------------|-------------|-------|
| ファイルディスクリプタ | パイプのfd管理 | `pipe()`が返すfd[0]（読み取り）とfd[1]（書き込み） |
| `read()`/`write()` | プロセス間通信 | パイプを通じた子プロセスへのデータ送受信 |
| エラーハンドリング | `fork()`/`execve()`のエラー処理 | 子プロセス生成失敗時のfd閉じ忘れ防止 |
| fdのライフサイクル | `dup2()`によるfdの複製・置換 | stdin/stdoutのリダイレクト |
| バッファリング | パイプのバッファ | パイプの内部バッファ（通常64KB）の理解 |

```
pipexの処理フロー:
  infile → [cmd1] → pipe → [cmd2] → outfile

  各段階でfdを正しく管理する必要がある:
  - open(infile)   → fd3
  - pipe()         → fd4(read), fd5(write)
  - open(outfile)  → fd6
  - fork()後に不要なfdをclose()
  - dup2()でstdin/stdoutを差し替え
```

### minitalk

- ファイルディスクリプタではなくシグナルでの通信だが、
  バッファリングの概念は共通（ビット単位でデータを蓄積して1文字にする）
- 信号（SIGUSR1/SIGUSR2）を使った1ビットずつの通信は、
  BUFFER_SIZE=1のget_next_lineに概念的に似ている

### so_long / fdf / fractol

- get_next_lineを使ってマップファイルを1行ずつ読み込む
- このプロジェクトのget_next_lineを直接再利用可能
- マップ解析の典型的なパターン:

```c
// so_longでのget_next_line活用例
int fd = open("map.ber", O_RDONLY);
char *line;
int row = 0;

while ((line = get_next_line(fd)) != NULL)
{
    parse_map_row(map, line, row);
    free(line);  // 毎行freeを忘れずに！
    row++;
}
close(fd);
```

### minishell

get_next_lineの知識が**最も高度に**発展するプロジェクトです。

- ファイルディスクリプタの高度な管理（リダイレクト、パイプ、ヒアドキュメント）
- get_next_lineを使ったコマンドライン入力の読み取り
  （readline()を使う場合でも、ヒアドキュメントの実装にget_next_lineが有用）
- 複数fdの同時管理（ボーナスの知識が活用される）
- シェルの内部バッファリング

```
minishellでのfd管理:
  $ cat < input.txt | grep "hello" > output.txt

  必要なfd操作:
  1. open("input.txt", O_RDONLY) → fd3
  2. pipe() → fd4(read), fd5(write)
  3. open("output.txt", O_WRONLY|O_CREAT) → fd6
  4. fork(): cat の子プロセス
     - dup2(fd3, STDIN_FILENO)   ← get_next_lineで学んだfd知識
     - dup2(fd5, STDOUT_FILENO)
     - 不要なfd全てclose()       ← リソース管理の知識
  5. fork(): grep の子プロセス
     - dup2(fd4, STDIN_FILENO)
     - dup2(fd6, STDOUT_FILENO)
     - 不要なfd全てclose()
```

### Philosophers

- リソース管理の概念（メモリの代わりにミューテックス/フォーク）
- get_next_lineのメモリ所有権 → Philosophersのリソース排他制御
- エラー時のクリーンアップパターン → デッドロック回避のための解放順序
- get_next_lineのstatic変数の制限（スレッドセーフでない） →
  Philosophersではmutexで保護する必要性

### cub3D / miniRT

- マップ/シーンファイルの読み込みにget_next_lineを使用
- 大きなファイルの効率的な処理（BUFFER_SIZEの最適化）
- ファイルパーシングのエラーハンドリング

```c
// cub3Dでのマップ読み込み
// get_next_lineの知識があるからこそ、エッジケースを意識した
// 堅牢なパーサーが書ける
char *line = get_next_line(fd);
while (line)
{
    if (line[0] == '\n')      // 空行の処理
        ;
    else if (is_map_line(line))
        parse_map(data, line);
    else if (is_config_line(line))
        parse_config(data, line);
    free(line);
    line = get_next_line(fd);
}
```

---

## 一般的なプログラミングスキルへの貢献

### 設計力（Software Design）

#### 関数の責務分割（Single Responsibility）

get_next_lineは5つの関数に分割されており、各関数が明確な責務を持ちます:

| 関数 | 責務 | 入力 → 出力 |
|------|------|------------|
| `get_next_line` | 全体の制御・状態管理 | fd → line |
| `ft_init_stash` | stashの初期化 | stash(NULL可) → stash(非NULL) |
| `ft_read_to_stash` | ファイル読み込みとstash蓄積 | fd, stash → 更新されたstash |
| `ft_get_line` | stashから1行を切り出し | stash → line |
| `ft_trim_stash` | stashから使用済み部分を除去 | stash → 残りのstash |

この分割は**テスト容易性**と**可読性**を向上させます。
各関数を独立してテスト・デバッグできるからです。

#### インターフェース設計

`get_next_line(int fd)`という極めてシンプルなAPIは、
複雑な内部状態（stash、バッファ、ファイルオフセット）を
完全に隠蔽（カプセル化）しています。

呼び出し側が知るべきことは3つだけ:
1. fdを渡す
2. 返り値がNULLになるまで呼び続ける
3. 各返り値をfreeする

#### エラーハンドリング戦略

get_next_lineでは「NULLを返してリソースを解放する」という
一貫したエラーハンドリング戦略を採用しています。
これは後のプロジェクトでも有用なパターンです。

### デバッグ力（Debugging Skills）

- valgrindの使用経験 → 後の全プロジェクトで必須ツール
- メモリ関連バグの特定と修正 → C言語で最も頻発するバグカテゴリ
- エッジケースの発見と対処 → 堅牢なソフトウェア開発の基礎
- GDB（GNU Debugger）を使ったステップ実行・変数監視

### システムプログラミング（Systems Programming）

- システムコールの理解 → OSとの対話方法
- カーネルとユーザー空間の境界 → セキュリティとパフォーマンスの理解
- 低レベルI/Oの実践 → 高レベルライブラリの内部動作の理解
- POSIX標準の概念 → 移植性のあるコードの理解

---

## 自己評価チェックリスト

プロジェクト完了時に、以下の全てに「はい」と答えられることを目指してください。
各カテゴリは前述の4つの柱に対応しています。

### 基本理解（static変数）

- [ ] static変数のライフタイムとスコープの違いを正確に説明できる
- [ ] static変数がデータセグメント（BSS）に格納されることを知っている
- [ ] static変数のゼロ初期化が保証される理由を説明できる
- [ ] ローカル変数との違いを3つ以上挙げられる
- [ ] get_next_lineでstatic変数が必要な理由を1文で説明できる

### 基本理解（ファイルI/O）

- [ ] ファイルディスクリプタとは何か、3文で説明できる
- [ ] fd 0, 1, 2の意味を即答できる
- [ ] `read()`の3つの戻り値パターンを即答できる
- [ ] ファイルオフセットの仕組みを図で説明できる
- [ ] システムコールと通常の関数呼び出しのコスト差を説明できる

### 基本理解（バッファ管理）

- [ ] 1バイトずつ読むことが非効率な理由を定量的に説明できる
- [ ] stashの役割を「読みすぎたデータの保持」で説明できる
- [ ] 任意の入力に対してstashの状態遷移をトレースできる
- [ ] BUFFER_SIZEが1, 42, 9999のいずれでも動作する理由を説明できる

### 基本理解（メモリ管理）

- [ ] メモリリークとは何か、どう検出するか説明できる
- [ ] ダングリングポインタとダブルフリーの違いを説明できる
- [ ] valgrindの基本的な出力を読み取れる
- [ ] "still reachable"と"definitely lost"の違いを説明できる

### 実装理解

- [ ] get_next_lineの処理フローを図で描ける
- [ ] `ft_strjoin`でs1をfreeする理由を「所有権移転」で説明できる
- [ ] `ft_init_stash`が必要な理由を説明できる
- [ ] `ft_trim_stash`がNULLを返すケースとその理由を説明できる
- [ ] `BUFFER_SIZE=1`で動作する理由を説明できる
- [ ] エラー発生時のメモリ解放フローを全パスで追跡できる
- [ ] `ft_read_to_stash`のwhileループの2つの終了条件を説明できる

### 応用理解

- [ ] ボーナスの複数fd対応の仕組みを`stash[fd]`で説明できる
- [ ] BUFFER_SIZEがパフォーマンスに与える影響をO(n²)問題で説明できる
- [ ] このプロジェクトの知識がpipexでどう活用されるか、3つ以上挙げられる
- [ ] このプロジェクトの知識がminishellでどう活用されるか説明できる
- [ ] 代替実装（連結リストなど）のトレードオフを議論できる
- [ ] 標準ライブラリの`fgets()`/`getline()`との違いを説明できる
- [ ] スレッドセーフの問題点と解決策を議論できる

### 評価防衛

- [ ] 「なぜグローバル変数ではなくstatic変数を使うのか？」に即答できる
- [ ] 「BUFFER_SIZE=0の場合はどうなるか？」に即答できる
- [ ] 「stdinからの読み取りは通常のファイルと何が違うか？」に答えられる
- [ ] 「このコードのメモリリークはないことをどう証明するか？」に答えられる
- [ ] 「ft_strjoinのO(n)がget_next_lineでO(n²)になりうる理由」を説明できる

---

## まとめ: get_next_lineが42カリキュラムで果たす役割

```
    libft                get_next_line             pipex / minishell
  ┌─────────┐          ┌─────────────┐          ┌──────────────────┐
  │ C言語の  │          │ システム     │          │ プロセス管理      │
  │ 基礎     │───────→ │ プログラミング│───────→ │ シェル実装        │
  │          │          │ の入口       │          │                  │
  │ ・文字列  │          │ ・static変数  │          │ ・pipe()         │
  │ ・リスト  │          │ ・fd / read() │          │ ・fork()         │
  │ ・メモリ  │          │ ・バッファ管理│          │ ・exec()         │
  │          │          │ ・メモリ所有権│          │ ・リダイレクト    │
  └─────────┘          └─────────────┘          └──────────────────┘
        ↓                     ↓                        ↓
  基礎スキル             応用スキル               高度なスキル
  の習得               の習得                   の習得
```

get_next_lineは、libftで習得した「道具」を使って「現実の問題」を
解く最初の経験です。ここで学ぶstatic変数、ファイルI/O、
バッファ管理、メモリ所有権の概念は、42のカリキュラム全体を通じて
繰り返し活用される**基盤的なスキル**です。

このプロジェクトを丁寧に理解することが、後のpipex、minishell、
cub3Dなどの高難度プロジェクトの成功を大きく左右します。
「動けばいい」ではなく、「なぜそう動くのか」を深く理解することを
目指してください。
