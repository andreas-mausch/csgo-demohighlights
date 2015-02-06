#include <stdio.h>
#include <ctype.h>

void hexdump(const void *address, long  length)
{
	char buffer[100];
	for (int rowStart = 0; rowStart < length; rowStart += 16)
	{
		int rowLength = 16;

		if (length - rowStart < rowLength)
		{
			rowLength = length - rowStart;
		}

		sprintf_s(buffer, 100, " >                            "
			"                      "
			"    %08lX", reinterpret_cast<const char *>(address) + rowStart);

		int indent = 1;
		int relPosition = 0;
		int index = 1 + indent;
		for (int c = 0; c < rowLength; c++, index += 2)
		{
			int index2 = 53 - 15 + indent + c;

			char character = reinterpret_cast<const char *>(address)[rowStart + c];
			sprintf_s(buffer + index, 100, "%02X ", static_cast<unsigned short>(character));

			if(!isprint(character))  character = '.'; // nonprintable char
			buffer[index2] = character;

			if (!(++relPosition & 3))     // extra blank after 4 bytes
			{  index++; buffer[index+2] = ' '; }
		}

		if (!(relPosition & 3)) index--;

		buffer[index  ]   = '<';
		buffer[index+1]   = ' ';

		printf("%s\n", buffer);
	}
}
