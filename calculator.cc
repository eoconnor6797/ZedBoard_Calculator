#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/mman.h>

// Physical base address of GPIO
const unsigned gpio_address = 0x400d0000;

// Length of memory-mapped IO window
const unsigned gpio_size = 0xff;

const int gpio_led1_offset = 0x12C; // Offset for LED1
const int gpio_led2_offset = 0x130; // Offset for LED2
const int gpio_led3_offset = 0x134; // Offset for LED3
const int gpio_led4_offset = 0x138; // Offset for LED4
const int gpio_led5_offset = 0x13C; // Offset for LED5
const int gpio_led6_offset = 0x140; // Offset for LED6
const int gpio_led7_offset = 0x144; // Offset for LED7
const int gpio_led8_offset = 0x148; // Offset for LED8

const int gpio_sw1_offset = 0x14C; // Offset for Switch 1
const int gpio_sw2_offset = 0x150; // Offset for Switch 2
const int gpio_sw3_offset = 0x154; // Offset for Switch 3
const int gpio_sw4_offset = 0x158; // Offset for Switch 4
const int gpio_sw5_offset = 0x15C; // Offset for Switch 5
const int gpio_sw6_offset = 0x160; // Offset for Switch 6
const int gpio_sw7_offset = 0x164; // Offset for Switch 7
const int gpio_sw8_offset = 0x168; // Offset for Switch 8

const int gpio_pbtnl_offset = 0x16C; // Offset for left push button
const int gpio_pbtnr_offset = 0x170; // Offset for right push button
const int gpio_pbtnu_offset = 0x174; // Offset for up push button
const int gpio_pbtnd_offset = 0x178; // Offset for down push button
const int gpio_pbtnc_offset = 0x17C; // Offset for center push button 

using namespace std;

char *Initialize(int *fd) {
	*fd = open( "/dev/mem", O_RDWR);
	return (char *) mmap(
			NULL,
			gpio_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			*fd,
			gpio_address);
}
void Finalize(char *ptr, int fd) {
	munmap(ptr, gpio_size);
	close(fd);
}

int RegisterRead(char *ptr, int offset) {
	return * (int *) (ptr + offset);
}

void RegisterWrite(char *ptr, int offset, int value) {
	* (int *) (ptr + offset) = value;
}

void SetLedNumber(char *ptr, int value) {
	RegisterWrite(ptr, gpio_led1_offset, value % 2);
	RegisterWrite(ptr, gpio_led2_offset, (value / 2) % 2);
	RegisterWrite(ptr, gpio_led3_offset, (value / 4) % 2);
	RegisterWrite(ptr, gpio_led4_offset, (value / 8) % 2);
	RegisterWrite(ptr, gpio_led5_offset, (value / 16) % 2);
	RegisterWrite(ptr, gpio_led6_offset, (value / 32) % 2);
	RegisterWrite(ptr, gpio_led7_offset, (value / 64) % 2);
	RegisterWrite(ptr, gpio_led8_offset, (value / 128) % 2);
} 

int compute(int op, int xx, int yy) {
	int answer = 0;
	switch(op) {
		case 0:
			answer = xx + yy;
			break;
		case 1:
			answer = xx - yy;
			break;
		case 2:
			answer = xx * yy;
			break;
		case 3:
			answer = xx / yy;
			break;
		default:
			return -1;
	}

	if (answer < 0) {
		answer *= -1;
	}

	return answer;
}

int ReadSwitches(char *ptr) {
	int sum = 0;
	if (RegisterRead(ptr, gpio_sw1_offset)) {
		sum += 1;
	}
	if (RegisterRead(ptr, gpio_sw2_offset)) {
		sum += 2;
	}

	if (RegisterRead(ptr, gpio_sw3_offset)) {
		sum += 4;
	}

	if (RegisterRead(ptr, gpio_sw4_offset)) {
		sum += 8;
	}
	if (RegisterRead(ptr, gpio_sw5_offset)) {
		sum += 16;
	}
	if (RegisterRead(ptr, gpio_sw6_offset)) {
		sum += 32;
	}
	if (RegisterRead(ptr, gpio_sw7_offset)) {
		sum += 64;
	}
	if (RegisterRead(ptr, gpio_sw8_offset)) {
		sum += 128;
	}

	return sum;
}

int main() {

	// Initialize
	int fd;
	char *ptr = Initialize(&fd);
	// Check error
	if (ptr == MAP_FAILED) {
		cout<<"Mapping I/O memory failed - Did you run with 'sudo'?\n";
		return -1;
	}
	int num = 0;
	int op = -1;
	int num2 = 0;

	cout<<"\n\n";
	cout<<"This is a simple calculator operated by the ZedBoard.\n";
	cout<<"It can perform the operations of addition, subtraction, multiplication,"; 
	cout<<"\nand division.\n";
	cout<<"\nThe max number is 255, if any operation results in an integer greater than that, overflow will occur.\n";
	cout<<"\nIf subtraction results in a number less than zero, the absolute value will be taken.\n";
	cout<<"\nAll division is integer division, meaning if the result of dividing two numbers \n";
	cout<<"is a decimal, it will be rounded down to the nearest integer.\n";
		while(1) {
			int not_pressed = 1;
			cout<<"\nMove ZedBoard switches to represent desired binary number\n";
			cout<<"\nWhen ready press center button\n";
			while(!RegisterRead(ptr, gpio_pbtnc_offset)) {
			}
			num = ReadSwitches(ptr);
			cout<<"First Number = "<<num<<"\n";
			cout<<"\nSelect which operation to preform:\n";
			cout<<"Left Button: Addition\n";
			cout<<"Right Button: Subtraction\n";
			cout<<"Up Button: Multiplication\n";
			cout<<"Down Button: Division\n";
			while(not_pressed) {
				if (RegisterRead(ptr, gpio_pbtnl_offset)) {
					op = 0;
					not_pressed = 0;
					cout<<"\nOperation is addition\n";
				}
				if (RegisterRead(ptr, gpio_pbtnr_offset)) {
					op = 1;
					not_pressed = 0;
					cout<<"\nOperation is subtraction\n";
				}	       
				if (RegisterRead(ptr, gpio_pbtnu_offset)) {
					op = 2;
					not_pressed = 0;
					cout<<"\nOperation is multiplication\n";
				}	       
				if (RegisterRead(ptr, gpio_pbtnd_offset)) {
					op = 3;
					not_pressed = 0;
					cout<<"\nOperation is division\n";
				}	       
			}	       
			cout<<"\nMove switches to represent desired binary number\n";	
			cout<<"\nWhen ready press up button\n";
			while(!RegisterRead(ptr, gpio_pbtnu_offset)) {
			}
			num2 = ReadSwitches(ptr);
			cout<<"Second Number = "<<num2<<"\n";
			int answer = compute(op, num, num2);
			if (answer == -1) {
				cout<<"Not a valid operation....Exiting";
				return -1;

			}

			SetLedNumber(ptr, answer);
			cout<<"\nAnswer = "<<answer<<"\n";

		}
	return 0;
}
