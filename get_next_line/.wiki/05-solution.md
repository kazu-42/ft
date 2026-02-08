# 05 - ソリューション解説

この章では、get_next_lineの実装を1行ずつ解説し、
複数のBUFFER_SIZEでの完全な実行トレースを提供します。
コードの「何をしているか」だけでなく「なぜそうするのか」を
徹底的に説明します。

---

## 1. アルゴリズム概要

### 1.1 4段階のアルゴリズム

```
┌─────────────────────────┐
│ 1. バリデーション         │  fd < 0, BUFFER_SIZE <= 0 のチェック
└────────────┬────────────┘
             v
┌─────────────────────────┐
│ 2. 初期化                │  stash が NULL なら空文字列を確保
│    (ft_init_stash)      │
└────────────┬────────────┘
             v
┌─────────────────────────┐
│ 3. 読み込みと蓄積        │  stash に \n が見つかるまで
│    (ft_read_to_stash)   │  read() を繰り返す
└────────────┬────────────┘
             v
┌─────────────────────────┐
│ 4. 行の抽出と残余保存     │  stash から 1行分を切り出し
│    (ft_get_line +        │  残りを次回用に保存
│     ft_trim_stash)       │
└─────────────────────────┘
```

### 1.2 なぜこの「stash蓄積パターン」を採用するのか

**問題**: read()はBUFFER_SIZE単位でデータを読むが、
行の区切り(\n)はBUFFER_SIZEの倍数の位置にあるとは限らない。

```
例: BUFFER_SIZE=5, ファイル="AB\nCDEF\nGH"

read()が返すデータ: "AB\nCD" | "EF\nGH"
                     ^^^^^^^^^^
                     行の境界とバッファの境界が一致しない!

必要な出力:
  1回目: "AB\n"
  2回目: "CDEF\n"
  3回目: "GH"
```

**解決策**: stashに「読みすぎたデータ」を蓄積し、
行の境界を自分で管理する。

他のアプローチとの比較:

| アプローチ | 利点 | 欠点 | 採用理由 |
|-----------|------|------|---------|
| **stash蓄積型** | シンプル、バグが少ない | strjoinの度にメモリ再確保 | Normに収まりやすい |
| リンクリスト型 | メモリコピーが少ない | 実装が複雑 | 25行制限で困難 |
| 固定バッファ型 | メモリ確保が少ない | 長い行に対応できない | 行の長さが不定なため不可 |
| リングバッファ型 | メモリ効率が良い | 実装が非常に複雑 | Normの制約で困難 |

---

## 2. ヘッダファイル解説

### 2.1 get_next_line.h の全行解説

```c
#ifndef GET_NEXT_LINE_H        // 多重include防止（開始）
# define GET_NEXT_LINE_H       // マクロ定義

# include <stdlib.h>           // malloc(), free(), size_t
# include <unistd.h>           // read(), ssize_t

# ifndef BUFFER_SIZE           // -D で指定されなかった場合
#  define BUFFER_SIZE 42       // デフォルト値 42
# endif                        // #ifndef BUFFER_SIZE の終了

// --- 関数プロトタイプ ---
char   *get_next_line(int fd);           // メイン関数
size_t  ft_strlen(const char *s);        // NULLセーフなstrlen
char   *ft_strchr(const char *s, int c); // NULLセーフなstrchr
char   *ft_strjoin(char *s1, char *s2);  // 結合 + s1解放
char   *ft_get_line(char *stash);        // 行の切り出し
char   *ft_trim_stash(char *stash);      // 残余の保存

#endif                         // 多重include防止（終了）
```

**設計ポイント**:
- `#ifndef`/`#define`/`#endif`のインクルードガードは必須
- BUFFER_SIZEの`#ifndef`ガードにより、`-D`フラグとデフォルト値の両方に対応
- `ft_strjoin`の引数は`char *s1`（constなし）。内部でs1をfreeするため。
- `ft_get_line`と`ft_trim_stash`の引数もconstなし。stashの操作のため。

---

## 3. get_next_line.c - メイン関数群

### 3.1 get_next_line() の全行解説

```c
char	*get_next_line(int fd)
{
	static char	*stash;    // [A] 呼び出し間で保持されるstash
	char		*line;      // [B] 返却する行

	if (fd < 0 || BUFFER_SIZE <= 0)    // [C] 入力バリデーション
		return (NULL);
	stash = ft_init_stash(stash);      // [D] stashの初期化
	if (!stash)                         // [E] malloc失敗チェック
		return (NULL);
	stash = ft_read_to_stash(fd, stash); // [F] データ読み込み
	if (!stash)                           // [G] エラーチェック
		return (NULL);
	if (stash[0] == '\0')               // [H] データなし(EOF)
	{
		free(stash);                    // [I] stash解放
		stash = NULL;                   // [J] NULLに設定
		return (NULL);                  // [K] EOF通知
	}
	line = ft_get_line(stash);          // [L] 行の切り出し
	stash = ft_trim_stash(stash);       // [M] 残余の保存
	return (line);                      // [N] 行を返す
}
```

**各行の詳細解説**:

```
[A] static char *stash
  → BSSセグメントに配置される8バイトのポインタ
  → プログラム開始時にNULLで初期化（明示的な初期化不要）
  → 関数が終了してもstashの値は保持される
  → これがget_next_lineの「記憶」の正体

[B] char *line
  → スタック上の一時変数（ポインタ）
  → ft_get_line()の戻り値を受け取る

[C] if (fd < 0 || BUFFER_SIZE <= 0)
  → 不正な引数を早期にリジェクト
  → この時点ではメモリ確保していないので、そのままreturn NULL
  → BUFFER_SIZE はマクロなのでコンパイル時に値が確定

[D] stash = ft_init_stash(stash)
  → stashがNULLなら空文字列""を確保
  → stashがNULLでなければそのまま返す（何もしない）
  → これにより、以降のft_strjoinでNULLを渡す心配がなくなる

[E] if (!stash)
  → ft_init_stash内のmallocが失敗した場合
  → この時点で確保されたリソースはないので、そのままNULL返却

[F] stash = ft_read_to_stash(fd, stash)
  → stashに\nが見つかるまでread()を繰り返す
  → stashを引数として渡し、更新されたstashを受け取る
  → static stashを直接更新する（stash = の代入が重要）

[G] if (!stash)
  → ft_read_to_stash内でread()エラー or malloc失敗
  → read_to_stash内でstashはfree済み
  → static stashがNULLになり、次回の呼び出しは初期状態から開始

[H] if (stash[0] == '\0')
  → readが0バイト返した（EOF）かつ前回の残りデータもない
  → stashは空文字列""の状態

[I] free(stash)
  → 空文字列を解放

[J] stash = NULL
  → static変数をNULLにリセット
  → これにより次回呼び出し時にft_init_stashで再初期化される
  → valgrindの"still reachable"も防止

[K] return (NULL)
  → EOFを呼び出し元に通知

[L] line = ft_get_line(stash)
  → stashから最初の行（\n含む）を切り出して新しいmallocメモリに格納
  → stash自体は変更されない（読み取りのみ）

[M] stash = ft_trim_stash(stash)
  → stashから使用済み部分（line分）を除去
  → 古いstashはft_trim_stash内部でfreeされる
  → 残りがあれば新しいmalloc、なければNULLが返される

[N] return (line)
  → 呼び出し元にlineを返す
  → 呼び出し元がfreeする責任を持つ
```

### 3.2 ft_init_stash() の全行解説

```c
static char	*ft_init_stash(char *stash)
{
	if (!stash)                          // stashがNULLの場合のみ
	{
		stash = malloc(sizeof(char) * 1); // 1バイト確保
		if (!stash)                       // malloc失敗チェック
			return (NULL);
		stash[0] = '\0';                 // 空文字列にする
	}
	return (stash);                      // 初期化済みstashを返す
}
```

```
ft_init_stash のメモリ変化:

ケース1: stash == NULL (初回 or EOF後)

  Before:  stash = NULL (何も指していない)

  malloc(1):
  stash ──→ ['\0']    ← ヒープ上の1バイト
             0x100

  After:   stash = 0x100 (空文字列を指す)


ケース2: stash != NULL (前回の残りデータあり)

  Before:  stash ──→ ['W']['o']['r']['l']['d']['\0']
                      0x200

  何もしない (そのまま返す)

  After:   stash ──→ ['W']['o']['r']['l']['d']['\0']
                      0x200  (変化なし)
```

### 3.3 ft_read_to_stash() の全行解説

```c
static char	*ft_read_to_stash(int fd, char *stash)
{
	char	*buf;
	int		bytes_read;

	buf = malloc(sizeof(char) * (BUFFER_SIZE + 1));  // [1]
	if (!buf)                                          // [2]
		return (NULL);
	bytes_read = 1;                                    // [3]
	while (!ft_strchr(stash, '\n') && bytes_read > 0)  // [4]
	{
		bytes_read = read(fd, buf, BUFFER_SIZE);        // [5]
		if (bytes_read == -1)                           // [6]
		{
			free(buf);                                  // [7]
			free(stash);                                // [8]
			return (NULL);                              // [9]
		}
		buf[bytes_read] = '\0';                         // [10]
		stash = ft_strjoin(stash, buf);                 // [11]
	}
	free(buf);                                          // [12]
	return (stash);                                     // [13]
}
```

**各行の詳細**:

```
[1] buf = malloc(BUFFER_SIZE + 1)
  → read()用の一時バッファを確保
  → +1はヌル終端用（read()はヌル終端を付けない）

[2] if (!buf) return NULL
  → malloc失敗時。stashはこの関数のものではないので
    ここではfreeしない（呼び出し元の判断に委ねる）
  → ただし注意: この場合get_next_lineでstashがリークする
    可能性がある。実装によってはここでstashもfreeすべき。

[3] bytes_read = 1
  → ループの初回実行を保証するためのダミー値
  → 0にするとループに入らない（bytes_read > 0 が偽になる）

[4] while (!ft_strchr(stash, '\n') && bytes_read > 0)
  → 2つの条件:
    a) stashに\nがない → まだ1行分のデータが揃っていない
    b) bytes_read > 0 → まだEOFに達していない
  → どちらかが偽になったらループ終了

[5] bytes_read = read(fd, buf, BUFFER_SIZE)
  → fdからBUFFER_SIZEバイトまで読み取る
  → 実際に読めたバイト数がbytes_readに入る
  → 0ならEOF、-1ならエラー

[6] if (bytes_read == -1)
  → readエラー（無効なfd、I/Oエラーなど）

[7] free(buf)
  → 読み取りバッファを解放

[8] free(stash)
  → 蓄積バッファも解放（エラー時は全リソースを解放）

[9] return NULL
  → NULLを返すことで、呼び出し元のstatic stashもNULLになる

[10] buf[bytes_read] = '\0'
  → read()はヌル終端を付けないため、手動で追加
  → bytes_readが3なら、buf[3] = '\0'
  → これでbufがC文字列として使える

[11] stash = ft_strjoin(stash, buf)
  → stash + buf を結合した新しい文字列を確保
  → 古いstashはft_strjoin内部でfreeされる
  → stashが新しいアドレスに更新される

[12] free(buf)
  → ループ終了後、一時バッファを解放

[13] return stash
  → 蓄積されたデータを返す
```

---

## 4. get_next_line_utils.c - ヘルパー関数群

### 4.1 ft_strlen() の解説

```c
size_t	ft_strlen(const char *s)
{
	size_t	i;

	i = 0;
	if (!s)               // NULLセーフ: sがNULLなら0を返す
		return (0);
	while (s[i])          // '\0'が見つかるまでカウント
		i++;
	return (i);
}
```

標準のstrlen()との違い: NULLを渡してもクラッシュしない。

### 4.2 ft_strchr() の解説

```c
char	*ft_strchr(const char *s, int c)
{
	if (!s)                        // NULLセーフ
		return (NULL);
	while (*s)                     // 文字列を走査
	{
		if (*s == (char)c)          // 文字cを発見
			return ((char *)s);     // その位置のポインタを返す
		s++;
	}
	if ((char)c == '\0')           // '\0'を検索する場合
		return ((char *)s);         // 文字列末尾のポインタを返す
	return (NULL);                 // 見つからなかった
}
```

**get_next_lineでの使い方**: `ft_strchr(stash, '\n')`
- stashに'\n'が含まれていれば、その位置のポインタが返る（非NULL）
- 含まれていなければNULLが返る
- while条件: `!ft_strchr(stash, '\n')` → '\n'がない間ループ

### 4.3 ft_strjoin() の解説（メモリ図付き）

```c
char	*ft_strjoin(char *s1, char *s2)
{
	size_t	i;
	size_t	j;
	char	*joined;

	if (!s1 || !s2)                                      // NULLチェック
		return (NULL);
	joined = malloc(sizeof(char) * (ft_strlen(s1)        // 結合後のサイズを計算
		+ ft_strlen(s2) + 1));                             // +1 は '\0' 用
	if (!joined)
	{
		free(s1);                                          // malloc失敗でもs1を解放!
		return (NULL);
	}
	i = 0;
	while (s1[i])                                         // s1をコピー
	{
		joined[i] = s1[i];
		i++;
	}
	j = 0;
	while (s2[j])                                         // s2をコピー
	{
		joined[i + j] = s2[j];
		j++;
	}
	joined[i + j] = '\0';                                 // ヌル終端
	free(s1);                                              // 古いs1(stash)を解放!
	return (joined);
}
```

```
ft_strjoin("Hel", "lo\n") の実行トレース:

Before:
  s1 ──→ ['H']['e']['l']['\0']    (3 + 1 = 4バイト)
          アドレス 0x100
  s2 ──→ ['l']['o']['\n']['\0']   (3 + 1 = 4バイト)
          アドレス 0x200

Step 1: malloc(3 + 3 + 1 = 7)
  joined ──→ [?][?][?][?][?][?][?]   (7バイト, 未初期化)
              アドレス 0x300

Step 2: s1 をコピー (i: 0→1→2→3)
  joined ──→ ['H']['e']['l'][?][?][?][?]
              i=0   i=1  i=2

Step 3: s2 をコピー (j: 0→1→2→3)
  joined ──→ ['H']['e']['l']['l']['o']['\n'][?]
                            j=0   j=1   j=2

Step 4: ヌル終端 (joined[3+3] = '\0')
  joined ──→ ['H']['e']['l']['l']['o']['\n']['\0']

Step 5: free(s1)
  0x100 の領域が解放される

After:
  joined ──→ ['H']['e']['l']['l']['o']['\n']['\0']
              アドレス 0x300

  戻り値: 0x300
  ★ s1(旧stash)は解放済み → stashに新しいjoined(0x300)が代入される
```

### 4.4 ft_get_line() の解説（メモリ図付き）

```c
char	*ft_get_line(char *stash)
{
	size_t	i;
	char	*line;

	i = 0;
	if (!stash || !stash[0])          // NULLチェック + 空文字列チェック
		return (NULL);
	while (stash[i] && stash[i] != '\n')  // \nまたは\0まで進む
		i++;
	if (stash[i] == '\n')             // \nが見つかった場合
		i++;                           // \nも含める（i = \nの次の位置）
	line = malloc(sizeof(char) * (i + 1));  // 行の長さ + \0
	if (!line)
		return (NULL);
	i = 0;                            // 再度先頭から
	while (stash[i] && stash[i] != '\n')  // \nまたは\0までコピー
	{
		line[i] = stash[i];
		i++;
	}
	if (stash[i] == '\n')             // \nがあれば
		line[i++] = '\n';             // \nもlineに含める
	line[i] = '\0';                   // ヌル終端
	return (line);
}
```

```
ft_get_line("Hello\nWorld") の実行トレース:

stash: ['H']['e']['l']['l']['o']['\n']['W']['o']['r']['l']['d']['\0']
         0    1    2    3    4    5     6    7    8    9   10    11

Phase 1: 行の長さを計算
  i=0: 'H' != '\n' → i++
  i=1: 'e' != '\n' → i++
  i=2: 'l' != '\n' → i++
  i=3: 'l' != '\n' → i++
  i=4: 'o' != '\n' → i++
  i=5: '\n' == '\n' → ループ終了
  stash[5] == '\n' → i++ → i=6
  → malloc(6 + 1 = 7)

Phase 2: コピー
  i=0: line[0] = 'H'
  i=1: line[1] = 'e'
  i=2: line[2] = 'l'
  i=3: line[3] = 'l'
  i=4: line[4] = 'o'
  stash[5] == '\n' → line[5] = '\n', i=6
  line[6] = '\0'

結果:
  line: ['H']['e']['l']['l']['o']['\n']['\0']
         0    1    2    3    4    5     6

  ★ stash は変更されない（読み取りのみ）
```

### 4.5 ft_trim_stash() の解説（メモリ図付き）

```c
char	*ft_trim_stash(char *stash)
{
	size_t	i;
	size_t	j;
	char	*trimmed;

	i = 0;
	if (!stash)                        // NULLチェック
		return (NULL);
	while (stash[i] && stash[i] != '\n')  // \nまたは\0まで進む
		i++;
	if (!stash[i])                     // \nが見つからない = 最終行
	{
		free(stash);                   // stash全体が1行 → 残りなし
		return (NULL);                 // NULLを返す(次回はinit_stashで初期化)
	}
	trimmed = malloc(sizeof(char) * (ft_strlen(stash) - i));  // 残りのサイズ
	if (!trimmed)
	{
		free(stash);                   // malloc失敗でもstashは解放
		return (NULL);
	}
	i++;                               // \nの次の位置に移動
	j = 0;
	while (stash[i])                   // 残りをコピー
		trimmed[j++] = stash[i++];
	trimmed[j] = '\0';                 // ヌル終端
	free(stash);                       // 古いstashを解放
	return (trimmed);                  // 残りデータを返す
}
```

```
ft_trim_stash("Hello\nWorld") の実行トレース:

stash: ['H']['e']['l']['l']['o']['\n']['W']['o']['r']['l']['d']['\0']
         0    1    2    3    4    5     6    7    8    9   10    11

Phase 1: \nの位置を探す
  i=0: 'H' → i++
  i=1: 'e' → i++
  ...
  i=5: '\n' → ループ終了
  stash[5] != '\0' → 残りデータあり

Phase 2: 残りのサイズを計算
  ft_strlen(stash) = 11
  11 - 5 = 6 → malloc(6)
  ★ なぜ6? → "World\0" = 5文字 + \0 = 6バイト

Phase 3: コピー
  i = 6 (i++で\nの次に移動)
  j=0: trimmed[0] = stash[6] = 'W'
  j=1: trimmed[1] = stash[7] = 'o'
  j=2: trimmed[2] = stash[8] = 'r'
  j=3: trimmed[3] = stash[9] = 'l'
  j=4: trimmed[4] = stash[10] = 'd'
  stash[11] = '\0' → ループ終了
  trimmed[5] = '\0'

Phase 4: 古いstashを解放
  free(stash)

結果:
  trimmed: ['W']['o']['r']['l']['d']['\0']
            0    1    2    3    4    5
```

---

## 5. 完全実行トレース: BUFFER_SIZE=1

ファイル内容: `"AB\nCD"` (5バイト)

```
=== get_next_line(fd) 呼び出し1 ===

[get_next_line]
  static stash = NULL (初回)
  fd >= 0, BUFFER_SIZE > 0 → OK

[ft_init_stash(NULL)]
  stash == NULL → malloc(1) → stash = ""
  return ""

[ft_read_to_stash(fd, "")]
  buf = malloc(2)  // BUFFER_SIZE(1) + 1
  bytes_read = 1 (ダミー)

  --- ループ 1回目 ---
  ft_strchr("", '\n') → NULL → !NULL = 真
  bytes_read(1) > 0 → 真 → ループ実行

  read(fd, buf, 1) → 1バイト  buf = "A\0"
  stash = ft_strjoin("", "A")
    joined = malloc(0 + 1 + 1 = 2)
    joined = "A\0"
    free("") ← 古いstash解放
    return "A"
  stash = "A"

  --- ループ 2回目 ---
  ft_strchr("A", '\n') → NULL → ループ続行
  read(fd, buf, 1) → 1バイト  buf = "B\0"
  stash = ft_strjoin("A", "B")
    joined = malloc(1 + 1 + 1 = 3) = "AB\0"
    free("A")
  stash = "AB"

  --- ループ 3回目 ---
  ft_strchr("AB", '\n') → NULL → ループ続行
  read(fd, buf, 1) → 1バイト  buf = "\n\0"
  stash = ft_strjoin("AB", "\n")
    joined = malloc(2 + 1 + 1 = 4) = "AB\n\0"
    free("AB")
  stash = "AB\n"

  --- ループ 4回目 ---
  ft_strchr("AB\n", '\n') → 非NULL → !非NULL = 偽 → ループ終了

  free(buf)
  return "AB\n"

[get_next_line 続き]
  stash = "AB\n"
  stash[0] = 'A' != '\0' → OK

  line = ft_get_line("AB\n")
    → line = "AB\n\0" (malloc(4))

  stash = ft_trim_stash("AB\n")
    i=0:'A', i=1:'B', i=2:'\n' → ループ終了
    stash[2]='\n' → 残りデータ確認
    ft_strlen("AB\n") = 3, 3 - 2 = 1 → malloc(1)
    i=3: stash[3]='\0' → ループ即終了
    trimmed = "\0" (空文字列)
    free("AB\n")
    return ""

  stash = ""
  return "AB\n"  ← 1行目を返す


=== get_next_line(fd) 呼び出し2 ===

[get_next_line]
  static stash = "" (前回の残り)

[ft_init_stash("")]
  stash != NULL → そのまま返す
  return ""

[ft_read_to_stash(fd, "")]
  buf = malloc(2)
  bytes_read = 1

  --- ループ 1回目 ---
  ft_strchr("", '\n') → NULL → ループ続行
  read → 1バイト  buf = "C\0"
  stash = ft_strjoin("", "C") = "C"

  --- ループ 2回目 ---
  ft_strchr("C", '\n') → NULL → ループ続行
  read → 1バイト  buf = "D\0"
  stash = ft_strjoin("C", "D") = "CD"

  --- ループ 3回目 ---
  ft_strchr("CD", '\n') → NULL → ループ続行
  read → 0バイト (EOF)
  bytes_read = 0 → ループ条件 bytes_read > 0 が偽 → ループ終了

  ★ bytes_read=0のとき、buf[0]='\0'、ft_strjoinは実行されない
  ★ 正確には: read後にbuf[0]='\0'は実行されるが、
    stash = ft_strjoin(stash, "") では stash は変化しない
    （ただし古いstashはfreeされ新しいmalloc）

  free(buf)
  return "CD"

[get_next_line 続き]
  stash = "CD"
  stash[0] = 'C' != '\0' → OK

  line = ft_get_line("CD")
    → '\n'なし → line = "CD\0" (malloc(3))

  stash = ft_trim_stash("CD")
    i=0:'C', i=1:'D', i=2:'\0' → ループ終了
    stash[2] = '\0' → \nなし → free("CD") → return NULL

  stash = NULL
  return "CD"  ← 2行目を返す（最終行、\nなし）


=== get_next_line(fd) 呼び出し3 ===

[get_next_line]
  static stash = NULL

[ft_init_stash(NULL)]
  → malloc(1) → stash = ""

[ft_read_to_stash(fd, "")]
  buf = malloc(2)
  bytes_read = 1

  --- ループ 1回目 ---
  ft_strchr("", '\n') → NULL → ループ続行
  read → 0バイト (EOF)
  buf[0] = '\0'
  stash = ft_strjoin("", "") = ""
  bytes_read = 0 → ループ終了

  free(buf)
  return ""

[get_next_line 続き]
  stash = ""
  stash[0] == '\0' → free("") → stash = NULL → return NULL
```

---

## 6. 完全実行トレース: BUFFER_SIZE=5

ファイル内容: `"AB\nCDEF\nGH"` (11バイト)

```
=== get_next_line(fd) 呼び出し1 ===

  stash = NULL → init → ""

  ft_read_to_stash:
    buf = malloc(6)

    ループ1回目:
      read → "AB\nCD" (5バイト)  buf = "AB\nCD\0"
      stash = ft_strjoin("", "AB\nCD") = "AB\nCD"
      ft_strchr("AB\nCD", '\n') → 非NULL → ループ終了

    free(buf)
    return "AB\nCD"

  ft_get_line("AB\nCD"):
    i: 0→'A', 1→'B', 2→'\n' → ループ終了
    stash[2]=='\n' → i=3
    malloc(4) → "AB\n\0"

  ft_trim_stash("AB\nCD"):
    i: 0→'A', 1→'B', 2→'\n' → ループ終了
    ft_strlen("AB\nCD")=5, 5-2=3 → malloc(3)
    i=3: copy 'C','D' → trimmed="CD\0"
    free("AB\nCD")

  stash = "CD"
  return "AB\n"


=== get_next_line(fd) 呼び出し2 ===

  stash = "CD" (前回の残り)
  init: stash != NULL → そのまま

  ft_read_to_stash:
    buf = malloc(6)

    ループ1回目:
      ft_strchr("CD", '\n') → NULL → ループ続行
      read → "EF\nGH" (5バイト)
      stash = ft_strjoin("CD", "EF\nGH") = "CDEF\nGH"
      ft_strchr("CDEF\nGH", '\n') → 非NULL → ループ終了

    free(buf)
    return "CDEF\nGH"

  ft_get_line("CDEF\nGH"):
    i: 0→'C', 1→'D', 2→'E', 3→'F', 4→'\n' → i=5
    malloc(6) → "CDEF\n\0"

  ft_trim_stash("CDEF\nGH"):
    i: 0→4('\n') → ft_strlen=7, 7-4=3 → malloc(3)
    → trimmed = "GH\0"
    free("CDEF\nGH")

  stash = "GH"
  return "CDEF\n"


=== get_next_line(fd) 呼び出し3 ===

  stash = "GH"

  ft_read_to_stash:
    ft_strchr("GH", '\n') → NULL → ループ開始
    read → 1バイト "H" ← 残り1バイト（ファイル末尾付近）

    ★ 注意: 実際にはread()は残りの1バイトを読む
      ファイル全体11バイト中、既に10バイト読了。残り1バイト。
      → read → "H" → 待って、ここは注意が必要。

    実際のファイルオフセット追跡:
      呼び出し1: read 5バイト → オフセット 0→5
      呼び出し2: read 5バイト → オフセット 5→10
      呼び出し3: read → 残り1バイト "H" → オフセット 10→11

      ★ 訂正: ファイル="AB\nCDEF\nGH" は11バイト
        read1: "AB\nCD" → offset 5
        read2: "EF\nGH" → offset 10
        read3: 残り1バイト → "H"?

      実際は offset=10 で残り1バイト 'H' のみ。
      ただし先にGを読んでいるか確認...

      ファイル: A B \n C D E F \n G  H
      index:    0 1 2  3 4 5 6 7  8  9
      → ファイルサイズ = 10バイト (0-9)

      read1: offset 0→5, buf = "AB\nCD"
      read2: offset 5→10, buf = "EF\nGH"
      read3: offset 10 = EOF → read → 0バイト

    修正版:
    ループ1回目:
      read → 0バイト (EOF)
      bytes_read = 0 → ループ終了

    free(buf)
    return "GH"

  stash = "GH", stash[0] = 'G' != '\0'

  ft_get_line("GH"):
    '\n'なし → line = "GH\0"

  ft_trim_stash("GH"):
    '\n'なし → free("GH") → return NULL

  stash = NULL
  return "GH"


=== get_next_line(fd) 呼び出し4 ===

  stash = NULL → init → ""
  read → 0 (EOF)
  stash = ""
  stash[0] == '\0' → free, NULL → return NULL
```

---

## 7. 完全実行トレース: BUFFER_SIZE=1024

ファイル内容: `"AB\nCDEF\nGH"` (10バイト)

```
=== get_next_line(fd) 呼び出し1 ===

  stash = NULL → init → ""

  ft_read_to_stash:
    buf = malloc(1025)

    ループ1回目:
      read(fd, buf, 1024) → 10バイト "AB\nCDEF\nGH"
      (1024バイト要求したが10バイトしかない → 10バイト返す)
      stash = ft_strjoin("", "AB\nCDEF\nGH") = "AB\nCDEF\nGH"
      ft_strchr に \n あり → ループ終了

    free(buf)
    return "AB\nCDEF\nGH"

  ft_get_line("AB\nCDEF\nGH") → "AB\n"
  ft_trim_stash("AB\nCDEF\nGH") → "CDEF\nGH"

  stash = "CDEF\nGH"
  return "AB\n"


=== get_next_line(fd) 呼び出し2 ===

  stash = "CDEF\nGH"

  ft_read_to_stash:
    ft_strchr("CDEF\nGH", '\n') → 非NULL → ループに入らない!
    ★ stashに既に\nがあるので、read()を1回も呼ばない!
    free(buf)
    return "CDEF\nGH"

  ft_get_line("CDEF\nGH") → "CDEF\n"
  ft_trim_stash("CDEF\nGH") → "GH"

  stash = "GH"
  return "CDEF\n"


=== get_next_line(fd) 呼び出し3 ===

  stash = "GH"

  ft_read_to_stash:
    ft_strchr("GH", '\n') → NULL → ループ開始
    read → 0バイト (EOF) → ループ終了(bytes_read=0)
    free(buf)
    return "GH"

  ft_get_line("GH") → "GH"
  ft_trim_stash("GH") → \nなし → free → NULL

  return "GH"
```

**BUFFER_SIZE=1024のポイント**:
- 1回のreadでファイル全体を読む
- 2回目の呼び出しではread()が一度も呼ばれない（stashに\nがあるため）
- 「可能な限り少なく読む」要件を満たしている

---

## 8. ボーナス版: 複数fd対応

### 8.1 変更点

唯一の変更: `static char *stash` → `static char *stash[MAX_FD]`

```c
/* 必須版 */
char	*get_next_line(int fd)
{
	static char	*stash;         // 1つのstash
	/* stash を使用 */
}

/* ボーナス版 */
char	*get_next_line(int fd)
{
	static char	*stash[MAX_FD]; // fdごとにstash
	/* stash[fd] を使用 */
}
```

### 8.2 ボーナス版の追加チェック

```c
if (fd < 0 || fd >= MAX_FD || BUFFER_SIZE <= 0)
    return (NULL);
// fd >= MAX_FD のチェックが追加されている
// → 配列の範囲外アクセス防止
```

### 8.3 ボーナス版のメモリイメージ

```
static char *stash[MAX_FD] のメモリレイアウト:

BSSセグメント (8192バイト = 1024 * 8):
┌──────┬──────────────────┐
│ [0]  │ NULL              │  stdin (未使用)
│ [1]  │ NULL              │  stdout (未使用)
│ [2]  │ NULL              │  stderr (未使用)
│ [3]  │ 0x55ab00001000 ──┼──→ ヒープ: "残りデータA..."
│ [4]  │ 0x55ab00002000 ──┼──→ ヒープ: "残りデータB..."
│ [5]  │ NULL              │  (EOF到達済み or 未使用)
│ ...  │ ...               │
│[1023]│ NULL              │
└──────┴──────────────────┘

get_next_line(3) → stash[3] を使用 (ファイルAの残りデータ)
get_next_line(4) → stash[4] を使用 (ファイルBの残りデータ)
                   stash[3] は一切影響を受けない!
get_next_line(3) → stash[3] の続きから処理
```

---

## 9. 変数の値の変遷テーブル

### BUFFER_SIZE=5, ファイル="Hi\nBye\n"

| ステップ | 操作 | stash | buf | line | bytes_read |
|---------|------|-------|-----|------|-----------|
| 1 | init_stash | `""` | - | - | - |
| 2 | malloc buf | `""` | `(未初期化)` | - | 1 |
| 3 | read | `""` | `"Hi\nBy"` | - | 5 |
| 4 | strjoin | `"Hi\nBy"` | `"Hi\nBy"` | - | 5 |
| 5 | \nあり→ループ終了 | `"Hi\nBy"` | freed | - | 5 |
| 6 | get_line | `"Hi\nBy"` | - | `"Hi\n"` | - |
| 7 | trim_stash | `"By"` | - | `"Hi\n"` | - |
| 8 | return | `"By"` | - | → 返却 | - |
| 9 | (呼出2) init | `"By"` | - | - | - |
| 10 | read | `"By"` | `"e\n"` | - | 2 |
| 11 | strjoin | `"Bye\n"` | `"e\n"` | - | 2 |
| 12 | \nあり→終了 | `"Bye\n"` | freed | - | 2 |
| 13 | get_line | `"Bye\n"` | - | `"Bye\n"` | - |
| 14 | trim_stash | `NULL` | - | `"Bye\n"` | - |
| 15 | return | `NULL` | - | → 返却 | - |
| 16 | (呼出3) init | `""` | - | - | - |
| 17 | read | `""` | `""` | - | 0 |
| 18 | stash[0]=='\0' | freed,NULL | - | - | - |
| 19 | return NULL | `NULL` | - | NULL | - |
