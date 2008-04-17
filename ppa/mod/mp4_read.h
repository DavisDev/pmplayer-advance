#ifndef __MP4_READ_H__
#define __MP4_READ_H__


#include <string.h>
#include <pspiofilemgr.h>
#include "mp4_file.h"
#include "time_math.h"
#include "common/mem64.h"
#include "common/buffered_reader.h"


struct mp4_read_struct {
	struct mp4_file_struct file;
	
	struct time_math_interleaving_struct interleaving;

	buffered_reader_t* buffered_reader;
	
	void 	     *video_output_buffer;
	void         *audio_cache_buffer;
	void	     *audio_output_buffer;
	unsigned int *audio_output_length;

};


struct mp4_read_output_struct {
	unsigned int number_of_audio_frames;
	unsigned int number_of_skip_audio_parts;
	unsigned int number_of_audio_parts;

	int first_delay;
	int last_delay;
	
	unsigned int  video_length;
	unsigned int *audio_length;

	void *video_buffer;
	void *audio_buffer;
};

struct mp4_video_read_output_struct {

	unsigned int  video_length;
	void *video_buffer;

};

struct mp4_audio_read_output_struct {
	unsigned int number_of_audio_frames;
	unsigned int number_of_skip_audio_parts;
	unsigned int number_of_audio_parts;

	int first_delay;
	int last_delay;
	
	unsigned int *audio_length;

	void *audio_buffer;
};

void mp4_read_safe_constructor(struct mp4_read_struct *p);
void mp4_read_close(struct mp4_read_struct *p);
char *mp4_read_open(struct mp4_read_struct *p, char *s);
char *mp4_read_get_video(struct mp4_read_struct *p, unsigned int packet, struct mp4_video_read_output_struct *output);
char *mp4_read_get_audio(struct mp4_read_struct *p, unsigned int packet, unsigned int audio_stream, struct mp4_audio_read_output_struct *output);

#endif
