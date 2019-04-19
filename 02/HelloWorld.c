#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

int main(void) {
	char buffer[12];
	read(STDIN_FILENO, buffer, 12);
	write(STDOUT_FILENO, buffer, 12);
}