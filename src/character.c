#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "character.h"

signed char bonusValueForAttribute(attrT a)
{
    return -3 + (a / 3);
}

char *bonusStrForAttribute(attrT a)
{
    static char ret[6];
    signed char b = bonusValueForAttribute(a);
    strcpy(ret, "     ");
    if (b > 0)
    {
        sprintf(ret, "(+%d)", b);
    }
    else if (b < 0)
    {
        sprintf(ret, "(%d)", b);
    }
    return ret;
}