/* 
 *	Copyright (C) 2009 cooleyes
 *	eyes.cooleyes@gmail.com 
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "mp4_read.h"

typedef char* (*f_mp4_read_fill_buffer)(struct mp4_read_struct *, int );
static f_mp4_read_fill_buffer p_mp4_read_fill_buffer;


static void zero_set_mp4_read_output_struct(struct mp4_read_output_struct *p) {
	p->size = 0;
	p->data = 0;
	p->timestamp = 0;
}

static void copy_mp4_read_output_struct(struct mp4_read_output_struct *dest, struct mp4_read_output_struct *src) {
	dest->size = src->size;
	dest->data = src->data;
	dest->timestamp = src->timestamp;
}

static int in_mp4_read_queue(struct mp4_read_output_struct* queue, unsigned int* queue_size, unsigned int* queue_rear, unsigned int queue_max, struct mp4_read_output_struct* item) {
	if ( *queue_size+1 > queue_max )
		return 0;
	copy_mp4_read_output_struct(&queue[*queue_rear], item);
	*queue_rear = (*queue_rear+1)%queue_max;
	*queue_size += 1;
	return 1;
}

static int out_mp4_read_queue(struct mp4_read_output_struct* queue, unsigned int* queue_size, unsigned int* queue_front, unsigned int queue_max, struct mp4_read_output_struct* item) {
	if ( *queue_size == 0 )
		return 0;
	copy_mp4_read_output_struct(item, &queue[*queue_front]);
	zero_set_mp4_read_output_struct(&queue[*queue_front]);
	*queue_front = (*queue_front+1)%queue_max;
	*queue_size -= 1;
	return 1;
}

static void clear_mp4_read_queue(struct mp4_read_output_struct* queue, unsigned int* queue_size, unsigned int* queue_front, unsigned int* queue_rear, unsigned int queue_max) {
	unsigned int i;
	for(i=0; i<queue_max; i++) {
		if ( queue[i].data )
			free_64(queue[i].data);
		zero_set_mp4_read_output_struct(&queue[i]);
	}
	*queue_size = 0;
	*queue_front = 0;
	*queue_rear = 0;
}

void mp4_read_safe_constructor(struct mp4_read_struct *p) {
	mp4_file_safe_constructor(&p->file);

	p->reader = 0;
	
	p->current_audio_track = 0;
	
	p->current_sample = 0;
	
	p->v_reader = 0;
	p->a_reader = 0;
	p->current_video_sample = 0;
	p->current_audio_sample = 0;
	
	int i;
	for(i=0; i<MP4_VIDEO_QUEUE_MAX; i++)
		zero_set_mp4_read_output_struct(&(p->video_queue[i]));
		
	p->video_queue_front = 0;
	p->video_queue_rear = 0;
	p->video_queue_size = 0;
		
	for(i=0; i<MP4_AUDIO_QUEUE_MAX; i++)
		zero_set_mp4_read_output_struct(&(p->audio_queue[i]));
	
	p->audio_queue_front = 0;
	p->audio_queue_rear = 0;
	p->audio_queue_size = 0;

}


void mp4_read_close(struct mp4_read_struct *p) {
	
	mp4_file_close(&p->file);

	if (!(p->reader))
		buffered_reader_close(p->reader);
		
	if (!(p->v_reader))
		buffered_reader_close(p->v_reader);
		
	if (!(p->a_reader))
		buffered_reader_close(p->a_reader);
	
	int i;
	for(i=0; i<MP4_VIDEO_QUEUE_MAX; i++) {
		if ( p->video_queue[i].data )
			free_64(p->video_queue[i].data);
	}
		
	for(i=0; i<MP4_AUDIO_QUEUE_MAX; i++) {
		if ( p->audio_queue[i].data )
			free_64(p->audio_queue[i].data);
	}
	
	
	mp4_read_safe_constructor(p);
}

char *mp4_read_fill_buffer_not_interlace(struct mp4_read_struct *p, int track_id) {
	int video_track = p->file.video_track_id;
	int audio_track = p->file.audio_track_ids[p->current_audio_track];
	int current_track = track_id;
	int track;
	int tmp;
	unsigned int track_sample; 
	uint64_t timestamp;
	if ( current_track == video_track ) {
		if ( p->current_video_sample >= p->file.sample_count )
			return "mp4_read_fill_buffer: eof";
		track = (int)(( p->file.samples[p->current_video_sample].sample_index & 0xFF000000 ) >> 24);
		if ( track != video_track )
			return "mp4_read_fill_buffer: eof";
		
		struct mp4_read_output_struct packet;
		packet.size = p->file.samples[p->current_video_sample].sample_size;
		packet.data = malloc_64(packet.size);
		if (packet.data == 0) {
			return "mp4_read_fill_buffer: can not malloc_64 data buffer";
		}
		if (buffered_reader_read(p->v_reader, packet.data, packet.size) != (uint32_t)(packet.size)) {
			free_64(packet.data);
			packet.data = 0;
			return("mp4_read_fill_buffer: can not read data");
		}
			
		track_sample = p->file.samples[p->current_video_sample].sample_index & 0x00FFFFFF;
		timestamp = 1000LL;
		timestamp *= track_sample;
		timestamp *= p->file.video_scale;
		timestamp /= p->file.video_rate;
		packet.timestamp = (int)timestamp;
		tmp = in_mp4_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_rear, MP4_VIDEO_QUEUE_MAX, &packet);
		if ( tmp == 0 )  {
			free_64(packet.data);
			return("mp4_read_fill_buffer: queue is full");
		}
		p->current_video_sample++;
	}
	else {
		if ( p->current_audio_sample >= p->file.sample_count )
			return "mp4_read_fill_buffer: eof";
		track = (int)(( p->file.samples[p->current_audio_sample].sample_index & 0xFF000000 ) >> 24);
		if ( track != audio_track )
			return "mp4_read_fill_buffer: eof";
			
		struct mp4_read_output_struct packet;
		packet.size = p->file.samples[p->current_audio_sample].sample_size;
		packet.data = malloc_64(packet.size);
		if (packet.data == 0) {
			return "mp4_read_fill_buffer: can not malloc_64 data buffer";
		}
		if (buffered_reader_read(p->a_reader, packet.data, packet.size) != (uint32_t)(packet.size)) {
			free_64(packet.data);
			packet.data = 0;
			return("mp4_read_fill_buffer: can not read data");
		}
			
		track_sample = p->file.samples[p->current_audio_sample].sample_index & 0x00FFFFFF;
		timestamp = 1000LL;
		timestamp *= track_sample;
		timestamp *= p->file.audio_resample_scale;
		timestamp /= p->file.audio_rate;
		packet.timestamp = (int)timestamp;
		tmp = in_mp4_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_rear, MP4_AUDIO_QUEUE_MAX, &packet);
		if ( tmp == 0 )  {
			free_64(packet.data);
			return("mp4_read_fill_buffer: queue is full");
		}
		p->current_audio_sample++;
	}
	return(0);
}

char *mp4_read_fill_buffer_interlace(struct mp4_read_struct *p, int track_id) {
	
	int video_track = p->file.video_track_id;
	int audio_track = p->file.audio_track_ids[p->current_audio_track];
	int current_track = track_id;
	int track;
	int tmp;
	unsigned int track_sample; 
	uint64_t timestamp;
	while(1) {
		if ( p->current_sample >= p->file.sample_count )
			return "mp4_read_fill_buffer: eof";
		track = (int)(( p->file.samples[p->current_sample].sample_index & 0xFF000000 ) >> 24);
		if(track == video_track || track == audio_track){
			struct mp4_read_output_struct packet;
			packet.size = p->file.samples[p->current_sample].sample_size;
			packet.data = malloc_64(packet.size);
			if (packet.data == 0) {
				return "mp4_read_fill_buffer: can not malloc_64 data buffer";
			}
			if (buffered_reader_read(p->reader, packet.data, packet.size) != (uint32_t)(packet.size)) {
				free_64(packet.data);
				packet.data = 0;
				return("mp4_read_fill_buffer: can not read data");
			}
			
			track_sample = p->file.samples[p->current_sample].sample_index & 0x00FFFFFF;
			if ( track  == video_track ) {
				timestamp = 1000LL;
				timestamp *= track_sample;
				timestamp *= p->file.video_scale;
				timestamp /= p->file.video_rate;
				packet.timestamp = (int)timestamp;
				tmp = in_mp4_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_rear, MP4_VIDEO_QUEUE_MAX, &packet);
			}
			else {
				timestamp = 1000LL;
				timestamp *= track_sample;
				timestamp *= p->file.audio_resample_scale;
				timestamp /= p->file.audio_rate;
				packet.timestamp = (int)timestamp;
				tmp = in_mp4_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_rear, MP4_AUDIO_QUEUE_MAX, &packet);
			}
			if ( tmp == 0 )  {
				free_64(packet.data);
				return("mp4_read_fill_buffer: queue is full");
			}
			p->current_sample++;
			if ( track == current_track )
				return(0);
		}
		else {
			buffered_reader_seek(p->reader,
				buffered_reader_position(p->reader) + p->file.samples[p->current_sample].sample_size);
			p->current_sample++;
		}
	}
	
	return(0);
}

char *mp4_read_open(struct mp4_read_struct *p, char *s) {
	
	mp4_read_safe_constructor(p);

	char *result = mp4_file_open(&p->file, s);
	if (result != 0) {
		mp4_read_close(p);
		return(result);
	}

	if ( p->file.is_not_interlace) {
		p_mp4_read_fill_buffer = mp4_read_fill_buffer_not_interlace;
		p->v_reader = buffered_reader_open(s, 64*1024, 0, 0x30);
		if (!p->v_reader) {
			mp4_read_close(p);
			return("mp4_read_open: can't open file(video)");
		}
		p->a_reader = buffered_reader_open(s, 32*1024, 0, 0x30);
		if (!p->a_reader) {
			mp4_read_close(p);
			return("mp4_read_open: can't open file(audio)");
		}
		
		buffered_reader_seek(p->v_reader, p->file.first_video_offset);
		p->current_video_sample = p->file.first_video_sample;
		buffered_reader_seek(p->a_reader, p->file.first_audio_offset);
		p->current_audio_sample = p->file.first_audio_sample;
		
	}
	else {
		p_mp4_read_fill_buffer = mp4_read_fill_buffer_interlace;
		p->reader = buffered_reader_open(s, 96*1024, 0, 0x30);
		if (!p->reader) {
			mp4_read_close(p);
			return("mp4_read_open: can't open file");
		}
	
		buffered_reader_seek(p->reader, p->file.first_video_offset);
		p->current_sample = p->file.first_video_sample;
	}

	return(0);
}

char *mp4_read_fill_buffer(struct mp4_read_struct *p, int track_id) {
	return p_mp4_read_fill_buffer(p, track_id);
}

int mp4_next_audio_frame(struct mp4_read_struct *p, int current_video_frame) {     
        double timestamp = 1.0f*current_video_frame;
        timestamp *= p->file.video_scale;
        timestamp *= p->file.audio_rate;
        timestamp /= p->file.video_rate;
        timestamp /= p->file.audio_resample_scale;
        return (int)timestamp;
}

void mp4_do_seek(struct mp4_read_struct *p, struct mp4_index_struct* index) {
	if ( p->file.is_not_interlace ) {
		buffered_reader_seek(p->v_reader, index->offset);
		p->current_video_sample = index->sample_index;
		
		int next_audio_offset = buffered_reader_position(p->a_reader);
		unsigned int next_audio_sample = p->file.first_audio_sample + mp4_next_audio_frame(p, p->current_video_sample - p->file.first_video_sample);
		unsigned int i;
		if ( next_audio_sample >= p->current_audio_sample ) {
			for(i=p->current_audio_sample; i<next_audio_sample; i++)
				next_audio_offset += p->file.samples[i].sample_size;
		}
		else {
			for(i=next_audio_sample; i<p->current_audio_sample; i++)
				next_audio_offset -= p->file.samples[i].sample_size;
		}
		buffered_reader_seek(p->a_reader, next_audio_offset);
		p->current_audio_sample = next_audio_sample;
	}
	else {
		buffered_reader_seek(p->reader, index->offset);
		p->current_sample = index->sample_index;
	}
}
	

char *mp4_read_seek(struct mp4_read_struct *p, int timestamp, int last_timestamp) {
	int i;
	struct mp4_index_struct* index = 0;
	if (timestamp < 0)
		timestamp = 0;
	if (last_timestamp < 0)
		last_timestamp = 0;
	for(i=0; i < p->file.index_count-1; i++) {
		unsigned int timecode = p->file.indexes[i].timestamp;
		unsigned int next_timecode = p->file.indexes[i+1].timestamp;
		if (timestamp >= (int)timecode && timestamp < (int)next_timecode) {
			break;
		}
	}
	if ( timestamp > last_timestamp ) { 
		if ( last_timestamp >= p->file.indexes[i].timestamp ) {
			i = i+1;
			if ( i > p->file.index_count-1 )
				i = p->file.index_count-1;
		}
	}
	else if ( timestamp < last_timestamp ) {
		if ( i+1 < p->file.index_count && last_timestamp < p->file.indexes[i+1].timestamp ) {
			i = i-1;
			if ( i < 0 )
				i = 0;
		}
	}
	index = p->file.indexes + i;
	
	mp4_do_seek(p, index);
	
	clear_mp4_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, &p->audio_queue_rear, MP4_VIDEO_QUEUE_MAX);
	clear_mp4_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, &p->video_queue_rear, MP4_AUDIO_QUEUE_MAX);
	char* result = 0;
	while(1) {
		result = mp4_read_fill_buffer(p, p->file.video_track_id);
		if (result) {
			return (result);
		}
		if ( p->video_queue_size > 0 )
			break; 
	}
	return(0);
}

char *mp4_read_keyframe_forward(struct mp4_read_struct *p, int keyframes) {
	int i;
	unsigned int current_sample;
	if ( p->file.is_not_interlace ) {
		current_sample = p->current_video_sample;
	}
	else {
		current_sample = p->current_sample;
	}
	struct mp4_index_struct* index = 0;
	for(i=0; i < p->file.index_count; i++) {
		index = p->file.indexes + i;
		if ( index->sample_index > current_sample )
			keyframes--;
		if(keyframes == 0)
			break;
	}
	
	mp4_do_seek(p, index);
	
	clear_mp4_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, &p->audio_queue_rear, MP4_VIDEO_QUEUE_MAX);
	clear_mp4_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, &p->video_queue_rear, MP4_AUDIO_QUEUE_MAX);
	char* result = 0;
	while(1) {
		result = mp4_read_fill_buffer(p, p->file.video_track_id);
		if (result) {
			return (result);
		}
		if ( p->video_queue_size > 0 )
			break; 
	}
	return(0);
}

char *mp4_read_keyframe_backward(struct mp4_read_struct *p, int keyframes) {
	int i;
	unsigned int current_sample;
	if ( p->file.is_not_interlace ) {
		current_sample = p->current_video_sample;
	}
	else {
		current_sample = p->current_sample;
	}
	struct mp4_index_struct* index = 0;
	for(i=p->file.index_count-1; i >= 0; i--) {
		index = p->file.indexes + i;
		if ( index->sample_index < current_sample )
			keyframes--;
		if(keyframes == 0)
			break;
	}
	
	mp4_do_seek(p, index);
	
	clear_mp4_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, &p->audio_queue_rear, MP4_VIDEO_QUEUE_MAX);
	clear_mp4_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, &p->video_queue_rear, MP4_AUDIO_QUEUE_MAX);
	char* result = 0;
	while(1) {
		result = mp4_read_fill_buffer(p, p->file.video_track_id);
		if (result) {
			return (result);
		}
		if ( p->video_queue_size > 0 )
			break; 
	}
	return(0);
}

char *mp4_read_get_video(struct mp4_read_struct *p, struct mp4_read_output_struct *output) {
	if ( p->video_queue_size > 0 )
		out_mp4_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, MP4_VIDEO_QUEUE_MAX, output);
	else {
		char* res = mp4_read_fill_buffer(p, p->file.video_track_id);
		if (res)
			return res;
		if ( !(p->video_queue_size>0) )
			return "mp4_read_get_video: video queue is empty";
		out_mp4_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, MP4_VIDEO_QUEUE_MAX, output);
	}
	return(0);
}

char *mp4_read_get_audio(struct mp4_read_struct *p, unsigned int audio_stream, struct mp4_read_output_struct *output){
	p->current_audio_track = audio_stream;
	if ( p->audio_queue_size > 0 )
		out_mp4_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, MP4_AUDIO_QUEUE_MAX, output);
	else {
		char* res = mp4_read_fill_buffer(p, p->file.audio_track_ids[audio_stream]);
		if (res)
			return res;
		if ( !(p->audio_queue_size>0) )
			return "mp4_read_get_audio: audio queue is empty";
		out_mp4_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, MP4_AUDIO_QUEUE_MAX, output);
	}
	return(0);
}
