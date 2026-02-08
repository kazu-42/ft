# 02 - 前提知識

このセクションでは、ft_printf の実装に必要な前提知識を、実践的な観点から詳しく解説します。01-背景知識が「なぜ」を扱ったのに対し、このセクションは「どうやって」に焦点を当てています。

---

## 1. C 言語の型システム

### 1.1 メモリ上での型の表現

C 言語の各型は、メモリ上で特定のサイズ（バイト数）を占有します。ft_printf では複数の型を扱うため、各型のメモリレイアウトを正確に理解する必要があります。

#### 64ビット環境での型サイズ一覧

| 型 | サイズ | ビット数 | 最小値 | 最大値 |
|----|--------|---------|--------|--------|
| `char` | 1 byte | 8 bits | -128 | 127 |
| `unsigned char` | 1 byte | 8 bits | 0 | 255 |
| `short` | 2 bytes | 16 bits | -32,768 | 32,767 |
| `unsigned short` | 2 bytes | 16 bits | 0 | 65,535 |
| `int` | 4 bytes | 32 bits | -2,147,483,648 | 2,147,483,647 |
| `unsigned int` | 4 bytes | 32 bits | 0 | 4,294,967,295 |
| `long` | 8 bytes | 64 bits | -9.2 * 10^18 | 9.2 * 10^18 |
| `unsigned long` | 8 bytes | 64 bits | 0 | 1.8 * 10^19 |
| `long long` | 8 bytes | 64 bits | (long と同じ) | (long と同じ) |
| `void *` | 8 bytes | 64 bits | - | - |

> **注意**: `long` のサイズは環境依存です。Linux/macOS の64ビット環境では8バイトですが、Windows の64ビット環境では4バイトです（LLP64 モデル）。42の環境（Linux/macOS）では8バイトです。

#### メモリレイアウトの可視化

```
int n = 0x12345678; のメモリ上の配置（リトルエンディアン）:

アドレス:  0x100  0x101  0x102  0x103
内容:      0x78   0x56   0x34   0x12
           ^^^^                 ^^^^
           LSB                  MSB
           (最下位バイト)        (最上位バイト)
```

**リトルエンディアン（little-endian）とは:**

x86/x86-64 プロセッサでは、数値の最下位バイト（Least Significant Byte）が最も低いアドレスに格納されます。これをリトルエンディアンと呼びます。

```
ビッグエンディアン: 0x12 0x34 0x56 0x78  (人間が読む順)
リトルエンディアン: 0x78 0x56 0x34 0x12  (x86 の格納順)
```

ft_printf では通常エンディアンを意識する必要はありませんが、デバッグでメモリをダンプした際に混乱しないために知っておくと有用です。

### 1.2 Pointer（ポインタ）

ポインタはメモリアドレスを格納する変数です。ft_printf では以下の場面でポインタを使用します。

```c
/* format string の走査 */
const char *format;
while (*format)   /* ポインタが指す文字を読む */
{
    format++;     /* ポインタを次の文字に進める */
}

/* 文字列引数の処理 */
char *str = va_arg(args, char *);
/* str は文字列の先頭を指すポインタ */

/* ポインタ引数の出力 */
void *ptr = va_arg(args, void *);
/* ptr はメモリアドレスとして数値化して出力する */
```

#### ポインタとアドレス演算

```c
int x = 42;
int *p = &x;    /* p は x のアドレスを指す */

/* メモリの様子: */
/*   x: [42]          @ アドレス 0x7ffd0100 */
/*   p: [0x7ffd0100]  @ アドレス 0x7ffd0108 */
```

**`void *` 型の特殊性:**

`void *` は「型のないポインタ」で、任意のポインタ型から/への暗黙の変換が可能です。

```c
int x = 42;
void *ptr = &x;           /* int * -> void * は暗黙に変換可能 */
int *ip = (int *)ptr;     /* void * -> int * はキャスト推奨 */
```

`%p` では `void *` を受け取り、そのアドレス値を16進数で出力します。

### 1.3 文字列操作

C 言語の文字列は **null-terminated（ヌル終端）** の `char` 配列です。

```c
char *str = "hello";
/* メモリ上: ['h']['e']['l']['l']['o']['\0'] */
/*           str はここ↑を指す               */
```

#### ft_strlen の実装

ft_printf 内で文字列の長さを計算するために、自前の `ft_strlen` が必要です（標準の `strlen` は使用禁止）。

```c
static int	ft_strlen(char *str)
{
	int	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}
```

**動作の詳細:**

```
str = "hello"

反復 | len | str[len] | '\0'?
-----|-----|----------|------
  0  |  0  |   'h'   |  No
  1  |  1  |   'e'   |  No
  2  |  2  |   'l'   |  No
  3  |  3  |   'l'   |  No
  4  |  4  |   'o'   |  No
  5  |  5  |   '\0'  |  Yes -> ループ終了

return 5
```

#### NULL 文字列の危険性

```c
char *str = NULL;
int len = ft_strlen(str);  /* segfault! */
/* str[0] は *(NULL + 0) = *(0x0) */
/* アドレス 0x0 へのアクセスは不正 */
```

これが `%s` で NULL チェックが必要な理由です。

### 1.4 型変換（Type Casting）

ft_printf では複数の型変換が必要です。

#### 暗黙の型変換（Implicit Conversion）

```c
int n = 42;
unsigned int u = n;        /* int -> unsigned int（暗黙） */
/* n が正の場合、値は変わらない */

int n = -1;
unsigned int u = n;        /* int -> unsigned int（暗黙） */
/* u = 4294967295 (0xFFFFFFFF) */
/* ビットパターンは同じ、解釈が変わる */
```

#### 明示的な型変換（Explicit Cast）

```c
/* va_arg で int を受け取り、char として使う */
unsigned char ch = (unsigned char)va_arg(args, int);

/* void * を unsigned long long に変換 */
unsigned long long addr = (unsigned long long)ptr;

/* 数字を文字に変換 */
char digit = (char)((n % 10) + '0');
```

#### Default Argument Promotion の再確認

可変長引数では自動的に型が昇格されるため、`va_arg` で指定する型は昇格後の型でなければなりません。

```c
/* 正しい使い方 */
int c = va_arg(args, int);            /* char の代わりに int */
char *s = va_arg(args, char *);       /* ポインタは昇格しない */
int n = va_arg(args, int);            /* int は昇格しない */
unsigned int u = va_arg(args, unsigned int); /* 昇格しない */
double d = va_arg(args, double);      /* float の代わりに double */
```

```c
/* 間違った使い方（undefined behavior） */
char c = va_arg(args, char);          /* NG! */
short s = va_arg(args, short);        /* NG! */
float f = va_arg(args, float);        /* NG! */
```

### 1.5 やってみよう: 型サイズの確認

```c
#include <stdio.h>

int	main(void)
{
	printf("char:               %zu bytes\n", sizeof(char));
	printf("int:                %zu bytes\n", sizeof(int));
	printf("unsigned int:       %zu bytes\n", sizeof(unsigned int));
	printf("long:               %zu bytes\n", sizeof(long));
	printf("long long:          %zu bytes\n", sizeof(long long));
	printf("unsigned long long: %zu bytes\n", sizeof(unsigned long long));
	printf("void *:             %zu bytes\n", sizeof(void *));
	return (0);
}
```

コンパイルして実行し、自分の環境での各型のサイズを確認してみましょう。

---

## 2. stdarg.h の深掘り

### 2.1 stdarg.h と varargs.h の歴史

`<stdarg.h>` は ANSI C（C89）で導入されたヘッダです。それ以前は `<varargs.h>` という非標準のヘッダが使われていました。

```c
/* 旧式: varargs.h（K&R C 時代）*/
#include <varargs.h>
int old_printf(va_alist)  /* 固定引数がない! */
    va_dcl
{
    va_list ap;
    va_start(ap);         /* 引数なし */
    /* ... */
}

/* 現代: stdarg.h（ANSI C 以降）*/
#include <stdarg.h>
int new_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);    /* 最後の固定引数を指定 */
    /* ... */
}
```

`<stdarg.h>` では最低1つの固定引数が必要です。これは `va_start` が固定引数のアドレスを起点にして可変長引数の位置を計算するためです。

### 2.2 各マクロの内部実装（概念的）

実際の `<stdarg.h>` の実装はコンパイラに依存しますが、概念的には以下のように動作します。

#### 古典的な実装（スタックベース、32ビット環境の場合）

```c
/* 概念的な実装（実際のヘッダとは異なる） */

typedef char *va_list;

#define va_start(ap, last) \
    (ap = (char *)&(last) + sizeof(last))
/*
 * ap を last の次のメモリ位置に設定する
 * last のアドレス + last のサイズ = 次の引数のアドレス
 */

#define va_arg(ap, type) \
    (*(type *)((ap += sizeof(type)) - sizeof(type)))
/*
 * 1. ap を type のサイズ分進める
 * 2. 進める前の位置から type サイズ分読み取る
 * 3. その値を返す
 */

#define va_end(ap) \
    (ap = (char *)0)
/*
 * ap を NULL にする（no-op の場合もある）
 */
```

**この実装が動く前提条件:**
- すべての引数がスタック上に連続して配置される
- 引数のアライメントが予測可能

#### 現代の実装（x86-64）

x86-64 では引数がレジスタとスタックに分散するため、上記の単純な実装は使えません。GCC/Clang では `va_list` はコンパイラの組み込み（builtin）として実装されています。

```c
/* GCC の実際の定義（概念的） */
typedef struct
{
    unsigned int    gp_offset;        /* 汎用レジスタオフセット */
    unsigned int    fp_offset;        /* 浮動小数点レジスタオフセット */
    void            *overflow_arg_area; /* スタック上の引数 */
    void            *reg_save_area;   /* レジスタ保存領域 */
}   __va_list_tag;

typedef __va_list_tag va_list[1];
```

### 2.3 va_list の使用パターン

ft_printf での典型的な使用パターンを見てみましょう。

```c
int	ft_printf(const char *format, ...)
{
	va_list	args;
	int		count;

	va_start(args, format);   /* (1) 初期化 */
	count = 0;
	while (*format)
	{
		if (*format == '%')
		{
			format++;
			/* (2) 変換指定子に応じて va_arg で取得 */
			if (*format == 'd')
				count += ft_print_nbr(va_arg(args, int));
			else if (*format == 's')
				count += ft_print_str(va_arg(args, char *));
			/* ... */
		}
		format++;
	}
	va_end(args);            /* (3) クリーンアップ */
	return (count);
}
```

**重要な制約:**

1. `va_arg` は **順番にしか** 引数を取得できない（ランダムアクセス不可）
2. 取得する引数の数は **format string から判断する**（自動では分からない）
3. 取得する型を **間違えると undefined behavior**
4. `va_start` と `va_end` は **必ず対にする**

### 2.4 va_arg の型不一致: 具体的な危険性

va_arg で指定する型が実際の引数の型と一致しない場合、何が起きるのか具体的に見てみましょう。

**ケース1: int の引数を char * として読む**

```c
ft_printf("%s", 42);  /* int を %s で読もうとする */
```

```
va_arg(args, char *) の動作:
1. 現在位置から 8 バイト（char * のサイズ）を読む
2. 実際には int の 42 (0x0000002A) が格納されている
3. これをポインタとして解釈: アドレス 0x2A を文字列として読もうとする
4. アドレス 0x2A は通常アクセス不可 -> segfault!
```

**ケース2: unsigned int の引数を int として読む**

```c
ft_printf("%d", (unsigned int)3000000000u);
```

```
va_arg(args, int) の動作:
1. 現在位置から 4 バイトを読む
2. 3000000000 のビットパターン: 10110010 11010000 01001000 00000000
3. signed int として解釈: -1294967296
4. 出力: "-1294967296"（期待は "3000000000"）
```

> **教訓**: format string と引数の型を一致させることは、プログラマの責任です。コンパイラは `-Wformat` オプションでこの不一致を検出できますが、ft_printf ではコンパイラが format string を解析できないため、警告は出ません。

### 2.5 やってみよう: stdarg.h の実験

**演習1: 複数の型を受け取る関数を書いてみよう**

```c
#include <stdarg.h>
#include <unistd.h>

/* format string に従って引数を出力する簡易版 */
/* 'd' = int, 'c' = char, 's' = string */
void	my_print(const char *types, ...)
{
	va_list	args;

	va_start(args, types);
	while (*types)
	{
		if (*types == 'd')
		{
			/* int の出力（簡略化） */
			int n = va_arg(args, int);
			/* ... n を出力する処理 ... */
		}
		else if (*types == 'c')
		{
			char c = (char)va_arg(args, int);
			write(1, &c, 1);
		}
		else if (*types == 's')
		{
			char *s = va_arg(args, char *);
			/* ... s を出力する処理 ... */
		}
		types++;
	}
	va_end(args);
}
```

**演習2: va_copy の使い方を体験する**

```c
#include <stdarg.h>
#include <stdio.h>

/* 引数を2回走査する関数 */
void	print_twice(int count, ...)
{
	va_list	args;
	va_list	copy;
	int		i;

	va_start(args, count);
	va_copy(copy, args);  /* コピーを作成 */
	/* 1回目の走査 */
	i = 0;
	while (i < count)
	{
		printf("%d ", va_arg(args, int));
		i++;
	}
	printf("\n");
	/* 2回目の走査（コピーを使う） */
	i = 0;
	while (i < count)
	{
		printf("%d ", va_arg(copy, int));
		i++;
	}
	printf("\n");
	va_end(copy);  /* コピーも va_end が必要 */
	va_end(args);
}
```

---

## 3. Static Library の作成

### 3.1 コンパイルの全体の流れ

C のソースコードが実行ファイルになるまでの過程を詳しく見てみましょう。

```
ソースファイル (.c)
       |
  [前処理 (preprocessing)]  --- #include, #define の展開
       |
  前処理済みソース (.i)
       |
  [コンパイル (compilation)]  --- C -> アセンブリ言語
       |
  アセンブリファイル (.s)
       |
  [アセンブル (assembly)]  --- アセンブリ -> 機械語
       |
  オブジェクトファイル (.o)
       |
  [リンク (linking)]  --- 複数の .o を結合
       |
  実行ファイル (a.out / program)
```

ft_printf では、**コンパイル + アセンブル** まで行い（`-c` フラグ）、生成されたオブジェクトファイルを **ar コマンド** でライブラリにまとめます。

### 3.2 オブジェクトファイル（.o）の詳細

```bash
cc -Wall -Wextra -Werror -c ft_printf.c -o ft_printf.o
```

`-c` フラグは「リンクせずにオブジェクトファイルだけ生成する」ことを指示します。

**オブジェクトファイルの中身:**

| セクション | 内容 |
|-----------|------|
| .text | 機械語命令（コンパイルされた関数のコード） |
| .data | 初期化済みグローバル変数 |
| .bss | 未初期化グローバル変数 |
| .rodata | 読み取り専用データ（文字列リテラルなど） |
| シンボルテーブル | 関数名、変数名とそのアドレス |
| 再配置情報 | リンク時にアドレスを修正するための情報 |

**シンボルの確認:**

```bash
nm ft_printf.o
```

出力例:
```
                 U ft_print_char
                 U ft_print_hex
                 U ft_print_nbr
                 U ft_print_ptr
                 U ft_print_str
                 U ft_print_unsigned
0000000000000000 t ft_print_format
0000000000000080 T ft_printf
```

| 記号 | 意味 |
|------|------|
| `T` | テキスト（コード）セクションで定義されたグローバルシンボル |
| `t` | テキストセクションで定義されたローカルシンボル（static 関数） |
| `U` | 未定義シンボル（他のオブジェクトファイルで定義されている） |

`ft_print_format` が小文字の `t` なのは `static` 関数だからです。外部から呼び出せません。

### 3.3 ar コマンドの詳細な使い方

```bash
# ライブラリの作成
ar rcs libftprintf.a ft_printf.o ft_print_char.o ft_print_str.o \
    ft_print_ptr.o ft_print_nbr.o ft_print_unsigned.o ft_print_hex.o

# ライブラリの内容を確認
ar -t libftprintf.a
# 出力:
# ft_printf.o
# ft_print_char.o
# ft_print_str.o
# ft_print_ptr.o
# ft_print_nbr.o
# ft_print_unsigned.o
# ft_print_hex.o

# シンボルの確認
nm libftprintf.a
# 出力: 各 .o ファイルのシンボルが表示される
```

**ar コマンドのその他のオプション:**

| オプション | 意味 | 使用例 |
|-----------|------|--------|
| `t` | ファイル一覧を表示 | `ar -t lib.a` |
| `x` | ファイルを取り出す | `ar -x lib.a file.o` |
| `d` | ファイルを削除 | `ar -d lib.a file.o` |

### 3.4 ライブラリのリンク方法

```bash
# 方法1: -L と -l オプション
cc main.c -L. -lftprintf -o program

# 方法2: ライブラリファイルを直接指定
cc main.c libftprintf.a -o program

# 方法3: ヘッダのインクルードパスも指定
cc -I. main.c -L. -lftprintf -o program
```

| オプション | 意味 |
|-----------|------|
| `-I.` | ヘッダファイルの検索パスにカレントディレクトリを追加 |
| `-L.` | ライブラリの検索パスにカレントディレクトリを追加 |
| `-lftprintf` | `libftprintf.a` をリンク（`lib` と `.a` は自動補完） |

### 3.5 やってみよう: ライブラリ作成の実験

**演習: 2つの関数を含むライブラリを作ってみよう**

1. `my_putchar.c` を作成:

```c
#include <unistd.h>

void	my_putchar(char c)
{
	write(1, &c, 1);
}
```

2. `my_putstr.c` を作成:

```c
#include <unistd.h>

void	my_putstr(char *str)
{
	int	i;

	i = 0;
	while (str[i])
		i++;
	write(1, str, i);
}
```

3. コンパイルとライブラリ作成:

```bash
cc -Wall -Wextra -Werror -c my_putchar.c -o my_putchar.o
cc -Wall -Wextra -Werror -c my_putstr.c -o my_putstr.o
ar rcs libmy.a my_putchar.o my_putstr.o
```

4. 使ってみる:

```bash
# main.c を作成してリンク
cc main.c -L. -lmy -o test
./test
```

---

## 4. Makefile の詳細

### 4.1 Make の基本概念

`make` はビルド自動化ツールです。**依存関係グラフ**と**タイムスタンプ比較**に基づいて、必要最小限のコマンドだけを実行します。

#### Makefile のルール構造

```makefile
ターゲット: 依存ファイル1 依存ファイル2
	コマンド（タブでインデント）
```

**重要**: コマンドの前のインデントは**必ずタブ文字**でなければなりません。スペースではエラーになります。

#### 依存関係グラフ

ft_printf の Makefile は以下の依存関係グラフを表現しています。

```
libftprintf.a
  |
  +-- ft_printf.o
  |     +-- ft_printf.c
  |     +-- ft_printf.h
  |
  +-- ft_print_char.o
  |     +-- ft_print_char.c
  |     +-- ft_printf.h
  |
  +-- ft_print_str.o
  |     +-- ft_print_str.c
  |     +-- ft_printf.h
  |
  +-- ft_print_ptr.o
  |     +-- ft_print_ptr.c
  |     +-- ft_printf.h
  |
  +-- ft_print_nbr.o
  |     +-- ft_print_nbr.c
  |     +-- ft_printf.h
  |
  +-- ft_print_unsigned.o
  |     +-- ft_print_unsigned.c
  |     +-- ft_printf.h
  |
  +-- ft_print_hex.o
        +-- ft_print_hex.c
        +-- ft_printf.h
```

#### タイムスタンプ比較の仕組み

`make` は以下のアルゴリズムで「何を再ビルドすべきか」を判断します。

```
1. ターゲットファイルが存在しない
   -> コマンドを実行

2. ターゲットファイルが存在するが、
   いずれかの依存ファイルがターゲットより新しい
   -> コマンドを実行

3. ターゲットファイルが存在し、
   すべての依存ファイルがターゲットより古い
   -> 何もしない（「up to date」）
```

**具体例:**

```bash
make              # 初回: すべてのファイルをコンパイル
make              # 2回目: "Nothing to be done" (何も変更なし)
touch ft_printf.c # ft_printf.c のタイムスタンプを更新
make              # ft_printf.o と libftprintf.a だけ再ビルド
                  # 他の .o ファイルは再コンパイルされない
```

### 4.2 ft_printf の Makefile 解説

```makefile
NAME    = libftprintf.a

CC      = cc
CFLAGS  = -Wall -Wextra -Werror

SRCS    = ft_printf.c \
          ft_print_char.c \
          ft_print_str.c \
          ft_print_ptr.c \
          ft_print_nbr.c \
          ft_print_unsigned.c \
          ft_print_hex.c

OBJS    = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)

%.o: %.c ft_printf.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

bonus: all

.PHONY: all clean fclean re bonus
```

#### 変数定義の解説

| 変数 | 値 | 説明 |
|------|-----|------|
| `NAME` | `libftprintf.a` | 生成するライブラリの名前 |
| `CC` | `cc` | 使用するコンパイラ |
| `CFLAGS` | `-Wall -Wextra -Werror` | コンパイルフラグ |
| `SRCS` | ソースファイル一覧 | `.c` ファイルの列挙 |
| `OBJS` | `$(SRCS:.c=.o)` | `.c` を `.o` に置換した一覧 |

**`$(SRCS:.c=.o)` の動作:**

```
SRCS = ft_printf.c ft_print_char.c ft_print_str.c ...
OBJS = ft_printf.o ft_print_char.o ft_print_str.o ...
```

これは **置換参照（substitution reference）** と呼ばれる Make の機能です。

#### パターンルール（Pattern Rule）の詳細

```makefile
%.o: %.c ft_printf.h
	$(CC) $(CFLAGS) -c $< -o $@
```

これは「任意の `.o` ファイルは、同名の `.c` ファイルと `ft_printf.h` に依存する」というルールです。

**自動変数（Automatic Variables）:**

| 変数 | 意味 | 例 |
|------|------|-----|
| `$@` | ターゲットのファイル名 | `ft_printf.o` |
| `$<` | 最初の依存ファイル | `ft_printf.c` |
| `$^` | すべての依存ファイル | `ft_printf.c ft_printf.h` |
| `$?` | ターゲットより新しい依存ファイル | （変更されたファイル） |

**展開例:**

```bash
# ft_printf.o を作る場合:
cc -Wall -Wextra -Werror -c ft_printf.c -o ft_printf.o
#                           ^^^^^^^^^^^    ^^^^^^^^^^^^
#                              $<              $@
```

#### 各ルールの詳細

**`all` ルール:**

```makefile
all: $(NAME)
```

デフォルトターゲット。`make` を引数なしで実行すると、最初に定義されたターゲット（`all`）が実行されます。`$(NAME)` に依存しているので、ライブラリのビルドが走ります。

**`$(NAME)` ルール:**

```makefile
$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)
```

すべてのオブジェクトファイルが揃ったら、`ar` コマンドでライブラリを作成します。

**`clean` ルール:**

```makefile
clean:
	rm -f $(OBJS)
```

オブジェクトファイルを削除します。`-f` フラグにより、ファイルが存在しなくてもエラーにならません。

**`fclean` ルール:**

```makefile
fclean: clean
	rm -f $(NAME)
```

`clean` を先に実行してからライブラリファイルも削除します。`fclean` は `clean` に依存しているため、`clean` が先に実行されます。

**`re` ルール:**

```makefile
re: fclean all
```

`fclean` の後に `all` を実行します。完全な再ビルドです。

> **注意**: `re: fclean all` は、`fclean` と `all` が依存関係として列挙されているだけです。Make は依存関係を左から右に処理しますが、これは Make の実装に依存する動作です。確実に順序を保証したい場合は、`re:` のコマンドとして `$(MAKE) fclean && $(MAKE) all` と書く方法もあります。

**`bonus` ルール:**

```makefile
bonus: all
```

このプロジェクトでは bonus と mandatory が同じファイルなので、単に `all` に委任しています。

#### .PHONY の意味

```makefile
.PHONY: all clean fclean re bonus
```

`.PHONY` は「これらのターゲットはファイル名ではない」と Make に伝えます。

**なぜ必要か:**

もしカレントディレクトリに `clean` という名前のファイルが存在した場合:

```bash
touch clean    # "clean" というファイルを作成
make clean     # .PHONY がないと "clean is up to date" になる!
```

`clean` がファイル名として解釈され、「`clean` ファイルは最新なので何もしない」と判断されてしまいます。`.PHONY` を指定すると、Make は常にそのルールのコマンドを実行します。

### 4.3 Relink 問題

**Relink（リリンク）とは:**

ソースファイルに変更がないのに、`make` を実行するたびにライブラリが再構築されてしまう問題です。

**Relink が起きる条件:**

1. 依存関係が正しく設定されていない
2. 毎回ライブラリを作り直す記述になっている

**悪い例（Relink が起きる）:**

```makefile
# NG: OBJS が $(NAME) の依存関係に含まれていない
all:
	$(CC) $(CFLAGS) -c $(SRCS)
	ar rcs $(NAME) $(OBJS)
```

この場合、`make` を実行するたびにすべてのソースがコンパイルされ、ライブラリが再構築されます。

**良い例（Relink しない）:**

```makefile
# OK: 依存関係が正しい
$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)

%.o: %.c ft_printf.h
	$(CC) $(CFLAGS) -c $< -o $@
```

この場合:
1. 各 `.o` ファイルは対応する `.c` ファイルが変更されたときだけ再コンパイル
2. `$(NAME)` はいずれかの `.o` ファイルが変更されたときだけ再構築

**Relink の確認方法:**

```bash
make        # 初回ビルド
make        # 2回目: "make: Nothing to be done for 'all'." なら OK
            # 何かコマンドが実行されたら Relink している
```

### 4.4 Make の暗黙のルール（Implicit Rules）

Make には組み込みの暗黙のルールがあり、明示的に書かなくても `.c` から `.o` への変換を行えます。

```makefile
# 暗黙のルール（Make に組み込み済み）:
# %.o: %.c
#     $(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
```

ただし、ft_printf では `ft_printf.h` をヘッダの依存関係に含める必要があるため、パターンルールを明示的に書いています。

### 4.5 やってみよう: Makefile の実験

**演習1: Relink のテスト**

```bash
make
make        # "Nothing to be done" が出ることを確認
touch ft_printf.c
make        # ft_printf.o だけ再コンパイルされることを確認
touch ft_printf.h
make        # すべての .o が再コンパイルされることを確認
```

**演習2: 依存関係の可視化**

```bash
make -n     # 実行されるコマンドを表示（実行はしない）
make -d     # 依存関係の解析過程を詳細に表示
```

---

## 5. 基数変換の数学

### 5.1 位取り記数法（Positional Notation）

私たちが普段使う10進数は「位取り記数法」です。各桁の値に桁の重み（基数の累乗）を掛けて合計します。

**10進数の例:**

```
42 = 4 * 10^1 + 2 * 10^0
   = 4 * 10   + 2 * 1
   = 40 + 2
   = 42
```

**16進数の例:**

```
0x2A = 2 * 16^1 + A * 16^0
     = 2 * 16   + 10 * 1
     = 32 + 10
     = 42
```

**一般化:**

基数 B での数値 (d_n d_{n-1} ... d_1 d_0)_B は:

```
V = d_n * B^n + d_{n-1} * B^{n-1} + ... + d_1 * B + d_0
```

### 5.2 N進数から10進数への変換

各桁に基数の累乗を掛けて合計します。

**16進数 -> 10進数:**

```
0xFF = 15 * 16 + 15 = 240 + 15 = 255
0x100 = 1 * 256 + 0 * 16 + 0 = 256
0xDEAD = 13*4096 + 14*256 + 10*16 + 13 = 53248 + 3584 + 160 + 13 = 57005
```

### 5.3 10進数からN進数への変換

除算と剰余の繰り返しで変換します。

#### アルゴリズムの正しさの証明

**命題**: 非負整数 V を基数 B (B >= 2) で表現したとき、`V mod B` は最下位桁に等しい。

**証明**:

V を B 進数で表現すると:
```
V = d_n * B^n + d_{n-1} * B^{n-1} + ... + d_1 * B + d_0
```

ここで d_0 以外のすべての項は B の倍数（B^1 以上を因数に持つ）なので:
```
V mod B = (d_n * B^n + ... + d_1 * B + d_0) mod B
        = d_0 mod B      (他の項はBで割り切れるため)
        = d_0             (0 <= d_0 < B なので)
```

同様に、`V / B` （整数除算）は:
```
V / B = d_n * B^{n-1} + ... + d_1
```

これは V の上位桁を表す数値であり、同じアルゴリズムを再帰的に適用できます。

**終了条件**: V < B のとき、V 自身が最上位桁であり、アルゴリズムは終了します。

#### 詳細な変換例

**4294967295 (UINT_MAX) を16進数に変換:**

```
Step 1: 4294967295 / 16 = 268435455, 余り 15 -> f
Step 2:  268435455 / 16 =  16777215, 余り 15 -> f
Step 3:   16777215 / 16 =   1048575, 余り 15 -> f
Step 4:    1048575 / 16 =     65535, 余り 15 -> f
Step 5:      65535 / 16 =      4095, 余り 15 -> f
Step 6:       4095 / 16 =       255, 余り 15 -> f
Step 7:        255 / 16 =        15, 余り 15 -> f
Step 8:         15 / 16 =         0, 余り 15 -> f

結果: ffffffff

検算: 0xFFFFFFFF = 2^32 - 1 = 4294967295  OK!
```

### 5.4 再帰による実装パターン

基数変換の再帰実装には2つのバリエーションがあります。

**パターン A: 1回の再帰呼び出し（推奨）**

```c
void	ft_put_hex(unsigned int n)
{
	char	*hex;

	hex = "0123456789abcdef";
	if (n >= 16)
		ft_put_hex(n / 16);   /* 上位桁を先に出力 */
	write(1, &hex[n % 16], 1); /* 自分の桁を出力 */
}
```

**パターン B: 2回の再帰呼び出し（非推奨）**

```c
void	ft_put_hex_bad(unsigned int n)
{
	char	*hex;

	hex = "0123456789abcdef";
	if (n >= 16)
	{
		ft_put_hex_bad(n / 16);  /* 上位桁 */
		ft_put_hex_bad(n % 16);  /* 下位桁 */
	}
	else
		write(1, &hex[n], 1);
}
```

パターンBは動作はしますが、パターンAより非効率（再帰呼び出し回数が多い）で、コードも分かりにくくなります。パターンAを推奨します。

### 5.5 桁数の計算

基数変換の際、出力した桁数をカウントする必要があります。

```c
static int	ft_hexlen(unsigned int n)
{
	int	len;

	len = 0;
	if (n == 0)
		return (1);  /* 0 の桁数は 1 */
	while (n > 0)
	{
		len++;
		n /= 16;
	}
	return (len);
}
```

**各数値の桁数:**

| 10進数 | 16進数 | 桁数 |
|--------|--------|------|
| 0 | 0 | 1 |
| 15 | f | 1 |
| 16 | 10 | 2 |
| 255 | ff | 2 |
| 256 | 100 | 3 |
| 65535 | ffff | 4 |
| 4294967295 | ffffffff | 8 |

> **注意**: `n == 0` のときに特別処理が必要です。while ループは `n > 0` が条件なので、`n == 0` のときはループが1回も実行されず、`len = 0` が返されてしまいます。

### 5.6 やってみよう: 基数変換の手計算

**問題1**: 1234567890 を16進数に変換してください。

```
1234567890 / 16 = 77160493, 余り 2
  77160493 / 16 =  4822530, 余り 13 -> d
   4822530 / 16 =   301408, 余り 2
    301408 / 16 =    18838, 余り 0
     18838 / 16 =     1177, 余り 6
      1177 / 16 =       73, 余り 9
        73 / 16 =        4, 余り 9
         4 / 16 =        0, 余り 4

結果: 499602d2

検算: 4*16^7 + 9*16^6 + 9*16^5 + 6*16^4 + 0*16^3 + 2*16^2 + 13*16 + 2
    = 1073741824 + 150994944 + 9437184 + 393216 + 0 + 512 + 208 + 2
    = 1234567890  OK!
```

**問題2**: 再帰で10進数を出力する場合、`ft_put_nbr(12345)` のコールスタックを書き出してください。

```
ft_put_nbr(12345)
  12345 >= 10: ft_put_nbr(1234)
    1234 >= 10: ft_put_nbr(123)
      123 >= 10: ft_put_nbr(12)
        12 >= 10: ft_put_nbr(1)
          1 < 10: write '1'
        12 % 10 = 2: write '2'
      123 % 10 = 3: write '3'
    1234 % 10 = 4: write '4'
  12345 % 10 = 5: write '5'

出力: "12345"
```

---

## 6. まとめ: 前提知識のチェックリスト

ft_printf の実装を始める前に、以下の項目を確認してください。

### C 言語の基礎

- [ ] `int`, `unsigned int`, `char`, `void *` のサイズを答えられる
- [ ] ポインタの基本操作（宣言、参照、デリファレンス）ができる
- [ ] 文字列がヌル終端であることを理解している
- [ ] 暗黙の型変換と明示的キャストの違いを理解している
- [ ] default argument promotion を理解している

### stdarg.h

- [ ] `va_list`, `va_start`, `va_arg`, `va_end` の使い方を覚えている
- [ ] `va_arg` で指定する型が昇格後の型でなければならない理由を理解している
- [ ] 可変長引数を使った簡単な関数を書ける

### Static Library

- [ ] `.c` -> `.o` -> `.a` の流れを理解している
- [ ] `ar rcs` の各フラグの意味を答えられる
- [ ] `-L` と `-l` オプションの使い方を理解している

### Makefile

- [ ] ルールの構造（ターゲット: 依存ファイル / コマンド）を理解している
- [ ] `$@`, `$<` の意味を答えられる
- [ ] `.PHONY` の目的を説明できる
- [ ] relink が起きる条件と防ぎ方を理解している
- [ ] `make clean`, `make fclean`, `make re` の違いを説明できる

### 基数変換

- [ ] 10進数から16進数への手動変換ができる
- [ ] 再帰で数値を出力する仕組みを理解している
- [ ] 桁数の計算方法を理解している
- [ ] `n == 0` の場合の特別処理の必要性を理解している
