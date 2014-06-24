/*! \file captool.c
 * \brief A simple tool for capturing DVB data
 * \author Tuomas Virtanen
 * \copyright MIT
 * \date 2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dvblib/dvblib.h"

#define READ_SIZE 16384

void print_status(int status) {
    if(status & DVB_STATUS_HAS_SIGNAL) printf("SIGNAL ");
    if(status & DVB_STATUS_HAS_CARRIER) printf("CARRIER ");
    if(status & DVB_STATUS_HAS_VITERBI) printf("VITERBI ");
    if(status & DVB_STATUS_HAS_SYNC) printf("SYNC ");
    if(status & DVB_STATUS_HAS_LOCK) printf("LOCK ");
    if(status & DVB_STATUS_TIMEDOUT) printf("TIMEDOUT ");
    if(status & DVB_STATUS_REINIT) printf("REINIT ");
}

int main(int argc, char *argv[]) {
    dvb_device dev;
    int freq, pid, stype, ptype,amount;
    int got, i;
    char buf[READ_SIZE*2+1];
    unsigned int status = 0;
    FILE *output;
    uint32_t ber, ub;
    int16_t snr, ss;
    int done = 0;
    int rsize;

    buf[READ_SIZE] = 0;
    
    // Check args ...
    if(argc < 4) {
        printf("Syntax: captool <amount> <freq> <pid> <ts_filter> <pes_filter>\n");
        printf("\nts_filter:\n");
        printf(" - 0\tSECTION filter\n");
        printf(" - 1\tPES filter\n");
        printf("\npes_filter:\n");
        for(i = 0; i < DVB_STREAM_TYPE_COUNT; i++) {
            printf(" - %d\t%s\n", i, dvb_get_stream_type_str(i));
        }
        return 0;
    }
    amount = atoi(argv[1]);
    freq = atoi(argv[2]);
    pid = atoi(argv[3]);
    stype = (atoi(argv[4]) > 0);
    if(stype == 1) {
        if(argc < 6) {
            printf("Must define a PES filter!\n");
            return 0;
        }
        ptype = atoi(argv[5]);
        if(ptype >= DVB_STREAM_TYPE_COUNT || ptype < 0) {
            printf("Must define a PES filter!\n");
            return 0;
        }
    }
    
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

    // Set buffer size to somethign higher
    if(dvb_set_buffer_size(&dev, READ_SIZE*2)) {
        printf("%s\n", dvb_get_error(&dev));
        goto error_2;
    }

    // Tune to requested channel
    printf("Tuning to %u\n", freq);
    if(dvb_tune(&dev, freq)) {
        printf("%s\n", dvb_get_error(&dev));
        goto error_2;
    }

    // Wait for signal to stabilize
    printf("Waiting for lock ...\n");
    while(!(status & DVB_STATUS_HAS_LOCK)) {
        dvb_get_status(&dev, &status);
        printf("Status: ");
        print_status(status);
        printf("\n");

        usleep(100000);
    }
    printf("Ok, got lock!\n");

    // Read stream and dump it to a file
    printf("Listening to PID %d\n", pid);
    if(stype == 1) {
        printf("Stream type selection: PES\n");
        dvb_init_pes_stream(&dev, pid, ptype);
    } else {
        printf("Stream type selection: Section\n");
        dvb_init_section_stream(&dev, pid);
    }

    // Just read anything that is available and clear the buffer.
    // Ignore any errors
    dvb_read_stream(&dev, buf, READ_SIZE*2);
    
    // Read or die!
    while(done < amount) {
        // Read data
        rsize = ((amount - done) > READ_SIZE) ? READ_SIZE : amount - done;
        got = dvb_read_stream(&dev, buf, rsize);
        if(got < 0) {
            printf("Error: %s\n", dvb_get_error(&dev));
            goto error_3;
        }
        fwrite(buf, 1, got, output);
        done += got;

        dvb_get_snr(&dev, &snr);
        dvb_get_ber(&dev, &ber);
        dvb_get_uncorrected_blocks(&dev, &ub);
        dvb_get_signal_strength(&dev, &ss);
        dvb_get_status(&dev, &status);

        printf("READ %d, DONE %d/%d, SNR %d, SS %d, BER %u, UB %u, flags = ", got, done, amount, snr, ss, ber, ub);
        print_status(status);
        printf("\n");
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
