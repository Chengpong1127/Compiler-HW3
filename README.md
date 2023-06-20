# mylex.l 的改動

- 將每一行的結果傳給generator做成註解印出。

# myyacc.y 的改動
- 將symbolTable交給generator處理。
- 將不需要處理的部分刪除，如Array。

# 其他說明

這次新增了JavaGenerator.cpp，用來產生Java程式碼。
myyacc.y的所有行為都交給generator處理，generator會根據行為產生Java程式碼。

## 編譯方法
```bash
make
```
執行指令為
```bash
./myparser.out test/test1.st && ./javaa/javaa output.jasm && java output
```
