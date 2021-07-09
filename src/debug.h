#include "types.h"

#ifdef DEBUG
word testMem(void);
#define MEMT testMem()
#else
#define MEMT
#endif