#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dvblib.h"

#define READ_SIZE 4096

int main(int argc, char *argv[]) {
	dvb_device dev;
	int freq, pid, stype;
	int got, i;
	char buf[READ_SIZE+1];
	FILE *output;

	buf[READ_SIZE] = 0;
	
	// Check args ...
	if(argc < 4) {
		printf("Syntax: test <freq> <pid> <0/1 section/pes>\n");
		return 0;
	}
	freq = atoi(argv[1]);
	pid = atoi(argv[2]);
	stype = (atoi(argv[3]) > 0);

	// Output dump goes here
	output = fopen("output.txt", "w");
	if(output == 0) {
		printf("Unable to open output file\n");
		goto error_0;
	}
	
	// Open the DVB device. Expect it to be DVB-T
	if(dvb_open(&dev, 0, 0, 0)) {
		printf("%s\n", dvb_get_error(&dev));
		goto error_1;
	}
    printf("Name: %s\n", dev.name);
	printf("Type: %s\n", dvb_get_type_str(&dev));

	// Tune to requested channel
	printf("Tuning to %u\n", freq);
	if(dvb_tune(&dev, freq)) {
		printf("%s\n", dvb_get_error(&dev));
		goto error_2;
	}

	// Wait for signal to stabilize
	printf("Waiting for lock ...\n");
	while(dvb_get_status(&dev) & DVB_STATUS_HAS_LOCK) {
		sleep(1);
	}
	printf("Ok, got lock!\n");

	// Read stream and dump it to a file
	printf("Listening to PID %d\n", pid);
	if(stype == 1) {
		printf("Stream type selection: PES\n");
		dvb_init_pes_stream(&dev, pid, DVB_STREAM_SUBTITLE);
	} else {
		printf("Stream type selection: Section\n");
		dvb_init_section_stream(&dev, pid);
	}
	
	for(i = 0; i < 100; i++) {
		got = dvb_read_stream(&dev, buf, READ_SIZE);
		if(got <= 0) {
			printf("End of stream!\n");
			goto error_3;
		}
		printf("Asked for %d bytes, got %d bytes.\n", READ_SIZE, got);
		fwrite(buf, 1, got, output);
	}
    dvb_stop_stream(&dev);

	// Close handles
	dvb_close(&dev);
	fclose(output);

	// All done
    return 0;

error_3:
    dvb_stop_stream(&dev);
error_2:
	dvb_close(&dev);
error_1:
	fclose(output);
error_0:
	return 1;
}
