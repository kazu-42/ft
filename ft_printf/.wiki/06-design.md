# 06 - 設計原則

このセクションでは、ft_printf の設計判断の背後にある原則と理論を深く解説します。「なぜこの設計なのか」「他の選択肢はなかったのか」を理解することで、自分自身の設計能力を向上させることが目標です。

---

## 1. Modular Design（モジュール分割設計）

### 1.1 1ファイル1責務の原則

各ソースファイルは1つの変換指定子（または関連する変換指定子のグループ）のみを担当します。これはソフトウェア工学における **Single Responsibility Principle（単一責任の原則、SRP）** の適用です。

```
ft_printf.c          => 全体の制御と dispatch（司令塔）
ft_print_char.c      => %c, %% の出力（文字）
ft_print_str.c       => %s の出力（文字列）
ft_print_ptr.c       => %p の出力（ポインタ）
ft_print_nbr.c       => %d, %i の出力（符号付き整数）
ft_print_unsigned.c  => %u の出力（符号なし整数）
ft_print_hex.c       => %x, %X の出力（16進数）
```

### 1.2 モジュール分割の判断基準

なぜこの粒度（granularity）で分割したのか、判断基準を明確にします。

```
分割基準1: 変換指定子の型の違い
  %d と %i → 同じ int 型、同じ処理 → 同じファイル
  %x と %X → 同じ unsigned int 型、大文字/小文字の違いのみ → 同じファイル
  %d と %u → signed と unsigned で処理が異なる → 別ファイル

分割基準2: 42 Norm の制約
  1ファイルあたり最大5関数
  ft_print_nbr.c: ft_numlen + ft_put_nbr + ft_print_nbr = 3関数 → OK
  ft_print_hex.c: ft_hexlen + ft_put_hex + ft_print_hex = 3関数 → OK
  もし全部を1ファイルにまとめたら: 14関数 → Norm 違反！

分割基準3: 変更の影響範囲
  %p のバグを修正する場合 → ft_print_ptr.c だけを変更
  他のファイルに影響しない → 安全に修正可能

分割基準4: テスト容易性
  ft_print_nbr だけを個別にテストできる
  cc -c ft_print_nbr.c && テスト → 高速な開発サイクル
```

### 1.3 モジュール分割の利点の具体例

**利点1: 可読性**

```
悪い例（全て1ファイル）:
  ft_printf.c (300行)
  → "あの関数どこだっけ..." とスクロールが必要

良い例（モジュール分割）:
  ft_print_hex.c (53行)
  → ファイルを開いた瞬間に全体を把握可能
  → ft_hexlen, ft_put_hex, ft_print_hex の3関数だけ
```

**利点2: 保守性（Maintainability）**

```
シナリオ: "%p" で NULL の出力が "(nil)" でなく "0x0" であるべきだと判明

修正手順:
  1. ft_print_ptr.c を開く
  2. ft_print_ptr 関数の ptr == 0 の分岐を修正
  3. 他のファイルは一切触らない
  4. ft_print_ptr.c だけ再コンパイル

もし全て1ファイルだったら:
  1. 300行のファイルから該当箇所を探す
  2. 修正時に別の関数を誤って壊すリスクがある
  3. ファイル全体を再コンパイル
```

**利点3: Norm 準拠**

```
42 Norm の制約:
  - 1ファイルあたり最大5関数
  - 1関数あたり最大25行

ファイルごとの関数数:
  ft_printf.c:          2関数 (ft_printf, ft_print_format)
  ft_print_char.c:      1関数 (ft_print_char)
  ft_print_str.c:       2関数 (ft_strlen, ft_print_str)
  ft_print_ptr.c:       3関数 (ft_ptr_len, ft_put_ptr, ft_print_ptr)
  ft_print_nbr.c:       3関数 (ft_numlen, ft_put_nbr, ft_print_nbr)
  ft_print_unsigned.c:  3関数 (ft_unumlen, ft_put_unsigned, ft_print_unsigned)
  ft_print_hex.c:       3関数 (ft_hexlen, ft_put_hex, ft_print_hex)

  全ファイルが5関数以内 → Norm 準拠
```

**利点4: 拡張性（Extensibility）**

```
bonus で flag を追加する場合:

  変更が必要なファイル:
    ft_printf.c → flag の parse 処理を追加
    各 ft_print_*.c → flag に応じた padding/precision を追加

  変更が不要なファイル:
    ft_print_char.c → %c には flag が少ない（width くらい）
    → 不要なファイルは触らなくてよい

  新規ファイルの追加:
    ft_parse_flags.c → flag 解析関数を追加
    → 既存のコードに影響を与えず機能追加可能
```

### 1.4 モジュール間のインターフェース設計

```
各モジュールの公開インターフェース（ft_printf.h）:

  int  ft_printf(const char *format, ...);
  int  ft_print_char(int c);
  int  ft_print_str(char *str);
  int  ft_print_ptr(unsigned long long ptr);
  int  ft_print_nbr(int n);
  int  ft_print_unsigned(unsigned int n);
  int  ft_print_hex(unsigned int n, int uppercase);

共通の規約:
  戻り値: 成功時は出力文字数（>= 0）、失敗時は -1
  副作用: fd=1（stdout）に write する

この統一されたインターフェースにより:
  1. ft_print_format からの呼び出しが一貫性を持つ
  2. 新しい出力関数を追加する際のテンプレートが明確
  3. エラーハンドリングのパターンが統一される
```

---

## 2. Function Dispatch Pattern（関数ディスパッチパターン）

### 2.1 Dispatcher とは

format string 中の変換指定子に応じて、適切な処理関数を呼び出すパターンです。これはデザインパターンにおける **Strategy Pattern（ストラテジーパターン）** の一種です。

```c
/* 本実装のディスパッチャ（if-else chain） */
static int ft_print_format(va_list args, char specifier)
{
    int count;

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

### 2.2 なぜ if-else chain なのか --- 関数ポインタ配列との比較

42 の Norm では関数ポインタ（function pointer）の配列を使った dispatch table も可能ですが、if-else chain の方が ft_printf では優れています。その理由を、実際のコードで比較しながら解説します。

**方式A: if-else chain（本実装で採用）**

```c
static int ft_print_format(va_list args, char specifier)
{
    int count;

    count = 0;
    if (specifier == 'c')
        count = ft_print_char(va_arg(args, int));
    else if (specifier == 's')
        count = ft_print_str(va_arg(args, char *));
    /* ... */
    return (count);
}
```

**方式B: 関数ポインタ配列（Dispatch Table）**

```c
/* 方式B の実装例（参考） */

/* 全ての handler が同じシグネチャを持つ必要がある */
typedef int (*t_handler)(va_list args);

static int handle_char(va_list args)
{
    return (ft_print_char(va_arg(args, int)));
}

static int handle_str(va_list args)
{
    return (ft_print_str(va_arg(args, char *)));
}

static int handle_ptr(va_list args)
{
    return (ft_print_ptr(va_arg(args, unsigned long long)));
}

static int handle_int(va_list args)
{
    return (ft_print_nbr(va_arg(args, int)));
}

static int handle_unsigned(va_list args)
{
    return (ft_print_unsigned(va_arg(args, unsigned int)));
}

static int handle_hex_lower(va_list args)
{
    return (ft_print_hex(va_arg(args, unsigned int), 0));
}

static int handle_hex_upper(va_list args)
{
    return (ft_print_hex(va_arg(args, unsigned int), 1));
}

static int handle_percent(va_list args)
{
    (void)args;  /* 引数を使わない */
    return (ft_print_char('%'));
}

/* dispatch table の初期化 */
static int ft_print_format(va_list args, char specifier)
{
    t_handler handlers[128];
    int       i;

    i = 0;
    while (i < 128)
        handlers[i++] = NULL;
    handlers['c'] = handle_char;
    handlers['s'] = handle_str;
    handlers['p'] = handle_ptr;
    handlers['d'] = handle_int;
    handlers['i'] = handle_int;
    handlers['u'] = handle_unsigned;
    handlers['x'] = handle_hex_lower;
    handlers['X'] = handle_hex_upper;
    handlers['%'] = handle_percent;
    if (specifier >= 0 && specifier < 128 && handlers[(int)specifier])
        return (handlers[(int)specifier](args));
    return (0);
}
```

**比較表:**

| 観点 | if-else chain (方式A) | Dispatch Table (方式B) |
|------|----------------------|----------------------|
| コード量 | 少ない（1関数20行） | 多い（wrapper関数8個 + 初期化） |
| 型安全性 | 高い（各va_argで正しい型を直接指定） | 低い（全handler同シグネチャ強制） |
| Norm 準拠 | 容易（1関数で完結） | 困難（wrapper関数が多く5関数制限に抵触） |
| 可読性 | 高い（specifier と handler の対応が明確） | やや低い（間接参照が入る） |
| 拡張性 | 普通（else if を追加） | 高い（配列に1行追加） |
| 実行時性能 | O(n) で条件分岐 | O(1) で配列アクセス |
| メモリ使用 | なし | 128 * sizeof(pointer) = 1024バイト |

**型安全性の問題が致命的:**

```c
/* 方式B の根本的な問題: va_arg の型が handler ごとに異なる */

typedef int (*t_handler)(va_list args);

/* 全ての handler が (va_list args) を受け取る共通シグネチャ */
/* しかし、va_arg で取得する型が異なる: */
/*   handle_char:    va_arg(args, int)                */
/*   handle_str:     va_arg(args, char *)             */
/*   handle_ptr:     va_arg(args, unsigned long long)  */

/* 方式A では、各 va_arg の型が明示的に書かれている: */
if (specifier == 'c')
    count = ft_print_char(va_arg(args, int));        /* int で取得 */
else if (specifier == 's')
    count = ft_print_str(va_arg(args, char *));      /* char * で取得 */

/* 型の違いがコード上で一目瞭然 */
```

**結論:** ft_printf の mandatory part では、if-else chain が最もバランスの良い選択です。dispatch table は specifier の数が多い場合（bonus でフラグが増える場合など）にメリットがありますが、mandatory の8種類では過剰設計（over-engineering）です。

### 2.3 Dispatch Table が有効なケース

参考として、dispatch table が有効になるケースも紹介します。

```c
/* ケース: 大量の specifier がある場合（仮想的な例） */
/* 26個の小文字全てが specifier で、全て同じ型（int）を取る場合 */

typedef int (*t_handler)(int);
t_handler handlers[128];

handlers['a'] = handle_a;
handlers['b'] = handle_b;
/* ... */
handlers['z'] = handle_z;

/* この場合、if-else が26個並ぶより dispatch table の方が良い */
/* しかし ft_printf では specifier は8種類だけなので不要 */
```

---

## 3. Recursion vs Iteration（再帰 vs 反復）

### 3.1 数値出力における再帰の必然性

数値を上位桁から出力するために、本実装では再帰（recursion）を使用しています。なぜ再帰が自然な選択なのかを、数学的背景から解説します。

```
問題: 数値 12345 を上位桁から1桁ずつ出力したい

数学的分解:
  12345 = 1 * 10^4 + 2 * 10^3 + 3 * 10^2 + 4 * 10^1 + 5 * 10^0

% 演算で得られるのは「最下位桁」:
  12345 % 10 = 5  ← 最後に出力したい桁
  1234  % 10 = 4
  123   % 10 = 3
  12    % 10 = 2
  1     % 10 = 1  ← 最初に出力したい桁

つまり、% で取得できる順序と出力したい順序が「逆」

この「逆順」問題を解決する方法:
  方法1: バッファに貯めて逆順に出力
  方法2: 再帰のコールスタックで自然に逆順を実現
  方法3: 桁数を先に計算して10^n で上位桁から取り出す
```

### 3.2 再帰方式の動作原理

```c
/* 再帰方式の本質 */
void ft_putnbr_rec(int n)
{
    if (n >= 10)
        ft_putnbr_rec(n / 10);  /* (A) 上位桁を先に処理 */
    char c = (n % 10) + '0';
    write(1, &c, 1);            /* (B) 自分の桁を出力 */
}
```

```
なぜ再帰で正しい順序になるのか:

ft_putnbr_rec(123) の実行順序:

  ft_putnbr_rec(123)
    (A) ft_putnbr_rec(12)     ← 上位桁を先に処理（再帰）
      (A) ft_putnbr_rec(1)   ← さらに上位を処理（再帰）
        1 < 10 → (A) をスキップ
        (B) write '1'         ← 最上位桁が最初に出力される
      (B) write '2'           ← 再帰から戻った後に2桁目を出力
    (B) write '3'             ← 最下位桁が最後に出力される

  出力: "123" ← 正しい順序！

原理:
  再帰呼び出し (A) は「上位桁の出力を待つ」効果がある
  コールスタックが LIFO（Last In, First Out）なので、
  最も深い呼び出し（最上位桁）が最初に write を実行する
```

### 3.3 反復（iterative）方式との詳細比較

**反復方式1: バッファ方式**

```c
void ft_putnbr_buf(unsigned int n)
{
    char buf[11];  /* unsigned int は最大10桁 + '\0' */
    int  i;

    if (n == 0)
    {
        write(1, "0", 1);
        return ;
    }
    i = 0;
    while (n > 0)
    {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    while (--i >= 0)
        write(1, &buf[i], 1);
}
/* 行数: 約15行 → Norm 25行には収まるが余裕が少ない */
/* エラーハンドリングを加えると25行を超える可能性大 */
```

**反復方式2: 除算方式**

```c
void ft_putnbr_div(unsigned int n)
{
    unsigned int divisor;
    char         c;

    if (n == 0)
    {
        write(1, "0", 1);
        return ;
    }
    divisor = 1;
    while (divisor <= n / 10)
        divisor *= 10;
    while (divisor > 0)
    {
        c = (n / divisor) + '0';
        write(1, &c, 1);
        n %= divisor;
        divisor /= 10;
    }
}
/* 行数: 約17行 → エラーハンドリングを加えると25行を超える */
/* divisor のオーバーフローに注意が必要 */
```

**再帰方式（本実装）**

```c
static int ft_put_nbr(int n)
{
    char c;

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
        n = -n;
    }
    if (n >= 10)
    {
        if (ft_put_nbr(n / 10) == -1)
            return (-1);
    }
    c = (n % 10) + '0';
    if (write(1, &c, 1) == -1)
        return (-1);
    return (0);
}
/* 行数: 24行 → Norm 25行にギリギリ収まる */
/* INT_MIN処理 + 負数処理 + エラーハンドリング全て含む */
```

**総合比較:**

| 観点 | バッファ方式 | 除算方式 | 再帰方式 |
|------|------------|---------|---------|
| Norm 25行 | 厳しい | 厳しい | ギリギリ収まる |
| エラー処理込み25行 | 超える | 超える | 収まる |
| 追加のローカル変数 | buf[11], i | divisor | c のみ |
| 理解しやすさ | 中（逆順ロジック） | 低（除算ロジック） | 高（自然な分解） |
| スタック使用 | 固定11バイト | 固定 | 最大10フレーム |
| オーバーフローリスク | なし | divisor計算時 | なし |

### 3.4 再帰の安全性分析

```
ft_put_nbr (int):
  int の最大桁数: 10桁（2147483647）
  → 最大再帰深度: 10
  → 最大スタック使用: 約 320 バイト

ft_put_unsigned (unsigned int):
  unsigned int の最大桁数: 10桁（4294967295）
  → 最大再帰深度: 10
  → 最大スタック使用: 約 320 バイト

ft_put_hex (unsigned int):
  16進数の最大桁数: 8桁（FFFFFFFF）
  → 最大再帰深度: 8
  → 最大スタック使用: 約 256 バイト

ft_put_ptr (unsigned long long):
  64ビットアドレスの最大桁数: 16桁
  → 最大再帰深度: 16（二分木再帰なのでやや多い）
  → 最大スタック使用: 約 512 バイト

デフォルトのスタックサイズ: 約 8MB (8,388,608 バイト)
最悪ケースの使用量: 約 512 バイト

結論: スタックオーバーフローのリスクは皆無
```

---

## 4. Error Handling in C（C言語のエラーハンドリングパターン）

### 4.1 戻り値によるエラー伝播パターン

C 言語には例外（exception）機構がないため、エラーは戻り値で通知します。ft_printf では以下の規約を統一的に適用しています。

```
ft_printf のエラーハンドリング規約:

  成功: 出力した文字数（>= 0）を返す
  失敗: -1 を返す

この規約は POSIX のシステムコール（write, read 等）と同じ:
  write() → 成功: 書き込んだバイト数、失敗: -1
  read()  → 成功: 読み取ったバイト数、失敗: -1
```

### 4.2 エラーチェックの3つのパターン

ft_printf 内で使われるエラーチェックパターンを分類します。

**パターン1: write の直接チェック**

```c
/* 最も基本的なパターン */
if (write(1, &ch, 1) == -1)
    return (-1);

/* 使用箇所: ft_print_char, ft_print_str, ft_put_nbr 等 */
/* write が -1 を返したら、即座に -1 を返す */
```

**パターン2: 再帰呼び出しのチェック**

```c
/* 再帰の戻り値をチェック */
if (ft_put_nbr(n / 10) == -1)
    return (-1);

/* 使用箇所: ft_put_nbr, ft_put_unsigned, ft_put_hex, ft_put_ptr */
/* 再帰先でエラーが起きたら、自分もエラーを返す */
```

**パターン3: 出力関数の戻り値チェック**

```c
/* ft_printf 内での統括チェック */
ret = ft_print_format(args, *format);
if (ret == -1)
    return (-1);
count += ret;

/* ft_print_format が -1 を返したら即座にエラー */
/* そうでなければ count に加算 */
```

### 4.3 エラー伝播チェーンの全体像

```
層                          成功時の戻り値      失敗時の戻り値
===                         ==============      ==============

Layer 4: write()            書き込みバイト数     -1
                                 |                    |
                                 v                    v
Layer 3: ft_put_XXX()       0 (成功)             -1 (伝播)
                                 |                    |
                                 v                    v
Layer 2: ft_print_XXX()     出力文字数            -1 (伝播)
                                 |                    |
                                 v                    v
Layer 1: ft_print_format()  出力文字数            -1 (伝播)
                                 |                    |
                                 v                    v
Layer 0: ft_printf()        count += 文字数       return -1
                                 |
                                 v
                            return count
```

### 4.4 他の言語との比較

```
C 言語（ft_printf の方式）:
  int ret = ft_put_nbr(n);
  if (ret == -1)
      return (-1);
  → 毎回チェックが必要。冗長だが明示的。

C++（例外を使う場合）:
  try {
      put_nbr(n);  // 失敗時は例外をスロー
  } catch (const std::exception& e) {
      return -1;
  }
  → try-catch で一括処理。エラーパスが分離される。

Rust（Result型を使う場合）:
  put_nbr(n)?;  // ? 演算子でエラーを自動伝播
  → 1文字でエラー伝播。コンパイル時に型チェック。

Go（複数戻り値）:
  err := putNbr(n)
  if err != nil {
      return -1, err
  }
  → C に似ているが、エラーと値を別々に返せる。
```

C 言語のエラーハンドリングは冗長ですが、「何が起きているか」が明示的で、制御フローが追いやすいという利点があります。

### 4.5 va_end が呼ばれないケースの分析

```c
int ft_printf(const char *format, ...)
{
    va_list args;

    va_start(args, format);  /* ← ここで初期化 */
    /* ... */
    if (ret == -1)
        return (-1);         /* ← va_end を呼ばずに return! */
    /* ... */
    va_end(args);            /* ← 正常パスでのみ到達 */
    return (count);
}
```

```
問題: エラー時に va_end が呼ばれない

C 標準（C99 7.15.1）の規定:
  "va_start マクロに対応する va_end マクロの呼び出しなしに
   関数から return してはならない。動作は未定義である。"

実際の影響（x86_64 Linux / macOS）:
  va_end は以下のように定義されていることが多い:
    #define va_end(ap) ((void)0)
  → 実質的に何もしない！
  → undefined behavior ではあるが、実害はない

厳密に修正する場合:
  int ft_printf(const char *format, ...)
  {
      va_list args;
      int     count;
      int     ret;
      int     error;

      va_start(args, format);
      error = 0;
      count = 0;
      while (*format && !error)
      {
          /* ... */
          if (ret == -1)
              error = 1;
          else
              count += ret;
          format++;
      }
      va_end(args);
      if (error)
          return (-1);
      return (count);
  }
  → しかし、これは25行制限を超えてしまう！

42 Norm との現実的な妥協:
  - va_end なしの early return を許容する
  - 実害がないことは検証済み
  - コードの簡潔さを優先する
```

---

## 5. Return Value Accumulation（戻り値の累積パターン）

### 5.1 設計方針

各出力関数は「自分が出力した文字数」を返します。呼び出し側はそれを累積して最終的な戻り値とします。

```c
/* 各関数のインターフェース */
int ft_print_char(int c);           /* 常に 1 を返す */
int ft_print_str(char *str);        /* strlen(str) を返す */
int ft_print_nbr(int n);            /* 桁数（符号含む）を返す */
int ft_print_unsigned(unsigned int n);  /* 桁数を返す */
int ft_print_hex(unsigned int n, int uppercase);  /* 桁数を返す */
int ft_print_ptr(unsigned long long ptr);  /* "0x" + 桁数 を返す */
```

```c
/* ft_printf での累積 */
count = 0;
/* ... */
count += ft_print_char(*format);        /* 通常文字: +1 */
count += ft_print_format(args, *format); /* 変換指定子: +N */
/* ... */
return (count);
```

### 5.2 エラー伝播との両立

`-1` をエラー値として予約し、正常時は非負の値（出力文字数）を返します。

```c
ret = ft_print_nbr(va_arg(args, int));
if (ret == -1)
    return (-1);  /* エラー伝播: -1 を count に加算してはいけない */
count += ret;     /* 正常時のみ累積 */
```

```
なぜ ret == -1 のチェックが必要か:

もしチェックしなかったら:
  ret = -1 (エラー)
  count += -1  → count が1減る
  → 最終的に間違った count を返してしまう
  → さらに write が失敗し続けても処理が続行される

チェックすることで:
  ret = -1 (エラー)
  return (-1)  → 即座にエラーを返す
  → 呼び出し元がエラーを検知できる
```

### 5.3 出力関数の戻り値の計算方法

**ft_print_nbr の戻り値の計算（2段階アプローチ）:**

```
Step 1: ft_put_nbr(n) で実際に出力する（戻り値は 0 or -1）
Step 2: ft_numlen(n) で文字数を計算する（戻り値は桁数）

なぜ2段階なのか:
  ft_put_nbr は再帰関数で、戻り値を「成功/失敗」に使っている
  桁数を同時に返すことができない（戻り値は1つしかない）
  → 別関数 ft_numlen で桁数を計算する

ft_print_nbr(n):
  if (ft_put_nbr(n) == -1)  ← Step 1: 出力
      return (-1);
  return (ft_numlen(n));     ← Step 2: 桁数を返す
```

---

## 6. Memory Safety（メモリ安全性）

### 6.1 malloc 不要の設計

mandatory part では `malloc` を一切使用しません。これは意図的な設計判断です。

```
数値の文字列変換における3つの方式:

方式1: malloc + sprintf（非推奨）
  char *str = malloc(12);
  sprintf(str, "%d", n);
  write(1, str, strlen(str));
  free(str);
  → malloc 失敗時の処理が必要
  → free を忘れるとメモリリーク
  → そもそも sprintf を使うなら ft_printf の意味がない

方式2: スタック上のバッファ（反復方式で使用）
  char buf[12];
  /* buf に数字を格納 */
  → malloc 不要
  → 関数終了時に自動解放
  → バッファサイズの計算が必要

方式3: 再帰（本実装で採用）
  if (n >= 10)
      ft_put_nbr(n / 10);
  write(1, &digit, 1);
  → malloc 不要
  → バッファ不要
  → コールスタックを「バッファ」として利用
```

### 6.2 malloc を使わない利点

```
1. メモリリーク（memory leak）のリスクがゼロ
   → free を呼ぶ必要がない
   → free を忘れるバグが発生しない

2. エラーハンドリングが単純
   → malloc 失敗時のパスを考慮しなくてよい
   → malloc が NULL を返した場合の分岐が不要

3. パフォーマンス
   → heap allocation のオーバーヘッドがない
   → malloc は内部でシステムコール（brk/mmap）を呼ぶことがある
   → 再帰のスタック使用はヒープよりも高速

4. スレッド安全性
   → malloc はスレッド間でヒープの競合が発生しうる
   → スタックは各スレッドに独立して割り当てられる
   → 本実装では write の thread safety に依存するが、
      malloc による追加のリスクはない
```

### 6.3 再帰の stack 使用量の詳細分析

```
各関数の最大再帰深度と stack 使用量:

ft_put_nbr (int):
  最大再帰深度: 10（int は最大10桁）
  1フレームあたり: 約 32 バイト（引数 + ローカル変数 + リターンアドレス）
  最大使用量: 10 * 32 = 320 バイト

ft_put_unsigned (unsigned int):
  最大再帰深度: 10（unsigned int は最大10桁）
  最大使用量: 10 * 32 = 320 バイト

ft_put_hex (unsigned int):
  最大再帰深度: 8（16進数は最大8桁: FFFFFFFF）
  最大使用量: 8 * 32 = 256 バイト

ft_put_ptr (unsigned long long):
  最大再帰深度: 16（16進数は最大16桁: FFFFFFFFFFFFFFFF）
  ※ 二分木再帰なので実際のフレーム数はやや多い
  最大使用量: 約 512 バイト

合計最悪ケース（1つの ft_printf 呼び出しで全種類を使う場合）:
  ft_printf のフレーム + ft_print_format のフレーム + 最深の再帰
  = 約 64 + 32 + 512 = 608 バイト

デフォルトスタックサイズ: 8MB = 8,388,608 バイト
使用率: 608 / 8,388,608 = 0.007%

→ スタックオーバーフローの心配は完全に不要
```

---

## 7. Extensibility（拡張性）

### 7.1 Bonus への対応設計

bonus part で flag（`-`, `0`, `.`, `#`, `+`, ` `）と field width を追加する場合の拡張方針です。

### 7.2 構造体による flag 管理

```c
typedef struct s_flags
{
    int     minus;      /* '-' flag: 左寄せ */
    int     zero;       /* '0' flag: ゼロ埋め */
    int     hash;       /* '#' flag: 0x prefix 等 */
    int     space;      /* ' ' flag: 正数の前にスペース */
    int     plus;       /* '+' flag: 正数の前に + */
    int     width;      /* field width: 最小表示幅 */
    int     precision;  /* '.' precision: 精度 */
    int     has_prec;   /* precision が指定されたか */
}   t_flags;
```

### 7.3 拡張時の変更点

```
1. ft_printf.c の変更:
   '%' の後に flag / width / precision を parse する処理を追加

   変更前:
     if (*format == '%')
     {
         format++;
         ret = ft_print_format(args, *format);
     }

   変更後:
     if (*format == '%')
     {
         format++;
         flags = ft_parse_flags(&format);  /* 新規追加 */
         ret = ft_print_format(args, *format, flags);  /* flags を渡す */
     }

2. 各出力関数の変更:
   t_flags を引数に追加し、padding / precision を適用

   変更前: int ft_print_nbr(int n);
   変更後: int ft_print_nbr(int n, t_flags flags);

3. 新規ファイルの追加:
   ft_parse_flags.c → flag 解析用関数

mandatory part を modular に設計しておけば:
  - ft_printf.c に parse 処理を追加
  - 各 ft_print_*.c に flags 引数を追加
  - 新しい ft_parse_flags.c を追加
  のように、既存コードへの変更が最小限で済む
```

### 7.4 mandatory 設計が bonus を容易にする理由

```
良い設計（本実装）:
  ft_print_nbr(int n)
    → ft_print_nbr(int n, t_flags flags) に変更するだけ
    → 関数の役割は変わらない（数値を出力する）
    → flags に応じた padding を追加するだけ

悪い設計（仮にモノリシックだったら）:
  ft_printf の中に全ての処理がべた書き
    → flag 処理をどこに入れるか不明
    → 条件分岐が複雑に入り組む
    → バグが入りやすい

つまり:
  mandatory で「1関数1責務」を守っておくと、
  bonus は「各関数に機能を足す」だけで済む

  mandatory で「全部入り」にしてしまうと、
  bonus は「既存の巨大関数をリファクタリング」してから機能追加
  → 工数が大幅に増える
```

---

## 8. Makefile の設計

### 8.1 本実装の Makefile の解説

```makefile
NAME    = libftprintf.a          # 生成するライブラリ名

CC      = cc                     # コンパイラ
CFLAGS  = -Wall -Wextra -Werror  # コンパイルフラグ

SRCS    = ft_printf.c \          # ソースファイル一覧
          ft_print_char.c \
          ft_print_str.c \
          ft_print_ptr.c \
          ft_print_nbr.c \
          ft_print_unsigned.c \
          ft_print_hex.c

OBJS    = $(SRCS:.c=.o)         # .c を .o に置換

all: $(NAME)                     # デフォルトターゲット

$(NAME): $(OBJS)                 # ライブラリの生成ルール
    ar rcs $(NAME) $(OBJS)       # 静的ライブラリにアーカイブ

%.o: %.c ft_printf.h             # パターンルール（暗黙のルール）
    $(CC) $(CFLAGS) -c $< -o $@  # .c から .o を生成

clean:                           # オブジェクトファイルの削除
    rm -f $(OBJS)

fclean: clean                    # ライブラリも削除
    rm -f $(NAME)

re: fclean all                   # クリーンビルド

bonus: all                       # bonus は all と同じ（mandatory のみの場合）

.PHONY: all clean fclean re bonus  # 偽ターゲットの宣言
```

### 8.2 relink 防止の仕組み

```
relink とは: 変更がないのに再ビルドすること

%.o: %.c ft_printf.h
  → .c ファイルまたは ft_printf.h が変更された場合のみ再コンパイル

$(NAME): $(OBJS)
  → .o ファイルが更新された場合のみ ar で再アーカイブ

検証方法:
  $ make           # 初回ビルド（全ファイルコンパイル）
  $ make           # 2回目（何もしない = relink しない）
  make: 'libftprintf.a' is up to date.

  $ touch ft_printf.c   # ft_printf.c のタイムスタンプを更新
  $ make           # ft_printf.c だけ再コンパイル + 再アーカイブ
```

---

## 9. 設計原則のまとめ

```
ft_printf の設計で適用された原則:

1. Single Responsibility Principle（単一責任の原則）
   → 1ファイル1変換、1関数1処理

2. Separation of Concerns（関心の分離）
   → 出力処理と桁数計算を分離

3. Information Hiding（情報隠蔽）
   → static 関数で実装詳細を隠蔽

4. Uniform Interface（統一インターフェース）
   → 全出力関数が「文字数 or -1」を返す

5. Defensive Programming（防御的プログラミング）
   → NULL チェック、write エラーチェック

6. KISS（Keep It Simple, Stupid）
   → if-else chain で十分、dispatch table は不要
   → malloc なしで実装可能なら malloc を使わない

7. YAGNI（You Ain't Gonna Need It）
   → mandatory で bonus の機能を先取りしない
   → 拡張可能な構造だけ用意しておく

これらの原則は ft_printf に限らず、
42 の全プロジェクトで応用できる普遍的な設計指針です。
```
