# 04 - 評価基準

この章では、get_next_lineの評価で何が確認されるか、
どのような質問が来るか、どのような失敗パターンがあるかを
徹底的に解説します。評価前の最終チェックとして活用してください。

42の評価は単なるテストではなく、「理解度の確認」です。
コードが動くだけでは不十分で、なぜそのコードが動くのか、
なぜその設計を選んだのかを説明できることが求められます。

---

## 1. 評価の流れ

### 1.1 評価者が確認する順序

```
Step 1: Normチェック
  └→ norminette で全ファイルがOKか確認
  └→ NGなら即座に0点

Step 2: コンパイル確認
  └→ 各BUFFER_SIZEでwarning/errorなしにコンパイルできるか
  └→ -D なしでもコンパイルできるか（デフォルト値）

Step 3: 機能テスト
  └→ 各種テストファイルで正しい出力が得られるか
  └→ エッジケースの処理が正しいか

Step 4: メモリチェック
  └→ valgrindでリークがないか確認
  └→ definitely lost が0であること

Step 5: 防衛質問
  └→ コードの理解度を確認する質問
  └→ 設計判断の理由を説明できるか
```

### 1.2 評価者が使うコマンド

```bash
# Norm チェック
norminette get_next_line.c get_next_line.h get_next_line_utils.c

# コンパイル（複数BUFFER_SIZE）
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 \
    get_next_line.c get_next_line_utils.c [test_main.c]

cc -Wall -Wextra -Werror -D BUFFER_SIZE=1 \
    get_next_line.c get_next_line_utils.c [test_main.c]

cc -Wall -Wextra -Werror -D BUFFER_SIZE=9999 \
    get_next_line.c get_next_line_utils.c [test_main.c]

cc -Wall -Wextra -Werror -D BUFFER_SIZE=10000000 \
    get_next_line.c get_next_line_utils.c [test_main.c]

# -Dなしのコンパイル
cc -Wall -Wextra -Werror \
    get_next_line.c get_next_line_utils.c [test_main.c]

# メモリチェック
valgrind --leak-check=full --show-leak-kinds=all ./gnl_test
```

### 1.3 評価の時間配分

```
一般的な評価の流れ（約30-45分）:

00:00 - 05:00  挨拶と概要説明
  → プロジェクトの概要を簡潔に説明
  → 使用した実装アプローチの概要

05:00 - 10:00  Normチェック
  → norminette を実行
  → 結果が全てOKであることを確認

10:00 - 20:00  機能テスト
  → 複数のテストファイルで動作確認
  → 複数のBUFFER_SIZEで確認
  → エッジケースのテスト

20:00 - 25:00  メモリチェック
  → valgrindで実行
  → リークがないことを確認

25:00 - 40:00  防衛質問
  → コードの詳細な質問
  → 設計判断の理由
  → エラー処理の確認

40:00 - 45:00  ボーナスの確認（提出した場合）
  → 複数fdの同時テスト
  → ボーナス固有の質問
```

---

## 2. 機能テストの詳細

### 2.1 テスト項目一覧

| # | テスト | 確認内容 | 合格基準 |
|---|-------|---------|---------|
| 1 | 通常ファイル | 複数行のファイルから正しく1行ずつ読み取れるか | 各行が\n付きで返される |
| 2 | \nの処理 | 返される行に\nが含まれるか | \nが行末に含まれる（最終行除く） |
| 3 | EOF処理 | 読み取り完了後にNULLを返すか | NULLが返される |
| 4 | 最終行 | \nで終わらないファイルの最終行 | \nなしの文字列が返される |
| 5 | 空ファイル | 空ファイルの処理 | 最初の呼び出しでNULL |
| 6 | stdin | 標準入力からの読み取り | 正常に動作する |
| 7 | 無効なfd | 負のfd、無効なfd | NULLが返される |
| 8 | 連続改行 | "\n\n\n"の処理 | 各"\n"が個別に返される |
| 9 | 長い行 | 10000文字以上の行 | 正しく全体が返される |
| 10 | 複数BUFFER_SIZE | 1, 42, 9999, 10000000 | 全てで正常動作 |

### 2.2 テスト用ファイルと期待される出力

```
=== test1.txt: "Hello World\n" ===

get_next_line(fd) -> "Hello World\n"
get_next_line(fd) -> NULL


=== test2.txt: "AAA\nBBB\nCCC" ===

get_next_line(fd) -> "AAA\n"
get_next_line(fd) -> "BBB\n"
get_next_line(fd) -> "CCC"    (最終行、\nなし)
get_next_line(fd) -> NULL


=== test3.txt: (空) ===

get_next_line(fd) -> NULL


=== test4.txt: "\n\n\n" ===

get_next_line(fd) -> "\n"
get_next_line(fd) -> "\n"
get_next_line(fd) -> "\n"
get_next_line(fd) -> NULL


=== test5.txt: "A" ===

get_next_line(fd) -> "A"
get_next_line(fd) -> NULL


=== fd = -1 ===

get_next_line(-1) -> NULL
```

### 2.3 BUFFER_SIZE別の動作確認ポイント

```
BUFFER_SIZE=1:
  ・1バイトずつ読み込む極端なケース
  ・ft_strjoinが行の長さ分呼ばれる
  ・read()のシステムコールが最多になる
  ・正しい出力が得られることを確認
  ・特に: 空文字列""との結合が正しく動作するか

BUFFER_SIZE=42:
  ・一般的なテストケース
  ・バッファの境界で\nが分断されるケースがある
  ・例: 50文字の行 → 42文字読み込み + 8文字読み込み

BUFFER_SIZE=9999:
  ・ほとんどの行が1回のreadで読み込まれる
  ・stashに複数行分のデータが蓄積される
  ・ft_trim_stash の正確さが重要

BUFFER_SIZE=10000000:
  ・約10MBのバッファ
  ・mallocが成功するか（メモリ十分な環境なら成功）
  ・1回のreadでファイル全体を読み込む可能性が高い
  ・正しく動作することを確認
```

### 2.4 エッジケースの重要度

```
優先度 高（評価で必ず確認される）:
  ・空ファイル → NULL
  ・BUFFER_SIZE=1 での正常動作
  ・fd=-1 → NULL（メモリリークなし）
  ・連続改行 "\n\n\n" → 3つの"\n"が返される
  ・最終行に\nがないファイル → \nなしの文字列

優先度 中（評価者によって確認される）:
  ・BUFFER_SIZE=10000000 での動作
  ・非常に長い行（100000文字）の読み取り
  ・close後のfdでの呼び出し
  ・BUFFER_SIZE=0 での動作（NULLを返すべき）

優先度 低（稀に確認される）:
  ・stdinからの読み取り
  ・バイナリデータを含むファイル
  ・BUFFER_SIZE=-1 での動作
```

---

## 3. メモリ管理チェック

### 3.1 valgrindでの確認

```bash
# デバッグ情報付きでコンパイル
cc -g -Wall -Wextra -Werror -D BUFFER_SIZE=42 \
    get_next_line.c get_next_line_utils.c main.c -o gnl

# valgrind実行
valgrind --leak-check=full --show-leak-kinds=all \
    --track-origins=yes ./gnl
```

### 3.2 期待されるvalgrind出力

```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks
==12345==   total heap usage: X allocs, X frees, Y bytes allocated
                              ^              ^
                              同じ数であること!

==12345== All heap blocks were freed -- no leaks are possible
==12345== ERROR SUMMARY: 0 errors from 0 contexts
```

### 3.3 "still reachable" の扱い

```
==12345== LEAK SUMMARY:
==12345==    definitely lost: 0 bytes in 0 blocks   <- 必ず0であること
==12345==    indirectly lost: 0 bytes in 0 blocks   <- 必ず0であること
==12345==      possibly lost: 0 bytes in 0 blocks   <- 必ず0であること
==12345==    still reachable: 42 bytes in 1 blocks  <- static変数の場合あり
```

**still reachable** はstatic変数`stash`に残るメモリです。

- 最後の`get_next_line`呼び出しでNULLが返された後、
  stashがNULLに設定されていれば、still reachableは0になります。
- 実装が正しければ、EOF時に`free(stash); stash = NULL;`が
  実行されるため、still reachableは0になるはずです。
- still reachableが残る場合: get_next_lineをEOF/NULLまで
  呼び切らなかった場合（例: 途中で読み取りをやめた場合）

### 3.4 テスト用main.c（リークチェック完全版）

```c
#include "get_next_line.h"
#include <fcntl.h>
#include <stdio.h>

int	main(void)
{
	int		fd;
	char	*line;

	/* テスト1: 通常ファイル */
	fd = open("test.txt", O_RDONLY);
	if (fd == -1)
		return (1);
	while ((line = get_next_line(fd)) != NULL)
	{
		printf("%s", line);
		free(line);            /* 各行を必ずfree */
	}
	close(fd);

	/* テスト2: 無効なfd */
	line = get_next_line(-1);  /* NULLが返されるはず */
	if (line != NULL)
		free(line);

	/* テスト3: 空ファイル */
	fd = open("empty.txt", O_RDONLY);
	if (fd != -1)
	{
		line = get_next_line(fd);
		if (line != NULL)
			free(line);
		close(fd);
	}
	return (0);
}
```

### 3.5 各BUFFER_SIZEでのvalgrindチェック

```bash
# 全てのBUFFER_SIZEでvalgrindを通すスクリプト
for bs in 1 42 100 9999 10000000; do
    echo "============================================"
    echo "BUFFER_SIZE=$bs"
    echo "============================================"
    cc -g -Wall -Wextra -Werror -D BUFFER_SIZE=$bs \
        get_next_line.c get_next_line_utils.c main.c -o gnl
    valgrind --leak-check=full --show-leak-kinds=all \
        --error-exitcode=1 ./gnl
    if [ $? -eq 0 ]; then
        echo "PASS"
    else
        echo "FAIL - Memory issues detected!"
    fi
    echo ""
done
```

### 3.6 メモリリークが発生しやすい箇所ランキング

```
1位: ft_strjoin で s1 を free し忘れ
  → 最も頻出するバグ
  → stash = ft_strjoin(stash, buf) で古い stash がリーク
  → valgrind: "definitely lost" in ft_strjoin

2位: read() エラー時に stash を free し忘れ
  → bytes_read == -1 のパスで stash を解放していない
  → 特に: buf は free するが stash を忘れるパターン

3位: ft_trim_stash で stash を free し忘れ
  → 新しい trimmed を作った後、古い stash を解放していない
  → valgrind: "definitely lost" in ft_trim_stash

4位: 呼び出し元で line を free し忘れ
  → get_next_line 側の問題ではないが、テスト時に注意
  → main.c で free(line) を忘れている

5位: ft_strjoin の malloc 失敗時に s1 を free し忘れ
  → joined = malloc(...) が NULL の場合
  → free(s1) をしないと、エラーパスでリークが発生
```

---

## 4. よくある失敗ポイント（詳細分析）

### 4.1 失敗パターン1: BUFFER_SIZE=1での動作不良

**症状**: BUFFER_SIZE=1で正しい行が返されない、または無限ループ

**原因分析**:

```
BUFFER_SIZE=1, ファイル: "AB\n"

正しい動作:
  read -> 'A'    buf = "A\0"   stash = "" + "A" = "A"     \nなし -> 続行
  read -> 'B'    buf = "B\0"   stash = "A" + "B" = "AB"   \nなし -> 続行
  read -> '\n'   buf = "\n\0"  stash = "AB" + "\n" = "AB\n" \nあり -> 停止

  ft_get_line("AB\n") -> "AB\n"
  ft_trim_stash("AB\n") -> \nの後が'\0' -> free(stash), return NULL

よくある間違い:
  ・ft_strjoinが空文字列""を正しく扱えない
  ・read()の戻り値が1（0ではない）のにEOFと判定してしまう
  ・buf[bytes_read]の'\0'付与を忘れている
```

**デバッグ方法**:

```c
/* BUFFER_SIZE=1 でのデバッグ出力例 */
fprintf(stderr, "read returned: %d\n", bytes_read);
fprintf(stderr, "buf = [%s] (hex: ", buf);
for (int k = 0; k <= bytes_read; k++)
    fprintf(stderr, "%02x ", (unsigned char)buf[k]);
fprintf(stderr, ")\n");
fprintf(stderr, "stash after join = [%s]\n",
    stash ? stash : "NULL");
```

### 4.2 失敗パターン2: メモリリーク（ft_strjoinでの古いポインタ）

**症状**: valgrindで "definitely lost" が報告される

```
==12345== 42 bytes in 1 blocks are definitely lost
==12345==    at 0x...: malloc (...)
==12345==    by 0x...: ft_strjoin (get_next_line_utils.c:50)
==12345==    by 0x...: ft_read_to_stash (get_next_line.c:30)
```

**原因と修正**:

```c
/* 間違い: ft_strjoin が s1 を free しない */
char	*ft_strjoin(char *s1, char *s2)
{
	char	*joined;

	joined = malloc(ft_strlen(s1) + ft_strlen(s2) + 1);
	/* ... コピー ... */
	return (joined);   /* s1 がリーク! */
}

/* 呼び出し元: */
stash = ft_strjoin(stash, buf);
/* stash の古いアドレスが失われ、free できなくなる */
```

```c
/* 正しい: ft_strjoin が s1 を free する */
char	*ft_strjoin(char *s1, char *s2)
{
	char	*joined;

	joined = malloc(ft_strlen(s1) + ft_strlen(s2) + 1);
	/* ... コピー ... */
	free(s1);           /* 古い s1(stash) を解放 */
	return (joined);
}
```

### 4.3 失敗パターン3: read()エラー時のメモリ解放不足

**症状**: 無効なfdを渡すとメモリリーク

```c
/* 間違い: stashを解放していない */
if (bytes_read == -1)
{
	free(buf);
	return (NULL);    /* stash がリーク! */
}

/* 正しい: buf と stash 両方を解放 */
if (bytes_read == -1)
{
	free(buf);
	free(stash);      /* stash も忘れずに解放 */
	return (NULL);
}
```

**valgrind出力例**:

```
==12345== 10 bytes in 1 blocks are definitely lost
==12345==    at 0x...: malloc (...)
==12345==    by 0x...: ft_init_stash (get_next_line.c:42)
                       <- stash の初期化時に確保されたメモリ
==12345==    by 0x...: get_next_line (get_next_line.c:55)
```

### 4.4 失敗パターン4: 連続\nの処理

**症状**: "\n\n\n" で3つの "\n" が返されない

```
正しい動作:

stash = "\n\n\n"

ft_get_line("\n\n\n"):
  i = 0, stash[0] = '\n' -> ループ即座に終了
  stash[i] == '\n' -> i++ -> i = 1
  line = malloc(2)  -> line = "\n\0"

ft_trim_stash("\n\n\n"):
  i = 0, stash[0] = '\n' -> ループ即座に終了
  i++ -> i = 1
  trimmed = malloc(2) -> trimmed = "\n\n\0"
  -> 残り2つの\nがtrimmedに

2回目: stash = "\n\n"
  -> 同様に "\n" を返し、stash = "\n"

3回目: stash = "\n"
  -> "\n" を返し、stash = NULL

4回目: stash = NULL -> ... -> return NULL
```

**よくある間違い**:

```
間違い1: ft_get_line で \n を含めずに返す
  → "" (空文字列) が返される
  → 原因: while (stash[i] != '\n') の後に i++ を忘れている

間違い2: ft_trim_stash で \n をスキップしない
  → 無限ループになる（常に \n で始まる stash が残る）
  → 原因: i++ を忘れている

間違い3: 空文字列 "" が返される
  → 原因: stash[0] == '\n' の場合の処理が不正
  → ft_get_line で \n 自体を行の一部として含めていない
```

### 4.5 失敗パターン5: BUFFER_SIZE > ファイルサイズの場合

**症状**: 1回のreadで全ファイルを読み込む場合に問題発生

```
BUFFER_SIZE=10000000, ファイル="AB\nCD\n" (6バイト)

read(fd, buf, 10000000) -> 6バイト (残りの9999994バイトは読めない)
buf = "AB\nCD\n\0"

stash = "" + "AB\nCD\n" = "AB\nCD\n"
\nあり -> ループ終了

ft_get_line("AB\nCD\n") -> "AB\n"
ft_trim_stash("AB\nCD\n") -> "CD\n"

2回目: stash = "CD\n" -> \nあり -> readループに入らない!
-> "CD\n" を返す

3回目: stash = "" -> read -> 0 (EOF) -> return NULL

問題が起きるケース:
  ・buf[bytes_read] = '\0' を忘れるとゴミデータが混入
  ・bytes_read が BUFFER_SIZE より小さいことを考慮していない
```

### 4.6 失敗パターン6: ft_strjoinのmalloc失敗時の処理

```c
char	*ft_strjoin(char *s1, char *s2)
{
	char	*joined;

	if (!s1 || !s2)
		return (NULL);
	joined = malloc(ft_strlen(s1) + ft_strlen(s2) + 1);
	if (!joined)
	{
		free(s1);      /* malloc失敗でもs1を解放する! */
		return (NULL); /* そうしないとs1がリークする */
	}
	/* ... コピー ... */
	free(s1);
	return (joined);
}
```

### 4.7 失敗パターン7: EOF後の再呼び出し

```
症状: EOFに達した後、再度get_next_lineを呼ぶとクラッシュ

正しい動作:
  get_next_line(fd) -> "last line"
  get_next_line(fd) -> NULL  (EOF)
  get_next_line(fd) -> NULL  (再度呼んでもNULL)
  get_next_line(fd) -> NULL  (何度呼んでもNULL)

  stash の状態:
    EOF時: free(stash) -> stash = NULL
    再呼び出し: stash == NULL -> ft_init_stash -> stash = ""
    -> read -> 0 (EOF) -> stash[0] == '\0'
    -> free(stash) -> stash = NULL -> return NULL

  → 安全に動作する
```

### 4.8 失敗パターン8: ボーナスでのfd範囲外アクセス

```
症状: fd >= MAX_FD の場合に配列の範囲外アクセスが発生

fd = 2048 の場合 (MAX_FD = 1024):
  stash[2048] -> 配列の範囲外! -> 未定義動作

対策:
  if (fd < 0 || fd >= MAX_FD || BUFFER_SIZE <= 0)
      return (NULL);
```

---

## 5. 防衛質問への準備

### 5.1 基礎的な質問

| 質問 | 答えるべきポイント |
|------|-----------------|
| static変数の役割は? | 関数呼び出し間でstashを保持する。ローカル変数は関数終了時に消滅するため使えない。 |
| static変数はメモリのどこにある? | BSSセグメント（初期化されていない場合）。スタックではなくデータセグメント。 |
| なぜft_strjoinでs1をfreeするのか? | `stash = ft_strjoin(stash, buf)`パターンで古いstashをリークさせないため。 |
| BUFFER_SIZE=1で効率が悪い理由は? | read()システムコールが1バイトごとに発生し、コンテキストスイッチのオーバーヘッドが大きい。 |
| read()の戻り値の意味は? | 正の値:読み取ったバイト数、0:EOF、-1:エラー |
| メモリリークとは? | mallocで確保したメモリをfreeせずにポインタを失うこと。valgrindで検出可能。 |

### 5.2 実装の詳細に関する質問

| 質問 | 答えるべきポイント |
|------|-----------------|
| ft_init_stashはなぜ必要? | stashがNULLの場合にft_strjoinがクラッシュしないよう、空文字列で初期化する。 |
| ft_trim_stashが内部でstashをfreeする理由は? | 古いstashは不要になるため。trim後の残りデータは新しいmallocで確保。 |
| グローバル変数を使わない理由は? | Norm違反。また、副作用が見えにくくバグの温床になる。static変数はスコープが限定されるためより安全。 |
| なぜ.cと_utils.cに分けるのか? | Normの「1ファイル最大5関数」制約。また状態管理とデータ操作の関心分離。 |
| lseek()を使わない理由は? | プロジェクト要件で禁止。また「可能な限り少なく読む」方針にlseekは不要。 |
| bytes_read = 1 の初期化の意味は? | whileループに最初に入るための条件を満たすため。実際の読み取り量ではない。 |

### 5.3 質問への回答例（模範解答）

**Q: static変数について説明してください**

```
回答例:
「static変数は、関数のローカルスコープを持ちながら、
 プログラムの実行期間中ずっとメモリ上に存在する変数です。

 通常のローカル変数はスタック上に確保され、
 関数が終了すると破棄されます。しかしget_next_lineでは
 前回読み込んだ残りのデータを次回の呼び出しで使う必要が
 あるため、関数終了後もデータを保持する仕組みが必要です。

 static変数はBSSセグメントに配置され、プログラム開始時に
 0（NULLポインタ）に初期化されます。これにより、
 初回呼び出し時にstashがNULLであることを検出して
 初期化処理を行うことができます。」
```

**Q: なぜft_strjoinでs1をfreeするのですか?**

```
回答例:
「stash = ft_strjoin(stash, buf) というパターンで使用するためです。

 この代入文では、ft_strjoinの戻り値がstashに代入されます。
 もしft_strjoin内でs1（古いstash）をfreeしなければ、
 古いstashのアドレスが失われてメモリリークが発生します。

 s1をfreeするこの設計は、所有権の移転パターンと呼ばれます。
 古いstashの所有権がft_strjoinに移り、ft_strjoinが
 解放してから新しいjoinedの所有権を呼び出し元に返します。

 代替として、呼び出し側でtmp変数に保存してfreeする方法も
 ありますが、free忘れのリスクが高くなるため、
 ft_strjoin内でfreeする方が安全です。」
```

**Q: BUFFER_SIZE=1のときの計算量は?**

```
回答例:
「BUFFER_SIZE=1の場合、n文字の行を読むのにO(n^2)の
 計算量がかかります。

 これは、ft_strjoinが毎回呼ばれ、各呼び出しで
 stash全体をコピーするためです。
 1回目は0+1=1文字、2回目は1+1=2文字、...、
 n回目はn-1+1=n文字のコピーが発生します。
 合計は1+2+...+n = n(n+1)/2 = O(n^2)です。

 ただし、実用的なBUFFER_SIZE（例えば42や4096）では
 strjoinの呼び出し回数が大幅に減るため、
 事実上O(n)の計算量になります。
 BUFFER_SIZE >= 行の長さ であれば、1行あたり
 最大2回のstrjoinで済みます。」
```

### 5.4 ボーナスに関する質問

| 質問 | 答えるべきポイント |
|------|-----------------|
| ボーナスでstatic配列を使う理由は? | fdごとに独立したstashを管理するため。stash[fd]でfd番号をインデックスに使う。 |
| MAX_FDの値の根拠は? | システムのOPEN_MAX（通常1024）に基づく。ulimit -nで確認可能。 |
| fdがMAX_FD以上だとどうなる? | fd >= MAX_FDのチェックでNULLを返す。配列の範囲外アクセスを防止。 |
| static char *stash[1024]のメモリ使用量は? | 1024 * 8 = 8192バイト（64ビットシステム）。BSSセグメントにNULL初期化。 |
| 複数fdを同時に使えるメリットは? | ファイルを交互に読み取れる。例: 設定ファイルとデータファイルを同時処理。 |

### 5.5 パフォーマンスに関する質問

| 質問 | 答えるべきポイント |
|------|-----------------|
| BUFFER_SIZEが大きいとなぜ速い? | read()の呼び出し回数が減り、コンテキストスイッチのコストが削減される。 |
| strjoinの計算量は? | O(n)（nは結合後の文字列長）。ただしループ内で呼ぶとO(n^2)になりうる。 |
| 最適なBUFFER_SIZEは? | 4096（ページサイズと一致）が理論的に効率的。大きすぎるとメモリ浪費。 |
| なぜ4096が効率的なのか? | ディスクI/Oとメモリのページサイズ（通常4KB）に一致し、カーネルが効率的に処理できる。 |

### 5.6 メモリ管理に関する質問

| 質問 | 答えるべきポイント |
|------|-----------------|
| "definitely lost"と"still reachable"の違いは? | definitely lost:ポインタが完全に失われたリーク。still reachable:終了時にまだポインタが存在するメモリ。 |
| ダングリングポインタとは? | free後のメモリを指すポインタ。アクセスすると未定義動作。対策:free後にNULL代入。 |
| ダブルフリーとは? | 同じポインタを2回freeすること。クラッシュやメモリ破壊の原因。 |
| use-after-freeとは? | free後のメモリにアクセスすること。データ破壊やセキュリティ脆弱性の原因。 |
| ft_strjoinでの所有権移転を説明してください | ft_strjoinはs1をfreeして新しいjoinedを返す。s1の所有権がft_strjoinに移り、joinedの所有権が呼び出し元に移転する。 |

### 5.7 応用的な質問

| 質問 | 答えるべきポイント |
|------|-----------------|
| get_next_lineはスレッドセーフか? | static変数を使用しているため、スレッドセーフではない。複数スレッドから同時に呼ぶとデータ競合が発生する。 |
| fgets()との違いは? | fgets()はstdioのバッファリングを使うライブラリ関数。get_next_lineは自前でバッファリングを実装。 |
| このプロジェクトの知識がpipexにどう活きるか? | fd管理、read/write、エラーハンドリングの経験が直接活用される。 |
| なぜread()は\nで止まらないのか? | read()はバイトストリームとして読み取る低レベル関数。行の概念は持たない。行の検出はアプリケーション側の責任。 |
| EOFは文字か? | EOFは文字ではなく、「もう読むデータがない」という状態を示す。read()が0を返すことで表現される。ファイル末尾にEOF文字は存在しない。 |

---

## 6. 自動テスターの対策

### 6.1 よく使われるテスター

テスターを使う場合、以下の点に注意してください。

- テスターの結果を鵜呑みにしない（テスター自体にバグがある場合もある）
- テスターの全テストに合格しても、手動テストを怠らない
- テスターがカバーしていないエッジケースもある

### 6.2 テスターが見つけやすいバグ

1. **BUFFER_SIZE=1での動作不良**: ほぼ全てのテスターがチェック
2. **メモリリーク**: valgrindと組み合わせてチェック
3. **最終行の\n処理**: \nありなし両方をチェック
4. **空ファイル**: NULLが返されるかチェック
5. **連続改行**: 各\nが個別に返されるかチェック

### 6.3 テスターが見つけにくいバグ

1. **read()エラー時のメモリリーク**: 無効なfdでのリーク
2. **malloc失敗時の処理**: 通常の環境ではmallocが失敗しにくい
3. **stdinからの読み取り**: 自動テストが困難
4. **BUFFER_SIZE=0**: テスターによってはチェックしない
5. **EOF後の再呼び出し**: テスターによってはスキップ

### 6.4 テスターの結果の解釈

```
テスターの出力例:

[OK]  test_basic_read
[OK]  test_multiple_lines
[KO]  test_empty_file           <- ここで失敗
[OK]  test_long_line
[KO]  test_buffer_size_1        <- ここで失敗

解析:
  ・test_empty_file 失敗
    → 空ファイルで NULL を返していない可能性
    → stash = "" の場合の処理を確認
    → stash[0] == '\0' のチェックを確認

  ・test_buffer_size_1 失敗
    → BUFFER_SIZE=1 での動作を確認
    → ft_strjoin("", "A") が正しく動作するか確認
    → buf[bytes_read] = '\0' を確認
```

---

## 7. 評価当日のチェックリスト

### 7.1 提出前の最終確認

```
コンパイル確認:
  [ ] cc -Wall -Wextra -Werror -D BUFFER_SIZE=1
  [ ] cc -Wall -Wextra -Werror -D BUFFER_SIZE=42
  [ ] cc -Wall -Wextra -Werror -D BUFFER_SIZE=9999
  [ ] cc -Wall -Wextra -Werror -D BUFFER_SIZE=10000000
  [ ] cc -Wall -Wextra -Werror (BUFFER_SIZE未指定)
  -> 全てwarning/errorなし

Norm確認:
  [ ] norminette 全ファイルでOK

機能確認:
  [ ] 通常の複数行ファイル -> 正しい出力
  [ ] 空ファイル -> NULL
  [ ] 改行のみのファイル -> "\n" -> NULL
  [ ] 連続改行 -> 各"\n"が個別に返される
  [ ] 最終行が\nで終わらない -> \nなしの文字列
  [ ] fd=-1 -> NULL
  [ ] 各BUFFER_SIZEで上記全て動作

メモリ確認:
  [ ] valgrind --leak-check=full -> リークなし
  [ ] BUFFER_SIZE=1 でもリークなし
  [ ] BUFFER_SIZE=10000000 でもリークなし
  [ ] 無効なfd でもリークなし

ボーナス（提出する場合）:
  [ ] 複数fdの交互読み取り -> 正常動作
  [ ] ボーナスファイルのNormチェック
  [ ] ボーナスファイルのvalgrindチェック
```

### 7.2 説明できるようにしておくこと

1. **コードの全行を説明できること**: 各行が何をしているか
2. **設計判断の理由**: なぜこの方法を選んだか
3. **エラー時の動作**: 各エラーケースでメモリがどう処理されるか
4. **BUFFER_SIZE=1の動作**: 1バイトずつの処理を追跡できること
5. **メモリの所有権**: 誰がどのメモリをfreeするか

### 7.3 評価当日の心構え

```
心構え1: 落ち着いて説明する
  → 急がずに、自分のペースで説明する
  → わからない場合は「わかりません」と正直に言う
  → 推測で答えるより、わからないと言う方が印象が良い

心構え2: コードを読みながら説明する
  → 口頭だけでなく、コードを指しながら説明する
  → 「ここの行で...」と具体的に示す

心構え3: 図を描いて説明する
  → メモリの状態を図で描くと理解が伝わりやすい
  → stashの変遷を矢印図で説明する

心構え4: 質問の意図を理解する
  → 評価者は「理解しているか」を確認したい
  → 暗記ではなく、理解に基づいた回答が求められる
```

---

## 8. やってみよう: 模擬評価

以下の質問に答えてみましょう。答えられない質問があれば、
該当するWikiのセクションを読み直してください。

### 8.1 基礎質問（全問即答できるべき）

1. ファイルディスクリプタとは何か、一言で説明してください。
2. read()の戻り値が0と-1の違いは何ですか?
3. static変数の値はいつ初期化されますか?
4. メモリリークとは何ですか?
5. BUFFER_SIZEが1の場合と1000の場合、何が違いますか?

### 8.2 実装質問（コードを見ながら答えられるべき）

6. ft_strjoinでs1をfreeする理由を、メモリ図で説明してください。
7. ft_init_stashがNULLの場合に空文字列を作る理由は?
8. ft_read_to_stashでread()が-1を返した場合、何をfreeしますか?
9. stash[0] == '\0'のチェックは何のためですか?
10. ft_trim_stashが\nなしの場合にNULLを返す理由は?

### 8.3 応用質問（深い理解が必要）

11. get_next_lineの行読み取りの計算量がO(n^2)になりうる条件は?
12. stdinからの読み取りと通常ファイルの読み取りの違いは?
13. ボーナスでstatic配列を使う代わりに、リンクリストを使う方法は?
14. このプロジェクトの知識がminishellのどの部分に活きますか?
15. valgrindの"still reachable"が発生する条件は?

### 8.4 模範解答

```
Q1: ファイルディスクリプタ
A: OSが管理する、開かれたファイルへの整数識別子です。
   プロセスごとにテーブルが管理され、0=stdin, 1=stdout,
   2=stderr が予約されています。

Q2: read()の戻り値 0 vs -1
A: 0はEOF（ファイル末尾に到達）を意味し、
   -1はエラー（不正なfd、アクセス権限なし等）を意味します。

Q3: static変数の初期化タイミング
A: プログラム開始時に0（ポインタの場合はNULL）に初期化されます。
   BSSセグメントに配置されるため、明示的な初期化がなくても
   0初期化が保証されています。

Q4: メモリリーク
A: mallocで確保したメモリをfreeせずに、そのアドレスを
   保持するポインタを失うことです。失われたメモリは
   プログラム終了まで解放されません。

Q5: BUFFER_SIZE=1 vs 1000
A: BUFFER_SIZE=1では1バイトずつread()を呼ぶため、
   システムコールの回数が1000倍になります。
   また、ft_strjoinも1000倍呼ばれ、計算量がO(n^2)になります。
   BUFFER_SIZE=1000では多くの場合1回のreadで1行全体を読め、
   計算量はO(n)に近くなります。
```

---

## 9. 評価後の振り返り

### 9.1 よくある評価後のフィードバック

```
フィードバック1: 「メモリリークがあった」
  → valgrindで全てのBUFFER_SIZEをテストしたか確認
  → 特にエラーパス（read=-1, malloc失敗）でリークがないか

フィードバック2: 「BUFFER_SIZE=1で動かなかった」
  → 空文字列""との結合が正しいか確認
  → buf[bytes_read] = '\0' を確認
  → ft_strjoinのNULLチェックを確認

フィードバック3: 「設計の説明が不十分だった」
  → 06-design.md を再読
  → 各設計判断の「なぜ」を自分の言葉で説明する練習

フィードバック4: 「エッジケースの処理が不完全」
  → 03-requirements.md のエッジケース表を確認
  → 全てのケースでテストを実施
```

### 9.2 次のプロジェクトへの活かし方

```
get_next_line で学んだことを次のプロジェクトに活かす:

1. メモリ管理の習慣
   → malloc後は必ずNULLチェック
   → free後はNULL代入
   → valgrindは最初から使う

2. テストの習慣
   → エッジケースを先にリストアップ
   → 複数の条件でテスト
   → 自動テストスクリプトを作る

3. 設計の習慣
   → 実装前にアルゴリズムを紙に書く
   → 関数の責務を明確にする
   → 「なぜ」を説明できる設計を心がける

4. デバッグの習慣
   → 問題が起きたらまずvalgrindを実行
   → printfデバッグは体系的に
   → gdbの基本操作を覚える
```
