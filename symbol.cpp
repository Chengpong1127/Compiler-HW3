#ifndef SYMBOL_H
#define SYMBOL_H

#include <map>
#include <string>
#include <stack>
#include <iostream>
#include <vector>
#include "y.tab.h"
using namespace std;


// 這個程式是用來實作SymbolTable的
class SymbolTable;
// 這是SymbolType的定義，包含了type和table和returnType，table 和 returnType只有在function的時候才會用到
struct SymbolType{
    int type;
    vector<int> table;
    int returnType;
    bool isConsistent;
    int index;
};


// 這是SymbolTable的定義
class SymbolTable {
private:
    map<string, SymbolType> symbolMap; //儲存符號的map
public:
    //新增變數符號
    void addSymbol(string name, int type, int index, bool isConsistent = false){
        symbolMap[name].type = type;
        symbolMap[name].isConsistent = isConsistent;
        symbolMap[name].index = index;
    }
    //新增函式符號
    void addFunctionSymbol(string name, int type, vector<int> parameterMap, int returnType, bool isConsistent = false){
        SymbolType symboltype = SymbolType{type, parameterMap, returnType, isConsistent};
        symbolMap[name] = symboltype;
    }
    void addArraySymbol(string name, int type, int arrayType, bool isConsistent = false){
        SymbolType symboltype = SymbolType{type, vector<int>(), arrayType, isConsistent};
        symbolMap[name] = symboltype;
    }
    int GetArrayReturnType(string name){
        return symbolMap[name].returnType;
    }
    const bool GetSymbolIsConsistent(string name){
        return symbolMap[name].isConsistent;
    }
    //取得函式回傳值型別
    const int getFunctionReturnType(string name){
        return symbolMap[name].returnType;
    }
    //取得函式參數型別清單
    const vector<int> getFunctionParameters(string name){
        return symbolMap[name].table;
    }
    //檢查函式是否有第index個參數
    const bool containsFunctionParameter(string name, int parameter_number){
        return symbolMap[name].table.size() > parameter_number;
    }
    //取得函式的參數數量
    const int getFunctionParameterNumber(string name){
        return symbolMap[name].table.size();
    }
    //檢查符號表是否有此符號
    const bool containsSymbol(const string name) {
        return symbolMap.find(name) != symbolMap.end();
    }
    //取得符號型別
    const int getSymbolType(const string name) {
        return symbolMap[name].type;
    }
    //取得符號的index
    const int getSymbolIndex(const string name){
        return symbolMap[name].index;
    }
    //印出符號表
    void dump(){
        for(auto i : symbolMap){
            cout<<i.first<<"\t"<<i.second.type<<endl;
        }
    }
    const int getSymbolCount(){
        return symbolMap.size();
    }
};

//符號表管理員
class SymbolTableManager{
private:
    stack<SymbolTable*> *SymbolTableStack = new stack<SymbolTable*>(); //儲存符號表的堆疊
    SymbolTable GlobalSymbolTable; //全域符號表
    int index = 0; //用來記錄目前的index是多少
public:
    int GetScopeNumber(){
        return SymbolTableStack->size();
    }

    //新增一個符號表於堆疊最上面
    void createSymbolTable() {
        SymbolTable* newSymbolTable = new SymbolTable();
        SymbolTableStack->push(newSymbolTable);
    }
    //刪除堆疊最上面的符號表
    void destroySymbolTable() {
        SymbolTable* symbolTableToDelete = SymbolTableStack->top();
        SymbolTableStack->pop();
        index -= symbolTableToDelete->getSymbolCount();
        delete symbolTableToDelete;

    }
    //在堆疊最上面的符號表新增一個變數符號
    void addSymbol(string name, int type, bool isConsistent = false) {
        SymbolTable* topSymbolTable = SymbolTableStack->top();
        topSymbolTable->addSymbol(name, type, index++, isConsistent);
    }
    void addGlobalSymbol(string name, int type, bool isConsistent = false) {
        GlobalSymbolTable.addSymbol(name, type, 0, isConsistent);
    }

    const bool checkSymbolLocal(string name){
        auto tempStack = *SymbolTableStack;
        while(!tempStack.empty()){
            auto table = tempStack.top();
            if(table->containsSymbol(name)){
                return true;
            }
            else{
                tempStack.pop();
            }
            
        }
        return false;
    }
    //在堆疊最上面的符號表新增一個函式符號
    void addFunctionSymbol(string name, int type, vector<int> parameters, int returnType, bool isConsistent = false){
        SymbolTable* topSymbolTable = SymbolTableStack->top();
        topSymbolTable->addFunctionSymbol(name, type, parameters, returnType, isConsistent);
    }
    //檢查是否有此符號存在於堆疊中的符號表中
    const bool containsSymbol(string name){
        auto type = getSymbolType(name);
        return type != -1;
    }
    const bool getSymbolIsConsistent(string name){
        auto tempStack = *SymbolTableStack;
        while(!tempStack.empty()){
            auto table = tempStack.top();
            if(table->containsSymbol(name)){
                return table->GetSymbolIsConsistent(name);
            }
            else{
                tempStack.pop();
            }
            
        }
        return false;
    }
    //取得符號存在於堆疊中的最靠近頂端的符號表中的型別
    const int getSymbolType(const string name) {
        auto tempStack = *SymbolTableStack;
        while(!tempStack.empty()){
            auto table = tempStack.top();
            if(table->containsSymbol(name)){
                return table->getSymbolType(name);
            }
            else{
                tempStack.pop();
            }
            
        }
        return -1;
    }
    //取得符號存在於堆疊中的最靠近頂端的符號表中的index
    const int getSymbolIndex(const string name){
        auto tempStack = *SymbolTableStack;
        while(!tempStack.empty()){
            auto table = tempStack.top();
            if(table->containsSymbol(name)){
                return table->getSymbolIndex(name);
            }
            else{
                tempStack.pop();
            }
            
        }
        return -1;
    }
    //取得函式存在於堆疊中的最靠近頂端的符號表中的回傳值型別
    const int getFunctionReturnType(string name){
        auto tempStack = *SymbolTableStack;
        while(!tempStack.empty()){
            auto table = tempStack.top();
            if(table->containsSymbol(name)){
                return table->getFunctionReturnType(name);
            }
            else{
                tempStack.pop();
            }
            
        }
        return -1;
    }
    //取得函式存在於堆疊中的最靠近頂端的符號表中的參數型別清單
    const vector<int> getFunctionParameters(string name){
        auto tempStack = *SymbolTableStack;
        while(!tempStack.empty()){
            auto table = tempStack.top();
            if(table->containsSymbol(name)){
                return table->getFunctionParameters(name);
            }
            else{
                tempStack.pop();
            }
            
        }
        return vector<int>();
        
    }
    //檢查函式存在於堆疊中的最靠近頂端的符號表中是否有指定index的參數
    const bool containsFunctionParameter(string name, int parameter_number){
        auto tempStack = *SymbolTableStack;
        while(!tempStack.empty()){
            auto table = tempStack.top();
            if(table->containsSymbol(name)){
                return table->containsFunctionParameter(name, parameter_number);
            }
            else{
                tempStack.pop();
            }
            
        }
        return false;
    }
    void addArraySymbol(string name, int type, int arrayType, bool isConsistent = false){
        SymbolTable* topSymbolTable = SymbolTableStack->top();
        topSymbolTable->addArraySymbol(name, type, arrayType, isConsistent);
    }
    int GetArrayReturnType(string name){
        auto tempStack = *SymbolTableStack;
        while(!tempStack.empty()){
            auto table = tempStack.top();
            if(table->containsSymbol(name)){
                return table->GetArrayReturnType(name);
            }
            else{
                tempStack.pop();
            }
            
        }
        return -1;
    }
    //印出堆疊最上面的符號表
    void dumpTop(){
        SymbolTableStack->top()->dump();
    }
    //印出堆疊中所有符號表
    void dumpAll() {
        stack<SymbolTable*> tmpStack = *SymbolTableStack;
        while (!tmpStack.empty()) {
            tmpStack.top()->dump();
            tmpStack.pop();
            cout<<endl;
        }
    }
};

#endif /* SymbolTable_h */