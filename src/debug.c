#include <stdio.h>

#include "debug.h"

// Easy function to print out 1-byte instructions and return the new offset.
static int simpleInstruction(const char * name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int constantInstruction(const char * name, Chunk * chunk, int offset) {
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

void disassembleChunk(Chunk * chunk, const char * name) {
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;) {
        // recall that instructions can be different size, hence not simply incrementing here
        offset = disassembleInstruction(chunk, offset);
    }
}

int disassembleInstruction(Chunk * chunk, int offset) {
    printf("%04d ", offset);
    if (offset > 0 &&
        chunk->lines[offset] == chunk->lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk->lines[offset]);
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_NOP:
            return simpleInstruction("OP_NOP", offset);
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}