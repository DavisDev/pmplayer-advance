/* 
 *	Copyright (C) 2010 cooleyes
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

#include "flv1_read.h"

static void zero_set_flv1_read_output_struct(struct flv1_read_output_struct *p) {
	p->size = 0;
	p->data = 0;
	p->timestamp = 0;
}

static void copy_flv1_read_output_struct(struct flv1_read_output_struct *dest, struct flv1_read_output_struct *src) {
	dest->size = src->size;
	dest->data = src->data;
	dest->timestamp = src->timestamp;
}

static int in_flv1_read_queue(struct flv1_read_output_struct* queue, unsigned int* queue_size, unsigned int* queue_rear, unsigned int queue_max, struct flv1_read_output_struct* item) {
	if ( *queue_size+1 > queue_max )
		return 0;
	copy_flv1_read_output_struct(&queue[*queue_rear], item);
	*queue_rear = (*queue_rear+1)%queue_max;
	*queue_size += 1;
	return 1;
}

static int out_flv1_read_queue(struct flv1_read_output_struct* queue, unsigned int* queue_size, unsigned int* queue_front, unsigned int queue_max, struct flv1_read_output_struct* item) {
	if ( *queue_size == 0 )
		return 0;
	copy_flv1_read_output_struct(item, &queue[*queue_front]);
	zero_set_flv1_read_output_struct(&queue[*queue_front]);
	*queue_front = (*queue_front+1)%queue_max;
	*queue_size -= 1;
	return 1;
}

static void clear_flv1_read_queue(struct flv1_read_output_struct* queue, unsigned int* queue_size, unsigned int* queue_front, unsigned int* queue_rear, unsigned int queue_max) {
	unsigned int i;
	for(i=0; i<queue_max; i++) {
		if ( queue[i].data )
			free_64(queue[i].data);
		zero_set_flv1_read_output_struct(&queue[i]);
	}
	*queue_size = 0;
	*queue_front = 0;
	*queue_rear = 0;
}

//static uint64_t flv1_read_be64(buffered_reader_t* reader) {
//	uint8_t data[8];
//	uint64_t result = 0;
//	int i;
//	
//	buffered_reader_read(reader, data, 8);
//	for (i = 0; i < 8; i++) {
//		result |= ((uint64_t)data[i]) << ((7 - i) * 8);
//	}
//
//	return result;
//}
//
static uint32_t flv1_read_be32(buffered_reader_t* reader) {
	uint8_t data[4];
	uint32_t result = 0;
	uint32_t a, b, c, d;
	
	buffered_reader_read(reader, data, 4);
	a = (uint8_t)data[0];
	b = (uint8_t)data[1];
	c = (uint8_t)data[2];
	d = (uint8_t)data[3];
	result = (a<<24) | (b<<16) | (c<<8) | d;

	return result;
}

static uint32_t flv1_read_be24(buffered_reader_t* reader) {
	uint8_t data[4];
	uint32_t result = 0;
	uint32_t a, b, c;
	
	buffered_reader_read(reader, data, 3);
	a = (uint8_t)data[0];
	b = (uint8_t)data[1];
	c = (uint8_t)data[2];
	result = (a<<16) | (b<<8) | c;

	return result;
}
//
//static uint16_t flv1_read_be16(buffered_reader_t* reader) {
//	uint8_t data[2];
//	uint16_t result = 0;
//	uint16_t a, b;
//	
//	buffered_reader_read(reader, data, 2);
//	a = (uint8_t)data[0];
//	b = (uint8_t)data[1];
//	result = (a<<8) | b;
//
//	return result;
//}

uint8_t flv1_read_8(buffered_reader_t* reader) {
	uint8_t result;
    
	buffered_reader_read(reader, &result, 1);
    
	return result;
}

void flv1_read_safe_constructor(struct flv1_read_struct *p) {
	flv1_file_safe_constructor(&p->file);

	p->reader = 0;
	
	p->current_timecode = 0;
	
	int i;
	for(i=0; i<FLV1_VIDEO_QUEUE_MAX; i++)
		zero_set_flv1_read_output_struct(&(p->video_queue[i]));
		
	p->video_queue_front = 0;
	p->video_queue_rear = 0;
	p->video_queue_size = 0;
		
	for(i=0; i<FLV1_AUDIO_QUEUE_MAX; i++)
		zero_set_flv1_read_output_struct(&(p->audio_queue[i]));
	
	p->audio_queue_front = 0;
	p->audio_queue_rear = 0;
	p->audio_queue_size = 0;

}


void flv1_read_close(struct flv1_read_struct *p) {
	
	flv1_file_close(&p->file);

	if (p->reader)
		buffered_reader_close(p->reader);
		
	int i;
	for(i=0; i<FLV1_VIDEO_QUEUE_MAX; i++) {
		if ( p->video_queue[i].data )
			free_64(p->video_queue[i].data);
	}
		
	for(i=0; i<FLV1_AUDIO_QUEUE_MAX; i++) {
		if ( p->audio_queue[i].data )
			free_64(p->audio_queue[i].data);
	}
	
	
	flv1_read_safe_constructor(p);
}

char *flv1_read_open(struct flv1_read_struct *p, char *s) {
	
	flv1_read_safe_constructor(p);

	char *result = flv1_file_open(&p->file, s);
	if (result != 0) {
		flv1_read_close(p);
		return(result);
	}


	//p->reader = buffered_reader_open(s, 32768, 0);
	p->reader = buffered_reader_open(s, 96*1024, 0, 0x30);
	if (!p->reader) {
		flv1_read_close(p);
		return("flv1_read_open: can't open file");
	}
	
	buffered_reader_seek(p->reader, p->file.info->data_start);

	return(0);
}

char *flv1_read_fill_buffer(struct flv1_read_struct *p, unsigned char track_type) {
	
	while(1) {
		
		int32_t pos = buffered_reader_position(p->reader);
		int32_t next;
		
		if ( pos >= buffered_reader_length(p->reader) )
			return "flv1_read_fill_buffer: eof";
			
		uint8_t tag_type;
		uint32_t data_size;
		uint32_t time_stamp, time_stamp_ext;
		
		flv1_read_be32(p->reader);//prev tag size
		tag_type = flv1_read_8(p->reader);
		data_size = flv1_read_be24(p->reader);
		time_stamp = flv1_read_be24(p->reader);
		time_stamp_ext = flv1_read_8(p->reader);
		time_stamp |= (time_stamp_ext << 24);
		flv1_read_be24(p->reader); // stream Id
		
		if ( data_size == 0 )
			continue;
		
		next = data_size + buffered_reader_position(p->reader);
		
		if ( tag_type == 0x08 || tag_type == 0x09 ) {
			struct flv1_read_output_struct packet;
			int tmp;
			if ( tag_type  == 0x08 ) {
				uint8_t audio_flag = flv1_read_8(p->reader);
				if ( (audio_flag & 0xF0) == 0xA0 ) { //AAC
					uint8_t aac_packet_type = flv1_read_8(p->reader);
					if ( aac_packet_type != 0x01 ) { //skip AAC sequence header
						buffered_reader_seek(p->reader, next);
						continue;
					}
					packet.size = data_size-2;
				}
				else {
					packet.size = data_size-1;
				}
				packet.data = malloc_64(packet.size);
				if (packet.data == 0) {
					return "flv1_read_fill_buffer: can not malloc_64 data buffer";
				}
				if (buffered_reader_read(p->reader, packet.data, packet.size) != (uint32_t)(packet.size)) {
					free_64(packet.data);
					packet.data = 0;
					return("flv1_read_fill_buffer: can not read data");
				}
				packet.timestamp = (int)time_stamp;
				tmp = in_flv1_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_rear, FLV1_AUDIO_QUEUE_MAX, &packet);
			}
			else {
				uint8_t video_flag = flv1_read_8(p->reader);
				if ( (video_flag & 0x0F) == 0x07 ) { //AVC
					uint8_t avc_packet_type = flv1_read_8(p->reader);
					if ( avc_packet_type != 0x01 ) { //skip AVC sequence header
						buffered_reader_seek(p->reader, next);
						continue;
					}
					flv1_read_be24(p->reader);
					packet.size = data_size-5;
				}
				else {
					packet.size = data_size-1;
				}
				packet.data = malloc_64(packet.size);
				if (packet.data == 0) {
					return "flv1_read_fill_buffer: can not malloc_64 data buffer";
				}
				if (buffered_reader_read(p->reader, packet.data, packet.size) != (uint32_t)(packet.size)) {
					free_64(packet.data);
					packet.data = 0;
					return("flv1_read_fill_buffer: can not read data");
				}
				packet.timestamp = (int)time_stamp;
				p->current_timecode = time_stamp;
				tmp = in_flv1_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_rear, FLV1_VIDEO_QUEUE_MAX, &packet);
			}
			if ( tmp == 0 )  {
				free_64(packet.data);
				if ( tag_type  == 0x09 ) 
					return("mp4_read_fill_buffer: video queue is full");
				else
					return("mp4_read_fill_buffer: audio queue is full");
			}		
			if ( tag_type == track_type )
				return(0);
		}
		else {
			buffered_reader_seek(p->reader, next);
		}
	}
	return(0);
}

char *flv1_read_seek(struct flv1_read_struct *p, int timestamp, int last_timestamp) {
	int32_t i;
	flv1info_index_t* index = 0;
	if (timestamp < 0)
		timestamp = 0;
	if (last_timestamp < 0)
		last_timestamp = 0;
	for(i=0; i < p->file.info->total_indexes-1; i++) {
		uint64_t timecode = p->file.info->indexes[i].timecode;
		uint64_t next_timecode = p->file.info->indexes[i+1].timecode;
		if (timestamp >= (int)timecode && timestamp < (int)next_timecode) {
			break;
		}
	}
	if ( timestamp > last_timestamp ) { 
		if ( last_timestamp >= p->file.info->indexes[i].timecode ) {
			i = i+1;
			if ( i > p->file.info->total_indexes-1 )
				i = p->file.info->total_indexes-1;
		}
	}
	else if ( timestamp < last_timestamp ) {
		if ( i+1 < p->file.info->total_indexes && last_timestamp < p->file.info->indexes[i+1].timecode ) {
			i = i-1;
			if ( i < 0 )
				i = 0;
		}
	}
	index = p->file.info->indexes + i;
	
	buffered_reader_seek(p->reader, index->filepos);
	
	clear_flv1_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, &p->audio_queue_rear, FLV1_VIDEO_QUEUE_MAX);
	clear_flv1_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, &p->video_queue_rear, FLV1_AUDIO_QUEUE_MAX);
	char* result = 0;
	while(1) {
		result = flv1_read_fill_buffer(p, 0x09);
		if (result) {
			return (result);
		}
		if ( p->video_queue_size > 0 )
			break; 
	}
	return(0);
}

char *flv1_read_keyframe_forward(struct flv1_read_struct *p, int keyframes) {
	int i;
	uint64_t current_timecode;
	current_timecode = p->current_timecode;
	
	flv1info_index_t* index = 0;
	for(i=0; i < p->file.info->total_indexes; i++) {
		index = p->file.info->indexes + i;
		if ( index->timecode > current_timecode )
			keyframes--;
		if(keyframes == 0)
			break;
	}
	
	buffered_reader_seek(p->reader, index->filepos);
	
	clear_flv1_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, &p->audio_queue_rear, FLV1_VIDEO_QUEUE_MAX);
	clear_flv1_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, &p->video_queue_rear, FLV1_AUDIO_QUEUE_MAX);
	char* result = 0;
	while(1) {
		result = flv1_read_fill_buffer(p, 0x09);
		if (result) {
			return (result);
		}
		if ( p->video_queue_size > 0 )
			break; 
	}
	return(0);
}

char *flv1_read_keyframe_backward(struct flv1_read_struct *p, int keyframes) {
	int i;
	uint64_t current_timecode;
	current_timecode = p->current_timecode;
	
	flv1info_index_t* index = 0;
	for(i=p->file.info->total_indexes-1; i >= 0; i--) {
		index = p->file.info->indexes + i;
		if ( index->timecode < current_timecode )
			keyframes--;
		if(keyframes == 0)
			break;
	}
	
	buffered_reader_seek(p->reader, index->filepos);
	
	clear_flv1_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, &p->audio_queue_rear, FLV1_VIDEO_QUEUE_MAX);
	clear_flv1_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, &p->video_queue_rear, FLV1_AUDIO_QUEUE_MAX);
	char* result = 0;
	while(1) {
		result = flv1_read_fill_buffer(p, 0x09);
		if (result) {
			return (result);
		}
		if ( p->video_queue_size > 0 )
			break; 
	}
	return(0);
}

char *flv1_read_get_video(struct flv1_read_struct *p, struct flv1_read_output_struct *output) {
	if ( p->video_queue_size > 0 )
		out_flv1_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, FLV1_VIDEO_QUEUE_MAX, output);
	else {
		char* res = flv1_read_fill_buffer(p, 0x09);
		if (res)
			return res;
		if ( !(p->video_queue_size>0) )
			return "flv1_read_get_video: video queue is empty";
		out_flv1_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, FLV1_VIDEO_QUEUE_MAX, output);
	}
	return(0);
}

char *flv1_read_get_audio(struct flv1_read_struct *p, unsigned int audio_stream, struct flv1_read_output_struct *output){
	if ( p->audio_queue_size > 0 )
		out_flv1_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, FLV1_AUDIO_QUEUE_MAX, output);
	else {
		char* res = flv1_read_fill_buffer(p, 0x08);
		if (res)
			return res;
		if ( !(p->audio_queue_size>0) )
			return "flv1_read_get_audio: audio queue is empty";
		out_flv1_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, FLV1_AUDIO_QUEUE_MAX, output);
	}
	return(0);
}
