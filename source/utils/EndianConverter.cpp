#include "EndianConverter.h"

unsigned int endian_swap(unsigned int number)
{
	return (number>>24) | ((number<<8) & 0x00FF0000) | ((number>>8) & 0x0000FF00) | (number<<24);
}
