# 08 - 学習目標

## 概要

ft_printf プロジェクトは、42 カリキュラムにおいて C 言語の中核的な能力を体系的に鍛える重要なマイルストーンです。このプロジェクトを通じて、以下の **4 つの主要な学習目標** を達成します。

1. **可変長引数（Variadic Functions）のマスタリー**
2. **型システム（Type System）の深い理解**
3. **ライブラリ作成スキル（Library Building）の習得**
4. **フォーマット文字列解析（Format String Parsing）の実践**

これらの目標は独立しているようで、実は密接に結びついています。可変長引数を正しく扱うには型システムの理解が不可欠であり、ライブラリとして提供するにはビルドシステムの知識が必要であり、フォーマット文字列の解析はプログラミング全般の基礎となるパーサー設計能力を養います。

本章では各学習目標について、**なぜ学ぶのか**、**何を学ぶのか**、**どこまでできれば合格か**、**後続課題とどう接続するか**、そして**自己評価チェックリスト**を詳述します。

---

## 1. 可変長引数（Variadic Functions）のマスタリー

### 1.1 学ぶべきこと

可変長引数（variadic functions）とは、引数の個数が固定されていない関数のことです。C 言語では `<stdarg.h>` ヘッダが提供するマクロ群を用いて実現します。

#### 基本マクロの詳細

| マクロ | 目的 | 詳細 |
|--------|------|------|
| `va_list` | 可変長引数リストを表す型 | 内部的にはポインタまたは構造体として実装されている。引数群を走査するための「カーソル」の役割を果たす |
| `va_start(ap, last)` | `va_list` の初期化 | `last` は最後の固定引数を指定する。これを起点として可変長引数の位置を計算する |
| `va_arg(ap, type)` | 次の引数を取得 | `type` で指定した型として引数を読み取り、内部ポインタを次の引数位置に進める |
| `va_end(ap)` | `va_list` の後処理 | `va_start` で確保したリソースを解放する。呼ばないと undefined behavior（未定義動作）の可能性がある |
| `va_copy(dest, src)` | `va_list` のコピー | 同じ引数リストを複数回走査したい場合に使う。`dest` は独立した `va_list` となる |

#### 使用パターンの詳細

**基本パターン:**

```c
int my_func(int count, ...)
{
    va_list args;
    va_start(args, count);      // count を起点に初期化
    for (int i = 0; i < count; i++)
    {
        int val = va_arg(args, int);  // int として取り出す
        // val を使った処理
    }
    va_end(args);               // 必ず呼ぶ
    return (0);
}
```

**ft_printf での使用パターン:**

```c
int ft_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);    // format を起点に初期化
    // format 文字列を走査し、specifier に応じて
    // va_arg(args, int), va_arg(args, char *) 等で引数を取得
    va_end(args);
    return (count);
}
```

**sentinel パターン（番兵値方式）:**

```c
// 最後の引数に NULL を置いて終了を検出するパターン
int concat_strings(char *first, ...)
{
    va_list args;
    char    *str;

    va_start(args, first);
    str = first;
    while (str != NULL)
    {
        // str を使った処理
        str = va_arg(args, char *);
    }
    va_end(args);
    return (0);
}
// 呼び出し例: concat_strings("Hello", " ", "World", NULL);
```

#### Default Argument Promotion（デフォルト引数昇格）

可変長引数を使う上で最も重要な概念が **default argument promotion** です。これは C 言語の仕様で、variadic function に渡される引数が自動的に昇格（promote）されるルールです。

| 元の型 | 昇格後の型 | 理由 |
|--------|-----------|------|
| `char` | `int` | 整数昇格（integer promotion） |
| `signed char` | `int` | 整数昇格 |
| `unsigned char` | `int` | 整数昇格 |
| `short` | `int` | 整数昇格 |
| `unsigned short` | `int` | 整数昇格（`int` で表現可能な場合） |
| `float` | `double` | 浮動小数点昇格 |
| `int` | `int` | 変化なし |
| `unsigned int` | `unsigned int` | 変化なし |
| `long` | `long` | 変化なし |
| `void *` | `void *` | 変化なし |

**重要:** `va_arg(args, char)` や `va_arg(args, short)` は **未定義動作（undefined behavior）** です。必ず昇格後の型を指定してください。

```c
// 正しい例
char c = (char)va_arg(args, int);       // int として取り出してから cast

// 間違い - 未定義動作
char c = va_arg(args, char);            // これは NG!
```

ft_printf のコードでは、`ft_print_char` が `int` 型の引数を受け取り、内部で `unsigned char` に cast しています。これは default argument promotion を正しく考慮した設計です。

```c
// ft_print_char.c より
int ft_print_char(int c)
{
    unsigned char ch;
    ch = (unsigned char)c;
    if (write(1, &ch, 1) == -1)
        return (-1);
    return (1);
}
```

#### va_copy の用途と重要性

`va_copy` は ft_printf の mandatory part では直接使いませんが、理解しておくべき重要なマクロです。

```c
void example(int count, ...)
{
    va_list args;
    va_list args_copy;

    va_start(args, count);
    va_copy(args_copy, args);    // args の現在位置をコピー

    // 1回目の走査: 合計値を計算
    int sum = 0;
    for (int i = 0; i < count; i++)
        sum += va_arg(args, int);

    // 2回目の走査: args_copy を使って別の処理
    for (int i = 0; i < count; i++)
        printf("%d ", va_arg(args_copy, int));

    va_end(args);
    va_end(args_copy);           // va_copy で作ったものも va_end が必要
}
```

**使用場面の例:**
- bonus part で width を計算するために一度引数を読み、もう一度出力するために読む場合
- 出力前に全体の長さを計算したい場合
- デバッグ用に引数リストを2回走査したい場合

#### va_start の第2引数がなぜ最後の固定引数なのか

`va_start` の第2引数に最後の固定引数を渡す理由は、コンパイラがスタック上の可変長引数の開始位置を計算するための基準点にするためです。

```c
int ft_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);  // format のアドレスを基準に
                              // 可変長引数の開始位置を算出
    // ...
    va_end(args);
}
```

スタックフレーム上のメモリレイアウト（概念図）:
```
高アドレス
  +------------------+
  | 第3可変引数       |
  +------------------+
  | 第2可変引数       |
  +------------------+
  | 第1可変引数       |
  +------------------+  <- va_start はここを計算する
  | format (固定引数) |  <- 基準点
  +------------------+
  | リターンアドレス   |
  +------------------+
低アドレス
```

### 1.2 なぜ重要か

variadic functions は C 言語の強力な機能で、以下のような場面で広く使われます。

#### 標準ライブラリでの利用

| 関数 | プロトタイプ | 用途 |
|------|------------|------|
| `printf` | `int printf(const char *format, ...)` | 書式付き出力 |
| `fprintf` | `int fprintf(FILE *stream, const char *format, ...)` | ファイルへの書式付き出力 |
| `sprintf` | `int sprintf(char *str, const char *format, ...)` | 文字列への書式付き出力 |
| `snprintf` | `int snprintf(char *str, size_t size, const char *format, ...)` | サイズ制限付き文字列出力 |
| `scanf` | `int scanf(const char *format, ...)` | 書式付き入力 |
| `open` | `int open(const char *path, int flags, ...)` | ファイルオープン（mode 引数がオプション） |
| `execl` | `int execl(const char *path, const char *arg, ...)` | プログラム実行 |
| `ioctl` | `int ioctl(int fd, unsigned long request, ...)` | デバイス制御 |
| `fcntl` | `int fcntl(int fd, int cmd, ...)` | ファイル記述子操作 |

#### 実務での利用場面

1. **ログ関数（Logging）**: アプリケーションのログを `printf` 風に出力する関数
   ```c
   void log_error(const char *format, ...);
   void log_debug(const char *format, ...);
   void log_info(const char *format, ...);
   ```

2. **エラーハンドリング**: エラーメッセージをフォーマットして出力
   ```c
   void fatal(const char *format, ...);
   void warn_user(const char *format, ...);
   ```

3. **ラッパー関数**: 既存の variadic function をラップする
   ```c
   int my_printf(const char *format, ...);
   int safe_snprintf(char *buf, size_t size, const char *format, ...);
   ```

4. **汎用コールバック**: 可変個の引数を受け取るコールバック機構

5. **シリアライゼーション**: 可変個のフィールドをバイト列に変換

6. **テスト用アサーション**: テストメッセージの書式付き出力
   ```c
   void test_assert(int condition, const char *format, ...);
   ```

#### 42 カリキュラムでの位置づけ

ft_printf は 42 カリキュラムの中で **最初に variadic functions を扱うプロジェクト** です。ここで得た知識は、後の課題で必須の基礎となります。特に minishell ではデバッグ出力に `ft_printf` そのものを活用することが多いです。

### 1.3 到達目標

#### 必須到達基準（Mandatory）

以下のすべてを満たすことが必要です。

- [ ] `va_start` と `va_end` の対応関係を正しく理解し、必ずペアで使用できる
- [ ] `va_start` の第2引数がなぜ最後の固定引数でなければならないか説明できる
- [ ] `va_arg` で取得する型の指定方法を理解している（default argument promotion を考慮）
- [ ] `va_arg(args, char)` がなぜ undefined behavior か説明できる
- [ ] variadic function を自分で設計・実装できる
- [ ] `va_copy` の用途を説明できる
- [ ] format string と `va_arg` の型が一致しない場合のリスクを説明できる

#### 上級到達基準（Advanced）

deeper understanding を示すための基準です。

- [ ] variadic function が内部的にどのように引数を取得しているか（ABI レベル）を説明できる
- [ ] default argument promotion のルールを完全に列挙できる
- [ ] `va_arg` で間違った型を指定した場合に何が起こるか説明できる
- [ ] variadic function の引数の数を特定する方法を複数挙げられる（format string、sentinel value、count 引数）
- [ ] x86-64 での引数渡しの仕組み（レジスタ割り当て）を概説できる

#### 具体的な確認ポイント

**Q: 以下のコードの問題点は何か？**
```c
int my_sum(int count, ...)
{
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; i++)
        sum += va_arg(args, int);
    return (sum);    // va_end が抜けている!
}
```
**A:** `va_end(args)` が呼ばれていない。一部の platform では memory leak やクラッシュの原因になる。

**Q: 以下のコードの問題点は何か？**
```c
void my_func(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char c = va_arg(args, char);    // NG!
    va_end(args);
}
```
**A:** `va_arg(args, char)` は undefined behavior。default argument promotion により `char` は `int` に昇格されるため、`va_arg(args, int)` を使い、取得後に `char` に cast する必要がある。

**Q: 以下のコードで何が起こるか？**
```c
ft_printf("%d", (long)2147483648);
```
**A:** `va_arg(args, int)` で `long` 値を `int` として読み取るため、undefined behavior。8 バイトのうち 4 バイトだけ読まれ、さらに後続の `va_arg` も位置がずれて連鎖的に不正な値を返す可能性がある。

### 1.4 深い理解のために: プラットフォーム別の内部実装

variadic functions の仕組みは platform 依存です。直接これを実装する必要はありませんが、理解しておくと debugging に大いに役立ちます。

#### x86-64 (System V ABI) - Linux / macOS

**レジスタ渡し:**
- 最初の 6 個の整数/pointer 引数: `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`
- 最初の 8 個の浮動小数点引数: `xmm0` 〜 `xmm7`
- それ以降はスタックに push

**`va_list` の実装:**
```c
// gcc の内部実装（概略）
typedef struct {
    unsigned int gp_offset;     // 次の整数レジスタのオフセット
    unsigned int fp_offset;     // 次の浮動小数点レジスタのオフセット
    void *overflow_arg_area;    // スタック上の引数領域へのポインタ
    void *reg_save_area;        // レジスタ保存領域へのポインタ
} va_list[1];
```

**スタックフレームの詳細図:**

```
高アドレス
+---------------------------+
| 呼び出し元のフレーム       |
+---------------------------+
| リターンアドレス           |
+---------------------------+  <-- RSP の初期位置
| 保存レジスタ               |
+---------------------------+
| ローカル変数               |
|  - va_list args            |
|  - int count               |
|  - int ret                 |
+---------------------------+
| レジスタ保存領域           |  <-- va_start がここにレジスタ値を保存
|  - RDI (format)            |
|  - RSI (第1可変引数)        |
|  - RDX (第2可変引数)        |
|  - RCX (第3可変引数)        |
|  - R8  (第4可変引数)        |
|  - R9  (第5可変引数)        |
+---------------------------+
低アドレス
```

**具体例:**
```c
ft_printf("%d %s %x", 42, "hello", 255);

レジスタの割り当て:
  RDI: "%d %s %x"  (format)      <- 固定引数
  RSI: 42           (第1可変引数)
  RDX: "hello" のアドレス (第2可変引数)
  RCX: 255          (第3可変引数)

va_start(args, format):
  args.gp_offset = 8    (RSIのオフセット。RDI(=8バイト)分スキップ)
  args.reg_save_area = [レジスタ保存領域のアドレス]

va_arg(args, int):             // %d -> 42
  reg_save_area + 8 から 4 バイト読む -> 42
  gp_offset = 16 に更新

va_arg(args, char *):          // %s -> "hello"
  reg_save_area + 16 から 8 バイト読む -> "hello" のアドレス
  gp_offset = 24 に更新

va_arg(args, unsigned int):    // %x -> 255
  reg_save_area + 24 から 4 バイト読む -> 255
  gp_offset = 32 に更新
```

#### ARM64 (AArch64) - Apple Silicon 等

- 最初の 8 個の整数/pointer 引数: `x0` 〜 `x7`
- 最初の 8 個の浮動小数点引数: `v0` 〜 `v7`
- それ以降はスタック経由
- 原理は x86-64 と同様だが、レジスタ数が異なる

#### 32-bit x86

- すべての引数がスタックに push される（右から左の順で）
- `va_list` は単純なポインタとして実装
- `va_arg` はポインタを型のサイズ分だけ進めるだけ
- 最もシンプルで理解しやすい実装

**この知識が役立つ場面:**
- GDB でのデバッグ時に引数の値を直接確認する
- セグメンテーションフォールトの原因が `va_arg` の型不一致だと推測する
- パフォーマンスチューニング時にレジスタ渡しとスタック渡しの違いを考慮する
- 他のプログラミング言語の FFI（Foreign Function Interface）を理解する

### 1.5 後続課題との接続

| 後続課題 | variadic functions との関連 |
|---------|--------------------------|
| **get_next_line** | 直接的には使わないが、関数の戻り値で状態を伝える設計パターンは同じ |
| **minitalk** | signal handler から可変長の情報を処理するパターンに通じる |
| **pipex** | `execve` 系関数の引数リスト構築に可変長の概念が使われる |
| **push_swap** | コマンドライン引数の可変長処理で同様の思考が必要 |
| **minishell** | 組み込みコマンドの引数処理で可変長引数的な処理が必要。`printf` 系のデバッグ出力にも活用 |
| **ft_ssl_md5** (bonus) | ハッシュ関数への可変長入力の処理 |
| **philosophers** | デバッグ出力に ft_printf を使うことで、スレッド安全性の問題を考えるきっかけになる |

### 1.6 自己評価チェックリスト

自分の理解度を 1〜5 で評価してください（1: 全く理解していない → 5: 完全に理解し説明できる）。

| 項目 | 自己評価 (1-5) |
|------|--------------|
| `va_list` の役割を説明できる | [ ] |
| `va_start` の第2引数の意味を説明できる | [ ] |
| `va_arg` の型指定で default argument promotion を考慮できる | [ ] |
| `va_end` を呼び忘れた場合のリスクを説明できる | [ ] |
| `va_copy` が必要な場面を具体例で説明できる | [ ] |
| variadic function を一から書ける | [ ] |
| `va_arg` で間違った型を指定した場合の動作を説明できる | [ ] |
| format string と va_arg の対応関係を実装できる | [ ] |
| variadic function の引数の数を知る方法を3つ以上挙げられる | [ ] |
| platform ごとの引数渡しの違いを概説できる | [ ] |

**目安:** すべてが 3 以上であれば mandatory part の defence は問題ないでしょう。4 以上が多ければ上級者レベルです。

---

## 2. 型システム（Type System）の理解

### 2.1 学ぶべきこと

C 言語の型システムは、プログラムの正確性・安全性・移植性に直結する基礎的かつ極めて重要な知識です。ft_printf では多種の型を扱うため、型システムの理解が不可欠です。

#### C 言語の基本型

##### 整数型

| 型 | サイズ (64-bit) | 範囲 | ft_printf での用途 |
|----|---------------|------|-------------------|
| `char` | 1 byte | -128 〜 127 | `%c` の出力（`int` から cast） |
| `unsigned char` | 1 byte | 0 〜 255 | `write` に渡す際の型 |
| `short` | 2 bytes | -32,768 〜 32,767 | 直接は使わない（promotion で `int` になる） |
| `int` | 4 bytes | -2,147,483,648 〜 2,147,483,647 | `%d`, `%i` の出力、`write` の戻り値 |
| `unsigned int` | 4 bytes | 0 〜 4,294,967,295 | `%u`, `%x`, `%X` の出力 |
| `long` | 8 bytes (LP64) | -9,223,372,036,854,775,808 〜 9,223,372,036,854,775,807 | `%p` の内部処理で間接的に使用 |
| `unsigned long long` | 8 bytes | 0 〜 18,446,744,073,709,551,615 | `%p` のポインタ値を保持 |

##### ポインタ型

| 型 | サイズ (64-bit) | 説明 |
|----|---------------|------|
| `void *` | 8 bytes | 汎用ポインタ。`%p` で出力するアドレスを保持 |
| `char *` | 8 bytes | 文字列ポインタ。`%s` で出力する文字列を指す |

##### サイズの platform 依存性

上記のサイズは 64-bit 環境（LP64 データモデル）での値です。他のデータモデルでは異なります。

| データモデル | `int` | `long` | `void *` | 代表的な環境 |
|-------------|-------|--------|----------|-------------|
| ILP32 | 4 | 4 | 4 | 32-bit Linux, 32-bit Windows |
| LLP64 | 4 | 4 | 8 | 64-bit Windows |
| LP64 | 4 | 8 | 8 | 64-bit Linux, macOS |

42 の環境は通常 LP64 です。しかし `sizeof` を使って移植性を確保する習慣をつけましょう。

#### 符号付き整数と符号なし整数

##### 2の補数表現（Two's Complement）

現代のほぼすべてのコンピュータは、符号付き整数を **2の補数（two's complement）** で表現しています。C23 規格ではこれが正式に要件となりました。

**32-bit int の例:**
```
 0 = 0000 0000 0000 0000 0000 0000 0000 0000
 1 = 0000 0000 0000 0000 0000 0000 0000 0001
-1 = 1111 1111 1111 1111 1111 1111 1111 1111
 42 = 0000 0000 0000 0000 0000 0000 0010 1010
-42 = 1111 1111 1111 1111 1111 1111 1101 0110
 2147483647 (INT_MAX) = 0111 1111 1111 1111 1111 1111 1111 1111
-2147483648 (INT_MIN) = 1000 0000 0000 0000 0000 0000 0000 0000
```

**2の補数で負数を求める方法:**
1. 正の数のビット列を書く
2. 全ビットを反転する（1の補数）
3. 1を加える

例: -42 を求める
```
 42 = 0000 0000 0000 0000 0000 0000 0010 1010
反転 = 1111 1111 1111 1111 1111 1111 1101 0101
+1   = 1111 1111 1111 1111 1111 1111 1101 0110 = -42
```

**2の補数の重要な性質:**
1. 最上位ビット（MSB）が 1 なら負数
2. 負数の絶対値は「全ビット反転 + 1」で得られる
3. `INT_MIN` の絶対値は `INT_MAX + 1` であり、`int` で表現できない
4. `-INT_MIN` は **overflow** し、**未定義動作（undefined behavior）** となる
5. 加算と減算は符号の有無に関わらず同じハードウェア回路で実行できる

**ft_printf での影響:**

`ft_print_nbr.c` で `n == -2147483648` を特別扱いしているのはこのためです。

```c
// ft_print_nbr.c の ft_put_nbr 関数より
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
    n = -n;    // INT_MIN の場合、ここに到達する前に処理済み
}
```

もし特別処理がなければ、`n = -(-2147483648)` は overflow して undefined behavior になります。

##### 符号なし整数の特性

`unsigned int` は符号ビットを持たず、全ビットが値を表現します。

```
unsigned int:
  0 = 0000 0000 0000 0000 0000 0000 0000 0000
  4294967295 (UINT_MAX) = 1111 1111 1111 1111 1111 1111 1111 1111
```

**重要な性質:**
- overflow しない（modular arithmetic で wrap around する）
- `0 - 1 = 4294967295`（UINT_MAX）
- `-1` を `unsigned int` に cast すると `4294967295` になる
- unsigned 型の演算結果は常に `[0, 2^N - 1]` の範囲に収まる（mod 2^N）

##### signed と unsigned の混在による罠

```c
// 危険な比較
int len = -1;
unsigned int size = 10;
if (len < size)
{
    // ここに来ない!
    // len が unsigned int に変換されて 4294967295 になり、
    // 4294967295 < 10 は false
}
```

この罠は ft_printf の実装では直接遭遇しませんが、後続課題で頻繁に現れます。

#### 型変換（Type Conversion）

##### 暗黙的型変換（Implicit Conversion）

```c
int    a = -1;
unsigned int b = a;    // b は 4294967295 になる

int    c = 42;
long   d = c;          // d は 42（安全な拡大変換）

unsigned int e = 3000000000;
int    f = e;          // 実装定義（implementation-defined）
```

##### 明示的型変換（Explicit Cast）

```c
// ft_print_char.c での使用例
int c = va_arg(args, int);
unsigned char ch = (unsigned char)c;    // 明示的 cast
write(1, &ch, 1);
```

**cast が必要な場面（ft_printf内）:**
1. `int` → `unsigned char`: `%c` の出力時
2. `void *` → `unsigned long long`: `%p` の出力時（実装では `va_arg(args, unsigned long long)` で直接取得）
3. `int` → 負号処理後の正数: `%d` の出力時 (`n = -n`)

##### Integer Promotion Rules（整数昇格規則）

式の中で `int` より小さい整数型が使われると、自動的に `int`（または `unsigned int`）に昇格されます。

```c
char a = 100;
char b = 50;
// a + b は char の加算ではなく、int の加算として実行される
int result = a + b;    // 150（char の範囲を超えても OK）
```

##### Usual Arithmetic Conversions（通常の算術変換）

異なる型同士の演算では、以下の優先順位で一方が変換されます。

```
long double > double > float > unsigned long long > long long >
unsigned long > long > unsigned int > int
```

#### メモリ上での表現（エンディアン）

```
int n = 305419896 (0x12345678):

リトルエンディアン（x86, ARM）:
メモリ: [78][56][34][12]  (下位バイトが低アドレス)

ビッグエンディアン（一部の MIPS, PowerPC）:
メモリ: [12][34][56][78]  (上位バイトが低アドレス)
```

42 の環境（x86-64）はリトルエンディアンです。ft_printf の実装ではエンディアンを意識する必要はありませんが、デバッガでメモリダンプを見る際に知っていると役立ちます。

#### ASCII コードと文字型

`%c` の出力では、整数値を文字として解釈します。

```
'0' = 48, '1' = 49, ..., '9' = 57  (連続している!)
'A' = 65, 'B' = 66, ..., 'Z' = 90  (連続している!)
'a' = 97, 'b' = 98, ..., 'z' = 122 (連続している!)
```

ft_printf で数字を文字に変換する際、この連続性を利用しています:
```c
c = (n % 10) + '0';  // 数値 0-9 を文字 '0'-'9' に変換
```

16進変換でも同様:
```c
hex = "0123456789abcdef";
write(1, &hex[n % 16], 1);  // 数値 0-15 を '0'-'f' に変換
```

### 2.2 なぜ重要か

型の理解は C プログラミングの基礎です。ft_printf では以下の型操作が必要です。

| specifier | 受け取る型 | 内部処理で使う型 | 処理内容 |
|-----------|----------|----------------|---------|
| `%c` | `int` (promoted from `char`) | `unsigned char` | 1文字出力 |
| `%s` | `char *` | `char *` | 文字列出力 |
| `%p` | `unsigned long long` (cast from `void *`) | `unsigned long long` | 16進アドレス出力 |
| `%d` | `int` | `int` | 符号付き10進出力 |
| `%i` | `int` | `int` | 符号付き10進出力 |
| `%u` | `unsigned int` | `unsigned int` | 符号なし10進出力 |
| `%x` | `unsigned int` | `unsigned int` | 16進小文字出力 |
| `%X` | `unsigned int` | `unsigned int` | 16進大文字出力 |
| `%%` | なし | なし | `%` 文字を出力 |

#### 型の理解が不十分だと起こるバグ

1. **INT_MIN の negation overflow**: `-INT_MIN` は undefined behavior
2. **signed/unsigned の混在比較**: `if (len < 0)` で `len` が `unsigned` なら常に false
3. **ポインタサイズの誤り**: `%p` で `unsigned int` を使うと 64-bit 環境でアドレスが切り詰められる
4. **promotion の無視**: `va_arg(args, char)` で undefined behavior
5. **文字列長計算時の overflow**: 極端に長い文字列で `int` が overflow する可能性
6. **16進変換での符号問題**: `int` を `%x` で出力すると、負数は unsigned に reinterpret される

### 2.3 到達目標

#### 必須到達基準

- [ ] `int` の範囲（`INT_MIN` = -2,147,483,648 から `INT_MAX` = 2,147,483,647）を即答できる
- [ ] `unsigned int` の範囲（0 から 4,294,967,295）を即答できる
- [ ] `-1` を `unsigned int` に cast した結果（4294967295）を説明できる
- [ ] `INT_MIN` の negation が undefined behavior である理由を 2の補数を使って説明できる
- [ ] pointer のサイズが platform 依存であることを理解し、具体的な値を挙げられる
- [ ] `%p` で `unsigned long long` を使う理由を説明できる
- [ ] `'0' + 5 == '5'` がなぜ成り立つか（ASCII の連続性）を説明できる

#### 上級到達基準

- [ ] 2の補数表現でビット列から値への変換、値からビット列への変換ができる
- [ ] integer promotion rules を正確に述べられる
- [ ] usual arithmetic conversions の優先順位を説明できる
- [ ] `sizeof` の結果が platform ごとに異なる型を列挙できる
- [ ] C99 以降の固定幅整数型（`int32_t`, `uint64_t` 等）の利点を説明できる
- [ ] リトルエンディアンとビッグエンディアンの違いを説明できる

### 2.4 型サイズの詳細一覧（64-bit LP64 環境）

| 型 | サイズ (bytes) | ビット数 | 最小値 | 最大値 | マクロ |
|----|--------------|---------|--------|--------|-------|
| `char` | 1 | 8 | -128 | 127 | `CHAR_MIN`, `CHAR_MAX` |
| `unsigned char` | 1 | 8 | 0 | 255 | `0`, `UCHAR_MAX` |
| `short` | 2 | 16 | -32,768 | 32,767 | `SHRT_MIN`, `SHRT_MAX` |
| `unsigned short` | 2 | 16 | 0 | 65,535 | `0`, `USHRT_MAX` |
| `int` | 4 | 32 | -2,147,483,648 | 2,147,483,647 | `INT_MIN`, `INT_MAX` |
| `unsigned int` | 4 | 32 | 0 | 4,294,967,295 | `0`, `UINT_MAX` |
| `long` | 8 | 64 | -9,223,372,036,854,775,808 | 9,223,372,036,854,775,807 | `LONG_MIN`, `LONG_MAX` |
| `unsigned long` | 8 | 64 | 0 | 18,446,744,073,709,551,615 | `0`, `ULONG_MAX` |
| `long long` | 8 | 64 | -9,223,372,036,854,775,808 | 9,223,372,036,854,775,807 | `LLONG_MIN`, `LLONG_MAX` |
| `unsigned long long` | 8 | 64 | 0 | 18,446,744,073,709,551,615 | `0`, `ULLONG_MAX` |
| `void *` | 8 | 64 | - | - | - |

### 2.5 後続課題との接続

| 後続課題 | 型システム知識の活用場面 |
|---------|----------------------|
| **get_next_line** | `ssize_t` (`read` の戻り値), `size_t` (メモリサイズ) の理解。`read` が 0 を返す場合（EOF）と -1 を返す場合（エラー）の区別 |
| **push_swap** | `int` の範囲チェック、`long` を使った overflow 検出、`atoi` の限界値処理 |
| **pipex** | `pid_t` 型、file descriptor の `int` 型、`execve` の引数の `char **` 型 |
| **minishell** | 終了コード（`unsigned char` 範囲 = 0〜255）、文字列操作での `size_t` 使用 |
| **philosophers** | `usleep` の `useconds_t` 型、`pthread_t` 型、タイムスタンプの `long long` 型 |
| **cub3d / miniRT** | `float` / `double` の精度問題、浮動小数点演算の丸め誤差 |

### 2.6 自己評価チェックリスト

| 項目 | 自己評価 (1-5) |
|------|--------------|
| `int` の範囲を暗記している | [ ] |
| `unsigned int` の範囲を暗記している | [ ] |
| 2の補数の変換方法を理解している | [ ] |
| `INT_MIN` の negation が UB である理由を説明できる | [ ] |
| signed <-> unsigned 変換のルールを理解している | [ ] |
| implicit conversion と explicit cast の違いを説明できる | [ ] |
| integer promotion のルールを述べられる | [ ] |
| ポインタのサイズが platform で異なることを理解している | [ ] |
| `sizeof` 演算子の使い方を理解している | [ ] |
| ft_printf の各 specifier で使う型を即答できる | [ ] |
| ASCII コード表の数字部分・アルファベット部分の連続性を理解している | [ ] |
| リトルエンディアンでのメモリレイアウトを描ける | [ ] |

---

## 3. ライブラリ作成スキル（Library Building）の習得

### 3.1 学ぶべきこと

#### コンパイルの全体像

C プログラムが実行可能ファイルになるまでの流れを理解しましょう。

```
ソースファイル (.c)
    | [プリプロセス] (cpp / cc -E)
前処理済みソース (.i)
    | [コンパイル] (cc -S)
アセンブリ (.s)
    | [アセンブル] (as / cc -c)
オブジェクトファイル (.o)
    | [リンク] (ld / cc)
実行可能ファイル (a.out)
```

**各段階の詳細:**

1. **プリプロセス（Preprocessing）**:
   - `#include` の展開（ヘッダファイルの挿入）
   - `#define` マクロの展開
   - `#ifdef` / `#ifndef` による条件付きコンパイル（include guard もここ）
   - コメントの除去
   - 確認コマンド: `cc -E ft_printf.c -o ft_printf.i`

2. **コンパイル（Compilation）**:
   - C ソースコードをアセンブリ言語に変換
   - 構文解析、意味解析、最適化
   - `-Wall -Wextra -Werror` はこの段階で警告/エラーを検出
   - 確認コマンド: `cc -S ft_printf.c -o ft_printf.s`

3. **アセンブル（Assembly）**:
   - アセンブリ言語を機械語（オブジェクトコード）に変換
   - `.o` ファイル（オブジェクトファイル）を生成
   - 確認コマンド: `cc -c ft_printf.c -o ft_printf.o`

4. **リンク（Linking）**:
   - 複数の `.o` ファイルを結合
   - ライブラリとの結合
   - シンボル（関数名、変数名）の解決
   - アドレスの再配置（relocation）
   - 実行可能ファイルの生成
   - 確認コマンド: `cc main.o -L. -lftprintf -o program`

#### オブジェクトファイル（Object File）

`.o` ファイルはコンパイル済みの機械語を含むが、まだ他のファイルとリンクされていない中間ファイルです。

```bash
# ソースファイルからオブジェクトファイルを生成
cc -Wall -Wextra -Werror -c ft_printf.c -o ft_printf.o

# オブジェクトファイルの中身を確認（シンボルテーブル）
nm ft_printf.o
```

`nm` の出力例:
```
0000000000000000 t ft_print_format    <- 't' = ローカル (static 関数)
0000000000000080 T ft_printf          <- 'T' = グローバル (公開関数)
                 U ft_print_char      <- 'U' = 未定義 (他のファイルで定義)
                 U ft_print_hex
                 U ft_print_nbr
                 U ft_print_ptr
                 U ft_print_str
                 U ft_print_unsigned
```

- `T`: テキスト（コード）セクションに定義されたグローバルシンボル
- `t`: テキストセクションに定義されたローカルシンボル（`static` 関数）
- `U`: 未定義シンボル（他のファイルで定義されている）

**static 関数がライブラリに与える影響:**

```c
/* ft_print_nbr.c */
static int ft_numlen(int n);    /* static: 外部から見えない (t) */
static int ft_put_nbr(int n);   /* static: 外部から見えない (t) */
int        ft_print_nbr(int n); /* non-static: 外部に公開 (T) */
```

リンカは `T`（大文字）のシンボルのみを外部から参照可能にします。`t`（小文字）のシンボルはそのオブジェクトファイル内でのみ有効です。これにより名前の衝突を防ぎ、カプセル化（encapsulation）を実現しています。

#### `ar` コマンドによる Static Library の作成

```bash
ar rcs libftprintf.a ft_printf.o ft_print_char.o ft_print_str.o \
    ft_print_ptr.o ft_print_nbr.o ft_print_unsigned.o ft_print_hex.o
```

**各オプションの意味:**

| オプション | 意味 | 詳細 |
|----------|------|------|
| `r` | replace | archive 内の同名ファイルを置換（なければ追加） |
| `c` | create | archive が存在しない場合に新規作成（警告を抑制） |
| `s` | symbol index | archive の先頭にシンボルインデックスを作成（`ranlib` と同等） |

**`ranlib` との関係:**

`ar rcs` の `s` オプションは `ranlib` コマンドと同じ効果があります。シンボルインデックスがないと、リンカがライブラリ内のシンボルを効率的に検索できません。

```bash
# 以下の2つは同等
ar rcs libftprintf.a *.o
# と
ar rc libftprintf.a *.o && ranlib libftprintf.a
```

**archive の中身を確認:**
```bash
ar t libftprintf.a           # メンバーの一覧
ar x libftprintf.a           # メンバーの展開
nm libftprintf.a             # シンボルの一覧
```

#### リンカの動作原理

```
main.o 内のシンボル:
  定義: main
  未解決: ft_printf

libftprintf.a 内のシンボル:
  ft_printf.o:
    定義: ft_printf, ft_print_format(static)
    未解決: ft_print_char, ft_print_str, ...
  ft_print_char.o:
    定義: ft_print_char
    未解決: write
  ft_print_str.o:
    定義: ft_print_str, ft_strlen(static)
    未解決: write
  ...

リンカの処理:
1. main.o を読み込み、未解決シンボル ft_printf を発見
2. libftprintf.a のシンボルインデックスを検索
3. ft_printf を定義している ft_printf.o を取り出し
4. ft_printf.o の未解決シンボル (ft_print_char等) を解決するため
   libftprintf.a からさらにオブジェクトファイルを取り出し
5. 全シンボルが解決されたら、すべてのオブジェクトファイルを結合
6. アドレスを再配置（relocation）して実行ファイルを生成
```

#### Library のリンク方法

```bash
# コンパイルとリンク
cc main.c -L. -lftprintf -o program

# -L. : カレントディレクトリでライブラリを検索
# -lftprintf : libftprintf.a をリンク（lib と .a を省略して指定）

# 以下と同等
cc main.c libftprintf.a -o program
```

#### Static Library vs Dynamic Library

| 項目 | Static Library (.a) | Dynamic Library (.so / .dylib) |
|------|-------------------|-------------------------------|
| **拡張子** | `.a` (archive) | `.so` (Linux), `.dylib` (macOS) |
| **リンク時期** | コンパイル時（static linking） | 実行時（dynamic linking） |
| **実行ファイルサイズ** | 大きい（ライブラリが組み込まれる） | 小さい（参照のみ） |
| **配布** | 単体で動作（依存なし） | ライブラリファイルが必要 |
| **更新** | 再コンパイル必要 | ライブラリ差し替えで OK |
| **メモリ使用** | プロセスごとにコピー | 複数プロセスで共有可能 |
| **作成コマンド** | `ar rcs` | `cc -shared` |
| **リンク速度** | 速い | やや遅い（実行時シンボル解決） |
| **42 の要件** | **必須**（subject で指定） | 不要 |

ft_printf では **static library** を作成します。42 の subject で要求されているのは static library です。

#### Makefile による Build 自動化

##### ft_printf の Makefile を詳細解説

```makefile
# 変数定義
NAME    = libftprintf.a      # 生成するライブラリ名
CC      = cc                 # コンパイラ
CFLAGS  = -Wall -Wextra -Werror  # コンパイルフラグ

# ソースファイルの列挙
SRCS    = ft_printf.c \
          ft_print_char.c \
          ft_print_str.c \
          ft_print_ptr.c \
          ft_print_nbr.c \
          ft_print_unsigned.c \
          ft_print_hex.c

# パターン置換でオブジェクトファイルリストを生成
# ft_printf.c -> ft_printf.o, ft_print_char.c -> ft_print_char.o, ...
OBJS    = $(SRCS:.c=.o)

# デフォルトターゲット（make と打つとこれが実行される）
all: $(NAME)

# ライブラリのビルドルール
# $(OBJS) すべてが存在し、かつ $(NAME) より新しければ実行
$(NAME): $(OBJS)
	ar rcs $(NAME) $(OBJS)

# パターンルール: .c -> .o
# 任意の .o ファイルは、同名の .c ファイルと ft_printf.h に依存する
%.o: %.c ft_printf.h
	$(CC) $(CFLAGS) -c $< -o $@

# クリーンアップ
clean:
	rm -f $(OBJS)       # オブジェクトファイルだけ削除

fclean: clean
	rm -f $(NAME)        # ライブラリも削除

re: fclean all           # 全削除してから再ビルド

bonus: all               # bonus は mandatory と同じ

.PHONY: all clean fclean re bonus  # ファイル名と衝突しないことを保証
```

##### 重要な Makefile の概念

**自動変数（Automatic Variables）:**

| 変数 | 意味 | 例 (`%.o: %.c ft_printf.h` の場合) |
|------|------|-----|
| `$@` | ターゲット名 | `ft_printf.o` |
| `$<` | 最初の依存ファイル | `ft_printf.c` |
| `$^` | すべての依存ファイル | `ft_printf.c ft_printf.h` |
| `$?` | ターゲットより新しい依存ファイル | （更新されたファイルのみ） |

**PHONY ターゲット:**
```makefile
.PHONY: all clean fclean re bonus
```
`.PHONY` に指定されたターゲットは、同名のファイルが存在しても常に実行されます。もし `.PHONY` を付けずに `all` というファイルが存在すると、`make all` は「all は最新です」と言って何もしなくなります。

**依存関係とタイムスタンプ:**

`make` はターゲットと依存ファイルのタイムスタンプを比較し、依存ファイルのほうが新しい場合のみルールを実行します。

##### Relink の防止

**relink** とは、ソースファイルに変更がないのに不必要な再コンパイルやライブラリの再作成が行われることです。42 の評価で relink はマイナスポイントです。

**確認方法:**
```bash
make          # 初回ビルド: 全ファイルがコンパイルされる
make          # 2回目: "Nothing to be done for 'all'." と表示されるべき
touch ft_print_char.c
make          # ft_print_char.o のみ再コンパイルされ、ライブラリが再作成されるべき
```

### 3.2 なぜ重要か

1. **コードの再利用**: 一度作った `libftprintf.a` を他のプロジェクトで使い回せる
2. **モジュール化**: 関連する関数をまとめて管理できる
3. **ビルドの効率化**: 変更のあったファイルだけ再コンパイルできる
4. **実務での必須知識**: 実務のプロジェクトでは必ずビルドシステムとライブラリの知識が求められる
5. **インターフェースと実装の分離**: `.h` と `.c` の分離の重要性を体感できる

### 3.3 到達目標

#### 必須到達基準

- [ ] static library と dynamic library の違いを 3 つ以上挙げて説明できる
- [ ] `ar rcs` の各オプション（`r`, `c`, `s`）の意味を説明できる
- [ ] Makefile の基本的な文法（ターゲット、依存関係、コマンド）を理解し、書ける
- [ ] `make` の dependency tracking（タイムスタンプ比較）の仕組みを説明できる
- [ ] `$(CC)`, `$(CFLAGS)`, `$@`, `$<` の意味を説明できる
- [ ] `.PHONY` の役割を説明できる
- [ ] relink を引き起こす原因と対策を説明できる

#### 上級到達基準

- [ ] `nm` コマンドの出力（`T`, `t`, `U` 等）を読み解ける
- [ ] `ranlib` と `ar s` の関係を説明できる
- [ ] リンカがシンボルを解決する手順を概説できる

### 3.4 後続課題との接続

| 後続課題 | ライブラリ/ビルド知識の活用場面 |
|---------|---------------------------|
| **push_swap** | `libft` + `ft_printf` をリンクして使う。Makefile でのマルチライブラリ管理 |
| **minishell** | 複数のライブラリ（libft, readline 等）を Makefile で管理 |
| **cub3d / miniRT** | MLX ライブラリのリンク。複雑な依存関係の管理 |

### 3.5 自己評価チェックリスト

| 項目 | 自己評価 (1-5) |
|------|--------------|
| コンパイルの4段階を説明できる | [ ] |
| `.o` ファイルの役割を理解している | [ ] |
| `ar rcs` の意味を説明できる | [ ] |
| `-L` と `-l` オプションの使い方を理解している | [ ] |
| static vs dynamic library の違いを説明できる | [ ] |
| Makefile のターゲット・依存関係・コマンドを書ける | [ ] |
| 自動変数 `$@`, `$<`, `$^` を使える | [ ] |
| `.PHONY` の必要性を説明できる | [ ] |
| relink が起きない Makefile を書ける | [ ] |
| `nm` でシンボルを確認できる | [ ] |

---

## 4. フォーマット文字列解析（Format String Parsing）の実践

### 4.1 学ぶべきこと

#### パーサーの基本概念

フォーマット文字列の解析は、**パーサー（parser）** の設計という広い概念の具体的な適用です。

ft_printf のフォーマット文字列は、以下の文法（grammar）に従います。

**Mandatory part の文法（簡略BNF）:**
```
format_string := (literal_char | conversion)*
literal_char  := <'%' 以外の任意の文字>
conversion    := '%' specifier
specifier     := 'c' | 's' | 'p' | 'd' | 'i' | 'u' | 'x' | 'X' | '%'
```

**Bonus part の文法（簡略BNF）:**
```
format_string := (literal_char | conversion)*
literal_char  := <'%' 以外の任意の文字>
conversion    := '%' [flags] [width] ['.' precision] specifier
flags         := ('-' | '0' | '#' | ' ' | '+')*
width         := ('1'..'9') ('0'..'9')*
precision     := ('0'..'9')*
specifier     := 'c' | 's' | 'p' | 'd' | 'i' | 'u' | 'x' | 'X' | '%'
```

#### ft_printf での解析フロー

実際のコードの動作を追跡します。

**解析の流れ（例: `ft_printf("Hello %s! %d%%", "World", 100)`）:**

```
Step  1: 'H' -> 通常文字 -> 'H' を出力 (count=1)
Step  2: 'e' -> 通常文字 -> 'e' を出力 (count=2)
Step  3: 'l' -> 通常文字 -> 'l' を出力 (count=3)
Step  4: 'l' -> 通常文字 -> 'l' を出力 (count=4)
Step  5: 'o' -> 通常文字 -> 'o' を出力 (count=5)
Step  6: ' ' -> 通常文字 -> ' ' を出力 (count=6)
Step  7: '%' -> format++ -> 's' を読む -> "World" を出力 (count=11)
Step  8: '!' -> 通常文字 -> '!' を出力 (count=12)
Step  9: ' ' -> 通常文字 -> ' ' を出力 (count=13)
Step 10: '%' -> format++ -> 'd' を読む -> "100" を出力 (count=16)
Step 11: '%' -> format++ -> '%' を読む -> '%' を出力 (count=17)
結果: "Hello World! 100%" (count=17)
```

#### Dispatch（分岐処理）の設計

`ft_print_format` は specifier に応じて適切な出力関数を呼び出す **dispatcher** です。

**設計上のポイント:**

1. **if-else チェーン**: 9 パターンなので if-else で十分
2. **`va_arg` の型と specifier の対応**: 間違うと undefined behavior
3. **戻り値の統一**: すべて「出力文字数」or「-1」
4. **`%%` の処理**: `va_arg` を呼ばない唯一のケース
5. **`%d` と `%i` の統合**: 同じ処理なので `||` で結合

#### 状態遷移図（State Machine）

**Mandatory Part:**

```
           +--- 通常文字を出力 ---+
           v                      |
    +-------------+        +-------------+
    |  NORMAL      |--'%'-->|  SPECIFIER   |
    |  (通常状態)  |        |  (変換待ち)   |
    +-------------+        +-------------+
           ^                      |
           +-- specifier を処理 --+

    '\0' で終了
```

**Bonus Part:**

```
    +-------------+
    |  NORMAL      |--'%'--> +----------+
    |  (通常状態)  |         |  FLAGS    |--flag文字--> (自身にループ)
    +-------------+         +----------+
           ^                      |
           |                 数字が来たら
           |                      v
           |                +----------+
           |                |  WIDTH   |--数字--> (自身にループ)
           |                +----------+
           |                      |
           |                '.' が来たら
           |                      v
           |                +----------+
           |                | PRECISION|--数字--> (自身にループ)
           |                +----------+
           |                      |
           |                specifier が来たら
           |                      v
           |                +----------+
           +----------------|  CONVERT  |--変換実行
                            +----------+
```

#### エラーハンドリングの設計

ft_printf のエラーハンドリングは **戻り値の連鎖（early return on error）** で実現されています。

```
write() が -1 を返す
    |
ft_print_char() / ft_put_nbr() 等が -1 を返す
    |
ft_print_nbr() / ft_print_hex() 等が -1 を返す
    |
ft_print_format() が -1 を返す
    |
ft_printf() が ret == -1 を検出し、-1 を返す
```

### 4.2 なぜ重要か

| 概念 | ft_printf との関連 |
|------|-------------------|
| **Lexer / Tokenizer** | format 文字列を「通常文字」と「変換指定」に分割 |
| **Parser** | specifier を解析し対応する処理を決定 |
| **State Machine** | NORMAL → SPECIFIER の状態遷移 |
| **Protocol Parsing** | フォーマットされたテキストの処理全般 |
| **Template Engine** | `%d` を実際の値に置換 |
| **Compiler Front-end** | 文字列を走査して意味を解釈する基本 |

### 4.3 到達目標

#### 必須到達基準

- [ ] format string を1文字ずつ走査して処理できる
- [ ] `%` の検出と specifier の読み取りができる
- [ ] specifier に応じた dispatch ができる
- [ ] `%%` の処理が正しくできる
- [ ] 戻り値でエラーを伝播させる設計ができる
- [ ] NULL format string のエラーハンドリングができる

#### 上級到達基準

- [ ] bonus の flag / width / precision の parse ロジックを設計できる
- [ ] 有限オートマトン（FSM）として parsing を説明できる
- [ ] 文法規則を BNF で記述できる

### 4.4 後続課題との接続

| 後続課題 | parsing 知識の活用場面 |
|---------|---------------------|
| **get_next_line** | buffer 内の改行文字を検索する処理 |
| **push_swap** | 引数文字列を整数に変換する処理 |
| **minishell** | コマンドラインの完全なパーサー。**最重要課題** |
| **cub3d / miniRT** | マップファイル・設定ファイルのパーサー |
| **webserv** | HTTP リクエスト/レスポンスのパーサー |

### 4.5 自己評価チェックリスト

| 項目 | 自己評価 (1-5) |
|------|--------------|
| format 文字列を while ループで走査できる | [ ] |
| `%` の検出と specifier の読み取りができる | [ ] |
| if-else チェーンで dispatch ができる | [ ] |
| `%%` を正しく処理できる | [ ] |
| エラーハンドリングの戻り値伝播を実装できる | [ ] |
| parsing を状態遷移図で説明できる | [ ] |
| bonus part の parsing 方針を説明できる | [ ] |

---

## 5. エラーハンドリングの設計

### 5.1 C 言語のエラー処理パターン

**パターン1: 特殊な戻り値（ft_printf で採用）**
```c
int result = some_function();
if (result == -1)
    /* エラー処理 */
```

**パターン2: errno**
```c
int fd = open("file.txt", O_RDONLY);
if (fd == -1)
    perror("open");
```

**パターン3: NULL ポインタ**
```c
char *ptr = malloc(100);
if (ptr == NULL)
    /* メモリ確保失敗 */
```

### 5.2 到達目標チェックリスト

- [ ] write のエラーを全箇所でチェックしている
- [ ] エラーの伝播チェーンが途切れていないことを確認できる
- [ ] ft_printf(NULL) で -1 が返ることを確認した
- [ ] C 言語のエラー処理パターンを3つ挙げられる

---

## 6. 発展的な話題

### 6.1 glibc の printf との比較

| 機能 | ft_printf | glibc printf |
|------|----------|-------------|
| バッファリング | なし（1文字ずつ write） | stdio バッファ（8192 バイト） |
| specifier 数 | 9 種 | 数十種 |
| 浮動小数点 | なし | `%f`, `%e`, `%g`, `%a` |
| コード量 | 約200行 | 約2000行以上 |

### 6.2 フォーマット文字列攻撃（Format String Attack）

```c
/* 危険 */
printf(user_input);        // ユーザー入力を直接 format string に!

/* 安全 */
printf("%s", user_input);  // ユーザー入力は引数として渡す
```

ft_printf では `%n` を実装しないため、この攻撃のリスクはありません。

### 6.3 発展演習

1. **ft_dprintf**: 任意の file descriptor に出力する版
2. **バッファリング版**: 内部バッファで write 回数を最小化
3. **`%b` 追加**: unsigned int を2進数で出力

---

## 7. 総合自己評価

### プロジェクト完了チェックリスト

| カテゴリ | 項目 | 達成 |
|---------|------|------|
| **基本機能** | すべての specifier（cspdiuxX%%）が正しく動作する | [ ] |
| **エッジケース** | INT_MIN, NULL string, NULL pointer が正しく処理される | [ ] |
| **エラー処理** | `write` のエラーが正しく伝播される | [ ] |
| **ビルド** | `make` で relink が起きない | [ ] |
| **ビルド** | `make clean`, `make fclean`, `make re` が正しく動作する | [ ] |
| **Norm** | 42 の Norm に準拠している | [ ] |
| **知識** | 可変長引数のマクロ群を説明できる | [ ] |
| **知識** | default argument promotion を説明できる | [ ] |
| **知識** | 型システムの基本を説明できる | [ ] |
| **知識** | 2の補数表現と INT_MIN の問題を説明できる | [ ] |
| **知識** | ライブラリの作成手順を説明できる | [ ] |
| **知識** | format string の解析方法を説明できる | [ ] |
| **知識** | エラーハンドリングの設計を説明できる | [ ] |
| **Defence** | 上記すべてについて質問に自信を持って答えられる | [ ] |

### 総合評価の目安

- **全項目にチェック**: defence で高い評価を得られるでしょう。
- **8割以上にチェック**: 基本的な defence は問題ないでしょう。
- **6割以下**: 追加の学習が必要です。

variadic functions、型システム、ライブラリ作成、format string parsing の知識は、42 カリキュラムの後続課題すべての基礎となります。ここでしっかりと理解を固めることが、今後のプロジェクトの成功に直結します。

---

## 8. エンディアンとメモリ表現の深い理解

### 8.1 リトルエンディアンとビッグエンディアン

```
int n = 0x12345678 のメモリ表現:

リトルエンディアン（x86-64, ARM64）:
アドレス: 0x100  0x101  0x102  0x103
値:       0x78   0x56   0x34   0x12
          LSB                   MSB

ビッグエンディアン（一部のPowerPC, SPARC）:
アドレス: 0x100  0x101  0x102  0x103
値:       0x12   0x34   0x56   0x78
          MSB                   LSB
```

**ft_printf への影響:**

`ft_print_char` で `int c` から `unsigned char ch` にキャストする際、エンディアンが関係します:

```c
int c = 65;  /* 'A' */

リトルエンディアン:
  &c を指すアドレスに [41][00][00][00] が格納
  write(1, &c, 1) は最初の1バイト 0x41 = 'A' を読む -> OK

ビッグエンディアン:
  &c を指すアドレスに [00][00][00][41] が格納
  write(1, &c, 1) は最初の1バイト 0x00 = NUL を読む -> NG!
```

だから `unsigned char ch = (unsigned char)c` にキャストしてから `write(1, &ch, 1)` とするのが、エンディアンに依存しない正しい実装です。

### 8.2 ポインタのメモリ表現

```
void *ptr = (void *)0x7FFF12345678 の場合:

64ビット リトルエンディアン:
アドレス: 0x200  0x201  0x202  0x203  0x204  0x205  0x206  0x207
値:       0x78   0x56   0x34   0x12   0xFF   0x7F   0x00   0x00

ft_print_ptr はこの 8 バイトの値を unsigned long long として受け取り、
16進数文字列 "7fff12345678" に変換して出力します。
```

---

## 9. 標準規格との対応

### 9.1 C11 標準における printf の仕様

C11 標準（ISO/IEC 9899:2011）の 7.21.6.1 節で printf の変換仕様が定義されています。ft_printf が対応する部分:

| C11 の記述 | ft_printf での対応 |
|-----------|-------------------|
| `%c`: int 引数を unsigned char に変換して出力 | ft_print_char: (unsigned char)c |
| `%s`: char 配列を指すポインタ | ft_print_str: char * 引数 |
| `%d`, `%i`: int 引数を符号付き10進で出力 | ft_print_nbr: int 引数 |
| `%u`: unsigned int を符号なし10進で出力 | ft_print_unsigned: unsigned int |
| `%x`, `%X`: unsigned int を16進で出力 | ft_print_hex: unsigned int + uppercase |
| `%p`: void * を implementation-defined 形式で出力 | ft_print_ptr: "0x" + 16進小文字 |
| `%%`: literal % を出力 | ft_print_char('%') |

### 9.2 Undefined Behavior の一覧

ft_printf で遭遇する可能性のある undefined behavior:

| 状況 | 結果 | 対策 |
|------|------|------|
| `va_arg` の型が実際の引数と異なる | 不正な値、segfault | 正しい型を指定 |
| 符号付き整数のオーバーフロー (-INT_MIN) | 不定 | INT_MIN を特別処理 |
| NULL ポインタのデリファレンス | segfault | NULL チェック |
| va_start 後に va_end を呼ばない | リソースリーク（理論上） | va_end を呼ぶ |

---

## 10. テスト方法論

### 10.1 テストの種類と ft_printf での適用

| テストの種類 | 内容 | ft_printf での例 |
|-------------|------|-----------------|
| 単体テスト | 1つの関数を個別にテスト | ft_print_char('A') |
| 結合テスト | 複数の関数を組み合わせてテスト | ft_printf("%d %s", 42, "hi") |
| 境界値テスト | 入力の境界での動作確認 | INT_MIN, UINT_MAX, NULL |
| 異常系テスト | 不正な入力に対する動作確認 | ft_printf(NULL) |
| 回帰テスト | バグ修正後の再テスト | 全テストスイートの再実行 |
