#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include<string.h>



int memoryLoad(int);

int cycle = 0;
int NumberofInstructions = 0;
int wbFlag = 0;

struct Queue* dec_q;
struct InstQueue* ex_q;
struct Queue* memrow_q;
struct Queue* memop_q;
struct Queue* wbi_q;
struct Queue* wbop_q;

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

// Function to get front of queue
struct decodedInstruction frontInst(struct InstQueue* queue)
{
    struct decodedInstruction emptyInstruction = {INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MIN};
    if (isInstEmpty(queue))
        return emptyInstruction;
    return queue->array[queue->front];
}
#pragma endregion

void addToMemory(int value){
    memory.rows[NumberofInstructions] = value;
    NumberofInstructions++;
}

int getInst(int i) {
    if(i == 9478144)
        return 1;
    if(i == 17866752)
        return 2;
    if(i == 26255360)
        return 3;
    if(i == 34643968)
        return 4;
    if(i == 43032576)
        return 5;
    if(i == 51421184)
        return 6;
    if(i == 59809792)
        return 7;
    return -1;
}




//fetch decode excute cycle

void fetch(struct Queue* dec_q){
    int instruction = memoryLoad(registers.pc);
    if(instruction != 0) {
        enqueue(dec_q, instruction);
        registers.pc++;
        printf("cycle %d: fetching instruction %d\n", cycle, getInst(instruction));
    }
}

void decode(struct Queue* dec_q, struct InstQueue* ex_q){
    printf("cycle %d: decoding instruction %d\n", cycle, getInst(front(dec_q)));

    if(cycle%2 == 1) {
        int temp;
        struct decodedInstruction decodedInstruction;
        int instruction = dequeue(dec_q);

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

        decodedInstruction.instruction = instruction;

        enqueueInst(ex_q, decodedInstruction);
    }
}

void execute(struct InstQueue* ex_q, struct Queue* memrow_q, struct Queue* memop_q, struct Queue* wbi_q, struct Queue* wbop_q){
    printf("cycle %d: executing instruction %d\n", cycle, getInst(frontInst(ex_q).instruction));
    
    if(cycle%2 == 1) {
        struct decodedInstruction instruction = dequeueInst(ex_q);
        // Add
        if (instruction.opcode == 0) {
            int result = registers.GPR[instruction.r2] + registers.GPR[instruction.r3];
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Subtract
        if (instruction.opcode == 1) {
            int result = registers.GPR[instruction.r2] - registers.GPR[instruction.r3];
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Multiply
        if (instruction.opcode == 2) {
            int result = registers.GPR[instruction.r2] * registers.GPR[instruction.r3];
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
            if (registers.GPR[instruction.r1] == registers.GPR[instruction.r2]) {
                registers.pc = instruction.address;
            }
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, -1);
            enqueue(wbop_q, -1);
        }

        // And
        if (instruction.opcode == 5) {
            int result = registers.GPR[instruction.r2] & registers.GPR[instruction.r3];
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Exclusive Or Immediate
        if (instruction.opcode == 6) {
            int result = registers.GPR[instruction.r2] ^ instruction.immediate;
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Jump
        if (instruction.opcode == 7) {
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, -1);
            enqueue(wbop_q, -1);
            registers.pc = instruction.address;
        }

        // Logical Shift Left
        if (instruction.opcode == 8) {
            int result = registers.GPR[instruction.r2] << instruction.shamt;
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Logical Shift Right
        if (instruction.opcode == 9) {
            int result = registers.GPR[instruction.r2] >> instruction.shamt;
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Move to Register
        if (instruction.opcode == 10) {
            int result = registers.GPR[instruction.r2];
            enqueue(memrow_q, -1);
            enqueue(memop_q, -1);
            enqueue(wbi_q, instruction.r1);
            enqueue(wbop_q, result);
        }

        // Move to Memory
        if (instruction.opcode == 11) {
            int result = registers.GPR[instruction.r1];
            enqueue(memrow_q, instruction.address);
            enqueue(memop_q, result);
            enqueue(wbi_q, -1);
            enqueue(wbop_q, -1);
        }
    }
}

int memoryLoad(int rowIndex){
    //returns value in this address
    return memory.rows[rowIndex];
}

void memoryStore(struct Queue* memrow_q, struct Queue* memop_q, struct Queue* wbi_q, struct Queue* wbop_q){
    //stores operand in rowIndex
    printf("cycle %d: memory store\n", cycle);

    if(!isEmpty(memop_q)) {
        if(front(memop_q) != -1) {
            memory.rows[dequeue(memrow_q)] = dequeue(memop_q);
        }
        else {
            dequeue(memrow_q);
            dequeue(memop_q);
        }
    }
    if(front(wbop_q) == -1)
        wbFlag = 2;
    else
        wbFlag = 1;
}

void writeBack(struct Queue* wbi_q, struct Queue* wbop_q){
    //write result(operand) back to GPR defined by the GPRIndex
    printf("cycle %d: writeback\n", cycle);
    
    int GPR = dequeue(wbi_q);
    int operand = dequeue(wbop_q);
    if(wbFlag == 2 || GPR == 0)
        return;
    // GPR - 1 because R1 is at index 0
    registers.GPR[GPR - 1] = operand;
}



//parsing
    //1-import text file into a string array
    char** readFile(int programNum){
        FILE *file;
        char line[100];

        // Open the file
        char fileName[64];
        sprintf(fileName, "Program_%d.txt", programNum); //Concatenate the filename
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
    //2-turn each string instruction to binary integer
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
        char **file = readFile(5); //change temp value
        int length = sizeof(file) / sizeof(char*);
        for(int i =0; i< length; i++){
            printf("Line: %s /n" , file[i]);
            int instruction = instructionToBinary(file[i]);
            printf("instruction: %i /n" , instruction);
        }
    }


//tesing
void main() {

    printf("instruction1:%i /n",instructionToBinary("ADD R1 R2 R3"));

    //printf("%d \n" , atoi("29"));
    
    /*char s1[50] = "hello ";
    char s2[] = "world";
    strcat(s1,s2);
    printf("%s \n", s1);*/
    //splitInstruction();

    /*dec_q = createQueue(16);
    ex_q = createInstQueue(16);
    memrow_q = createQueue(16);
    memop_q = createQueue(16);
    wbi_q = createQueue(16);
    wbop_q = createQueue(16);

    memory.rows[0] = 0b00000000100100001010000000000000;
    memory.rows[1] = 0b00000001000100001010000000000000;
    memory.rows[2] = 0b00000001100100001010000000000000;
    memory.rows[3] = 0b00000010000100001010000000000000;
    memory.rows[4] = 0b00000010100100001010000000000000;
    memory.rows[5] = 0b00000011000100001010000000000000;
    memory.rows[6] = 0b00000011100100001010000000000000;

    while(cycle <= 19) {
        if(wbFlag != 0) {
            wbFlag = 0;
            writeBack(wbi_q, wbop_q);
        }
        if(cycle%2 == 0 && !isEmpty(memop_q)) {
            memoryStore(memrow_q, memop_q, wbi_q, wbop_q);
        }
        if(!isInstEmpty(ex_q)) {
            execute(ex_q, memrow_q, memop_q, wbi_q, wbop_q);
        }
        if(!isEmpty(dec_q)) {
            decode(dec_q, ex_q);
        }
        if(cycle%2 == 1) {
            fetch(dec_q);
        }
        cycle++;
    }*/
}