#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include<stdlib.h> 
#include <string.h>

using namespace std;

static int MACHINE_SIZE = 512;
static int linenum;
static int lineoffset;
static ifstream file;
static string filename;
string buf;
string readline;
string prev_token;
bool lineend = true;
int instru_num;
int total_instru_num = 0;
int base_address = 0;
int modIndex = 0;
int modNum = 1;


class linker
{
public:
	linker();
	~linker();
};

class Warning 
{
	int code;
	string sym;
	int val;
	int modLength;
	int mod;
public:
	void setCode(int c) {
		this->code = c;
	}
	void setSym(string s) {
		this->sym = s;
	}
	void setVal(int v) {
		this->val = v;
	}
	void setLength(int l) {
		this->modLength = l;
	}
	void setMod(int m) {
		this->mod = m;
	}
	string getInfo() {
		switch (this->code)
		{
		case 2:
			return "Error: This variable is multiple times defined; first value used";
		case 3:
			return "Error: " + sym + " is not defined; zero used";
		case 4:
			return "Warning: Module " + to_string(mod) + ": " + sym + " was defined but never used";
		case 5:
			return "Warning: Module " + to_string(mod) + ": " + sym + " too big "+ to_string(val) +"(max = "+ to_string(modLength) +") assume zero relative";
		case 6:
			return "Error: External address exceeds length of uselist; treated as immediate";
		case 7:
			return "Warning: Module " + to_string(mod) + ": " + sym + " appeared in the uselist but was not actually used";
		case 8:
			return "Error: Absolute address exceeds module size; zero used";
		case 9:
			return "Error: Relative address exceeds module size; zero used";
		case 10:
			return "Error: Illegal Imediate value; treated as 9999";
		case 11:
			return "Error: Illegal opcode; treated as 9999";
		default:
			return "NONE";
		}
	}
};

class Symbol
{
	bool used;
	int modulenum;
	string name;
	int val;
	vector<Warning> warnList;


public:
	Symbol() {
		this->used = false;
	}
	
	string getName() {
		return this->name;
	}

	int getVal() {
		return this->val;
	}

	void setName(string n) {
		this->name = n;
	}
	void setVal(int v) {
		this->val = v;
	}
	void insertWarnList(Warning warn) {
		this->warnList.push_back(warn);
	}
	vector<Warning> getWarnList() {
		return this->warnList;
	}
	bool getUsed() {
		return this->used;
	}
	void setUsed() {
		this->used = true;
	}
	int getModuleNum() {
		return this->modulenum;
	}
	void setModuleNum(int n) {
		this->modulenum = n;
	}
	
};


class Instru
{
	string addressing;
	int opcode;
	int oprand;
	vector<Warning> warnList;
public:
	void setAddressing(string a) {
		this->addressing = a;
	}
	void setOpcode(int o) {
		this->opcode = o;
	}
	void setOprand(int o) {
		this->oprand = o;
	}
	void insertWarnList(Warning warn) {
		this->warnList.push_back(warn);
	}
	vector<Warning> getWarnList() {
		return this->warnList;
	}
	string getAddressing() {
		return this->addressing;
	}
	int getOprand() {
		return this->oprand;
	}
	int getOpcode() {
		return this->opcode;
	}
};

class Module
{
	int length;
	int address;
	vector<Symbol> defList;
	vector<string> useList;
	vector<Instru> instruList;
	vector<Warning> warnList;

public:
	int getAddress() {
		return this->address;
	}
	void setAddress(int a) {
		this->address = a;
	}
	int getLength() {
		return this->length;
	}
	void setLength(int len) {
		this->length = len;
	}
	void insertDef(Symbol symbol) {
		this->defList.push_back(symbol);
	}
	void insertUse(string symName) {
		this->useList.push_back(symName);
	}
	void insertInstru(Instru instruction) {
		this->instruList.push_back(instruction);
	}
	void setDefList(vector<Symbol> dl) {
		this->defList = dl;
	}
	void setUseList(vector<string> ul) {
		this->useList = ul;
	}
	void setInstruList(vector<Instru> il) {
		this->instruList = il;
	}
	vector<Instru> getInstruList() {
		return this->instruList;
	}
	vector<string> getUseList() {
		return this->useList;
	}
	vector<Symbol> getDefList() {
		return this->defList;
	}
	void insertWarnList(Warning warn) {
		this->warnList.push_back(warn);
	}
	vector<Warning> getWarnList() {
		return this->warnList;
	}
};



vector<Symbol> definedSymbol;
vector<Warning> warningList;
vector<Module> modList;
vector<Symbol> not_defined_list;



void _parseerror(int errcode) {
	static char* errstr[] = {
		"NUM_EXPECTED",              // Number Expected
		"SYM_EXPECTED",              // Symbol Expected
		"ADDR_EXPECTED",             // Addressing Expected which is A/E/I/R
		"SYM_TOO_LONG",              // Symbol Name is too long
		"TOO_MANY_DEF_IN_MODULE",    // > 16
		"TOO_MANY_USE_IN_MODULE",    // > 16
		"TOO_MANY_INSTR",            // total num_instr exceeds moemory size (512)
	};
	printf("Parse Error Line %d offset %d : %s\n", linenum, lineoffset, errstr[errcode]);
//	system("pause");
}

bool isNumber(string token) {
	for (int i = 0; i < token.length(); i++) {
		if (!isdigit(token.at(i))) {
			return false;
		}
	}
	return true;
}

bool checkSymName(string sym) {
	if (!isalpha(sym.at(0))) {
		_parseerror(1);
		exit(1);
	}
	if (sym.length() > 16) {
		_parseerror(3);
		exit(1);
	}
	for (int i = 1; i < sym.length(); i++) {
		if (!isalnum(sym.at(i))) {
			_parseerror(1);
			exit(1);
		}
	}
	return true;
}

bool checkAddr(string addr) {
	if (addr == "I" || addr == "A" || addr == "E" || addr == "R") {
		return true;
	}
	else {
		_parseerror(2);
		exit(1);
	}
}

char* readToken() {
	char str[2048];
	char *p;
	char *token;
	if (lineend) {
		if (!getline(file, readline)) {
			lineoffset += prev_token.length();
			return NULL;
		}
		linenum++;
		strcpy(str, readline.c_str());

	}
	else {
		strcpy(str, buf.c_str());
	}
	
	token = strtok_r(str, " \n\t", &p);
	buf = p;
	if (strlen(p) == 0) {
		lineend = true;
	}
	else {
		lineend = false;
	}
	if (token != NULL) {
		lineoffset = readline.length() - strlen(p) - strlen(token) + 1;
		prev_token = token;
		return token;
	}

	while (getline(file, readline)) {
		linenum++;
		strcpy(str, readline.c_str());
		token = strtok_r(str, " \n\t", &p);

		if (token != NULL) {
			lineoffset = readline.length() - strlen(p) - strlen(token) + 1;
			buf = p;
			if (strlen(p) != 0) {
				lineend = false;
			}
			else {
				lineend = true;
			}
			prev_token = token;
			return token;
		}
	}
//	lineoffset += prev_token.length();
	return NULL;
}

Symbol readDef() {
	//(S,R) pairs
	Symbol sym;
	bool same = false;
	sym.setModuleNum(modNum);
	//read symbol name
	char *token = readToken();
	if (token != NULL) {
		checkSymName(token);
		for (int i = 0; i < definedSymbol.size(); i++) {
			if (definedSymbol[i].getName() == token) {
				Warning warn;
				warn.setCode(2);
				warn.setSym(token);
				definedSymbol[i].insertWarnList(warn);
				same = true;
			}
		}
		if (same) {
			sym.setName("");
		}
		sym.setName(token);
	}
	else {
		_parseerror(1);
		exit(1);
	}
	//read symbol value
	token = readToken();
	if (token != NULL && isNumber(token)) {
		sym.setVal(atoi(token)+base_address);
	}
	else {
		_parseerror(0);
		exit(1);
	}
	if (!same) {
		definedSymbol.push_back(sym);
	}
	return sym;
}

vector<Symbol> readDefList() {

	int pairsNum = 0;
	vector<Symbol> result;
	char* token = readToken();
	if (token != NULL) {
		if (isNumber(token)) {
			pairsNum = atoi(token);
		}
		else {
			_parseerror(0);
			exit(1);
		}

		if (pairsNum > 16) {
			lineoffset--;
			_parseerror(4);
			exit(1);
		}
	}
	else {
		_parseerror(0);
		exit(1);
	}

	for (int i = 0; i < pairsNum; i++) {
		Symbol sym = readDef();
		if (sym.getName() != "") {
			result.push_back(sym);

		}
	}
	return result;

}



vector<string> readUseList() {
	int num = 0;
	vector<string> result;
	char* token = readToken();
	if (token != NULL) {
		if (isNumber(token)) {
			num = atoi(token);
		}
		else {
			_parseerror(0);
			exit(1);
		}

		if (num > 16) {
			lineoffset--;
			_parseerror(5);
			exit(1);
		}
	}
	else {
		_parseerror(0);
		exit(1);
	}

	for (int i = 0; i < num; i++) {
		token = readToken();
		string sym;
		if (token != NULL) {
			checkSymName(token);
			sym = token;
		}
		else {
			_parseerror(1);
			exit(1);
		}
		result.push_back(sym);
	}
	return result;
}

vector<Instru> readInstruList() {
	vector<Instru> instruList;
	char* token = readToken();
	if (token != NULL) {
		if (isNumber(token)) {
			instru_num = atoi(token);
		}
		else {
			_parseerror(0);
			exit(1);
		}
		total_instru_num += instru_num;
		if (total_instru_num >= 512-1) {
			lineoffset--;
			_parseerror(6);
			exit(1);
		}
	}
	else {
		_parseerror(0);
		exit(1);
	}

	for (int i = 0; i < instru_num; i++) {
		//read addressing
		token = readToken();
		Instru instruction;
		if (token != NULL) {
			checkAddr(token);
			instruction.setAddressing(token);
			
		}
		else {
			_parseerror(2);
			exit(1);
		}

		token = readToken();
		if (token != NULL) {
			if (!isNumber(token)) {
				_parseerror(0);
				exit(1);
			}
		}
		instruList.push_back(instruction);
	}
	return instruList;
}

Instru readInstru() {
	Instru instruction;
	string token = readToken();
	instruction.setAddressing(token);
	if (token == "I") {
		token = readToken();
		if (token.length() > 4) {
			Warning warn;
			warn.setCode(10);
			instruction.insertWarnList(warn);
			instruction.setOpcode(9);
			instruction.setOprand(999);
		}
		else {
			if (token.length() == 4) {
				instruction.setOpcode(token.at(0) - '0');
				instruction.setOprand(stoi(token.substr(1, 3)));
			}
			else {
				instruction.setOpcode(0);
				instruction.setOprand(stoi(token));
			}

		}
	}
	else {
		string current_addr = token;
		token = readToken();
		if (token.length() > 4) {
			Warning warn;
			warn.setCode(11);
			instruction.insertWarnList(warn);
			instruction.setOpcode(9);
			instruction.setOprand(999);
		}
		else {
			if (current_addr == "A") {
				int add = stoi(token.substr(1, 3));
				instruction.setOpcode(token.at(0) - '0');

				if (add > MACHINE_SIZE) {
					Warning warn;
					warn.setCode(8);
					instruction.insertWarnList(warn);
					instruction.setOprand(0);
				}
				else {
					if (token.length() == 4) {
						instruction.setOprand(stoi(token.substr(1, 3)));
					}
					else {
						instruction.setOpcode(0);
						instruction.setOprand(stoi(token));
					}
					
				}
			}
			else if (current_addr == "E") {
				int useLen = modList[modIndex].getUseList().size();
				int add;
				int opCode;
				if (token.length() == 4) {
					opCode = token.at(0) - '0';
					add = stoi(token.substr(1, 3));
				}
				else {
					opCode = 0;
					add = stoi(token);
				}
				
				if (add + 1 > useLen) {
					Warning warn;
					warn.setCode(6);
					instruction.insertWarnList(warn);
					instruction.setAddressing("I");
					instruction.setOpcode(opCode);
					instruction.setOprand(add);
				}
				else {
					instruction.setOpcode(opCode);
					int index = add;
					instruction.setOprand(add);
					bool isDefined = false;
					string symName = modList[modIndex].getUseList()[index];
					for (int k = 0; k < definedSymbol.size(); k++) {
						if (definedSymbol[k].getName() == symName) {
							isDefined = true;
							break;
						}
					}
					if (!isDefined) {
						Warning warn;
						warn.setCode(3);
						warn.setSym(symName);
						instruction.insertWarnList(warn);
						Symbol sym;
						sym.setName(symName);
						sym.setVal(0);
						sym.setUsed();
						not_defined_list.push_back(sym);
						//					definedSymbol.push_back(sym);
					}

				}
			}
			else {
				int modLen = modList[modIndex].getLength();
				int add;
				if (token.length() == 4) {
					instruction.setOpcode(token.at(0) - '0');
					add = stoi(token.substr(1, 3));
				}
				else {
					instruction.setOpcode(0);
					add = stoi(token);
				}
				if (add > modLen) {
					Warning warn;
					warn.setCode(9);
					instruction.insertWarnList(warn);
					instruction.setOprand(0);
				}
				else {
					instruction.setOprand(add);

				}
			}
		}
		
	}
	return instruction;
}

vector<Instru> readInstruList2() {
	vector<Instru> result;

	char* token = readToken();
	instru_num = atoi(token);
	

	for (int i = 0; i < instru_num; i++) {
		Instru instruction = readInstru();
		result.push_back(instruction);
	}
	return result;
}



void pass1() {
	file.open(filename);
//	linenum = 1;

//	while (getline(file, readline)) {
//		lineend = false;
//		buf = buf + " " + readline;

	lineend = true;
	linenum = 0;
	while (1) {
		if (lineend) {
			if (!getline(file, readline)) {
				break;
			}
			lineend = false;
			buf = readline;
			linenum++;
			/*
			string t = readToken();	
			if (t.c_str() == NULL) {
				break;
			}
			else {
				buf =  t + " " + buf;
				lineend = false;
			}
			*/
		}

		Module mod;
		mod.setAddress(base_address);

		mod.setDefList(readDefList());
		mod.setUseList(readUseList());
		mod.setInstruList(readInstruList());
		int len = mod.getInstruList().size();
		mod.setLength(len);

		vector<Symbol> defList = mod.getDefList();
		for (int i = 0; i < defList.size(); i++) {
			if (defList[i].getVal()-base_address > len) {
				Warning warn;
				warn.setCode(5);
				warn.setSym(defList[i].getName());
				warn.setMod(modNum);
				warn.setLength(len - 1);
				warn.setVal(defList[i].getVal());
				defList[i].setVal(0);
				warningList.push_back(warn);

				for (int j = 0; j < definedSymbol.size(); j++) {
					if (defList[i].getName() == definedSymbol[j].getName()) {
						definedSymbol[j].setVal(0);
						break;
					}
				}
			}
			
		}
		modList.push_back(mod);
		modNum++;
		base_address += len;
//		linenum++;
	}

	file.close();
	for (int i = 0; i < warningList.size(); i++) {
		cout << warningList[i].getInfo() << endl;
	}
	warningList.clear();
	cout << "Symbol Table" << endl;
	for (int i = 0; i < definedSymbol.size(); i++) {
		cout << definedSymbol[i].getName() << "=" << definedSymbol[i].getVal() << ' ';
		for (int j = 0; j < definedSymbol[i].getWarnList().size(); j++) {
			cout << definedSymbol[i].getWarnList()[0].getInfo();
		}
		cout << endl;
	}
	cout << endl;
}

void pass2() {
	file.open(filename);
	vector<string> fin_used_list;

	while (getline(file, readline)) {
		lineend = false;
		buf = buf + " " + readline;
		readDefList();
		readUseList();
		vector<Instru> instructionList = readInstruList2();
		modList[modIndex].setInstruList(instructionList);
		for (int i = 0; i < instructionList.size(); i++) {
			if (instructionList[i].getAddressing() == "E") {
				int useIndex = instructionList[i].getOprand();
				string usedSymbol = modList[modIndex].getUseList()[useIndex];
				for (int j = 0; j < definedSymbol.size(); j++) {
					if (definedSymbol[j].getName() == usedSymbol) {
						definedSymbol[j].setUsed();
						break;
					}
				}
				fin_used_list.push_back(usedSymbol);
			}
		}
		vector<string> not_used_list = modList[modIndex].getUseList();
		int index = 0;
		while (not_used_list.size() != 0 && index <= not_used_list.size() ) {
			bool isErased = false;
			for (int i = 0; i < fin_used_list.size(); i++) {
				if (not_used_list[index] == fin_used_list[i]) {
					not_used_list.erase(not_used_list.begin()+index);
					isErased = true;
				}
				if (not_used_list.size() == 0) {
					break;
				}
			}
			if (!isErased) {
				index++;
			}
		}
		for (int i = 0; i < not_used_list.size(); i++) {
			Warning warn;
			warn.setCode(7);
			warn.setMod(modIndex+1);
			warn.setSym(not_used_list[i]);
			modList[modIndex].insertWarnList(warn);
		}

		modIndex++;
	}

	for (int i = 0; i < definedSymbol.size(); i++) {
		if (!definedSymbol[i].getUsed()) {
			Warning warn;
			warn.setCode(4);
			warn.setMod(definedSymbol[i].getModuleNum());
			warn.setSym(definedSymbol[i].getName());
			warningList.push_back(warn);
		}
	}

	cout << "Memory Map" << endl;
	base_address = 0;
	for (int i = 0; i < modList.size(); i++) {
		vector<Instru> insList = modList[i].getInstruList();

		for (int j = 0; j < insList.size(); j++) {


			printf("%03d", i + j + base_address);
			cout << ": ";

			string addressing = insList[j].getAddressing();
			if (addressing == "R") {
				if (insList[j].getOprand() == 999) {
					cout << "9999" << ' ';
				}
				else {
					int op = insList[j].getOpcode();
					int opr = insList[j].getOprand() + modList[i].getAddress();
					printf("%04d", op * 1000 + opr);
					cout << ' ';
				}
			}
			else if (addressing == "E") {
				if (insList[j].getOprand() == 999) {
					cout << "9999" << ' ';
				}
				else {
					int op = insList[j].getOpcode();
					int opr = insList[j].getOprand();
					vector<string> uL = modList[i].getUseList();
					string use_symbol = uL[opr];
					int sym_val = 0;
					bool isDefined = false;
					for (int i = 0; i < definedSymbol.size(); i++) {
						if (definedSymbol[i].getName() == use_symbol) {
							sym_val = definedSymbol[i].getVal();
							isDefined = true;
							break;
						}
					}
					if (!isDefined) {
						for (int i = 0; i < not_defined_list.size(); i++) {
							if (not_defined_list[i].getName() == use_symbol) {
								sym_val = 0;
								break;
							}
						}
					}
					printf("%04d", op * 1000 + sym_val);
					cout << ' ';
				}
				
			}
			else {
				int op = insList[j].getOpcode();
				int opr = insList[j].getOprand();
				printf("%04d", op * 1000 + opr);
				cout << ' ';
			}

			vector<Warning> wL = insList[j].getWarnList();
			for (int k = 0; k < wL.size(); k++) {
				cout << wL[k].getInfo();
			}
			cout << endl;

		}

		for (int j = 0; j < modList[i].getWarnList().size(); j++) {
			cout << modList[i].getWarnList()[j].getInfo() << endl;
		}

		base_address += modList[i].getLength() - 1;
	}

	cout << endl;
	for (int i = 0; i < warningList.size(); i++) {
		cout << warningList[i].getInfo() << endl;
	}
	file.close();
}

int main(int argc, char* argv[]) {
	filename = argv[1];
//	filename = "input-20";
	pass1();
	pass2();
//	system("pause");
	return 0;
}
