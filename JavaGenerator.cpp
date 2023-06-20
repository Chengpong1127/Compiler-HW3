#include <string>
#include <vector>
#include <iostream>
#include "y.tab.h"
#include <fstream>
#include <map>
#include "symbol.cpp"
#include <stack>
#include <vector>
using namespace std;

extern int linenum;
extern FILE file;



class JavaGenerater{
    ofstream file;
    
    SymbolTableManager symbolTableManager;
    SymbolTable GlobalSymbolTable;
    string className;
    int loopID = 0;
    stack<int> loopStack;
    int ifID = 0;
    stack<int> ifStack;
    const string IFEXIT = "ifexit";
    const string IFELSE = "ifelse";
    const string LOOPSTART = "loopstart";
    const string LOOPEXIT = "loopexit";
    const string LOOPTRUE = "looptrue";
    const string LOOPFALSE = "loopfalse";
    int ClabelNumber = 0;
    vector<int> functionTable;
    string GetTab(){
        int tabNumber = symbolTableManager.GetScopeNumber();
        string tab = "";
        for(int i = 0; i < tabNumber; i++){
            tab += "\t";
        }
        return tab;
    }
    bool global = true;
    bool hasMain = false;
    bool inFunction = false;
    void checkMain(){
        if(!hasMain && !inFunction){
            MainClassDeclaration();
        }
    }
    map<int, string> INT2TYPE = {
        {INT, "int"},
        {REAL, "float"},
        {BOOL, "boolean"},
        {STRING, "java.lang.String"},
        {VOID, "void"},
    };
    map<int, string> INT2OP = {
        {PLUS, "iadd"},
        {MINUS, "isub"},
        {TIMES, "imul"},
        {DIV, "idiv"},
        {MOD, "irem"},
        {AND, "iand"},
        {OR, "ior"},
        {NOT, "ixor"},
        {GT, "ifgt"},
        {LT, "iflt"},
        {GE, "ifge"},
        {LE, "ifle"},
        {EQ, "ifeq"},
        {NE, "ifne"},
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
    }
    void AddLocalVar(string name, int type, string value){
        symbolTableManager.addSymbol(name, type);
        PutLocalVar(name);
    }

    void GetLocalVar(string name){
        file << GetTab() << "iload " << symbolTableManager.getSymbolIndex(name) << endl;
    }

    void PutLocalVar(string name){
        file << GetTab() << "istore " << symbolTableManager.getSymbolIndex(name) << endl;
    }
    void StartScope(){
        file << GetTab() << "{" << endl;
        symbolTableManager.createSymbolTable();
    }
    void EndScope(){
        symbolTableManager.destroySymbolTable();
        file << GetTab() << "}" << endl;
    }

    void LabelStatement(string label){
        file << GetTab() << label << ":" << endl;
        Command("nop");
    }
    void GOTOStatement(string label){
        file << GetTab() << "goto " << label << endl;
    }
    string GetLabelWithScope(string label){
        return label + to_string(symbolTableManager.GetScopeNumber());
    }
    string GetLabelWithScope(string label, int scope){
        return label + to_string(scope);
    }
    string GetLabelWithScope(string label, int scope1, int scope2){
        return label + to_string(scope1) + "_" + to_string(scope2);
    }
    string GetLabelNumber(int scope1, int scope2){
        return to_string(scope1) + "_" + to_string(scope2);
    }
    void PushConstValue(string name, bool local = true){
        checkMain();
        if(local){
            Push(symbolTableManager.GetStrVal(name));
        }else{
            Push(GlobalSymbolTable.GetStrVal(name));
        }
    }

public:
    JavaGenerater(string filename, SymbolTableManager& symbolTableManager){
        className = filename;
        file.open(filename + ".jasm");

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
        global = false;
        hasMain = true;
        auto args = vector<string>();
        args.push_back("java.lang.String[]");
        FunctionDeclaration("main", "void", args);
    }

    void VarDeclaration(string name, int type){
        if(global){
            AddGlobalVar(name, type);
        }else{
            checkMain();
            AddLocalVar(name, type);
        }
    }
    void VarDeclaration(string name, int type, string value){
        if(global){
            AddGlobalVar(name, type, value);
        }else{
            checkMain();
            AddLocalVar(name, type);
            PutLocalVar(name);
        }
    }
    void ConstDeclaration(string name, int type, string value){
        if(global){
            GlobalSymbolTable.addSymbol(name, type,0 , true, value);
        }else{
            symbolTableManager.addSymbol(name, type, true, value);
        }
        
    }


    void LoadIdentifier(string name){
        checkMain();
        if(symbolTableManager.containsSymbol(name)){
            if(symbolTableManager.getSymbolIsConsistent(name)){
                PushConstValue(name, true);
            }else{
                GetLocalVar(name);
            }
        }else if(GlobalSymbolTable.containsSymbol(name)){
            if(GlobalSymbolTable.GetSymbolIsConsistent(name)){
                PushConstValue(name, false);
            }else{
                GetGlobalVar(name);
            }
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

    string GetConstValue(string name){
        if(symbolTableManager.containsSymbol(name)){
            return symbolTableManager.GetStrVal(name);
        }else{
            return GlobalSymbolTable.GetStrVal(name);
        }
    }

    

    void Push(string value){
        checkMain();
        file << GetTab() << "ldc " << value << endl;
    }
    void Push(int value){
        checkMain();
        file << GetTab() << "ldc " << to_string(value) << endl;
    }

    void IFInit(){
        checkMain();
        ifID++;
        ifStack.push(ifID);
        file << GetTab() << "ifeq " << GetLabelWithScope(IFELSE, ifID) << endl;
        symbolTableManager.createSymbolTable();
    }
    void GOTOExit(){
        checkMain();
        GOTOStatement(GetLabelWithScope(IFEXIT, ifStack.top()));
    }
    void IFElse(){
        checkMain();
        symbolTableManager.destroySymbolTable();
        GOTOExit();
        LabelStatement(GetLabelWithScope(IFELSE, ifStack.top()));
        symbolTableManager.createSymbolTable();
    }
    void IFEnd(){
        symbolTableManager.destroySymbolTable();
        checkMain();
        LabelStatement(GetLabelWithScope(IFEXIT, ifStack.top()));
        ifStack.pop();
    }

    void LoopInit(){
        checkMain();
        loopID++;
        loopStack.push(loopID);
        LabelStatement(GetLabelWithScope(LOOPSTART, loopID));
        symbolTableManager.createSymbolTable();
    }
    void LoopEnd(){
        checkMain();
        symbolTableManager.destroySymbolTable();
        GOTOStatement(GetLabelWithScope(LOOPSTART, loopStack.top()));
        LabelStatement(GetLabelWithScope(LOOPEXIT, loopStack.top()));
        loopStack.pop();
    }
    void ExitLoop(){
        checkMain();
        
        GOTOStatement(GetLabelWithScope(LOOPEXIT, loopStack.top()));
    }
    void ExitLoopWhen(){
        checkMain();
        file << GetTab() << "ifgt " << GetLabelWithScope(LOOPEXIT, loopStack.top()) << endl;
    }

    void ForInit(string name, string from, string to, bool increase = true){
        checkMain();
        symbolTableManager.createSymbolTable();
        VarDeclaration(name, INT);
        Push(from);
        PutIdentifier(name);
        LoopInit();

        LoadIdentifier(name);
        Push(to);
        CompareOperator(increase? GT: LT);
        ExitLoopWhen();
        
    }
    void ForEnd(string name, bool increase = true){
        checkMain();
        LoadIdentifier(name);
        Push("1");
        Operator(increase? PLUS: MINUS);
        PutIdentifier(name);
        LoopEnd();
        symbolTableManager.destroySymbolTable();
    }
    
    void Command(string command){
        checkMain();
        file << GetTab() << command << endl;
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
    void CompareOperator(int op){
        const string CTRUE = "true";
        const string CFALSE = "false";
        Command("isub");
        Command(INT2OP[op] + " " + GetLabelWithScope(CTRUE, ClabelNumber));
        Command("iconst_0");
        GOTOStatement(GetLabelWithScope(CFALSE, ClabelNumber));
        LabelStatement(GetLabelWithScope(CTRUE, ClabelNumber));
        Command("iconst_1");
        LabelStatement(GetLabelWithScope(CFALSE, ClabelNumber));
        ClabelNumber++;
    }
    void Skip(){
        checkMain();
        Command("getstatic java.io.PrintStream java.lang.System.out");
        Command("invokevirtual void java.io.PrintStream.println()");
    }

    void FunctionDeclaration(string name, string returnType, vector<string> args){
        global = false;
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
    
    void AddParameter(string name, int type){
        functionTable.push_back(type);
        AddLocalVar(name, type);
    }
    void FunctionInit(){
        inFunction = true;
        symbolTableManager.createSymbolTable();
        functionTable.clear();
    }

    void FunctionCreate(string name, int type, int returnType){
        auto argsString = vector<string>();
        for(auto arg : functionTable){
            argsString.push_back(INT2TYPE[arg]);
        }
        FunctionDeclaration(name, INT2TYPE[returnType], argsString);
        GlobalSymbolTable.addFunctionSymbol(name, FUNCTION, functionTable, returnType);
    }
    void FunctionEnd(){
        Return();
        EndScope();
        inFunction = false;
        symbolTableManager.destroySymbolTable();
    }
        
    void FunctionCall(string name){
        checkMain();
        auto ps = GlobalSymbolTable.getFunctionParameters(name);
        auto returnType = GlobalSymbolTable.getFunctionReturnType(name);
        file << GetTab() << "invokestatic " << INT2TYPE[returnType] << " " << className << "." << name << "(";
        
        for(int i = 0; i < ps.size(); i++){
            file << INT2TYPE[ps[i]];
            if(i != ps.size() - 1){
                file << ", ";
            }
        }
        file << ")" << endl;
    }
    void BlockInit(){
        checkMain();
        symbolTableManager.createSymbolTable();
    }
    void BlockEnd(){
        checkMain();
        symbolTableManager.destroySymbolTable();
    }

    void Return(){
        checkMain();
        file << GetTab() << "return" << endl;
    }
    void Result(int type){
        checkMain();
        switch (type)
        {
        case INT:
            Command("ireturn");
            break;
        case REAL:
            Command("freturn");
            break;
        case BOOL:
            Command("ireturn");
            break;
        
        default:
            break;
        }
    }

    SymbolTable& GetGlobalSymbolTable(){
        return GlobalSymbolTable;
    }
    void WriteCode(string code){
        return;
        file << "/* ";
        for(auto c : code){
            if(c == '\n'){
            }else{
                file << c;
            }
        }
        file << " */" << endl;
    }
};