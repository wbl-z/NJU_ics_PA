#include "trap.h"
int main()
{
    int i = 1;
    int j = -i;
    nemu_assert(j == -i);
    HIT_GOOD_TRAP;
	return 0;
}