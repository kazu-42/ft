# 07 - 学習ガイド

このセクションでは、ft_printf の実装を段階的に進めるためのロードマップ、重要概念の深掘り、デバッグ技法、テスト戦略、そして実践演習問題を提供します。

---

## 1. 8ステップ学習パス

### Step 1: variadic functions（可変長引数）を理解する

**目標:** `<stdarg.h>` の `va_list`, `va_start`, `va_arg`, `va_end` を正しく使えるようになる

#### 背景知識

C 言語の関数は通常、引数の数と型が固定です。しかし `printf` のように「任意の数の引数」を受け取る関数が必要な場面があります。これを実現するのが variadic functions（可変長引数関数）です。

```c
/* 固定引数の関数 */
int add_two(int a, int b);          /* 常に2つの int */

/* 可変長引数の関数 */
int ft_printf(const char *fmt, ...); /* ... は「任意の数の引数」を表す */
```

#### 4つのマクロの役割

```
va_list:  可変長引数を走査するためのイテレータ（型）
va_start: イテレータを初期化する（最後の固定引数を基点に）
va_arg:   次の引数を取得してイテレータを進める
va_end:   イテレータのクリーンアップ

使用順序:
  va_list args;           // (1) 宣言
  va_start(args, last);   // (2) 初期化（last = 最後の固定引数）
  val = va_arg(args, T);  // (3) 型 T として次の引数を取得（何回でも呼べる）
  va_end(args);           // (4) クリーンアップ
```

#### 練習コード

```c
#include <stdarg.h>
#include <stdio.h>

/* 練習1: 可変個の int を合計する関数 */
int my_sum(int count, ...)
{
    va_list args;
    int     total;
    int     i;

    total = 0;
    va_start(args, count);
    i = 0;
    while (i < count)
    {
        total += va_arg(args, int);
        i++;
    }
    va_end(args);
    return (total);
}

/* 練習2: 型が混在する可変長引数 */
void my_print(const char *types, ...)
{
    va_list args;

    va_start(args, types);
    while (*types)
    {
        if (*types == 'i')
            printf("int: %d\n", va_arg(args, int));
        else if (*types == 's')
            printf("str: %s\n", va_arg(args, char *));
        else if (*types == 'f')
            printf("double: %f\n", va_arg(args, double));
        types++;
    }
    va_end(args);
}

int main(void)
{
    printf("sum = %d\n", my_sum(3, 10, 20, 30));  /* 60 */
    printf("sum = %d\n", my_sum(5, 1, 2, 3, 4, 5)); /* 15 */

    my_print("isf", 42, "hello", 3.14);
    /* 出力:
       int: 42
       str: hello
       double: 3.140000
    */
    return (0);
}
```

#### やってみよう演習

1. `my_max(int count, ...)` を作成: 可変個の int から最大値を返す関数
2. `my_concat(int count, ...)` を作成: 可変個の `char *` を連結して stdout に出力する関数
3. `va_arg` に間違った型（例: int の引数を `va_arg(args, long long)` で取得）を指定するとどうなるか実験する

---

### Step 2: write system call を使いこなす

**目標:** `write(2)` の使い方、エラーチェック、fd の概念を理解する

#### 背景知識

```c
#include <unistd.h>

ssize_t write(int fd, const void *buf, size_t count);

/*
  fd:    ファイルディスクリプタ（出力先）
  buf:   書き込むデータへのポインタ
  count: 書き込むバイト数
  戻り値: 書き込んだバイト数（成功）、-1（失敗）
*/
```

```
ファイルディスクリプタ（fd）:
  0 = stdin  (標準入力)   ← キーボードからの入力
  1 = stdout (標準出力)   ← 画面への出力（ft_printf の出力先）
  2 = stderr (標準エラー) ← エラーメッセージ用
```

#### 練習コード

```c
#include <unistd.h>

/* 1文字出力 */
void my_putchar(char c)
{
    write(1, &c, 1);
}

/* 文字列出力 */
void my_putstr(char *str)
{
    int len;

    len = 0;
    while (str[len])
        len++;
    write(1, str, len);
}

/* エラーチェック付き1文字出力 */
int my_putchar_safe(char c)
{
    if (write(1, &c, 1) == -1)
        return (-1);
    return (1);
}

int main(void)
{
    my_putchar('A');
    my_putchar('\n');
    my_putstr("Hello, World!\n");

    /* stderr への出力（debug 用） */
    write(2, "DEBUG: this goes to stderr\n", 27);

    return (0);
}
```

#### やってみよう演習

1. `my_putchar_fd(char c, int fd)` を作成: 任意の fd に1文字出力
2. `close(1)` した後に `write(1, "A", 1)` を呼び、戻り値が -1 であることを確認
3. `write(1, "Hello", 5)` と `write(1, "Hello", 3)` の出力の違いを確認

---

### Step 3: 数値を文字列に変換して出力する

**目標:** 再帰を使って int を1桁ずつ文字に変換して出力する

#### 背景知識

```
数値 -> 文字の変換:
  '0' = 48 (ASCII)
  '1' = 49
  '2' = 50
  ...
  '9' = 57

  数字 n (0-9) を文字に変換: c = n + '0'
  例: 5 + '0' = 5 + 48 = 53 = '5'

  逆（文字 -> 数字）: n = c - '0'
  例: '5' - '0' = 53 - 48 = 5
```

#### 練習コード

```c
#include <unistd.h>

void my_putnbr(int n)
{
    char c;

    /* Step 3a: INT_MIN の特別処理 */
    if (n == -2147483648)
    {
        write(1, "-2147483648", 11);
        return ;
    }
    /* Step 3b: 負の数の処理 */
    if (n < 0)
    {
        write(1, "-", 1);
        n = -n;
    }
    /* Step 3c: 再帰で上位桁から出力 */
    if (n >= 10)
        my_putnbr(n / 10);
    /* Step 3d: 最下位桁を文字に変換して出力 */
    c = (n % 10) + '0';
    write(1, &c, 1);
}

int main(void)
{
    my_putnbr(0);       write(1, "\n", 1); /* "0" */
    my_putnbr(42);      write(1, "\n", 1); /* "42" */
    my_putnbr(-42);     write(1, "\n", 1); /* "-42" */
    my_putnbr(2147483647);  write(1, "\n", 1); /* "2147483647" */
    my_putnbr(-2147483648); write(1, "\n", 1); /* "-2147483648" */
    return (0);
}
```

#### やってみよう演習

1. `my_putnbr(12345)` の再帰展開を紙に書き出す（コールスタックを描く）
2. INT_MIN の特別処理を削除して `my_putnbr(-2147483648)` を実行し、何が起きるか確認
3. `my_putnbr_unsigned(unsigned int n)` を作成（INT_MIN処理と負数処理が不要）
4. 再帰を使わずにバッファ方式で `my_putnbr_iter(int n)` を作成し、動作を比較

---

### Step 4: 16進数変換を実装する

**目標:** 10進数を16進数に変換する仕組みを理解し、lookup table を使った出力を実装する

#### 背景知識

```
16進数（hexadecimal）:
  10進数: 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
  16進数: 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f

変換アルゴリズム:
  255 を16進数に変換:
    255 / 16 = 15 ... 余り 15 -> hex[15] = 'f'
    15 / 16 = 0  ... 余り 15 -> hex[15] = 'f'
    → "ff"

  42 を16進数に変換:
    42 / 16 = 2  ... 余り 10 -> hex[10] = 'a'
    2 / 16 = 0   ... 余り 2  -> hex[2]  = '2'
    → "2a"
```

#### 練習コード

```c
#include <unistd.h>

void my_puthex(unsigned int n)
{
    char *hex;

    hex = "0123456789abcdef";
    if (n >= 16)
        my_puthex(n / 16);
    write(1, &hex[n % 16], 1);
}

/* 大文字版 */
void my_puthex_upper(unsigned int n)
{
    char *hex;

    hex = "0123456789ABCDEF";
    if (n >= 16)
        my_puthex_upper(n / 16);
    write(1, &hex[n % 16], 1);
}

int main(void)
{
    my_puthex(0);        write(1, "\n", 1); /* "0" */
    my_puthex(255);      write(1, "\n", 1); /* "ff" */
    my_puthex(4294967295); write(1, "\n", 1); /* "ffffffff" */
    my_puthex_upper(255);  write(1, "\n", 1); /* "FF" */
    return (0);
}
```

#### やってみよう演習

1. `my_putptr(void *ptr)` を作成: "0x" prefix を付けてポインタを16進数出力
2. 手計算で 0xDEADBEEF を10進数に変換する
3. 手計算で 1000 を16進数に変換する（答え: 0x3E8）

---

### Step 5: ft_printf の骨格を作る

**目標:** 最小限の変換指定子（%c, %s）だけ対応する ft_printf を作り、全体構造を確立する

#### 進め方

```
Step 5a: format string を走査して通常文字を出力するだけの版

  int ft_printf(const char *format, ...)
  {
      while (*format)
      {
          write(1, format, 1);  /* 全文字をそのまま出力 */
          format++;
      }
      return (0);  /* 仮の戻り値 */
  }

  テスト: ft_printf("Hello World\n");
  期待: "Hello World\n" が出力される

Step 5b: '%' を検出して %c を処理する版

  int ft_printf(const char *format, ...)
  {
      va_list args;
      va_start(args, format);
      while (*format)
      {
          if (*format == '%')
          {
              format++;
              if (*format == 'c')
                  ft_print_char(va_arg(args, int));
          }
          else
              ft_print_char(*format);
          format++;
      }
      va_end(args);
      return (0);
  }

  テスト: ft_printf("char: %c\n", 'A');
  期待: "char: A\n" が出力される

Step 5c: %s を追加

  テスト: ft_printf("str: %s\n", "Hello");
  期待: "str: Hello\n" が出力される
```

#### やってみよう演習

1. Step 5a ~ 5c を順番に実装し、各段階でテストする
2. `%c` と `%s` だけの ft_printf で `ft_printf("%c%s%c\n", 'A', "BC", 'D')` が正しく動くことを確認
3. `ft_printf("%s", NULL)` で "(null)" が出力されるか確認

---

### Step 6: 戻り値を追加する

**目標:** 各出力関数に「出力した文字数」の戻り値を追加し、ft_printf で累積する

#### 進め方

```
変更前（戻り値なし）:
  void ft_print_char(int c)
  {
      write(1, &(char){c}, 1);
  }

変更後（戻り値あり）:
  int ft_print_char(int c)
  {
      unsigned char ch;

      ch = (unsigned char)c;
      if (write(1, &ch, 1) == -1)
          return (-1);
      return (1);
  }

ft_printf 内:
  変更前: ft_print_char(*format);
  変更後: count += ft_print_char(*format);
```

```
テスト方法:
  int ret;

  ret = ft_printf("Hello\n");
  printf("ft_printf returned: %d\n", ret);
  /* 期待: 6（'H','e','l','l','o','\n'） */

  ret = ft_printf("%c", 'A');
  printf("ft_printf returned: %d\n", ret);
  /* 期待: 1 */

  ret = ft_printf("%s", "Hello");
  printf("ft_printf returned: %d\n", ret);
  /* 期待: 5 */
```

#### やってみよう演習

1. 全ての出力関数に戻り値を追加し、`printf` の戻り値と比較する
2. `ft_printf("")` の戻り値が 0 であることを確認
3. `ft_printf("%c", '\0')` の戻り値が 1 であることを確認

---

### Step 7: エラー処理を追加する

**目標:** `write` のエラーチェックと `-1` の伝播を全関数に追加する

#### 進め方

```
全ての write 呼び出しにエラーチェックを追加:

  変更前:
    write(1, &ch, 1);
    return (1);

  変更後:
    if (write(1, &ch, 1) == -1)
        return (-1);
    return (1);

全ての関数呼び出しにエラーチェックを追加:

  変更前:
    count += ft_print_char(*format);

  変更後:
    ret = ft_print_char(*format);
    if (ret == -1)
        return (-1);
    count += ret;
```

#### やってみよう演習

1. `close(1)` してから `ft_printf("test")` を呼び、戻り値が -1 であることを確認
2. エラーチェックを1箇所だけ漏らした場合にどうなるか考える
3. 再帰関数（ft_put_nbr 等）でのエラー伝播が正しく動くことをテストする

---

### Step 8: テストする

**目標:** 本物の `printf` と出力・戻り値を完全に一致させる

#### 手動テストの体系

```c
#include <stdio.h>
#include <limits.h>
#include "ft_printf.h"

/* テストマクロ（便利ツール） */
/* 注意: このマクロは提出コードには含めない */
#define TEST(fmt, ...) do { \
    int r1 = printf(fmt, ##__VA_ARGS__); \
    int r2 = ft_printf(fmt, ##__VA_ARGS__); \
    if (r1 != r2) \
        printf("MISMATCH: printf=%d, ft_printf=%d\n", r1, r2); \
} while(0)

int main(void)
{
    /* %c のテスト */
    TEST("char: %c\n", 'A');
    TEST("char: %c\n", '\0');
    TEST("char: %c\n", 127);

    /* %s のテスト */
    TEST("str: %s\n", "Hello");
    TEST("str: %s\n", "");
    TEST("str: %s\n", (char *)NULL);

    /* %d のテスト */
    TEST("int: %d\n", 0);
    TEST("int: %d\n", -1);
    TEST("int: %d\n", 42);
    TEST("int: %d\n", INT_MAX);
    TEST("int: %d\n", INT_MIN);

    /* %u のテスト */
    TEST("uint: %u\n", 0);
    TEST("uint: %u\n", (unsigned int)-1);  /* UINT_MAX */

    /* %x, %X のテスト */
    TEST("hex: %x\n", 0);
    TEST("hex: %x\n", 255);
    TEST("HEX: %X\n", 255);
    TEST("hex: %x\n", UINT_MAX);

    /* %p のテスト */
    int x = 42;
    TEST("ptr: %p\n", (void *)&x);
    TEST("ptr: %p\n", (void *)NULL);

    /* %% のテスト */
    TEST("100%%\n");
    TEST("%%%%\n");

    /* 組み合わせテスト */
    TEST("%d %s %c\n", 42, "hello", '!');

    return (0);
}
```

#### やってみよう演習

1. 上記のテストコードを実行し、全て PASS することを確認
2. 自分で追加のテストケースを10個考えて追加する
3. テスターを使って自動テストを実行する（次セクション参照）

---

## 2. 重要な概念

### 2.1 Default Argument Promotion（デフォルト引数昇格）

variadic function に渡される引数は自動的に promote されます。

```
昇格の規則:
  char   -> int       （整数昇格）
  short  -> int       （整数昇格）
  float  -> double    （浮動小数点昇格）
  int    -> int       （変化なし）
  unsigned int -> unsigned int（変化なし）
  pointer -> pointer  （変化なし）

重要な結果:
  va_arg(args, char)   → 不正！（undefined behavior）
  va_arg(args, int)    → 正しい（char は int に昇格済み）

  va_arg(args, float)  → 不正！（undefined behavior）
  va_arg(args, double) → 正しい（float は double に昇格済み）
```

### 2.2 INT_MIN の特殊性

```
2の補数表現（two's complement）:

  正の数の表現（例: +5）:
    0000 0101 = +5

  負の数の表現（例: -5）:
    ビット反転: 1111 1010
    +1:         1111 1011 = -5

  32ビット int の範囲:
    INT_MAX =  2147483647 = 0111 1111 1111 1111 1111 1111 1111 1111
    INT_MIN = -2147483648 = 1000 0000 0000 0000 0000 0000 0000 0000

  非対称性:
    |INT_MIN| = 2147483648
    |INT_MAX| = 2147483647
    → |INT_MIN| > |INT_MAX| → INT_MIN の絶対値は int に収まらない！

  -INT_MIN の問題:
    int n = -2147483648;
    n = -n;
    → -(-2147483648) = +2147483648
    → int の最大値は 2147483647
    → オーバーフロー → undefined behavior！
```

### 2.3 unsigned と signed の関係

```c
/* -1 を unsigned int に cast */
unsigned int u = (unsigned int)(-1);
/* u = 4294967295 (0xFFFFFFFF) */

/* これは2の補数表現がそのまま unsigned として解釈されるため */
/* -1 のビットパターン: 11111111 11111111 11111111 11111111 */
/* unsigned として解釈: 4294967295 */

/* ft_printf での応用 */
ft_printf("%u", -1);   /* 出力: "4294967295" */
ft_printf("%x", -1);   /* 出力: "ffffffff" */

/* これは printf と同じ動作 */
/* %u や %x は unsigned int として引数を受け取るため、 */
/* -1 のビットパターンがそのまま unsigned として解釈される */
```

### 2.4 NULL pointer と NULL string の違い

```c
/* NULL string (%s) */
char *str = NULL;
ft_printf("%s", str);  /* "(null)" を出力 */
/* ft_print_str 内で NULL チェックされ、"(null)" に置き換わる */

/* NULL pointer (%p) */
void *ptr = NULL;
ft_printf("%p", ptr);  /* "(nil)" を出力 */
/* ft_print_ptr 内で ptr == 0 をチェックし、"(nil)" を出力 */

/* 空文字列は NULL ではない！ */
char *empty = "";
ft_printf("%s", empty);  /* "" を出力（何も出力しない、戻り値は 0） */
/* empty は有効なポインタ（'\0' だけの文字列を指している） */
```

---

## 3. Debugging Tips（デバッグ技法）

### 3.1 printf でデバッグしない

ft_printf を `printf` でデバッグすると、出力が混ざって分かりにくくなります。

```c
/* 悪い例: stdout が混ざる */
int ft_print_nbr(int n)
{
    printf("DEBUG: ft_print_nbr(%d) called\n", n);  /* これも stdout に出力 */
    /* ... ft_print_nbr の実装 ... */
}

/* 良い例: stderr を使う */
int ft_print_nbr(int n)
{
    write(2, "DEBUG: entering ft_print_nbr\n", 29);  /* stderr に出力 */
    /* ... ft_print_nbr の実装 ... */
}

/* デバッグ出力を分離する方法: */
/* 端末での実行: */
/*   ./test 2>/dev/null    → デバッグ出力を非表示 */
/*   ./test 2>debug.log    → デバッグ出力をファイルに保存 */
/*   ./test 1>/dev/null    → 通常出力を非表示（デバッグだけ見える） */
```

### 3.2 1つずつ変換を追加する

```
推奨順序:
  1. %c  → 最もシンプル（1文字出力するだけ）
  2. %%  → %c と同じ ft_print_char を使う
  3. %s  → 文字列出力（NULL チェックを忘れずに）
  4. %d  → 再帰による数値出力の基本
  5. %i  → %d と同じ処理
  6. %u  → %d から負数処理を除いたもの
  7. %x  → 16進数出力（小文字）
  8. %X  → %x と同じ処理（大文字）
  9. %p  → 16進数 + "0x" prefix + unsigned long long

各ステップで:
  1. 関数を実装
  2. printf と出力を比較
  3. 戻り値を比較
  4. 境界値テスト
  5. 次の変換に進む
```

### 3.3 戻り値の確認方法

```c
/* テストコード */
int ret1, ret2;

/* printf と ft_printf を交互に呼んで比較 */
ret1 = printf("test: %d\n", 42);
ret2 = ft_printf("test: %d\n", 42);

/* 出力が混ざるので、改行で区切る */
printf("\n--- Results ---\n");
printf("printf ret: %d\n", ret1);
printf("ft_printf ret: %d\n", ret2);

if (ret1 != ret2)
    printf("MISMATCH!\n");
else
    printf("OK\n");
```

### 3.4 Boundary Value Testing（境界値テスト）

```c
/* 各変換の境界値を網羅的にテスト */

/* %d 境界値 */
ft_printf("%d", 0);           /* "0" */
ft_printf("%d", -1);          /* "-1" */
ft_printf("%d", 1);           /* "1" */
ft_printf("%d", 2147483647);  /* "2147483647" (INT_MAX) */
ft_printf("%d", -2147483648); /* "-2147483648" (INT_MIN) */
ft_printf("%d", 10);          /* "10" (2桁の最小値) */
ft_printf("%d", -10);         /* "-10" */

/* %u 境界値 */
ft_printf("%u", 0);           /* "0" */
ft_printf("%u", 4294967295);  /* "4294967295" (UINT_MAX) */

/* %x 境界値 */
ft_printf("%x", 0);           /* "0" */
ft_printf("%x", 15);          /* "f" (1桁の最大値) */
ft_printf("%x", 16);          /* "10" (2桁の最小値) */
ft_printf("%x", 4294967295);  /* "ffffffff" */

/* %s 境界値 */
ft_printf("%s", "");          /* "" (空文字列) */
ft_printf("%s", (char *)NULL); /* "(null)" */

/* %p 境界値 */
ft_printf("%p", (void *)NULL); /* "(nil)" */
ft_printf("%p", (void *)1);   /* "0x1" */

/* %c 境界値 */
ft_printf("%c", 0);           /* '\0' (見えないが1文字出力) */
ft_printf("%c", 127);         /* DEL文字 */

/* %% */
ft_printf("%%");              /* "%" */
ft_printf("%%%%");            /* "%%" */
```

### 3.5 Valgrind によるメモリチェック

```bash
# ft_printf は malloc を使わないが、念のためチェック
cc -Wall -Wextra -Werror -g main.c -L. -lftprintf -o test
valgrind --leak-check=full ./test

# 期待される出力:
# All heap blocks were freed -- no leaks are possible
# ERROR SUMMARY: 0 errors from 0 contexts
```

---

## 4. Makefile デバッグ法

### 4.1 よくある Makefile の問題と解決法

**問題1: relink する**

```bash
$ make
cc -Wall -Wextra -Werror -c ft_printf.c -o ft_printf.o
# ... 全ファイルコンパイル ...
ar rcs libftprintf.a *.o

$ make
cc -Wall -Wextra -Werror -c ft_printf.c -o ft_printf.o  # なぜ再コンパイル!?
# ... relink している!

原因: パターンルールの依存関係が正しくない
  悪い例: %.o: %.c
  良い例: %.o: %.c ft_printf.h  ← ヘッダファイルも依存に含める

確認方法:
  $ make
  $ make
  # 2回目の make で何も出力されなければ OK
  # "make: 'libftprintf.a' is up to date." と出るのが正しい
```

**問題2: ヘッダ変更が反映されない**

```bash
# ft_printf.h を変更したのに再コンパイルされない

原因: %.o ルールの依存に ft_printf.h が含まれていない
  悪い例: %.o: %.c
  良い例: %.o: %.c ft_printf.h

修正後の確認:
  $ make
  $ touch ft_printf.h   # ヘッダのタイムスタンプを更新
  $ make                 # 全 .c が再コンパイルされるはず
```

**問題3: bonus ターゲットの relink**

```bash
# "make bonus" が毎回再リンクする

原因: bonus ターゲットの依存関係が不適切

  悪い例:
    bonus:
        $(CC) $(CFLAGS) ...   # 毎回実行される

  良い例:
    bonus: all                 # all と同じ（mandatory only の場合）
    # もしくは
    bonus: $(BONUS_OBJS)
        ar rcs $(NAME) $(BONUS_OBJS)
```

### 4.2 Makefile のデバッグコマンド

```bash
# make の動作を詳細表示
make -n              # 実行するコマンドを表示（実行はしない）
make -n | head -20   # 最初の20行だけ表示

# 変数の値を確認
make -p | grep SRCS  # SRCS 変数の値を表示

# 依存関係の確認
make --debug=b       # 基本的なデバッグ情報

# クリーンビルド
make fclean && make  # 全て再ビルド

# relink チェック
make && make         # 2回目で何も出力されなければ OK
```

---

## 5. テスターの使い方

### 5.1 Tripouille/printfTester

最も人気のあるテスターです。

```bash
# インストール
cd ~
git clone https://github.com/Tripouille/printfTester.git
cd printfTester

# ft_printf のパスを設定（Makefile を確認）
# テスターの Makefile 内で PRINTF_PATH を自分のプロジェクトのパスに変更
# または、テスターのディレクトリに libftprintf.a をコピー

# 実行
make m    # mandatory テスト
make b    # bonus テスト
make a    # all テスト

# 出力の読み方
# [OK] → テスト通過
# [KO] → テスト失敗（出力または戻り値の不一致）
# [CRASH] → segfault 等のクラッシュ
```

### 5.2 ft_printf_tester（paulo-music のテスター）

```bash
# インストール
cd ~
git clone https://github.com/paulo-music/ft_printf_tester.git
cd ft_printf_tester

# 実行前に、自分の ft_printf を make しておく
cd ~/ft_printf && make

# テスト実行
cd ~/ft_printf_tester
bash test.sh ../ft_printf

# テスト結果の確認
# OK / KO で表示される
```

### 5.3 テスターの出力が理解できないとき

```bash
# テスターが KO を出した場合の調査手順:

# 1. どのテストケースが失敗したか特定
#    テスターの出力に表示される（例: "Test 42: %d with INT_MIN"）

# 2. そのテストケースを手動で実行
echo '
#include <stdio.h>
#include "ft_printf.h"
int main(void) {
    int r1, r2;
    r1 = printf("%d", -2147483648);
    printf(" (ret=%d)\n", r1);
    r2 = ft_printf("%d", -2147483648);
    printf(" (ret=%d)\n", r2);
    return (0);
}
' > debug_test.c
cc -Wall -Wextra -Werror debug_test.c -I ~/ft_printf -L ~/ft_printf -lftprintf -o debug_test
./debug_test

# 3. 出力と戻り値の差を確認
# 4. 該当する関数のコードを見直す
```

---

## 6. printfnbr tester の使い方

printfnbr tester は ft_printf の数値出力に特化したテスターです。特に `%d`, `%u`, `%x`, `%X` の境界値テストに優れています。

### 6.1 セットアップ

```bash
# GitHub で printfnbr tester を検索してクローン
cd ~
git clone <printfnbr_tester_url> printfnbr_tester
cd printfnbr_tester

# 自分の ft_printf をビルド
cd ~/ft_printf && make

# テスター実行
cd ~/printfnbr_tester
# テスターの README に従って実行
# 一般的には:
bash run_tests.sh ~/ft_printf
# または
make test PRINTF_DIR=~/ft_printf
```

### 6.2 テスト内容

```
printfnbr tester が検証する主な項目:

%d テスト:
  - 0, 1, -1
  - INT_MAX (2147483647)
  - INT_MIN (-2147483648)
  - 境界付近: INT_MAX-1, INT_MIN+1
  - 桁境界: 9, 10, 99, 100, 999, 1000, ...

%u テスト:
  - 0, 1
  - UINT_MAX (4294967295)
  - 符号変換: (unsigned int)-1 → UINT_MAX

%x/%X テスト:
  - 0, 1, 15, 16
  - 0xFF, 0x100
  - UINT_MAX (ffffffff)
  - 大文字/小文字の一貫性

戻り値テスト:
  - 各テストケースで printf と ft_printf の戻り値を比較
```

---

## 7. よくある間違い20選

### 間違い 1: format が NULL のチェック漏れ

```c
/* 間違い */
int ft_printf(const char *format, ...)
{
    va_start(args, format);
    while (*format)  /* format が NULL なら segfault! */

/* 正しい */
int ft_printf(const char *format, ...)
{
    if (!format)
        return (-1);
    va_start(args, format);
```

### 間違い 2: va_arg で間違った型を指定

```c
/* 間違い: char は int に promote されるので char で取得してはいけない */
va_arg(args, char);     /* undefined behavior! */

/* 正しい */
va_arg(args, int);      /* char は int に promote される */
```

### 間違い 3: %p で unsigned int を使う

```c
/* 間違い: 64ビット環境ではポインタが4バイトに収まらない */
ft_print_ptr(va_arg(args, unsigned int));

/* 正しい */
ft_print_ptr(va_arg(args, unsigned long long));
```

### 間違い 4: INT_MIN の特別処理を忘れる

```c
/* 間違い */
if (n < 0)
{
    write(1, "-", 1);
    n = -n;  /* n = -2147483648 の時 overflow! */
}

/* 正しい */
if (n == -2147483648)
{
    write(1, "-2147483648", 11);
    return (0);
}
if (n < 0)
{
    write(1, "-", 1);
    n = -n;
}
```

### 間違い 5: %s で NULL チェック漏れ

```c
/* 間違い */
int ft_print_str(char *str)
{
    int len = ft_strlen(str);  /* str が NULL なら segfault! */

/* 正しい */
int ft_print_str(char *str)
{
    if (!str)
        str = "(null)";
    int len = ft_strlen(str);
```

### 間違い 6: write のエラーチェック漏れ

```c
/* 間違い */
write(1, &ch, 1);
return (1);

/* 正しい */
if (write(1, &ch, 1) == -1)
    return (-1);
return (1);
```

### 間違い 7: %% で引数を消費する

```c
/* 間違い */
else if (specifier == '%')
    count = ft_print_char(va_arg(args, int));  /* 引数を不要に消費! */

/* 正しい */
else if (specifier == '%')
    count = ft_print_char('%');  /* va_arg を呼ばない */
```

### 間違い 8: ft_numlen で n == 0 の処理漏れ

```c
/* 間違い */
static int ft_numlen(int n)
{
    int len = 0;
    while (n != 0)  /* n == 0 のときループしない → len = 0 を返す */
    {
        len++;
        n /= 10;
    }
    return (len);  /* 0 は "0" で1桁なのに 0 を返してしまう */
}

/* 正しい */
static int ft_numlen(int n)
{
    int len = 0;
    if (n <= 0)
        len = 1;   /* 0 の場合は "0" で1桁、負の場合は "-" で1文字 */
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    return (len);
}
```

### 間違い 9: ft_print_char で int をそのまま write に渡す

```c
/* 間違い（ビッグエンディアン環境で問題） */
int ft_print_char(int c)
{
    write(1, &c, 1);  /* ビッグエンディアンでは上位バイト(0x00)が出力される */
    return (1);
}

/* 正しい */
int ft_print_char(int c)
{
    unsigned char ch = (unsigned char)c;
    if (write(1, &ch, 1) == -1)
        return (-1);
    return (1);
}
```

### 間違い 10: %p で NULL を "0x0" と出力する

```c
/* 間違い（Linux glibc では "(nil)" が正しい） */
int ft_print_ptr(unsigned long long ptr)
{
    write(1, "0x", 2);
    ft_put_ptr(ptr);  /* ptr == 0 なら "0x0" と出力される */
}

/* 正しい */
int ft_print_ptr(unsigned long long ptr)
{
    if (ptr == 0)
    {
        write(1, "(nil)", 5);
        return (5);
    }
    write(1, "0x", 2);
    /* ... */
}
```

### 間違い 11: static 関数にし忘れる

```c
/* 間違い: ヘルパー関数が外部に公開されてしまう */
int ft_strlen(char *str)  /* 他のファイルの ft_strlen と名前衝突! */

/* 正しい */
static int ft_strlen(char *str)  /* このファイル内でのみ使用 */
```

### 間違い 12: Makefile で .h の依存関係を忘れる

```makefile
# 間違い: ヘッダ変更時に再コンパイルされない
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 正しい
%.o: %.c ft_printf.h
	$(CC) $(CFLAGS) -c $< -o $@
```

### 間違い 13: format string の '%' の後の文字を読み飛ばす

```c
/* 間違い: format++ を2回実行していない */
if (*format == '%')
{
    ret = ft_print_format(args, *(format + 1));
    /* format++ をここでしていない! */
}
format++;
/* format は '%' の次の specifier を指している → 次のループで specifier が通常文字として出力される */

/* 正しい */
if (*format == '%')
{
    format++;  /* specifier に進む */
    ret = ft_print_format(args, *format);
}
format++;  /* 次の文字に進む */
```

### 間違い 14: count を初期化し忘れる

```c
/* 間違い */
int ft_printf(const char *format, ...)
{
    int count;  /* 未初期化! ゴミ値が入っている */
    /* ... */
    count += ret;  /* ゴミ値 + ret = 不正な値 */

/* 正しい */
int ft_printf(const char *format, ...)
{
    int count;
    count = 0;  /* 必ず初期化 */
```

### 間違い 15: ft_put_hex で n % 16 と n / 16 を間違える

```c
/* 間違い: 上位桁と下位桁が逆 */
if (n >= 16)
    ft_put_hex(n % 16, uppercase);  /* これは下位桁! */
write(1, &hex[n / 16], 1);          /* これは上位桁! */

/* 正しい */
if (n >= 16)
    ft_put_hex(n / 16, uppercase);  /* 上位桁を先に再帰 */
write(1, &hex[n % 16], 1);          /* 下位桁（自分の桁）を出力 */
```

### 間違い 16: va_end の呼び忘れ（正常パス）

```c
/* 間違い: 正常パスで va_end を忘れている */
int ft_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    /* ... */
    return (count);  /* va_end がない! */
}

/* 正しい */
int ft_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    /* ... */
    va_end(args);
    return (count);
}
```

### 間違い 17: ft_print_ptr の count 計算ミス

```c
/* 間違い: "0x" の2文字を count に含め忘れる */
int ft_print_ptr(unsigned long long ptr)
{
    write(1, "0x", 2);
    ft_put_ptr(ptr);
    return (ft_ptr_len(ptr));  /* "0x" の2文字が含まれていない! */
}

/* 正しい */
int ft_print_ptr(unsigned long long ptr)
{
    write(1, "0x", 2);
    ft_put_ptr(ptr);
    return (2 + ft_ptr_len(ptr));  /* "0x"(2) + 16進数桁数 */
}
```

### 間違い 18: ft_print_format で未知の specifier を処理しない

```c
/* 危険: 未知の specifier で count が未定義 */
static int ft_print_format(va_list args, char specifier)
{
    if (specifier == 'c')
        return (ft_print_char(va_arg(args, int)));
    /* ... 他の specifier ... */
    /* 未知の specifier の場合、何も return しない! */
}

/* 安全: count を 0 で初期化して返す */
static int ft_print_format(va_list args, char specifier)
{
    int count;

    count = 0;
    if (specifier == 'c')
        count = ft_print_char(va_arg(args, int));
    /* ... */
    return (count);  /* 未知の specifier の場合は 0 を返す */
}
```

### 間違い 19: include guard を忘れる

```c
/* 間違い: 多重インクルード時にコンパイルエラー */
/* ft_printf.h */
#include <stdarg.h>
#include <unistd.h>
int ft_printf(const char *format, ...);

/* 正しい */
/* ft_printf.h */
#ifndef FT_PRINTF_H
# define FT_PRINTF_H

# include <stdarg.h>
# include <unistd.h>

int ft_printf(const char *format, ...);

#endif
```

### 間違い 20: Makefile で bonus ターゲットの relink

```makefile
# 間違い: make bonus が毎回全ファイルを再コンパイルする
bonus:
	$(CC) $(CFLAGS) -c $(SRCS)
	ar rcs $(NAME) $(OBJS)

# 正しい: mandatory only の場合、bonus は all と同じ
bonus: all
```

---

## 8. 学習チェックリスト

以下の項目を全てチェックできたら、ft_printf の実装は完了です。

```
基本実装:
  [ ] ft_printf が format string を正しく走査する
  [ ] %c で文字を正しく出力する
  [ ] %s で文字列を正しく出力する（NULL 対応含む）
  [ ] %p でポインタを正しく出力する（NULL = "(nil)" 対応含む）
  [ ] %d と %i で符号付き整数を正しく出力する（INT_MIN 含む）
  [ ] %u で符号なし整数を正しく出力する（UINT_MAX 含む）
  [ ] %x で小文字16進数を正しく出力する
  [ ] %X で大文字16進数を正しく出力する
  [ ] %% でリテラル '%' を出力する

戻り値:
  [ ] 各関数が正しい文字数を返す
  [ ] ft_printf の戻り値が printf と一致する
  [ ] エラー時に -1 を返す

エラー処理:
  [ ] format が NULL の場合に -1 を返す
  [ ] 全ての write にエラーチェックがある
  [ ] エラーが正しく伝播する

Norm:
  [ ] norminette でエラーがない
  [ ] 各ファイルが5関数以内
  [ ] 各関数が25行以内
  [ ] ヘルパー関数が static

Makefile:
  [ ] make で libftprintf.a が生成される
  [ ] make 2回目で relink しない
  [ ] make clean でオブジェクトファイルが削除される
  [ ] make fclean でライブラリも削除される
  [ ] make re でクリーンビルドされる

テスト:
  [ ] 全境界値テストが通る
  [ ] テスターで KO がない
  [ ] valgrind でエラーがない
```
