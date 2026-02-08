# 03 - 要件定義

このセクションでは、ft_printf プロジェクトの要件を正確かつ詳細に定義します。「何を作るのか」を曖昧さなく理解することが、実装の第一歩です。

---

## 1. 変換テーブル

### 1.1 Mandatory Part の変換指定子

| 変換指定子 | 引数の型 | va_arg での取得型 | 出力形式 | 出力例 |
|-----------|---------|-----------------|---------|--------|
| `%c` | `char` | `int` | 1文字 | `'A'` -> `A` |
| `%s` | `char *` | `char *` | 文字列 | `"hello"` -> `hello` |
| `%p` | `void *` | `unsigned long long` | `0x` + 16進小文字（NULLは`(nil)`） | `0x7fff5fbff8ac` |
| `%d` | `int` | `int` | 10進符号付き整数 | `-42` -> `-42` |
| `%i` | `int` | `int` | 10進符号付き整数 | `42` -> `42` |
| `%u` | `unsigned int` | `unsigned int` | 10進符号なし整数 | `4294967295` |
| `%x` | `unsigned int` | `unsigned int` | 16進小文字 | `255` -> `ff` |
| `%X` | `unsigned int` | `unsigned int` | 16進大文字 | `255` -> `FF` |
| `%%` | なし | なし | literal `%` | `%%` -> `%` |

#### 各変換指定子の設計意図

**%c - 文字出力**

なぜ `va_arg(args, int)` で取得するのか: default argument promotion により、`char` は `int` に昇格されます。C 言語の可変長引数では `char` を直接取得できません。

```c
ft_printf("%c", 'A');    /* 'A' は int に昇格されて渡される */
ft_printf("%c", 65);     /* 65 = 'A' の ASCII コード */
ft_printf("%c", '\0');   /* NULL 文字も1文字として出力される */
```

**%s - 文字列出力**

NULL が渡された場合の動作が重要です。標準の printf は `(null)` と出力します。

```c
ft_printf("%s", "hello");   /* "hello" を出力 */
ft_printf("%s", "");         /* 何も出力しない（空文字列） */
ft_printf("%s", NULL);       /* "(null)" を出力 */
```

**%p - ポインタ出力**

なぜ `0x` prefix が付くのか: これはポインタ値が16進数であることを視覚的に明示するための慣例です。C 言語の16進数リテラル（`0xFF` 等）と同じ prefix を使うことで、一貫性を保っています。

```c
int x = 42;
ft_printf("%p", &x);      /* 例: 0x7ffd559ffc54 */
ft_printf("%p", NULL);     /* (nil) --- Linux の printf 準拠 */
```

> **プラットフォーム差異**: macOS の printf は NULL ポインタを `0x0` と出力しますが、Linux の printf は `(nil)` と出力します。42の環境に合わせて実装してください。この実装では `(nil)` を出力する設計を採用しています。

**%d / %i - 符号付き整数出力**

%d と %i は printf の mandatory part では同一の動作をします。歴史的に、`%d` は "decimal"（10進数）、`%i` は "integer"（整数）の略です。scanf では `%i` は8進数や16進数も受け付けるなど動作が異なりますが、printf では同じです。

```c
ft_printf("%d", 42);          /* "42" */
ft_printf("%i", 42);          /* "42"（%d と同じ） */
ft_printf("%d", -42);         /* "-42" */
ft_printf("%d", 0);           /* "0" */
ft_printf("%d", 2147483647);  /* "2147483647" (INT_MAX) */
ft_printf("%d", -2147483648); /* "-2147483648" (INT_MIN) -- 特別処理が必要 */
```

**%u - 符号なし整数出力**

符号なし整数は常に 0 以上です。負の数の処理が不要なため、`%d` より実装が単純です。

```c
ft_printf("%u", 0);            /* "0" */
ft_printf("%u", 42);           /* "42" */
ft_printf("%u", 4294967295u);  /* "4294967295" (UINT_MAX) */
```

**%x / %X - 16進数出力**

小文字と大文字の違いだけです。内部のアルゴリズムは同一で、lookup table を切り替えるだけです。

```c
ft_printf("%x", 255);     /* "ff" */
ft_printf("%X", 255);     /* "FF" */
ft_printf("%x", 0);       /* "0" */
ft_printf("%x", 16);      /* "10" */
ft_printf("%X", 4294967295u); /* "FFFFFFFF" */
```

**%% - パーセント記号の出力**

`%` は変換指定の開始文字として使われるため、literal の `%` を出力するには `%%` と書く必要があります。

```c
ft_printf("100%%");        /* "100%" */
ft_printf("%%%%");         /* "%%" */
ft_printf("a%%b%%c");      /* "a%b%c" */
```

### 1.2 Bonus Part の追加要素

| フラグ/機能 | 説明 | 例 |
|------------|------|-----|
| `-` | 左寄せ（フィールド幅内で） | `%-10d` で `42` -> `42        ` |
| `0` | ゼロ埋め（`-` と併用時は `-` が優先） | `%010d` で `42` -> `0000000042` |
| `.` | 精度指定 | `%.5d` で `42` -> `00042` |
| `#` | alternate form | `%#x` で `255` -> `0xff` |
| `+` | 正の数に `+` を付ける | `%+d` で `42` -> `+42` |
| ` ` | 正の数の前にスペース | `% d` で `42` -> ` 42` |
| width | 最小フィールド幅 | `%10d` で `42` -> `        42` |

---

## 2. プロトタイプと戻り値

### 2.1 関数プロトタイプ

```c
int ft_printf(const char *format, ...);
```

**引数:**
- `format`: 書式文字列。`const char *` 型（読み取り専用）。
- `...`: 可変長引数。format string に対応する数と型の引数。

### 2.2 戻り値の厳密な定義

**成功時**: 出力された **バイト数**（文字数）を返す。

```c
int ret;

ret = ft_printf("hello");        /* ret = 5 */
ret = ft_printf("");              /* ret = 0 */
ret = ft_printf("%c", 'A');      /* ret = 1 */
ret = ft_printf("%c", '\0');     /* ret = 1 (NULL文字も1バイト) */
ret = ft_printf("%s", "abc");    /* ret = 3 */
ret = ft_printf("%s", NULL);     /* ret = 6 ("(null)" = 6文字) */
ret = ft_printf("%d", 42);       /* ret = 2 ("42" = 2文字) */
ret = ft_printf("%d", -42);      /* ret = 3 ("-42" = 3文字、'-' 含む) */
ret = ft_printf("%d", 0);        /* ret = 1 ("0" = 1文字) */
ret = ft_printf("%%");           /* ret = 1 ("%" = 1文字) */
ret = ft_printf("%p", NULL);     /* ret = 5 ("(nil)" = 5文字) */
```

**エラー時**: `-1` を返す。

エラーが発生するケース:
- `format` が `NULL` の場合
- `write()` system call が失敗した場合

```c
ret = ft_printf(NULL);            /* ret = -1 */
/* write が EBADF 等で失敗した場合も ret = -1 */
```

### 2.3 戻り値の計算方法

各変換の出力文字数を個別に計算し、合計します。

| 変換 | 出力文字数の計算 |
|------|-----------------|
| 通常文字 | 常に 1 |
| `%c` | 常に 1（`'\0'` を含む） |
| `%s` | 文字列の長さ（NULL の場合は 6） |
| `%p` | NULL: 5（"(nil)"）、非NULL: 2 + 16進桁数（"0x" + hex digits） |
| `%d` / `%i` | 桁数 + 符号（負の場合は +1） |
| `%u` | 桁数 |
| `%x` / `%X` | 16進桁数 |
| `%%` | 常に 1 |

#### 戻り値の計算トレース例

```c
ft_printf("Name: %s, Age: %d, Hex: %x%%\n", "Bob", 25, 255);
```

```
要素          | 文字列      | 文字数 | 累積count
--------------|------------|--------|----------
"Name: "      | "Name: "   | 6      | 6
%s ("Bob")    | "Bob"      | 3      | 9
", Age: "     | ", Age: "  | 7      | 16
%d (25)       | "25"       | 2      | 18
", Hex: "     | ", Hex: "  | 7      | 25
%x (255)      | "ff"       | 2      | 27
%%            | "%"        | 1      | 28
"\n"          | "\n"       | 1      | 29

戻り値: 29
```

---

## 3. 本物の printf との詳細比較

### 3.1 基本テストケース

以下のすべてのケースで、ft_printf は printf と同じ出力・同じ戻り値を返す必要があります。

```c
/* 文字 */
printf("char: %c\n", 'A');          /* "char: A\n", ret=8 */
printf("char: %c\n", '0');          /* "char: 0\n", ret=8 */
printf("char: %c\n", ' ');          /* "char:  \n", ret=8 */
printf("null char: %c\n", '\0');    /* "null char: \0\n", ret=12 */

/* 文字列 */
printf("str: %s\n", "hello");       /* "str: hello\n", ret=11 */
printf("str: %s\n", "");            /* "str: \n", ret=6 */
printf("str: %s\n", NULL);          /* "str: (null)\n", ret=12 */

/* 整数 */
printf("int: %d\n", 0);             /* "int: 0\n", ret=7 */
printf("int: %d\n", -1);            /* "int: -1\n", ret=8 */
printf("int: %d\n", 42);            /* "int: 42\n", ret=8 */
printf("int: %i\n", 42);            /* "int: 42\n", ret=8 */
printf("int: %d\n", 2147483647);    /* "int: 2147483647\n", ret=16 */
printf("int: %d\n", -2147483648);   /* "int: -2147483648\n", ret=17 */

/* 符号なし整数 */
printf("uint: %u\n", 0);            /* "uint: 0\n", ret=8 */
printf("uint: %u\n", 42);           /* "uint: 42\n", ret=9 */
printf("uint: %u\n", 4294967295u);  /* "uint: 4294967295\n", ret=17 */

/* 16進数 */
printf("hex: %x\n", 0);             /* "hex: 0\n", ret=7 */
printf("hex: %x\n", 255);           /* "hex: ff\n", ret=8 */
printf("hex: %X\n", 255);           /* "hex: FF\n", ret=8 */
printf("hex: %x\n", 4294967295u);   /* "hex: ffffffff\n", ret=14 */
printf("hex: %X\n", 4294967295u);   /* "hex: FFFFFFFF\n", ret=14 */

/* パーセント */
printf("%%\n");                      /* "%\n", ret=2 */
printf("100%%\n");                   /* "100%\n", ret=5 */
printf("%%%%\n");                    /* "%%\n", ret=3 */

/* ポインタ */
int x;
printf("ptr: %p\n", &x);            /* "ptr: 0x...\n" */
printf("ptr: %p\n", NULL);          /* "ptr: (nil)\n" (Linux) */

/* 複合 */
printf("%d%s%x%c%%", 42, "abc", 255, '!');
/* "42abcff!%", ret=10 */

/* 空文字列 */
printf("");                          /* "", ret=0 */
```

### 3.2 Edge Case の詳細

#### NULL 文字列（%s に NULL）

```c
ft_printf("%s", NULL);
/* 期待出力: (null) */
/* 期待戻り値: 6 */
```

なぜ `(null)` なのか: C 標準では `%s` に NULL を渡した場合の動作は undefined behavior ですが、glibc（Linux の C ライブラリ）は慣例的に `(null)` を出力します。多くの42の evaluator はこの動作を期待します。

#### NULL ポインタ（%p に NULL）

```c
ft_printf("%p", NULL);
/* Linux: "(nil)", 戻り値 5 */
/* macOS: "0x0", 戻り値 3 */
```

42の環境に応じて実装してください。この実装では `(nil)` を出力します。

#### INT_MIN（%d / %i に -2147483648）

```c
ft_printf("%d", -2147483648);
/* 期待出力: -2147483648 */
/* 期待戻り値: 11 */
```

`-INT_MIN` がオーバーフローするため、特別な処理が必要です（詳細は 01-背景知識 参照）。

#### UINT_MAX（%u に最大値）

```c
ft_printf("%u", 4294967295u);
/* 期待出力: 4294967295 */
/* 期待戻り値: 10 */
```

#### ゼロの出力

```c
ft_printf("%d", 0);   /* "0", ret = 1 */
ft_printf("%u", 0);   /* "0", ret = 1 */
ft_printf("%x", 0);   /* "0", ret = 1 */
ft_printf("%X", 0);   /* "0", ret = 1 */
```

> **よくあるバグ**: 桁数計算関数で `n == 0` の場合を特別処理し忘れると、何も出力されないか、桁数が 0 として返されます。

#### NULL 文字の出力（%c に '\0'）

```c
int ret = ft_printf("%c", '\0');
/* '\0' が出力される（端末には見えない） */
/* ret = 1 */
```

NULL 文字はバイト値 `0x00` であり、れっきとした1バイトのデータです。端末には何も表示されませんが、確かに1バイト出力されています。

#### 連続した %%

```c
ft_printf("%%%%");
/* 出力: "%%" */
/* 戻り値: 2 */
/* 解析: %% -> '%', %% -> '%' */

ft_printf("%%%%%%");
/* 出力: "%%%" */
/* 戻り値: 3 */
```

#### 空の format string

```c
ft_printf("");
/* 出力: （なし） */
/* 戻り値: 0 */
```

#### format が NULL

```c
ft_printf(NULL);
/* 出力: なし */
/* 戻り値: -1 */
/* segfault してはいけない */
```

---

## 4. Compilation 要件

### 4.1 コンパイラとフラグ

```bash
cc -Wall -Wextra -Werror
```

| フラグ | 説明 | 具体例 |
|--------|------|--------|
| `-Wall` | 一般的な警告をすべて有効化 | 未使用変数、暗黙の関数宣言など |
| `-Wextra` | 追加の警告を有効化 | 未使用パラメータ、符号の比較など |
| `-Werror` | すべての警告をエラーとして扱う | 警告があるとコンパイル失敗 |

**`-Werror` が厳しい理由:**

通常なら無視できる軽微な警告も、エラーとして扱われます。

```c
/* -Wextra による警告（=> エラー） */
int	ft_func(int unused_param)  /* 未使用パラメータ */
{
    return (0);
}

/* 対処法: (void) キャストで「意図的に未使用」と明示 */
int	ft_func(int unused_param)
{
    (void)unused_param;
    return (0);
}
```

### 4.2 ライブラリの作成方法

```bash
# OK: ar コマンドを使用
ar rcs libftprintf.a *.o

# NG: libtool は禁止
libtool -static -o libftprintf.a *.o  /* 使用禁止! */
```

**なぜ libtool が禁止なのか:**

`libtool` は macOS の Xcode に含まれるツールで、`ar` のラッパーです。42の subject では環境依存のツールの使用を避け、POSIX 標準の `ar` コマンドのみを使用することが要求されています。

### 4.3 ヘッダファイル

```c
/* ft_printf.h */
#ifndef FT_PRINTF_H
# define FT_PRINTF_H

# include <stdarg.h>
# include <unistd.h>

int	ft_printf(const char *format, ...);
int	ft_print_char(int c);
int	ft_print_str(char *str);
int	ft_print_ptr(unsigned long long ptr);
int	ft_print_nbr(int n);
int	ft_print_unsigned(unsigned int n);
int	ft_print_hex(unsigned int n, int uppercase);

#endif
```

**include guard が必要な理由:**

ヘッダが複数回インクルードされた場合に、重複定義エラーを防ぎます。

```c
/* include guard がない場合 */
#include "ft_printf.h"
#include "ft_printf.h"  /* 2回目: 重複定義エラー! */

/* include guard がある場合 */
#include "ft_printf.h"  /* FT_PRINTF_H が define される */
#include "ft_printf.h"  /* FT_PRINTF_H が既に定義済み -> スキップ */
```

---

## 5. 提出ファイル構成

```
ft_printf/
  Makefile              -- ビルドスクリプト
  ft_printf.h           -- ヘッダファイル
  ft_printf.c           -- メイン関数 + ディスパッチャ
  ft_print_char.c       -- %c, %% の出力
  ft_print_str.c        -- %s の出力（ft_strlen 含む）
  ft_print_ptr.c        -- %p の出力
  ft_print_nbr.c        -- %d, %i の出力
  ft_print_unsigned.c   -- %u の出力
  ft_print_hex.c        -- %x, %X の出力
```

### ファイル構成の根拠

| ファイル | 関数数 | 理由 |
|---------|--------|------|
| `ft_printf.c` | 2 | ft_printf + ft_print_format（static） |
| `ft_print_char.c` | 1 | ft_print_char のみ |
| `ft_print_str.c` | 2 | ft_strlen（static）+ ft_print_str |
| `ft_print_ptr.c` | 3 | ft_ptr_len + ft_put_ptr（static）+ ft_print_ptr |
| `ft_print_nbr.c` | 3 | ft_numlen + ft_put_nbr（static）+ ft_print_nbr |
| `ft_print_unsigned.c` | 3 | ft_unumlen + ft_put_unsigned（static）+ ft_print_unsigned |
| `ft_print_hex.c` | 3 | ft_hexlen + ft_put_hex（static）+ ft_print_hex |

42 Norm の「1ファイル5関数以内」の制限を自然に満たしています。

---

## 6. 禁止事項

### 6.1 使用禁止の関数

subject で許可されている関数は以下のみです。

| 許可関数 | ヘッダ | 用途 |
|---------|--------|------|
| `malloc` | `<stdlib.h>` | 動的メモリ確保（mandatory では不要） |
| `free` | `<stdlib.h>` | メモリ解放（mandatory では不要） |
| `write` | `<unistd.h>` | 文字の出力 |
| `va_start` | `<stdarg.h>` | 可変長引数の初期化 |
| `va_arg` | `<stdarg.h>` | 引数の取得 |
| `va_copy` | `<stdarg.h>` | va_list のコピー |
| `va_end` | `<stdarg.h>` | 可変長引数のクリーンアップ |

**使用禁止の関数の例:**

| 禁止関数 | 理由 | 代替手段 |
|---------|------|---------|
| `printf` | 当然ながら禁止（自分で実装するため） | ft_printf 自体 |
| `sprintf`, `snprintf` | printf 系関数は全て禁止 | 自前実装 |
| `itoa` | 数値->文字列変換は自前で実装 | 再帰 or バッファ |
| `strlen` | 文字列長の計算は自前で実装 | `ft_strlen` |
| `putchar` | stdio.h の関数は禁止 | `write(1, &c, 1)` |
| `puts`, `fputs` | stdio.h の関数は禁止 | `write(1, str, len)` |

### 6.2 禁止パターン

| 禁止事項 | 理由 |
|---------|------|
| `libtool` の使用 | `ar` コマンドのみ使用可 |
| グローバル変数 | Norm 違反 |
| `for` 文 | Norm 違反（`while` を使用） |
| 25行を超える関数 | Norm 違反 |
| 1ファイル5関数超 | Norm 違反 |
| 80文字を超える行 | Norm 違反 |

---

## 7. 本物の printf との網羅的な比較表

### 7.1 正常系テスト

| # | 入力 | 期待出力 | 期待戻り値 | 備考 |
|---|------|---------|-----------|------|
| 1 | `ft_printf("abc")` | `abc` | `3` | 通常文字のみ |
| 2 | `ft_printf("")` | （なし） | `0` | 空文字列 |
| 3 | `ft_printf("%c", 'Z')` | `Z` | `1` | 文字 |
| 4 | `ft_printf("%c", 0)` | `\0` | `1` | NULL文字 |
| 5 | `ft_printf("%s", "hi")` | `hi` | `2` | 文字列 |
| 6 | `ft_printf("%s", "")` | （なし） | `0` | 空文字列 |
| 7 | `ft_printf("%d", 42)` | `42` | `2` | 正の整数 |
| 8 | `ft_printf("%d", -42)` | `-42` | `3` | 負の整数 |
| 9 | `ft_printf("%d", 0)` | `0` | `1` | ゼロ |
| 10 | `ft_printf("%i", 42)` | `42` | `2` | %d と同じ |
| 11 | `ft_printf("%u", 42)` | `42` | `2` | 符号なし |
| 12 | `ft_printf("%u", 0)` | `0` | `1` | ゼロ |
| 13 | `ft_printf("%x", 255)` | `ff` | `2` | 16進小文字 |
| 14 | `ft_printf("%X", 255)` | `FF` | `2` | 16進大文字 |
| 15 | `ft_printf("%x", 0)` | `0` | `1` | ゼロ |
| 16 | `ft_printf("%%")` | `%` | `1` | パーセント |

### 7.2 境界値テスト

| # | 入力 | 期待出力 | 期待戻り値 | 備考 |
|---|------|---------|-----------|------|
| 17 | `ft_printf("%d", 2147483647)` | `2147483647` | `10` | INT_MAX |
| 18 | `ft_printf("%d", -2147483648)` | `-2147483648` | `11` | INT_MIN |
| 19 | `ft_printf("%u", 4294967295u)` | `4294967295` | `10` | UINT_MAX |
| 20 | `ft_printf("%x", 4294967295u)` | `ffffffff` | `8` | UINT_MAX hex |
| 21 | `ft_printf("%X", 4294967295u)` | `FFFFFFFF` | `8` | UINT_MAX HEX |
| 22 | `ft_printf("%s", NULL)` | `(null)` | `6` | NULL文字列 |
| 23 | `ft_printf("%p", NULL)` | `(nil)` | `5` | NULLポインタ (Linux) |

### 7.3 複合テスト

| # | 入力 | 期待出力 | 期待戻り値 |
|---|------|---------|-----------|
| 24 | `ft_printf("%d%d", 1, 2)` | `12` | `2` |
| 25 | `ft_printf("%s%s", "ab", "cd")` | `abcd` | `4` |
| 26 | `ft_printf("a%cb%sc", 'X', "YZ")` | `aXbYZc` | `7` |
| 27 | `ft_printf("%%%%")` | `%%` | `2` |
| 28 | `ft_printf("%%%%%%")` | `%%%` | `3` |
| 29 | `ft_printf("%d%s%x%c%%", 42, "abc", 255, '!')` | `42abcff!%` | `10` |

### 7.4 エラー系テスト

| # | 入力 | 期待戻り値 | 備考 |
|---|------|-----------|------|
| 30 | `ft_printf(NULL)` | `-1` | format が NULL |

---

## 8. やってみよう: 要件の確認演習

### 演習1: 戻り値の手計算

以下の各呼び出しについて、出力と戻り値を手計算してください。

```c
/* (a) */
ft_printf("Hello, %s! You scored %d/%d (%u%%)\n",
    "Alice", 95, 100, 95);

/* (b) */
ft_printf("%p %p %p\n", NULL, (void *)0x42, (void *)0xDEAD);

/* (c) */
ft_printf("%x + %X = %d\n", 10, 10, 20);

/* (d) */
ft_printf("%c%c%c%c%c", 72, 101, 108, 108, 111);
```

**回答:**

**(a)**
```
出力: "Hello, Alice! You scored 95/100 (95%)\n"
内訳: "Hello, "(7) + "Alice"(5) + "! You scored "(13) + "95"(2) +
      "/"(1) + "100"(3) + " ("(2) + "95"(2) + "%"(1) + ")\n"(2) = 38
戻り値: 38
```

**(b)**
```
出力: "(nil) 0x42 0xdead\n"
内訳: "(nil)"(5) + " "(1) + "0x42"(4) + " "(1) + "0xdead"(6) + "\n"(1) = 18
戻り値: 18
```

**(c)**
```
出力: "a + A = 20\n"
内訳: "a"(1) + " + "(3) + "A"(1) + " = "(3) + "20"(2) + "\n"(1) = 11
戻り値: 11
```

**(d)**
```
出力: "Hello"
(72='H', 101='e', 108='l', 108='l', 111='o')
戻り値: 5
```

### 演習2: バグのあるテストケースの作成

以下の条件を満たすテストケースを作成してください。

1. INT_MIN を使って戻り値が正しいか確認するテスト
2. NULL 文字列と NULL ポインタを同時にテストするテスト
3. %% が3つ以上連続するテスト
4. 1回の呼び出しで全変換指定子を使うテスト

---

## 9. まとめ: 要件チェックリスト

実装前に以下を確認してください。

### 変換指定子
- [ ] `%c` : 1文字出力（`'\0'` 含む）
- [ ] `%s` : 文字列出力（NULL -> "(null)"）
- [ ] `%p` : ポインタ出力（"0x" + hex、NULL -> "(nil)"）
- [ ] `%d` : 符号付き10進数（INT_MIN 対応）
- [ ] `%i` : %d と同じ動作
- [ ] `%u` : 符号なし10進数（UINT_MAX 対応）
- [ ] `%x` : 16進小文字
- [ ] `%X` : 16進大文字
- [ ] `%%` : literal %

### 戻り値
- [ ] 成功時: 出力文字数
- [ ] `'\0'` は 1 文字としてカウント
- [ ] エラー時: -1
- [ ] format が NULL の場合: -1

### コンパイル
- [ ] `cc -Wall -Wextra -Werror` でエラーなし
- [ ] `ar rcs` でライブラリ作成（libtool 不使用）
- [ ] Makefile の全ルール動作確認
- [ ] relink しないことを確認

### Norm
- [ ] `norminette` で全ファイルエラーなし
- [ ] 各関数 25 行以内
- [ ] 各ファイル 5 関数以内
- [ ] 各行 80 文字以内

---

## 10. 詳細な動作仕様: 各変換指定子

### 10.1 %c の完全仕様

**入力**: `va_arg(args, int)` で取得した `int` 値

**処理**:
1. `int` を `unsigned char` にキャスト（下位8ビットを取り出す）
2. `write(1, &ch, 1)` で1バイト出力

**出力文字数**: 常に `1`

**特殊ケース**:
- `'\0'`（NULL文字）: 出力される。端末には見えないが、1バイトとして出力される
- `127`（DEL文字）: 出力される。端末での表示は環境依存
- 負の値: `unsigned char` にキャストされるため、0-255 の範囲に正規化される

```c
ft_printf("%c", 'A');     /* 出力: 'A', ret=1 */
ft_printf("%c", 0);       /* 出力: '\0', ret=1 */
ft_printf("%c", 256);     /* 出力: '\0' (256 % 256 = 0), ret=1 */
ft_printf("%c", -1);      /* 出力: 0xFF (255), ret=1 */
```

### 10.2 %s の完全仕様

**入力**: `va_arg(args, char *)` で取得した文字列ポインタ

**処理**:
1. NULL チェック。NULL の場合は `"(null)"` に置き換え
2. `ft_strlen` で文字列長を計算
3. `write(1, str, len)` で文字列全体を出力

**出力文字数**: 文字列の長さ（NULL の場合は `6`、空文字列の場合は `0`）

**特殊ケース**:
- `NULL`: `"(null)"` を出力（6文字）
- `""`: 何も出力しない（0文字）
- 文字列中に `'\0'` がある場合: `'\0'` の手前までが出力される（C文字列の仕様通り）

### 10.3 %p の完全仕様

**入力**: `va_arg(args, unsigned long long)` で取得したアドレス値

**処理**:
1. NULL チェック（ptr == 0）。NULL の場合は `"(nil)"` を出力（Linux 準拠の場合）
2. `"0x"` prefix を出力
3. アドレス値を16進小文字で出力（再帰）

**出力文字数**: NULL の場合は `5`（"(nil)"）。非NULL の場合は `2 + 16進桁数`

**特殊ケース**:
- `NULL`: `"(nil)"`（Linux）または `"0x0"`（macOS）
- 小さいアドレス: 先頭の0は出力しない（例: `0x42`、`0x0` ではなく `0x42`）

### 10.4 %d / %i の完全仕様

**入力**: `va_arg(args, int)` で取得した符号付き整数

**処理**:
1. INT_MIN チェック。`-2147483648` の場合は文字列を直接出力
2. 負の場合は `'-'` を出力し、`n = -n` で正に変換
3. 再帰で上位桁から出力

**出力文字数**: 桁数 + 符号（負の場合は+1）

**桁数の一覧**:

| 値 | 出力 | 文字数 |
|-----|------|--------|
| 0 | "0" | 1 |
| 1 | "1" | 1 |
| -1 | "-1" | 2 |
| 42 | "42" | 2 |
| -42 | "-42" | 3 |
| 2147483647 | "2147483647" | 10 |
| -2147483648 | "-2147483648" | 11 |

### 10.5 %u の完全仕様

**入力**: `va_arg(args, unsigned int)` で取得した符号なし整数

**処理**:
1. 再帰で上位桁から出力（符号処理不要）

**出力文字数**: 桁数

| 値 | 出力 | 文字数 |
|-----|------|--------|
| 0 | "0" | 1 |
| 42 | "42" | 2 |
| 4294967295 | "4294967295" | 10 |

### 10.6 %x / %X の完全仕様

**入力**: `va_arg(args, unsigned int)` で取得した符号なし整数

**処理**:
1. 16で割り続け、余りを16進数文字に変換
2. 再帰で上位桁から出力
3. `%x` は小文字（"0123456789abcdef"）、`%X` は大文字（"0123456789ABCDEF"）

**出力文字数**: 16進桁数

| 10進値 | %x 出力 | %X 出力 | 文字数 |
|---------|---------|---------|--------|
| 0 | "0" | "0" | 1 |
| 10 | "a" | "A" | 1 |
| 15 | "f" | "F" | 1 |
| 16 | "10" | "10" | 2 |
| 255 | "ff" | "FF" | 2 |
| 4294967295 | "ffffffff" | "FFFFFFFF" | 8 |

### 10.7 %% の完全仕様

**入力**: なし（va_arg を呼ばない）

**処理**: `ft_print_char('%')` で `'%'` を1文字出力

**出力文字数**: 常に `1`

---

## 11. コンパイルフラグの詳細

### -Wall の主な警告

| 警告 | 説明 | ft_printf での発生例 |
|------|------|---------------------|
| -Wimplicit-function-declaration | 宣言なしで関数を使用 | ft_printf.h を include し忘れ |
| -Wunused-variable | 未使用の変数 | デバッグ用変数の消し忘れ |
| -Wreturn-type | return 文の欠如 | 非void関数で return を書き忘れ |
| -Wformat | format string と引数の不一致 | printf のデバッグコード |

### -Wextra の追加警告

| 警告 | 説明 | ft_printf での発生例 |
|------|------|---------------------|
| -Wunused-parameter | 未使用の引数 | 将来の拡張で使わない引数 |
| -Wsign-compare | signed/unsigned の比較 | int と unsigned int の比較 |

### -Werror が厳しい理由

すべての警告がコンパイルエラーになるため、「警告は無視して後で直す」ということができません。これは良いプログラミング習慣を強制する効果があります。

---

## 12. やってみよう: 要件確認の実践

### 実践1: 全変換の出力を printf と比較するテストプログラムを書く

以下のテンプレートを使って、全変換の出力と戻り値を printf と比較するテストプログラムを作成してください。

```c
#include <stdio.h>
#include <limits.h>
#include "ft_printf.h"

void	test(const char *name, int r1, int r2)
{
	if (r1 == r2)
		printf("[OK] %s: ret=%d\n", name, r1);
	else
		printf("[NG] %s: printf=%d, ft_printf=%d\n",
			name, r1, r2);
}

int	main(void)
{
	int	r1;
	int	r2;

	/* テストケースをここに追加 */
	r1 = printf("%d", 42);
	printf("\n");
	r2 = ft_printf("%d", 42);
	printf("\n");
	test("%d 42", r1, r2);

	return (0);
}
```

### 実践2: edge case のリストを作成する

以下の各変換指定子について、テストすべき edge case をリストアップしてください。

- %c: '\0', 127, 負の値
- %s: NULL, 空文字列, 長い文字列
- %d: 0, -1, INT_MAX, INT_MIN
- %u: 0, UINT_MAX
- %x/%X: 0, 16, UINT_MAX
- %p: NULL, 小さいアドレス, 大きいアドレス
- %%: 連続, 他の変換との組み合わせ
