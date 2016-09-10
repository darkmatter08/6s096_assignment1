#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0
#define OUTPUT_WIDTH 50

typedef union float_bits {
	float f;
	uint8_t bits[sizeof(float)];
} FLOAT_BITS_UNION;

void handle_file(char in_file[], char out_file[]);
char* get_binary_rep(float* base_10);
// void print_byte(uint8_t byte, int flag);
int sprintf_byte(char* buffer, uint8_t byte, int flag);

const char *bit_rep[16] = {
	[ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
	[ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
	[ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
	[12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

// flag indicates whether the leading 4 bits of the byte should
// should only print the last 3 bits (only for mantissa_2)
int sprintf_byte(char* buffer, uint8_t byte, int flag)
{
	if(flag) {
		return sprintf(buffer, "%s%s", (bit_rep[byte >> 4]) + strlen(bit_rep[byte >> 4]) - 3, bit_rep[byte & 0x0F]);
	} else {
		return sprintf(buffer, "%s%s", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
	}
}

char* get_binary_rep(float* base_10) {
	FLOAT_BITS_UNION data;
	data.f = *base_10;
	/*
	x86 is Little Endian - The bytes are stored backwards
	but the bits within the bytes are stored forward (normal)
	[3        ][2        ][1        ][0        ]
 	 SEEE EEEE  EMMM MMMM  MMMM MMMM  MMMM MMMM
	*/
	
	// get sign bit (MSB)
	uint8_t sign = data.bits[3] >> 7;
	if(DEBUG)
		printf("sign: %i\n", sign);

	uint8_t exponent = (data.bits[3] << 1) + (data.bits[2] >> 7);
	if(DEBUG)
		printf("exponent: %i\n", exponent);

	// mask with 0xFF so that C does not use an additional byte for the shifting.
	uint8_t mantissa_2 = (((data.bits[2] << 1) & 0xFF) >> 1);
	
	uint8_t mantissa_1 = ((data.bits[1]));

	uint8_t mantissa_0 = ((data.bits[0]));

	uint32_t mantissa = (mantissa_2 << 16) + (mantissa_1 << 8) + (mantissa_0);
	if(DEBUG)
		printf("mantissa: %i\n", mantissa);

	char* result;
	result = (char *) malloc(sizeof(char)*OUTPUT_WIDTH);
	int length = 0;
	
	if(sign){
		length += sprintf(result+length, "-");
	}

	length += sprintf(result+length, "1.");
	// drop first 0 in mantissa_2
	length += sprintf_byte(result+length, mantissa_2, 1);
	length += sprintf_byte(result+length, mantissa_1, 0);
	length += sprintf_byte(result+length, mantissa_0, 0);
	
	// length += sprintf(result+length, "1.");
	length += sprintf(result+length, " * 2^%i\n", exponent-127);
	return result;
}

void handle_file(char in_file[], char out_file[]) {
	FILE* f = fopen(in_file, "r");
	FILE* check = fopen(out_file, "r");

	int lines_to_read;
	fscanf(f, "%d", &lines_to_read);
	printf("Handling %s\n", in_file);

	for(int i = 0; i < lines_to_read; ++i) {
		// read in data
		float data;
		fscanf(f, "%f", &data);
		printf("%f\n", data);

		// get output representation
		char* output = get_binary_rep(&data);
		printf("%s", output);

		// get check data
		char check_data[OUTPUT_WIDTH];
		fgets(check_data, OUTPUT_WIDTH, check);
		printf("%s", check_data);
		
		// verify match
		if(!strcmp(output, check_data)) { // strings match
			printf("pass\n\n");
		} else {
			printf("fail\n\n");
		}

		free(output);
	}

	fclose(f);
	return;
}

int main(int argc, char* argv[]) {
	int max_file = 7;
	if(argc > 1) {
		max_file = atoi(argv[1]);
	}

	char in_file[14];
	char out_file[15];
	for (int i = 1; i <= max_file; ++i) {
		sprintf(in_file, "floating.%d.in", i);
		sprintf(out_file, "floating.%d.out", i);
		handle_file(in_file, out_file);
	}
	return(0);
}
