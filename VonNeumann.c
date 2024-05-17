#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>



int dataMemoryLoad(int);
int instructionsMemoryLoad(int);

int cycle = 1;
int wbFlag = 0;
int memOriginalInstruction = 0;
int wbOriginalInstruction = 0;

struct Queue* dec_q; //to be decoded instruction queue
struct InstQueue* ex_q; //to be executed instruction queue
struct Queue* memrow_q; // memory access, address in memory
struct Queue* memop_q; // memory access, result to put in memory
struct Queue* wbi_q; // write back, destination
struct Queue* wbop_q; // write back, result

struct Memory{
    int rows[2048];
} memory; // Declare memory as a global variable

struct Registers{
    int GPR[31];
    int r0;
    int pc;
} registers = {0}; // Declare registers as a global variable

struct decodedInstruction{
    int opcode;          // Opcode specifying the operation to be performed - [31:28]
    int r1;              // Index of the first source/destination register - [27:23]
    int r2;              // Index of the second source register - [22:18]
    int r3;              // Index of the third source/destination register (for some instructions) - [17:12]
    int shamt;           // Shift amount (for shift instructions) - [12:0]
    int immediate;       // Immediate value (for immediate arithmetic/logic operations) - [17:0]
    int address;         // Memory address (for load/store instructions) - [27:0]
    int instruction;     // original instruction, for debugging purposes
};

#pragma region int queue implementation
struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
};

// function to create a queue of given capacity. It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(
        queue->capacity * sizeof(int));
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}
 
// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue. It changes rear and size
void enqueue(struct Queue* queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    // printf("%d enqueued to queue\n", item);
}
 
// Function to remove an item from queue. It changes front and size
int dequeue(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

//removes end of queue
int dequeueEnd(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->rear];
    queue->rear = (queue->rear - 1);
    queue->size = queue->size - 1;
    return item;
}

// Function to get front of queue
int front(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}
#pragma endregion

#pragma region decodedInstruction queue implementation
struct InstQueue {
    int front, rear, size;
    unsigned capacity;
    struct decodedInstruction* array;
};

// Function to create a queue of given capacity. It initializes size of queue as 0
struct InstQueue* createInstQueue(unsigned capacity)
{
    struct InstQueue* queue = (struct InstQueue*)malloc(sizeof(struct InstQueue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;

    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (struct decodedInstruction*)malloc(queue->capacity * sizeof(struct decodedInstruction));
    return queue;
}

// Queue is full when size becomes equal to the capacity
int isInstFull(struct InstQueue* queue)
{
    return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isInstEmpty(struct InstQueue* queue)
{
    return (queue->size == 0);
}

// Function to add an item to the queue. It changes rear and size
void enqueueInst(struct InstQueue* queue, struct decodedInstruction item)
{
    if (isInstFull(queue))
        return;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    // printf("Instruction enqueued to queue\n");
}

// Function to remove an item from queue. It changes front and size
struct decodedInstruction dequeueInst(struct InstQueue* queue)
{
    struct decodedInstruction emptyInstruction = {INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN};
    if (isInstEmpty(queue))
        return emptyInstruction;
    struct decodedInstruction item = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
//removes end of queue
struct decodedInstruction dequeueInstEnd(struct InstQueue* queue)
{
    struct decodedInstruction emptyInstruction = {INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN};
    if (isInstEmpty(queue))
        return emptyInstruction;
    queue->rear = (queue->rear - 1);
    queue->size = queue->size - 1;
    struct decodedInstruction item = queue->array[queue->rear];
    return item;
}
// Function to get front of queue
struct decodedInstruction frontInst(struct InstQueue* queue)
{
    struct decodedInstruction emptyInstruction = {INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN};
    if (isInstEmpty(queue))
        return emptyInstruction;
    return queue->array[queue->front];
}
#pragma endregion




void decToBin(int num){
    /*int num2 = num;
    num = abs(num);
    char binary[256];
    int length = 0;
    do{
        if( num%2 == 0) binary[length] = '0';
        else binary[length] = '1';
        num/=2;
        length++;
    }while(length<31);
    if(num2<0){
        binary[length+1] = '1';
    }
    else{
        binary[length+1] = '0';
    }
    binary[length] = '\0';
    int middle = length/2;
    char temp;
    for(int i =0; i< middle;i++){
        temp = binary[i];
        binary[i] = binary[length - i - 1];
        binary[length - i - 1] = temp;
    }
    printf("%s" , binary);*/
    char binary[33]; // Array to store the binary representation + null terminator
    int length = 32; // Fixed length for 32-bit representation

    // Handle the two's complement representation
    for (int i = length - 1; i >= 0; i--) {
        binary[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }
    binary[length] = '\0'; // Null-terminate the string

    printf("%s", binary);
}
//fetch decode excute cycle



void fetch(struct Queue* dec_q){
    int instruction = instructionsMemoryLoad(registers.pc);
    if(instruction != 0) {
        enqueue(dec_q, instruction);
        printf("cycle: %d, pc: %d, fetching instruction %d  ", cycle , registers.pc , instruction);
        decToBin(instruction);
        printf("\n");
        registers.pc++;
    }
}

void decode(struct Queue* dec_q, struct InstQueue* ex_q){
    
    printf("cycle: %d, decoding instruction %d ", cycle, front(dec_q));
    decToBin(front(dec_q));
    if(cycle%2 == 1) {
        struct decodedInstruction decodedInstruction;
        int instruction = dequeue(dec_q);
        unsigned int temp = (unsigned int)instruction;
        
        
        temp = instruction & 0b11110000000000000000000000000000;
        temp = temp >> 28;
        decodedInstruction.opcode = (int)temp;
        printf(", opcode:%d, ", decodedInstruction.opcode);

        temp = instruction & 0b00001111100000000000000000000000;
        temp = temp >> 23;
        decodedInstruction.r1 = temp;
        printf("R1:%d, ", decodedInstruction.r1);

        temp = instruction & 0b00000000011111000000000000000000;
        temp = temp >> 18;
        decodedInstruction.r2 = temp;
        printf("R2:%d, ", decodedInstruction.r2);

        temp = instruction & 0b00000000000000111110000000000000;
        temp = temp >> 13;
        decodedInstruction.r3 = temp;
        printf("R3:%d, ", decodedInstruction.r3);

        temp = instruction & 0b00000000000000000001111111111111;
        decodedInstruction.shamt = temp;
        printf("Shamt:%d, ", decodedInstruction.shamt);

        temp = instruction & 0b00000000000000111111111111111111;
        decodedInstruction.immediate = temp;
        printf("immediate:%d, ", temp);

        temp = instruction & 0b00001111111111111111111111111111;
        decodedInstruction.address = temp;
        printf("address:%d ", temp);

        decodedInstruction.instruction = instruction;

        enqueueInst(ex_q, decodedInstruction);
        

    }
    printf("\n");
}

void execute(struct Queue* dec_q,struct InstQueue* ex_q, struct Queue* memrow_q, struct Queue* memop_q, struct Queue* wbi_q, struct Queue* wbop_q){
    printf("cycle: %d, pc: %d, executing instruction %d ", cycle,registers.pc, frontInst(ex_q).instruction);
    decToBin(frontInst(ex_q).instruction);
    struct decodedInstruction instructiontemp = frontInst(ex_q);
    printf(", opcode: %d, R1: %d, R2: %d, R3: %d, shamt: %d, immediate: %d, address: %d ", instructiontemp.opcode,
            instructiontemp.r1, instructiontemp.r2, instructiontemp.r3, instructiontemp.shamt , instructiontemp.immediate, instructiontemp.address);
    printf("\n");
    memOriginalInstruction = frontInst(ex_q).instruction;

    if(cycle%2 == 1) {
        struct decodedInstruction instruction = dequeueInst(ex_q);
        // Add
        if (instruction.opcode == 0) {
            // -1 as if RS==5, we need to load R4 from GPR as it starts from R1, R0 is stored alone
            int result = registers.GPR[instruction.r2 -1] + registers.GPR[instruction.r3 -1];
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Subtract
        if (instruction.opcode == 1) {
            int result = registers.GPR[instruction.r2-1] - registers.GPR[instruction.r3-1];
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Multiply
        if (instruction.opcode == 2) {
            int result = registers.GPR[instruction.r2-1] * registers.GPR[instruction.r3-1];
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Move Immediate
        if (instruction.opcode == 3) {
            int result = instruction.immediate;
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Jump If Equal
        if (instruction.opcode == 4) {
            if (registers.GPR[instruction.r1-1] == registers.GPR[instruction.r2-1]) {
                if(registers.pc + instruction.immediate < 0){
                    registers.pc = 0;
                }else{
                    registers.pc += (instruction.immediate-1);
                }
                //remove instruction that was fetched and one that was decoded
                if(instruction.immediate>1){
                    dequeueEnd(dec_q);
                }
                dequeueInstEnd(ex_q);
                }
            
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, -1);
            enqueue(wbop_q, -1);
        }

        // And
        if (instruction.opcode == 5) {
            int result = registers.GPR[instruction.r2-1] & registers.GPR[instruction.r3-1];
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Exclusive Or Immediate
        if (instruction.opcode == 6) {
            int result = registers.GPR[instruction.r2-1] ^ instruction.immediate;
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Jump
        if (instruction.opcode == 7) {
            int temp = registers.pc && 0b11110000000000000000000000000000;
            registers.pc = temp | instruction.address; 
            printf("PC VALUE BITCHES %d" , registers.pc);
            //was the last line in the "if condition" but i moved it up as first line
            //remove instruction that was fetched and one that was decoded
            if(instruction.immediate%2 == 0){
                registers.pc -=1;
            }
            if(instruction.immediate == 2){
                dequeueEnd(dec_q);
            }
            else if (instruction.immediate != 1){
                dequeueEnd(dec_q);
                dequeueInstEnd(ex_q);
            }
            
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, -1);
            enqueue(wbop_q, -1);
        }

        // Logical Shift Left
        if (instruction.opcode == 8) {
            int result = registers.GPR[instruction.r2-1] << instruction.shamt;
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Logical Shift Right
        if (instruction.opcode == 9) {
            int result = registers.GPR[instruction.r2-1] >> instruction.shamt;
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Move to Register
        if (instruction.opcode == 10) {
            int result =0;
            if(registers.GPR[instruction.r2-1] + instruction.immediate > 0){
                result = memory.rows[1024 + registers.GPR[instruction.r2-1] + instruction.immediate];
            }
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Move to Memory
        if (instruction.opcode == 11) {
            int address = registers.GPR[instruction.r2-1] + instruction.immediate;
            if(address<0){
                address = 0;
            }
            int result = registers.GPR[instruction.r1-1];
            enqueue(memrow_q, address);
            enqueue(memop_q, result);
            enqueue(wbi_q, -1);
            enqueue(wbop_q, -1);
        }
    }
}

int dataMemoryLoad(int rowIndex){
    //returns value in this address
    return memory.rows[1024 + rowIndex];
}

int instructionsMemoryLoad(int rowIndex){
    //returns value in this address
    return memory.rows[rowIndex];
}

void memoryStore(int originalInstruction,struct Queue* memrow_q, struct Queue* memop_q, struct Queue* wbi_q, struct Queue* wbop_q){
    //stores operand in rowIndex
    printf("cycle: %i, address: %d, value: %d, memory store instruction: ",cycle, front(memrow_q),front(memop_q));
    decToBin(originalInstruction);
    printf("\n");
    wbOriginalInstruction = memOriginalInstruction;

    if(!isEmpty(memop_q)) {
        if(front(memop_q) != -1) {
            memory.rows[1024 + dequeue(memrow_q)] = dequeue(memop_q);
        }
        else {
            dequeue(memrow_q);
            dequeue(memop_q);
        }
    }
    if(front(wbop_q) == -1)
        wbFlag = 2; // 2= false
    else
        wbFlag = 1; // 1= true
}

void writeBack(int originalInstruction,struct Queue* wbi_q, struct Queue* wbop_q){
    //write result(operand) back to GPR defined by the GPRIndex
    printf("cycle:%d, address:%d, value:%d, writeback instruction: ", cycle, front(wbi_q), front(wbop_q));
    decToBin(originalInstruction);
    printf("\n");

    int GPR = dequeue(wbi_q);
    int operand = dequeue(wbop_q);
    if(wbFlag == 2 || GPR == 0)
        return;
    // GPR - 1 because R1 is at index 0
    registers.GPR[GPR - 1] = operand;
}



//parsing: read text file of MIPS, convert to int instruction and store in memory
    //1-import text file into a string array: helper method
    char** readFile(){
        FILE *file;
        char line[100];

        // Open the file
        char fileName[64];
        sprintf(fileName, "testData.txt"); //Concatenate the filename
        printf("Opening file:%s\n", fileName);
        file = fopen(fileName, "r");

        if (file == NULL)
        {
            perror("Error opening file");
            return NULL;
        }

        // Allocate memory for storing lines
        char **lines = (char **)malloc(256 * sizeof(char *));
        if (lines == NULL)
        {
            perror("Memory allocation error");
            fclose(file);
            return NULL;
        }

        int i = 0;
        // Read lines until the end of the file
        while (fgets(line, sizeof(line), file) != NULL && i < 256)
        {
            lines[i] = strdup(line); // Allocate memory and copy line
            i++;
        }
        lines[i] = NULL;
        // Close the file
        fclose(file);

        return lines;
    }
    //2-turn each string instruction to binary integer : helper method
    int instructionToBinary(char* linePtr){
        char line[50];
        strcpy(line, linePtr);
        char d[] = " ";
        char s[4][10]; // Assuming maximum portion length is 10 characters
        int count = 0;

    //2-split each instruction into words
        char *portion = strtok(line, d);
        while (portion != NULL && count < 4) {
            strcpy(s[count], portion);
            count++;
            portion = strtok(NULL, d);
        }

        int length = sizeof(s) / sizeof(s[0]); 

        int instruction = 0;

    //3-convert each parsed string to concatenated string of 0s and 1s
        //operation
        if(strcmp(s[0], "ADD") == 0){
            instruction = instruction | 0b00000000000000000000000000000000;
        }else if (strcmp(s[0], "SUB") == 0) {
            instruction = instruction | 0b00010000000000000000000000000000;
        }else if (strcmp(s[0], "MUL") == 0) {
            instruction = instruction | 0b00100000000000000000000000000000;
        }else if (strcmp(s[0], "MOVI") == 0) {
            instruction = instruction | 0b00110000000000000000000000000000;
        }else if (strcmp(s[0], "JEQ") == 0) {
           instruction = instruction | 0b01000000000000000000000000000000;
        }else if (strcmp(s[0], "AND") == 0) {
            instruction = instruction | 0b01010000000000000000000000000000;
        }else if (strcmp(s[0], "XORI") == 0) {
            instruction = instruction | 0b01100000000000000000000000000000;
        }else if (strcmp(s[0], "JMP") == 0) {
            instruction = instruction | 0b01110000000000000000000000000000;
        }else if (strcmp(s[0], "LSL") == 0) {
            instruction = instruction | 0b10000000000000000000000000000000;
        }else if (strcmp(s[0], "LSR") == 0) {
            instruction = instruction | 0b10010000000000000000000000000000;
        }else if (strcmp(s[0], "MOVR") == 0) {
            instruction = instruction | 0b10100000000000000000000000000000;
        }else if (strcmp(s[0], "MOVM") == 0) {
            instruction = instruction | 0b10110000000000000000000000000000;
        }
        
        //operands
        if(strcmp(s[0], "ADD") == 0 || strcmp(s[0], "SUB") == 0 || strcmp(s[0], "MUL") == 0 || strcmp(s[0], "AND") == 0 ){
            //convert from string to register numbers
            char *ptr1 = strchr(s[1], 'R');
            int number1 = atoi(ptr1 + 1);
            char *ptr2 = strchr(s[2], 'R');
            int number2 = atoi(ptr2 + 1);
            char *ptr3 = strchr(s[3], 'R');
            int number3 = atoi(ptr3 + 1);
            number1 = number1 << 23;
            number2 = number2 << 18;
            number3 = number3 << 13;
            instruction = instruction | number1;
            instruction = instruction | number2;
            instruction = instruction | number3;

        }else if(strcmp(s[0], "LSL") == 0 || strcmp(s[0], "LSR") == 0){
            //convert from string to register numbers
            char *ptr1 = strchr(s[1], 'R');
            int number1 = atoi(ptr1 + 1);
            char *ptr2 = strchr(s[2], 'R');
            int number2 = atoi(ptr2 + 1);
            int decimalshamt = atoi(s[3]);
            //convert from decimal1 to binary
            number1 = number1 << 23;
            number2 = number2 << 18;
            instruction = instruction | number1;
            instruction = instruction | number2;
            instruction = instruction | decimalshamt;
        }else if(strcmp(s[0], "JEQ") == 0 || strcmp(s[0], "XORI") == 0 || strcmp(s[0], "MOVR") == 0 || strcmp(s[0], "MOVM") == 0){
            //convert from string to register numbers
            char *ptr1 = strchr(s[1], 'R');
            int number1 = atoi(ptr1 + 1);
            char *ptr2 = strchr(s[2], 'R');
            int number2 = atoi(ptr2 + 1);
            int decimalimmediate = atoi(s[3]);
            //convert from decimal1 to binary
            number1 = number1 << 23;
            number2 = number2 << 18;
            instruction = instruction | number1;
            instruction = instruction | number2;
            instruction = instruction | decimalimmediate;
        }else if(strcmp(s[0], "MOVI") == 0){
            //convert from string to register numbers
            char *ptr1 = strchr(s[1], 'R');
            int number1 = atoi(ptr1 + 1);
            int decimalimmediate = atoi(s[2]);
            //convert from decimal1 to binary
            number1 = number1 << 23;
            instruction = instruction | number1;
            instruction = instruction | decimalimmediate;
        }else if(strcmp(s[0], "JMP") == 0){
            //convert from string to decimal
            int decimal = atoi(s[1]);
            //convert from decimal to binary
            instruction = instruction | decimal;
            
        }
        return instruction;
        
    }
    //3- add to memory
    void initializeMemory(){
        //read file
        int length = 0; //number of instructions
        char **file = readFile(); //change temp value
        while (file[length] != NULL) {
            length++;
        }
        //convert instruction to binary and store in memory
        for(int i =0; i< length; i++){
            int instruction = instructionToBinary(file[i]);
            memory.rows[i] = instruction;
            printf("Line: %s" , file[i]);
            printf("instruction: %i \n" , memory.rows[i]);
            printf("\n" );
            //printf("instruction: %i \n" , instruction);
        }

    }


void start(){
    initializeMemory();

    dec_q = createQueue(100);
    ex_q = createInstQueue(100);
    memrow_q = createQueue(100);
    memop_q = createQueue(100);
    wbi_q = createQueue(100);
    wbop_q = createQueue(100);
    

    while((memory.rows[registers.pc]!=0 &&  registers.pc < 1023 )|| 
    !isEmpty(dec_q) || !isInstEmpty(ex_q) || !isEmpty(memrow_q) || !isEmpty(memop_q) || !isEmpty(wbi_q) || !isEmpty(wbop_q)) {
    //while(cycle <= 19){
        printf("Cycle: %i \n", cycle);
        
        if(wbFlag != 0) {
            wbFlag = 0;
            writeBack(wbOriginalInstruction,wbi_q, wbop_q);
        }
        if(cycle%2 == 0 && !isEmpty(memop_q)) {
            memoryStore(memOriginalInstruction,memrow_q, memop_q, wbi_q, wbop_q);
        }
        if(!isInstEmpty(ex_q)) {
            execute(dec_q,ex_q, memrow_q, memop_q, wbi_q, wbop_q);
        }
        if(!isEmpty(dec_q)) {
            decode(dec_q, ex_q);
        }
        if(cycle%2 == 1) {
            fetch(dec_q);
        }
        cycle++;

        printf("\n");
    }

    printf("Contents after Last clock cycle: \n \n");
    printf("PC:%d \n" , registers.pc);
    printf("R0:%d \n" , registers.r0);
    for(int i = 0; i< 32 ; i ++){
        printf("R%d:%d \n" ,i+1, registers.GPR[i]);
    }
    for(int i = 0; i<2048;i++){
        printf("memory row %d:%d, " , i+1, memory.rows[i]);
    }
}
//tesing
void main() {
    start();
    /*memory.rows[0] = 0b00000000100100001010000000000000;
    memory.rows[1] = 0b00000001000100001010000000000000;
    memory.rows[2] = 0b00000001100100001010000000000000;
    memory.rows[3] = 0b00000010000100001010000000000000;
    memory.rows[4] = 0b00000010100100001010000000000000;
    memory.rows[5] = 0b00000011000100001010000000000000;
    memory.rows[6] = 0b00000011100100001010000000000000;*/

    /*
    Expected output:

    Opening file:testData.txt
    length: 12

    Line: ADD R16 R16 R16
    instruction: 138543104 00001000010000100000000000000000

    Line: SUB R16 R16 R16
    instruction: 406978560 00011000010000100000000000000000

    Line: MUL R16 R16 R16
    instruction: 675414016 00101000010000100000000000000000

    Line: MOVI R17 131072
    instruction: 948043776 00111000100000100000000000000000

    Line: JEQ R16 R16 131072
    instruction: 1212284928 01001000010000100000000000000000

    Line: AND R16 R16 R16
    instruction: 1480720384 01011000010000100000000000000000

    Line: XORI R16 R16 131072
    instruction: 1749155840 01101000010000100000000000000000

    Line: JMP 134217728
    instruction: 2013265920 01111000000000000000000000000000

    Line: LSL R16 R16 4096
    instruction: -2009067520 10001000010000000001000000000000

    Line: LSR R16 R16 4096
    instruction: -1740632064 10011000010000000001000000000000

    Line: MOVR R16 R16 131072
    instruction: -1472069632 10101000010000100000000000000000

    Line: MOVM R16 R16 131072
    instruction: -1203634176 10111000010000100000000000000000

    */
    /*
    1)Line: MOVI R2 2
    instruction: 822083586 
    0011 00010 00000 000000000000000010

    2)Line: MOVI R3 3
    instruction: 830472195 
    0011 00011 00000 000000000000000011

    3)Line: MOVI R5 10
    instruction: 847249418 
    0011 00101 00000 000000000000001010

    4)Line: MOVI R6 5
    instruction: 855638021 
    0011 00110 00000 000000000000000101

    5)Line: ADD R1 R2 R3
    instruction: 8937472 
    0000 00001 00010 00011 0000000000000

    6)Line: SUB R4 R5 R6
    instruction: 303349760 
    0001 00100 00101 00110 0000000000000

    7)Line: JMP 134217728
    instruction: 2013265920 
    0111 1000000000000000000000000000
    */
    

    
}