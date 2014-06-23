#ifndef _DVBLIB_H
#define _DVBLIB_H

enum DVB_TYPE {
    DVB_TYPE_TERRESTIAL,
    DVB_TYPE_SATELLITE,
    DVB_TYPE_CABLE,
};

enum DVB_STATUS {
    DVB_STATUS_HAS_SIGNAL = 0x1,
    DVB_STATUS_HAS_CARRIER = 0x2,
    DVB_STATUS_HAS_VITERBI = 0x4,
    DVB_STATUS_HAS_SYNC = 0x8,
    DVB_STATUS_HAS_LOCK = 0x10,
    DVB_STATUS_TIMEDOUT = 0x20,
    DVB_STATUS_REINIT = 0x40,
};

enum DVB_STREAM_TYPE {
    DVB_STREAM_VIDEO,
    DVB_STREAM_AUDIO,
    DVB_STREAM_TELETEXT,
    DVB_STREAM_SUBTITLE,
    DVB_STREAM_PCR,
    DVB_STREAM_OTHER
};

typedef struct {
    int fd_frontend;
    int fd_demuxer;
    char error[128];
    char name[128];
    int type;
} dvb_device;

int dvb_open(dvb_device *dev, int dev_id, int frontend_id, int demuxer_id);
void dvb_close(dvb_device *dev);
int dvb_tune(dvb_device *dev, size_t frequency);
int dvb_get_status(dvb_device *dev);
const char* dvb_get_error(dvb_device *dev);
const char* dvb_get_type_str(dvb_device *dev);
const char* dvb_get_status_str(int status_id);
int dvb_init_pes_stream(dvb_device *dev, int pid, int pes_type);
int dvb_init_section_stream(dvb_device *dev, int pid);
ssize_t dvb_read_stream(dvb_device *dev, char *buffer, size_t size);
void dvb_stop_stream(dvb_device *dev);


#endif // _DVBLIB_H
