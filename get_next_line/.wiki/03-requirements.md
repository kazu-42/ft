# 03 - 要件と仕様の詳細

この章では、get_next_lineの要件と仕様を完全に定義します。
「何を実装すべきか」を曖昧さなく理解することが、
正しい実装への第一歩です。

プロジェクトの仕様を正確に理解することは、
実装以上に重要です。仕様の誤解は「全体のやり直し」に
つながりますが、実装のバグは「部分的な修正」で済みます。

---

## 1. 関数仕様

### 1.1 プロトタイプ

```c
char *get_next_line(int fd);
```

### 1.2 入出力仕様の完全定義

| 項目 | 詳細 |
|------|------|
| **関数名** | `get_next_line` |
| **引数** | `int fd` - 読み取り元のファイルディスクリプタ |
| **戻り値（通常行）** | 読み取った行（`\n`を含む）。ヒープ上にmallocされた文字列。 |
| **戻り値（最終行）** | ファイルが`\n`で終わらない場合、`\n`なしの文字列 |
| **戻り値（EOF）** | `NULL`（読み取るデータがない） |
| **戻り値（エラー）** | `NULL`（不正なfd、read失敗、malloc失敗） |
| **副作用** | static変数に残りデータを保持する |
| **メモリ責任** | 返された行は**呼び出し元がfreeする責任**を持つ |

### 1.3 プロトタイプが固定されている理由

```
get_next_lineのプロトタイプは以下に固定されています:
  char *get_next_line(int fd);

これを変更することはできません（要件違反になります）。

変更できない理由:
  1. 外部のコードがこのプロトタイプで呼び出す前提
  2. テスターがこのシグネチャで呼ぶ
  3. 後続プロジェクトがこのインターフェースを利用する

制約から生まれる設計:
  → 「状態を引数で渡す」設計はできない
  → static変数を使う必然性
  → 呼び出し元がstashの存在を知る必要がない
    （カプセル化）
```

### 1.4 動作の具体例

```
=== 通常のファイル ===

ファイル内容: "Line1\nLine2\nLine3\n"
                                     ^最後に\nがある

get_next_line(fd) -> "Line1\n"   // \n を含む
get_next_line(fd) -> "Line2\n"   // \n を含む
get_next_line(fd) -> "Line3\n"   // \n を含む（最終行も\nあり）
get_next_line(fd) -> NULL        // EOF
get_next_line(fd) -> NULL        // 以降もずっとNULL


=== 最終行が\nで終わらないファイル ===

ファイル内容: "Line1\nLine2\nLine3"
                                   ^\nがない

get_next_line(fd) -> "Line1\n"   // \n を含む
get_next_line(fd) -> "Line2\n"   // \n を含む
get_next_line(fd) -> "Line3"     // 最終行は\nなし
get_next_line(fd) -> NULL        // EOF


=== 空ファイル ===

ファイル内容: ""（0バイト）

get_next_line(fd) -> NULL        // 即座にNULL


=== 改行のみのファイル ===

ファイル内容: "\n"

get_next_line(fd) -> "\n"        // 改行1文字だけの行
get_next_line(fd) -> NULL        // EOF


=== 連続改行のファイル ===

ファイル内容: "\n\n\n"

get_next_line(fd) -> "\n"        // 1つ目の改行
get_next_line(fd) -> "\n"        // 2つ目の改行
get_next_line(fd) -> "\n"        // 3つ目の改行
get_next_line(fd) -> NULL        // EOF


=== 改行のない1行だけのファイル ===

ファイル内容: "Hello"

get_next_line(fd) -> "Hello"     // \nなし
get_next_line(fd) -> NULL        // EOF
```

### 1.5 「行」の定義

get_next_lineにおける「行」の正確な定義:

```
行（line）の定義:
  1. '\n' で終わる文字列
     → 例: "Hello\n"、"\n"、"ABC\n"
  2. ファイル末尾の場合は '\n' なしの文字列
     → 例: "Hello"（ファイルが "Hello" で終わる場合）
  3. 空行も行として数える
     → "\n" は1文字の行（改行のみ）

行でないもの:
  ・空文字列 "" → これは行ではなくEOF
  ・NULL → エラーまたはEOF

注意:
  ・C言語的には、返される文字列は常に '\0' で終端される
  ・'\n' は行の一部として含まれる（行末記号ではなく行の末尾文字）
  ・返されるメモリは常にmallocで確保される
```

### 1.6 戻り値のメモリレイアウト

```
get_next_line(fd) が "Hello\n" を返す場合:

返されるポインタ
      |
      v
  ヒープ上のメモリ:
  +-----+-----+-----+-----+-----+------+------+
  | 'H' | 'e' | 'l' | 'l' | 'o' | '\n' | '\0' |
  +-----+-----+-----+-----+-----+------+------+
    [0]   [1]   [2]   [3]   [4]   [5]    [6]

  malloc(7) で確保された7バイト
  呼び出し元が free() する義務がある


get_next_line(fd) が "A" を返す場合（最終行、\nなし）:

返されるポインタ
      |
      v
  ヒープ上のメモリ:
  +-----+------+
  | 'A' | '\0' |
  +-----+------+
    [0]   [1]

  malloc(2) で確保された2バイト


get_next_line(fd) が "\n" を返す場合:

返されるポインタ
      |
      v
  ヒープ上のメモリ:
  +------+------+
  | '\n' | '\0' |
  +------+------+
    [0]    [1]

  malloc(2) で確保された2バイト
```

---

## 2. 提出ファイル一覧

### 2.1 必須ファイル（Mandatory）

| ファイル | 関数数 | 説明 |
|---------|-------|------|
| `get_next_line.h` | 0 | ヘッダファイル: プロトタイプ、BUFFER_SIZEデフォルト定義 |
| `get_next_line.c` | 3 | メイン関数: `get_next_line()`, `ft_init_stash()`, `ft_read_to_stash()` |
| `get_next_line_utils.c` | 5 | ヘルパー: `ft_strlen()`, `ft_strchr()`, `ft_strjoin()`, `ft_get_line()`, `ft_trim_stash()` |

### 2.2 ボーナスファイル

| ファイル | 変更点 |
|---------|--------|
| `get_next_line_bonus.h` | `MAX_FD`マクロの追加定義 |
| `get_next_line_bonus.c` | `static char *stash`を`static char *stash[MAX_FD]`に変更 |
| `get_next_line_utils_bonus.c` | 内容は通常版と同一 |

### 2.3 各ファイルの内容と制約

```
get_next_line.h:
  +--------------------------------------+
  | #ifndef GET_NEXT_LINE_H              | インクルードガード
  | # define GET_NEXT_LINE_H             |
  |                                      |
  | # include <stdlib.h>                 | malloc, free, size_t
  | # include <unistd.h>                 | read, ssize_t
  |                                      |
  | # ifndef BUFFER_SIZE                 | デフォルト値ガード
  | #  define BUFFER_SIZE 42             |
  | # endif                              |
  |                                      |
  | char   *get_next_line(int fd);       | 関数プロトタイプ
  | size_t  ft_strlen(const char *s);    |
  | char   *ft_strchr(const char *s, int c); |
  | char   *ft_strjoin(char *s1, char *s2);  |
  | char   *ft_get_line(char *stash);    |
  | char   *ft_trim_stash(char *stash);  |
  |                                      |
  | #endif                               | インクルードガード閉じ
  +--------------------------------------+

get_next_line.c:
  +--------------------------------------+
  | #include "get_next_line.h"           |
  |                                      |
  | static ft_read_to_stash() [25行以内] | read + stash蓄積
  | static ft_init_stash()    [25行以内] | NULL->"" 初期化
  | get_next_line()           [25行以内] | 全体制御
  |                                      |
  | 合計3関数 (上限5関数)                 |
  +--------------------------------------+

get_next_line_utils.c:
  +--------------------------------------+
  | #include "get_next_line.h"           |
  |                                      |
  | ft_strlen()     [25行以内]           | NULLセーフ文字列長
  | ft_strchr()     [25行以内]           | NULLセーフ文字検索
  | ft_strjoin()    [25行以内]           | 結合+s1解放
  | ft_get_line()   [25行以内]           | 行切り出し
  | ft_trim_stash() [25行以内]           | 残余管理
  |                                      |
  | 合計5関数 (上限5関数 = ちょうど上限!) |
  +--------------------------------------+
```

### 2.4 ファイル間の依存関係

```
依存関係図:

  get_next_line.h
        |
        v
  +-----+-----+
  |           |
  v           v
  gnl.c      gnl_utils.c
  |           |
  |  depends  |
  +-----→-----+

gnl.c が gnl_utils.c の関数を呼ぶ:
  ft_read_to_stash → ft_strchr, ft_strjoin
  get_next_line    → ft_get_line, ft_trim_stash

gnl_utils.c 内の関数同士:
  ft_strjoin → ft_strlen
  ft_get_line → 独立
  ft_trim_stash → ft_strlen

循環依存なし:
  gnl_utils.c は gnl.c に依存しない → テスト容易
```

---

## 3. コンパイル要件

### 3.1 必須コンパイルフラグ

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42
```

| フラグ | 意味 | 影響 |
|-------|------|------|
| `-Wall` | 一般的な警告を全て有効化 | 未使用変数等があるとエラー |
| `-Wextra` | 追加の警告を有効化 | 未使用パラメータ等もエラー |
| `-Werror` | 全ての警告をエラーとして扱う | 警告が1つでもあるとコンパイル失敗 |
| `-D BUFFER_SIZE=n` | BUFFER_SIZEマクロをnとして定義 | read()の読み取りサイズ |

### 3.2 テストすべきBUFFER_SIZE値の完全リスト

| 値 | テスト意図 | 注意点 |
|----|----------|--------|
| 1 | 1バイトずつ読む極端なケース | read()回数が最大、strjoin回数も最大 |
| 2 | 2バイトずつ | \nがちょうど境界にくるケースのテスト |
| 42 | 標準的なサイズ | 一般的なテスト |
| 100 | 小さなファイルを1回で読めるサイズ | |
| 9999 | 大きめのバッファ | |
| 10000000 | 非常に大きなバッファ（10MB） | malloc成功前提。一度でファイル全体を読む可能性 |
| 未指定 | デフォルト値（42）が正しく設定されているか | #ifndef パターンの検証 |
| 0 | 読み取り不可（NULLを返すべき） | BUFFER_SIZE <= 0 チェック |
| -1 | 負の値（NULLを返すべき） | BUFFER_SIZE <= 0 チェック |

### 3.3 コンパイルコマンドの全パターン

```bash
# 必須テスト: 全て warning/error なしでコンパイルできること

# 1. 標準テスト
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 \
    get_next_line.c get_next_line_utils.c main.c -o gnl

# 2. BUFFER_SIZE=1 テスト
cc -Wall -Wextra -Werror -D BUFFER_SIZE=1 \
    get_next_line.c get_next_line_utils.c main.c -o gnl_1

# 3. BUFFER_SIZE=9999 テスト
cc -Wall -Wextra -Werror -D BUFFER_SIZE=9999 \
    get_next_line.c get_next_line_utils.c main.c -o gnl_9999

# 4. BUFFER_SIZE=10000000 テスト
cc -Wall -Wextra -Werror -D BUFFER_SIZE=10000000 \
    get_next_line.c get_next_line_utils.c main.c -o gnl_big

# 5. BUFFER_SIZE 未指定テスト（デフォルト値42）
cc -Wall -Wextra -Werror \
    get_next_line.c get_next_line_utils.c main.c -o gnl_default

# 6. デバッグ用（valgrind/gdb）
cc -Wall -Wextra -Werror -g -D BUFFER_SIZE=42 \
    get_next_line.c get_next_line_utils.c main.c -o gnl_debug

# 7. ボーナス
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 \
    get_next_line_bonus.c get_next_line_utils_bonus.c main.c \
    -o gnl_bonus
```

---

## 4. エッジケース完全網羅

### 4.1 ファイル内容に関するエッジケース

| # | ケース | ファイル内容 | 期待される動作 |
|---|-------|------------|-------------|
| 1 | 空ファイル | (0バイト) | 最初の呼び出しでNULL |
| 2 | 1文字のみ（改行あり） | `"A\n"` | `"A\n"` -> NULL |
| 3 | 1文字のみ（改行なし） | `"A"` | `"A"` -> NULL |
| 4 | 改行のみ | `"\n"` | `"\n"` -> NULL |
| 5 | 連続改行 | `"\n\n\n"` | `"\n"` -> `"\n"` -> `"\n"` -> NULL |
| 6 | 最終行が改行で終わる | `"AB\n"` | `"AB\n"` -> NULL |
| 7 | 最終行が改行で終わらない | `"AB"` | `"AB"` -> NULL |
| 8 | 非常に長い行 | 10000文字+`\n` | 正しく全体を返す |
| 9 | 複数の普通の行 | `"A\nB\nC\n"` | `"A\n"` -> `"B\n"` -> `"C\n"` -> NULL |
| 10 | 空行を含む | `"A\n\nB\n"` | `"A\n"` -> `"\n"` -> `"B\n"` -> NULL |

### 4.2 fdに関するエッジケース

| # | ケース | fd値 | 期待される動作 |
|---|-------|------|-------------|
| 1 | 負のfd | -1 | 即座にNULL（readを呼ばない） |
| 2 | 無効なfd | 999（開いていない） | NULL（readが-1を返す） |
| 3 | 閉じられたfd | close後に呼ぶ | NULL（readが-1を返す） |
| 4 | 標準入力 | 0 (stdin) | 正常に動作する（ブロッキング） |
| 5 | 正常なファイル | 3- | 正常動作 |

### 4.3 BUFFER_SIZEに関するエッジケース

| # | ケース | BUFFER_SIZE | 期待される動作 |
|---|-------|-------------|-------------|
| 1 | ゼロ | 0 | NULLを返す |
| 2 | 負の値 | -1 | NULLを返す |
| 3 | 1（最小有効値） | 1 | 正常動作（遅いが正確） |
| 4 | ファイルサイズより大 | 10000000 | 1回のreadで全データ取得、正常動作 |
| 5 | ちょうど1行分 | 行の長さと同じ | 正常動作 |
| 6 | 行の途中で切れる | 行の長さ-1 | 2回のreadで1行分、正常動作 |

### 4.4 メモリに関するエッジケース

| # | ケース | 期待される動作 |
|---|-------|-------------|
| 1 | malloc失敗 | NULLを返す（メモリリークなし） |
| 2 | 連続呼び出し後のEOF | stashが適切に解放される |
| 3 | 返されたlineをfreeしない | 呼び出し元の責任（get_next_line側の問題ではない） |

### 4.5 各エッジケースの動作トレース

```
=== ケース: 空ファイル (0バイト) ===

get_next_line(fd):
  stash = NULL -> ft_init_stash -> stash = "" (malloc(1))
  ft_read_to_stash:
    buf = malloc(BUFFER_SIZE + 1)
    stashに\nなし -> readループに入る
    read(fd, buf, BUFFER_SIZE) -> 0 (EOF) -> ループ終了
    free(buf)
    return stash = ""
  stash[0] == '\0' -> free(stash) -> stash = NULL -> return NULL

  結果: NULL（正しい動作）


=== ケース: "\n" (改行のみ) ===

get_next_line(fd) 呼び出し1:
  stash = NULL -> ft_init_stash -> stash = ""
  ft_read_to_stash:
    stashに\nなし -> read -> "\n" (1バイト or BUFFER_SIZE分)
    stash = "" + "\n" = "\n"
    stashに\nあり -> ループ終了
  ft_get_line("\n") -> line = "\n"
  ft_trim_stash("\n"):
    \nの次が'\0' -> free(stash) -> return NULL
  return "\n"

get_next_line(fd) 呼び出し2:
  stash = NULL -> ft_init_stash -> stash = ""
  ft_read_to_stash:
    read -> 0 (EOF)
  stash[0] == '\0' -> return NULL


=== ケース: fd = -1 ===

get_next_line(-1):
  fd < 0 -> return NULL（即座に終了）
  メモリ確保なし、readなし


=== ケース: "Hello" (改行なし、1行のみ) ===

get_next_line(fd) 呼び出し1:
  stash = NULL -> ft_init_stash -> stash = ""
  ft_read_to_stash:
    stashに\nなし -> read -> "Hello" (BUFFER_SIZE>=5なら1回)
    stash = "" + "Hello" = "Hello"
    stashに\nなし -> read -> 0 (EOF) -> ループ終了
  stash[0] != '\0' -> 続行
  ft_get_line("Hello"):
    while (stash[i] && stash[i] != '\n') -> i = 5
    stash[5] == '\0' -> '\n'なし
    line = malloc(6) -> "Hello\0"
  ft_trim_stash("Hello"):
    while (stash[i] && stash[i] != '\n') -> i = 5
    !stash[5] -> free(stash) -> return NULL
  return "Hello"

get_next_line(fd) 呼び出し2:
  stash = NULL -> ft_init_stash -> stash = ""
  ft_read_to_stash:
    read -> 0 (EOF)
  stash[0] == '\0' -> return NULL
```

### 4.6 BUFFER_SIZE境界ケースの詳細トレース

```
=== BUFFER_SIZE=3, ファイル="ABCD\n" ===

get_next_line(fd):
  stash = "" (初期化済み)

  ft_read_to_stash:
    1回目のread: read(fd, buf, 3) -> 3
      buf = "ABC\0"
      stash = "" + "ABC" = "ABC"
      \nなし -> 続行

    2回目のread: read(fd, buf, 3) -> 2
      buf = "D\n\0"     ← 3バイト要求したが2バイトしか読めない
      stash = "ABC" + "D\n" = "ABCD\n"
      \nあり -> ループ終了

  ft_get_line("ABCD\n") -> "ABCD\n"
  ft_trim_stash("ABCD\n"):
    \nの後が'\0' -> free(stash) -> return NULL

  return "ABCD\n"

ポイント:
  ・read()は要求したバイト数より少なく返すことがある
  ・buf[bytes_read] = '\0' で正しくヌル終端されること
  ・バッファ境界で\nが分断されても正しく動作すること
```

---

## 5. 使用禁止事項

### 5.1 禁止関数

| 関数/ライブラリ | 理由 |
|---------------|------|
| libft | このプロジェクト独自の実装が求められる |
| `lseek()` | ファイルオフセットの手動操作は禁止 |
| `fgets()` | stdio系のバッファリング関数は禁止 |
| `getline()` | 同上 |

### 5.2 禁止パターン

| パターン | 理由 |
|---------|------|
| グローバル変数 | Normの規定およびプロジェクト要件 |
| ファイル全体の一括読み込み（lseekでサイズ取得等） | 「可能な限り少なく読む」要件に違反 |

### 5.3 「可能な限り少なく読む」要件の解釈

この要件は以下を意味します:

1. stashに既に`\n`がある場合、read()を呼ばない
2. `\n`が見つかった時点でreadループを終了する
3. ファイルサイズを事前に調べてまとめて読むことはしない

```
正しい動作:
  stash = "Hello\nWorld\n"
  -> \nが既にあるので、readを呼ばずにstashから行を切り出す

  stash = "Hello"
  -> \nがないので、readを呼んでデータを追加する
  -> "HelloWorld\nFoo" になったら、\nが見つかったのでread終了

正しくない動作:
  lseek(fd, 0, SEEK_END) でファイルサイズを取得
  -> malloc(filesize) で全体を確保
  -> read(fd, buf, filesize) で一括読み込み
  -> これは禁止!
```

### 5.4 使用できる関数の一覧

```
使用可能な外部関数:
  read()   - ファイルからの読み取り（唯一のI/O関数）
  malloc() - メモリの動的確保
  free()   - メモリの解放

使用可能だが不要な関数（テストコードで使用可）:
  open()   - ファイルを開く（main.cで使用）
  close()  - ファイルを閉じる（main.cで使用）
  printf() - 出力（テスト・デバッグ用）
  write()  - 出力（テスト・デバッグ用）

使用禁止:
  lseek()    - ファイルオフセットの操作
  fgets()    - stdioの行読み取り
  getline()  - POSIXの行読み取り
  fread()    - stdioのバッファ読み取り
  mmap()     - メモリマッピング
  libft関数  - 自前で実装する必要
```

---

## 6. Norm準拠チェックリスト

### 6.1 関数ルール

- [ ] 1関数あたり最大25行（開き中括弧`{`と閉じ中括弧`}`を除く）
- [ ] 1ファイルあたり最大5関数
- [ ] 関数パラメータは最大4つ
- [ ] 変数宣言は関数の先頭に配置
- [ ] 1行に1つの変数宣言（`int a, b;`は禁止）
- [ ] 宣言と実行コードの間に空行

### 6.2 フォーマット

- [ ] 1行は最大80文字（コメント含む）
- [ ] インデントはタブ（スペースではない）
- [ ] 各ファイルの先頭に42ヘッダ
- [ ] ヘッダファイルにインクルードガード

### 6.3 具体例: Norm準拠のコード

```c
/* 良い例: Norm準拠 */
char	*ft_get_line(char *stash)
{
	size_t	i;          /* 変数宣言は先頭 */
	char	*line;      /* 1行に1つの宣言 */
	                    /* 宣言と実行の間に空行 */
	i = 0;
	if (!stash || !stash[0])
		return (NULL);
	while (stash[i] && stash[i] != '\n')
		i++;
	if (stash[i] == '\n')
		i++;
	line = malloc(sizeof(char) * (i + 1));
	if (!line)
		return (NULL);
	/* ... 以下省略（25行以内に収める） */
	return (line);
}

/* 悪い例: Norm違反 */
char *ft_get_line(char *stash) {   /* { は別の行に */
    size_t i;                       /* タブでなくスペース */
    char *line;
    i = 0;                          /* 宣言と実行の間に空行がない */
    int j = 0;                      /* 宣言が先頭でない */
    /* ... */
}
```

### 6.4 Normで見落としやすいポイント

```
見落としポイント1: return 文の括弧
  × return NULL;
  ○ return (NULL);
  → 42 Norm では return の値を括弧で囲む

見落としポイント2: ポインタの宣言
  × char* ptr;
  ○ char	*ptr;
  → * は変数名側に付ける、タブでインデント

見落としポイント3: if/while の後の空白
  × if(!stash)
  ○ if (!stash)
  → キーワードと括弧の間にスペース

見落としポイント4: 関数の引数の空白
  × func( arg1,arg2 )
  ○ func(arg1, arg2)
  → 括弧の内側にスペースなし、カンマの後にスペース

見落としポイント5: 演算子の空白
  × i=0;
  ○ i = 0;
  → 代入演算子の前後にスペース

見落としポイント6: 行末の空白
  × "stash = NULL;   \n"  (末尾にスペース)
  ○ "stash = NULL;\n"
  → 行末に不要な空白があるとNorm違反
```

### 6.5 ファイルごとのNormチェック

```
get_next_line.h:
  - [ ] 42ヘッダが正しい形式
  - [ ] #ifndef GET_NEXT_LINE_H / #define / #endif
  - [ ] BUFFER_SIZEの #ifndef ガード
  - [ ] 全関数のプロトタイプが宣言されている
  - [ ] 不要な #include がない
  - [ ] 関数定義が含まれていない（プロトタイプのみ）

get_next_line.c:
  - [ ] 5関数以下（現在3関数）
  - [ ] 各関数が25行以下（中括弧除く）
  - [ ] 変数宣言が関数先頭
  - [ ] 1行80文字以下
  - [ ] static関数にstaticキーワードがある
  - [ ] #include "get_next_line.h" がある

get_next_line_utils.c:
  - [ ] 5関数以下（現在ちょうど5関数）
  - [ ] 各関数が25行以下
  - [ ] #include "get_next_line.h" がある
```

### 6.6 norminetteコマンドの使い方

```bash
# 個別ファイルのチェック
norminette get_next_line.c

# 出力例（成功）:
# get_next_line.c: OK!

# 出力例（失敗）:
# get_next_line.c: Error!
# Error: SPACE_BEFORE_FUNC    (line:  15, col:   6): Bad spacing before function name
# Error: TOO_MANY_FUNCS       (line:  80): Too many functions in file

# 全ファイル一括チェック
norminette get_next_line.c get_next_line.h get_next_line_utils.c

# ボーナスも含めた全ファイルチェック
norminette get_next_line*.c get_next_line*.h
```

---

## 7. ボーナス要件

### 7.1 ボーナスの概要

ボーナスでは、**複数のファイルディスクリプタに対して同時に**
get_next_lineを呼び出せるようにします。

### 7.2 必須版の問題点

```c
// 必須版: static変数が1つだけ
static char *stash;

// 問題のシナリオ:
get_next_line(3);  // stash = "file_a の残りデータ"
get_next_line(4);  // stash に file_b のデータが混ざる!
get_next_line(3);  // file_a の残りデータが消えている!
```

### 7.3 ボーナス版の解決策

```c
// ボーナス版: fdごとに独立したstash
static char *stash[MAX_FD];

get_next_line(3);  // stash[3] = "file_a の残りデータ"
get_next_line(4);  // stash[4] = "file_b の残りデータ"
                   // stash[3] は影響を受けない!
get_next_line(3);  // stash[3] の続きから正常に読める!
```

### 7.4 ボーナスの追加チェック

```c
// fdの範囲チェック（必須版にはない）
if (fd < 0 || fd >= MAX_FD || BUFFER_SIZE <= 0)
    return (NULL);
```

### 7.5 ボーナスのテスト方法

```c
// テスト用 main.c
int	main(void)
{
	int		fd1;
	int		fd2;
	char	*line;

	fd1 = open("file1.txt", O_RDONLY);
	fd2 = open("file2.txt", O_RDONLY);
	line = get_next_line(fd1);
	printf("fd1: %s", line);
	free(line);
	line = get_next_line(fd2);
	printf("fd2: %s", line);
	free(line);
	line = get_next_line(fd1);
	printf("fd1: %s", line);
	free(line);
	close(fd1);
	close(fd2);
	return (0);
}
```

### 7.6 ボーナスの詳細テスト

```c
// 3つのfdの交互読み取りテスト
int	main(void)
{
	int		fd1;
	int		fd2;
	int		fd3;
	char	*line;

	fd1 = open("file1.txt", O_RDONLY);
	fd2 = open("file2.txt", O_RDONLY);
	fd3 = open("file3.txt", O_RDONLY);

	// 交互に読み取り
	line = get_next_line(fd1);  // file1 の1行目
	printf("fd1-1: %s", line);
	free(line);
	line = get_next_line(fd2);  // file2 の1行目
	printf("fd2-1: %s", line);
	free(line);
	line = get_next_line(fd3);  // file3 の1行目
	printf("fd3-1: %s", line);
	free(line);
	line = get_next_line(fd1);  // file1 の2行目
	printf("fd1-2: %s", line);
	free(line);
	// ...

	close(fd1);
	close(fd2);
	close(fd3);
	return (0);
}
```

---

## 8. テストファイルの作り方

### 8.1 基本テストファイル

```bash
# 通常の複数行ファイル
printf "Line 1\nLine 2\nLine 3\n" > test_normal.txt

# 改行なしの最終行
printf "Line 1\nLine 2\nLast line without newline" > test_no_newline.txt

# 空ファイル
> test_empty.txt

# 改行のみ
printf "\n" > test_newline_only.txt

# 連続改行
printf "\n\n\n" > test_multi_newline.txt

# 1文字のみ
printf "A" > test_single_char.txt

# 1文字+改行
printf "A\n" > test_single_char_nl.txt

# 長い行（1000文字）
python3 -c "print('A' * 1000)" > test_long_line.txt

# 非常に長い行（100000文字）
python3 -c "print('B' * 100000)" > test_very_long_line.txt

# 多数の短い行
python3 -c "
for i in range(1000):
    print(f'Line {i}')
" > test_many_lines.txt

# 空行を含むファイル
printf "Hello\n\nWorld\n\n\nEnd\n" > test_empty_lines.txt
```

### 8.2 テストの網羅性確認

```
テストマトリクス:

                BUFFER_SIZE
                1    42   9999  10000000
空ファイル       [  ] [  ] [  ]  [  ]
1行(NLあり)     [  ] [  ] [  ]  [  ]
1行(NLなし)     [  ] [  ] [  ]  [  ]
複数行          [  ] [  ] [  ]  [  ]
長い行          [  ] [  ] [  ]  [  ]
連続改行        [  ] [  ] [  ]  [  ]
改行のみ        [  ] [  ] [  ]  [  ]
fd=-1           [  ] [  ] [  ]  [  ]
stdin           [  ] [  ] [  ]  [  ]

全てのマスで正常に動作すること!
```

### 8.3 テストファイルの中身をhexで確認する

```bash
# ファイルの内容を16進数で表示
xxd test_normal.txt

# 出力例:
# 00000000: 4c69 6e65 2031 0a4c 696e 6520 320a 4c69  Line 1.Line 2.Li
# 00000010: 6e65 2033 0a                              ne 3.

# 0a = '\n' であることを確認
# ファイル末尾の 0a の有無で最終行の動作が変わる

# 空ファイルの確認
xxd test_empty.txt
# (何も出力されない = 0バイト)

# 連続改行の確認
xxd test_multi_newline.txt
# 00000000: 0a0a 0a                                  ...
# 3バイト、全て 0a (\n)
```

---

## 9. 「正しさ」のチェック方法

### 9.1 出力の正確性チェック

```c
// テスト用main.c: 行ごとに番号と長さを表示
int	main(void)
{
	int		fd;
	char	*line;
	int		i;

	fd = open("test.txt", O_RDONLY);
	i = 1;
	while ((line = get_next_line(fd)) != NULL)
	{
		printf("Line %d (len=%zu): [%s]", i,
			strlen(line), line);
		free(line);
		i++;
	}
	printf("EOF (NULL returned)\n");
	close(fd);
	return (0);
}
```

### 9.2 メモリの正確性チェック

```bash
# 全てのBUFFER_SIZEでvalgrindを通す
for bs in 1 42 9999 10000000; do
    echo "=== BUFFER_SIZE=$bs ==="
    cc -g -D BUFFER_SIZE=$bs gnl.c gnl_utils.c main.c -o gnl
    valgrind --leak-check=full --error-exitcode=1 ./gnl
    echo "Exit code: $?"
done
```

### 9.3 Normチェック

```bash
norminette get_next_line.c get_next_line.h get_next_line_utils.c
# 全ファイルで "OK!" が出力されることを確認
```

### 9.4 差分チェック（期待出力との比較）

```bash
# 期待出力ファイルを作成
cat > expected.txt << 'EOF'
Line 1: [Line 1
]
Line 2: [Line 2
]
Line 3: [Line 3
]
EOF (NULL returned)
EOF

# 実際の出力と比較
./gnl > actual.txt
diff expected.txt actual.txt
# 差分がなければ正しい
```
