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

    string GetTab(){
        int tabNumber = symbolTableManager.GetScopeNumber();
        string tab = "";
        for(int i = 0; i < tabNumber; i++){
            tab += "\t";
        }
        return tab;
    }
public:
    JavaGenerater(string filename, SymbolTableManager& symbolTableManager){
        file.open(filename);
        INT2TYPE[INT] = "int";
        INT2TYPE[REAL] = "float";
        INT2TYPE[BOOL] = "boolean";
        INT2TYPE[STRING] = "String";

        this->symbolTableManager = symbolTableManager; 
    }

    void ProgramInit(){
        file << GetTab() << "class output" << endl;
        file << GetTab() << "{" << endl;
    }

    void MainClassDeclaration(string name){
        FunctionDeclaration(name, "void", vector<string>());
    }

    void VarDeclaration(string name, int type){
        symbolTableManager.addSymbol(name, type);
    }


    void Push(string value){
        file << GetTab() << "ldc " << value << endl;
    }

    void AssignLocal(string name){
        file << GetTab() << "istore " << symbolTableManager.getSymbolIndex(name) << endl;
    }

    void LoadLocal(string name){
        file << GetTab() << "iload " << symbolTableManager.getSymbolIndex(name) << endl;
    }

    void Put(int type){
        file << GetTab() << "getstatic java.io.PrintStream java.lang.System.out" << endl;
        file << GetTab() << "invokevirtual void java.io.PrintStream.print(" << INT2TYPE[type] << ")" << endl;
    }


    void Add(){
        file << GetTab() << "iadd" << endl;
    }
    void Sub(){
        file << GetTab() << "isub" << endl;
    }

    void Mul(){
        file << GetTab() << "imul" << endl;
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
        file << "{" << endl; 
    }
    void EndScope(){
        file << "}" << endl;
    }
};