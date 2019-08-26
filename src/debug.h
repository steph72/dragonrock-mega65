#ifdef DEBUG
void testMem(void);
#define MEMT testMem()
#else
#define MEMT
#endif