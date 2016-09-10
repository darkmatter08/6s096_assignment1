#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef union float_bits {
	float f;
	uint8_t bits[sizeof(float)];
} FLOAT_BITS_UNION;

void handle_file(char filename[]);
void print_binary_rep(float* base_10);
void print_hex(FLOAT_BITS_UNION data);
void print_byte(uint8_t byte, int flag);

const char *bit_rep[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

// flag indicates whether the leading 4 bits of the byte should
// should only print the last 3 bits (only for mantissa_2)
void print_byte(uint8_t byte, int flag)
{
    if(flag) {
    	printf("%s%s", (bit_rep[byte >> 4]) + strlen(bit_rep[byte >> 4]) - 3, bit_rep[byte & 0x0F]);
    } else {
    	printf("%s%s", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
    }
}

int main(int argc, char* argv[]) {
	int max_file = 7;
	if(argc > 1) {
		max_file = atoi(argv[1]);
	}
	
	char filename[14];
	for (int i = 1; i <= max_file; ++i) {
		sprintf(filename, "floating.%d.in", i);
		handle_file(filename);
	}
	return(0);
}

void handle_file(char filename[]) {
	FILE* f = fopen(filename, "r");

	int lines_to_read;
	fscanf(f, "%d", &lines_to_read);
	printf("First line of %s: %d\n", filename, lines_to_read);

	for(int i = 0; i < lines_to_read; ++i) {
		float data;
		fscanf(f, "%f", &data);
		printf("Line %d of %s: %f\n", i+1, filename, data);
		print_binary_rep(&data);
	}

	fclose(f);
	return;
}

void print_binary_rep(float* base_10) {
	FLOAT_BITS_UNION data;
	data.f = *base_10;
	print_hex(data);
	/*
	x86 is Little Endian - The bytes are stored backwards
	but the bits within the bytes are stored forward (normal)
	[3        ][2        ][1        ][0        ]
 	 SEEE EEEE  EMMM MMMM  MMMM MMMM  MMMM MMMM
	*/
	
	// get sign bit (MSB)
	uint8_t sign = data.bits[3] >> 7;
	printf("sign %i\n", sign);

	uint8_t exponent = (data.bits[3] << 1) + (data.bits[2] >> 7);
	printf("exponent %i\n", exponent);

	// mask with 0xFF so that C does not use an additional byte for the shifting.
	uint8_t mantissa_2 = 
		(((data.bits[2] << 1) & 0xFF) >> 1);
	
	uint8_t mantissa_1 = 
		((data.bits[1]));

	uint8_t mantissa_0 = 
		((data.bits[0]));

	uint32_t mantissa = (mantissa_2 << 16) + (mantissa_1 << 8) + (mantissa_0);
	printf("mantissa %i\n", mantissa);

	if(sign){
		printf("-");
	}
	printf("1.");
	// drop first 0 in mantissa_2
	print_byte(mantissa_2, 1);
	print_byte(mantissa_1, 0);
	print_byte(mantissa_0, 0);
	printf(" * 2^%i\n\n", exponent-127);
}

void print_hex(FLOAT_BITS_UNION data) {
	printf("0x");
	for (unsigned int i = 0; i < sizeof(float); ++i)
	{
		printf("%02X", data.bits[i]);
	}
	printf("\n");
}