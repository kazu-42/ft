# 04 - 評価基準

このセクションでは、ft_printf の評価で確認される全項目を詳細に解説します。evaluator がチェックする内容、よくあるバグとその修正方法、defense（口頭試問）の完全対策を含みます。

---

## 1. Evaluator がチェックする項目

### 1.1 Makefile の動作

evaluator は以下の順序で Makefile をテストします。

**チェック1: 初回ビルド**

```bash
make
```

- `libftprintf.a` が生成されるか確認
- `cc` と `-Wall -Wextra -Werror` が使われているか確認
- エラーや警告が出ないか確認

**チェック2: Relink テスト**

```bash
make
```

2回目の `make` で **何も起きない** ことを確認します。

```
make: Nothing to be done for 'all'.
```

もし2回目の `make` で `ar rcs` が再実行される場合、relink が発生しています。これは減点対象です。

**チェック3: clean**

```bash
make clean
ls *.o  # .o ファイルが存在しないことを確認
ls libftprintf.a  # ライブラリは残っていることを確認
```

**チェック4: fclean**

```bash
make fclean
ls *.o  # .o ファイルが存在しないことを確認
ls libftprintf.a  # ライブラリも削除されていることを確認
```

**チェック5: re**

```bash
make re
```

`fclean` + `all` が実行され、完全な再ビルドが行われることを確認。

**チェック6: ar 使用確認**

```bash
# Makefile の中を確認
# libtool が使われていないこと
# ar rcs (または ar -rcs) が使われていること
```

### 1.2 Norm 準拠

```bash
norminette *.c *.h
```

すべてのファイルで `OK` が出力されることを確認します。

**Norm の主要ルール:**

| ルール | 制限 | よくある違反 |
|--------|------|------------|
| 関数の行数 | 25行以内（`{` と `}` を含む） | 複雑な条件分岐 |
| ファイルの関数数 | 5個以内 | ヘルパー関数の増加 |
| 1行の文字数 | 80文字以内 | 長い条件式、長い関数名 |
| 関数の引数 | 4個以内 | 構造体でまとめる必要 |
| 変数宣言 | 関数の先頭のみ | ブロック内での宣言 |
| 宣言と処理の間 | 空行が必要 | 空行の挿入忘れ |
| `for` 文 | 禁止 | `while` に書き換え |
| include guard | 必須 | `#ifndef`/`#define`/`#endif` |
| グローバル変数 | 禁止 | static 変数で代替（非推奨） |
| インデント | タブ | スペースの使用 |

**Norm 違反の具体例と修正:**

```c
/* NG: for 文の使用 */
for (int i = 0; i < len; i++)
    write(1, &str[i], 1);

/* OK: while 文に書き換え */
i = 0;
while (i < len)
{
    write(1, &str[i], 1);
    i++;
}
```

```c
/* NG: 変数宣言が関数の先頭にない */
int ft_func(void)
{
    int a;
    a = 42;
    int b;     /* NG: 処理の後に宣言 */
    b = a + 1;
    return (b);
}

/* OK: 変数は先頭で宣言 */
int ft_func(void)
{
    int a;
    int b;

    a = 42;
    b = a + 1;
    return (b);
}
```

### 1.3 Library の作成

- `libftprintf.a` がリポジトリの **ルートディレクトリ** に生成されるか
- `ar` コマンドで作成されているか（`libtool` 不使用）
- リンクして正常に動作するか

```bash
# evaluator のテスト手順
make
cc -Wall -Wextra -Werror main.c -L. -lftprintf -o test
./test
```

### 1.4 機能テスト

各変換指定子について、本物の `printf` と同じ出力・戻り値を返すかテストされます。

evaluator は通常、以下のようなテストコードを使います。

```c
#include <stdio.h>
#include <limits.h>
#include "ft_printf.h"

int	main(void)
{
	int	r1;
	int	r2;

	r1 = printf("test: %d\n", 42);
	r2 = ft_printf("test: %d\n", 42);
	if (r1 != r2)
		printf("FAIL: printf=%d, ft_printf=%d\n", r1, r2);
	else
		printf("PASS: ret=%d\n", r1);
	return (0);
}
```

---

## 2. Edge Case（境界値テスト）の完全ガイド

### 2.1 NULL 文字列（%s に NULL を渡す）

```c
ft_printf("%s", NULL);
/* 期待出力: (null) */
/* 期待戻り値: 6 */
```

**なぜ危険か:**

NULL ポインタをデリファレンスすると segfault が発生します。

```c
/* 危険なコード */
int	ft_print_str(char *str)
{
    int	len;

    len = ft_strlen(str);  /* str == NULL -> segfault! */
    write(1, str, len);     /* str == NULL -> segfault! */
}
```

**正しい実装:**

```c
int	ft_print_str(char *str)
{
    int	len;

    if (!str)
        str = "(null)";  /* NULL を安全な文字列に置き換え */
    len = ft_strlen(str);
    if (write(1, str, len) == -1)
        return (-1);
    return (len);
}
```

### 2.2 NULL ポインタ（%p に NULL を渡す）

```c
ft_printf("%p", NULL);
/* Linux の printf: "(nil)" */
/* macOS の printf: "0x0" */
```

この実装では `(nil)` を出力する設計を採用しています。

```c
int	ft_print_ptr(unsigned long long ptr)
{
    int	count;

    if (ptr == 0)
    {
        if (write(1, "(nil)", 5) == -1)
            return (-1);
        return (5);
    }
    /* 非NULL の場合は 0x + 16進数 */
    if (write(1, "0x", 2) == -1)
        return (-1);
    count = 2;
    if (ft_put_ptr(ptr) == -1)
        return (-1);
    count += ft_ptr_len(ptr);
    return (count);
}
```

### 2.3 INT_MIN（-2147483648）の処理

```c
ft_printf("%d", -2147483648);
/* 期待出力: -2147483648 */
/* 期待戻り値: 11 */
```

**なぜ特別か: ビットレベルの解説**

```
INT_MIN = -2147483648
ビットパターン: 10000000 00000000 00000000 00000000

-INT_MIN の計算（2の補数の否定）:
1. ビット反転: 01111111 11111111 11111111 11111111 = 2147483647
2. +1:         10000000 00000000 00000000 00000000 = -2147483648 (!)

つまり -INT_MIN = INT_MIN（32ビットの世界では）
これは undefined behavior!
```

**間違ったコード:**

```c
/* NG: -n がオーバーフロー */
static int	ft_put_nbr(int n)
{
    if (n < 0)
    {
        write(1, "-", 1);
        n = -n;  /* n == INT_MIN のとき undefined behavior! */
    }
    /* ... */
}
```

**正しいコード:**

```c
static int	ft_put_nbr(int n)
{
    if (n == -2147483648)
    {
        if (write(1, "-2147483648", 11) == -1)
            return (-1);
        return (0);
    }
    if (n < 0)
    {
        if (write(1, "-", 1) == -1)
            return (-1);
        n = -n;  /* 安全: n != INT_MIN */
    }
    /* ... */
}
```

**別の解法: long を使う**

```c
static int	ft_put_nbr(long n)
{
    if (n < 0)
    {
        write(1, "-", 1);
        n = -n;  /* long なので安全（64ビット環境） */
    }
    /* ... */
}
```

> **注意**: `long` を使う方法はエレガントですが、va_arg で `int` を受け取った後に `long` にキャストする必要があります。

### 2.4 UINT_MAX の処理

```c
ft_printf("%u", 4294967295u);
/* 期待出力: 4294967295 */
/* 期待戻り値: 10 */
```

`unsigned int` は常に非負なので、符号の処理は不要です。ただし、再帰の深さが最大10になるため、桁数計算が正しいことを確認してください。

### 2.5 ゼロの処理

```c
ft_printf("%d", 0);   /* "0", ret=1 */
ft_printf("%u", 0);   /* "0", ret=1 */
ft_printf("%x", 0);   /* "0", ret=1 */
ft_printf("%X", 0);   /* "0", ret=1 */
```

**よくあるバグ: 桁数が 0 になる**

```c
/* NG: n == 0 のとき len = 0 が返る */
static int	ft_numlen(int n)
{
    int	len;

    len = 0;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    return (len);  /* n == 0 のとき len = 0 */
}

/* OK: n <= 0 のとき len = 1 から開始 */
static int	ft_numlen(int n)
{
    int	len;

    len = 0;
    if (n <= 0)
        len = 1;  /* 0 または負の数は最低1桁 */
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    return (len);
}
```

### 2.6 %c で '\0' を渡す

```c
int ret = ft_printf("%c", '\0');
/* '\0' を出力（端末には見えない） */
/* ret = 1 */
```

**確認方法:**

```bash
# 出力をファイルにリダイレクトしてバイト数を確認
./test > output.txt
wc -c output.txt  # 1 output.txt
xxd output.txt     # 00000000: 00  .
```

### 2.7 連続した %

```c
ft_printf("%%%%");     /* "%%" を出力, ret=2 */
ft_printf("%%%%%%");   /* "%%%" を出力, ret=3 */
ft_printf("a%%b%%c");  /* "a%b%c" を出力, ret=5 */
```

### 2.8 format が NULL

```c
ft_printf(NULL);
/* segfault してはいけない */
/* 戻り値: -1 */
```

### 2.9 % の後に何もない場合

```c
ft_printf("hello %");
/* この動作は undefined だが、segfault しないことが望ましい */
```

format string が `%` で終わる場合、`format++` で `'\0'` を読みます。このとき、ディスパッチャが `'\0'` に対して何もしない（`count = 0` を返す）設計であれば、`format++` で `'\0'` を超えたところでループの `while (*format)` が false になり、安全に終了します。

ただし、`format++` が `'\0'` を超えると `while (*format)` で `'\0'` の次のバイトを読むことになるため、`%` の後が `'\0'` の場合に `format++` をスキップする処理を入れる方が安全です。

---

## 3. よくあるバグの詳細分析

### 3.1 INT_MIN の処理ミス

**症状**: `ft_printf("%d", -2147483648)` で正しく出力されない（ゴミの値が出る、または segfault）。

**原因**: `-n` がオーバーフローする。

**修正**: INT_MIN を特別処理する（前述）。

**テストコード:**

```c
int r1 = printf("%d", -2147483648);
printf("\n");
int r2 = ft_printf("%d", -2147483648);
printf("\n");
printf("printf ret: %d, ft_printf ret: %d\n", r1, r2);
/* r1 == r2 == 11 であるべき */
```

### 3.2 NULL 文字列の未処理

**症状**: `ft_printf("%s", NULL)` で segfault。

**原因**: NULL チェックなしで `ft_strlen(NULL)` や `write(1, NULL, ...)` を呼んでいる。

**修正**: 関数の先頭で NULL チェックを行う。

```c
int	ft_print_str(char *str)
{
    if (!str)          /* NULL チェック */
        str = "(null)";
    /* 以降は str が NULL でないことが保証される */
}
```

### 3.3 %p の NULL 処理

**症状**: `ft_printf("%p", NULL)` で `0x` だけ出力される、または `0x0` が出力される（環境に依存）。

**原因**: NULL ポインタの特別処理が不足。

**修正**: ptr == 0 のときに環境に応じた出力を行う。

### 3.4 va_arg の型不一致

**症状**: 出力がゴミの値になる、segfault する。

**原因**: `va_arg` で指定した型が実際の引数の型と一致しない。

**具体例:**

```c
/* NG: char * の引数を int で取得しようとする */
if (specifier == 's')
    count = ft_print_str((char *)va_arg(args, int));
/* int として 4 バイト読んでポインタに変換 -> 不正なアドレス */

/* OK */
if (specifier == 's')
    count = ft_print_str(va_arg(args, char *));
/* char * として 8 バイト（64bit環境）読む -> 正しいアドレス */
```

### 3.5 write のエラーチェック漏れ

**症状**: write が失敗してもエラーが伝播しない。戻り値が正の値になる。

**原因**: write の戻り値をチェックしていない。

```c
/* NG */
write(1, &c, 1);  /* 失敗しても無視 */
return (1);

/* OK */
if (write(1, &c, 1) == -1)
    return (-1);   /* エラーを伝播 */
return (1);
```

**全レイヤーでのエラーチェック:**

```
write() で -1 が返る
  -> ft_put_nbr() が -1 を検出、-1 を返す
    -> ft_print_nbr() が -1 を検出、-1 を返す
      -> ft_print_format() が -1 を返す
        -> ft_printf() が -1 を検出、-1 を返す
```

どこか1箇所でもチェックが漏れると、エラーが正しく伝播しません。

### 3.6 Makefile の relink 問題

**症状**: `make` を2回実行すると、2回目でも `ar rcs` が実行される。

**原因**: 依存関係が正しく設定されていない。

**確認方法:**

```bash
make
make 2>&1 | grep -v "Nothing"
# 何も出力されなければ OK
# ar や cc のコマンドが表示されたら relink している
```

**よくある原因と修正:**

```makefile
# NG: 依存関係なしのルール（毎回実行される）
all:
	$(CC) $(CFLAGS) -c $(SRCS)
	ar rcs $(NAME) $(OBJS)

# OK: 依存関係あり（タイムスタンプで判断される）
$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)

%.o: %.c ft_printf.h
	$(CC) $(CFLAGS) -c $< -o $@
```

### 3.7 64ビットアドレスの切り詰め（%p）

**症状**: `%p` で上位ビットが失われ、正しいアドレスが出力されない。

**原因**: `unsigned int`（32ビット）でアドレスを受け取っている。

```c
/* NG: 64ビットアドレスが切り詰められる */
unsigned int addr = (unsigned int)ptr;
/* 0x7ffd559ffc54 -> 0x559ffc54 */

/* OK: unsigned long long で受け取る */
unsigned long long addr = (unsigned long long)ptr;
```

### 3.8 桁数計算のバグ

**症状**: 戻り値が実際の出力文字数と一致しない。

**原因**: 桁数計算関数にバグがある。

**チェックすべきケース:**

| 値 | 正しい桁数 | よくある誤り |
|-----|-----------|------------|
| `0` | `1` | `0`（ループが実行されない） |
| `-1` | `2`（"-1"） | `1`（符号を忘れる） |
| `-2147483648` | `11` | `10`（符号を忘れる）またはオーバーフロー |
| `2147483647` | `10` | - |

---

## 4. Norm 準拠チェックリスト

### 4.1 全項目一覧

| # | 項目 | 制限 | 確認コマンド |
|---|------|------|------------|
| 1 | 関数の行数 | 25行以内（`{` `}` 含む） | `norminette` |
| 2 | ファイルの関数数 | 5個以内 | `norminette` |
| 3 | 1行の文字数 | 80文字以内 | `norminette` |
| 4 | 関数の引数 | 4個以内 | `norminette` |
| 5 | 変数宣言の位置 | 関数の先頭のみ | `norminette` |
| 6 | 宣言と処理の間 | 空行必須 | `norminette` |
| 7 | `for` 文 | 禁止 | `norminette` |
| 8 | include guard | 必須 | `norminette` |
| 9 | グローバル変数 | 禁止 | `norminette` |
| 10 | インデント | タブのみ | `norminette` |
| 11 | return の括弧 | 必須: `return (value);` | `norminette` |
| 12 | ヘッダの42コメント | 必須 | `norminette` |

### 4.2 25行制限の対処法

25行制限は ft_printf で最も苦労するルールの一つです。

**テクニック1: 条件の簡潔化**

```c
/* NG: 冗長（行数を消費） */
if (specifier == 'c')
{
    count = ft_print_char(va_arg(args, int));
}
else if (specifier == 's')
{
    count = ft_print_str(va_arg(args, char *));
}

/* OK: 中括弧を省略（1行の場合） */
if (specifier == 'c')
    count = ft_print_char(va_arg(args, int));
else if (specifier == 's')
    count = ft_print_str(va_arg(args, char *));
```

**テクニック2: ヘルパー関数への分離**

```c
/* NG: 1関数が長すぎる */
int	ft_print_nbr(int n)
{
    /* 符号処理 + 出力 + 桁数計算 で 25行を超える */
}

/* OK: 機能を分離 */
static int	ft_put_nbr(int n)  { /* 出力のみ */ }
static int	ft_numlen(int n)   { /* 桁数計算のみ */ }
int	ft_print_nbr(int n)
{
    if (ft_put_nbr(n) == -1)
        return (-1);
    return (ft_numlen(n));
}
```

---

## 5. Defense（口頭試問）完全対策

### 5.1 基本質問（必ず聞かれる）

**Q: variadic functions の仕組みを説明してください。**

回答のポイント:
- `va_list` で引数リストの位置を追跡する
- `va_start` で初期化（最後の固定引数を起点）
- `va_arg` で型を指定して順番に取得
- `va_end` でクリーンアップ
- default argument promotion（char -> int, float -> double）に注意
- 型の不一致は undefined behavior

**Q: %p の NULL の場合はどう処理していますか？**

回答のポイント:
- NULL ポインタ（ptr == 0）を検出
- `(nil)` を直接 write で出力（Linux の printf 準拠の場合）
- macOS では `0x0` を出力（環境依存であることを説明できると良い）

**Q: INT_MIN はどう処理していますか？**

回答のポイント:
- 2の補数表現では `-INT_MIN` がオーバーフローする
- `INT_MIN` の絶対値（2147483648）は `int` の範囲外
- `n == -2147483648` を検出して文字列を直接出力
- これが undefined behavior を避けるため必要であることを説明

**Q: 戻り値はどのように計算していますか？**

回答のポイント:
- 各出力関数が自分の出力文字数を返す
- ft_printf がそれを `count` に累積
- エラー時は `-1` を返す
- 桁数は出力とは別の関数で計算（責務の分離）

**Q: write が失敗した場合はどうなりますか？**

回答のポイント:
- すべての write の戻り値をチェック
- `-1` が返されたら即座に `-1` を上位に返す
- エラーの伝播チェーン: write -> ft_put_xxx -> ft_print_xxx -> ft_printf

### 5.2 設計に関する質問

**Q: なぜ malloc を使わずに実装できるのですか？**

回答のポイント:
- 再帰を使って上位桁から順に write で出力
- バッファに溜めて逆順出力する方式の代わりに、コールスタックを「バッファ」として利用
- 再帰深度は最大10（int の桁数）で、スタックオーバーフローの危険なし
- malloc を使わないことで、メモリリークのリスクがゼロ

**Q: この設計の弱点は何ですか？**

回答のポイント:
- write の呼び出し回数が多い（1文字ずつ出力）
- 本物の printf は内部バッファでこれを最適化
- ただし教育目的としては、シンプルさが優先
- パフォーマンスが問題になるケースでは、バッファリング版を実装すべき

**Q: なぜ if-else chain で dispatch しているのですか？関数ポインタ配列は使えませんか？**

回答のポイント:
- 関数ポインタ配列も可能だが、各変換で va_arg の型が異なるため利点が薄い
- if-else chain の方が明示的で読みやすい
- Norm の25行制限内で初期化するのが難しい
- 変換指定子が9種類と少ないので、if-else で十分

**Q: bonus part の flag をどう実装しますか？**

回答のポイント:
- 構造体（`t_flags`）でフラグの状態を管理
- `%` の後、specifier の前に flag/width/precision を parse
- 各出力関数に `t_flags` を渡して、padding/prefix を適用
- mandatory の modular 設計がそのまま拡張可能

**Q: Makefile の relink を防ぐにはどうしますか？**

回答のポイント:
- 正しい依存関係の設定（`.o` -> `.c` + `.h`）
- Make のタイムスタンプ比較の仕組みを説明
- パターンルール `%.o: %.c ft_printf.h` でヘッダの変更も検出

### 5.3 コンピュータサイエンスに関する質問

**Q: 2の補数とは何ですか？なぜ使われるのですか？**

回答のポイント:
- 符号付き整数のビット表現方式
- MSB が符号ビット（0=正、1=負）
- 利点: 加算回路が1つで済む、ゼロが1つだけ、減算が加算に変換可能
- 欠点: 正と負の範囲が非対称（負の方が1つ多い）

**Q: static library と dynamic library の違いは何ですか？**

回答のポイント:
- static: コンパイル時にリンク、実行ファイルに組み込み、単体で動作
- dynamic: 実行時にリンク、共有可能、ファイルが小さい
- ft_printf は static library（`.a`）
- dynamic library は `.so`（Linux）/ `.dylib`（macOS）

**Q: write system call とは何ですか？putchar との違いは？**

回答のポイント:
- write はカーネルに直接命令する system call
- putchar は stdio のバッファを経由する C ライブラリ関数
- write は即座にカーネルに書き込みを依頼
- putchar はバッファに溜めてから write を呼ぶ（バッファリング）
- ft_printf では write を直接使い、stdio のバッファとの干渉を避ける

### 5.4 defense で差がつくポイント

以下の内容を答えられると、深い理解をアピールできます。

1. **ABI レベルの説明**: va_list がレジスタ保存領域とスタックをどう扱うか
2. **エンディアンの知識**: リトルエンディアンでのメモリレイアウト
3. **セキュリティ**: format string 攻撃（%n による書き込み）の存在を知っている
4. **パフォーマンス**: system call のコスト、バッファリングの効果
5. **標準規格**: C11 の仕様を参照して回答できる

---

## 6. テスト戦略

### 6.1 手動テストのテンプレート

```c
#include <stdio.h>
#include <limits.h>
#include "ft_printf.h"

void	test_char(void)
{
	int	r1;
	int	r2;

	printf("=== %%c tests ===\n");
	r1 = printf("char: %c\n", 'A');
	r2 = ft_printf("char: %c\n", 'A');
	printf("ret: printf=%d, ft_printf=%d %s\n\n",
		r1, r2, r1 == r2 ? "OK" : "FAIL");
}

int	main(void)
{
	test_char();
	/* 他のテストも同様に追加 */
	return (0);
}
```

### 6.2 自動テスターの活用

以下の有名なテスターを活用できます（GitHub で検索）。

- **Tripouille/printfTester** - 最も広く使われるテスター
- **ft_printf_tester** - 基本的なテストを網羅
- **printfTester** - 詳細なテスト

> **注意**: テスターは万能ではありません。テスターが通っても edge case を見逃している可能性があります。必ず手動でも重要なケースをテストしてください。

### 6.3 評価前の最終チェックリスト

- [ ] `norminette *.c *.h` でエラーなし
- [ ] `make` で `libftprintf.a` が生成される
- [ ] `make` を2回実行して relink しない
- [ ] `make clean` で `.o` ファイルが削除される
- [ ] `make fclean` で `.o` と `.a` が削除される
- [ ] `make re` で完全再ビルドされる
- [ ] 全変換指定子の基本テストが通る
- [ ] INT_MIN, UINT_MAX, NULL 文字列, NULL ポインタのテストが通る
- [ ] `%c` で `'\0'` を渡しても正しく動作する
- [ ] `ft_printf(NULL)` で segfault しない
- [ ] 戻り値が本物の printf と一致する
- [ ] write エラー時に -1 が返る

---

## 7. 詳細なテストケース集

### 7.1 %c の完全テスト

```c
/* 基本テスト */
ft_printf("%c", 'A');        /* 出力: A, ret: 1 */
ft_printf("%c", '0');        /* 出力: 0, ret: 1 */
ft_printf("%c", ' ');        /* 出力: (space), ret: 1 */

/* 境界値テスト */
ft_printf("%c", '\0');       /* 出力: (NUL), ret: 1 */
ft_printf("%c", 127);        /* 出力: DEL文字, ret: 1 */
ft_printf("%c", 1);          /* 出力: SOH(制御文字), ret: 1 */

/* promotion テスト */
ft_printf("%c", 65);         /* 出力: A, ret: 1 (int -> char) */
ft_printf("%c", 256 + 65);   /* 出力: A, ret: 1 (下位8ビットのみ) */
```

### 7.2 %s の完全テスト

```c
/* 基本テスト */
ft_printf("%s", "hello");     /* 出力: hello, ret: 5 */
ft_printf("%s", "");          /* 出力: (empty), ret: 0 */
ft_printf("%s", "a");         /* 出力: a, ret: 1 */

/* 特殊文字を含む文字列 */
ft_printf("%s", "hello\tworld");  /* 出力: hello\tworld, ret: 11 */
ft_printf("%s", "line1\nline2");  /* 出力: line1\nline2, ret: 11 */

/* NULL テスト */
ft_printf("%s", NULL);        /* 出力: (null), ret: 6 */

/* 長い文字列 */
ft_printf("%s", "abcdefghijklmnopqrstuvwxyz");
/* 出力: abcdefghijklmnopqrstuvwxyz, ret: 26 */
```

### 7.3 %d / %i の完全テスト

```c
/* 基本テスト */
ft_printf("%d", 0);           /* 出力: 0, ret: 1 */
ft_printf("%d", 1);           /* 出力: 1, ret: 1 */
ft_printf("%d", -1);          /* 出力: -1, ret: 2 */
ft_printf("%d", 42);          /* 出力: 42, ret: 2 */
ft_printf("%d", -42);         /* 出力: -42, ret: 3 */

/* 境界値テスト */
ft_printf("%d", 2147483647);  /* 出力: 2147483647, ret: 10 */
ft_printf("%d", -2147483648); /* 出力: -2147483648, ret: 11 */

/* %i は %d と同じ */
ft_printf("%i", 42);          /* 出力: 42, ret: 2 */
ft_printf("%i", -2147483648); /* 出力: -2147483648, ret: 11 */

/* 桁数テスト */
ft_printf("%d", 9);           /* ret: 1 */
ft_printf("%d", 10);          /* ret: 2 */
ft_printf("%d", 99);          /* ret: 2 */
ft_printf("%d", 100);         /* ret: 3 */
ft_printf("%d", 999999999);   /* ret: 9 */
ft_printf("%d", 1000000000);  /* ret: 10 */
```

### 7.4 %u の完全テスト

```c
/* 基本テスト */
ft_printf("%u", 0);            /* 出力: 0, ret: 1 */
ft_printf("%u", 42);           /* 出力: 42, ret: 2 */

/* 境界値テスト */
ft_printf("%u", 4294967295u);  /* 出力: 4294967295, ret: 10 */

/* signed -> unsigned 変換テスト */
ft_printf("%u", (unsigned int)-1);  /* 出力: 4294967295, ret: 10 */
```

### 7.5 %x / %X の完全テスト

```c
/* 基本テスト */
ft_printf("%x", 0);            /* 出力: 0, ret: 1 */
ft_printf("%x", 10);           /* 出力: a, ret: 1 */
ft_printf("%x", 15);           /* 出力: f, ret: 1 */
ft_printf("%x", 16);           /* 出力: 10, ret: 2 */
ft_printf("%x", 255);          /* 出力: ff, ret: 2 */
ft_printf("%x", 256);          /* 出力: 100, ret: 3 */

/* 大文字テスト */
ft_printf("%X", 255);          /* 出力: FF, ret: 2 */
ft_printf("%X", 10);           /* 出力: A, ret: 1 */

/* 境界値テスト */
ft_printf("%x", 4294967295u);  /* 出力: ffffffff, ret: 8 */
ft_printf("%X", 4294967295u);  /* 出力: FFFFFFFF, ret: 8 */
```

### 7.6 %p の完全テスト

```c
int x = 42;

/* 基本テスト */
ft_printf("%p", &x);           /* 出力: 0x7fff..., ret: varies */
ft_printf("%p", (void *)1);    /* 出力: 0x1, ret: 3 */
ft_printf("%p", (void *)0xff); /* 出力: 0xff, ret: 4 */

/* NULL テスト */
ft_printf("%p", NULL);         /* 出力: (nil), ret: 5 */
ft_printf("%p", (void *)0);    /* 出力: (nil), ret: 5 */
```

### 7.7 %% の完全テスト

```c
ft_printf("%%");               /* 出力: %, ret: 1 */
ft_printf("100%%");            /* 出力: 100%, ret: 4 */
ft_printf("%%%%");             /* 出力: %%, ret: 2 */
ft_printf("%%%%%%");           /* 出力: %%%, ret: 3 */
ft_printf("a%%b%%c");          /* 出力: a%b%c, ret: 5 */
```

### 7.8 組み合わせテスト

```c
/* 全変換を1つの呼び出しで */
ft_printf("%c%s%p%d%i%u%x%X%%",
    'A', "BC", (void *)255, -42, 42, 42u, 255, 255);
/* 出力: ABC0xff-424242ffFF% */

/* 引数なしの format string */
ft_printf("Hello World\n");
/* 出力: Hello World\n, ret: 12 */

/* 変換のみの format string */
ft_printf("%d%d%d", 1, 2, 3);
/* 出力: 123, ret: 3 */
```

---

## 8. evaluator が使うかもしれないテスターへの対策

### 8.1 主要なテスターと対策

**Tripouille/printfTester:**

このテスターは以下のカテゴリでテストします:
- 各変換の基本テスト
- NULL テスト
- 境界値テスト
- 戻り値テスト

**対策: 特に注意すべきテストケース:**

```c
/* NULL 文字列 */
ft_printf("%s", NULL);  /* "(null)" */

/* NULL ポインタ */
ft_printf("%p", NULL);  /* "(nil)" */

/* INT_MIN */
ft_printf("%d", -2147483648);  /* "-2147483648" */

/* UINT_MAX */
ft_printf("%u", 4294967295u);  /* "4294967295" */

/* 空の format */
ft_printf("");  /* ret = 0 */
```

### 8.2 テスター実行前のチェックリスト

テスターを実行する前に、以下を手動で確認してください:

```
[ ] make が成功する
[ ] make を2回実行して relink しない
[ ] norminette がパスする
[ ] 基本テスト（各変換1つずつ）が通る
[ ] NULL テスト（%s NULL, %p NULL, ft_printf(NULL)）が通る
[ ] INT_MIN テストが通る
[ ] 戻り値が printf と一致する
```

---

## 9. 評価で高得点を取るためのアドバイス

### 9.1 コードの美しさ

評価者はコードの品質も評価します。以下の点に注意してください:

**一貫性のある命名規則:**
- public 関数: `ft_print_xxx`
- static 関数: `ft_put_xxx`, `ft_xxxlen`
- 変数: `count`, `ret`, `len`, `c`

**一貫性のあるエラー処理パターン:**
```c
/* 全関数で同じパターンを使う */
ret = ft_something();
if (ret == -1)
    return (-1);
count += ret;
```

**適切なコメント（Norm 準拠）:**
```c
/* INT_MIN は -n がオーバーフローするため特別処理 */
if (n == -2147483648)
```

### 9.2 defense の態度

- 自分のコードを理解していることを示す
- 「なぜ」そう実装したかを説明できる
- 設計のトレードオフを議論できる
- わからないことは正直に言い、推論を示す

### 9.3 よくある減点ポイント

| 減点項目 | 対策 |
|---------|------|
| Norm 違反 | `norminette` で事前確認 |
| relink | `make` 2回実行で確認 |
| segfault | NULL チェック + edge case テスト |
| 戻り値の不一致 | printf と網羅的に比較 |
| write エラー未処理 | 全 write の戻り値をチェック |
| libtool の使用 | ar のみ使用 |
| 禁止関数の使用 | write, va_start/arg/end/copy, malloc, free のみ |

---

## 10. 評価の流れ（タイムライン）

### 10.1 典型的な評価セッションの進行

```
0:00 - 0:05  挨拶、プロジェクトの概要説明
0:05 - 0:15  Makefile のチェック（make, make clean, make fclean, make re, relink テスト）
0:15 - 0:20  norminette の実行
0:20 - 0:40  機能テスト（各変換指定子 + edge case）
0:40 - 0:55  defense（口頭試問）
0:55 - 1:00  フィードバック、評価
```

### 10.2 defense でよく聞かれる質問のまとめ

頻出度順:

1. **variadic functions の仕組みを説明してください**（ほぼ確実に聞かれる）
2. **INT_MIN はどう処理していますか？**（高頻度）
3. **write が失敗した場合はどうなりますか？**（高頻度）
4. **なぜ malloc を使わないのですか？**（中頻度）
5. **この設計の弱点は何ですか？**（中頻度）
6. **2の補数とは何ですか？**（evaluator 依存）
7. **static library と dynamic library の違いは？**（evaluator 依存）
8. **Makefile の relink を防ぐには？**（evaluator 依存）

### 10.3 defense で差をつけるテクニック

**コードを見ながら説明する:**
- 抽象的な説明だけでなく、実際のコードの該当箇所を示す
- 「ここで INT_MIN をチェックして、この行で直接文字列を出力しています」

**設計の代替案を示す:**
- 「if-else の代わりに関数ポインタテーブルも考えましたが、va_arg の型が異なるため if-else を選びました」

**CS の知識を織り交ぜる:**
- 「va_list は x86-64 ABI ではレジスタ保存領域へのポインタを内部に持っています」
- 「write は system call なので、呼び出しのたびにユーザーモードからカーネルモードへのコンテキストスイッチが発生します」
