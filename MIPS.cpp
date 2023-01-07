#include <iostream>
#include <string>
#include <fstream>
using namespace std;

int rs, rt, rd, number, EX_RegDst, EX_ALUSrc, EX_Branch, EX_MemRead, EX_MemWrite, EX_RegWrite, EX_MemtoReg,
MEM_Branch, MEM_MemRead, MEM_MemWrite, MEM_RegWrite, MEM_MemtoReg, MEM_Result, Write , MEM_Dst, 
WB_RegWrite, WB_MemtoReg, WB_Result, Read, WB_Dst;
static int Memory[32] = { 0 }, Register[32] = { 0 }, current_line = 0, max_line, stall_count = 0, cycle_count = 1;
string operation;
char word[5][4];
bool IF_start = true, ID_start = false, EX_start = false, MEM_start = false, WB_start = false, isbeq = false;

#define LW 1
#define SW 2
#define ADD 3
#define SUB 3
#define BEQ 4
#define DOUBLE_DIGITS 1
#define SINGLE 2
#define CHECK 3
int convert_int(int space,int move, int select) {
    switch (select) {
        case DOUBLE_DIGITS: 
            return  10 * (operation[space + move] - '0') + (operation[space + (move + 1)] - '0');
        case SINGLE: 
            return (operation[space + move] - '0');
        case CHECK:
            if (operation[space + move] >= '0' && operation[space + move] <= '9')
                return true;
            else return false;
    }
}

void WB() {
    if (word[4][0] != '0') {
        fstream output;
        output.open("result.txt", ios::app);
        output << "	" << word[4] << ":WB ";
        output << WB_RegWrite;
        if (WB_MemtoReg == 2) output << 'X' << endl;
        else output << WB_MemtoReg << endl;
        //reset
        for (int i = 0; i < 4; i++)
            word[4][i] = '0';
        output.close();
    }

    //Write lw
    if (WB_RegWrite == 1 && WB_MemtoReg == 1)
        Register[WB_Dst] = Read;
    //Write R-format(add, sub)
    else if (WB_RegWrite == 1 && WB_MemtoReg == 0)
        Register[WB_Dst] = WB_Result;
    WB_RegWrite = 0; WB_MemtoReg = 0;
    WB_start = false;
}

void MEM() {
    if (word[3][0] != '0') {
        fstream output;
        output.open("result.txt", ios::app);
        output << "	" << word[3] << ":MEM ";
        output << MEM_Branch << MEM_MemRead << MEM_MemWrite << " " << MEM_RegWrite;
        if (MEM_MemtoReg == 2) output << 'X' << endl;
        else output << MEM_MemtoReg << endl;

        for (int i = 0; i < 4; i++) {
            word[4][i] = word[3][i];
            word[3][i] = '0';
        }
        output.close();
    }
    //MEM_MemRead is 1, is lw
    if (MEM_MemRead == 1)
        Read = Memory[MEM_Result];
    //MEM_MemWrite is 1, is sw
    if (MEM_MemWrite == 1)
        Memory[MEM_Result] = Write;

    //Transfer
    WB_MemtoReg = MEM_MemtoReg;
    WB_Result = MEM_Result;
    WB_Dst = MEM_Dst;
    WB_RegWrite = MEM_RegWrite;
    MEM_Branch = 0; MEM_MemRead = 0, MEM_MemWrite = 0;
    MEM_RegWrite = 0; MEM_MemtoReg = 0;
    MEM_start = false; WB_start = true;
}

void EX() {
    if (word[2][0] != '0') {
        fstream output;
        output.open("result.txt", ios::app);
        output << "	" << word[2] << ":EX ";
        if (EX_RegDst == 2) output << "X";
        else output << EX_RegDst;
        output << EX_ALUSrc << " " << EX_Branch << EX_MemRead << EX_MemWrite << " " << EX_RegWrite;
        if (EX_MemtoReg == 2) output << 'X' << endl;
        else output << EX_MemtoReg << endl;

        for (int i = 0; i < 4; i++) {
            word[3][i] = word[2][i];
            word[2][i] = '0';
        }
        output.close();
    }
    if (EX_ALUSrc == 0) //R-format and beq
        MEM_Result = rt + rs;
    else if (EX_ALUSrc == 1) //I-format
        MEM_Result = rs + number;

    if (EX_Branch == 1 && MEM_Result == 0) {
        isbeq = true;
    }


    MEM_Dst = rd;
    Write = rt;
    MEM_MemWrite = EX_MemWrite;
    MEM_MemtoReg = EX_MemtoReg;
    MEM_RegWrite = EX_RegWrite;
    MEM_MemRead = EX_MemRead;
    MEM_Branch = EX_Branch;

    EX_start = false; MEM_start = true;
}

void ID() {
    if (stall_count > 0) {
        ID_start = true;
        if (word[1][0] != '0') {
            fstream output;
            output.open("result.txt", ios::app);
            output << "	" << word[1] << ":ID" << endl;
            output.close();
        }
        return;
    }

    if (isbeq) {
        word[1][0] = '0';
        isbeq = false;
        rs = 0;
        rt = 1;
    }
    else {
        int reg_rs, reg_rt;
        int blank = 0;
        int space[3];
        int function = 0;

        for (int i = 0; i < operation.length(); i++)
            if (operation[i] == ' ') {
                space[blank] = i;
                blank++;
            }

        if (blank == 2) {
            if (operation[0] == 'l') {  //lw
                EX_RegDst = 0, EX_ALUSrc = 1, EX_MemtoReg = 1, EX_RegWrite = 1, EX_MemRead = 1; EX_MemWrite = 0, EX_Branch = 0;
                function = LW;
                if (convert_int(space[0], 3, CHECK)) //check double digit
                    rd = convert_int(space[0], 2, DOUBLE_DIGITS);
                else if (convert_int(space[0], 2, CHECK)) //check single digit
                    rd = convert_int(space[0], 2, SINGLE);
            }
            else if (operation[0] == 's') { //sw 
                EX_RegDst = 2, EX_ALUSrc = 1, EX_MemtoReg = 2, EX_RegWrite = 0, EX_MemRead = 0; EX_MemWrite = 1, EX_Branch = 0;
                function = SW;
                if (convert_int(space[0], 3, CHECK)) {
                    reg_rt = convert_int(space[0], 2, DOUBLE_DIGITS);
                    rt = Register[reg_rt];
                }
                else if (convert_int(space[0], 2, CHECK)) {
                    reg_rt = convert_int(space[0], 2, SINGLE);
                    rt = Register[reg_rt];
                }
            }

            if (convert_int(space[1], 2, CHECK)) {
                number = convert_int(space[1], 1, DOUBLE_DIGITS) / 4;
                if (convert_int(space[1], 6, CHECK)) {
                    reg_rs = convert_int(space[1], 5, DOUBLE_DIGITS);
                    rs = Register[reg_rs];
                }
                else if (convert_int(space[1], 5, CHECK)) {
                    reg_rs = convert_int(space[1], 5, SINGLE);
                    rs = Register[reg_rs];
                }
            }
            else if (convert_int(space[1], 1, CHECK)) {
                number = convert_int(space[1], 1, SINGLE) / 4;
                if (convert_int(space[1], 5, CHECK)) {
                    reg_rs = convert_int(space[1], 4, DOUBLE_DIGITS);
                    rs = Register[reg_rs];
                }
                else if (convert_int(space[1], 4, CHECK)) {
                    reg_rs = convert_int(space[1], 4, SINGLE);
                    rs = Register[reg_rs];
                }
            }
        }

        else if (blank == 3) {
            // add
            if (operation[0] == 'a') {
                EX_RegDst = 1, EX_ALUSrc = 0, EX_MemtoReg = 0, EX_RegWrite = 1, EX_MemRead = 0; EX_MemWrite = 0, EX_Branch = 0;
                function = ADD;
                if (convert_int(space[0], 3, CHECK))
                    rd = convert_int(space[0], 2, DOUBLE_DIGITS);
                else if (convert_int(space[0], 2, CHECK))
                    rd = convert_int(space[0], 2, SINGLE);
                if (convert_int(space[1], 3, CHECK)) {
                    reg_rs = convert_int(space[1], 2, DOUBLE_DIGITS);
                    rs = Register[reg_rs];
                }
                else if (convert_int(space[1], 2, CHECK)) {
                    reg_rs = convert_int(space[1], 2, SINGLE);
                    rs = Register[reg_rs];
                }
                if (operation[space[2] + 1] == '$') {
                    if (convert_int(space[2], 3, CHECK)) {
                        reg_rt = convert_int(space[2], 2, DOUBLE_DIGITS);
                        rt = Register[reg_rt];
                    }
                    else if (convert_int(space[2], 2, CHECK)) {
                        reg_rt = convert_int(space[2], 2, SINGLE);
                        rt = Register[reg_rt];
                    }
                }
                //decimal number
                else {
                    number = operation[operation.length() - 1] - '0';
                    if (operation[space[2] + 1] == '-')
                        number = -number;
                    rt = number;
                }
            }
            // sub
            if (operation[0] == 's') {
                EX_RegDst = 1, EX_ALUSrc = 0, EX_MemtoReg = 0, EX_RegWrite = 1, EX_MemRead = 0; EX_MemWrite = 0, EX_Branch = 0;
                function = SUB;
                if (convert_int(space[0], 3, CHECK))
                    rd = convert_int(space[0], 2, DOUBLE_DIGITS);
                else if (convert_int(space[0], 2, CHECK))
                    rd = convert_int(space[0], 2, SINGLE);
                if (convert_int(space[1], 3, CHECK)) {
                    reg_rs = convert_int(space[1], 2, DOUBLE_DIGITS);
                    rs = Register[reg_rs];
                }
                else if (convert_int(space[1], 2, CHECK)) {
                    reg_rs = convert_int(space[1], 2, SINGLE);
                    rs = Register[reg_rs];
                }
                if (operation[space[2] + 1] == '$') {
                    if (convert_int(space[2], 3, CHECK)) {
                        reg_rt = convert_int(space[2], 2, DOUBLE_DIGITS);
                        rt = -Register[reg_rt];
                    }
                    else if (convert_int(space[2], 2, CHECK)) {
                        reg_rt = convert_int(space[2], 2, SINGLE);
                        rt = -Register[reg_rt];
                    }
                }
                //decimal number
                else {
                    number = -(operation[operation.length() - 1] - '0');
                    if (operation[space[2] + 1] == '-')
                        number = -(number);
                    rt = number;
                }
            }
            // beq
            else if (operation[0] == 'b') {
                EX_RegDst = 2, EX_ALUSrc = 0, EX_MemtoReg = 2, EX_RegWrite = 0, EX_MemRead = 0; EX_MemWrite = 0, EX_Branch = 1;
                function = BEQ;
                if (convert_int(space[0], 3, CHECK)) {
                    reg_rs = convert_int(space[0], 2, DOUBLE_DIGITS);
                    rs = Register[reg_rs];
                }
                else if (convert_int(space[0], 2, CHECK)) {
                    reg_rs = convert_int(space[0], 2, SINGLE);
                    rs = Register[reg_rs];
                }

                if (convert_int(space[1], 3, CHECK)) {
                    reg_rt = convert_int(space[1], 2, DOUBLE_DIGITS);
                    rt = -Register[reg_rt];
                }
                else if (convert_int(space[1], 2, CHECK)) {
                    reg_rt = convert_int(space[1], 2, SINGLE);
                    rt = -Register[reg_rt];
                }
            }
        }
        //STALL
        if (function == LW) {
            reg_rs = rs + number;
            if (MEM_Dst == reg_rs && MEM_RegWrite == 1) {
                stall_count = 2;
                EX_RegDst = 0, EX_ALUSrc = 0, EX_Branch = 0, EX_MemRead = 0, EX_MemWrite = 0, EX_RegWrite = 0, EX_MemtoReg = 0;
            }
        }
        else if (function == SW) {
            if (MEM_Dst == reg_rt && MEM_RegWrite == 1) {
                stall_count = 2;
                EX_RegDst = 0, EX_ALUSrc = 0, EX_Branch = 0; EX_MemRead = 0, EX_MemWrite = 0, EX_RegWrite = 0, EX_MemtoReg = 0;
            }
        }
        else if (function == ADD) {
            if ((MEM_Dst == reg_rs || MEM_Dst == reg_rt) && MEM_RegWrite == 1) {
                stall_count = 2;
                EX_RegDst = 0, EX_ALUSrc = 0, EX_Branch = 0, EX_MemRead = 0, EX_MemWrite = 0, EX_RegWrite = 0, EX_MemtoReg = 0;
            }
        }
        else if (function == BEQ) {
            if ((MEM_Dst == reg_rs || MEM_Dst == reg_rt) && MEM_RegWrite == 1) {
                stall_count = 2;
                EX_RegDst = 0, EX_ALUSrc = 0, EX_Branch = 0, EX_MemRead = 0, EX_MemWrite = 0, EX_RegWrite = 0, EX_MemtoReg = 0;
            }
        }
    }

    if (word[1][0] != '0') {
        fstream output;
        output.open("result.txt", ios::app);
        output << "	" << word[1] << ":ID" << endl;
        output.close();
    }
    if (stall_count != 2)
        for (int i = 0; i < 4; i++) {
            word[2][i] = word[1][i];
            word[1][i] = '0';
        }
    ID_start = false; EX_start = true;
}

void IF() {
    bool last_line = false;
    string prev = operation;
    fstream input;
    input.open("memory.txt");
    int i = 0;
    // skip line
    for (i = 0; i < current_line; i++)
        getline(input, operation);

    getline(input, operation);
    // when it is the current line
    if (i == current_line) {
        for (int i = 0; i < 3; i++)
            word[0][i] = operation[i];
        word[0][3] = '\0';
        fstream output;
        output.open("result.txt", ios::app);
        output << "	" << word[0] << ":IF" << endl;
        output.close();
        if (stall_count > 0) {
            ID_start = true;
            IF_start = true;
            operation = prev;
            return;
        }
        //move the word to ID
        for (int i = 0; i < 4; i++) {
            word[1][i] = word[0][i];
            word[0][i] = '0';
        }
        current_line++;
    }
    input.close();
    IF_start = false; ID_start = true;
}

int main() {
    //Set all value
    //Set the value of 32 registers to 1
    for (int i = 0; i < 32; ++i) {
        Memory[i] = 1;
        Register[i] = 1;
    }
    Register[0] = 0;
    //EX
    EX_RegDst = 0, EX_Branch = 0, EX_RegWrite = 0,
        EX_ALUSrc = 0, EX_MemRead = 0, EX_MemWrite = 0, EX_MemtoReg = 0, rs = 0, rt = 0, rd = 0, number = 0;
    //MEM
    MEM_Branch = 0, MEM_RegWrite = 0, MEM_MemRead = 0, MEM_MemWrite = 0,
        MEM_MemtoReg = 0, MEM_Result = 0, Write = 0, MEM_Dst = 0;
    //WB
    WB_RegWrite = 0, WB_MemtoReg = 0, WB_Result = 0, Read = 0, WB_Dst = 0;

    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 4; j++)
            word[i][j] = '0';


    fstream open_file;
    open_file.open("memory.txt");
    while (getline(open_file, operation))
        max_line++;
    open_file.close();

    fstream output;
    output.open("result.txt", ios::out);
    output.close();

    do {
        fstream output;
        output.open("result.txt", ios::app);
        output << "Cycle " << cycle_count << endl;
        output.close();
        if (WB_start)
            WB();
        if (MEM_start)
            MEM();
        if (EX_start)
            EX();
        if (ID_start || stall_count > 0)
            ID();
        if (current_line == max_line)
            IF_start = false;
        else 
            IF();

        cycle_count++;
        stall_count--;
    } while (IF_start || ID_start || EX_start || MEM_start || WB_start);

    output.open("result.txt", ios::app);
    output << endl << "It needs " << cycle_count - 1 << " Cycles" << endl;
    for (int i = 0; i < 32; i++)
        output << "$" << i << "\t";
    output << endl;
    for (int i = 0; i < 32; i++)
        output << Register[i] << "\t";
    output << endl;
    for (int i = 0; i < 32; i++)
        output << "W" << i << "\t";
    output << endl;
    for (int i = 0; i < 32; i++)
        output << Memory[i] << "\t";
    output << endl;
    output.close();
    return 0;
}
