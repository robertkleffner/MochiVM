#include <stdio.h>

#include "debug.h"

static short getShort(uint8_t* buffer, int offset) {
    return (buffer[offset] << 8) | buffer[offset+1];
}

static uint16_t getUShort(uint8_t* buffer, int offset) {
    return (uint16_t)((buffer[offset] << 8) | buffer[offset+1]);
}

static int getInt(uint8_t* buffer, int offset) {
    return (buffer[offset] << 24) | (buffer[offset+1] << 16) | (buffer[offset+2] << 8) | buffer[offset+3];
}

// Easy function to print out 1-byte instructions and return the new offset.
static int simpleInstruction(const char * name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int byteArgInstruction(const char* name, MochiVM* vm, int offset) {
    printf("%-16s %d\n", name, vm->block->code.data[offset + 1]);
    return offset + 2;
}

static int shortArgInstruction(const char* name, MochiVM* vm, int offset) {
    uint8_t* code = vm->block->code.data;
    printf("%-16s %d\n", name, getShort(code, offset+1));
    return offset + 3;
}

static int intArgInstruction(const char* name, MochiVM* vm, int offset) {
    uint8_t* code = vm->block->code.data;
    printf("%-16s %d\n", name, getInt(code, offset+1));
    return offset + 5;
}

static int constantInstruction(const char * name, MochiVM* vm, int offset) {
    uint8_t constant = vm->block->code.data[offset + 1];
    printf("%-16s %-4d '", name, constant);
    printValue(vm, vm->block->constants.data[constant]);
    printf("'\n");
    return offset + 2;
}

static int closureInstruction(const char* name, MochiVM* vm, int offset) {
    uint8_t* code = vm->block->code.data;
    offset += 1;

    int body = getInt(code, offset); offset += 4;
    uint8_t paramCount = code[offset]; offset += 1;
    uint16_t captureCount = getUShort(code, offset); offset += 2;
    printf("%-16s %-8d %-3d %-5d ( ", name, body, paramCount, captureCount);
    for (int i = 0; i < captureCount; i++) {
        printf("%5d:%5d ", getShort(code, offset + i*4), getShort(code, offset + 2 + i*4));
    }
    printf(")\n");
    return offset + captureCount * 4;
}

static int actionInstruction(const char* name, MochiVM* vm, int offset) {
    uint8_t* code = vm->block->code.data;
    offset += 1;

    int handleId = getInt(code, offset); offset += 4;
    int handlerId = code[offset]; offset += 1;
    printf("%-16s %-8d %-3d\n", name, handleId, handlerId);
    return offset;
}

void disassembleChunk(MochiVM* vm, const char * name) {
    printf("== %s ==\n", name);

    for (int offset = 0; offset < vm->block->code.count;) {
        // recall that instructions can be different size, hence not simply incrementing here
        offset = disassembleInstruction(vm, offset);
    }
}

int disassembleInstruction(MochiVM* vm, int offset) {
    ASSERT(offset < vm->block->lines.count, "No line at the specified offset!");
    ASSERT(offset < vm->block->code.count, "No instruction at the specified offset!");

    printf("%04d ", offset);
    if (offset > 0 && vm->block->lines.data[offset] == vm->block->lines.data[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", vm->block->lines.data[offset]);
    }

    uint8_t instruction = vm->block->code.data[offset];
    switch (instruction) {
        case CODE_NOP:
            return simpleInstruction("NOP", offset);
        case CODE_ABORT:
            return byteArgInstruction("ABORT", vm, offset);
        case CODE_CONSTANT:
            return constantInstruction("CONSTANT", vm, offset);
        case CODE_NEGATE:
            return simpleInstruction("NEGATE", offset);
        case CODE_ADD:
            return simpleInstruction("ADD", offset);
        case CODE_SUBTRACT:
            return simpleInstruction("SUBTRACT", offset);
        case CODE_MULTIPLY:
            return simpleInstruction("MULTIPLY", offset);
        case CODE_DIVIDE:
            return simpleInstruction("DIVIDE", offset);
        case CODE_EQUAL:
            return simpleInstruction("EQUAL", offset);
        case CODE_GREATER:
            return simpleInstruction("GREATER", offset);
        case CODE_LESS:
            return simpleInstruction("LESS", offset);
        case CODE_TRUE:
            return simpleInstruction("TRUE", offset);
        case CODE_FALSE:
            return simpleInstruction("FALSE", offset);
        case CODE_NOT:
            return simpleInstruction("NOT", offset);
        case CODE_CONCAT:
            return simpleInstruction("CONCAT", offset);
        case CODE_STORE:
            return byteArgInstruction("STORE", vm, offset);
        case CODE_FIND: {
            uint8_t* code = vm->block->code.data;
            offset += 1;

            uint16_t frameIdx = getShort(code, offset); offset += 2;
            uint16_t varIdx = getShort(code, offset); offset += 2;
            printf("%-16s %-5d %-5d\n", "FIND", frameIdx, varIdx);
            return offset;
        }
        case CODE_FORGET:
            return simpleInstruction("FORGET", offset);
        case CODE_CALL_FOREIGN:
            return shortArgInstruction("CALL_FOREIGN", vm, offset);
        case CODE_CALL:
            return intArgInstruction("CALL", vm, offset);
        case CODE_TAILCALL:
            return intArgInstruction("TAILCALL", vm, offset);
        case CODE_OFFSET:
            return intArgInstruction("OFFSET", vm, offset);
        case CODE_RETURN:
            return simpleInstruction("RETURN", offset);
        case CODE_CLOSURE:
            return closureInstruction("CLOSURE", vm, offset);
        case CODE_RECURSIVE:
            return closureInstruction("RECURSIVE", vm, offset);
        case CODE_MUTUAL:
            return intArgInstruction("MUTUAL", vm, offset);
        case CODE_HANDLE: {
            uint8_t* code = vm->block->code.data;
            offset += 1;

            int16_t after = getShort(code, offset); offset += 2;
            int handleId = getInt(code, offset); offset += 4;
            uint8_t params = code[offset]; offset += 1;
            uint8_t handlers = code[offset]; offset += 1;
            printf("%-16s a(%d) id(%d) p(%d) h(%d)\n", "HANDLE", after, handleId, params, handlers);
            return offset;
        }
        case CODE_INJECT:
            return intArgInstruction("INJECT", vm, offset);
        case CODE_EJECT:
            return intArgInstruction("EJECT", vm, offset);
        case CODE_COMPLETE:
            return simpleInstruction("COMPLETE", offset);
        case CODE_ESCAPE:
            return actionInstruction("ESCAPE", vm, offset);
        case CODE_REACT:
            return actionInstruction("REACT", vm, offset);
        case CODE_CALL_CONTINUATION:
            return simpleInstruction("CALL_CONTINUATION", offset);
        case CODE_TAILCALL_CONTINUATION:
            return simpleInstruction("TAILCALL_CONTINUATION", offset);
        case CODE_ZAP:
            return simpleInstruction("ZAP", offset);
        case CODE_SWAP:
            return simpleInstruction("SWAP", offset);
        case CODE_LIST_NIL:
            return simpleInstruction("LIST_NIL", offset);
        case CODE_LIST_CONS:
            return simpleInstruction("LIST_CONS", offset);
        case CODE_LIST_HEAD:
            return simpleInstruction("LIST_HEAD", offset);
        case CODE_LIST_TAIL:
            return simpleInstruction("LIST_TAIL", offset);
        case CODE_LIST_IS_EMPTY:
            return simpleInstruction("LIST_IS_EMPTY", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}