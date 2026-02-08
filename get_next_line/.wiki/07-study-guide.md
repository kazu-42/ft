# 07 - 学習ガイドとデバッグ手法

この章では、get_next_lineの推奨学習順序、
デバッグ技法（gdb、valgrind、printfデバッグ）の詳細、
実践的な演習問題を提供します。

実装に行き詰まったとき、最も重要なのは「体系的なデバッグ」です。
闇雲にコードを変更するのではなく、仮説を立て、検証し、
原因を特定するという科学的なアプローチが必要です。

---

## 1. 推奨学習順序

### Phase 1: 基礎理解（1-2日）

```
Day 1:
  午前: ファイルディスクリプタとread()の理解
    ・man 2 read を読む
    ・簡単なプログラムでファイルを開いてread()する練習
    ・read()の3つの戻り値を確認

  午後: static変数とメモリ管理の実験
    ・static変数のカウンタープログラムを書く
    ・malloc/freeの基本パターンを練習
    ・valgrindを初めて使ってみる

Day 2:
  午前: get_next_lineのアルゴリズム理解
    ・05-solution.md を読む
    ・紙にアルゴリズムの流れを描く
    ・BUFFER_SIZE=1での動作を手で追跡する

  午後: 03-requirements.md でエッジケースを確認
    ・全てのエッジケースをリストアップ
    ・各ケースでの期待動作を理解
```

### Phase 2: ヘルパー関数の実装（1日）

```
Day 3:
  1. ft_strlen - NULLセーフ版
     テスト: ft_strlen(NULL), ft_strlen(""), ft_strlen("abc")

  2. ft_strchr - NULLセーフ版
     テスト: ft_strchr(NULL, 'a'), ft_strchr("hello", 'l'),
             ft_strchr("hello", '\0'), ft_strchr("hello", 'z')

  3. ft_strjoin - s1をfreeするカスタム版
     ★ 最重要関数! 慎重にテスト
     テスト: ft_strjoin("", "a"), ft_strjoin("abc", "def"),
             malloc失敗時の動作

  4. 各関数を個別にテストするmain.cを書く
```

### Phase 2.5: ヘルパー関数の詳細テスト

```
ft_strlen のテスト:
  assert(ft_strlen("Hello") == 5);
  assert(ft_strlen("") == 0);
  assert(ft_strlen(NULL) == 0);
  assert(ft_strlen("\n") == 1);
  assert(ft_strlen("AB\nCD") == 5);

ft_strchr のテスト:
  assert(ft_strchr("hello", 'h') == &"hello"[0]);
  assert(ft_strchr("hello", 'l') == &"hello"[2]);
  assert(ft_strchr("hello", 'z') == NULL);
  assert(ft_strchr("hello", '\0') == &"hello"[5]);
  assert(ft_strchr(NULL, 'a') == NULL);
  assert(ft_strchr("\n", '\n') == &"\n"[0]);

ft_strjoin のテスト（特に重要）:
  ※ s1がfreeされるので、malloc済みの文字列を渡す

  char *s1 = strdup("");
  char *result = ft_strjoin(s1, "Hello");
  assert(strcmp(result, "Hello") == 0);
  free(result);

  s1 = strdup("Hello");
  result = ft_strjoin(s1, " World");
  assert(strcmp(result, "Hello World") == 0);
  free(result);

  s1 = strdup("AB");
  result = ft_strjoin(s1, "\n");
  assert(strcmp(result, "AB\n") == 0);
  free(result);
```

### Phase 3: コア実装（2-3日）

```
Day 4-5:
  1. get_next_line()の骨格を書く
  2. ft_init_stash()を実装
  3. ft_read_to_stash()を実装
  4. まず1行だけ読めるバージョンを作る
  5. 複数行に対応する
  6. ft_get_line()とft_trim_stash()を実装

Day 6:
  7. エッジケースの処理を追加
  8. BUFFER_SIZE=1, 42, 10000000 でテスト
  9. valgrindでメモリリークをチェック
```

### Phase 3.5: 段階的な実装アプローチ

```
Step 1: 最小限の動作を確認
  → 1行だけのファイルを読める状態にする
  → "Hello\n" を正しく返すことを確認

Step 2: 複数行に対応
  → "A\nB\nC\n" で3回呼んで3行返すことを確認
  → stashの残りデータが正しく管理されることを確認

Step 3: 最終行の処理
  → "A\nB" で最終行が\nなしで返されることを確認
  → EOF後にNULLが返されることを確認

Step 4: エッジケースの処理
  → 空ファイル -> NULL
  → fd = -1 -> NULL
  → "\n\n\n" -> 3つの"\n"

Step 5: 各BUFFER_SIZEでテスト
  → BUFFER_SIZE=1, 42, 9999, 10000000
  → 全てで同じ結果が得られることを確認

Step 6: メモリリークの修正
  → valgrindで全テストケースを確認
  → definitely lost = 0
```

### Phase 4: テストとデバッグ（1-2日）

```
Day 7:
  1. 全テストファイルを作成
  2. 各BUFFER_SIZEで全テストを実行
  3. valgrindを全テストケースで実行
  4. norminetteでチェック
  5. 友人とコードレビュー
```

### Phase 5: ボーナス（1日）

```
Day 8:
  1. static char *stash を static char *stash[MAX_FD] に変更
  2. stash -> stash[fd] に全て書き換え
  3. fd >= MAX_FD のチェックを追加
  4. 複数fdの同時読み取りテスト
```

---

## 2. デバッグ手法の詳細

### 2.1 printfデバッグ

開発中に最も手軽なデバッグ方法です。

```c
/* デバッグ用マクロ（提出前に削除すること!） */
#define DEBUG 1

#if DEBUG
# define DBG(fmt, ...) fprintf(stderr, \
    "[DEBUG %s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
# define DBG(fmt, ...)
#endif
```

```c
/* 使用例 */
char	*get_next_line(int fd)
{
	static char	*stash;
	char		*line;

	DBG("=== get_next_line(fd=%d) called ===", fd);
	DBG("stash = [%s]", stash ? stash : "NULL");

	/* ... 処理 ... */

	stash = ft_read_to_stash(fd, stash);
	DBG("after read_to_stash: stash = [%s]",
		stash ? stash : "NULL");

	if (!stash)
	{
		DBG("stash is NULL, returning NULL");
		return (NULL);
	}

	line = ft_get_line(stash);
	DBG("line = [%s]", line ? line : "NULL");

	stash = ft_trim_stash(stash);
	DBG("after trim: stash = [%s]", stash ? stash : "NULL");

	return (line);
}
```

**printfデバッグの出力例**:

```
[DEBUG get_next_line.c:52] === get_next_line(fd=3) called ===
[DEBUG get_next_line.c:53] stash = [NULL]
[DEBUG get_next_line.c:58] after read_to_stash: stash = [Hello\nWorld]
[DEBUG get_next_line.c:67] line = [Hello\n]
[DEBUG get_next_line.c:70] after trim: stash = [World]
```

**注意事項**:
- 必ず`stderr`に出力する（stdoutはget_next_lineの結果と混ざる）
- 提出前に全てのデバッグ出力を削除すること
- NULLポインタをprintfに渡すとクラッシュするので、必ず三項演算子でチェック

### 2.2 printfデバッグの応用: 関数ごとのログ

```c
/* ft_read_to_stash 用のデバッグ */
static char	*ft_read_to_stash(int fd, char *stash)
{
	char	*buf;
	int		bytes_read;
	int		loop_count;

	loop_count = 0;
	DBG("ft_read_to_stash: fd=%d, stash=[%s]",
		fd, stash ? stash : "NULL");
	buf = malloc(sizeof(char) * (BUFFER_SIZE + 1));
	if (!buf)
	{
		DBG("buf malloc failed!");
		return (NULL);
	}
	bytes_read = 1;
	while (!ft_strchr(stash, '\n') && bytes_read > 0)
	{
		bytes_read = read(fd, buf, BUFFER_SIZE);
		DBG("  loop %d: read returned %d", loop_count, bytes_read);
		if (bytes_read == -1)
		{
			DBG("  read error! freeing buf and stash");
			free(buf);
			free(stash);
			return (NULL);
		}
		buf[bytes_read] = '\0';
		DBG("  buf = [%s]", buf);
		stash = ft_strjoin(stash, buf);
		DBG("  stash after join = [%s]",
			stash ? stash : "NULL");
		loop_count++;
	}
	free(buf);
	DBG("ft_read_to_stash: returning stash=[%s]",
		stash ? stash : "NULL");
	return (stash);
}
```

### 2.3 printfデバッグの出力例: BUFFER_SIZE=3, ファイル="ABCD\n"

```
[DEBUG gnl.c:20] ft_read_to_stash: fd=3, stash=[]
[DEBUG gnl.c:32]   loop 0: read returned 3
[DEBUG gnl.c:40]   buf = [ABC]
[DEBUG gnl.c:42]   stash after join = [ABC]
[DEBUG gnl.c:32]   loop 1: read returned 2
[DEBUG gnl.c:40]   buf = [D\n]
[DEBUG gnl.c:42]   stash after join = [ABCD\n]
[DEBUG gnl.c:46] ft_read_to_stash: returning stash=[ABCD\n]
[DEBUG gnl.c:67] line = [ABCD\n]
[DEBUG gnl.c:70] after trim: stash = [NULL]
```

### 2.4 gdbによるデバッグ

gdbはGNU Debuggerで、プログラムを1行ずつ実行し、
変数の値やメモリの内容を確認できます。

```bash
# デバッグ情報付きでコンパイル
cc -g -Wall -Wextra -Werror -D BUFFER_SIZE=5 \
    get_next_line.c get_next_line_utils.c main.c -o gnl

# gdbで起動
gdb ./gnl
```

**gdbの基本コマンド**:

| コマンド | 短縮形 | 説明 |
|---------|-------|------|
| `break main` | `b main` | main関数にブレークポイント設置 |
| `break get_next_line` | `b get_next_line` | 関数にブレークポイント |
| `break get_next_line.c:24` | `b gnl.c:24` | 特定の行にブレークポイント |
| `run` | `r` | プログラム実行開始 |
| `next` | `n` | 次の行を実行（関数内に入らない） |
| `step` | `s` | 次の行を実行（関数内に入る） |
| `continue` | `c` | 次のブレークポイントまで実行 |
| `print stash` | `p stash` | 変数stashの値を表示 |
| `print *stash` | `p *stash` | stashが指す先の値を表示 |
| `print (char*)stash` | | stashを文字列として表示 |
| `x/10c stash` | | stashから10バイトを文字として表示 |
| `x/10x stash` | | stashから10バイトを16進数で表示 |
| `info locals` | | ローカル変数の一覧 |
| `backtrace` | `bt` | 関数呼び出しスタック |
| `quit` | `q` | gdb終了 |
| `watch stash` | | stashが変更されたら停止 |
| `display stash` | | 毎ステップでstashを表示 |
| `info breakpoints` | `i b` | ブレークポイント一覧 |
| `delete 1` | `d 1` | ブレークポイント1を削除 |

**gdbセッション例**:

```
(gdb) break get_next_line
Breakpoint 1 at 0x401234: file get_next_line.c, line 52.

(gdb) run
Starting program: /home/user/gnl
Breakpoint 1, get_next_line (fd=3) at get_next_line.c:52
52          if (fd < 0 || BUFFER_SIZE <= 0)

(gdb) print stash
$1 = 0x0                         <- NULL（初回呼び出し）

(gdb) next
54          stash = ft_init_stash(stash);

(gdb) next
56          if (!stash)

(gdb) print stash
$2 = 0x5555555592a0 ""           <- 空文字列に初期化された

(gdb) next
58          stash = ft_read_to_stash(fd, stash);

(gdb) step                        <- ft_read_to_stashの中に入る
ft_read_to_stash (fd=3, stash=0x5555555592a0 "")
    at get_next_line.c:16

(gdb) next
20          buf = malloc(sizeof(char) * (BUFFER_SIZE + 1));

(gdb) next
24          bytes_read = 1;

(gdb) next
25          while (!ft_strchr(stash, '\n') && bytes_read > 0)

(gdb) next
27              bytes_read = read(fd, buf, BUFFER_SIZE);

(gdb) next
31              buf[bytes_read] = '\0';

(gdb) print bytes_read
$3 = 5

(gdb) print buf
$4 = 0x5555555592c0 "AB\nCD"

(gdb) x/6c buf
0x5555555592c0: 65 'A'  66 'B'  10 '\n'  67 'C'  68 'D'  0 '\0'

(gdb) continue
Line 1: [AB\n]
Breakpoint 1, get_next_line (fd=3) at get_next_line.c:52

(gdb) print stash
$5 = 0x555555559320 "CD"         <- 前回の残りデータ!
```

### 2.5 gdbでメモリの内容を確認する

```
(gdb) x/20c stash
  stashから20バイトを文字として表示

  0x5555555592a0: 72 'H' 101 'e' 108 'l' 108 'l' 111 'o' 10 '\n'
  0x5555555592a6: 87 'W' 111 'o' 114 'r' 108 'l' 100 'd' 0 '\0'

(gdb) x/20x stash
  stashから20バイトを16進数で表示

  0x5555555592a0: 0x48 0x65 0x6c 0x6c 0x6f 0x0a 0x57 0x6f
  0x5555555592a8: 0x72 0x6c 0x64 0x00 0x00 0x00 0x00 0x00
```

### 2.6 gdbの便利なテクニック

```
テクニック1: 条件付きブレークポイント
  (gdb) break ft_read_to_stash if bytes_read == 0
  → bytes_read が 0（EOF）のときだけ停止

テクニック2: watchpoint
  (gdb) watch stash
  → stash の値が変更されるたびに停止
  → 「stash がいつ NULL になるか」を追跡するのに有用

テクニック3: 変数の自動表示
  (gdb) display stash
  (gdb) display bytes_read
  → 毎ステップで stash と bytes_read を表示

テクニック4: バックトレースの確認
  (gdb) backtrace
  #0  ft_strjoin (s1=0x0, s2=0x...) at gnl_utils.c:50
  #1  ft_read_to_stash (fd=3, stash=0x0) at gnl.c:34
  #2  get_next_line (fd=3) at gnl.c:58
  #3  main () at main.c:15
  → セグフォルト時にどの関数のどの行で発生したか特定

テクニック5: メモリの書き換え（デバッグ用）
  (gdb) set stash = "test\n"
  → stash の値を強制的に変更してテスト
  → 「もし stash がこの値だったら?」のテストに有用
```

### 2.7 valgrindの詳細な使い方

```bash
# 基本的なメモリリークチェック
valgrind --leak-check=full ./gnl

# 全てのリーク種類を表示
valgrind --leak-check=full --show-leak-kinds=all ./gnl

# 未初期化値の追跡
valgrind --track-origins=yes ./gnl

# 範囲外アクセスの検出
valgrind --tool=memcheck ./gnl

# 最大限の情報（推奨）
valgrind --leak-check=full --show-leak-kinds=all \
    --track-origins=yes --verbose ./gnl

# ファイルに結果を保存
valgrind --leak-check=full --log-file=valgrind.log ./gnl
```

**valgrindの主要なエラーメッセージと対処法**:

```
1. "Invalid read of size X"
   → 解放済みメモリや範囲外を読んでいる
   → use-after-free か buffer overflow を確認
   → スタックトレースの行番号を確認

2. "Invalid write of size X"
   → 解放済みメモリや範囲外に書き込んでいる
   → buf[bytes_read] = '\0' の位置が正しいか確認
   → malloc のサイズが十分か確認

3. "Conditional jump or move depends on uninitialised value"
   → 未初期化の変数を条件分岐に使っている
   → 変数の初期化を確認
   → 特に: bytes_read の初期化

4. "X bytes in Y blocks are definitely lost"
   → メモリリーク確定
   → スタックトレースを見てどのmallocが原因か特定
   → free忘れの箇所を修正

5. "Invalid free() / delete / delete[]"
   → mallocしていないアドレスをfreeしようとしている
   → ダブルフリーの可能性
   → free後にポインタをNULLに設定しているか確認
```

### 2.8 valgrindの出力を読む実践

```
実際のvalgrind出力例（リークがある場合）:

==12345== HEAP SUMMARY:
==12345==     in use at exit: 42 bytes in 1 blocks
==12345==   total heap usage: 15 allocs, 14 frees, 500 bytes allocated

この出力から読み取れること:
  ・15回mallocして14回freeした → 1回分のfreeが足りない
  ・終了時に42バイトが未解放
  → どの malloc が原因かをスタックトレースで確認

==12345== 42 bytes in 1 blocks are definitely lost in loss record 1 of 1
==12345==    at 0x4C2FB55: malloc (vg_replace_malloc.c:...)
==12345==    by 0x401234: ft_strjoin (get_next_line_utils.c:50)
==12345==    by 0x401567: ft_read_to_stash (get_next_line.c:34)
==12345==    by 0x401789: get_next_line (get_next_line.c:62)
==12345==    by 0x401900: main (main.c:15)

この出力から読み取れること:
  ・ft_strjoin の50行目で malloc された42バイトがリーク
  ・ft_read_to_stash の34行目から ft_strjoin が呼ばれている
  → ft_strjoin で s1 を free し忘れている可能性が高い
```

---

## 3. テストファイルの作り方

### 3.1 基本テストファイル一覧

```bash
# テストディレクトリの作成
mkdir -p tests

# 1. 通常の複数行ファイル
printf "Hello World\nThis is line 2\nLast line\n" > tests/normal.txt

# 2. 最終行に\nがないファイル
printf "Line 1\nLine 2\nNo newline at end" > tests/no_nl.txt

# 3. 空ファイル
> tests/empty.txt

# 4. 改行のみ
printf "\n" > tests/nl_only.txt

# 5. 連続改行
printf "\n\n\n" > tests/multi_nl.txt

# 6. 1文字のみ
printf "A" > tests/single_char.txt

# 7. 1文字+改行
printf "A\n" > tests/single_char_nl.txt

# 8. 長い行（1000文字+改行）
python3 -c "print('A' * 1000)" > tests/long_line.txt

# 9. 非常に長い行（100000文字+改行）
python3 -c "print('B' * 100000)" > tests/very_long.txt

# 10. 多数の短い行（1000行）
python3 -c "
for i in range(1000):
    print(f'Line {i:04d}')
" > tests/many_lines.txt

# 11. 空行を含むファイル
printf "Hello\n\nWorld\n\n\nEnd\n" > tests/empty_lines.txt

# 12. タブや特殊文字を含む
printf "Tab\there\nSpace  here\nEnd\n" > tests/special.txt

# 13. 1行のみで改行あり
printf "Only one line\n" > tests/one_line_nl.txt

# 14. 1行のみで改行なし
printf "Only one line" > tests/one_line_no_nl.txt

# ファイルの確認
ls -la tests/
for f in tests/*; do echo "=== $f ==="; xxd "$f" | head -5; done
```

### 3.2 テスト用main.c（包括的テスト）

```c
#include "get_next_line.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

static void	test_file(const char *path)
{
	int		fd;
	char	*line;
	int		i;

	printf("=== Testing: %s ===\n", path);
	fd = open(path, O_RDONLY);
	if (fd == -1)
	{
		printf("Error: cannot open %s\n", path);
		return ;
	}
	i = 1;
	while ((line = get_next_line(fd)) != NULL)
	{
		printf("  Line %d (len=%zu): [%s]",
			i, strlen(line), line);
		if (line[strlen(line) - 1] != '\n')
			printf("  <no newline>\n");
		free(line);
		i++;
	}
	printf("  (NULL returned)\n\n");
	close(fd);
}

static void	test_invalid_fd(void)
{
	char	*line;

	printf("=== Testing: invalid fd (-1) ===\n");
	line = get_next_line(-1);
	printf("  Result: %s\n\n",
		line ? line : "NULL");
	if (line)
		free(line);
}

int	main(void)
{
	test_file("tests/normal.txt");
	test_file("tests/no_nl.txt");
	test_file("tests/empty.txt");
	test_file("tests/nl_only.txt");
	test_file("tests/multi_nl.txt");
	test_file("tests/single_char.txt");
	test_file("tests/long_line.txt");
	test_file("tests/empty_lines.txt");
	test_invalid_fd();
	return (0);
}
```

### 3.3 自動テストスクリプト

```bash
#!/bin/bash
# test_all.sh - 全BUFFER_SIZEで全テストを実行

BUFFER_SIZES="1 42 100 9999 10000000"
PASS=0
FAIL=0

for bs in $BUFFER_SIZES; do
    echo "============================================"
    echo "Testing with BUFFER_SIZE=$bs"
    echo "============================================"

    # コンパイル
    cc -Wall -Wextra -Werror -g -D BUFFER_SIZE=$bs \
        get_next_line.c get_next_line_utils.c test_main.c -o gnl_test
    if [ $? -ne 0 ]; then
        echo "FAIL: Compilation failed with BUFFER_SIZE=$bs"
        FAIL=$((FAIL + 1))
        continue
    fi

    # 実行
    ./gnl_test > output_$bs.txt 2>&1
    if [ $? -ne 0 ]; then
        echo "FAIL: Runtime error with BUFFER_SIZE=$bs"
        FAIL=$((FAIL + 1))
        continue
    fi

    # valgrind
    valgrind --leak-check=full --error-exitcode=1 \
        ./gnl_test > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo "FAIL: Memory leak with BUFFER_SIZE=$bs"
        FAIL=$((FAIL + 1))
    else
        echo "PASS"
        PASS=$((PASS + 1))
    fi

    rm -f gnl_test output_$bs.txt
done

echo "============================================"
echo "Results: $PASS passed, $FAIL failed"
echo "============================================"
```

---

## 4. 実践演習

### 4.1 演習1: コードの穴埋め問題

以下のft_strjoinの一部が欠けています。穴を埋めてください。

```c
char	*ft_strjoin(char *s1, char *s2)
{
	size_t	i;
	size_t	j;
	char	*joined;

	if (!s1 || !s2)
		return (______);        /* (A) 何を返す? */
	joined = malloc(sizeof(char) * (______ + ______ + 1));  /* (B) */
	if (!joined)
	{
		______;                  /* (C) 何をfreeする? */
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
		joined[____] = s2[j];   /* (D) インデックスは? */
		j++;
	}
	joined[____] = '\0';         /* (E) ヌル終端の位置は? */
	______;                      /* (F) 何をfreeする? */
	return (joined);
}
```

**答え**:
- (A): `NULL`
- (B): `ft_strlen(s1)` + `ft_strlen(s2)` + 1
- (C): `free(s1)` - malloc失敗でもs1(旧stash)を解放
- (D): `i + j`
- (E): `i + j`
- (F): `free(s1)` - 古いstashを解放

### 4.2 演習2: バグの特定問題

以下のコードにはバグがあります。全て見つけてください。

```c
/* バグ入りバージョン */
char	*ft_trim_stash(char *stash)
{
	int		i;           /* バグ1: size_tではなくint */
	int		j;
	char	*trimmed;

	i = 0;
	while (stash[i] && stash[i] != '\n')  /* バグ2: NULLチェックなし */
		i++;
	if (!stash[i])
	{
		free(stash);
		return (NULL);
	}
	trimmed = malloc(ft_strlen(stash) - i);
	if (!trimmed)
		return (NULL);   /* バグ3: stashをfreeしていない */
	i++;
	j = 0;
	while (stash[i])
		trimmed[j++] = stash[i++];
	trimmed[j] = '\0';
	/* バグ4: stashをfreeしていない */
	return (trimmed);
}
```

**答え**:
1. `int i, j` -> `size_t i, j` (符号なし整数を使うべき)
2. `stash`がNULLの場合にstash[i]でセグフォルト -> 冒頭にNULLチェック追加
3. malloc失敗時にstashをfreeしていない -> `free(stash); return (NULL);`
4. 正常終了時にstashをfreeしていない -> `free(stash);`を追加

### 4.3 演習3: 実行トレース問題

以下の状況での変数の値を追跡してください。

```
ファイル内容: "X\n"
BUFFER_SIZE = 10

get_next_line(fd) 呼び出し1:
  stash の初期値: ______
  ft_init_stash 後の stash: ______
  read() の戻り値: ______
  buf の内容: ______
  ft_strjoin 後の stash: ______
  ft_get_line の戻り値: ______
  ft_trim_stash 後の stash: ______
  get_next_line の戻り値: ______

get_next_line(fd) 呼び出し2:
  stash の初期値: ______
  get_next_line の戻り値: ______
```

**答え**:
```
呼び出し1:
  stash の初期値: NULL
  ft_init_stash 後の stash: "" (空文字列, malloc(1))
  read() の戻り値: 2 ("X\n"の2バイト)
  buf の内容: "X\n\0"
  ft_strjoin 後の stash: "X\n"
  ft_get_line の戻り値: "X\n"
  ft_trim_stash 後の stash: NULL (\nの後に何もないため)
  get_next_line の戻り値: "X\n"

呼び出し2:
  stash の初期値: NULL
  ft_init_stash 後の stash: "" (malloc(1))
  read() の戻り値: 0 (EOF)
  stash[0] == '\0' -> free(stash) -> stash = NULL
  get_next_line の戻り値: NULL
```

### 4.4 演習4: メモリ状態の追跡

以下の時点でのヒープの状態を図示してください。

```
ファイル: "AB\nC"  BUFFER_SIZE=5

get_next_line(fd) 1回目、ft_read_to_stashの中:

Time 1: buf = malloc(6) 直後
  ヒープ: ______

Time 2: read() -> "AB\nC\0" 直後
  ヒープ: ______

Time 3: stash = ft_strjoin("", "AB\nC") 直後
  ヒープ: ______

Time 4: free(buf) 直後
  ヒープ: ______

Time 5: line = ft_get_line("AB\nC") 直後
  ヒープ: ______

Time 6: stash = ft_trim_stash("AB\nC") 直後
  ヒープ: ______
```

**答え**:
```
Time 1: buf = malloc(6)
  ヒープ: [stash:"" 1B] [buf:?????? 6B]

Time 2: read -> buf = "AB\nC\0"
  ヒープ: [stash:"" 1B] [buf:"AB\nC\0" 6B]

Time 3: ft_strjoin("", "AB\nC")
  -> joined = malloc(5) = "AB\nC\0"
  -> free("") <- 古いstash解放
  ヒープ: [buf:"AB\nC\0" 6B] [stash:"AB\nC\0" 5B]

Time 4: free(buf)
  ヒープ: [stash:"AB\nC\0" 5B]

Time 5: line = ft_get_line("AB\nC") -> line = "AB\n\0"
  -> line = malloc(4)
  ヒープ: [stash:"AB\nC\0" 5B] [line:"AB\n\0" 4B]

Time 6: stash = ft_trim_stash("AB\nC")
  -> trimmed = malloc(2) = "C\0"
  -> free("AB\nC") <- 古いstash解放
  ヒープ: [line:"AB\n\0" 4B] [trimmed:"C\0" 2B]
  stash = trimmed = "C"

  ※ line は呼び出し元が free する
```

### 4.5 演習5: 設計の選択問題

**問題**: ft_strjoinでs1をfreeする設計にしましたが、
もしfreeしない（標準のstrjoinと同じ）設計にした場合、
ft_read_to_stashのコードはどう変わりますか?

```c
/* s1をfreeしないft_strjoin */
char	*ft_strjoin_nofree(char *s1, char *s2)
{
	/* ... s1をfreeしない ... */
	return (joined);
}

/* ft_read_to_stashはどう書き直す? */
static char	*ft_read_to_stash(int fd, char *stash)
{
	char	*buf;
	char	*tmp;        /* 一時変数が必要! */
	int		bytes_read;

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
			free(stash);
			return (NULL);
		}
		buf[bytes_read] = '\0';
		tmp = stash;                    /* 古いstashを保存 */
		stash = ft_strjoin_nofree(stash, buf);
		free(tmp);                      /* 古いstashを解放 */
	}
	free(buf);
	return (stash);
}
```

**結論**: s1をfreeしない場合、呼び出し側で毎回tmpに保存して
freeする必要がある。s1をfreeする設計の方がコードが簡潔で、
free忘れのリスクが低い。

### 4.6 演習6: エラーパスの追跡

**問題**: 以下の各エラーシナリオで、確保された全メモリが
正しく解放されるかを追跡してください。

```
シナリオ1: ft_init_stash で malloc 失敗
  get_next_line():
    stash = NULL
    ft_init_stash(NULL):
      malloc(1) -> NULL!
      return NULL
    stash = NULL
    if (!stash) -> true
    return NULL

  解放漏れ: なし（何も確保されていない）
  結果: OK


シナリオ2: ft_read_to_stash で buf の malloc 失敗
  get_next_line():
    stash = "" (ft_init_stash で確保済み)
    ft_read_to_stash(fd, stash):
      malloc(BUFFER_SIZE + 1) -> NULL!
      return NULL
    stash = NULL   <- stash(= "")のアドレスが失われた!
    if (!stash) -> true
    return NULL

  解放漏れ: あり! ft_init_stash で確保した "" がリーク!
  対策: ft_read_to_stash が NULL を返す前に stash を free するか、
        呼び出し元で元の stash を保持しておく


シナリオ3: read() が -1 を返す
  ft_read_to_stash(fd, stash):
    buf = malloc(BUFFER_SIZE + 1) -> 成功
    read() -> -1
    free(buf)
    free(stash)
    return NULL

  解放漏れ: なし（buf と stash 両方解放）
  結果: OK
```

---

## 5. よくある落とし穴と回避方法

### 5.1 落とし穴一覧

| # | 落とし穴 | 症状 | 回避方法 |
|---|---------|------|---------|
| 1 | ft_strjoinでs1をfreeし忘れ | メモリリーク | ft_strjoin内でfree(s1) |
| 2 | read()エラー時のstash解放忘れ | メモリリーク | buf + stash 両方free |
| 3 | BUFFER_SIZE=0/負の値の未チェック | 不正動作 | 冒頭でチェック |
| 4 | buf[bytes_read]のヌル終端忘れ | ゴミデータ混入 | buf[bytes_read]='\0' |
| 5 | ft_trim_stashでのダブルfree | クラッシュ | 新ポインタをstashに代入 |
| 6 | size_tに-1を代入 | 巨大正数になる | int or ssize_tを使う |
| 7 | stash=NULLのままft_strjoinに渡す | クラッシュ | ft_init_stashで初期化 |
| 8 | ft_get_lineでmallocサイズ不足 | バッファオーバーフロー | +1を忘れない |
| 9 | ft_trim_stashのmallocサイズ計算ミス | 文字切れ/オーバーフロー | ft_strlen(stash)-i |
| 10 | EOF時にstash=""をfreeし忘れ | still reachable | free+NULL代入 |

### 5.2 デバッグフローチャート

```
バグが発生した!
      |
      v
  セグメンテーションフォルト?
  +-- YES -> NULLポインタアクセスの可能性
  |          gdb で backtrace を確認
  |          NULLチェックの追加
  |
  +-- NO -> 出力が間違っている?
           +-- YES -> printfデバッグでstashの状態を追跡
           |          BUFFER_SIZE=1で動作を確認
           |          ft_get_line, ft_trim_stashの境界条件を確認
           |
           +-- NO -> メモリリーク?
                    +-- YES -> valgrindでリーク箇所を特定
                    |          ft_strjoinのfree(s1)を確認
                    |          エラーパス全てでfreeしているか確認
                    |
                    +-- NO -> 無限ループ?
                             -> readの戻り値チェックを確認
                             -> bytes_readの初期値を確認
                             -> whileの条件を確認
```

### 5.3 段階的なデバッグアプローチ

```
Level 1: まずシンプルなケースで確認
  ・"Hello\n" を BUFFER_SIZE=42 で読む
  ・これが動かないなら基本的なバグがある

Level 2: 複数行で確認
  ・"A\nB\nC\n" を BUFFER_SIZE=42 で読む
  ・ft_trim_stash の動作を確認

Level 3: エッジケースで確認
  ・空ファイル、"\n\n\n"、fd=-1
  ・各ケースで正しい結果が返されるか

Level 4: BUFFER_SIZE=1 で確認
  ・全てのテストを BUFFER_SIZE=1 で実行
  ・このテストが最も厳しい

Level 5: メモリリークの確認
  ・全テストケースで valgrind を実行
  ・definitely lost = 0 を確認
```

---

## 6. マスターすべき重要概念チェックリスト

### 必須概念

| 概念 | 理解度チェック | 関連セクション |
|------|--------------|-------------|
| ファイルディスクリプタ | fdが何を表すか3文で説明できる | 01 |
| read()システムコール | 3つの戻り値の意味を即答できる | 01 |
| static変数 | ローカル変数との違いを4つ挙げられる | 01 |
| malloc/free | メモリリークの原因と防止法を説明できる | 01, 02 |
| ポインタ | 矢印図でstashの変遷を描ける | 02 |
| 文字列操作 | ヌル終端の概念を説明できる | 02 |

### 応用概念

| 概念 | 理解度チェック | 関連セクション |
|------|--------------|-------------|
| バッファリング | なぜバッファリングが必要か3つの理由を挙げられる | 01, 06 |
| 所有権管理 | どの関数がどのメモリをfreeすべきか全て説明できる | 06 |
| エラー伝播 | エラー発生時のリソース解放フローを全て追跡できる | 06 |
| 計算量 | BUFFER_SIZE=1でO(n^2)になる理由を説明できる | 06 |
| Norm準拠 | 25行/5関数制限を理解し、遵守できる | 03 |

### 自己評価チェック

```
以下の質問に全て答えられるか確認:

1. get_next_lineはなぜstatic変数を使うのか?
   -> プロトタイプが固定されていて状態を引数で渡せないから

2. ft_strjoinでs1をfreeするのはなぜか?
   -> stash = ft_strjoin(stash, buf) パターンで
      古いstashのアドレスが失われるのを防ぐため

3. BUFFER_SIZE=1でO(n^2)になるのはなぜか?
   -> ft_strjoinが毎回stash全体をコピーするため、
      1+2+3+...+n = n(n+1)/2 のコピーが発生

4. 空ファイルでNULLを返す仕組みは?
   -> stash="" -> read -> 0(EOF) -> stash[0]=='\0'
      -> free(stash) -> stash=NULL -> return NULL

5. fd=-1でメモリリークしない理由は?
   -> if (fd < 0) で即座にreturn NULLし、
      mallocを一切行っていないから
```
