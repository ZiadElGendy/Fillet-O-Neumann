#include <stdio.h>

int cycle = 0;
struct Memory{
    int rows[2048];
} memory; // Declare memory as a global variable

struct Registers{
    int GPR[31];
    int zero;
    int pc;
} registers; // Declare registers as a global variable

struct decodedInstruction{
    int opcode;          // Opcode specifying the operation to be performed - [31:28]
    int r1;              // Index of the first source/destination register - [27:23]
    int r2;              // Index of the second source register - [22:18]
    int r3;              // Index of the third source/destination register (for some instructions) - [17:12]
    int shamt;           // Shift amount (for shift instructions) - [12:0]
    int immediate;       // Immediate value (for immediate arithmetic/logic operations) - [17:0]
    int address;         // Memory address (for load/store instructions) - [27:0]
};

int fetch(){
    //acess pc, get instruction, increment pc
}
struct decodedInstruction decode(int instruction){
    int temp;
    struct decodedInstruction decodedInstruction;

    temp = instruction & 0b11110000000000000000000000000000;
    temp = temp >> 28;
    decodedInstruction.opcode = temp;

    temp = instruction & 0b00001111100000000000000000000000;
    temp = temp >> 23;
    decodedInstruction.r1 = temp;

    temp = instruction & 0b00000000011111000000000000000000;
    temp = temp >> 18;
    decodedInstruction.r2 = temp;

    temp = instruction & 0b00000000000000111111000000000000;
    temp = temp >> 12;
    decodedInstruction.r3 = temp;

    temp = instruction & 0b00000000000000000000111111111111;
    decodedInstruction.shamt = temp;

    temp = instruction & 0b00000000000000111111111111111111;
    decodedInstruction.immediate = temp;

    temp = instruction & 0b00001111111111111111111111111111;
    decodedInstruction.address = temp;

    return decodedInstruction;
}

void execute(struct decodedInstruction instruction){
    // Add
    if (instruction.opcode == 0) {
        int result = registers.GPR[instruction.r2] + registers.GPR[instruction.r3];
        writeBack(instruction.r1, result);
    }

    // Subtract
    if (instruction.opcode == 1) {
        int result = registers.GPR[instruction.r2] - registers.GPR[instruction.r3];
        writeBack(instruction.r1, result);
    }

    // Multiply
    if (instruction.opcode == 2) {
        int result = registers.GPR[instruction.r2] * registers.GPR[instruction.r3];
        writeBack(instruction.r1, result);
    }

    // Move Immediate
    if (instruction.opcode == 3) {
        int result = instruction.immediate;
        writeBack(instruction.r1, result);
    }

    // Jump If Equal
    if (instruction.opcode == 4) {
        if (registers.GPR[instruction.r1] == registers.GPR[instruction.r2]) {
            registers.pc = instruction.address;
        }
    }

    // And
    if (instruction.opcode == 5) {
        int result = registers.GPR[instruction.r2] & registers.GPR[instruction.r3];
        writeBack(instruction.r1, result);
    }

    // Exclusive Or Immediate
    if (instruction.opcode == 6) {
        int result = registers.GPR[instruction.r2] ^ instruction.immediate;
        writeBack(instruction.r1, result);
    }

    // Jump
    if (instruction.opcode == 7) {
        registers.pc = instruction.address;
    }

    // Logical Shift Left
    if (instruction.opcode == 8) {
        int result = registers.GPR[instruction.r2] << instruction.shamt;
        writeBack(instruction.r1, result);
    }

    // Logical Shift Right
    if (instruction.opcode == 9) {
        int result = registers.GPR[instruction.r2] >> instruction.shamt;
        writeBack(instruction.r1, result);
    }

    // Move to Register
    if (instruction.opcode == 10) {
        int result = registers.GPR[instruction.r2];
        writeBack(instruction.r1, result);
    }

    // Move to Memory
    if (instruction.opcode == 11) {
        int result = registers.GPR[instruction.r1];
        memoryStore(instruction.address, result);
    }
}
int memoryLoad(int rowIndex){
    //returns value in this address
    return memory.rows[rowIndex];
}
void memoryStore(int rowIndex, int operand){
    //stores operand in rowIndex
    memory.rows[rowIndex] = operand;
}
void writeBack(int GPRIndex, int operand){
    //write result(operand) back to GPR defined by the GPRIndex
}
