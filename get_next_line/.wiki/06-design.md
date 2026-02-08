# 06 - 設計原則とトレードオフ

この章では、get_next_lineの設計判断の理由、
代替案との比較、パフォーマンス分析を詳しく解説します。
「なぜこう作ったのか」を説明できることは、
評価での防衛質問に答えるために不可欠です。

設計とは「複数の選択肢の中から、特定の基準に基づいて
最適なものを選ぶこと」です。この章では、各設計判断について
「他にどんな選択肢があったか」「なぜこの方法を選んだか」
「選ばなかった方法の利点と欠点は何か」を明確にします。

---

## 1. 単一責任の原則（Single Responsibility Principle）

### 1.1 SRPとは何か

単一責任の原則（SRP）は、ソフトウェア設計の最も基本的な原則の一つです。
「各関数（またはモジュール）は、たった1つの責務（仕事）のみを持つべき」
という考え方です。

なぜSRPが重要なのか:

1. **デバッグが容易になる**: バグの原因が特定の関数に局所化される
2. **テストが容易になる**: 各関数を独立してテストできる
3. **理解が容易になる**: 各関数の名前から何をするか推測できる
4. **変更が容易になる**: ある機能を変更しても他に影響しない
5. **再利用が容易になる**: 汎用的な関数は他のプロジェクトでも使える

42 Normの「1関数最大25行」という制約は、事実上SRPを強制しています。
25行で1つの責務を超えることは非常に困難だからです。

### 1.2 関数の責務マップ

各関数は明確に1つの責務のみを持つように設計されています。

```
┌──────────────────────────────────────────────────────────────┐
│                   get_next_line (指揮者)                       │
│  責務: 全体のオーケストレーション、エラーチェック、状態管理       │
│  static stash の唯一の「所有者」                               │
└─────────┬─────────────────┬──────────────────────────────────┘
          │                 │
   ┌──────v──────┐   ┌─────v──────────┐
   │ ft_init_    │   │ ft_read_to_    │
   │ stash       │   │ stash          │
   │ stash初期化  │   │ read+stash蓄積 │
   └─────────────┘   └─────┬──────────┘
                           │
                    ┌──────┴──────────────────┐
                    │                         │
             ┌──────v──────┐          ┌──────v────────┐
             │ ft_get_line │          │ ft_trim_stash │
             │ 行の切り出し │          │ 残余データ管理 │
             └─────────────┘          └───────────────┘

  [ユーティリティ層]
   ft_strlen    ft_strchr    ft_strjoin
   文字列長     文字検索     文字列結合+解放
```

### 1.3 オーケストラの指揮者モデル

get_next_lineの設計は、オーケストラに例えるとわかりやすくなります。

```
get_next_line() = 指揮者
  → 自分では楽器を演奏しない（低レベルの処理をしない）
  → 各パートに「いつ、何を演奏するか」を指示する
  → 全体の流れを制御する
  → エラー時には演奏を停止する

ft_init_stash() = チューニング担当
  → 演奏前の準備を行う
  → 楽器（stash）が使える状態かチェックする

ft_read_to_stash() = 第一バイオリン
  → メインの仕事（データ読み込み）を担当
  → 最も複雑で重要なパート
  → 内部でft_strjoinを使う

ft_get_line() = ソロパート
  → stashから必要な部分だけを抽出する
  → 独立して動作可能

ft_trim_stash() = 舞台係
  → 使い終わった楽譜（データ）を片付ける
  → 次の演奏に備える

ft_strlen, ft_strchr, ft_strjoin = 共通の技法
  → どのパートからも使われる基本技術
```

### 1.4 各関数の責務の厳密な定義

| 関数 | 入力 | 出力 | 責務 | freeする対象 |
|------|------|------|------|-------------|
| `get_next_line` | fd | line or NULL | 全体制御・状態管理 | stash(EOF時) |
| `ft_init_stash` | stash(NULL可) | 初期化済みstash | NULL->"" の変換 | なし |
| `ft_read_to_stash` | fd, stash | 更新されたstash | fd読み込み+蓄積 | buf, stash(エラー時) |
| `ft_get_line` | stash | line | stashから1行を抽出 | なし |
| `ft_trim_stash` | stash | trimmedまたはNULL | 使用済み部分を除去 | stash(古い) |
| `ft_strlen` | string | length | 文字列長の計算 | なし |
| `ft_strchr` | string, char | pointerまたはNULL | 文字の検索 | なし |
| `ft_strjoin` | s1, s2 | joined | 文字列の結合 | s1(古い) |

### 1.5 なぜ関心を分離するのか

```
悪い例: 全てを1つの関数で行う

char *get_next_line(int fd)
{
    // 60行以上の巨大関数
    // バリデーション + 初期化 + 読み込み +
    // 蓄積 + 行切り出し + 残余管理 + エラー処理
    // -> Normの25行制限を超える
    // -> デバッグ困難
    // -> テスト困難
    // -> バグが発生しやすい
}

良い例: 責務ごとに関数を分割

get_next_line -> バリデーション + 全体制御（10行程度）
ft_init_stash -> 初期化のみ（5行程度）
ft_read_to_stash -> 読み込み+蓄積（15行程度）
ft_get_line -> 行の切り出し（15行程度）
ft_trim_stash -> 残余管理（15行程度）

-> 各関数が25行以下（Norm準拠）
-> 各関数を個別にテスト可能
-> バグの局所化が容易
```

### 1.6 責務の分離の実例: バグ修正の容易さ

あるバグが報告されたとします: 「最終行が改行で終わらない場合に
空文字列が返される」

```
巨大関数の場合:
  → 60行以上の関数を全て読んで、行切り出しと
    残余管理の部分を特定する必要がある
  → 修正が他の処理に影響する可能性がある

分割設計の場合:
  → ft_get_line か ft_trim_stash のどちらかが原因と即座にわかる
  → ft_get_line のみを10行程度読めば原因特定できる
  → 修正が他の関数に影響しないことが保証される
```

---

## 2. メモリ安全性パターン

### 2.1 パターン1: NULLチェックの徹底

全てのmalloc呼び出し後に、戻り値がNULLでないか確認します。

```c
buf = malloc(sizeof(char) * (BUFFER_SIZE + 1));
if (!buf)
    return (NULL);  // 確保失敗時は即座にNULLを返す
```

**なぜ必要か**: mallocはメモリ不足時にNULLを返します。
NULLポインタをデリファレンスするとセグメンテーションフォルトが発生します。

**実際のシナリオ**: malloc失敗は実務では稀ですが、以下の場面で起こります:

```
1. システムの物理メモリ + スワップが枯渇している
2. プロセスのメモリ制限（ulimit -v）に達した
3. BUFFER_SIZE=10000000000 (10GB) のような極端な値が指定された
4. メモリフラグメンテーションが激しい場合
```

### 2.2 パターン2: free後のNULL代入

```c
free(stash);
stash = NULL;     // ダングリングポインタを防止
return (NULL);
```

**なぜ必要か**: free後もポインタは古いアドレスを保持しています。
NULLに設定することで、万が一再度アクセスした場合に
セグフォルトが発生し、バグを検出しやすくなります。

```
free前:
  stash ──→ [解放されたメモリ] (アクセスすると未定義動作)
             使える「かもしれない」し、クラッシュする「かもしれない」
             → 最悪のパターン: たまたま動いてしまい、
               別の環境やタイミングでクラッシュする

NULL代入後:
  stash ──→ NULL (アクセスすると確実にセグフォルト)
             → 常に再現可能なバグとなり、検出が容易

防衛的プログラミングの原則:
  「沈黙のバグより、騒がしいバグの方が良い」
  サイレントなメモリ破壊より、即座のクラッシュの方が
  デバッグしやすく、修正も容易。
```

### 2.3 パターン3: 所有権の移転（Ownership Transfer）

get_next_lineの設計で最も重要な概念の一つが「所有権の移転」です。

**所有権とは**: あるメモリ領域をfreeする責任を持つこと。

```
ft_strjoin における所有権の移転:

  呼び出し前:
    stash が "Hello" の所有権を持つ
    buf が "World" の所有権を持つ（bufは呼び出し元が管理）

  ft_strjoin(stash, buf) 実行:
    joined = malloc(...) -> "HelloWorld" の新しい所有権が生まれる
    free(s1)  -> "Hello" の所有権が消滅
    return joined

  呼び出し後:
    stash = joined -> stash が "HelloWorld" の所有権を持つ
    "Hello" は解放済み -> 所有権なし

┌──────────────────────────────────────────────────┐
│ 所有権の流れ:                                      │
│                                                    │
│  stash -> "Hello" ─── ft_strjoin に所有権を渡す     │
│                       ↓                            │
│                  free("Hello") <- 所有権消滅         │
│                       ↓                            │
│                  return "HelloWorld" <- 新しい所有権  │
│                       ↓                            │
│  stash = "HelloWorld" <- stash が新しい所有権を取得   │
└──────────────────────────────────────────────────┘
```

**所有権の概念が重要な理由**: この概念は後続のプロジェクト
（特にminishell）で頻繁に登場します。コマンドの引数リスト、
環境変数のコピー、リダイレクトのファイル名など、
「誰がfreeする責任を持つか」を常に明確にする必要があります。

### 2.4 パターン4: エラー時の全リソース解放

```c
if (bytes_read == -1)
{
    free(buf);    // 1. 読み取りバッファを解放
    free(stash);  // 2. 蓄積バッファも解放
    return (NULL); // 3. NULLを返す -> static stash もNULLに
}
```

**なぜ両方freeするのか**: エラーが発生した場合、
この関数から正常に戻ることはできません。
確保した全てのリソースを解放してからNULLを返す必要があります。

**エラー時のリソース解放の原則**:

```
原則1: エラーが発生した時点で、その関数が確保した
       全てのリソースを解放する

原則2: 呼び出し元から受け取ったリソースについては、
       「エラー時にどうするか」を関数の契約で明確にする

ft_read_to_stashの場合:
  → buf は自分で確保した → 自分でfreeする（義務）
  → stash は呼び出し元から受け取った
    → 通常: そのまま返す
    → エラー時: freeしてNULLを返す（契約の一部）
    → 呼び出し元(get_next_line)は stash = NULL を受け取る
```

### 2.5 メモリ所有権の完全な図

```
┌──────────────────────────────────────────────────────────────┐
│ get_next_line() のメモリ所有権マップ                            │
│                                                              │
│ buf:                                                         │
│   確保: ft_read_to_stash [malloc(BUFFER_SIZE + 1)]           │
│   解放: ft_read_to_stash [free(buf)] <- 同一関数内で完結       │
│   エラー時: ft_read_to_stash [free(buf)]                      │
│                                                              │
│ stash:                                                       │
│   初回確保: ft_init_stash [malloc(1)]                         │
│   更新: ft_strjoin [旧stashをfree + 新joinedを返す]            │
│   縮小: ft_trim_stash [旧stashをfree + 新trimmedを返す]        │
│   EOF解放: get_next_line [free(stash); stash=NULL]            │
│   エラー解放: ft_read_to_stash [free(stash)]                   │
│                                                              │
│ line:                                                        │
│   確保: ft_get_line [malloc(line_len + 1)]                    │
│   解放: 呼び出し元の責任!                                      │
│          get_next_line() はlineの所有権を呼び出し元に移転する   │
│                                                              │
│ joined (ft_strjoin内部):                                      │
│   確保: ft_strjoin [malloc(len1 + len2 + 1)]                 │
│   → 戻り値として返され、stash に代入される                      │
│   → 所有権は ft_strjoin -> ft_read_to_stash -> stash に移転   │
│                                                              │
│ trimmed (ft_trim_stash内部):                                  │
│   確保: ft_trim_stash [malloc(remaining_len + 1)]            │
│   → 戻り値として返され、stash に代入される                      │
│   → 所有権は ft_trim_stash -> get_next_line -> stash に移転   │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 2.6 所有権の移転チェーン: 完全な追跡

1回の get_next_line() 呼び出しにおける所有権の変遷:

```
時刻 T0: get_next_line() 開始
  所有者: stash -> NULL（初回）or 前回のtrimmed（2回目以降）

時刻 T1: ft_init_stash() 実行
  所有者: stash -> malloc(1)で確保した""
  新規所有権: stashが""を所有

時刻 T2: ft_read_to_stash() 内 - buf 確保
  所有者: buf -> malloc(BS+1)で確保したバッファ
  新規所有権: buf が確保したバッファを所有

時刻 T3: ft_strjoin(stash, buf) 実行
  1. joined = malloc(...)  <- 新しい所有権が生まれる
  2. free(s1)             <- 古いstashの所有権が消滅
  3. return joined        <- joined の所有権が呼び出し元に移転
  結果: stash = joined    <- stashが新しい結合文字列を所有

時刻 T4: free(buf) 実行
  bufの所有権が消滅

時刻 T5: ft_get_line(stash) 実行
  1. line = malloc(...)   <- 新しい所有権が生まれる
  2. stash は変更なし     <- stash は依然として所有権を持つ
  結果: line が新しい行文字列を所有

時刻 T6: ft_trim_stash(stash) 実行
  1. trimmed = malloc(...) <- 新しい所有権が生まれる
  2. free(stash)           <- 古いstashの所有権が消滅
  3. return trimmed        <- trimmedの所有権が呼び出し元に移転
  結果: stash = trimmed    <- stashが残余データを所有

時刻 T7: return line
  lineの所有権が呼び出し元に移転
  呼び出し元が free(line) する義務を負う
```

---

## 3. エラーハンドリング戦略

### 3.1 エラーの種類と対応の全体図

```
┌─────────────────────────────────────────────────────────────┐
│ エラー種別          │ 発生箇所        │ 対応                  │
├─────────────────────┼────────────────┼──────────────────────┤
│ 不正な引数          │ get_next_line   │ 即座にNULL           │
│ (fd<0,BS<=0)        │ 冒頭           │ リソース確保前なので  │
│                     │                │ freeは不要           │
├─────────────────────┼────────────────┼──────────────────────┤
│ malloc失敗          │ ft_init_stash  │ NULLを返す           │
│ (stash初期化)       │                │ リソースなし          │
├─────────────────────┼────────────────┼──────────────────────┤
│ malloc失敗          │ ft_read_to_    │ NULLを返す           │
│ (buf確保)           │ stash冒頭      │ stashは呼び出し元管理│
├─────────────────────┼────────────────┼──────────────────────┤
│ read()エラー        │ ft_read_to_    │ buf+stashをfree     │
│ (戻り値-1)          │ stashループ内  │ static stash->NULL    │
├─────────────────────┼────────────────┼──────────────────────┤
│ malloc失敗          │ ft_strjoin     │ s1(stash)をfree     │
│ (joined確保)        │                │ NULLを返す           │
├─────────────────────┼────────────────┼──────────────────────┤
│ EOF                 │ get_next_line  │ stashをfree          │
│ (stash[0]=='\0')    │ read_to_stash後│ stash=NULL           │
│                     │                │ NULLを返す           │
├─────────────────────┼────────────────┼──────────────────────┤
│ malloc失敗          │ ft_get_line    │ NULLを返す           │
│ (line確保)          │                │ stashは維持           │
├─────────────────────┼────────────────┼──────────────────────┤
│ malloc失敗          │ ft_trim_stash  │ stashをfree          │
│ (trimmed確保)       │                │ NULLを返す           │
└─────────────────────┴────────────────┴──────────────────────┘
```

### 3.2 エラー伝播の流れ

```
ft_strjoin でmalloc失敗:
  → NULLを返す
  → ft_read_to_stash: stash = NULL
  → ft_read_to_stashのwhile条件: ft_strchr(NULL, '\n')
    → ft_strchrはNULLセーフなのでNULLを返す
    → !NULL = 真 → ループ続行
    → 次のread()で再びft_strjoinを呼ぶ...
    → 注意: stashがNULLの場合の処理に要注意!

実装では ft_strjoin が s1=NULL の場合 NULL を返し、
ft_read_to_stash のループが継続するが stash が NULL のまま
問題が発生する可能性がある。
→ ft_strjoin の NULLチェック (if (!s1 || !s2) return NULL)
  と、ft_read_to_stash での stash の NULL チェックが重要。
```

### 3.3 エラー伝播の連鎖: 詳細分析

以下にエラーが発生した場合の伝播パスを全て示します。

```
パス1: fd < 0
  get_next_line() → return NULL
  メモリ操作: なし（何も確保していない）
  stash の状態: 変化なし

パス2: ft_init_stash で malloc 失敗
  ft_init_stash() → return NULL
  get_next_line() → stash = NULL → if (!stash) → return NULL
  メモリ操作: なし
  stash の状態: NULL のまま

パス3: ft_read_to_stash で buf の malloc 失敗
  ft_read_to_stash() → return NULL
  get_next_line() → stash = NULL → if (!stash) → return NULL
  メモリ操作: なし（buf は確保されていない）
  stash の状態: NULL（元のstashは失われる!）
  注意: stash は引数として渡されたが、
        ft_read_to_stash は NULL を返すだけ。
        get_next_line 側で stash = NULL となり、
        元の stash がリークする可能性がある。
        → 実装上の注意点

パス4: read() が -1 を返す
  ft_read_to_stash():
    free(buf) → buf 解放
    free(stash) → stash 解放
    return NULL
  get_next_line() → stash = NULL → if (!stash) → return NULL
  メモリ操作: buf と stash の両方が解放済み
  stash の状態: NULL（正しくクリーンアップされている）

パス5: EOF（read() が 0 を返し、stash が空）
  ft_read_to_stash() → return stash = ""
  get_next_line() → stash[0] == '\0'
    → free(stash) → stash = NULL → return NULL
  メモリ操作: stash が正しく解放される
  stash の状態: NULL（次回呼び出しに備えてクリーン）
```

### 3.4 防衛的プログラミング（Defensive Programming）

get_next_lineでは「防衛的プログラミング」の手法を多用しています。

```
防衛的プログラミングの原則:
  1. 入力を信用しない → 全ての引数をバリデーション
  2. 戻り値を信用しない → malloc、read の戻り値を全てチェック
  3. 状態を信用しない → stash が NULL の可能性を常に考慮
  4. 失敗時には安全な状態にする → free + NULL 代入

具体例:
  ft_strlen(NULL)  → 0 を返す（クラッシュしない）
  ft_strchr(NULL, c) → NULL を返す（クラッシュしない）
  ft_strjoin(NULL, s2) → NULL を返す（クラッシュしない）
  ft_get_line(NULL) → NULL を返す（クラッシュしない）
  ft_trim_stash(NULL) → NULL を返す（クラッシュしない）

→ 全てのユーティリティ関数が NULL セーフ!
→ NULLが伝播しても、クラッシュではなく NULL が返る
```

---

## 4. ファイル分割の設計

### 4.1 なぜ .c と _utils.c に分けるのか

**主な理由: 42 Normの制約**

Normでは1ファイルあたり最大5関数までと定められています。
get_next_lineは計8関数（static 2 + public 6）が必要です。

```
必要な関数: 8個
Norm上限: 5個/ファイル
→ 最低2ファイルが必要

分割方針:
  get_next_line.c (3関数): 状態管理に関わる関数
    get_next_line()     <- static stash にアクセス
    ft_init_stash()     <- static (ファイル内のみ公開)
    ft_read_to_stash()  <- static (ファイル内のみ公開)

  get_next_line_utils.c (5関数): 純粋なデータ操作
    ft_strlen()         <- 状態を持たない
    ft_strchr()         <- 状態を持たない
    ft_strjoin()        <- 状態を持たない
    ft_get_line()       <- 状態を持たない
    ft_trim_stash()     <- 状態を持たない
```

### 4.2 分割の基準

| 基準 | get_next_line.c | get_next_line_utils.c |
|------|----------------|---------------------|
| static stashへのアクセス | あり | なし |
| static関数 | あり（init, read_to_stash） | なし |
| 再利用可能性 | 低い（プロジェクト固有） | 高い（汎用的な文字列操作） |
| テスト容易性 | 低い（状態に依存） | 高い（入出力が明確） |
| 変更頻度 | 低い（ロジックが安定） | やや高い（最適化の余地） |

### 4.3 この分割の利点

1. **関心の分離**: 状態管理とデータ操作が明確に分離
2. **テスタビリティ**: utils関数を個別にテスト可能
3. **再利用性**: ft_strlen, ft_strchr等は他のプロジェクトでも使える
4. **可読性**: 各ファイルの役割が明確
5. **保守性**: バグ修正時に影響範囲が限定される

### 4.4 代替の分割方法

```
代替案1: 全関数を1ファイルに入れる
  → Norm 違反（5関数制限を超える）
  → 不可

代替案2: 3ファイルに分ける
  get_next_line.c      → get_next_line(), ft_init_stash(), ft_read_to_stash()
  get_next_line_str.c  → ft_strlen(), ft_strchr(), ft_strjoin()
  get_next_line_line.c → ft_get_line(), ft_trim_stash()
  → 可能だが、3ファイル提出はプロジェクト要件で制限
  → 2ファイル（.c + _utils.c）が要件

代替案3: 異なる分け方
  get_next_line.c      → get_next_line() のみ
  get_next_line_utils.c → 残り7関数
  → Norm 違反（7関数 > 5関数制限）
  → 不可
```

### 4.5 staticキーワードの戦略的使用

```c
// get_next_line.c 内:
static char *ft_init_stash(char *stash);    // static!
static char *ft_read_to_stash(int fd, char *stash);  // static!
char *get_next_line(int fd);                // 外部公開

// get_next_line_utils.c 内:
// 全て外部公開（ヘッダでプロトタイプ宣言）
```

```
static 関数のスコープ:
  ft_init_stash, ft_read_to_stash は get_next_line.c 内でのみ使える
  → 外部から直接呼ぶことを禁止
  → カプセル化（実装の詳細を隠す）
  → ヘッダファイルにプロトタイプを書く必要がない

非static 関数（utils）:
  ft_strlen 等は ヘッダ で宣言し、どこからでも呼べる
  → get_next_line.c からも呼べる
  → テスト用 main.c からも直接テストできる
```

---

## 5. トレードオフ分析

### 5.1 メモリコピー vs 実装の複雑さ

**現在の実装**: ft_strjoinで毎回新しい文字列を確保・コピー

```
計算量分析（BUFFER_SIZE=1で n文字の行を読む場合）:

ft_strjoin呼び出し回数: n回
各呼び出しでのコピー量:
  1回目: 0 + 1 = 1文字
  2回目: 1 + 1 = 2文字
  3回目: 2 + 1 = 3文字
  ...
  n回目: (n-1) + 1 = n文字

総コピー量: 1 + 2 + 3 + ... + n = n(n+1)/2 = O(n^2)

例: 1000文字の行、BUFFER_SIZE=1
  コピー量: 1000 * 1001 / 2 = 500,500文字!
```

**代替案: リンクリストで断片を管理**

```
リンクリスト方式:
  read -> buf1 = "A" -> リストに追加
  read -> buf2 = "B" -> リストに追加
  ...
  read -> bufn = "\n" -> リストに追加
  → リスト全体を1回の走査でlineに結合

  コピー量: n文字（1回だけ）= O(n)

  ただし:
  → リンクリストのノード管理が必要
  → mallocの回数は同じ（各ノードに1回）
  → 実装が複雑でNormの25行制限に収まりにくい
  → ポインタ操作のバグが起きやすい
```

**代替案: 動的配列（realloc相当）方式**

```
動的配列方式:
  初期: stash = malloc(128)  capacity = 128, len = 0
  read -> stash に追記
  容量不足時 → 容量を2倍に拡張（realloc相当）

  コピー量: amortized O(n)
    → 拡張回数: log2(n) 回
    → 各拡張でのコピー: 128, 256, 512, ...
    → 合計コピー量: 128 + 256 + ... ≈ 2n = O(n)

  ただし:
  → Cの realloc() は使用可能だが、
    自前実装する場合は malloc + copy + free
  → 実装が複雑になる
  → Normの25行制限への適合が困難
```

**比較表**:

| 方式 | 時間計算量(最悪) | メモリ使用量 | 実装複雑度 | Norm適合性 |
|------|---------------|------------|----------|----------|
| stash蓄積型 | O(n^2) | O(n) | 低 | 高 |
| リンクリスト型 | O(n) | O(n) + ノードオーバーヘッド | 高 | 低 |
| 動的配列型 | amortized O(n) | O(n) + 余剰容量 | 中 | 中 |

**結論**: stash蓄積型を採用。理由:
1. BUFFER_SIZE=1は評価時のテストケースだが、実用的にはBUFFER_SIZE>=42を使う
2. 実用的なBUFFER_SIZEではstrjoinの呼び出し回数が少なく、O(n^2)は問題にならない
3. Normの25行制限内で確実に実装可能
4. バグのリスクが低い

### 5.2 BUFFER_SIZEの影響の定量分析

```
n = ファイルサイズ（バイト）
L = 平均行長（バイト）
B = BUFFER_SIZE

read()呼び出し回数 ≈ ceil(n / B) + 1
ft_strjoin呼び出し回数 ≈ ceil(n / B)
1行あたりのstrjoin回数 ≈ ceil(L / B)

例: n=10000, L=100

  B=1:    read 10001回, strjoin/行 100回
  B=10:   read 1001回,  strjoin/行 10回
  B=100:  read 101回,   strjoin/行 1回
  B=1024: read 11回,    strjoin/行 0-1回
  B=10000: read 2回,    strjoin/行 0-1回

  B >= L の場合:
    1行の読み込みに必要なstrjoinは高々2回
    → 実質O(n)の計算量
```

### 5.3 BUFFER_SIZEとシステムコールのコスト

```
各BUFFER_SIZEでの処理コスト概算:

B=1 (1バイトずつ):
  10000バイトのファイル:
    read()  10001回 x 約1.0μs = 約 10.0ms
    strjoin 10000回 x 約0.1μs = 約 1.0ms (平均行長50)
    合計: 約 11ms

B=42 (42バイトずつ):
  10000バイトのファイル:
    read()  239回 x 約1.0μs = 約 0.24ms
    strjoin 239回 x 約0.05μs = 約 0.01ms
    合計: 約 0.25ms

B=4096 (4KBずつ, ページサイズ):
  10000バイトのファイル:
    read()  3回 x 約1.0μs = 約 0.003ms
    strjoin 3回 x 約0.05μs = 約 0.00015ms
    合計: 約 0.003ms

B=1とB=4096の速度差: 約 3700倍!
→ BUFFER_SIZEはパフォーマンスに劇的な影響を与える
```

### 5.4 メモリ使用量の分析

```
get_next_line が使用するメモリ:

1. buf: BUFFER_SIZE + 1 バイト（一時的）
2. stash: 最大で max(BUFFER_SIZE, 最長行の長さ) バイト
3. line: 現在の行の長さ + 1 バイト（返却後は呼び出し元の管理）
4. trimmed: stashの残りの長さ + 1 バイト（一時的）

ピーク使用量 ≈ BUFFER_SIZE + stash + line + trimmed
            ≈ BUFFER_SIZE + 2 * 最長行の長さ + 残りデータ

B=1 の場合:
  buf: 2バイト
  stash: 最長行の長さ (例: 1000バイト)
  ピーク: 約 2000バイト

B=4096 の場合:
  buf: 4097バイト
  stash: 最大で max(4096, 最長行の長さ)
  ピーク: 約 4097 + 行の長さ x 2 バイト

B=10000000 の場合:
  buf: 10000001バイト (約 10MB!)
  stash: 最長行の長さ
  ピーク: 約 10MB + 行の長さ x 2
```

### 5.5 static変数 vs 他の方法

| 方法 | 利点 | 欠点 |
|------|------|------|
| static変数（採用） | シグネチャがシンプル、プロジェクト要件準拠 | スレッドセーフでない、複数fd困難（必須版） |
| パラメータで状態を渡す | 明示的、テスト容易 | プロトタイプが変わる（要件違反） |
| グローバル変数 | アクセスが容易 | Norm違反、副作用が見えにくい |
| 構造体をmallocして返す | 状態が明示的 | プロトタイプが変わる |

### 5.6 スレッドセーフティの問題

get_next_lineがスレッドセーフでない理由を詳細に解説します。

```
スレッド1とスレッド2が同時にget_next_lineを呼んだ場合:

タイミング1:
  スレッド1: stash = ft_read_to_stash(3, stash)
  → stash = "Hello\nWorld"

タイミング2:
  スレッド2: stash = ft_read_to_stash(4, stash)
  → stash が上書きされる! "Hello\nWorld" が消失!
  → stash = "Foo\nBar"（スレッド2のデータ）

タイミング3:
  スレッド1: line = ft_get_line(stash)
  → "Foo\n" を返してしまう!（スレッド2のデータ!）

原因: static 変数は全スレッドで共有されるため

解決策（参考 - このプロジェクトの範囲外）:
  1. mutex（排他制御）を使う
  2. スレッドローカルストレージ（__thread）を使う
  3. 状態を引数で渡す設計にする
```

---

## 6. ボーナス設計: 静的配列によるfd管理

### 6.1 なぜ配列なのか

```
要件: fdごとに独立したstashを管理する
→ fd をキーとして stash にアクセスしたい
→ fd は非負整数（0, 1, 2, 3, ...）
→ 配列のインデックスとして直接使える!

stash[fd] でO(1)アクセス
→ ハッシュテーブルやリンクリストより高速かつシンプル
```

### 6.2 MAX_FDの選択

```c
#ifndef MAX_FD
# define MAX_FD 1024
#endif
```

1024はLinuxのデフォルトの`OPEN_MAX`値です。

```bash
# 確認方法:
ulimit -n     # 出力: 1024 (typical)
getconf OPEN_MAX  # 出力: 1024 (typical)
```

### 6.3 静的配列のメモリ効率

```
static char *stash[1024]:

BSS セグメント使用量:
  1024 * sizeof(char*) = 1024 * 8 = 8192バイト (8KB)
  → 常に8KBを使用（fdの使用数に関わらず）

ヒープ使用量:
  使用中のfdに対してのみ:
  stash[3] = "残りデータ" → strlen("残りデータ") + 1 バイト
  stash[4] = NULL          → 0バイト

  → 実際に開いているfdの残りデータ分のみ
```

### 6.4 必須版からボーナス版への変更箇所

```
必須版 → ボーナス版の変更は最小限:

1. ヘッダファイル:
   + #ifndef MAX_FD
   + # define MAX_FD 1024
   + #endif

2. get_next_line() の変更:
   - static char *stash;
   + static char *stash[MAX_FD];

   - if (fd < 0 || BUFFER_SIZE <= 0)
   + if (fd < 0 || fd >= MAX_FD || BUFFER_SIZE <= 0)

   全ての stash → stash[fd] に置換:
   - stash = ft_init_stash(stash);
   + stash[fd] = ft_init_stash(stash[fd]);

   - stash = ft_read_to_stash(fd, stash);
   + stash[fd] = ft_read_to_stash(fd, stash[fd]);

   - if (stash[0] == '\0')
   + if (stash[fd][0] == '\0')

   以降も同様に stash → stash[fd]
```

### 6.5 代替設計: リンクリストによるfd管理

```
リンクリスト方式:
  struct s_fd_stash {
      int fd;
      char *stash;
      struct s_fd_stash *next;
  };
  static struct s_fd_stash *list;

  利点:
  ・使用中のfdの分だけメモリを使う
  ・MAX_FDの制限がない
  ・fdが疎（例: fd=3と fd=10000）でも効率的

  欠点:
  ・アクセスがO(n)（nは使用中のfd数）
  ・実装が複雑（ノード追加/検索/削除）
  ・Normの25行制限で困難
  ・バグのリスクが高い
  ・ノードごとに追加のメモリオーバーヘッド（fd + next ポインタ）
```

### 6.6 代替設計: ハッシュテーブルによるfd管理

```
ハッシュテーブル方式:
  #define TABLE_SIZE 256
  struct s_entry {
      int fd;
      char *stash;
      struct s_entry *next;  // チェイン法
  };
  static struct s_entry *table[TABLE_SIZE];

  ハッシュ関数: fd % TABLE_SIZE

  利点:
  ・平均O(1)アクセス
  ・fdの上限制限がない
  ・メモリ効率が良い（使用中のfdのみ）

  欠点:
  ・実装が非常に複雑
  ・Normの25行/5関数制限で実現不可能
  ・get_next_lineの範囲を大幅に超える複雑さ
```

---

## 7. 実際のCライブラリ(glibc)のgetline()との比較

### 7.1 getline()の仕組み

glibc の `getline()` は以下のような仕組みで動作します:

```c
ssize_t getline(char **lineptr, size_t *n, FILE *stream);

/* 特徴:
 * ・FILE* を使用（stdio のバッファリングを活用）
 * ・lineptr にバッファを渡し、必要に応じて realloc する
 * ・n にバッファサイズを渡す
 * ・行の長さに応じてバッファを自動拡張
 */
```

### 7.2 get_next_lineとの比較

| 項目 | get_next_line | glibc getline() |
|------|-------------|-----------------|
| I/O層 | read() (システムコール直接) | FILE* (stdio バッファリング) |
| バッファ管理 | 毎回malloc/free | realloc で拡張 |
| 状態保持 | static変数 | FILE*構造体内 |
| 計算量(最悪) | O(n^2) per line | O(n) per line (realloc) |
| スレッドセーフ | いいえ | 条件付きyes |
| 標準準拠 | 42独自 | POSIX.1-2008 |
| 学習目的 | バッファリングの理解 | 実用的な行読み取り |
| エラー処理 | NULL返却 | -1返却 + errno |

### 7.3 getline()が効率的な理由

```
getline() の内部実装（概念的）:

1. FILE* の内部バッファ（通常 8KB）を使用
   → read() の呼び出し回数を最小化
   → カーネルとのコンテキストスイッチを削減

2. realloc で行バッファを拡張
   → 前の内容をコピーする必要がない場合がある
     （realloc がインプレースで拡張できる場合）
   → 最悪でも O(n) のコピー

3. バッファの再利用
   → 2回目以降の呼び出しでは、前回のバッファを再利用
   → mallocの呼び出し回数が最小化される

get_next_line との違い:
  get_next_line: 毎回 malloc + copy + free の繰り返し
  getline():     realloc で必要な分だけ拡張、バッファ再利用
```

### 7.4 学びのポイント

get_next_lineを自分で実装することで理解できること:

```
1. なぜバッファリングが必要なのか
   → read() の1バイト呼び出しのコストを体感する
   → BUFFER_SIZE=1 vs BUFFER_SIZE=4096 の速度差

2. なぜ標準ライブラリが存在するのか
   → 自前実装の複雑さと、ライブラリの便利さの対比
   → 車輪の再発明を通じて、車輪の価値を知る

3. メモリ管理の難しさ
   → 所有権、リーク、ダングリングポインタを実体験
   → ガベージコレクション（GC）がない言語の厳しさ

4. 設計判断の重要性
   → シンプルさ vs パフォーマンスのトレードオフ
   → Norm制約が設計にどう影響するか
```

---

## 8. やってみよう: 設計の選択問題

### 8.1 問題1: リンクリストでfdごとのstashを管理する設計

配列方式と比較して、利点と欠点は何ですか?

```
考えるべきポイント:
  ・fdが1000個開かれている場合の配列のメモリ使用量は?
  ・fdが3つだけ開かれている場合のリンクリストのメモリ使用量は?
  ・アクセス速度の違いは?
  ・実装の複雑さの違いは?
  ・Norm制約下で実装可能か?
```

### 8.2 問題2: strjoinのO(n^2)問題の解決

BUFFER_SIZE=1のときのstrjoinの計算量がO(n^2)になる問題を
解決する方法を、Norm準拠で考えてください。

```
ヒント:
  ・stashを毎回全体コピーするのではなく、
    十分な大きさで再確保する方法は?
  ・realloc()に相当する処理を自前で実装できるか?
  ・ただし、Normの25行制限と5関数制限の中で実現可能か?
```

### 8.3 問題3: エラー処理の設計選択

ft_strjoinでmallocが失敗した場合、現在の実装ではs1(旧stash)を
freeしています。もしfreeしない設計にした場合、
どのような問題が発生しますか?

```
考えるべきポイント:
  ・ft_strjoinの呼び出し元でs1をfreeできるか?
  ・stash = ft_strjoin(stash, buf) のパターンで
    joinedがNULLの場合、stashの値はどうなるか?
  ・古いstashのアドレスは失われないか?
```

### 8.4 問題4: パフォーマンスの最適化

以下の3つの最適化案のうち、Norm準拠で実装可能なものはどれですか?

```
案A: stashをrealloc相当で拡張し、コピーコストを削減
案B: 複数回のreadをまとめて行い、システムコール回数を削減
案C: stashのfreeを遅延し、次回のstrjoinで再利用する
```

---

## 9. まとめ: 設計判断の根拠一覧

| 設計判断 | 根拠 |
|---------|------|
| stash蓄積パターンの採用 | シンプルさ、Norm適合性、バグリスクの低さ |
| ft_strjoinでs1をfree | 呼び出し元でのfree忘れ防止、所有権の明確化 |
| static変数の使用 | プロジェクト要件（プロトタイプ固定）、呼び出し間の状態保持 |
| ファイルの2分割 | Normの5関数/ファイル制限、関心の分離 |
| NULLセーフなユーティリティ | stashがNULLの場合のクラッシュ防止 |
| ft_init_stashでの空文字列初期化 | ft_strjoinにNULLを渡さないため |
| buf[bytes_read]でのヌル終端付与 | read()はヌル終端を付けないため |
| エラー時の全リソース解放 | メモリリーク防止 |
| ボーナスでの配列使用 | O(1)アクセス、シンプルな実装 |
| MAX_FD=1024の選択 | システムのOPEN_MAXに基づく、実用的な上限 |
| static関数の使用（init, read_to） | カプセル化、外部からのアクセス制限 |
| bytes_read=1での初期化 | whileループへの初回突入を保証 |

---

## 10. 設計原則の応用: 後続プロジェクトでの活用

### 10.1 pipexでの活用

```
pipexで使う get_next_line の設計パターン:
  ・fd管理: open/close の対応、dup2 の使用
  ・エラーハンドリング: fork/pipe/execve の戻り値チェック
  ・リソース管理: 子プロセスでの fd リーク防止
  ・所有権概念: パイプの読み書きfdの管理
```

### 10.2 minishellでの活用

```
minishellで使う get_next_line の設計パターン:
  ・static変数の活用: 環境変数の管理
  ・メモリ所有権: コマンド引数リストの所有権管理
  ・バッファ管理: 入力行のパーシング
  ・エラー伝播: コマンド実行エラーの連鎖処理
  ・ファイル分割: モジュール化された設計
```

### 10.3 cub3D/miniRTでの活用

```
3Dプロジェクトで使う get_next_line の設計パターン:
  ・ファイルI/O: マップファイル(.cub)やシーンファイル(.rt)の読み込み
  ・get_next_lineの直接使用: 設定ファイルの行ごとのパース
  ・メモリ管理: テクスチャデータ、ピクセルバッファの管理
  ・エラー処理: ファイルフォーマットの検証
```

### 10.4 設計パターンの抽象化

get_next_lineで学んだ設計パターンを一般化すると:

```
パターン1: 状態保持パターン（Static State Pattern）
  → 関数呼び出し間で状態を保持する必要がある場合
  → static変数、構造体、グローバル変数で実現

パターン2: 所有権移転パターン（Ownership Transfer Pattern）
  → 動的メモリの管理責任を明確にする
  → 「誰が確保し、誰が解放するか」のルールを定義

パターン3: 蓄積バッファパターン（Accumulation Buffer Pattern）
  → 小さなデータを繰り返し受け取り、条件を満たすまで蓄積
  → ネットワークプログラミングでも同じパターンを使用

パターン4: エラー安全パターン（Error Safety Pattern）
  → エラー発生時に全リソースを安全に解放
  → 部分的な失敗でも一貫した状態を維持

パターン5: 関数分割パターン（Function Decomposition Pattern）
  → 複雑な処理を小さな独立した関数に分割
  → 各関数が単一の責務を持つ
```
