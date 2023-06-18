#include <string>
#include <vector>
#include <iostream>
#include "y.tab.h"
#include <fstream>
#include <map>
#include "symbol.cpp"
using namespace std;

extern int linenum;
extern FILE file;



class JavaGenerater{
    ofstream file;
    map<int, string> INT2TYPE;
    SymbolTableManager symbolTableManager;
    SymbolTable GlobalSymbolTable;
    string className;
    string GetTab(){
        int tabNumber = symbolTableManager.GetScopeNumber();
        string tab = "";
        for(int i = 0; i < tabNumber; i++){
            tab += "\t";
        }
        return tab;
    }
    bool hasMain = false;
    void checkMain(){
        if(!hasMain){
            MainClassDeclaration();
        }
    }
    map<int, string> INT2OP = {
        {PLUS, "iadd"},
        {MINUS, "isub"},
        {TIMES, "imul"},
        {DIV, "idiv"},
        {MOD, "irem"},
        {AND, "iand"},
        {OR, "ior"},
        {NOT, "ineg"},
        {LT, "if_icmplt"},
        {GT, "if_icmpgt"},
        {LE, "if_icmple"},
    };
    
    void AddGlobalVar(string name, int type){
        GlobalSymbolTable.addSymbol(name, type, 0);
        file << GetTab() << "field static " << INT2TYPE[type] << " " << name << endl;
    }

    void AddGlobalVar(string name, int type, string value){
        GlobalSymbolTable.addSymbol(name, type, 0);
        file << GetTab() << "field static " << INT2TYPE[type] << " " << name << " = " << value << endl;
    }

    void GetGlobalVar(string name){
        file << GetTab() << "getstatic " << INT2TYPE[GlobalSymbolTable.getSymbolType(name)] << " " << className << "." << name << endl;
    }

    void PutGlobalVar(string name){
        file << GetTab() << "putstatic " << INT2TYPE[GlobalSymbolTable.getSymbolType(name)] << " " << className << "." << name << endl;
    }

    void AddLocalVar(string name, int type){
        symbolTableManager.addSymbol(name, type);
        file << GetTab() << "istore " << symbolTableManager.getSymbolIndex(name) << endl;
    }

    void GetLocalVar(string name){
        file << GetTab() << "iload " << symbolTableManager.getSymbolIndex(name) << endl;
    }

    void PutLocalVar(string name){
        file << GetTab() << "iload " << symbolTableManager.getSymbolIndex(name) << endl;
    }
    void StartScope(){
        file << GetTab() << "{" << endl;
        symbolTableManager.createSymbolTable();
    }
    void EndScope(){
        symbolTableManager.destroySymbolTable();
        file << GetTab() << "}" << endl;
    }

public:
    JavaGenerater(string filename, SymbolTableManager& symbolTableManager){
        className = filename;
        file.open(filename + ".jasm");
        INT2TYPE[INT] = "int";
        INT2TYPE[REAL] = "float";
        INT2TYPE[BOOL] = "boolean";
        INT2TYPE[STRING] = "String";

        this->symbolTableManager = symbolTableManager; 
    }

    int GetSymbolType(string name){
        if(symbolTableManager.containsSymbol(name)){
            return symbolTableManager.getSymbolType(name);
        }else{
            return GlobalSymbolTable.getSymbolType(name);
        }
    }

    void ProgramInit(){
        file << GetTab() << "class " << className << endl;
        StartScope();
    }

    void ProgramEnd(){
        checkMain();
        Return();
        EndScope();
        EndScope();
    }

    void MainClassDeclaration(){
        hasMain = true;
        auto args = vector<string>();
        args.push_back("java.lang.String[]");
        FunctionDeclaration("main", "void", args);
    }

    void VarDeclaration(string name, int type){
        if(hasMain){
            AddLocalVar(name, type);
        }else{
            AddGlobalVar(name, type);
        }
    }



    void LoadIdentifier(string name){
        if(symbolTableManager.containsSymbol(name)){
            GetLocalVar(name);
        }else{
            GetGlobalVar(name);
        }
    }

    void PutIdentifier(string name){
        checkMain();
        if(symbolTableManager.containsSymbol(name)){
            PutLocalVar(name);
        }else{
            PutGlobalVar(name);
        }
    }


    void Push(string value){
        checkMain();
        file << GetTab() << "ldc " << value << endl;
    }


    void PutInit(){
        checkMain();
        file << GetTab() << "getstatic java.io.PrintStream java.lang.System.out" << endl;
    }

    void PutEnd(int type){
        file << GetTab() << "invokevirtual void java.io.PrintStream.print(" << INT2TYPE[type] << ")" << endl;
    }


    void Operator(int op){
        file << GetTab() << INT2OP[op] << endl;
    }

    void FunctionDeclaration(string name, string returnType, vector<string> args){
        file << GetTab() << "method public static " << returnType << " " << name << "(";
        for(int i = 0; i < args.size(); i++){
            file << args[i];
            if(i != args.size() - 1){
                file << ", ";
            }
        }
        file << ")" << endl;
        file << GetTab() << "max_stack 15" << endl;
        file << GetTab() << "max_locals 15" << endl;
        StartScope();
    }
    
    

    void Return(){
        checkMain();
        file << GetTab() << "return" << endl;
    }

    SymbolTable& GetGlobalSymbolTable(){
        return GlobalSymbolTable;
    }
};