#include <stdio.h>

int cycle =0;
struct Memory{
    int rows[2048];
};
struct Registers{
    int GPR[31];
    int zero;
    int pc;
};
struct decodedInstruction{
    int operationNum;
    int operand1Index;
    int operand2Index;
};
int fetch(){
    //acess pc, get instruction, increment pc
}
struct decodedInstruction decode(int instruction){
    //read Instruction from fetch, decode it, return operation number and operand indices
}
void execute(struct decodedInstruction instruction){
    //do all ALU operations
}
int memoryLoad(int rowIndex){
    //returns value in this address
}
void memoryStore(int rowIndex, int operand){
    //stores operand in rowIndex
}
void writeBack(int GPRIndex, int operand){
    //write result(operand) back to GPR defined by the GPRIndex
}
