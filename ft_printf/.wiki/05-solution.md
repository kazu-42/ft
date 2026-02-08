# 05 - 解法解説

このセクションでは、ft_printf の実装を関数ごとに詳細に解説し、ステップバイステップの実行トレースを通じて動作を完全に理解することを目指します。すべてのコードパス、エッジケース、内部メカニズムを網羅的に扱います。

---

## 1. アーキテクチャ概要

### 1.1 全体構造図

ft_printf は「format string parser + dispatcher + 個別出力関数群」という3層構造を持ちます。この構造は、標準ライブラリの `printf` の設計を簡略化したものです。

```
ft_printf(format, ...)
  |
  +-- NULL チェック (format == NULL なら -1 を返す)
  |
  +-- va_start(args, format)
  |
  +-- format string を1文字ずつ走査 (while ループ)
  |     |
  |     +-- '%' 以外の文字 (通常文字)
  |     |     +-- ft_print_char(*format) で標準出力(fd=1)に出力
  |     |     +-- 戻り値をチェック (-1 ならエラー伝播)
  |     |     +-- count に加算
  |     |
  |     +-- '%' を検出 (変換指定子の開始)
  |           +-- format++ で次の文字(specifier)を読む
  |           +-- ft_print_format(args, *format) で dispatch
  |           |     |
  |           |     +-- 'c' --> ft_print_char(va_arg(args, int))
  |           |     |           引数を int として取得し、1文字出力
  |           |     |
  |           |     +-- 's' --> ft_print_str(va_arg(args, char *))
  |           |     |           引数を char* として取得し、文字列出力
  |           |     |           NULL の場合は "(null)" を出力
  |           |     |
  |           |     +-- 'p' --> ft_print_ptr(va_arg(args, unsigned long long))
  |           |     |           引数を unsigned long long として取得
  |           |     |           "0x" prefix + 16進数で出力
  |           |     |           NULL(0) の場合は "(nil)" を出力
  |           |     |
  |           |     +-- 'd'/'i' --> ft_print_nbr(va_arg(args, int))
  |           |     |               引数を int として取得し、10進数出力
  |           |     |               負の数は '-' prefix 付き
  |           |     |               INT_MIN は特別処理
  |           |     |
  |           |     +-- 'u' --> ft_print_unsigned(va_arg(args, unsigned int))
  |           |     |           引数を unsigned int として取得
  |           |     |           符号なし10進数として出力
  |           |     |
  |           |     +-- 'x' --> ft_print_hex(va_arg(args, unsigned int), 0)
  |           |     |           引数を unsigned int として取得
  |           |     |           小文字16進数(abcdef)で出力
  |           |     |
  |           |     +-- 'X' --> ft_print_hex(va_arg(args, unsigned int), 1)
  |           |     |           引数を unsigned int として取得
  |           |     |           大文字16進数(ABCDEF)で出力
  |           |     |
  |           |     +-- '%' --> ft_print_char('%')
  |           |                 引数を消費せず、'%' 文字を出力
  |           |
  |           +-- 戻り値をチェック (-1 ならエラー伝播)
  |           +-- count に加算
  |
  +-- va_end(args)  -- 可変長引数のクリーンアップ
  +-- return (count) -- 出力した総文字数を返す
```

**この構造の意味:**

1. **Parser（解析器）**: `ft_printf` の while ループが format string を1文字ずつ解析し、通常文字と変換指定子を区別します
2. **Dispatcher（振り分け器）**: `ft_print_format` が変換指定子の文字に応じて適切な出力関数を呼び出します
3. **Handlers（処理関数群）**: `ft_print_char`, `ft_print_str` 等の各関数が実際の出力を担当します

この3層分離により、新しい変換指定子を追加する場合も dispatcher に1つの `else if` を追加し、対応する handler を作成するだけで済みます。

### 1.2 ファイル構成と責務

| ファイル | 関数 | 可視性 | 責務 | 戻り値の意味 |
|---------|------|--------|------|-------------|
| `ft_printf.c` | `ft_printf` | public | メインエントリポイント、format string の走査、戻り値の累積 | 総出力文字数 or -1 |
| | `ft_print_format` | static | 変換指定子に応じた dispatch、va_arg の型を決定 | 出力文字数 or -1 |
| `ft_print_char.c` | `ft_print_char` | public | 1文字出力（%c, %% で使用、通常文字の出力にも使用） | 常に 1 or -1 |
| `ft_print_str.c` | `ft_strlen` | static | 文字列長の計算（libft を使わない独立実装） | 文字列の長さ |
| | `ft_print_str` | public | 文字列出力（%s）、NULL 時は "(null)" を出力 | 出力文字数 or -1 |
| `ft_print_ptr.c` | `ft_ptr_len` | static | ポインタの16進桁数を計算 | 桁数 |
| | `ft_put_ptr` | static | ポインタを16進数で再帰出力（実際の write を行う） | 0(成功) or -1 |
| | `ft_print_ptr` | public | ポインタ出力（%p）、"0x" prefix 付き、NULL は "(nil)" | 出力文字数 or -1 |
| `ft_print_nbr.c` | `ft_numlen` | static | 符号付き整数の表示桁数を計算（'-' 符号含む） | 表示文字数 |
| | `ft_put_nbr` | static | 符号付き整数を再帰出力（INT_MIN 特別処理あり） | 0(成功) or -1 |
| | `ft_print_nbr` | public | 符号付き整数出力（%d, %i） | 出力文字数 or -1 |
| `ft_print_unsigned.c` | `ft_unumlen` | static | 符号なし整数の桁数を計算 | 桁数 |
| | `ft_put_unsigned` | static | 符号なし整数を再帰出力 | 0(成功) or -1 |
| | `ft_print_unsigned` | public | 符号なし整数出力（%u） | 出力文字数 or -1 |
| `ft_print_hex.c` | `ft_hexlen` | static | 16進数の桁数を計算 | 桁数 |
| | `ft_put_hex` | static | 16進数を再帰出力（大文字/小文字切替対応） | 0(成功) or -1 |
| | `ft_print_hex` | public | 16進数出力（%x, %X） | 出力文字数 or -1 |

**可視性（visibility）について:**

- **public**: ヘッダファイル `ft_printf.h` で宣言され、他のファイルから呼び出し可能
- **static**: そのファイル内でのみ使用可能。外部に公開する必要のないヘルパー関数は必ず `static` にする

`static` にする理由:
1. **名前空間の汚染防止**: 他のファイルの関数名と衝突しない
2. **カプセル化（encapsulation）**: 実装の詳細を隠蔽し、public インターフェースだけを公開
3. **コンパイラ最適化**: static 関数はコンパイラがインライン化しやすい
4. **Norm 準拠**: 42 の Norm では、外部に公開しない関数は static にすることが推奨される

### 1.3 関数の呼び出し関係（Call Graph）

```
ft_printf ──── ft_print_format ──── ft_print_char    [%c, %%]
          |          |
          |          +──── ft_print_str ──── ft_strlen    [%s]
          |          |
          |          +──── ft_print_ptr ──── ft_put_ptr (再帰)    [%p]
          |          |                  +──── ft_ptr_len
          |          |
          |          +──── ft_print_nbr ──── ft_put_nbr (再帰)    [%d, %i]
          |          |                  +──── ft_numlen
          |          |
          |          +──── ft_print_unsigned ── ft_put_unsigned (再帰)  [%u]
          |          |                     +── ft_unumlen
          |          |
          |          +──── ft_print_hex ──── ft_put_hex (再帰)    [%x, %X]
          |                             +──── ft_hexlen
          |
          +──── ft_print_char    [通常文字の出力]
```

**呼び出し関係のパターン:**

数値系の出力関数（nbr, unsigned, hex, ptr）はすべて同じ3関数パターンを持ちます:

```
ft_print_XXX(n)          -- public: 出力して文字数を返す
  +-- ft_put_XXX(n)      -- static: 再帰で実際に write する (0 or -1)
  +-- ft_XXXlen(n)       -- static: 表示文字数を計算する (桁数)
```

### 1.4 データフロー図

```
[呼び出し元]
     |
     | ft_printf("Test %d %s", -42, "hello")
     v
[ft_printf]
     |
     | format string: "Test %d %s"
     | va_list: { -42, "hello" }
     |
     +---> 'T' -> ft_print_char('T') -> write(1,"T",1) -> count += 1
     +---> 'e' -> ft_print_char('e') -> write(1,"e",1) -> count += 1
     +---> 's' -> ft_print_char('s') -> write(1,"s",1) -> count += 1
     +---> 't' -> ft_print_char('t') -> write(1,"t",1) -> count += 1
     +---> ' ' -> ft_print_char(' ') -> write(1," ",1) -> count += 1
     +---> '%' -> format++ -> 'd'
     |           ft_print_format(args, 'd')
     |             -> va_arg(args, int) -> -42
     |             -> ft_print_nbr(-42)
     |               -> ft_put_nbr(-42)  [write "-42"]
     |               -> ft_numlen(-42)   [return 3]
     |             -> return 3
     |           count += 3
     +---> ' ' -> ft_print_char(' ') -> write(1," ",1) -> count += 1
     +---> '%' -> format++ -> 's'
     |           ft_print_format(args, 's')
     |             -> va_arg(args, char *) -> "hello"
     |             -> ft_print_str("hello")
     |               -> ft_strlen("hello") -> 5
     |               -> write(1, "hello", 5)
     |             -> return 5
     |           count += 5
     +---> '\0' -> ループ終了
     |
     | va_end(args)
     | return count = 1+1+1+1+1+3+1+5 = 14
     v
[呼び出し元に 14 が返る]
```

---

## 2. 各関数の完全なコードウォークスルー

### 2.1 ft_printf() - メイン関数

#### ソースコード（注釈付き）

```c
int	ft_printf(const char *format, ...)
{
	va_list	args;       /* 可変長引数を走査するためのイテレータ */
	int		count;      /* 出力した文字の総数を追跡するカウンタ */
	int		ret;        /* 各出力関数の戻り値を一時保存する変数 */

	if (!format)           /* -------- (1) NULL チェック -------- */
		return (-1);
	va_start(args, format); /* -------- (2) 可変長引数の初期化 -------- */
	count = 0;
	while (*format)         /* -------- (3) format string の走査 -------- */
	{
		if (*format == '%') /* -------- (4) 変換指定子の検出 -------- */
		{
			format++;
			ret = ft_print_format(args, *format);
			if (ret == -1)
				return (-1);
			count += ret;
		}
		else                /* -------- (5) 通常文字の出力 -------- */
		{
			ret = ft_print_char(*format);
			if (ret == -1)
				return (-1);
			count += ret;
		}
		format++;           /* -------- (6) 次の文字へ -------- */
	}
	va_end(args);           /* -------- (7) クリーンアップ -------- */
	return (count);         /* -------- (8) 合計文字数を返す -------- */
}
```

#### 各ステップの徹底解説

**(1) NULL チェック --- `if (!format) return (-1);`**

format が NULL の場合、`*format` でデリファレンス（dereference）すると segmentation fault になるため、最初にチェックします。

```
なぜ NULL チェックが必要か:

ft_printf(NULL);  を呼ばれた場合:

  format = NULL (= 0x0)
  *format  ->  アドレス 0x0 を読もうとする
           ->  OS がアクセスを禁止
           ->  SIGSEGV (segmentation fault)
           ->  プログラムがクラッシュ

NULL チェックにより:
  format = NULL
  !format = !NULL = !(0) = 1 (true)
  return (-1)  ->  安全にエラーを返す
```

**標準 printf との違い:** 標準の `printf(NULL)` は未定義動作（undefined behavior）です。多くの実装では segfault しますが、ft_printf では防御的プログラミング（defensive programming）として `-1` を返します。

**`!format` vs `format == NULL`:** C 言語では `!format` と `format == NULL` は同義です。`!` 演算子はポインタが NULL（0）のとき true を返します。42 Norm ではどちらのスタイルも許容されます。

**(2) va_start --- `va_start(args, format);`**

```
va_start の内部動作（概念図）:

関数呼び出し時のスタック:
  ft_printf("Hello %d", 42) の場合

  スタック（高アドレス -> 低アドレス）:
  +------------------+
  | 42               |  <- 第2引数（可変長引数）
  +------------------+
  | "Hello %d" のアドレス |  <- format（最後の固定引数）
  +------------------+
  | リターンアドレス    |
  +------------------+

  va_start(args, format) の効果:
  args が format の次の引数（42）を指すように初期化される

  args --> [42]

  以後、va_arg(args, int) で 42 を取得できる
```

`format` は最後の固定引数（last named parameter）です。va_start はこの引数の位置を起点にして、可変長引数リストへのアクセスを準備します。

**重要な制約:**
- va_start の第2引数は、必ず関数の最後の固定引数でなければなりません
- va_start を呼んだら、正常パスでは必ず va_end を呼ぶ必要があります
- va_start を呼ぶ前に va_arg を使うことはできません

**(3) format string の走査 --- `while (*format)`**

```
while (*format) の動作:

format = "Hi %d\n"

イテレーション:
  format[0] = 'H'  (0x48)  -> *format は 0x48 (true)  -> ループ実行
  format[1] = 'i'  (0x69)  -> *format は 0x69 (true)  -> ループ実行
  format[2] = ' '  (0x20)  -> *format は 0x20 (true)  -> ループ実行
  format[3] = '%'  (0x25)  -> *format は 0x25 (true)  -> ループ実行
  format[4] = 'd'  (0x64)  -> *format は 0x64 (true)  -> ループ実行
  format[5] = '\n' (0x0A)  -> *format は 0x0A (true)  -> ループ実行
  format[6] = '\0' (0x00)  -> *format は 0x00 (false) -> ループ終了

'\0' (null terminator) の値は 0 なので、C の条件式では false と評価される
それ以外の全ての文字は非 0 なので true と評価される
```

**(4) '%' の検出 --- `if (*format == '%')`**

`%` を見つけたら、`format++` で次の文字（変換指定子、specifier）に進め、ディスパッチャに渡します。

```
format string: "Value: %d"
               0123456789

*format が '%' (位置6) のとき:
  format++         -> format は位置7 を指す
  *format = 'd'    -> specifier
  ft_print_format(args, 'd')  -> dispatcher に 'd' を渡す

  format++（ループ末尾） -> format は位置8 を指す
  -> 合計で2文字分（'%' と 'd'）をスキップ
```

**format++ が2回実行される理由:**
1回目: `%` を見つけた直後（ステップ4内）--- specifier を読むため
2回目: while ループの末尾（ステップ6）--- 次の文字に進むため

これにより、`%d` のような2文字の変換指定子全体をスキップします。

**(5) 通常文字の出力 --- `ret = ft_print_char(*format);`**

`%` 以外の文字はそのまま `ft_print_char` で1文字ずつ出力します。改行 `\n`、タブ `\t`、スペースなどの制御文字もここで処理されます。

```
通常文字の処理:
  *format = 'A' の場合:
    ft_print_char('A')
      -> write(1, "A", 1)
      -> return 1
    ret = 1
    count += 1

  *format = '\n' の場合:
    ft_print_char('\n')
      -> write(1, "\n", 1)  <- 改行も1文字
      -> return 1
    ret = 1
    count += 1
```

**(6) format++ --- 次の文字へ進める**

```
通常文字の場合:  format++ が1回 -> 1文字分進む（正しい）
変換指定子の場合: format++ が2回 -> 2文字分進む（'%' + specifier を飛ばす）

例: format = "A%dB"
  ループ1: *format='A', 通常文字 -> 出力 -> format++ -> format は '%' を指す
  ループ2: *format='%' -> format++ (内部) -> *format='d' -> dispatch
           -> format++ (末尾) -> format は 'B' を指す
  ループ3: *format='B', 通常文字 -> 出力 -> format++ -> format は '\0' を指す
  ループ4: *format='\0' -> while 終了
```

**(7) va_end --- `va_end(args);`**

va_start と対になるクリーンアップマクロです。ループを正常に抜けた場合にのみ到達します。

```
va_start / va_end の関係:

  va_start(args, format);    <- リソースを確保（初期化）
  // ... 処理 ...
  va_end(args);              <- リソースを解放（クリーンアップ）

C 標準（C99 7.15.1）:
  "va_start と va_end の呼び出しは同じ関数内で対応していなければならない"
  "va_start の後に va_end を呼ばない場合、動作は未定義"
```

> **注意**: エラーで early return する場合（`if (ret == -1) return (-1);`）、va_end が呼ばれません。C 標準では va_start 後に va_end を呼ばないと undefined behavior ですが、実際にはほとんどのプラットフォーム（x86_64 Linux, macOS）で問題になりません。これは、現代の ABI（Application Binary Interface）では va_list が単なるポインタであり、va_end が実質的に何もしないためです。厳密には、エラー時も va_end を呼ぶ方が望ましいですが、42 Norm の25行制限との兼ね合いで省略されることが多いです。

**(8) return (count) --- 合計文字数を返す**

```
count の累積過程の例:

ft_printf("Hi %d!\n", 42) の場合:

  'H'  -> count = 0 + 1 = 1
  'i'  -> count = 1 + 1 = 2
  ' '  -> count = 2 + 1 = 3
  '%d' -> ft_print_nbr(42) -> 2文字 -> count = 3 + 2 = 5
  '!'  -> count = 5 + 1 = 6
  '\n' -> count = 6 + 1 = 7

  return 7
```

この戻り値は標準 printf と同じ仕様です。`printf` は「出力に成功した文字数」を返し、エラー時は負の値を返します。

### 2.2 ft_print_format() - ディスパッチャ

#### ソースコード（注釈付き）

```c
static int	ft_print_format(va_list args, char specifier)
{
	int	count;

	count = 0;
	if (specifier == 'c')
		count = ft_print_char(va_arg(args, int));
	else if (specifier == 's')
		count = ft_print_str(va_arg(args, char *));
	else if (specifier == 'p')
		count = ft_print_ptr(va_arg(args, unsigned long long));
	else if (specifier == 'd' || specifier == 'i')
		count = ft_print_nbr(va_arg(args, int));
	else if (specifier == 'u')
		count = ft_print_unsigned(va_arg(args, unsigned int));
	else if (specifier == 'x')
		count = ft_print_hex(va_arg(args, unsigned int), 0);
	else if (specifier == 'X')
		count = ft_print_hex(va_arg(args, unsigned int), 1);
	else if (specifier == '%')
		count = ft_print_char('%');
	return (count);
}
```

#### ディスパッチャの詳細解説

**役割:** format string 中の `%` の直後にある文字（specifier）を見て、対応する出力関数を呼び出す「振り分け役」です。

**va_arg の型指定が重要な理由:**

各変換指定子に対して正しい型を va_arg に指定する必要があります。間違った型を指定すると undefined behavior になります。

```
va_arg の型指定と Default Argument Promotion の関係:

可変長引数として渡される値は、C 言語の規則により自動的に promote されます:

  呼び出し: ft_printf("%c", 'A')

  'A' の型は char (1バイト)
  | Default Argument Promotion
  v
  int (4バイト) に promote される
  |
  v
  va_arg(args, int) で取得  <- char ではなく int を指定する！

型指定一覧と理由:
  %c  -> va_arg(args, int)
         char は int に promote されるため

  %s  -> va_arg(args, char *)
         ポインタは promote されない

  %p  -> va_arg(args, unsigned long long)
         void * のサイズは環境依存（64bit では 8バイト）
         unsigned long long は確実に 8バイト以上

  %d/%i -> va_arg(args, int)
           int はそのまま

  %u  -> va_arg(args, unsigned int)
         unsigned int はそのまま

  %x/%X -> va_arg(args, unsigned int)
           unsigned int はそのまま

  %%  -> va_arg を呼ばない！
         引数を消費しない（リテラル '%' を出力するだけ）
```

**va_arg を間違った型で呼ぶとどうなるか:**

```c
/* 危険な例（やってはいけない） */
ft_printf("%d", 42);

/* もし dispatcher 内で誤って以下のように書いたら: */
va_arg(args, long long);  /* int なのに long long で読んでしまった */

/* スタックから 8 バイト読んでしまい、次の va_arg の位置がずれる */
/* 後続の引数がすべて壊れる */

/*
  スタック: [42 (4byte)] [???? (4byte)] [次の引数...]
             ^^^^^^^^^^^^^^^^^^^^^^^
             long long として 8 バイト読んでしまう
             -> 42 + ゴミデータ が混ざった値になる
*/
```

**`%%` は引数を消費しない:**

```c
ft_printf("100%%");   /* 出力: "100%", 戻り値: 4 */
ft_printf("%d%%", 42); /* 出力: "42%",  戻り値: 3 */
```

`%%` のとき、`ft_print_char('%')` を直接呼びます。va_arg は呼ばれないので、va_list の位置は進みません。

**不明な specifier の場合:**

```c
ft_printf("%z");  /* 'z' はどの条件にも該当しない */
/* count = 0 のまま return される */
/* '%z' に対して何も出力せず、0 を返す */
```

この動作は実装の選択です。標準 printf では不明な specifier は undefined behavior ですが、この実装では無視する方式を採っています。

**`%d` と `%i` の統合:**

```c
else if (specifier == 'd' || specifier == 'i')
    count = ft_print_nbr(va_arg(args, int));
```

`%d` と `%i` は ft_printf の mandatory part では完全に同じ動作です。標準 printf でも `printf` 内では同じですが、`scanf` では異なる動作をします（`%i` は 0x prefix で16進数、0 prefix で8進数を受け付ける）。

### 2.3 ft_print_char() - 文字出力

#### ソースコード（注釈付き）

```c
int	ft_print_char(int c)
{
	unsigned char	ch;

	ch = (unsigned char)c;        /* int -> unsigned char に変換 */
	if (write(1, &ch, 1) == -1)  /* 1バイトを stdout に出力 */
		return (-1);              /* write 失敗時はエラーを返す */
	return (1);                   /* 成功時は常に 1（1文字出力した） */
}
```

#### 引数が int 型である理由

`ft_print_char` は `char` ではなく `int` を受け取ります。これは2つの理由があります:

1. **Default Argument Promotion**: 可変長引数として渡された `char` は自動的に `int` に promote されるため、`va_arg(args, int)` で取得する
2. **通常文字の出力にも使用**: `ft_printf` 内で `ft_print_char(*format)` と呼ばれる。`*format` は `char` 型だが、関数に渡す際に `int` に promote される

#### unsigned char キャストの詳細

```
なぜ unsigned char にキャストするのか:

ケース1: 通常の ASCII 文字 (0-127)
  int c = 65 ('A')

  int のメモリレイアウト（リトルエンディアン、4バイト）:
  アドレス: [低] -> [高]
  バイト:   [41]  [00]  [00]  [00]
             ^^
  write(1, &c, 1) はアドレスの先頭1バイト(0x41='A')を読む
  -> リトルエンディアンでは問題なし

  unsigned char ch = (unsigned char)65;
  ch のメモリ: [41]
  write(1, &ch, 1) -> 'A'
  -> 確実に正しい

ケース2: 拡張 ASCII (128-255)
  int c = 200

  int のメモリ: [C8]  [00]  [00]  [00]
  write(1, &c, 1) -> 0xC8 -> OK（リトルエンディアン）

  unsigned char ch = (unsigned char)200;
  ch のメモリ: [C8]
  write(1, &ch, 1) -> 0xC8 -> 確実にOK

ケース3: ビッグエンディアン環境（SPARC, PowerPC 等）
  int c = 65 ('A')

  int のメモリ（ビッグエンディアン）:
  アドレス: [低] -> [高]
  バイト:   [00]  [00]  [00]  [41]
             ^^
  write(1, &c, 1) はアドレスの先頭1バイト(0x00)を読む
  -> '\0' が出力される！意図と異なる！

  unsigned char ch = (unsigned char)65;
  ch のメモリ: [41]
  write(1, &ch, 1) -> 'A' -> 正しい！

結論: unsigned char にキャストすることで、エンディアンに依存しない
      ポータブルなコードになる
```

#### write システムコールの詳細

```c
ssize_t write(int fd, const void *buf, size_t count);
```

```
write(1, &ch, 1) の各引数:

  1    = fd（ファイルディスクリプタ）
         0 = stdin（標準入力）
         1 = stdout（標準出力）  <- ft_printf はここに出力
         2 = stderr（標準エラー出力）

  &ch  = buf（書き込むデータの先頭アドレス）
         ch が 'A' なら、'A' が格納されているメモリのアドレス

  1    = count（書き込むバイト数）
         1バイト = 1文字

戻り値:
  成功時: 書き込んだバイト数（通常は count と同じ）
  失敗時: -1（errno にエラー原因がセットされる）

write が失敗するケース:
  - fd が無効（close(1) された後など）
  - ディスクが一杯（リダイレクト先がファイルの場合）
  - パイプの読み取り側が閉じられた（SIGPIPE）
  - シグナルによる中断（EINTR）
```

#### '\0' を出力する場合

```c
ft_printf("%c", '\0');

/* ft_print_char(0) が呼ばれる */
/* ch = (unsigned char)0 = '\0' */
/* write(1, &ch, 1) -> NULL バイトを1つ出力 */
/* 端末には何も表示されないが、1バイトは出力されている */
/* return 1 -> ft_printf の戻り値に 1 が加算される */
```

### 2.4 ft_print_str() - 文字列出力

#### ソースコード（注釈付き）

```c
static int	ft_strlen(char *str)
{
	int	len;

	len = 0;
	while (str[len])  /* '\0' に達するまでカウント */
		len++;
	return (len);
}

int	ft_print_str(char *str)
{
	int	len;

	if (!str)              /* NULL ポインタチェック */
		str = "(null)";    /* NULL なら "(null)" 文字列に置き換え */
	len = ft_strlen(str);  /* 文字列長を計算 */
	if (write(1, str, len) == -1)  /* 文字列全体を1回で出力 */
		return (-1);
	return (len);          /* 出力した文字数を返す */
}
```

#### ft_strlen の動作詳細

```
ft_strlen("Hello") の動作:

  str = "Hello"
  メモリ: ['H']['e']['l']['l']['o']['\0']
  インデックス: 0    1    2    3    4    5

  len = 0: str[0] = 'H' (0x48 != 0) -> len = 1
  len = 1: str[1] = 'e' (0x65 != 0) -> len = 2
  len = 2: str[2] = 'l' (0x6C != 0) -> len = 3
  len = 3: str[3] = 'l' (0x6C != 0) -> len = 4
  len = 4: str[4] = 'o' (0x6F != 0) -> len = 5
  len = 5: str[5] = '\0' (0x00 == 0) -> ループ終了

  return 5

ft_strlen("") の動作:
  str = ""
  メモリ: ['\0']
  len = 0: str[0] = '\0' -> ループ実行されない
  return 0
```

#### NULL チェックの流れ

```
ft_print_str(NULL) の詳細な動作:

1. str = NULL (= 0x0)
2. !str -> !NULL -> !(0) -> 1 (true) -> 条件成立
3. str = "(null)"
   str ポインタが "(null)" という文字列リテラルを指すように変更
   "(null)" という文字列は .rodata セグメントに存在する
4. ft_strlen("(null)") -> 6
5. write(1, "(null)", 6) -> "(null)" を出力
6. return 6
```

**なぜ "(null)" を出力するのか:**

これは Linux の glibc（GNU C Library）の `printf` の動作に準拠しています。

```c
/* 標準 printf の動作（Linux glibc） */
printf("%s", NULL);  /* 出力: "(null)" */

/* C 標準規格（C99/C11）では: */
/* %s に NULL を渡すことは undefined behavior */
/* しかし、多くの実装は "(null)" を出力する */
```

#### なぜ文字列全体を1回の write で出力するのか

```
方法1: 1文字ずつ write（非効率）
  write(1, "H", 1);
  write(1, "e", 1);
  write(1, "l", 1);
  write(1, "l", 1);
  write(1, "o", 1);
  -> 5回のシステムコール

方法2: まとめて write（効率的）
  write(1, "Hello", 5);
  -> 1回のシステムコール

システムコールのコスト:
  各 write 呼び出しで:
  1. ユーザモード -> カーネルモードへの遷移（context switch）
  2. カーネル内でバッファにデータをコピー
  3. カーネルモード -> ユーザモードへの遷移

  1回の write に比べ、5回の write は約5倍のオーバーヘッド
```

### 2.5 ft_print_ptr() - ポインタ出力

#### ソースコード（注釈付き）

```c
static int	ft_ptr_len(unsigned long long ptr)
{
	int	len;

	len = 0;
	if (ptr == 0)
		return (1);         /* 0 は "0" で1桁 */
	while (ptr > 0)
	{
		len++;
		ptr /= 16;         /* 16で割って桁数をカウント */
	}
	return (len);
}

static int	ft_put_ptr(unsigned long long ptr)
{
	char	*hex;
	int		ret;

	hex = "0123456789abcdef";  /* lookup table */
	if (ptr >= 16)             /* まだ上位桁がある */
	{
		ret = ft_put_ptr(ptr / 16);   /* 上位桁を先に出力（再帰） */
		if (ret == -1)
			return (-1);
		ret = ft_put_ptr(ptr % 16);   /* 下位桁を出力（再帰） */
		if (ret == -1)
			return (-1);
	}
	else                       /* base case: 1桁（0-15） */
	{
		if (write(1, &hex[ptr], 1) == -1)  /* 対応する16進文字を出力 */
			return (-1);
	}
	return (0);
}

int	ft_print_ptr(unsigned long long ptr)
{
	int	count;

	if (ptr == 0)                          /* NULL ポインタ */
	{
		if (write(1, "(nil)", 5) == -1)
			return (-1);
		return (5);
	}
	if (write(1, "0x", 2) == -1)           /* "0x" prefix */
		return (-1);
	count = 2;
	if (ft_put_ptr(ptr) == -1)             /* 16進数部分を再帰出力 */
		return (-1);
	count += ft_ptr_len(ptr);              /* 桁数を加算 */
	return (count);
}
```

#### ft_put_ptr の再帰構造の詳細

この実装の `ft_put_ptr` は**二分木型の再帰**を行います。`ptr >= 16` のとき、`ptr / 16`（上位桁）と `ptr % 16`（最下位桁）の両方に対して再帰呼び出しを行います。

```
ft_put_ptr(0x1A3) の動作 (419 in decimal):

ft_put_ptr(419)
  419 >= 16 -> true
  +-- ft_put_ptr(419 / 16 = 26)      <- 上位桁を処理
  |     26 >= 16 -> true
  |     +-- ft_put_ptr(26 / 16 = 1)  <- さらに上位
  |     |     1 < 16 -> base case
  |     |     write hex[1] = '1'      <- 出力 (1)
  |     |     return 0
  |     +-- ft_put_ptr(26 % 16 = 10) <- 下位
  |     |     10 < 16 -> base case
  |     |     write hex[10] = 'a'     <- 出力 (2)
  |     |     return 0
  |     return 0
  +-- ft_put_ptr(419 % 16 = 3)       <- 最下位桁を処理
  |     3 < 16 -> base case
  |     write hex[3] = '3'            <- 出力 (3)
  |     return 0
  return 0

出力順: '1' -> 'a' -> '3'
結果: "1a3"
```

**二分木再帰の視覚化:**

```
           ft_put_ptr(419)
          /                \
  ft_put_ptr(26)      ft_put_ptr(3)
   /          \              |
ft_put_ptr(1)  ft_put_ptr(10)  write '3'
    |              |
  write '1'     write 'a'

左から右に読むと: '1' 'a' '3' -> 正しい順序！
```

#### NULL ポインタの処理

```
ft_print_ptr(0) の動作:

1. ptr == 0 -> true
2. write(1, "(nil)", 5)
3. return 5

出力: "(nil)"
戻り値: 5
```

**Linux vs macOS の NULL ポインタ出力の違い:**

```
Linux (glibc):
  printf("%p", NULL);  -> "(nil)"

macOS (Apple libc):
  printf("%p", NULL);  -> "0x0"

42 の ft_printf では、Linux (glibc) の動作に合わせるのが一般的。
ただし、macOS で開発する場合はテスターで "(nil)" と "0x0" の
どちらが期待されるか確認すること。
```

#### unsigned long long を使う理由

```
64ビット環境でのポインタサイズ:

  void *ptr;
  sizeof(ptr) = 8 バイト（64ビット）

各型のサイズ（64ビット Linux）:
  unsigned int        = 4 バイト (32ビット) -> ポインタが入りきらない！
  unsigned long       = 8 バイト (64ビット) -> OK
  unsigned long long  = 8 バイト (64ビット) -> OK

  ポインタの範囲: 0x0000000000000000 ~ 0xFFFFFFFFFFFFFFFF
  unsigned int の範囲: 0x00000000 ~ 0xFFFFFFFF
  -> unsigned int では上位32ビットが切り捨てられる！

例: ptr = 0x7FFF12345678 の場合
  (unsigned int)ptr = 0x12345678  <- 上位の 0x7FFF が消える！
  (unsigned long long)ptr = 0x7FFF12345678  <- 正しい
```

### 2.6 ft_print_nbr() - 符号付き整数出力

#### ソースコード（注釈付き）

```c
static int	ft_numlen(int n)
{
	int	len;

	len = 0;
	if (n <= 0)
		len = 1;        /* n==0 なら "0" の1桁、n<0 なら "-" の1文字分 */
	while (n != 0)
	{
		len++;
		n /= 10;        /* 10で割って桁数をカウント */
	}
	return (len);
}

static int	ft_put_nbr(int n)
{
	char	c;

	if (n == -2147483648)                      /* INT_MIN 特別処理 */
	{
		if (write(1, "-2147483648", 11) == -1)
			return (-1);
		return (0);
	}
	if (n < 0)                                 /* 負の数の処理 */
	{
		if (write(1, "-", 1) == -1)
			return (-1);
		n = -n;
	}
	if (n >= 10)                               /* 再帰: 上位桁を先に出力 */
	{
		if (ft_put_nbr(n / 10) == -1)
			return (-1);
	}
	c = (n % 10) + '0';                        /* 最下位桁を文字に変換 */
	if (write(1, &c, 1) == -1)
		return (-1);
	return (0);
}

int	ft_print_nbr(int n)
{
	if (ft_put_nbr(n) == -1)
		return (-1);
	return (ft_numlen(n));
}
```

#### ft_numlen の動作詳細

```
ft_numlen の設計のポイント:

  if (n <= 0)
      len = 1;

  この1行で2つのケースを同時に処理している:

  ケース1: n == 0
    -> "0" と表示する -> 1桁
    -> len = 1 からスタート
    -> while (0 != 0) は実行されない
    -> return 1

  ケース2: n < 0  (例: n = -42)
    -> "-42" と表示する -> '-' + 2桁 = 3文字
    -> len = 1 からスタート（'-' 符号の分）
    -> while (-42 != 0): len=2, n=-4
    -> while (-4 != 0):  len=3, n=0
    -> while (0 != 0):   終了
    -> return 3

  ケース3: n > 0  (例: n = 42)
    -> "42" と表示する -> 2桁
    -> n > 0 なので len = 0 からスタート
    -> while (42 != 0): len=1, n=4
    -> while (4 != 0):  len=2, n=0
    -> while (0 != 0):  終了
    -> return 2
```

**全境界値での ft_numlen の動作:**

```
ft_numlen(0):            len=1 -> while不実行 -> 1
ft_numlen(1):            len=0 -> 1回ループ -> 1
ft_numlen(9):            len=0 -> 1回ループ -> 1
ft_numlen(10):           len=0 -> 2回ループ -> 2
ft_numlen(99):           len=0 -> 2回ループ -> 2
ft_numlen(100):          len=0 -> 3回ループ -> 3
ft_numlen(2147483647):   len=0 -> 10回ループ -> 10  (INT_MAX)
ft_numlen(-1):           len=1 -> 1回ループ -> 2
ft_numlen(-9):           len=1 -> 1回ループ -> 2
ft_numlen(-10):          len=1 -> 2回ループ -> 3
ft_numlen(-2147483648):  len=1 -> 10回ループ -> 11  (INT_MIN)
```

**なぜ `n != 0` でループするのか（`n > 0` ではなく）:**

```
n < 0 の場合（例: n = -42）:
  -42 / 10 = -4  (C99: 0方向への切り捨て)
  -4 / 10 = 0

  n != 0 なら: -42 -> -4 -> 0 で2回ループ -> len=1+2=3 ("-42")
  n > 0 なら:  -42 > 0 は false -> ループ0回 -> len=1 (間違い！)

  n != 0 を使うことで、負の数でも正しくカウントできる
```

#### INT_MIN 処理の完全解説

```
なぜ INT_MIN (-2147483648) を特別扱いするのか:

2の補数表現（two's complement）:
  32ビット int の範囲:
    INT_MAX =  2147483647 = 0x7FFFFFFF = 0111 1111 ... 1111
    INT_MIN = -2147483648 = 0x80000000 = 1000 0000 ... 0000

  注目: |INT_MIN| = 2147483648 > INT_MAX = 2147483647
        -> INT_MIN の絶対値は int では表現できない！

通常の負数処理:
  n = -42 の場合:
    n = -n;  -> n = 42  <- int の範囲内なので OK

  n = -2147483648 の場合:
    n = -n;  -> n = 2147483648  <- int の範囲外！
    -> signed integer overflow -> undefined behavior!

実際に起こること（多くの環境で）:
  -(-2147483648) = 2147483648
  しかし int は最大 2147483647
  2の補数では: 2147483648 = 0x80000000 = -2147483648
  つまり: -(-2147483648) = -2147483648  <- 元に戻ってしまう！

解決策: INT_MIN を文字列リテラルとして直接出力
  if (n == -2147483648)
  {
      write(1, "-2147483648", 11);
      return (0);
  }
```

**INT_MIN 問題の別解（参考）:**

```c
/* 別解1: long を使う */
static int ft_put_nbr(int n)
{
    long ln = (long)n;  /* int -> long に拡張 */
    if (ln < 0)
    {
        write(1, "-", 1);
        ln = -ln;  /* long なら 2147483648 を保持できる */
    }
    /* 以下、ln を使って処理 */
}

/* 別解2: unsigned int を使う */
static int ft_put_nbr(int n)
{
    unsigned int un;
    if (n < 0)
    {
        write(1, "-", 1);
        un = (unsigned int)(-(n + 1)) + 1;
        /* -(- 2147483648 + 1) + 1 = 2147483647 + 1 = 2147483648 */
        /* unsigned int は 4294967295 まで保持できるので OK */
    }
    else
        un = (unsigned int)n;
    /* 以下、un を使って処理 */
}

/* 本実装の方式: 文字列リテラルで直接出力（最もシンプル） */
if (n == -2147483648)
{
    write(1, "-2147483648", 11);
    return (0);
}
```

#### ft_put_nbr の再帰展開（n = -42）

```
ft_put_nbr(-42)
|
+-- n == -2147483648? -> -42 != -2147483648 -> No
+-- n < 0? -> -42 < 0 -> Yes
|    write(1, "-", 1) -> '-' を出力
|    n = -(-42) = 42
|
+-- n >= 10? -> 42 >= 10 -> Yes
|    ft_put_nbr(42 / 10 = 4)
|    |
|    +-- n == -2147483648? -> No
|    +-- n < 0? -> 4 >= 0 -> No
|    +-- n >= 10? -> 4 < 10 -> No (base case!)
|    +-- c = 4 % 10 + '0' = 4 + 48 = 52 = '4'
|    +-- write(1, &c, 1) -> '4' を出力
|    return 0
|
+-- c = 42 % 10 + '0' = 2 + 48 = 50 = '2'
+-- write(1, &c, 1) -> '2' を出力
return 0

出力順: '-' -> '4' -> '2'
結果: "-42"
```

### 2.7 ft_print_unsigned() - 符号なし整数出力

#### ソースコード（注釈付き）

```c
static int	ft_unumlen(unsigned int n)
{
	int	len;

	len = 0;
	if (n == 0)
		return (1);      /* 0 は "0" で1桁 */
	while (n > 0)
	{
		len++;
		n /= 10;
	}
	return (len);
}

static int	ft_put_unsigned(unsigned int n)
{
	char	c;

	if (n >= 10)         /* まだ上位桁がある */
	{
		if (ft_put_unsigned(n / 10) == -1)
			return (-1);
	}
	c = (n % 10) + '0'; /* 最下位桁を文字に変換 */
	if (write(1, &c, 1) == -1)
		return (-1);
	return (0);
}

int	ft_print_unsigned(unsigned int n)
{
	if (ft_put_unsigned(n) == -1)
		return (-1);
	return (ft_unumlen(n));
}
```

#### ft_print_nbr との比較

```
ft_print_nbr (signed int)         vs   ft_print_unsigned (unsigned int)

引数の型:                                引数の型:
  int n                                    unsigned int n

範囲:                                    範囲:
  -2147483648 ~ 2147483647                 0 ~ 4294967295

INT_MIN 特別処理:                        INT_MIN 特別処理:
  あり（n == -2147483648）                 不要（常に非負）

負の数の処理:                            負の数の処理:
  あり（n < 0 -> write "-" -> n = -n）     不要（常に非負）

ループ条件 (numlen):                     ループ条件 (unumlen):
  n != 0（負の数にも対応するため）        n > 0（常に非負なので同義）
```

**ft_put_unsigned の再帰展開（n = 4294967295, UINT_MAX）:**

```
ft_put_unsigned(4294967295)
|
+-- 4294967295 >= 10 -> true
|   ft_put_unsigned(4294967295 / 10 = 429496729)
|   |
|   +-- 429496729 >= 10 -> true
|   |   ft_put_unsigned(42949672)
|   |   ... (再帰が続く、最終的に ft_put_unsigned(4) に到達)
|   |
|   |   [base case] 4 < 10
|   |   c = 4 + '0' = '4', write '4'
|   |
|   [各段で余りを出力]
|   ...
|
+-- c = 4294967295 % 10 + '0' = 5 + '0' = '5'
+-- write '5'

出力: "4294967295"
再帰深度: 10（10桁なので10段）
```

### 2.8 ft_print_hex() - 16進数出力

#### ソースコード（注釈付き）

```c
static int	ft_hexlen(unsigned int n)
{
	int	len;

	len = 0;
	if (n == 0)
		return (1);       /* 0 は "0" で1桁 */
	while (n > 0)
	{
		len++;
		n /= 16;         /* 16で割って桁数をカウント */
	}
	return (len);
}

static int	ft_put_hex(unsigned int n, int uppercase)
{
	char	*hex;

	if (uppercase)                         /* 大文字/小文字の選択 */
		hex = "0123456789ABCDEF";
	else
		hex = "0123456789abcdef";
	if (n >= 16)                           /* まだ上位桁がある */
	{
		if (ft_put_hex(n / 16, uppercase) == -1)
			return (-1);
	}
	if (write(1, &hex[n % 16], 1) == -1)   /* lookup table で変換して出力 */
		return (-1);
	return (0);
}

int	ft_print_hex(unsigned int n, int uppercase)
{
	if (ft_put_hex(n, uppercase) == -1)
		return (-1);
	return (ft_hexlen(n));
}
```

#### Lookup Table（ルックアップテーブル）の仕組み

```
hex = "0123456789abcdef" は文字の配列として使われる:

  hex[0]  = '0'    hex[8]  = '8'
  hex[1]  = '1'    hex[9]  = '9'
  hex[2]  = '2'    hex[10] = 'a'
  hex[3]  = '3'    hex[11] = 'b'
  hex[4]  = '4'    hex[12] = 'c'
  hex[5]  = '5'    hex[13] = 'd'
  hex[6]  = '6'    hex[14] = 'e'
  hex[7]  = '7'    hex[15] = 'f'

n % 16 は必ず 0~15 の範囲 -> 配列の有効なインデックス
```

#### ft_put_hex の再帰展開（n = 255, uppercase = 0）

```
ft_put_hex(255, 0)
|
+-- hex = "0123456789abcdef"
+-- 255 >= 16 -> true
|   ft_put_hex(255 / 16 = 15, 0)
|   |
|   +-- hex = "0123456789abcdef"
|   +-- 15 < 16 -> false（再帰しない）
|   +-- write hex[15 % 16] = hex[15] = 'f'  <- 出力 (1)
|   +-- return 0
|
+-- write hex[255 % 16] = hex[15] = 'f'     <- 出力 (2)
+-- return 0

出力: "ff"
```

#### ft_put_hex vs ft_put_ptr の違い

```
ft_put_hex (unsigned int):
  if (n >= 16)
      ft_put_hex(n / 16, uppercase);  <- 1回の再帰呼び出し
  write hex[n % 16]                    <- 常に実行

ft_put_ptr (unsigned long long):
  if (ptr >= 16)
      ft_put_ptr(ptr / 16);           <- 1回目の再帰（上位桁）
      ft_put_ptr(ptr % 16);           <- 2回目の再帰（下位桁）
  else
      write hex[ptr]                   <- base case のみ実行

ft_put_hex は「再帰 + 毎回 write」パターン
ft_put_ptr は「二分木再帰」パターン

どちらも正しい結果を出力するが、アプローチが異なる。
ft_put_hex の方が再帰呼び出し回数が少なく効率的。
```

---

## 3. ステップバイステップ実行トレース

### 3.1 完全トレース: ft_printf("Test %d %s %p", -42, NULL, ptr)

前提条件:
```c
int x = 100;
void *ptr = &x;  /* 仮に 0x7ffd5678 とする */

int ret = ft_printf("Test %d %s %p", -42, NULL, ptr);
```

```
== ft_printf 開始 ==

引数の状態:
  format = "Test %d %s %p"
  va_list: { -42 (int), NULL (char *), 0x7ffd5678 (unsigned long long) }

format が NULL? -> "Test %d %s %p" は NULL でない -> 続行
va_start(args, format)
count = 0

--- ループ開始 ---

[pos=0] *format = 'T' (0x54)
  '%' でない -> 通常文字
  ft_print_char('T')
    ch = (unsigned char)'T' = 0x54
    write(1, &ch, 1) -> 'T' を出力 -> 成功
    return 1
  ret = 1, ret != -1 -> OK
  count = 0 + 1 = 1
  format++ -> pos=1

[pos=1] *format = 'e' (0x65)
  '%' でない -> 通常文字
  ft_print_char('e') -> write(1, "e", 1) -> 成功 -> return 1
  count = 1 + 1 = 2
  format++ -> pos=2

[pos=2] *format = 's' (0x73)
  count = 2 + 1 = 3, format++ -> pos=3

[pos=3] *format = 't' (0x74)
  count = 3 + 1 = 4, format++ -> pos=4

[pos=4] *format = ' ' (0x20)
  count = 4 + 1 = 5, format++ -> pos=5

[pos=5] *format = '%' (0x25) <- 変換指定子を検出！
  format++ -> pos=6, *format = 'd'
  ft_print_format(args, 'd')
    specifier = 'd'
    'd' == 'd' || 'd' == 'i'? -> Yes!
    va_arg(args, int) -> -42 を取得（va_list が次の引数に進む）
    ft_print_nbr(-42)
      ft_put_nbr(-42)
        -42 == -2147483648? No
        -42 < 0? Yes
          write(1, "-", 1) -> '-' を出力
          n = -(-42) = 42
        42 >= 10? Yes
          ft_put_nbr(42 / 10 = 4)
            4 < 10 (base case)
            c = 4 % 10 + '0' = '4'
            write(1, &c, 1) -> '4' を出力
            return 0
        c = 42 % 10 + '0' = '2'
        write(1, &c, 1) -> '2' を出力
        return 0
      ft_numlen(-42) -> 3
      return 3
  ret = 3
  count = 5 + 3 = 8
  format++ -> pos=7

[pos=7] *format = ' ' (0x20)
  count = 8 + 1 = 9, format++ -> pos=8

[pos=8] *format = '%' (0x25) <- 変換指定子を検出！
  format++ -> pos=9, *format = 's'
  ft_print_format(args, 's')
    specifier = 's'
    va_arg(args, char *) -> NULL を取得
    ft_print_str(NULL)
      str = NULL, !str -> true
      str = "(null)"
      ft_strlen("(null)") -> 6
      write(1, "(null)", 6) -> "(null)" を出力
      return 6
  ret = 6
  count = 9 + 6 = 15
  format++ -> pos=10

[pos=10] *format = ' ' (0x20)
  count = 15 + 1 = 16, format++ -> pos=11

[pos=11] *format = '%' (0x25) <- 変換指定子を検出！
  format++ -> pos=12, *format = 'p'
  ft_print_format(args, 'p')
    specifier = 'p'
    va_arg(args, unsigned long long) -> 0x7ffd5678 を取得
    ft_print_ptr(0x7ffd5678)
      ptr = 0x7ffd5678 (2147534456)
      ptr == 0? No
      write(1, "0x", 2) -> "0x" を出力
      count = 2
      ft_put_ptr(0x7ffd5678) -> [再帰で "7ffd5678" を出力]
      ft_ptr_len(0x7ffd5678) -> 8
      count = 2 + 8 = 10
      return 10
  ret = 10
  count = 16 + 10 = 26
  format++ -> pos=13

[pos=13] *format = '\0' (0x00)
  while (*format) -> while(0) -> false -> ループ終了

va_end(args)
return count = 26

== ft_printf 終了 ==

最終出力: "Test -42 (null) 0x7ffd5678"
戻り値: 26

検証: T(1)+e(1)+s(1)+t(1)+" "(1)+"-42"(3)+" "(1)+"(null)"(6)+" "(1)+"0x7ffd5678"(10) = 26
```

### 3.2 再帰の完全トレース: ft_put_nbr(12345)

```
ft_put_nbr(12345) が呼ばれた

Step 1: 初回チェック
  12345 == -2147483648? -> No
  12345 < 0? -> No
  12345 >= 10? -> Yes

Step 2: 再帰呼び出し ft_put_nbr(1234)
  1234 >= 10? -> Yes

  Step 3: 再帰呼び出し ft_put_nbr(123)
    123 >= 10? -> Yes

    Step 4: 再帰呼び出し ft_put_nbr(12)
      12 >= 10? -> Yes

      Step 5: 再帰呼び出し ft_put_nbr(1)
        1 >= 10? -> No  ** BASE CASE **
        c = 1 % 10 + '0' = '1'
        write(1, "1", 1) -> 出力 (1)
        return 0

      (Step 4 に戻る)
      c = 12 % 10 + '0' = '2'
      write(1, "2", 1) -> 出力 (2)
      return 0

    (Step 3 に戻る)
    c = 123 % 10 + '0' = '3'
    write(1, "3", 1) -> 出力 (3)
    return 0

  (Step 2 に戻る)
  c = 1234 % 10 + '0' = '4'
  write(1, "4", 1) -> 出力 (4)
  return 0

(Step 1 に戻る)
c = 12345 % 10 + '0' = '5'
write(1, "5", 1) -> 出力 (5)
return 0

出力順: '1' -> '2' -> '3' -> '4' -> '5'
結果: "12345"
```

#### コールスタックの可視化

```
コールスタック（メモリの低いアドレスが上）:

[呼び出し時（push）]              [戻り時（pop）]

         底                              底
+------------------+            +------------------+
| ft_put_nbr(12345)|            | ft_put_nbr(12345)| write '5' (5)
+------------------+            +------------------+
| ft_put_nbr(1234) |            | ft_put_nbr(1234) | write '4' (4)
+------------------+            +------------------+
| ft_put_nbr(123)  |            | ft_put_nbr(123)  | write '3' (3)
+------------------+            +------------------+
| ft_put_nbr(12)   |            | ft_put_nbr(12)   | write '2' (2)
+------------------+            +------------------+
| ft_put_nbr(1)    | <- TOP     | ft_put_nbr(1)    | write '1' (1)
+------------------+            +------------------+
      push方向                         pop方向

時系列:
  push(12345) -> push(1234) -> push(123) -> push(12) -> push(1)
  -> write '1' -> pop -> write '2' -> pop -> write '3' -> pop
  -> write '4' -> pop -> write '5' -> pop

各スタックフレームに格納される情報:
  +-----------------------------+
  | リターンアドレス              |
  | ローカル変数 c (char, 1byte) |
  | 引数 n (int, 4bytes)        |
  | パディング                   |
  +-----------------------------+
  -> 1フレームあたり約 16~32 バイト
  -> 5段の再帰で約 80~160 バイト
  -> int の最大桁数は10桁 -> 最大10段 -> 約 160~320 バイト
  -> スタックオーバーフローの心配は不要
```

### 3.3 再帰の完全トレース: ft_put_hex(0xDEAD, 0)

```
ft_put_hex(57005, 0)  <- 0xDEAD = 57005

hex = "0123456789abcdef"

ft_put_hex(57005, 0)
| 57005 >= 16 -> true
| ft_put_hex(57005 / 16 = 3562, 0)
| | 3562 >= 16 -> true
| | ft_put_hex(3562 / 16 = 222, 0)
| | | 222 >= 16 -> true
| | | ft_put_hex(222 / 16 = 13, 0)
| | | | 13 < 16 -> 再帰しない
| | | | write hex[13 % 16] = hex[13] = 'd'  <- 出力 (1)
| | | | return 0
| | | write hex[222 % 16] = hex[14] = 'e'   <- 出力 (2)
| | | return 0
| | write hex[3562 % 16] = hex[10] = 'a'    <- 出力 (3)
| | return 0
| write hex[57005 % 16] = hex[13] = 'd'     <- 出力 (4)
| return 0

出力順: 'd' -> 'e' -> 'a' -> 'd'
結果: "dead"

検算: 0xDEAD = 13*16^3 + 14*16^2 + 10*16^1 + 13*16^0
            = 53248 + 3584 + 160 + 13 = 57005
```

### 3.4 エラー伝播の完全フロー追跡

#### シナリオ1: stdout が閉じられている場合

```c
close(1);  /* stdout を閉じる */
int ret = ft_printf("Hello %d", 42);
```

```
ft_printf("Hello %d", 42) 開始
  format != NULL -> OK
  va_start(args, format)
  count = 0

  [pos=0] *format = 'H', '%' でない
    ft_print_char('H')
      ch = 'H'
      write(1, &ch, 1)
        -> fd=1 が閉じている
        -> write は -1 を返す（errno = EBADF）
      write == -1 -> true
      return (-1)
    ret = -1
    ret == -1 -> true!
    return (-1)  <- ft_printf が即座に -1 を返す

  'H' 以降の文字は一切処理されない
  va_end は呼ばれない（early return のため）

戻り値: -1
出力: なし
```

#### シナリオ2: 数値出力の途中でエラーが起きる場合

```
ft_printf("%d", 12345) で、'3' の出力時に write が失敗した場合:

ft_printf -> ft_print_format -> ft_print_nbr -> ft_put_nbr(12345)
  ft_put_nbr(1234)
    ft_put_nbr(123)
      ft_put_nbr(12)
        ft_put_nbr(1)
          write '1' -> OK
        write '2' -> OK
      write '3' -> FAIL! (-1)
      return (-1)    <- ft_put_nbr(123) が -1 を返す
    -1 を検出 -> return (-1)    <- ft_put_nbr(1234)
  -1 を検出 -> return (-1)      <- ft_put_nbr(12345)
ft_print_nbr: ft_put_nbr が -1 -> return (-1)
ft_print_format: -1 を返す
ft_printf: ret == -1 -> return (-1)

結果: "12" が出力された後にエラー。ft_printf は -1 を返す。
```

#### エラー伝播フロー図

```
                   成功パス                    エラーパス

write(1, &c, 1)   -> 1 (成功)                -> -1 (失敗)
        |                                            |
        v                                            v
ft_put_nbr         -> 0 (成功)                -> -1 (伝播)
        |                                            |
        v                                            v
ft_print_nbr       -> ft_numlen(n) (文字数)   -> -1 (伝播)
        |                                            |
        v                                            v
ft_print_format    -> 文字数 (>= 0)           -> -1 (伝播)
        |                                            |
        v                                            v
ft_printf          -> count += 文字数          -> return -1
        |
        v
ft_printf          -> return count
```

---

## 4. 設計の選択とその理由

### 4.1 なぜ桁数計算を別関数にするのか

**問題**: 再帰で出力する `ft_put_nbr` から桁数を返すのは困難です。

```c
/* 問題の本質: 戻り値が2つの意味を持てない */
static int ft_put_nbr(int n)
{
    /* ... 再帰で出力 ... */
    return (0);     /* 0 = 成功 */
    return (-1);    /* -1 = エラー */
    /* 桁数を返す余地がない! */
}
```

**解決策: 責務の分離（Separation of Concerns）**

```
ft_print_nbr(n)         -- 統括関数
  |
  +-- ft_put_nbr(n)     -- 責務: 出力だけ行う (0=成功, -1=エラー)
  |
  +-- ft_numlen(n)       -- 責務: 桁数だけ計算する (桁数を返す)
  |
  +-- return ft_numlen(n)
```

### 4.2 なぜ再帰を使うのか

数値を上位桁から出力するには、「逆順」の処理が必要です。

**方法1: バッファ方式**

```c
/* 配列に逆順に格納してから正順に出力 */
char buf[12];
int i = 0;
while (n > 0)
{
    buf[i++] = (n % 10) + '0';
    n /= 10;
}
/* buf を逆順に出力 */
while (--i >= 0)
    write(1, &buf[i], 1);
```

**方法2: 再帰方式（採用）**

```c
/* コールスタックが自然に逆順を実現 */
if (n >= 10)
    ft_put_nbr(n / 10);  /* 上位桁を先に */
write(1, &digit, 1);      /* 自分の桁を後に */
```

**比較:**

| 観点 | バッファ方式 | 再帰方式 |
|------|------------|---------|
| コード量 | やや多い | 少ない |
| Norm 25行制限 | 収まりにくい | 収まりやすい |
| 可読性 | やや低い | 高い |
| malloc | 不要（スタック配列） | 不要 |
| スタック使用量 | 少ない（配列のみ） | 再帰分（最大10段） |
| 性能 | 関数呼び出しなし | 関数呼び出しオーバーヘッド |

ft_printf では Norm の25行制限とコードの簡潔さから再帰方式が採用されています。

---

## 5. やってみよう: 実行トレースの練習

### 演習1: 16進数と unsigned の組み合わせ

`ft_printf("%x + %X = %u\n", 255, 255, 510)` を完全にトレースしてください。

ヒント: 255 の16進数は `ff`（小文字）/ `FF`（大文字）、510 の unsigned 出力は `510` です。

期待される出力: `ff + FF = 510\n`
期待される戻り値: 16

**チェックポイント:**
1. `%x` で ft_print_hex(255, 0) が呼ばれることを確認
2. `%X` で ft_print_hex(255, 1) が呼ばれることを確認
3. `%u` で ft_print_unsigned(510) が呼ばれることを確認
4. ft_put_hex(255, 0) の再帰展開を書き出す
5. 各 write の呼び出し順序を記録する
6. count の累積を追跡する

### 演習2: INT_MIN のトレース

`ft_printf("%d", -2147483648)` を ft_put_nbr の再帰を含めて完全にトレースしてください。

**チェックポイント:**
1. ft_put_nbr で INT_MIN 特別処理が発動することを確認
2. write(1, "-2147483648", 11) が呼ばれることを確認
3. ft_numlen(-2147483648) が 11 を返すことを確認
4. ft_printf の戻り値が 11 であることを確認

### 演習3: NULL ポインタと通常ポインタ

`ft_printf("%p%p", NULL, (void *)0xABC)` を完全にトレースしてください。

ヒント: NULL ポインタは `(nil)`（5文字）、0xABC は `0xabc`（5文字）と出力されます。

期待される出力: `(nil)0xabc`
期待される戻り値: 10

### 演習4: 全変換指定子を含むトレース

`ft_printf("%c%s%p%d%i%u%x%X%%", 'A', "BC", (void *)16, -1, 0, 42, 255, 255)` をトレースしてください。

期待される出力: `ABC0x10-1042ffFF%`

### 演習5: エラー伝播の思考実験

以下のコードを実行した場合、何が起きるか考えてください:

```c
close(1);
int ret = ft_printf("Hello %d World %s!", 42, "test");
```

**考えるポイント:**
1. 最初の `ft_print_char('H')` で何が起きるか
2. ft_printf の戻り値は何か
3. `va_end` は呼ばれるか
4. もし `close(1)` の代わりに、3文字目の出力で初めてエラーが起きたら、戻り値はどうなるか
