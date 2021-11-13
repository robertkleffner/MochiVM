// Test the foreign function capabilities of the VM

BEGIN_TEST("LibUV foreign function tests")

WRITE_INST(CALL_FOREIGN, 123)
WRITE_BYTE(0, 123)
WRITE_BYTE(0, 123)
WRITE_INST(CALL_FOREIGN, 124)
WRITE_BYTE(0, 124)
WRITE_BYTE(1, 124)
WRITE_INST(ABORT, 103)
WRITE_BYTE(0, 103)

VERIFY_FRAMES(0)
VERIFY_STACK(0)

END_TEST();




BEGIN_TEST("LibUV basic timer test");

CONSTANT(TEST_DOUBLE_VAL(1000))
CONSTANT(TEST_DOUBLE_VAL(2))

WRITE_INT_INST(CALL, 10, 1); // 5
WRITE_INT_INST(TAILCALL, 33, 2); //10

// main
WRITE_INT_INST(CLOSURE, 27, 3);
WRITE_BYTE(0, 3);
WRITE_SHORT(0, 3); // 18

WRITE_INST(CONSTANT, 3);
WRITE_BYTE(0, 3);

WRITE_INST(CALL_FOREIGN, 3);
WRITE_BYTE(0, 3);
WRITE_BYTE(0, 3); // 23

WRITE_INST(CALL_FOREIGN, 4);
WRITE_SHORT(2, 4);

WRITE_INST(RETURN, 4);

// callback
WRITE_INST(CALL_FOREIGN, 5);
WRITE_SHORT(1, 5);

WRITE_INST(CONSTANT, 5);
WRITE_BYTE(1, 5);
WRITE_INST(RETURN, 5);

// end
WRITE_INST(ABORT, 15);
WRITE_BYTE(0, 15);

VERIFY_FRAMES(0);
VERIFY_STACK(1);
VERIFY_NUMBER(2);

END_TEST();




BEGIN_TEST("SDL init and quit test");

CONSTANT(TEST_I32_VAL(SDL_INIT_EVERYTHING));

WRITE_INST(CONSTANT, 0);
WRITE_BYTE(0, 0);
WRITE_INST(CALL_FOREIGN, 1);
WRITE_SHORT(3, 1);

WRITE_INST(CALL_FOREIGN, 2);
WRITE_SHORT(4, 2);

WRITE_INST(ABORT, 3);
WRITE_BYTE(0, 3);

VERIFY_STACK(1);
VERIFY_FRAMES(0);
VERIFY_NUMBER(0);

END_TEST();