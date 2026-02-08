# get_next_line Wiki

## 概要

このWikiは42プロジェクト「get_next_line」の包括的な学習リソースです。
ファイルディスクリプタから1行ずつ読み取る関数の実装を通じて、
static変数、ファイルI/O、バッファ管理、メモリ管理を深く学びます。

このプロジェクトは42カリキュラムにおける重要なマイルストーンであり、
C言語の低レベルプログラミングの基礎を固める土台となります。
ここで身につけるスキルは、pipex、minishell、cub3Dなど
後続のプロジェクト全てに直結します。

---

## このWikiの使い方

### 初めて取り組む人へ

get_next_lineプロジェクトに初めて取り組む場合は、以下の順序で読むことを
強く推奨します。

```
[STEP 1] 01-background.md    ─── まずここから。全ての基礎知識
              │
              v
[STEP 2] 02-prerequisites.md ─── 前提知識の確認と補強
              │
              v
[STEP 3] 03-requirements.md  ─── 何を実装すべきか正確に理解する
              │
              v
[STEP 4] 05-solution.md      ─── アルゴリズムとコードの詳解
              │
              v
[STEP 5] 06-design.md        ─── なぜこの設計なのか理解する
              │
              v
[STEP 6] 07-study-guide.md   ─── デバッグ技法と実践演習
              │
              v
[STEP 7] 04-evaluation.md    ─── 評価に備える
              │
              v
[STEP 8] 08-learning-goals.md ── 学んだことを振り返る
              │
              v
[STEP 9] 09-comprehension-check.md ── 理解度を最終確認する
```

### 評価前の復習として使う人へ

評価が近い場合は以下を重点的に確認してください。

1. **04-evaluation.md** - 評価基準と防衛質問
2. **09-comprehension-check.md** - 理解度確認60問以上
3. **05-solution.md** - コードの詳細な動作説明
4. **06-design.md** - 設計判断の根拠

### デバッグに困っている人へ

バグが見つからない場合は以下が役立ちます。

1. **07-study-guide.md** - デバッグ手法の詳細（gdb、valgrind、printfデバッグ）
2. **05-solution.md** の実行トレースセクション
3. **09-comprehension-check.md** のコードトレース問題

---

## 目次

| # | ファイル | 内容 | 想定読了時間 |
|---|---------|------|------------|
| 01 | [01-background.md](01-background.md) | 背景知識 - fd、read()、static変数、メモリ管理、Unix哲学 | 60-90分 |
| 02 | [02-prerequisites.md](02-prerequisites.md) | 前提知識 - C言語基礎、システムコール、デバッグ入門 | 45-60分 |
| 03 | [03-requirements.md](03-requirements.md) | 要件と仕様の詳細、エッジケース完全網羅 | 30-45分 |
| 04 | [04-evaluation.md](04-evaluation.md) | 評価基準、防衛質問、チェックリスト | 45-60分 |
| 05 | [05-solution.md](05-solution.md) | ソリューション解説 - アルゴリズム・コード詳解・実行トレース | 90-120分 |
| 06 | [06-design.md](06-design.md) | 設計原則、トレードオフ、代替案の比較分析 | 60-90分 |
| 07 | [07-study-guide.md](07-study-guide.md) | 学習ガイド、デバッグ手法、実践演習 | 60-90分 |
| 08 | [08-learning-goals.md](08-learning-goals.md) | 学習目標、後続プロジェクトとの関連、発展的話題 | 45-60分 |
| 09 | [09-comprehension-check.md](09-comprehension-check.md) | 理解度確認 - Q&A 60問以上（コードトレース含む） | 90-120分 |

---

## プロジェクト構成

```
get_next_line/
├── get_next_line.c            # メイン関数の実装（3関数）
│   ├── get_next_line()        #   外部公開: 全体制御と状態管理
│   ├── ft_init_stash()        #   static: stashの初期化
│   └── ft_read_to_stash()     #   static: fdからの読み込みとstash蓄積
│
├── get_next_line.h            # ヘッダファイル
│   ├── インクルードガード       #   #ifndef GET_NEXT_LINE_H
│   ├── BUFFER_SIZEデフォルト   #   #ifndef BUFFER_SIZE → 42
│   └── 全関数プロトタイプ       #   6関数分の宣言
│
├── get_next_line_utils.c      # ヘルパー関数群（5関数）
│   ├── ft_strlen()            #   NULLセーフな文字列長計算
│   ├── ft_strchr()            #   NULLセーフな文字検索
│   ├── ft_strjoin()           #   文字列結合（s1を解放する特殊版）
│   ├── ft_get_line()          #   stashから1行を切り出す
│   └── ft_trim_stash()        #   stashから使用済み部分を除去
│
├── get_next_line_bonus.c      # ボーナス: 複数fd対応版（3関数）
│   └── static char *stash[MAX_FD] を使用
│
├── get_next_line_bonus.h      # ボーナス用ヘッダ
│   └── MAX_FD定義を追加
│
├── get_next_line_utils_bonus.c  # ボーナス用ヘルパー（内容は通常版と同一）
│
├── .docs/
│   └── subject.md             # 課題文書（公式）
│
└── .wiki/
    ├── README.md              # このファイル
    ├── 01-background.md       # 背景知識
    ├── 02-prerequisites.md    # 前提知識
    ├── 03-requirements.md     # 要件仕様
    ├── 04-evaluation.md       # 評価基準
    ├── 05-solution.md         # ソリューション解説
    ├── 06-design.md           # 設計原則
    ├── 07-study-guide.md      # 学習ガイド
    ├── 08-learning-goals.md   # 学習目標
    └── 09-comprehension-check.md  # 理解度確認
```

---

## データフローの全体像

get_next_lineの全体的なデータの流れを理解することが、
このプロジェクトの攻略の鍵です。

```
┌────────────────────────────────────────────────────────────────┐
│                    ファイル（ディスク上）                        │
│  "Hello\nWorld\nFoo"                                           │
└──────────────────────────┬─────────────────────────────────────┘
                           │
                     open() で fd を取得
                           │
                           v
┌────────────────────────────────────────────────────────────────┐
│              カーネル空間（OS が管理）                           │
│  ┌──────────────────────────────────────┐                      │
│  │  ファイルオフセット: 現在の読み取り位置  │                      │
│  │  read() のたびにオフセットが進む         │                      │
│  └──────────────────────────────────────┘                      │
└──────────────────────────┬─────────────────────────────────────┘
                           │
                     read(fd, buf, BUFFER_SIZE)
                     システムコール（コンテキストスイッチ発生）
                           │
                           v
┌────────────────────────────────────────────────────────────────┐
│              ユーザー空間（あなたのプログラム）                   │
│                                                                │
│  ┌─────────────┐    ft_strjoin     ┌─────────────────────┐     │
│  │ buf (一時的) │ ──────────────→  │ stash (static蓄積)   │     │
│  │ malloc確保   │   buf の内容を    │ 前回の残り + 新規読込│     │
│  │ read後にfree │   stash に結合    │ \n が見つかるまで蓄積│     │
│  └─────────────┘                   └──────────┬──────────┘     │
│                                               │                │
│                              ft_get_line ──→ line (返却)       │
│                              ft_trim_stash ──→ 残り (次回用)    │
│                                                                │
│  ┌─────────────────────────────────────────────────┐           │
│  │ line は呼び出し元が free() する責任を持つ          │           │
│  └─────────────────────────────────────────────────┘           │
└────────────────────────────────────────────────────────────────┘
```

### 1回の get_next_line() 呼び出しの処理フロー

```
get_next_line(fd) が呼ばれた
         │
         v
    fd < 0 or BUFFER_SIZE <= 0 ?
    ┌──── YES ──→ return NULL（即座にエラー）
    │
    NO
    │
    v
    stash == NULL ?
    ┌──── YES ──→ ft_init_stash: malloc(1), stash = ""
    │
    NO（前回の残りデータがある）
    │
    v
    ft_read_to_stash(fd, stash)
    ┌─────────────────────────────────────┐
    │ buf = malloc(BUFFER_SIZE + 1)       │
    │ while (stash に \n がない            │
    │        && bytes_read > 0)           │
    │ {                                   │
    │     read(fd, buf, BUFFER_SIZE)      │
    │     buf[bytes_read] = '\0'          │
    │     stash = ft_strjoin(stash, buf)  │
    │ }                                   │
    │ free(buf)                           │
    │ return stash                        │
    └─────────────────────────────────────┘
         │
         v
    stash[0] == '\0' ?（何もデータがない）
    ┌──── YES ──→ free(stash), stash = NULL, return NULL
    │
    NO
    │
    v
    line = ft_get_line(stash)   ← stash から最初の行を切り出し
    stash = ft_trim_stash(stash) ← stash の残りを保持
         │
         v
    return line ← 呼び出し元が free する
```

---

## クイックスタート

### コンパイルと実行

```bash
# テスト用ファイルの作成
printf "Hello World\nThis is line 2\nLast line" > test.txt

# コンパイル（BUFFER_SIZE=42で）
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 \
    get_next_line.c get_next_line_utils.c your_main.c -o gnl

# 実行
./gnl

# 別のBUFFER_SIZEでテスト
cc -Wall -Wextra -Werror -D BUFFER_SIZE=1 \
    get_next_line.c get_next_line_utils.c your_main.c -o gnl_bs1
./gnl_bs1

cc -Wall -Wextra -Werror -D BUFFER_SIZE=10000000 \
    get_next_line.c get_next_line_utils.c your_main.c -o gnl_big
./gnl_big
```

### テスト用main.c

```c
#include "get_next_line.h"
#include <fcntl.h>
#include <stdio.h>

int	main(void)
{
	int		fd;
	char	*line;
	int		line_num;

	fd = open("test.txt", O_RDONLY);
	if (fd == -1)
	{
		printf("Error: cannot open file\n");
		return (1);
	}
	line_num = 1;
	while ((line = get_next_line(fd)) != NULL)
	{
		printf("Line %d: [%s]", line_num, line);
		free(line);
		line_num++;
	}
	printf("(EOF reached, NULL returned)\n");
	close(fd);
	return (0);
}
```

### メモリリークチェック

```bash
# デバッグ情報付きでコンパイル（-gオプション）
cc -Wall -Wextra -Werror -g -D BUFFER_SIZE=42 \
    get_next_line.c get_next_line_utils.c your_main.c -o gnl

# valgrindで実行
valgrind --leak-check=full --show-leak-kinds=all \
    --track-origins=yes ./gnl

# 期待される出力（リークなしの場合）:
# ==XXXXX== All heap blocks were freed -- no leaks are possible
# ==XXXXX== ERROR SUMMARY: 0 errors from 0 contexts
```

### Norm チェック

```bash
# 全ファイルのNormチェック
norminette get_next_line.c get_next_line.h get_next_line_utils.c

# ボーナスファイルも含める場合
norminette get_next_line_bonus.c get_next_line_bonus.h \
    get_next_line_utils_bonus.c

# 全ファイルで "OK!" が出力されればOK
```

---

## 各ファイルの概要と行数目安

このWikiの各ファイルは、以下の分量と内容を含んでいます。

| ファイル | 主な内容 | 特に力を入れた部分 |
|---------|---------|------------------|
| 01-background | fdの歴史、read()の深掘り、static変数のメモリ配置、Unix哲学 | メモリレイアウトのASCII図、カーネルの仕組み |
| 02-prerequisites | ポインタの完全理解、文字列操作、スタック/ヒープの詳細 | バイトレベルのメモリ図、ポインタの矢印図 |
| 03-requirements | 全エッジケースの網羅、Norm準拠チェックリスト | BUFFER_SIZEごとの動作表、入出力仕様の完全定義 |
| 04-evaluation | 評価者の視点、防衛質問集、失敗パターン分析 | よくある失敗の具体例とvalgrind出力 |
| 05-solution | 全関数の1行ずつの解説、実行トレース3パターン | BUFFER_SIZE=1,5,1024での完全トレース |
| 06-design | 設計判断の理由、代替案の比較、計算量分析 | メモリ所有権の図解、パフォーマンス分析 |
| 07-study-guide | gdb/valgrindの実践、テストファイル作成、穴埋め問題 | デバッグセッションの完全再現 |
| 08-learning-goals | 後続プロジェクトへの橋渡し、glibc比較、ベンチマーク | pipex/minishellとの具体的な関連 |
| 09-comprehension-check | 60問以上のQ&A、コードトレース、メモリ状態図問題 | 実際の評価で聞かれる質問の網羅 |

---

## 用語集

このWikiで使用される主要な用語の一覧です。

| 用語 | 英語 | 説明 |
|------|------|------|
| ファイルディスクリプタ | File Descriptor (fd) | OSが管理する、開かれたファイルへの整数識別子 |
| システムコール | System Call | ユーザープログラムからカーネルの機能を呼び出す仕組み |
| バッファ | Buffer | データを一時的に蓄える領域 |
| スタッシュ | Stash | get_next_lineで使う蓄積バッファ（static変数） |
| ヒープ | Heap | malloc()で動的に確保されるメモリ領域 |
| スタック | Stack | ローカル変数が自動確保されるメモリ領域 |
| データセグメント | Data Segment / BSS | static変数やグローバル変数が格納されるメモリ領域 |
| EOF | End Of File | ファイルの終端を示す状態 |
| ダングリングポインタ | Dangling Pointer | 解放済みメモリを指すポインタ |
| メモリリーク | Memory Leak | 確保したメモリを解放せずに参照を失うこと |
| ヌル終端 | Null Termination | 文字列の終わりを示す'\0'文字 |
| コンテキストスイッチ | Context Switch | ユーザーモードとカーネルモードの切り替え |
| Norm | Norminette | 42のコーディング規約とそのチェックツール |
| 所有権 | Ownership | メモリを解放する責任を持つこと |

---

## 変更履歴

| 日付 | 内容 |
|------|------|
| 2026-02-08 | Wiki全体を大幅改訂。全ファイルの内容を約5倍に拡充。メモリ図、実行トレース、演習問題を大量追加。 |
