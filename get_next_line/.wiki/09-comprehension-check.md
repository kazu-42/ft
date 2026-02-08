# 09 - 理解度確認

この章では、get_next_lineプロジェクトの理解度を網羅的に確認します。
基礎知識、実装の理解、エッジケース、設計選択、コードトレース（メモリ状態図付き）、
バグ発見、評価防衛Q&A、穴埋めコード問題を含む70問を用意しています。

評価前の最終チェックとして、全ての問題に答えられるか確認してください。

---

## Part 1: 基礎知識の確認（Q1〜Q10）

---

### Q1: ファイルディスクリプタ（file descriptor）とは何ですか？

**答え**: ファイルディスクリプタ（fd）は、Unix/Linuxシステムにおいて
開かれたファイルやI/Oリソースを識別するための非負整数です。
プロセスごとにfdテーブルが管理され、`open()`で新しいfdが割り当てられます。
標準入力（0）、標準出力（1）、標準エラー（2）は自動的に開かれています。

**解説**: fdはカーネル内のファイルテーブルエントリへのインデックスです。
`open()`は常に未使用の最小のfd番号を返します。例えばfd 0,1,2が使用中なら、
`open()`は3を返します。fdの上限はシステムの`ulimit -n`で確認できます。

```
プロセスのfdテーブル:
┌─────┬──────────────────────────┐
│ fd  │ 参照先                    │
├─────┼──────────────────────────┤
│  0  │ stdin（端末入力）          │
│  1  │ stdout（端末出力）         │
│  2  │ stderr（エラー出力）       │
│  3  │ file.txt（open()で取得）   │
│  4  │ pipe（パイプ読み取り端）    │
└─────┴──────────────────────────┘
```

Unixでは「全てはファイルである（Everything is a file）」という哲学があり、
通常のファイルだけでなく、端末デバイス、パイプ、ソケットも
fdを通じて統一的にアクセスできます。これがget_next_lineの
fdを引数に取る設計の背景です。

---

### Q2: `read()`の戻り値が0の場合と-1の場合の違いは？

**答え**:
- **0**: ファイルの終端（EOF）に到達した。正常な終了状態で、
  データは全て読み終わったことを意味します。
- **-1**: エラーが発生した。無効なfd、権限エラー、I/Oエラーなどが
  考えられます。`errno`にエラーの詳細が設定されます。

**解説**: get_next_lineでの処理の違いを確認しましょう。

```c
bytes_read = read(fd, buf, BUFFER_SIZE);

// 3つのパターンとget_next_lineでの対応:
if (bytes_read > 0)    // 正常読み取り → bufをstashに結合して継続
if (bytes_read == 0)   // EOF到達 → ループ終了、stashの残りを処理
if (bytes_read == -1)  // エラー → buf, stashをfreeしてNULL返却
```

重要な注意: `read()`が正の値を返す場合でも、要求した`BUFFER_SIZE`より
少ないバイト数しか読めないことがあります（部分読み取り、partial read）。
これはエラーではなく正常な動作です。特にstdin、パイプ、ソケットからの
読み取りで頻繁に発生します。

---

### Q3: static変数と通常のローカル変数の違いを4つ挙げてください。

**答え**:
1. **ライフタイム（lifetime）**: ローカル変数は関数終了時に消滅するが、
   static変数はプログラム終了まで存在する
2. **初期化**: ローカル変数は関数呼び出しごとに不定値（明示的に初期化しないとゴミ値）、
   static変数はプログラム開始時に一度だけゼロ初期化される（ポインタならNULL）
3. **格納場所**: ローカル変数はスタック（Stack）上に、
   static変数はデータセグメント（BSS）上に配置される
4. **関数間のデータ保持**: ローカル変数は関数終了で失われるが、
   static変数は次回の関数呼び出しでも前回の値を保持している

（スコープは同じ: 宣言された関数内のみ）

**解説**: get_next_lineでの具体例で理解を深めましょう。

```c
char *get_next_line(int fd)
{
    static char *stash;  // BSS上。初回NULL。関数終了後も残る
    char        *line;   // スタック上。毎回新規。関数終了で消滅

    // 1回目の呼び出し: stash = NULL（ゼロ初期化の保証）
    // 2回目の呼び出し: stash = ft_trim_stashが前回返した値
    //                  （前回の読み残しデータへのポインタ）
    // line は毎回未初期化の状態から始まる
}
```

```
メモリレイアウト:
  BSS:     [stash: 8バイトのポインタ]  ←── プログラム全体で1つ
  Stack:   [line: 関数呼び出しごとに生成・消滅]
  Heap:    [stashが指す文字列データ] ←── mallocで確保
```

---

### Q4: なぜget_next_lineでグローバル変数（global variable）は禁止されているのですか？

**答え**: グローバル変数は以下の問題があります:
1. プログラムのどこからでもアクセス・変更可能で、バグの原因になりやすい
2. 関数の独立性を損なう（副作用が見えにくい）
3. 42 Normでもグローバル変数の使用は制限されている
4. static変数はスコープが関数内に限定されるため、より安全な代替手段

**解説**: 仮にstashをグローバル変数にした場合の危険性を考えてみましょう。

```c
// 危険な実装例（グローバル変数使用）
char *g_stash;  // ファイルの先頭で宣言

void some_other_function(void)
{
    free(g_stash);       // 別の関数がstashを壊せてしまう！
    g_stash = "hello";   // mallocしていない文字列リテラルを代入
                         // → 後でfreeするとクラッシュ
}

char *get_next_line(int fd)
{
    // g_stashが別の関数で変更されている可能性がある
    // → 予測不能な動作、デバッグ困難
}
```

static変数なら、`get_next_line`の中からしかアクセスできないため、
他の関数から誤って変更されるリスクがゼロです。これが**カプセル化**
（encapsulation）の基本的な考え方です。

---

### Q5: fdが整数（int）である理由を2つ挙げてください。

**答え**:
1. **安全性（抽象化）**: ユーザープログラムがカーネル内部のデータ構造に
   直接アクセスできないようにするため（間接参照レイヤー）。
   fdは単なる「番号札」であり、実際のファイル情報はカーネル空間に隔離されています。
2. **効率性**: 整数は配列のインデックスとして使え、
   O(1)でカーネルのfdテーブルにアクセスできます。
   メモリ使用量も最小限（4バイト）です。

**解説**: この設計がget_next_lineのボーナスにも直接活きています。
`static char *stash[MAX_FD]`で`stash[fd]`としてO(1)アクセスできるのは、
fdが連続した整数であるおかげです。もしfdが文字列やポインタだったら、
ハッシュマップが必要になり、実装が大幅に複雑化します。

---

### Q6: `read()`が要求したバイト数より少ないバイト数を返す場合を3つ挙げてください。

**答え**:
1. **ファイル末尾に近い**: 残りバイト数がBUFFER_SIZEより少ない場合
2. **stdin（端末入力）**: ユーザーがEnterを押した時点で利用可能な分だけ返る
3. **パイプやソケット**: 書き込み側が送信した分だけが利用可能

**解説**: これらはエラーではなく正常な動作です。get_next_lineでは
`buf[bytes_read] = '\0'`で実際に読み取った分だけを文字列として処理するため、
部分読み取りでも正しく動作します。

```
例: BUFFER_SIZE=10, ファイル残り3バイト "XYZ"
  read(fd, buf, 10) → 戻り値 3
  buf の内容: ['X']['Y']['Z'][?][?][?][?][?][?][?]
                                ↑ この先はゴミ値
  buf[3] = '\0'
  buf の内容: ['X']['Y']['Z']['\0']  ← 正しい文字列
```

---

### Q7: BUFFER_SIZEがパフォーマンスに影響する理由は？

**答え**: `read()`はシステムコールであり、呼び出すたびに
ユーザーモードからカーネルモードへのコンテキストスイッチが発生します。
BUFFER_SIZEが小さいとread()の呼び出し回数が増え、
コンテキストスイッチのオーバーヘッドが累積して遅くなります。

**解説**: 具体的な数値で比較します。

```
10000バイトのファイルを読む場合:

BUFFER_SIZE=1:
  read()呼び出し: 10000回
  コンテキストスイッチ: 10000回（各数μs）
  ft_strjoin呼び出し: 10000回
  コピー総量: 1+2+3+...+10000 ≈ 5000万バイト（O(n²)）
  → 非常に遅い

BUFFER_SIZE=4096:
  read()呼び出し: 3回
  コンテキストスイッチ: 3回
  ft_strjoin呼び出し: 3回
  コピー総量: ≈ 30000バイト（O(n)）
  → 数千倍高速

BUFFER_SIZE=4096が推奨される理由:
  Linuxのメモリページサイズ（4KB）と一致
  → カーネルのページキャッシュと効率的に連携
```

---

### Q8: メモリリーク（memory leak）とは何ですか？get_next_lineで最も起きやすいパターンは？

**答え**: メモリリークとは、`malloc()`で確保したメモリを`free()`せずに
そのポインタを失うことです。プログラムが使えないのに解放もできないメモリが蓄積されます。

get_next_lineで最も起きやすいパターン:
`stash = ft_strjoin(stash, buf)`で、ft_strjoinが古いstash（s1）を
freeしない場合、stashポインタが新しいアドレスに上書きされ、
古いstashのメモリが解放不能になります。

**解説（メモリ状態図）**:

```
ft_strjoinでs1をfreeしない場合（バグ版）:

呼び出し前:
  stash ─→ [H][e][l][l][o][\0]  @ 0x1000 (malloc済み)

ft_strjoin(stash, " Wo") 実行:
  joined ─→ [H][e][l][l][o][ ][W][o][\0]  @ 0x2000 (新しいmalloc)
  // s1(0x1000)をfreeしない！

stash = joined の代入後:
  stash ─→ @ 0x2000

  0x1000 の "Hello" → 誰も参照していない → リーク！
  このメモリは二度とfreeできない
```

---

### Q9: ssize_tとsize_tの違いは？

**答え**:
- `size_t`: 符号なし（unsigned）整数型。0以上の値のみ。
  `malloc()`の引数や`ft_strlen()`の戻り値に使用。
- `ssize_t`: 符号あり（signed）整数型。負の値も表現可能。
  `read()`の戻り値に使用（-1を返す可能性があるため）。

**解説**: get_next_lineの実装で`int bytes_read`を使っていますが、
厳密には`ssize_t bytes_read`が型として正確です。`int`でも
通常のBUFFER_SIZEでは問題ありませんが、BUFFER_SIZEがINT_MAXを超える場合に
オーバーフローの可能性があります。42のNormではssize_tが許可されない場合があるため、
intが使われることもあります。

---

### Q10: 「Everything is a file」というUnixの哲学とget_next_lineの関係は？

**答え**: Unixの設計哲学で、通常のファイル、デバイス、パイプ、ソケットなど
異なるI/Oリソースを全て「ファイル」として統一的に扱う考え方です。

get_next_lineとの関係: get_next_lineは引数にfdを取るため、
通常のファイルだけでなく、stdin（端末入力）やパイプからも
同じインターフェースで行を読み取れます。

**解説**: 具体例で確認します。

```c
// 通常のファイルから読む
int fd = open("test.txt", O_RDONLY);
char *line = get_next_line(fd);  // ファイルから1行

// stdinから読む（fd=0）
char *input = get_next_line(0);  // 端末入力から1行

// パイプから読む
int pipefd[2];
pipe(pipefd);
char *data = get_next_line(pipefd[0]);  // パイプから1行

// 全て同じget_next_line関数で動作する！
```

---

## Part 2: 実装の理解（Q11〜Q25）

---

### Q11: BUFFER_SIZEが1の場合、"Hello\n"を読むのに何回`read()`が呼ばれますか？

**答え**: 6回です。

```
1回目: read() → 'H'  → stash="H",     ft_strchr: \nなし → 継続
2回目: read() → 'e'  → stash="He",    ft_strchr: \nなし → 継続
3回目: read() → 'l'  → stash="Hel",   ft_strchr: \nなし → 継続
4回目: read() → 'l'  → stash="Hell",  ft_strchr: \nなし → 継続
5回目: read() → 'o'  → stash="Hello", ft_strchr: \nなし → 継続
6回目: read() → '\n' → stash="Hello\n", ft_strchr: \nあり → ループ終了
```

**解説**: whileループの条件`!ft_strchr(stash, '\n') && bytes_read > 0`に注目。
各read()後にbufをstashに結合し、ft_strchrで\nを探します。
6回目で\nが見つかりループが終了します。
さらに6回のft_strjoinが呼ばれ、コピー総量は1+2+3+4+5+6=21バイトです。

---

### Q12: `ft_strjoin`でs1をfreeする理由を説明してください。

**答え**: `stash = ft_strjoin(stash, buf)`というパターンで使用するためです。
ft_strjoinは新しく結合された文字列を返しますが、古いstashのメモリは不要になります。
s1（=古いstash）を内部でfreeしないと、stashポインタが新しいアドレスに上書きされ、
古いstashへの参照が失われてメモリリークが発生します。

**解説（メモリ状態図）**:

```
ft_strjoin呼び出し前:
  stash ─→ [H][e][l][l][o][\0]       @ 0x1000 (malloc済み)
  buf   ─→ [\n][W][\0]               @ 0x2000 (malloc済み)

ft_strjoin内部処理:
  joined ─→ [H][e][l][l][o][\n][W][\0]  @ 0x3000 (新しくmalloc)
  free(s1) → 0x1000 を解放 ✓

ft_strjoin呼び出し後:
  stash ─→ @ 0x3000 = "Hello\nW"     (新しいメモリ)
  buf   ─→ @ 0x2000 = "\nW"          (変更なし、後でfree)
  0x1000: 解放済み ✓ (リークなし)

もしfree(s1)をしなかった場合:
  0x1000: "Hello" ← 誰も参照していない → メモリリーク！
```

これが**所有権移転（ownership transfer）**パターンです。
ft_strjoinはs1の所有権を受け取り（freeする責任を引き受け）、
joinedの所有権を呼び出し元に返します。

---

### Q13: `ft_init_stash`はなぜ必要ですか？

**答え**: stashがNULLの場合（初回呼び出し時、またはEOF後の再利用時）、
ft_strjoinにNULLを渡すとNULLチェックに引っかかりNULLが返されます。
空文字列""で初期化することで、ft_strjoinが安全にstashとbufを結合できます。

**解説**: 初期化を忘れた場合の問題を追跡します。

```c
// ft_init_stashがない場合の問題:
// 初回呼び出し → stash = NULL (static変数のゼロ初期化)
// ft_read_to_stash(fd, NULL)
//   → whileループ内: ft_strjoin(NULL, buf)
//     → if (!s1 || !s2) return (NULL)  ← NULLチェックで即座にNULL返却
//     → stash = NULL → NULLが返って以降何も読めない

// ft_init_stashがある場合:
// 初回呼び出し → stash = NULL
// stash = ft_init_stash(NULL) → malloc(1), stash[0]='\0' → stash = ""
// ft_read_to_stash(fd, "")
//   → ft_strjoin("", buf) → 安全に動作 ✓
```

ft_init_stashは1バイトだけmallocして'\0'を入れるシンプルな関数ですが、
このNULLチェックのパターンはC言語プログラミングで頻出します。

---

### Q14: `ft_trim_stash`が`\n`なしの場合にNULLを返す理由は？

**答え**: stashに`\n`がない場合、stash全体が最後の行として
`ft_get_line`で返されます。残りのデータがないため、
stashをfreeしてNULLを返します。次のget_next_line呼び出し時に、
ft_init_stashがNULLを検出して新しい空文字列を確保します。

**解説**: 具体例で確認します。

```
ファイル内容: "Hello"（\nなし、EOF）

ft_read_to_stash後: stash = "Hello"
ft_get_line(stash) → line = "Hello"（\nなしの行）

ft_trim_stash("Hello"):
  i=0: 'H'→i++, i=1: 'e'→i++, ..., i=5: '\0'→ループ終了
  if (!stash[5]) → true → free(stash), return NULL

stash = NULL
→ 次回呼び出しでft_init_stash → stash = ""
→ read() → 0(EOF) → stash="" → stash[0]=='\0' → return NULL
```

---

### Q15: `stash[0] == '\0'`のチェックは何のためですか？

**答え**: ft_read_to_stashから戻った後、stashが空文字列""の場合を検出します。
これはreadが0バイト返した（EOF）かつ前回の残りデータもない状態を意味します。
この場合、stashをfreeしてNULLにし、NULLを返してEOFを通知します。

**解説**: このチェックがない場合の問題:

```c
// チェックなしの場合:
// stash = ""（空文字列）
// ft_get_line("") → if(!stash || !stash[0]) return NULL → line = NULL
// ft_trim_stash("") → while(stash[i]&&...) → 即ループ終了
//                    → if(!stash[0]) → free, return NULL
// → line = NULL が返される（結果は正しい）
// しかし！stashがまだfreeされていない状態で
// ft_trim_stashに渡されるため、一手遅い解放になる
//
// 明示的なチェックで:
// stash[0]=='\0' → 即座にfree(stash), stash=NULL, return NULL
// → クリーンで意図が明確
```

---

### Q16: `ft_read_to_stash`で`bytes_read`の初期値を1にする理由は？

**答え**: whileループの条件`!ft_strchr(stash, '\n') && bytes_read > 0`で、
初回のループ実行を保証するためです。0にすると`bytes_read > 0`が偽になり、
ループに一度も入らずにstashが更新されません。

**解説**: 各初期値の動作比較:

```c
bytes_read = 0;  // ダメ: ループに入れない
bytes_read = 1;  // OK: ループに入れる
bytes_read = 42; // OK: 動作するが意図が不明確

// ただし、stashに前回の残りで既に\nがある場合:
// !ft_strchr(stash, '\n') → false → ループに入らない
// → read()は呼ばれず、bytes_readの初期値は影響しない
// → これが「可能な限り少なく読む」要件を満たす仕組み
```

---

### Q17: `buf[bytes_read] = '\0'`が必要な理由は？

**答え**: `read()`はバイト列を読み取るだけで、ヌル終端('\0')を付けません。
C言語の文字列として扱う（ft_strjoinで結合する）ために、
手動でヌル終端を追加する必要があります。

**解説**:

```
read(fd, buf, 5) → 3バイト読み取り

bufの中身（'\0'追加前）:
  ['A']['B']['C'][?][?][?]
                  ↑ ゴミ値！

buf[3] = '\0' 追加後:
  ['A']['B']['C']['\0'][?][?]
                  ↑ 正しい文字列終端

もし'\0'を付けなかった場合:
  ft_strjoin(stash, buf)
  → ft_strlen(buf) がゴミ値を読み続ける
  → バッファオーバーリード → 未定義動作（クラッシュ or ゴミデータ混入）
```

---

### Q18: `ft_strjoin`のmalloc失敗時にs1をfreeする理由は？

**答え**: ft_strjoinはs1の所有権を受け取る契約（ownership contract）です。
malloc失敗時でもs1の解放責任はft_strjoinにあります。
freeしないとs1（旧stash）がリークします。

**解説**:

```c
// ft_strjoinの正しいエラーハンドリング:
if (!joined)
{
    free(s1);      // malloc失敗でもs1の解放責任を果たす
    return (NULL);
}

// もしfree(s1)がなかった場合:
// stash = ft_strjoin(stash, buf) → NULL
// stash = NULL → 古いstashのアドレスが失われる → リーク！
```

これはmalloc失敗という稀なケースでのみ発生するバグで、
通常のテストでは見つかりにくい危険なパターンです。

---

### Q19: `ft_get_line`はstashを変更しますか？

**答え**: いいえ。ft_get_lineはstashを読み取るだけで、変更しません。
stashの内容から行を切り出して新しくmallocした文字列にコピーし、
そのポインタを返します。stashの変更はft_trim_stashが担当します。

**解説**: これは**関心の分離（Separation of Concerns）**の原則に基づいています。

```
ft_get_line: stashから行をコピー（stashは変更しない）
  stash = "AB\nCD" → line = "AB\n" (新しいmalloc)
  stash は "AB\nCD" のまま

ft_trim_stash: stashの使用済み部分を除去（stashをfreeする）
  stash = "AB\nCD" → trimmed = "CD" (新しいmalloc)
  古いstash "AB\nCD" はfree済み
```

もしft_get_lineがstashを変更してしまうと、ft_trim_stashが
正しい位置から切り出せなくなります。

---

### Q20: `ft_trim_stash`のmallocサイズ`ft_strlen(stash) - i`は正しいですか？

**答え**: はい、正しいです。

**解説**: 具体例で検証します。

```
stash = "AB\nCD" (長さ5), i = 2 (\nの位置)

ft_strlen(stash) - i = 5 - 2 = 3
コピー対象: stash[3..4] = "CD" (2文字)
必要サイズ: 2文字 + '\0' = 3バイト ✓

一般化:
  stashの長さ = L
  \nの位置 = i
  \n以降の文字数 = L - i - 1（\n自体を除く）
  必要サイズ = (L - i - 1) + 1('\0') = L - i ✓
```

---

### Q21: `read()`ではなく`fgets()`や`getline()`を使わない理由は？

**答え**:
1. **プロジェクト要件**: 使用可能な関数が`read()`, `malloc()`, `free()`のみ
2. **学習目的**: 低レベルのバッファリングI/Oを自分で実装することが課題の本質
3. **理解の深化**: fgets/getlineの内部動作を理解するために、
   同等の機能を自分で実装する

**解説**: fgets()は内部的にstdioのバッファ（通常8KB）を使い、
read()を必要な時だけ呼びます。get_next_lineで実装しているのは
まさにこの機能の簡易版です。

---

### Q22: `read()`エラー時にstashもfreeする理由は？

**答え**: read()エラーは回復不能な状態（無効なfd等）を意味します。
stashに蓄積されたデータは不完全であり、使い物になりません。
メモリリークを防ぐために、stashも含めて全リソースを解放します。
NULLを返すことで、static stashもNULLに設定され、
次回の呼び出しは初期状態から始まります。

**解説**:

```c
// ft_read_to_stash内:
if (bytes_read == -1)
{
    free(buf);    // ① 一時バッファを解放
    free(stash);  // ② 蓄積データを解放
    return (NULL); // → get_next_lineでstash = NULLになる
}
```

---

### Q23: 「可能な限り少なく読む」要件はどう満たしていますか？

**答え**: 2つの仕組みで実現しています:

1. **stashに\nがあればread()を呼ばない**: ft_read_to_stashのwhileループで
   `!ft_strchr(stash, '\n')`を最初にチェック。stashの残りに
   既に\nが含まれていれば、ループに入らずread()を一切呼びません。

2. **\nが見つかったらループを即終了**: read()後にstashに\nが含まれた時点で
   whileループ条件が偽になり、余分なread()を行いません。

**解説**:

```
例: BUFFER_SIZE=10, ファイル="AB\nCD\nEF\n"

1回目get_next_line:
  stash="", \nなし → read() → "AB\nCD\nEF\n" (全10B)
  stash="AB\nCD\nEF\n", \nあり → ループ終了（read 1回のみ）
  line="AB\n", stash="CD\nEF\n"

2回目get_next_line:
  stash="CD\nEF\n", \nあり → ループに入らない！read() 0回！
  line="CD\n", stash="EF\n"

3回目get_next_line:
  stash="EF\n", \nあり → ループに入らない！read() 0回！
  line="EF\n", stash=NULL

→ 2回目と3回目はread()を一切呼ばない = 可能な限り少なく読んでいる
```

---

### Q24: get_next_lineをNULLが返された後に、別のfdで呼ぶとどうなりますか？（必須版）

**答え**: static変数stashはNULLになっているため、
ft_init_stashで空文字列に初期化されます。
新しいfdからの読み取りが正常に行われます。
必須版では1つのstatic変数しかないため、
前のfdと新しいfdの状態が混在する危険はありません
（前のfdのstashはNULLにクリア済み）。

**解説**: ただし、前のfdがEOFまで読み切られていない場合は問題があります。

```c
// 問題のあるケース（必須版）:
get_next_line(fd1);  // stash = "前回の残り"
get_next_line(fd2);  // stash にfd1の残りデータが混在！
// → fd2のデータの前にfd1の残りが付いてしまう
```

これがボーナスの複数fd対応（`stash[MAX_FD]`）が必要な理由です。

---

### Q25: ボーナスで`static char *stash[MAX_FD]`を使う理由は？

**答え**: 複数のfdから交互に読み取る場合、各fdの読み取り状態
（残りデータ）を独立して管理する必要があります。
配列にすることで、`stash[fd]`としてfd番号をインデックスに使い、
各fdに対応する蓄積データを個別に管理できます。
O(1)でアクセス可能で、実装もシンプルです。

**解説**:

```c
// 必須版: static char *stash → 1つのfdのみ安全に管理可能
// ボーナス版:
static char *stash[MAX_FD];
// stash[3] → fd=3の残りデータ
// stash[4] → fd=4の残りデータ
// stash[5] → fd=5の残りデータ
// → 各fdが完全に独立した状態を持てる

// 64ビットシステムでのメモリ使用:
// MAX_FD=1024の場合: 1024 * 8バイト = 8KB（BSS上、ゼロ初期化）
// 各stash[fd]はNULL → 使用時にのみヒープメモリを消費
```

---

## Part 3: エッジケースの理解（Q26〜Q35）

---

### Q26: 空ファイルに対してget_next_lineを呼ぶとどうなりますか？

**答え**: NULLが返されます。

```
1. stash=NULL → ft_init_stash → stash=""
2. ft_read_to_stash:
   bytes_read=1（初期値）
   ループ条件: ft_strchr("",'\n')=NULL → 入る
   read() → 0(EOF), bytes_read=0
   buf[0]='\0' → stash=ft_strjoin("","")=""
   ループ条件: bytes_read=0 → 終了
3. stash="" → stash[0]=='\0' → free(stash), stash=NULL
4. return NULL
```

---

### Q27: "\n\n\n"というファイルの場合、get_next_lineは何回何を返しますか？

**答え**:
1. `"\n"` — 最初の改行
2. `"\n"` — 2番目の改行
3. `"\n"` — 3番目の改行
4. `NULL` — EOF

**解説**: 改行文字自体が1つの行として扱われます。
ft_get_lineは先頭から最初の\nまで（\n含む）を切り出すため、
"\n"が1行として返されます。空行（改行のみの行）は
テキストファイルで普通に存在するため、正しい動作です。

---

### Q28: fd=-1を渡した場合の動作は？

**答え**: get_next_lineの冒頭で`fd < 0`チェックに引っかかり、
即座にNULLが返されます。read()は呼ばれず、メモリも確保されません。
stashのstatic変数にも影響しません。

**解説**: stashの状態は変わりません。前回の呼び出しで
stashにデータが残っていた場合、そのデータはそのままです。
これは意図的な設計で、不正なfdでstashを壊さないようにしています。

---

### Q29: ファイルの途中でclose(fd)してからget_next_lineを呼ぶとどうなりますか？

**答え**: stashに前回の残りデータがある場合、以下のように動作します:

```
ケース1: stashに\nが含まれている
  → read()を呼ばずにstashから行を返す（正常動作）

ケース2: stashに\nが含まれていない
  → read()が呼ばれる → -1（無効なfd）
  → buf、stashをfreeしてNULL返却

ケース3: stash=NULL
  → ft_init_stash → stash=""
  → read()が呼ばれる → -1
  → buf、stashをfreeしてNULL返却
```

---

### Q30: BUFFER_SIZEがファイルサイズより大きい場合の動作は？

**答え**: 1回のread()でファイル全体が読み込まれます。
戻り値はファイルサイズ（BUFFER_SIZEより小さい値）です。
stashにファイル全体が蓄積された後、get_next_lineは
通常通り1行ずつ切り出して返します。2回目以降のget_next_lineでは
stashに既にデータがあるため、readが呼ばれないこともあります。

**解説**:

```
BUFFER_SIZE=10000, ファイル="AB\nCD\n"(6バイト)

1回目get_next_line:
  read() → "AB\nCD\n" (6バイト, bytes_read=6)
  stash = "AB\nCD\n", \nあり → ループ終了
  line = "AB\n", stash = "CD\n"

2回目get_next_line:
  stash = "CD\n", \nあり → read()不要！
  line = "CD\n", stash = NULL

3回目: NULL
```

---

### Q31: BUFFER_SIZE=0の場合、どうなりますか？

**答え**: get_next_lineの冒頭で`BUFFER_SIZE <= 0`のチェックにより
NULLを返します。

**解説**: BUFFER_SIZE=0でmallocやreadまで進んでしまうと:
- `malloc(0 + 1)` = `malloc(1)`: 成功するが意味のないバッファ
- `read(fd, buf, 0)`: 0バイト読み取り要求→戻り値0（EOF扱い）
- → 永遠にEOFとして処理される不正動作
明示的なチェックでこれを防止しています。

---

### Q32: stdinからの読み取りは通常のファイルと何が異なりますか？

**答え**:
1. ユーザーが入力を完了する（Enterを押す）まで`read()`がブロックする
2. EOFはCtrl+D（Unix/Linux）で送信される
3. ファイルサイズが事前にわからない
4. `read()`はBUFFER_SIZEより少ないバイト数を返す可能性が高い
   （ユーザーが入力した分だけ）

ただし、get_next_lineのロジック自体は同じように動作します。
fdが0（stdin）でも3（通常ファイル）でも、read()のインターフェースは同一です。

---

### Q33: 非常に長い行（100万文字）の場合、何が起きますか？

**答え**: BUFFER_SIZEごとにread()→ft_strjoinが繰り返され、
stashが徐々に大きくなります。最終的に\nが見つかるかEOFに達するまで
stashにデータが蓄積されます。メモリが十分にあれば正常に動作しますが、
BUFFER_SIZE=1の場合は100万回のft_strjoinが発生し、
O(n²)のコピーが発生するため非常に遅くなります。

**解説**: 計算量の見積もり:

```
BUFFER_SIZE=1, 行の長さ=1000000:
  ft_strjoin呼び出し: 1000000回
  コピー総量: 1+2+3+...+1000000 ≈ 5×10^11 バイト（500GB!）
  → 実際にはメモリ再確保のコストも加わり、極めて遅い

BUFFER_SIZE=4096, 行の長さ=1000000:
  ft_strjoin呼び出し: ~244回
  コピー総量: 4096+8192+...+1000000 ≈ 数百MBレベル
  → 実用的な速度
```

---

### Q34: `malloc()`が失敗した場合、プログラムはどうなるべきですか？

**答え**: NULLを返し、メモリリークを起こさないようにすべきです。
各関数でmallocの戻り値をチェックし、NULLの場合は
確保済みのリソースを適切に解放してからNULLを返します。

**解説**: get_next_lineの各malloc失敗時の対応:

```
ft_init_stash:  malloc失敗 → NULL返却 → get_next_lineがNULL返却
ft_read_to_stash(buf): malloc失敗 → NULL返却（stashは呼び出し側の責任）
ft_strjoin:     malloc失敗 → free(s1), NULL返却
ft_get_line:    malloc失敗 → NULL返却
ft_trim_stash:  malloc失敗 → free(stash), NULL返却
```

---

### Q35: 以下のファイル内容でBUFFER_SIZE=3の場合、read()は何回呼ばれますか？

ファイル内容: "ABCDEF\n" (7バイト)

**答え**: 3回呼ばれます。

```
1回目read: "ABC" (3B) → stash="ABC", \nなし → 継続
2回目read: "DEF" (3B) → stash="ABCDEF", \nなし → 継続
3回目read: "\n"  (1B) → stash="ABCDEF\n", \nあり → 終了
```

（4回目のread()は次のget_next_line呼び出し時に発生します）

---

## Part 4: 設計選択の理解（Q36〜Q45）

---

### Q36: なぜget_next_line.cとget_next_line_utils.cに分けるのですか？

**答え**:
1. **Normの制約**: 1ファイルに最大5関数まで。get_next_line全体では
   8関数あり、1ファイルに収められない
2. **関心の分離**: get_next_line.cは状態管理（static変数、read()）を担当し、
   utils.cは純粋なデータ操作（文字列処理）を担当する
3. **再利用性**: utils.cの関数（ft_strlen等）は他のプロジェクトでも使用可能

**解説**: 分割の方針:

```
get_next_line.c（状態管理層）:
  get_next_line    ← static変数を所有、全体制御
  ft_read_to_stash ← read()システムコールを呼ぶ
  ft_init_stash    ← stashの初期化

get_next_line_utils.c（データ操作層）:
  ft_strlen        ← 純粋関数（副作用なし）
  ft_strchr        ← 純粋関数
  ft_strjoin       ← メモリ操作（副作用: s1をfree）
  ft_get_line      ← メモリ操作（新しいメモリを確保）
  ft_trim_stash    ← メモリ操作（副作用: stashをfree）
```

---

### Q37: `ft_strjoin`の計算量がO(n)なのに、get_next_lineの行読み取りがO(n²)になりうる理由は？

**答え**: BUFFER_SIZE=1の場合、n文字の行を読むのにn回のft_strjoinが
呼ばれます。各ft_strjoinはstash全体をコピーするため、
コピー量は1+2+3+...+n = n(n+1)/2 = O(n²)になります。
BUFFER_SIZEが大きい場合はft_strjoinの呼び出し回数が減り、
実質的にO(n)に近づきます。

**解説**: 連結リストベースの代替実装で改善可能です。

```
文字列結合方式（現在の実装）:
  "" → "A" → "AB" → "ABC" → "ABCD"
  各ステップでstash全体をコピー → O(n²)

連結リスト方式（代替）:
  [A] → [A]-[B] → [A]-[B]-[C] → [A]-[B]-[C]-[D]
  各ステップでノード追加のみ → O(1)
  最後にまとめてコピー → O(n)
  合計 O(n)
```

---

### Q38: static変数を使う代わりの方法はありますか？なぜ使わないのですか？

**答え**:
代替案: 構造体へのポインタを引数で渡す方法

```c
char *get_next_line(int fd, t_gnl_state *state);
```

使わない理由: プロジェクト要件でプロトタイプが`char *get_next_line(int fd)`に
固定されているため。引数を追加できません。

**解説**: この代替案は実際により良い設計です。
状態を外部から渡すことで、テストが容易になり、
複数の独立した読み取りセッションを管理でき、
スレッドセーフにもなります。しかし課題の制約上、使えません。

---

### Q39: get_next_lineはスレッドセーフ（thread-safe）ですか？

**答え**: いいえ。static変数を使用しているため、スレッドセーフではありません。
複数スレッドから同時にget_next_lineを呼ぶと、stashへの同時アクセスで
データ競合（data race）が発生します。

**解説**: 解決策は3つあります:
1. **ミューテックス**: pthread_mutexでget_next_line全体をロック
2. **Thread-Local Storage**: `__thread static char *stash`で各スレッドに独立したstash
3. **リエントラント版**: 引数で状態を渡す（Q38の代替案）

---

### Q40: ボーナスの`static char *stash[MAX_FD]`のメモリ使用量は？

**答え**: 64ビットシステムでMAX_FD=1024の場合:
1024 * 8バイト = 8192バイト（8KB）がBSSセグメントに配置されます。
NULLで初期化されるため、各stash[fd]は使用時にのみヒープメモリを消費します。

---

### Q41: リンクリスト型の実装と比較したトレードオフは？

**答え**:

| 観点 | stash蓄積型 | リンクリスト型 |
|------|-----------|-------------|
| 実装の簡単さ | シンプル | 複雑 |
| Norm準拠 | 容易 | 困難（関数数/行数制限） |
| コピー効率 | O(n²)（BUFFER_SIZE=1時） | O(n) |
| メモリオーバーヘッド | なし | ノード管理の分 |
| バグリスク | 低い | 高い（リスト操作ミス） |

結論: 42の課題制約下ではstash蓄積型が適切です。

---

### Q42: なぜ`lseek()`は禁止されているのですか？

**答え**:
1. プロジェクト要件で使用可能関数が限定されている
2. get_next_lineは「順次読み取り」が目的で、ランダムアクセスは不要
3. lseek()を使えばファイルサイズ取得も可能だが、
   「可能な限り少なく読む」方針に反する
4. stdinやパイプではlseek()が使えない（シーク不可能なストリーム）

---

### Q43: `ft_strjoin`のs1とs2の非対称性（s1はfreeするがs2はしない）の理由は？

**答え**: 使用パターン`stash = ft_strjoin(stash, buf)`において:
- **s1（stash）**: 前回のmallocで確保されたデータで、結合後は不要 → free
- **s2（buf）**: ft_read_to_stash内の一時バッファで、ループ内で再利用 → freeしない
  （bufのfreeはループ終了後にft_read_to_stashが行う）

**解説**: s2もfreeしてしまうと、次のループ反復でbufが解放済みになり、
再利用できません。bufのライフサイクルはft_read_to_stashが管理しています。

---

### Q44: valgrindで"still reachable"が発生する条件は？

**答え**: get_next_lineをEOF（NULLが返される）まで呼び切らなかった場合に発生します。
例えばファイルの途中で読み取りを中止した場合、static変数stashに
まだデータが残っており、プログラム終了時にstatic変数のポインタが
まだそのメモリを指しているため、"still reachable"として報告されます。

**解説**: "still reachable"と"definitely lost"の違い:

```
"still reachable": ポインタがまだ有効 → 理論上freeできる状態
  → static char *stash が指すメモリ
  → 一般的に許容される

"definitely lost": ポインタが完全に失われた → freeする手段がない
  → ft_strjoinでs1をfree忘れした場合
  → 確実なバグ、修正必須
```

---

### Q45: get_next_lineの知識がpipexでどう活用されますか？（5つ）

**答え**:
1. **fd管理**: pipe()が返す2つのfd（読み取り端・書き込み端）の概念と管理
2. **read()/close()**: パイプからのread、使い終わったfdのclose
3. **エラーハンドリング**: fork()、execve()失敗時のリソース（fd含む）の解放パターン
4. **dup2()によるfd操作**: stdin/stdoutの置き換え（fdの番号の意味の理解）
5. **バッファリング**: パイプの内部バッファ（通常64KB）の存在の理解

---

## Part 5: コードトレース問題（メモリ状態図付き）（Q46〜Q55）

---

### Q46: 以下の実行結果はどうなりますか？

```c
// ファイル内容: "A\n" (2バイト), BUFFER_SIZE=10
char *line;
int fd = open("file.txt", O_RDONLY);

line = get_next_line(fd);  // (1)
printf("[%s]", line);
free(line);

line = get_next_line(fd);  // (2)
printf("[%s]", line ? line : "NULL");

close(fd);
```

**答え**:
1. `[A\n]` — "A\n"が返される（\nを含む）
2. `[NULL]` — EOF到達、NULLが返される

**解説（詳細トレース）**:

```
=== 1回目 get_next_line(fd) ===
stash = NULL（初回）
ft_init_stash(NULL) → malloc(1) → stash = "" [@ 0xA]

ft_read_to_stash(fd, ""):
  buf = malloc(11) [@ 0xB]
  bytes_read = 1
  ループ: ft_strchr("", '\n')=NULL, bytes_read>0 → 入る
    read(fd, buf, 10) → 2, buf = "A\n\0"
    stash = ft_strjoin("", "A\n") → malloc(3) [@ 0xC] → "A\n"
    free("") [@ 0xA]
  ループ: ft_strchr("A\n", '\n') = 非NULL → 出る
  free(buf) [@ 0xB]
  return "A\n" [@ 0xC]

stash[0]='A' ≠ '\0' → 続行
line = ft_get_line("A\n") → malloc(3) [@ 0xD] → "A\n"
stash = ft_trim_stash("A\n"):
  i=1('\n'位置), malloc(3-1=2) [@ 0xE]
  i=2, stash[2]='\0' → コピーなし → trimmed="" → free("A\n") [@ 0xC]
  return "" [@ 0xE]
stash = "" [@ 0xE]
return line="A\n" [@ 0xD]

=== 2回目 get_next_line(fd) ===
stash = "" (前回の残り)
ft_init_stash("") → stash != NULL → そのまま

ft_read_to_stash:
  buf = malloc(11) [@ 0xF]
  ループ: ft_strchr("",'\n')=NULL, bytes_read=1>0 → 入る
    read() → 0(EOF), bytes_read=0
  ループ: bytes_read=0 → 出る
  free(buf) [@ 0xF]
  return "" [@ 0xE]

stash[0]=='\0' → free("") [@ 0xE], stash=NULL, return NULL
```

---

### Q47: 以下のケースでメモリリークは発生しますか？

```c
int fd = open("file.txt", O_RDONLY);
char *line = get_next_line(fd);
// lineをfreeせずに再呼び出し
line = get_next_line(fd);
free(line);
close(fd);
```

**答え**: はい。最初のget_next_lineの戻り値（line）がfreeされずに
上書きされるため、1行分のメモリリークが発生します。

**解説（メモリ状態図）**:

```
line = get_next_line(fd);  // line → [最初の行] @ 0x100

line = get_next_line(fd);  // line → [2番目の行] @ 0x200
// 0x100 は誰も参照していない → リーク！

free(line);  // 0x200 のみ解放

結果: 0x100 のメモリがリーク（definitely lost）
```

---

### Q48: `ft_trim_stash("Hello\nWorld\nFoo")`の動作をトレースしてください。

**答え**:

```
入力: stash = "Hello\nWorld\nFoo" (15文字) @ 0x100

Step 1: \nを探す
  i=0:'H', i=1:'e', i=2:'l', i=3:'l', i=4:'o' → i=5:'\n' → 終了

Step 2: stash[5]='\n' → NULLではない → 残りデータあり

Step 3: サイズ計算
  ft_strlen("Hello\nWorld\nFoo") = 15
  15 - 5 = 10 → malloc(10) @ 0x200

Step 4: i++ → i=6（'\n'の次の文字から）

Step 5: コピー
  stash[6]='W' → trimmed[0]='W'
  stash[7]='o' → trimmed[1]='o'
  stash[8]='r' → trimmed[2]='r'
  stash[9]='l' → trimmed[3]='l'
  stash[10]='d' → trimmed[4]='d'
  stash[11]='\n' → trimmed[5]='\n'
  stash[12]='F' → trimmed[6]='F'
  stash[13]='o' → trimmed[7]='o'
  stash[14]='o' → trimmed[8]='o'
  stash[15]='\0' → ループ終了
  trimmed[9] = '\0'

Step 6: free(stash) @ 0x100

return "World\nFoo" @ 0x200
```

---

### Q49: `ft_get_line`と`ft_trim_stash`を連続で呼んだ場合のメモリ状態は？

```c
// stash = "AB\nCD" @ 0x100
line = ft_get_line(stash);
stash = ft_trim_stash(stash);
```

**答え（メモリ状態図）**:

```
初期状態:
  stash @ 0x100: [A][B][\n][C][D][\0]

ft_get_line("AB\nCD") 実行後:
  line @ 0x200:  [A][B][\n][\0]          ← 新しくmalloc
  stash @ 0x100: [A][B][\n][C][D][\0]   ← 変更なし

ft_trim_stash("AB\nCD") 実行後:
  line @ 0x200:   [A][B][\n][\0]         ← そのまま
  stash @ 0x300:  [C][D][\0]             ← 新しくmalloc
  0x100: 解放済み                         ← ft_trim_stash内でfree

最終メモリ状態:
  ヒープ上に2つのブロック:
  - 0x200: "AB\n" → 呼び出し側がfreeする責任
  - 0x300: "CD"   → stash（static変数）が保持
```

---

### Q50: BUFFER_SIZE=2, ファイル内容="AB\n"の場合の完全トレース

**答え**:

```
=== get_next_line(fd) 呼び出し1 ===

stash = NULL → ft_init_stash → malloc(1) → "" [@ 0xA]
buf = malloc(3) [@ 0xB]

ループ1:
  read(fd, buf, 2) → 2, buf="AB"
  stash = ft_strjoin("", "AB"):
    malloc(3) [@ 0xC] → "AB"
    free("") [@ 0xA]
  stash = "AB" [@ 0xC]
  ft_strchr("AB", '\n') = NULL → 継続

ループ2:
  read(fd, buf, 2) → 1, buf="\n"
  stash = ft_strjoin("AB", "\n"):
    malloc(4) [@ 0xD] → "AB\n"
    free("AB") [@ 0xC]
  stash = "AB\n" [@ 0xD]
  ft_strchr("AB\n", '\n') = 非NULL → 終了

free(buf) [@ 0xB]

line = ft_get_line("AB\n") → malloc(4) [@ 0xE] → "AB\n"
stash = ft_trim_stash("AB\n"):
  i=2, stash[2]='\n'
  malloc(4-2=2) [@ 0xF] → ""
  i=3, stash[3]='\0' → コピーなし → trimmed=""
  free("AB\n") [@ 0xD]
  return "" [@ 0xF]

return "AB\n" [@ 0xE]

malloc/free追跡:
  malloc: 0xA(1), 0xB(3), 0xC(3), 0xD(4), 0xE(4), 0xF(2) = 6回
  free:   0xA, 0xB, 0xC, 0xD = 4回
  使用中: 0xE(line), 0xF(stash) = 2ブロック

=== get_next_line(fd) 呼び出し2 ===

stash = "" [@ 0xF]
ft_init_stash: stash != NULL → そのまま

buf = malloc(3) [@ 0xG]
ループ: ft_strchr("",'\n')=NULL → 入る
  read() → 0(EOF), bytes_read=0
ループ: bytes_read=0 → 出る
free(buf) [@ 0xG]

stash="" → stash[0]=='\0' → free("") [@ 0xF], stash=NULL, return NULL

最終: malloc=8回, free=6回 (+呼び出し側のfree(line)で7回) → 使用中=0 ✓
```

---

### Q51: BUFFER_SIZE=10, ファイル内容="\n"（改行のみ1バイト）の場合のトレース

**答え**:

```
get_next_line(fd) 呼び出し1:

  stash = NULL → init → "" [malloc(1)]
  buf = malloc(11)

  ループ: ft_strchr("",'\n')=NULL → 入る
    read() → "\n" (1バイト), bytes_read=1
    buf[1]='\0' → buf="\n"
    stash = ft_strjoin("", "\n") → "\n" [malloc(2), free("")]
  ループ: ft_strchr("\n",'\n') = 非NULL → 出る
  free(buf)

  line = ft_get_line("\n"):
    i=0: '\n' → ループ即終了
    stash[0]=='\n' → i++ → i=1
    malloc(2) → "\n\0"
    return "\n"

  stash = ft_trim_stash("\n"):
    i=0: '\n' → ループ即終了
    stash[0]='\n' → 非NULL → 残りデータ処理
    ft_strlen("\n") - 0 = 1 → malloc(1)
    i++ → i=1, stash[1]='\0' → コピーなし
    trimmed = "" → free("\n")
    return ""

  return "\n"

get_next_line(fd) 呼び出し2:
  stash = ""
  read() → 0(EOF)
  stash[0]=='\0' → free, return NULL

結果: 1回目 "\n", 2回目 NULL
```

---

### Q52: 以下のコードでstashの値はいくつですか？（各行の後）

```c
// BUFFER_SIZE=5, ファイル内容="Hello World\n" (12バイト)
// get_next_line(fd) の内部実行

stash = ft_init_stash(stash);        // stash = ?
// read → "Hello"
stash = ft_strjoin(stash, buf);      // stash = ?
// read → " Worl"
stash = ft_strjoin(stash, buf);      // stash = ?
// read → "d\n"
stash = ft_strjoin(stash, buf);      // stash = ?
line = ft_get_line(stash);           // line = ?
stash = ft_trim_stash(stash);        // stash = ?
```

**答え**:
1. `stash = ""`（空文字列、初回のためNULL→""に初期化）
2. `stash = "Hello"`（""と"Hello"を結合）
3. `stash = "Hello Worl"`（"Hello"と" Worl"を結合）
4. `stash = "Hello World\n"`（"Hello Worl"と"d\n"を結合）
5. `line = "Hello World\n"`（先頭から\nまでを切り出し）
6. `stash = NULL`（\nの後に何もない → free → NULL）

---

### Q53: 以下のコードの問題点を全て見つけてください。

```c
char *get_next_line(int fd)
{
    static char *stash;
    char *line;

    stash = ft_init_stash(stash);
    stash = ft_read_to_stash(fd, stash);
    if (!stash)
        return (NULL);
    line = ft_get_line(stash);
    stash = ft_trim_stash(stash);
    return (line);
}
```

**答え**: 3つの問題があります:
1. **`fd < 0 || BUFFER_SIZE <= 0`のチェックがない**: 不正な引数で
   read()が呼ばれ、未定義動作やエラーの原因になる
2. **`ft_init_stash`の戻り値のNULLチェックがない**: malloc失敗時に
   stash=NULLのままft_read_to_stashに渡される
3. **`stash[0] == '\0'`のチェックがない**: EOF時にstashが空文字列""のまま
   ft_get_line/ft_trim_stashに渡され、stashが適切に解放されない
   可能性がある（"still reachable"リーク）

---

### Q54: 以下の`ft_strjoin`にバグはありますか？

```c
char *ft_strjoin(char *s1, char *s2)
{
    size_t i;
    size_t j;
    char *joined;

    joined = malloc(ft_strlen(s1) + ft_strlen(s2) + 1);
    if (!joined)
        return (NULL);
    i = 0;
    while (s1[i])
    {
        joined[i] = s1[i];
        i++;
    }
    j = 0;
    while (s2[j])
    {
        joined[i + j] = s2[j];
        j++;
    }
    joined[i + j] = '\0';
    free(s1);
    return (joined);
}
```

**答え**: 2つの問題があります:
1. **s1, s2のNULLチェックがない**: NULLを渡すとft_strlen(NULL)は0を返すが
   `while(s1[i])`でNULLポインタ参照 → セグフォルト
2. **malloc失敗時にs1をfreeしていない**: s1(旧stash)がリーク

正しくは冒頭に`if (!s1 || !s2) return (NULL);`を追加し、
malloc失敗時に`free(s1); return (NULL);`とすべきです。

---

### Q55: 以下のメモリ状態から、次に何が起きるかを答えてください。

```
ヒープの状態:
  [stash: "AB\nCD\nEF" at 0x100]
  [buf: "GH\n" at 0x200]

ft_strjoin(stash, buf) が呼ばれる
```

**答え**:

```
Step 1: サイズ計算
  ft_strlen("AB\nCD\nEF") = 8
  ft_strlen("GH\n") = 3
  malloc(8 + 3 + 1 = 12) → "AB\nCD\nEFGH\n\0" @ 0x300

Step 2: s1のコピー
  joined = "AB\nCD\nEF"

Step 3: s2のコピー
  joined = "AB\nCD\nEFGH\n"

Step 4: '\0'追加
  joined[11] = '\0'

Step 5: free(s1) → 0x100 解放

Step 6: return 0x300

呼び出し後:
  stash → 0x300 = "AB\nCD\nEFGH\n"
  buf → 0x200 = "GH\n"（変更なし）
  whileループ: ft_strchr("AB\nCD\nEFGH\n", '\n') → 位置2 → 非NULL → 終了
```

---

## Part 6: バグ発見問題（Q56〜Q60）

---

### Q56: 以下の`ft_read_to_stash`にメモリリークがあります。見つけてください。

```c
static char *ft_read_to_stash(int fd, char *stash)
{
    char *buf;
    int bytes_read;

    buf = malloc(sizeof(char) * (BUFFER_SIZE + 1));
    if (!buf)
        return (NULL);
    bytes_read = 1;
    while (!ft_strchr(stash, '\n') && bytes_read > 0)
    {
        bytes_read = read(fd, buf, BUFFER_SIZE);
        if (bytes_read == -1)
        {
            free(buf);
            return (NULL);     // ← ここに問題!
        }
        buf[bytes_read] = '\0';
        stash = ft_strjoin(stash, buf);
    }
    free(buf);
    return (stash);
}
```

**答え**: read()エラー時にstashをfreeしていません。
stashはmallocで確保されたメモリを指しているため、
freeせずにNULLを返すとメモリリークが発生します。

**修正**:
```c
if (bytes_read == -1)
{
    free(buf);
    free(stash);     // ← stashも解放する!
    return (NULL);
}
```

**解説**: この関数がNULLを返すと、get_next_lineで`stash = NULL`になります。
stashをfreeせずにNULLにすると、古いstashのアドレスが失われ
"definitely lost"リークになります。

---

### Q57: 以下の`ft_get_line`にバグがあります。見つけてください。

```c
char *ft_get_line(char *stash)
{
    size_t i;
    char   *line;

    i = 0;
    while (stash[i] && stash[i] != '\n')
        i++;
    if (stash[i] == '\n')
        i++;
    line = malloc(sizeof(char) * i);  // ← バグ
    if (!line)
        return (NULL);
    i = 0;
    while (stash[i] && stash[i] != '\n')
    {
        line[i] = stash[i];
        i++;
    }
    if (stash[i] == '\n')
        line[i++] = '\n';
    line[i] = '\0';
    return (line);
}
```

**答え**: 2つのバグがあります:
1. `malloc(i)`ではなく`malloc(i + 1)`が必要。'\0'のための1バイトが不足。
   `line[i] = '\0'`でバッファオーバーフロー（ヒープオーバーフロー）
2. `!stash || !stash[0]`のNULLチェックがない。NULLポインタ参照の危険

---

### Q58: 以下の`ft_trim_stash`にバグがあります。見つけてください。

```c
char *ft_trim_stash(char *stash)
{
    size_t i;
    size_t j;
    char   *trimmed;

    i = 0;
    if (!stash)
        return (NULL);
    while (stash[i] && stash[i] != '\n')
        i++;
    if (!stash[i])
    {
        free(stash);
        return (NULL);
    }
    trimmed = malloc(sizeof(char) * (ft_strlen(stash) - i));
    if (!trimmed)
        return (NULL);  // ← バグ
    i++;
    j = 0;
    while (stash[i])
        trimmed[j++] = stash[i++];
    trimmed[j] = '\0';
    free(stash);
    return (trimmed);
}
```

**答え**: malloc失敗時にstashをfreeしていません。
trimmedのmallocが失敗した場合、stashのメモリが解放されずリークします。

**修正**:
```c
if (!trimmed)
{
    free(stash);   // ← stashもfreeする
    return (NULL);
}
```

---

### Q59: 以下の呼び出しコードのバグを全て指摘してください。

```c
int fd = open("file.txt", O_RDONLY);
char *line;

while (1)
{
    line = get_next_line(fd);
    printf("%s", line);
    free(line);
}
close(fd);
```

**答え**: 3つのバグがあります:
1. `line`がNULLの場合（EOF時）、`printf("%s", NULL)`は未定義動作
   （多くの実装で"(null)"と表示されるが保証なし）
2. whileループが無限ループになり、EOF後もget_next_lineを呼び続ける
3. `close(fd)`に到達しない（無限ループのため）

**修正**:
```c
while ((line = get_next_line(fd)) != NULL)
{
    printf("%s", line);
    free(line);
}
close(fd);
```

---

### Q60: 以下のプログラムのvalgrind出力を予測してください。

```c
int main(void)
{
    int fd = open("test.txt", O_RDONLY); // "Hello\n"
    char *line1 = get_next_line(fd);
    char *line2 = get_next_line(fd); // NULL
    free(line1);
    close(fd);
    return (0);
}
```

**答え**:
```
==XXXXX== HEAP SUMMARY:
==XXXXX==     in use at exit: 0 bytes in 0 blocks
==XXXXX==   total heap usage: X allocs, X frees, Y bytes allocated
==XXXXX== All heap blocks were freed -- no leaks are possible
==XXXXX== ERROR SUMMARY: 0 errors from 0 contexts
```

リークなしです。理由:
- line1: `free(line1)`で解放される
- line2: NULLなのでfree不要
- stash: 2回目の呼び出しでEOF到達時にfree→NULLに設定される
- buf: ft_read_to_stash内で毎回freeされる

---

## Part 7: 評価防衛Q&A（Q61〜Q65）

---

### Q61: 「このプロジェクトで最も注意が必要な点は何ですか？」

**模範回答**: メモリ管理です。特に以下の3点に注意しました:
1. **ft_strjoinで古いstashをfreeする**: 結合のたびに古いメモリを解放し、
   所有権を新しいメモリに移転しています
2. **read()エラー時のクリーンアップ**: stashとbufの両方を確実にfreeします
3. **EOF到達時のstash解放**: 最後のget_next_line呼び出しで
   stashをfreeしてNULLに設定します

valgrindで`--leak-check=full --show-leak-kinds=all`を使って
全パスでリークがないことを確認しています。

---

### Q62: 「BUFFER_SIZE=10000000でもプログラムは動きますか？」

**模範回答**: はい、動作します。`malloc(10000001)`が成功すればですが、
メモリ不足でmallocが失敗した場合はNULLを返します。
mallocの戻り値を全箇所でチェックしているため、クラッシュはしません。

```bash
# 実際にテスト可能:
cc -D BUFFER_SIZE=10000000 get_next_line.c get_next_line_utils.c main.c
valgrind --leak-check=full ./a.out
```

---

### Q63: 「なぜグローバル変数ではなくstatic変数を使うのですか？」

**模範回答**: 3つの理由があります:
1. **スコープの制限**: static変数はget_next_line関数内からしかアクセスできず、
   他の関数から誤って変更されるリスクがありません
2. **Normの要件**: 42のコーディング規約でグローバル変数は原則禁止です
3. **カプセル化**: 内部状態を隠蔽することで、APIがシンプルになり、
   呼び出し側は`get_next_line(fd)`だけ知っていれば使えます

---

### Q64: 「valgrindの"still reachable"は問題ですか？」

**模範回答**: "still reachable"はプログラム終了時にまだポインタが
残っているメモリです。static変数stashが最後にNULL以外を指している場合に
発生します。厳密にはリークですが、OSがプロセス終了時に回収するため、
一般的には許容されます。

"definitely lost"は確実なリークなので問題です。
私の実装では、get_next_lineをEOFまで呼び切れば（NULLが返されるまで）、
stashはNULLになり"still reachable"も発生しません。

---

### Q65: 「代替実装として連結リストを使う方法を知っていますか？」

**模範回答**: はい。連結リストを使うと、ft_strjoinの繰り返しによる
O(n²)のコピーコストをO(n)に削減できます。

```
現在の方式:
  stash = ft_strjoin(stash, buf) を繰り返す
  → 毎回stash全体をコピー → O(n²)

連結リスト方式:
  各read()の結果をリストのノードとして追加（O(1)）
  \nが見つかったら全ノードをまとめて1つの文字列にコピー（O(n)）
  → 合計 O(n)
```

トレードオフ:
- 連結リスト: メモリ効率が良いが実装が複雑、ノード管理のバグリスク
- 文字列結合: 実装がシンプルだがBUFFER_SIZE=1では非効率

この課題では実装のシンプルさとNorm準拠を優先して文字列結合方式を採用しました。

---

## Part 8: 穴埋めコード問題（Q66〜Q70）

---

### Q66: `ft_read_to_stash`の穴埋め

以下のコードの`/* A */`〜`/* D */`を埋めてください。

```c
static char *ft_read_to_stash(int fd, char *stash)
{
    char *buf;
    int  bytes_read;

    buf = malloc(sizeof(char) * (/* A */ + 1));
    if (!buf)
        return (NULL);
    bytes_read = 1;
    while (!ft_strchr(stash, /* B */) && bytes_read > 0)
    {
        bytes_read = read(fd, buf, /* C */);
        if (bytes_read == /* D */)
        {
            free(buf);
            free(stash);
            return (NULL);
        }
        buf[bytes_read] = '\0';
        stash = ft_strjoin(stash, buf);
    }
    free(buf);
    return (stash);
}
```

**答え**:
- `/* A */` = `BUFFER_SIZE` — バッファの確保サイズ（+1は'\0'用）
- `/* B */` = `'\n'` — 改行文字を探す
- `/* C */` = `BUFFER_SIZE` — read()の読み取り要求サイズ
- `/* D */` = `-1` — read()のエラー戻り値

**解説**: `BUFFER_SIZE + 1`でmallocするのは、read()がBUFFER_SIZEバイトまで
読み取った後に`buf[bytes_read] = '\0'`でヌル終端を追加するためです。
BUFFER_SIZE分だけmallocすると、最大読み取り時にバッファオーバーフローです。

---

### Q67: `ft_get_line`の穴埋め

以下のコードの`/* A */`〜`/* D */`を埋めてください。

```c
char *ft_get_line(char *stash)
{
    size_t i;
    char   *line;

    i = 0;
    if (!stash || !stash[0])
        return (NULL);
    while (stash[i] && stash[i] != /* A */)
        i++;
    if (stash[i] == '\n')
        /* B */;
    line = malloc(sizeof(char) * (/* C */));
    if (!line)
        return (NULL);
    i = 0;
    while (stash[i] && stash[i] != '\n')
    {
        line[i] = stash[i];
        i++;
    }
    if (stash[i] == '\n')
        line[/* D */] = '\n';
    line[i] = '\0';
    return (line);
}
```

**答え**:
- `/* A */` = `'\n'` — 改行文字まで進む
- `/* B */` = `i++` — \nを含めるためにカウントを1増やす
- `/* C */` = `i + 1` — 文字数 + '\0'の分
- `/* D */` = `i++` — line[i]に'\n'を書き込み、iをインクリメント

**解説**: `line[i++] = '\n'`は「line[i]に'\n'を書き込んでからiを1増やす」
という意味です。この後の`line[i] = '\0'`で、'\n'の次の位置に
ヌル終端が正しく置かれます。

---

### Q68: `ft_trim_stash`の穴埋め

以下のコードの`/* A */`〜`/* D */`を埋めてください。

```c
char *ft_trim_stash(char *stash)
{
    size_t i;
    size_t j;
    char   *trimmed;

    i = 0;
    if (!stash)
        return (NULL);
    while (stash[i] && stash[i] != '\n')
        i++;
    if (/* A */)
    {
        free(stash);
        return (NULL);
    }
    trimmed = malloc(sizeof(char) * (/* B */));
    if (!trimmed)
    {
        free(stash);
        return (NULL);
    }
    /* C */;
    j = 0;
    while (stash[i])
        trimmed[j++] = stash[/* D */];
    trimmed[j] = '\0';
    free(stash);
    return (trimmed);
}
```

**答え**:
- `/* A */` = `!stash[i]` — \nが見つからなかった（文字列末尾に到達）
- `/* B */` = `ft_strlen(stash) - i` — 残りの文字数 + '\0'
- `/* C */` = `i++` — \nをスキップして次の文字から開始
- `/* D */` = `i++` — stashの残り部分を1文字ずつコピー

**解説**: `!stash[i]`は`stash[i] == '\0'`と同義。whileループが
\nを見つけずに末尾に到達した = データ全体が1行として使われた
= 残りデータなし → stashをfreeしてNULL返却。

---

### Q69: `get_next_line`の穴埋め

以下のコードの`/* A */`〜`/* D */`を埋めてください。

```c
char *get_next_line(int fd)
{
    /* A */ char *stash;
    char         *line;

    if (fd < 0 || /* B */ <= 0)
        return (NULL);
    stash = ft_init_stash(stash);
    if (!stash)
        return (NULL);
    stash = ft_read_to_stash(fd, stash);
    if (!stash)
        return (NULL);
    if (/* C */)
    {
        free(stash);
        stash = NULL;
        return (NULL);
    }
    line = ft_get_line(stash);
    stash = /* D */(stash);
    return (line);
}
```

**答え**:
- `/* A */` = `static` — 呼び出し間で状態を保持
- `/* B */` = `BUFFER_SIZE` — バッファサイズの妥当性チェック
- `/* C */` = `stash[0] == '\0'` — 空文字列（データなし/EOF）の判定
- `/* D */` = `ft_trim_stash` — 使用済み部分を除去して残りを新しいstashに

---

### Q70: `ft_strjoin`の穴埋め

以下のコードの`/* A */`〜`/* D */`を埋めてください。

```c
char *ft_strjoin(char *s1, char *s2)
{
    size_t i;
    size_t j;
    char   *joined;

    if (!s1 || !s2)
        return (NULL);
    joined = malloc(sizeof(char) * (ft_strlen(s1) + ft_strlen(s2) + /* A */));
    if (!joined)
    {
        /* B */;
        return (NULL);
    }
    i = 0;
    while (s1[i])
    {
        joined[i] = s1[i];
        i++;
    }
    j = 0;
    while (s2[j])
    {
        joined[/* C */] = s2[j];
        j++;
    }
    joined[i + j] = '\0';
    /* D */;
    return (joined);
}
```

**答え**:
- `/* A */` = `1` — '\0'終端文字のための1バイト
- `/* B */` = `free(s1)` — malloc失敗時にs1を解放（所有権の責任）
- `/* C */` = `i + j` — s1の後にs2を連結するためのオフセット
- `/* D */` = `free(s1)` — 古いs1（stash）を解放（所有権移転の完了）

**解説**: `/* B */`と`/* D */`が同じ`free(s1)`であることに注目してください。
ft_strjoinはs1の所有権を受け取る契約なので、正常系でもエラー系でも
必ずs1をfreeする責任があります。

---

## まとめ

この理解度確認は以下のカテゴリで構成されています:

| セクション | 問題番号 | 内容 | 問題数 |
|-----------|---------|------|-------|
| 基礎知識 | Q1〜Q10 | fd, read(), static変数, メモリレイアウト | 10問 |
| 実装の理解 | Q11〜Q25 | 関数の動作、状態遷移、所有権パターン | 15問 |
| エッジケース | Q26〜Q35 | 空ファイル, 不正fd, 大きなBUFFER_SIZE | 10問 |
| 設計選択 | Q36〜Q45 | ファイル分割, 計算量, API設計 | 10問 |
| コードトレース | Q46〜Q55 | メモリ状態図, malloc/free追跡, バグ発見 | 10問 |
| バグ発見 | Q56〜Q60 | メモリリーク, off-by-one, 未定義動作 | 5問 |
| 評価防衛Q&A | Q61〜Q65 | 評価で聞かれる典型的な質問と模範回答 | 5問 |
| 穴埋めコード | Q66〜Q70 | 各関数のキーポイントの理解確認 | 5問 |
| **合計** | | | **70問** |

全問に自信を持って答えられるようになれば、get_next_lineの評価で
困ることはありません。特にPart 7（評価防衛Q&A）は
評価直前の最終確認に活用してください。

答えられなかった問題があれば、以下のWikiセクションを参照してください:

| 問題カテゴリ | 参照先 |
|------------|--------|
| 基礎知識 (Q1-Q10) | 01-background.md |
| 実装の理解 (Q11-Q25) | 05-solution.md |
| エッジケース (Q26-Q35) | 03-requirements.md |
| 設計の理解 (Q36-Q45) | 06-design.md |
| コードトレース (Q46-Q55) | 05-solution.md |
| バグ発見 (Q56-Q60) | 05-solution.md |
| 評価防衛 (Q61-Q65) | 04-evaluation.md |
| 穴埋めコード (Q66-Q70) | 05-solution.md |
