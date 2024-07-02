# コンパイラ
CC = gcc

# コンパイルオプション
CFLAGS = -Wall -O2

# リンクオプション
LDFLAGS = -lm

# オブジェクトファイル
OBJ = original.o original_func.o

# 実行ファイル
TARGET = original

# デフォルトターゲット
all: $(TARGET)

# 実行ファイルのリンク
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# 個別のオブジェクトファイルの生成
Onsei.o: original.c original.h
	$(CC) $(CFLAGS) -c original.c

voice_change3.o: original_func.c original.h
	$(CC) $(CFLAGS) -c original_func.c

# クリーンアップ
clean:
	rm -f $(OBJ) $(TARGET)

# ターゲットを指定してクリーンアップ
.PHONY: all clean
