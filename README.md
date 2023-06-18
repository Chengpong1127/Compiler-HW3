# mylex.l 的改動

- 刪除main 函數。
- SymbolTable 實作移動到symbol.cpp 裏面。
- token() 多傳入一個參數是用來return的token。
- 當偵測到operater 如 "+ - * /" 等，會將型態設定給yylval.type。
- 當偵測到NumericalConstants和identifier 時會將value 傳給yylval.sval。

# 其他說明

資料夾裡主要的程式檔案有mylex.l, myyacc.y, symbol.cpp, util.cpp。
symbol.cpp 是用來實作symbol table 的，util.cpp 是用來實作一些輔助的函數。
- 測試的檔案放在test資料夾裡面。
- 編譯出來的檔案是myparser.out, 編譯指令為
```bash
make
```
執行指令為
```bash
./myparser.out test/test1.st
```
