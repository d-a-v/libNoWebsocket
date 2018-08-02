
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <wsc.h>

char line [1024];

int main (void)
{
	const char* handshake;
	while (1)
	{
		if (!fgets(line, sizeof line, stdin))
			return 1;
		if ((handshake = ws_handshake(line)))
			break;
	}
	
	fprintf(stderr, "send: %s", handshake);
	write(1, handshake, strlen(handshake));
	
	char buf [1024];

	const char* hello = "hello, world";
	write(1, ws_out_preamble, ws_output_preamble_length(strlen(hello)));
	write(1, hello, strlen(hello));

	size_t l = 0;
	while (1)
	{
		// fill buffer
		ssize_t r = read(0, buf + l, sizeof buf - l);
		if (r <= 0)
		{
			fprintf(stderr, "peer has closed\n");
			break;
		}
		l += r;
		
		char* data = buf;
		size_t data_size = ws_decode_full_frame(&data, l);
		
		if (data_size)
		{
			fprintf(stderr, "received (now=%d(total=%d) user=%d): ", (int)r, (int)l, (int)data_size);
			for (size_t i = 0; i < data_size; i++)
				fprintf(stderr, "%c", data[i]);
			fprintf(stderr, "\n");
			
			write(1, ws_out_preamble, ws_output_preamble_length(data_size + 5));
			write(1, "echo ", 5);
			write(1, data, data_size);
				
			// shift buffer (todo: circular buffer)
			memmove(buf, data + data_size, l -= (data + data_size - buf));
		}
	}
	
	fprintf(stderr, "out\n");
	
	return 0;
}
