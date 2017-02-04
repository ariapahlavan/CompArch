#include <stdio.h>      /* standard input/output library */
#include <stdlib.h>     /* Standard C Library */
#include <string.h>     /* String operations library */
#include <ctype.h>      /* Library for useful character operations */
#include <limits.h>     /* Library for definitions of common variable type characteristics */
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>


/**----------------------------------- Definitions ------------------------------------*/
#define MAX_NUM_OPCODES 31
#define MAX_LINE_LENGTH 255
#define MAX_LABEL_LEN 20
#define MAX_SYMBOLS 255
#define MAX_REGISTERS 8
#define MAX_OFFSET6 31
#define MIN_OFFSET6 -32
#define FILL 28
#define ORIG 29
#define END 30
#define EMPTY_VAL 0
#define INVALID_LABEL_LENGTH 4
#define LOG_LEVEL INFO
#define MAX_ADDRESS USHRT_MAX
#define pc(lc) lc+2
#define caller(arg) __FUNCTION__, __LINE__, arg
#define debug __FUNCTION__, __LINE__, DEBUG
#define info __FUNCTION__, __LINE__, INFO
#define warn __FUNCTION__, __LINE__, WARN
#define error __FUNCTION__, __LINE__, ERROR
#define nibble 4
#define adjOpcode(val) val << 3*nibble
#define adjArgOne(val) val << 2*nibble+1
#define adjArgTwo(val) val << nibble+2

#define ANRM  "\x1B[0m"
#define ARED  "\x1B[31m"
#define AGRN  "\x1B[32m"
#define AYEL  "\x1B[33m"
#define ABLU  "\x1B[34m"
#define AMAG  "\x1B[35m"
#define ACYN  "\x1B[36m"
#define AWHT  "\x1B[37m"


/**-------------------------------- Structures & Enums --------------------------------*/
enum {
    DONE, OK, EMPTY_LINE
};
enum ASM_PHASE {
    FIRST_PASS, SEC_PASS
};
enum DATA_TYPE {
    INSTRUCTION,
    PTR,
    CHAR,
    INT_8,
    INT_16,
    INT_32,
    INT_64,
    UINT_8,
    UINT_16,
    UINT_32,
    UINT_64,
    STR,
    INT,
    LONG,
    LONG_INT,
    ULONG,
    ULONG_INT,
    S,
    I,
    L,
    C,
    P,
    I8,
    I16,
    I32,
    I64,
    UI,
    UI8,
    UI16,
    UI32,
    UI64,
    UL,
    ULI,
    LI,
    N,
    ENDL,
    T,
    TAB
};
enum LOG_LEVELS {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    NONE
};
enum OPCODES {
    add, and, br, brn, brp, brnp, brz, brnz, brzp, brnzp, jmp, ret, jsr, jsrr, ldb,
    ldw, lea, not, rti, lshf, rshfl, rshfa, stb, stw, trap, xor, halt, nop, fill, orig, end
};
enum REGISTERS {
    r0, r1, r2, r3, r4, r5, r6, r7
};

typedef struct {
    uint16_t address;
    char label[MAX_LABEL_LEN + 1];
} TableEntry;
typedef struct INSTRUCTION {
    char *label;
    char *opcode;
    char *arg1;
    char *arg2;
    char *arg3;
    char *arg4;

    enum OPCODES enumOpcode;
    int16_t decodResult;

    bool hasLabel;
    bool isPseudoOpe;
} Instruction;

/**--------------------------------- Global Variables ---------------------------------*/
TableEntry symbolTable[MAX_SYMBOLS];
static FILE *outfile;
static int symbolTableLength = 0;
uint16_t locationCntr = -1;
static int logNum = 0;
const static int trapVectorVal[] = {0x20, 0x21, 0x22, 0x23, 0x25};
const static char *trapVectorTable[] = {"x20", "x21", "x22", "x23", "x25"};
const static char *enumStrings[] = {"DEBUG", "INFO", "WARN", "ERROR"};
const static char *pseudoOps[] = {".orig", ".end", ".fill"};
const static char *invalidLabels[] = {"in", "out", "getc", "puts"};
const static char *registers[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};

const static char *lowerOpcodes[] = {"add", "and", "br", "brn", "brp", "brnp", "brz", "brnz", "brzp",
                                     "brnzp", "jmp", "ret", "jsr", "jsrr", "ldb", "ldw", "lea", "not",
                                     "rti", "lshf", "rshfl", "rshfa", "stb", "stw", "trap", "xor", "halt", "nop",
                                     ".fill", ".orig", ".end"};

/*const static int opcodes[] = {0001, 0101, 0000111, 0000100, 0000001, 0000101, 0000010, 0000110,
                              0000011, 0000111, 1100000, 1100000111000000, 01001, 0100000, 0010,
                              0110, 1110, 1001, 1000000000000000, 1101, 1101, 1101,
                              0011, 0111, 11110000, 1001, 1111000000100101, 0000000000000000};*/
const static int opcodes[] = {1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 12, 12, 4, 4, 2,
                              6, 14, 9, 8, 13, 13, 13, 3, 7, 15, 9, 0xF025, 0, 0, 0, 0};

const enum OPCODES opcodeEnum[] = {add, and, br, brn, brp, brnp, brz, brnz, brzp, brnzp,
                                   jmp, ret, jsr, jsrr, ldb, ldw, lea, not, rti, lshf,
                                   rshfl, rshfa, stb, stw, trap, xor, halt, nop,
                                   fill, orig, end};


/**-------------------------------- Function Declarations -----------------------------*/
int isOpcode(char *potential_opcode);

void parsCommandLine(int argc, char *argv[]);

void testIsOpcode();

void testLimits();

void loggingMsg(const char *func, int line, enum LOG_LEVELS lvl, const char *msg);

enum OPCODES isPsuedoup(const char *opcode);

enum OPCODES getOpcodeEnum(Instruction *inst);

void logging(const char *func, int line, enum LOG_LEVELS lvl, int num, ...);

void print(int num, ...);

void println(int num, ...);

void printList(int num, va_list valist);

void outputDouble(double value);

void output(void *V, enum DATA_TYPE Type);

void colorPrint(enum LOG_LEVELS lvl, const char *txt);

int getLogNum();

void testOpenCloseFile(int argc, char *argv[], FILE *infile, FILE *outfile);

void closeFiles(FILE *file);

FILE *openFile(char *filename, char *permission);

void testToNum();

void writeToFile(FILE *file, const char *text);

void testWriteToFile(const FILE *infile);

Instruction *newInstruction();

void freeInstruction(Instruction *instruction);

int parseFile(const FILE *infile, Instruction *i, enum ASM_PHASE passNum);

int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode,
                 char **pArg1, char **pArg2, char **pArg3, char **pArg4);

void addLebel(char *label, int lc);

int getTableSize();

void increaseTabelSize();

void errorOpenFile(const char *filename);

void errorCloseFile();

void errorIncreaseTableSize();

void errorAddLabel(const char *dupLabel);

void errorCopyLabel(char *labelCpy, char *label);

void testSymbolTabel();

int toNum(char *pStr);

void validateLabel(char *label);

void errorValidateLabel(char *label);

int isAlnumeric(char *label);

int isTrapLabel(const char *label);

void toString(const Instruction *i);

void tryAddLabel(const Instruction *i);

int incrementLocationCntr();

void decodeInstruction(Instruction *i);

void printAll();

uint16_t initLocationCntr(const Instruction *i);

void validateAddrStr(char *addr);

void errorValidateAddr(int addr);

void validateFillVal(char *val);

void errorGetOpcodeEnum(const char *opcode);

void errorUnauthorizedInstruction(Instruction *i);

int16_t processArg(Instruction *i, enum OPCODES opcode, int argNum);

int16_t validateRegister(const char *reg, bool isHard);


void errorValidateRegister(const char *reg);


void processPseudoOpe(Instruction *i);

int16_t outputFill(const char *arg);

void writeIntToFile(FILE *file, uint16_t val);

void validateArgNum(Instruction *i);

bool isEmptyArg(const char *arg);

void errorValidArgNum(Instruction *i, int numExtraArgs);

void outputDecodeResults(Instruction *i, enum OPCODES opcode, uint16_t output);

void errorLCoutOfBound();

void errorPCoutOfBound();

uint16_t getLabelAddr(const char *label);

void errorLabelNotFound(const char *label);

void errorOffset6OutOfBound(int16_t arg);

void errorAmnt4outOfBound(Instruction *i);

int validateTrapArg(const char *arg);

void errorTrapVector(const char *arg);

void errorSpecOutOfBound(Instruction *i);

void errorNegTrapVector(const char *arg);

/**-------------------------------- Function Definitions ------------------------------*/
int main(int argc, char *argv[]) {
    loggingMsg(debug, "Initializing...");

    /* open input/output files*/
    FILE *infile = openFile(argv[1], "r");
    outfile = openFile(argv[2], "w");

    Instruction i;

    parseFile(infile, &i, FIRST_PASS);
    testSymbolTabel();
    rewind(infile);
    parseFile(infile, &i, SEC_PASS);

    /* close input/output files*/
    closeFiles(infile);
    closeFiles(outfile);
    exit(0);
}

void testSymbolTabel() {
    int i;

    for (i = 0; i < getTableSize(); ++i) {
        logging(debug, 6, S, "'", S, symbolTable[i].label, S, "' at index ", I, i, S, " with address: ", I,
                symbolTable[i].address);
    }
}

int parseFile(const FILE *infile, Instruction *i, enum ASM_PHASE passNum) {
    bool isPassedOrig = false, isPassedEnd = false;

    char lLine[MAX_LINE_LENGTH + 1];
    int lRet, instructionCounter = 0;
    uint16_t lc = 0;

    do {
        if (isPassedEnd) break;

        lRet = readAndParse(infile, lLine, &i->label, &i->opcode,
                            &i->arg1, &i->arg2, &i->arg3, &i->arg4);

        if (lRet != DONE && lRet != EMPTY_LINE) {/* instruction */
            instructionCounter++;
            logging(debug, 1, INSTRUCTION, i);

            if (passNum == FIRST_PASS) tryAddLabel(i);
            if (strcmp(i->opcode, ".end") == 0) isPassedEnd = true;
            if (passNum == SEC_PASS && isPassedOrig) decodeInstruction(i);

            if (isPassedOrig) lc = incrementLocationCntr(); /* increment PC and LC */

            if (strcmp(i->opcode, ".orig") == 0) {
                isPassedOrig = true;
                if (passNum == SEC_PASS) decodeInstruction(i);
                lc = initLocationCntr(i);

            } else if (!isPassedOrig) errorUnauthorizedInstruction(i);

        }

    } while (lRet != DONE);

    logging(debug, 2, S, "Number of instruction in the file=", I, instructionCounter);

    return instructionCounter;
}

void errorUnauthorizedInstruction(Instruction *i) {
    loggingMsg(error, "Found instruction before .ORIG");
    exit(4);
}

uint16_t initLocationCntr(const Instruction *i) {
    validateAddrStr(i->arg1);
    locationCntr = toNum(i->arg1);
    logging(debug, 2, S, "Initialized LC to ", I, locationCntr);
    return locationCntr;
}

uint16_t getLocationCntr() {
    if (locationCntr < 0 || locationCntr > USHRT_MAX) errorLCoutOfBound();
    return locationCntr;
}

uint16_t getProgramCntr() {
    if (locationCntr < 0 || locationCntr > USHRT_MAX) errorPCoutOfBound();
    return pc(locationCntr);
}

void errorPCoutOfBound() {
    /*TODO what happens if pc is out of bound?*/
    logging(error, 2, S, "Program counter is out of bound: ", I, pc(locationCntr));
    exit(4);
}

void errorLCoutOfBound() {
    logging(error, 2, S, "Location counter is out of bound: ", I, locationCntr);
    exit(4);
}

void validateAddrStr(char *addr) {
    int address = toNum(addr);

    if (address < 0 || address > MAX_ADDRESS)
        errorValidateAddr(address);
}

void validateFillVal(char *val) {
    int result = toNum(val);

    if (result < 0 || result > USHRT_MAX)
        errorValidateAddr(result);
}

void errorValidateAddr(int addr) {
    logging(error, 2, S, "Found an invalid address=", I, addr);
    exit(4);
}

void decodeInstruction(Instruction *i) {
    uint16_t output = 0, mask1 = 0x0E00;

    enum OPCODES opcode = getOpcodeEnum(i);

    output = adjOpcode(opcodes[opcode]);
    logging(debug, 2, I, output, INSTRUCTION, i);

    validateArgNum(i);

    uint16_t arg1 = processArg(i, opcode, 1);
    uint16_t o = output + arg1;
    logging(debug, 6, I, arg1, S, " + ", I, output, S, " = ", I, o, INSTRUCTION, i);
    output = o;


    uint16_t arg2 = processArg(i, opcode, 2);
    o = output + arg2;
    logging(debug, 6, I, arg2, S, " + ", I, output, S, " = ", I, o, INSTRUCTION, i);
    output = o;

    outputDecodeResults(i, opcode, output);
}

void outputDecodeResults(Instruction *i, enum OPCODES opcode, uint16_t output) {
    uint8_t reg;
    int16_t imm5mask = 0x001F;
    int16_t pcoff9mask = 0x01FF;
    int16_t pcoff11mask = 0x07FF;
    int16_t offset6mask = 0x003F;
    int16_t amount4mask = 0x000F;


    switch (opcode) {
        case add:
        case and:
        case xor:
            reg = validateRegister(i->arg3, false);
            if (validateRegister(i->arg3, false) != -1) { /*register arg*/
                output += reg;
                writeIntToFile(outfile, output);
                logging(debug, 2, S, "ADD result is: ", I, output);
            } else { /*immediate value arg*/
                int arg = (toNum(i->arg3) & imm5mask);

                if (arg < -16 || arg > 15) errorSpecOutOfBound(i);

                output += (arg + 0x0020);
                writeIntToFile(outfile, output);
                logging(debug, 2, S, "ADD result is: ", I, output);
            }

            break;
        case ldb:
        case ldw:
        case stb:
        case stw:
            if (true) {
                int16_t arg = toNum(i->arg3);
                if (arg > MAX_OFFSET6 || arg < MIN_OFFSET6) errorOffset6OutOfBound(arg);
                output += arg & offset6mask;
                writeIntToFile(outfile, output);
                logging(debug, 2, S, "LDB result is: ", I, output);
            }
            break;
        case not:
            if (true) {
                int16_t rest = 0x003F;
                output += rest;
                writeIntToFile(outfile, output);
                logging(debug, 2, S, "NOT result is: ", I, output);
            }
            break;
        case brn:
        case brp:
        case brnp:
        case brz:
        case brnz:
        case brzp:
        case br:
        case brnzp:
        case lea:
            if (true) {
                const char *label;
                uint16_t pcoff9, addr;


                if (i->enumOpcode == lea) label = i->arg2;
                else label = i->arg1;

                addr = getLabelAddr(label);
                pcoff9 = (((int) (addr - getProgramCntr()) >> 1) & pcoff9mask);
                output += pcoff9;
                writeIntToFile(outfile, output);
                logging(debug, 2, S, "BR or LEA result is: ", I, output);
            }
            break;
        case jmp:
        case jsrr:
            writeIntToFile(outfile, output);
            logging(debug, 2, S, "JMP or JSRR result is: ", I, output);
            break;
        case jsr:
            if (true) {
                uint16_t pcoff11;
                int16_t addr = getLabelAddr(i->arg1);

                pcoff11 = (((int) (addr - getProgramCntr()) >> 1) & pcoff9mask);
                output += (pcoff11 + 0x0800);
                writeIntToFile(outfile, output);
                logging(debug, 2, S, "JSR result is: ", I, output);
            }
            break;
        case lshf:
        case rshfa:
        case rshfl:
            if (true) {
                int16_t val = toNum(i->arg3);
                if (val < 0 || val > 15) errorAmnt4outOfBound(i);

                output += (val & amount4mask);

                if (opcode == rshfl) output += 0x0010;
                if (opcode == rshfa) output += 0x0030;

                writeIntToFile(outfile, output);
                logging(debug, 2, S, "SHFT result is: ", I, output);
            }
            break;
        case trap:
            output += trapVectorVal[validateTrapArg(i->arg1)];
            writeIntToFile(outfile, output);
            logging(debug, 2, S, "TRAP result is: ", I, output);
            break;
    }
}

void errorSpecOutOfBound(Instruction *i) {
    logging(error, 2, S, "Found out of boound areguement in: ", INSTRUCTION, i);
    exit(4);
}

int validateTrapArg(const char *arg) {
    int i;

    if (toNum(arg) < 0) errorNegTrapVector(arg);

    for (i = 0; i < 5; ++i)
        if (strcmp(arg, trapVectorTable[i]) == 0) return i;

    errorTrapVector(arg);
}

void errorNegTrapVector(const char *arg) {
    logging(error, 2, S, "Negative trap vector: ", S, arg);
    exit(3);
}

void errorTrapVector(const char *arg) {
    logging(error, 2, S, "Invalid trap vector: ", S, arg);
    exit(4);
}

void errorAmnt4outOfBound(Instruction *i) {
    logging(error, 2, S, "Amount 4 outof bound for: ", INSTRUCTION, i);
    exit(3);
}

void errorOffset6OutOfBound(int16_t arg) {
    logging(error, 2, S, "Could not find label: ", I16, arg);
    exit(4);
}

uint16_t getLabelAddr(const char *label) {
    logging(debug, 2, S, "Getting label: ", S, label);
    int i;

    for (i = 0; i < getTableSize(); ++i)
        if (strcmp(label, symbolTable[i].label) == 0)
            return symbolTable[i].address;

    errorLabelNotFound(label);
}

void errorLabelNotFound(const char *label) {
    logging(error, 3, S, "Could not find label: '", S, label, S, "'");
    exit(4);
}

void validateArgNum(Instruction *i) {
    switch (i->enumOpcode) {
        /*3 args*/
        case add:
        case and:
        case ldb:
        case ldw:
        case lshf:
        case rshfl:
        case rshfa:
        case stb:
        case stw:
        case xor:
            if (!isEmptyArg(i->arg4)) errorValidArgNum(i, 1);
            break;

            /*2 args*/
        case not:
        case lea:
            if (!isEmptyArg(i->arg3)) errorValidArgNum(i, 1);
            if (!isEmptyArg(i->arg4)) errorValidArgNum(i, 2);
            break;

            /*1 arg*/
        case brn:
        case brp:
        case brnp:
        case brz:
        case brnz:
        case brzp:
        case br:
        case brnzp:
        case jmp:
        case jsrr:
        case jsr:
        case trap:
        case fill:
        case orig:
            if (!isEmptyArg(i->arg2)) errorValidArgNum(i, 1);
            if (!isEmptyArg(i->arg3)) errorValidArgNum(i, 2);
            if (!isEmptyArg(i->arg4)) errorValidArgNum(i, 3);
            break;

            /*0 args*/
        case ret:
        case rti:
        case halt:
        case nop:
        case end:
            if (!isEmptyArg(i->arg1)) errorValidArgNum(i, 1);
            if (!isEmptyArg(i->arg2)) errorValidArgNum(i, 2);
            if (!isEmptyArg(i->arg3)) errorValidArgNum(i, 3);
            if (!isEmptyArg(i->arg4)) errorValidArgNum(i, 4);
            break;
    }
}

void errorValidArgNum(Instruction *i, int numExtraArgs) {
    logging(error, 4, S, "Found at least ", I, numExtraArgs, S, " extra args in: ", INSTRUCTION, i);
    exit(4);
}

bool isEmptyArg(const char *arg) {
    if (strcmp(arg, "") != 0)
        return false;
    else
        return true;
}

int16_t outputFill(const char *arg) {
    validateFillVal(arg);
    writeIntToFile(outfile, toNum(arg));

    return 0;
}

int16_t processArg(Instruction *i, enum OPCODES opcode, int argNum) {
    /*logging();*/
    uint16_t result = 0;
    uint8_t reg = 0;
    const char *arg;

    if (argNum == 1) arg = i->arg1;
    else arg = i->arg2;

    switch (opcode) {
        case add:
        case and:
        case ldb:
        case ldw:
        case not:
        case lshf:
        case rshfl:
        case rshfa:
        case stb:
        case stw:
        case xor:
            result = reg = validateRegister(arg, true);
            if (argNum == 1) result = adjArgOne(result);
            else result = adjArgTwo(result);
            logging(debug, 7, S, arg, S, " >> R", I, reg, S, ": ", I, result, S, " as arg #", I, argNum);
            break;
        case brn:
            if (argNum == 1) result = adjArgOne(4);
            break;
        case brp:
            if (argNum == 1) result = adjArgOne(1);
            break;
        case brnp:
            if (argNum == 1) result = adjArgOne(5);
            break;
        case brz:
            if (argNum == 1) result = adjArgOne(2);
            break;
        case brnz:
            if (argNum == 1) result = adjArgOne(6);
            break;
        case brzp:
            if (argNum == 1) result = adjArgOne(3);
            break;
        case br:
        case brnzp:
            if (argNum == 1) result = adjArgOne(7);
            break;
        case rti:
            if (argNum == 1) writeToFile(outfile, "0x8000\n");
            break;
        case ret:
            if (argNum == 1) writeToFile(outfile, "0xC1C0\n");
            break;
        case jmp:
        case jsrr:
            if (argNum == 1) result = adjArgOne(000);
            if (argNum == 2) {
                result = reg = validateRegister(i->arg1, true);
                result = adjArgTwo(result);
                logging(debug, 7, S, arg, S, " >> R", I, reg, S, ": ", I, result, S, " as arg #", I, argNum);
            }
            break;
        case jsr:
            break;
        case lea:
            if (argNum == 1) {
                result = reg = validateRegister(arg, true);
                result = adjArgOne(result);
            }
            break;
        case trap:
            if (argNum == 1) result = adjArgOne(000);
            break;
        case halt:
            if (argNum == 1) writeIntToFile(outfile, opcodes[halt]);
            break;
        case nop:
            if (argNum == 1) writeIntToFile(outfile, adjOpcode(opcodes[nop]));
            break;
        case fill:
        case orig:
            if (argNum == 1) result = outputFill(i->arg1);
            logging(debug, 2, S, "Fill current address with ", I, toNum(i->arg1));
        case end:
            break;
    }
    return result;
}

int16_t validateRegister(const char *reg, bool isHard) {
    int i, result = -1;

    for (i = 0; i < MAX_REGISTERS; ++i)
        if (strcmp(reg, registers[i]) == 0) {
            result = i;
            break;
        }

    if (result == -1 && isHard) errorValidateRegister(reg);

    return result;
}

void errorValidateRegister(const char *reg) {
    logging(error, 3, S, "Found an invalid register: '", S, reg, S, "'");
    exit(4);
}

enum OPCODES getOpcodeEnum(Instruction *inst) {
    int j;
    const char *opcode = inst->opcode;

    enum OPCODES result = isPsuedoup(opcode);

    if (result != -1) inst->isPseudoOpe = true;
    else inst->isPseudoOpe = false;

    if (result == -1)
        for (j = 0; j < MAX_NUM_OPCODES; ++j)
            if (strcmp(opcode, lowerOpcodes[j]) == 0)
                result = opcodeEnum[j];

    if (result == -1) errorGetOpcodeEnum(opcode);

    inst->enumOpcode = result;

    logging(debug, 5,
            S, opcode, S, " =? ", S, lowerOpcodes[inst->enumOpcode],
            S, ", and a pseudoup? ", I, inst->isPseudoOpe
    );

    return result;
}

void errorGetOpcodeEnum(const char *opcode) {
    logging(error, 3, S, "Invalid opcode: '", S, opcode, S, "'");
    exit(4);
}

enum OPCODES isPsuedoup(const char *opcode) {
    if (strcmp(opcode, lowerOpcodes[ORIG]) == 0)
        return orig;
    else if (strcmp(opcode, lowerOpcodes[FILL]) == 0)
        return fill;
    else if (strcmp(opcode, lowerOpcodes[END]) == 0)
        return end;
    else
        return -1;
}

int incrementLocationCntr() {
    if (locationCntr == (uint16_t) -1) {
        logging(error, 1, S, "Unauthorized memory access");
        exit(4);
    } else {
        locationCntr = pc(locationCntr);
        return locationCntr;
    }
}

void tryAddLabel(const Instruction *i) {
    if (i->label != NULL && strlen(i->label) != 0) {
        logging(debug, 3, S, "cur label: '", S, i->label, S, "'");
        validateLabel(i->label);
        addLebel(i->label, getLocationCntr());
    }
}

void toString(const Instruction *i) {
    logging(debug, 13,
            S, "{Label: "AMAG" ", S, i->label,
            S, ANRM", Opcode:"AMAG" ", S, i->opcode,
            S, ANRM", Arg1:"  AMAG" ", S, i->arg1,
            S, ANRM", Arg2:"  AMAG" ", S, i->arg2,
            S, ANRM", Arg3:"  AMAG" ", S, i->arg3,
            S, ANRM", Arg4:"  AMAG" ", S, i->arg4, S, ANRM"}"
    );
}

void validateLabel(char *label) {
    if (label[0] == 'x'
        || !isAlnumeric(label)
        || !isTrapLabel(label)
        || !isalpha(label[0])
            )
        errorValidateLabel(label);
}

int isTrapLabel(const char *label) {
    int i;
    for (i = 0; i < INVALID_LABEL_LENGTH; ++i)
        if (strcmp(label, invalidLabels[i]) == 0)
            return 0;

    return 1;
}

int isAlnumeric(char *label) {
    int i;
    for (i = 0; i < strlen(label); ++i)
        if (!isalnum(label[i]))
            return 0;

    return 1;
}

void errorValidateLabel(char *label) {
    logging(error, 3, S, "Found an invalid label '", S, label, S, "'");
    exit(4);
}

void addLebel(char *label, int lc) {
    int curTabelIndex = getTableSize();
    int i;

    /* label mustn't be duplicate */
    if (curTabelIndex != 0) {
        for (i = 0; i < curTabelIndex; ++i)
            if (strcmp(label, symbolTable[i].label) == 0)
                errorAddLabel(label);
    }

    char *labelCpy = strcpy(symbolTable[curTabelIndex].label, label);
    symbolTable[curTabelIndex].address = lc;

    if (strcmp(labelCpy, label) != 0)
        errorCopyLabel(labelCpy, label);

    logging(debug, 4,
            S, "Add symbol: '",
            S, symbolTable[curTabelIndex].label,
            S, "' with addr=",
            I, symbolTable[curTabelIndex].address, ", as element #", I,
            curTabelIndex);
    increaseTabelSize();
}

void errorCopyLabel(char *labelCpy, char *label) {
    logging(warn, 5, S, "Copy was unsuccessful, expected '", S, label, S, "',  actual '", S, labelCpy, S, "'");
    exit(4);
}

void errorAddLabel(const char *dupLabel) {
    logging(error, 2, S, "Duplicate label found: ", S, dupLabel);
    exit(4);
}

void increaseTabelSize() {
    loggingMsg(debug, "Incrementing tabel size");

    if (symbolTableLength == MAX_SYMBOLS)
        errorIncreaseTableSize();

    symbolTableLength++;
}

void errorIncreaseTableSize() {
    logging(error, 1, S, "Cannot increase tabel size bigger than limit!");
    exit(4);
}

int getTableSize() {
    return symbolTableLength;
}

void writeToFile(FILE *file, const char *text) {
    loggingMsg(debug, "Writing to file...");
    int lInstr;
    fprintf(file, text, lInstr);

    logging(debug, 2,
            S, "Success: wrote to file: ", S, text
    );
}

void writeIntToFile(FILE *file, uint16_t val) {
    loggingMsg(debug, "Writing to file...");
    int mask = 0x0000F000, i;

    writeToFile(outfile, "0x");

    for (i = 0; i < 4; ++i) {
        /* mask nibbles of value, right shift to first nibble position, output */
        fprintf(file, "%X", (mask & val) >> (3 - i) * nibble);
        mask >>= nibble;
    }

    writeToFile(outfile, "\n");

    logging(debug, 2,
            S, "Success: wrote to file: ", I, val
    );
}

FILE *openFile(char *filename, char *permission) {
    logging(debug, 2,
            S, "Openning file: ", S, filename);

    /* open the source file */
    FILE *file = fopen(filename, permission);

    if (!file)
        errorOpenFile(filename);

    else {
        logging(debug, 2,
                S, "Successfully opened file: ",
                S, filename
        );
    }

    return file;
}

void errorOpenFile(const char *filename) {
    logging(warn, 2,
            S, ARED "Failed" ANRM " to open file ", S, filename
    );
    exit(4);
}

void closeFiles(FILE *file) {
    logging(debug, 2,
            S, "Closing file descriptor: ",
            I, file
    );

    if (fclose(file) != 0)
        errorCloseFile();

    else {
        loggingMsg(debug, "Successfully closed file");
    }
}

void errorCloseFile() {
    logging(error, 1,
            S, "Error wile attempting to close file descriptor: "
    );
}

int isOpcode(char *potential_opcode) {
    int result = -1;
    int i;

    for (i = 0; i < MAX_NUM_OPCODES; ++i) {
        int curCompResult = strcmp(potential_opcode, lowerOpcodes[i]);

        if (curCompResult == 0) {
            result = 1;
            break;
        }
    }

    return result;
}

int toNum(char *pStr) {
    char *t_ptr;
    char *orig_pStr;
    int t_length, k;
    int lNum, lNeg = 0;
    long int lNumLong;

    orig_pStr = pStr;
    if (*pStr == '#')                /* decimal */
    {
        pStr++;
        if (*pStr == '-')                /* dec is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k = 0; k < t_length; k++) {
            if (!isdigit(*t_ptr)) {
                logging(error, 2,
                        S, "invalid decimal operand, ", S, orig_pStr
                );
                exit(4);
            }
            t_ptr++;
        }
        lNum = atoi(pStr);
        if (lNeg)
            lNum = -lNum;

        return lNum;
    } else if (*pStr == 'x')    /* hex     */
    {
        pStr++;
        if (*pStr == '-')                /* hex is negative */
        {
            lNeg = 1;
            pStr++;
        }
        t_ptr = pStr;
        t_length = strlen(t_ptr);
        for (k = 0; k < t_length; k++) {
            if (!isxdigit(*t_ptr)) {
                logging(error, 2,
                        S, "invalid hex operand, ", S, orig_pStr
                );
                exit(4);
            }
            t_ptr++;
        }
        lNumLong = strtol(pStr, NULL, 16);    /* convert hex string into integer */
        lNum = (lNumLong > INT_MAX) ? INT_MAX : lNumLong;
        if (lNeg)
            lNum = -lNum;
        return lNum;
    } else {
        logging(error, 2,
                S, "Invalid operand, ", S, orig_pStr
        );
        exit(4);  /* This has been changed from error code 3 to error code 4, see clarification 12 */
    }
}

/* Note: MAX_LINE_LENGTH, OK, EMPTY_LINE, and DONE are defined values */
int readAndParse(FILE *pInfile, char *pLine, char **pLabel, char **pOpcode,
                 char **pArg1, char **pArg2, char **pArg3, char **pArg4) {
    char *lRet, *lPtr;
    int i;
    if (!fgets(pLine, MAX_LINE_LENGTH, pInfile))
        return (DONE);
    for (i = 0; i < strlen(pLine); i++)
        pLine[i] = tolower(pLine[i]);

    /* convert entire line to lowercase */
    *pLabel = *pOpcode = *pArg1 = *pArg2 = *pArg3 = *pArg4 = pLine + strlen(pLine);

    /* ignore the comments */
    lPtr = pLine;

    while (*lPtr != ';' && *lPtr != '\0' &&
           *lPtr != '\n')
        lPtr++;

    *lPtr = '\0';
    if (!(lPtr = strtok(pLine, "\t\n ,")))
        return (EMPTY_LINE);

    if (isOpcode(lPtr) == -1 && lPtr[0] != '.') /* found a label */
    {
        *pLabel = lPtr;
        if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);
    }

    *pOpcode = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);

    *pArg1 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);

    *pArg2 = lPtr;
    if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);

    *pArg3 = lPtr;

    if (!(lPtr = strtok(NULL, "\t\n ,"))) return (OK);

    *pArg4 = lPtr;

    return (OK);
}

void parsCommandLine(int argc, char **argv) {
    char *prgName = NULL;
    char *iFileName = NULL;
    char *oFileName = NULL;

    prgName = argv[0];
    iFileName = argv[1];
    oFileName = argv[2];

    logging(debug, 6,
            S, "program name = ", S, prgName, N,
            S, "input file name = ", S, iFileName, N,
            S, "output file name = ", S, oFileName
    );
}

/**------------------------------------- Tests ---------------------------------------*/
void testOpenCloseFile(int argc, char *argv[], FILE *infile, FILE *outfile) {
    /* open the source file */
    infile = fopen(argv[1], "r");
    outfile = fopen(argv[2], "w");

    if (!infile) {
        logging(warn, 2,
                S, "Error: Cannot open file ", S, argv[1]);
        exit(4);
    }
    if (!outfile) {
        logging(warn, 2,
                S, "Error: Cannot open file ", S, argv[2]);
        exit(4);
    } else {
        logging(debug, 1,
                S, "Successfully opened both files!"
        );
    }

    /* Do stuff with files */

    closeFiles(infile);
    closeFiles(outfile);
}

void testWriteToFile(const FILE *infile) {
    writeToFile(infile, "\t.ORIG x3000\n");
    writeToFile(infile, "A\tAND R0, R0, #0\n");
    writeToFile(infile, "\tLEA R0, A\n");
    writeToFile(infile, "\tHALT\n");
    writeToFile(infile, "\t.END\n");
}

void testIsOpcode() {
    char *myOpcode = "AND";

    logging(debug, 2,
            S, "The reselt is: ",
            I, isOpcode(myOpcode)
    );
}

void testToNum() {
    /*=====================================Test 1========================================*/
    loggingMsg(debug, "Converting string to num for valid value 'x3000'");

    int result = toNum("xffff");

    if (result > MAX_ADDRESS)
        logging(error, 3, I, result, S, " -> Overflow occured: ", I, USHRT_MAX);

    if (result != 0xffff) {
        logging(error, 2,
                S, "Failed: expected result=0xffff but actual result=", I, result
        );
    } else {}
    logging(debug, 2,
            S, "Success: expected result=0xffff and actual result=", I, result
    );

    /*=====================================Test 2========================================*/
    loggingMsg(debug, "Converting string to num for valid value '#45'");

    result = toNum("#-45");

    if (result != -45)
        logging(error, 2,
                S, "Failed: expected result=-45 but actual result=", I, result
        );
    else
        logging(debug, 2,
                S, "Success: expected result=-45 and actual result=", I, result
        );

    /*=====================================Test 3========================================*/
    loggingMsg(debug, "Converting string to num for invalid value '0'. Will exit(4)!");
    result = toNum("0");
}

void testLimits() {
    logging(debug,
            26, /*(51-36)*2 = 31; subtracting line numbers, to get total number of outputings*/
            S, "The number of bits in a byte = ", I8, CHAR_BIT, N,
            S, "The minimum value of SIGNED CHAR = ", I8, SCHAR_MIN, N,
            S, "The maximum value of SIGNED CHAR = ", I8, SCHAR_MAX, N,
            S, "The maximum value of UNSIGNED CHAR = ", UI8, UCHAR_MAX, N,
            S, "The minimum value of SHORT INT = ", I16, SHRT_MIN, N,
            S, "The maximum value of SHORT INT = ", I16, SHRT_MAX, N,
            S, "The minimum value of INT = ", I, INT_MIN, N,
            S, "The maximum value of INT = ", I, INT_MAX, N,
            S, "The minimum value of CHAR = ", I8, CHAR_MIN, N,
            S, "The maximum value of CHAR = ", I8, CHAR_MAX, N,
            S, "The minimum value of LONG = ", L, LONG_MIN, N,
            S, "The maximum value of LONG = ", L, LONG_MAX, N,
            S, "The maximum value of UNSIGNED LONG = ", UL, ULONG_MAX
    );
}

/**-------------------------------- Logging Library ---------------------------------*/
void printList(int num, va_list valist) {
    int i;
    num *= 2;

    enum DATA_TYPE T;
    void *V;

    /* access all the arguments assigned to valist */
    for (i = 0; i < num;) {
        T = va_arg(valist, enum DATA_TYPE);

        if (T == N || T == ENDL) {
            V = EMPTY_VAL;
            T == ENDL ? i += 2 : 0;
        } else {
            V = va_arg(valist, void *);
            i += 2;
        }

        output(V, T);
    }
}

void print(int num, ...) {
    va_list valist;

    /* initialize valist for num number of arguments */
    va_start(valist, num);

    printList(num, valist);

    /* clean memory reserved for valist */
    va_end(valist);
}

void println(int num, ...) {
    va_list valist;

    /* initialize valist for num number of arguments */
    va_start(valist, num);

    printList(num, valist);

    /* clean memory reserved for valist */
    va_end(valist);

    output(EMPTY_VAL, ENDL);
}

void outputDouble(double value) {
    printf("%f", value);
}

void output(void *V, enum DATA_TYPE Type) {
    Instruction *i = (Instruction *) V;

    switch (Type) {
        case INSTRUCTION:
            printf("\n" AWHT "");
            if (i->label != NULL && strcmp("", i->label) != 0) printf("%s   ", i->label);
            printf("%s", i->opcode);
            if (i->arg1 != NULL && strcmp("", i->arg1) != 0) printf(", %s", i->arg1);
            if (i->arg2 != NULL && strcmp("", i->arg2) != 0) printf(", %s", i->arg2);
            if (i->arg3 != NULL && strcmp("", i->arg3) != 0) printf(", %s", i->arg3);
            if (i->arg4 != NULL && strcmp("", i->arg4) != 0) printf(", %s", i->arg4);
            printf(ANRM "");
            break;
        case P:
        case PTR:
            printf("%p", (int *) V);
            break;
        case C:
        case CHAR:
            printf("%c", (char) V);
            break;
        case S:
        case STR:
            printf("%s", (char *) V);
            break;
        case I8:
        case INT_8:
            printf("%d %s(0x%X)%s", (int8_t) V, AWHT, (int8_t) V, ANRM);
            break;
        case I16:
        case INT_16:
            printf("%d %s(0x%X)%s", (int16_t) V, AWHT, (int16_t) V, ANRM);
            break;
        case I32:
        case INT_32:
            printf("%d %s(0x%X)%s", (int32_t) V, AWHT, (int32_t) V, ANRM);
            break;
        case I64:
        case INT_64:
            printf("%ld %s(0x%lX)%s", (int64_t) V, AWHT, (int64_t) V, ANRM);
            break;
        case UI8:
        case UINT_8:
            printf("%u %s(0x%X)%s", (uint8_t) V, AWHT, (uint8_t) V, ANRM);
            break;
        case UI16:
        case UINT_16:
            printf("%u %s(0x%X)%s", (uint16_t) V, AWHT, (uint16_t) V, ANRM);
            break;
        case UI32:
        case UINT_32:
            printf("%u %s(0x%X)%s", (uint32_t) V, AWHT, (uint32_t) V, ANRM);
            break;
        case UI64:
        case UINT_64:
            printf("%llu %s(0x%llX)%s", (uint64_t) V, AWHT, (uint64_t) V, ANRM);
            break;
        case I:
        case INT:
            printf("%d %s(0x%X)%s", (int) V, AWHT, (int) V, ANRM);
            break;
        case L:
        case LONG:
            printf("%ld %s(0x%lX)%s", (long) V, AWHT, (long) V, ANRM);
            break;
        case LI:
        case LONG_INT:
            printf("%ld %s(0x%lX)%s", (long int) V, AWHT, (long int) V, ANRM);
            break;
        case UL:
        case ULONG:
            printf("%lu %s(0x%lX)%s", (unsigned long) V, AWHT, (unsigned long) V, ANRM);
            break;
        case ULI:
        case ULONG_INT:
            printf("%lu %s(0x%lX)%s", (unsigned long int) V, AWHT, (unsigned long int) V, ANRM);
            break;
        case N:
        case ENDL:
            printf("\n");
            break;
        case T:
        case TAB:
            printf("\t");
            break;
        default:
            printf("INVALID ARGUMENTS\n");
    }
}

void loggingMsg(const char *func, int line, enum LOG_LEVELS lvl, const char *msg) {
    if (lvl >= LOG_LEVEL) {
        int curLogNum = getLogNum();
        setColorPrint(lvl);
        printf("[%s]: func:'%s' → line:%d (%d)%s\n%s", enumStrings[lvl], func, line, curLogNum, ANRM, msg);
        setColorPrint(lvl);
        printf(" (%d)%s\n", curLogNum, ANRM);
    }
}

void logging(const char *func, int line, enum LOG_LEVELS lvl, int num, ...) {
    va_list valist;

    /* initialize valist for num number of arguments */
    va_start(valist, num);

    if (lvl >= LOG_LEVEL) {
        int curLogNum = getLogNum();
        setColorPrint(lvl);
        printf("[%s]: func:'%s' → line:%d (%d)\n" ANRM, enumStrings[lvl], func, line, curLogNum);
        printList(num, valist);
        setColorPrint(lvl);
        printf(" (%d)\n" ANRM, curLogNum);
    }

    /* clean memory reserved for valist */
    va_end(valist);

}

int getLogNum() {
    int curNum = logNum;
    logNum += 1;
    return curNum;
}

void colorPrint(enum LOG_LEVELS lvl, const char *txt) {
    switch (lvl) {
        case DEBUG:
            printf(ACYN "%s" ANRM, txt);
            break;
        case INFO:
            printf(ABLU "%s" ANRM, txt);
            break;
        case WARN:
            printf(AYEL "%s" ANRM, txt);
            break;
        case ERROR:
            printf(ARED "%s" ANRM, txt);
            break;
    }
}

void setColorPrint(enum LOG_LEVELS lvl) {
    switch (lvl) {
        case DEBUG:
            printf(ACYN "");
            break;
        case INFO:
            printf(ABLU "");
            break;
        case WARN:
            printf(AYEL "");
            break;
        case ERROR:
            printf(ARED "");
            break;
    }
}

Instruction *newInstruction() {
    Instruction *instruction = malloc(sizeof(Instruction *));
}

void freeInstruction(Instruction *instruction) {
    free(instruction);
}

void printAll() {
    int i;
    for (i = 0; i < MAX_NUM_OPCODES; ++i) {
        printf("%d) %s-----%d\n", i + 1, lowerOpcodes[i], opcodes[i]);

    }

}
