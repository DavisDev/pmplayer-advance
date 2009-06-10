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

#include "mkv_read.h"
#include "ebml_id.h"
#include "subtitle_parse.h"
#include "common/libminiconv.h"

static void zero_set_mkv_read_output_struct(struct mkv_read_output_struct *p) {
	p->size = 0;
	p->data = 0;
	p->timestamp = 0;
}

static void copy_mkv_read_output_struct(struct mkv_read_output_struct *dest, struct mkv_read_output_struct *src) {
	dest->size = src->size;
	dest->data = src->data;
	dest->timestamp = src->timestamp;
}

static int in_mkv_read_queue(struct mkv_read_output_struct* queue, unsigned int* queue_size, unsigned int* queue_rear, unsigned int queue_max, struct mkv_read_output_struct* item) {
	if ( *queue_size+1 > queue_max )
		return 0;
	copy_mkv_read_output_struct(&queue[*queue_rear], item);
	*queue_rear = (*queue_rear+1)%queue_max;
	*queue_size += 1;
	return 1;
}

static int out_mkv_read_queue(struct mkv_read_output_struct* queue, unsigned int* queue_size, unsigned int* queue_front, unsigned int queue_max, struct mkv_read_output_struct* item) {
	if ( *queue_size == 0 )
		return 0;
	copy_mkv_read_output_struct(item, &queue[*queue_front]);
	zero_set_mkv_read_output_struct(&queue[*queue_front]);
	*queue_front = (*queue_front+1)%queue_max;
	*queue_size -= 1;
	return 1;
}

static void clear_mkv_read_queue(struct mkv_read_output_struct* queue, unsigned int* queue_size, unsigned int* queue_front, unsigned int* queue_rear, unsigned int queue_max) {
	unsigned int i;
	for(i=0; i<queue_max; i++) {
		if ( queue[i].data )
			free_64(queue[i].data);
		zero_set_mkv_read_output_struct(&queue[i]);
	}
	*queue_size = 0;
	*queue_front = 0;
	*queue_rear = 0;
}

//static uint64_t mkv_read_be64(buffered_reader_t* reader) {
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
//static uint32_t mkv_read_be32(buffered_reader_t* reader) {
//	uint8_t data[4];
//	uint32_t result = 0;
//	uint32_t a, b, c, d;
//	
//	buffered_reader_read(reader, data, 4);
//	a = (uint8_t)data[0];
//	b = (uint8_t)data[1];
//	c = (uint8_t)data[2];
//	d = (uint8_t)data[3];
//	result = (a<<24) | (b<<16) | (c<<8) | d;
//
//	return result;
//}
//
//static uint16_t mkv_read_be16(buffered_reader_t* reader) {
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

uint8_t mkv_read_8(buffered_reader_t* reader) {
	uint8_t result;
    
	buffered_reader_read(reader, &result, 1);
    
	return result;
}

uint32_t mkv_ebml_read_id (buffered_reader_t* reader, int32_t *length) {
	int32_t i, len_mask = 0x80;
	uint32_t id;

	for (i=0, id=mkv_read_8(reader); i<4 && !(id & len_mask); i++)
		len_mask >>= 1;
	if (i >= 4)
		return EBML_ID_INVALID;
	if (length)
		*length = i + 1;
	while (i--)
		id = (id << 8) | mkv_read_8(reader);
	return id;
}

uint64_t mkv_ebml_read_length(buffered_reader_t* reader, int32_t* length) {
	int32_t i, j, num_ffs = 0, len_mask = 0x80;
	uint64_t len;

	for (i=0, len=mkv_read_8(reader); i<8 && !(len & len_mask); i++)
		len_mask >>= 1;
	if (i >= 8)
		return EBML_UINT_INVALID;
	j = i+1;
	if (length)
		*length = j;
	if ((int)(len &= (len_mask - 1)) == len_mask - 1)
		num_ffs++;
	while (i--) {
		len = (len << 8) | mkv_read_8(reader);
		if ((len & 0xFF) == 0xFF)
			num_ffs++;
	}
	if (j == num_ffs)
		return EBML_UINT_INVALID;
	return len;
}

uint64_t mkv_ebml_read_uint(buffered_reader_t* reader, uint64_t* length) {
	uint64_t len, value = 0;
	int32_t l;

	len = mkv_ebml_read_length (reader, &l);
	if (len == EBML_UINT_INVALID || len < 1 || len > 8)
		return EBML_UINT_INVALID;
	if (length)
		*length = len + l;

	while (len--)
		value = (value << 8) | mkv_read_8(reader);

	return value;
}

int64_t mkv_ebml_read_int(buffered_reader_t* reader, uint64_t* length) {
	int64_t value = 0;
	uint64_t len;
	int32_t l;

	len = mkv_ebml_read_length (reader, &l);
	if (len == EBML_UINT_INVALID || len < 1 || len > 8)
		return EBML_INT_INVALID;
	if (length)
		*length = len + l;

	len--;
	l = mkv_read_8(reader);
	if (l & 0x80)
		value = -1;
	value = (value << 8) | l;
	while (len--)
		value = (value << 8) | mkv_read_8(reader);

	return value;
}

int32_t mkv_ebml_read_skip(buffered_reader_t* reader, uint64_t* length) {
	uint64_t len;
	int32_t l;

	len = mkv_ebml_read_length (reader, &l);
	if (len == EBML_UINT_INVALID)
		return 1;
	if (length)
		*length = len + l;

	buffered_reader_seek(reader, buffered_reader_position(reader)+len);

	return 0;
}

uint64_t mkv_ebml_read_vlen_uint(uint8_t* buffer, int32_t* length) {
	int32_t i, j, num_ffs = 0, len_mask = 0x80;
	uint64_t num;

	for (i=0, num=*buffer++; i<8 && !(num & len_mask); i++)
		len_mask >>= 1;
	if (i >= 8)
		return EBML_UINT_INVALID;
	j = i+1;
	if (length)
		*length = j;
	if ((int)(num &= (len_mask - 1)) == len_mask - 1)
		num_ffs++;
	while (i--) {
		num = (num << 8) | *buffer++;
		if ((num & 0xFF) == 0xFF)
			num_ffs++;
	}
	if (j == num_ffs)
		return EBML_UINT_INVALID;
	return num;
}

int64_t mkv_ebml_read_vlen_int(uint8_t* buffer, int32_t *length) {
	uint64_t unum;
	int32_t l;

	unum = mkv_ebml_read_vlen_uint (buffer, &l);
	if (unum == EBML_UINT_INVALID)
		return EBML_INT_INVALID;
	if (length)
		*length = l;

	return unum - ((1 << ((7 * l) - 1)) - 1);
}

void mkv_read_safe_constructor(struct mkv_read_struct *p) {
	mkv_file_safe_constructor(&p->file);

	p->reader = 0;
	
	p->current_audio_track = 0;
	
	p->block_buffer = 0;
	
	int i;
	for(i=0; i<MKV_VIDEO_QUEUE_MAX; i++)
		zero_set_mkv_read_output_struct(&(p->video_queue[i]));
		
	p->video_queue_front = 0;
	p->video_queue_rear = 0;
	p->video_queue_size = 0;
		
	for(i=0; i<MKV_AUDIO_QUEUE_MAX; i++)
		zero_set_mkv_read_output_struct(&(p->audio_queue[i]));
	
	p->audio_queue_front = 0;
	p->audio_queue_rear = 0;
	p->audio_queue_size = 0;
	
	p->cluster_size = 0;
	p->cluster_timecode = 0;
	p->blockgroup_size = 0;
	p->block_size = 0;

}


void mkv_read_close(struct mkv_read_struct *p) {
	
	mkv_file_close(&p->file);

	if (!(p->reader))
		buffered_reader_close(p->reader);
		
	if (p->block_buffer != 0)
		free_64(p->block_buffer);
	
	int i;
	for(i=0; i<MKV_VIDEO_QUEUE_MAX; i++) {
		if ( p->video_queue[i].data )
			free_64(p->video_queue[i].data);
	}
		
	for(i=0; i<MKV_AUDIO_QUEUE_MAX; i++) {
		if ( p->audio_queue[i].data )
			free_64(p->audio_queue[i].data);
	}
	
	
	mkv_read_safe_constructor(p);
}

char *mkv_read_open(struct mkv_read_struct *p, char *s) {
	
	mkv_read_safe_constructor(p);

	char *result = mkv_file_open(&p->file, s);
	if (result != 0) {
		mkv_read_close(p);
		return(result);
	}


	//p->reader = buffered_reader_open(s, 32768, 0);
	p->reader = buffered_reader_open(s, 96*1024, 0, 0x30);
	if (!p->reader) {
		mkv_read_close(p);
		return("mkv_read_open: can't open file");
	}
	
	buffered_reader_seek(p->reader, p->file.info->indexes[0].filepos);

	return(0);
}

int mkv_read_block_lacing(uint8_t *buffer, uint64_t *size, uint8_t *laces, int32_t *all_lace_sizes) {
	uint32_t total = 0;
	uint8_t flags;
	int i;

	/* lacing flags */
	flags = *buffer++;
	(*size)--;

	switch ((flags & 0x06) >> 1) {
		case 0:  /* no lacing */
			*laces = 1;
			all_lace_sizes[0] = *size;
			break;

		case 1:  /* xiph lacing */
		case 2:  /* fixed-size lacing */
		case 3:  /* EBML lacing */
			*laces = *buffer++;
			(*size)--;
			(*laces)++;
			switch ((flags & 0x06) >> 1) {
				case 1:  /* xiph lacing */
					for (i=0; i < *laces-1; i++) {
						all_lace_sizes[i] = 0; 
						do {
							all_lace_sizes[i] += *buffer;
							(*size)--;
						} while (*buffer++ == 0xFF);
						total += all_lace_sizes[i];
					}
					all_lace_sizes[i] = *size - total;
					break;

				case 2:  /* fixed-size lacing */
					for (i=0; i < *laces; i++)
						all_lace_sizes[i] = *size / *laces;
					break;

				case 3:  /* EBML lacing */ {
					int32_t l;
					uint64_t num = mkv_ebml_read_vlen_uint(buffer, &l);
					if (num == EBML_UINT_INVALID) {
						return 0;
					}
					buffer += l;
					*size -= l;

					total = all_lace_sizes[0] = num;
					for (i=1; i < *laces-1; i++) {
						int64_t snum;
						snum = mkv_ebml_read_vlen_int(buffer, &l);
						if (snum == EBML_INT_INVALID) {
							return 0;
						}
						buffer += l;
						*size -= l;
						all_lace_sizes[i] = all_lace_sizes[i-1] + snum;
						total += all_lace_sizes[i];
					}
					all_lace_sizes[i] = *size - total;
					break;
				}
			}
			break;
	}
	return 1;
}

int mkv_is_subtitle_track(struct mkv_read_struct *p, int32_t tracknum, uint32_t* type) {
	int i;
	for(i=0; i<p->file.subtitle_tracks; i++) {
		if ( p->file.info->tracks[p->file.subtitle_track_ids[i]]->tracknum == tracknum ) {
			*type = p->file.info->tracks[p->file.subtitle_track_ids[i]]->video_type;
			return 1;
		}
	}
	return 0;
}

void mkv_handle_text_subtitle_block(struct mkv_read_struct *p, uint8_t *block_buffer, uint64_t block_size, 
	uint64_t timecode, uint64_t duration, int32_t tracknum, uint32_t type) {
	char trackname[32];
	memset(trackname, 0, 32);
	sprintf(trackname, "mkv subtitle track(%d)", tracknum);
	struct subtitle_parse_struct *cur_parser = 0;
	int i;
	for(i=0; i < MAX_SUBTITLES; i++) {
		if (strcmp(trackname, subtitle_parser[i].filename) == 0 ) {
			cur_parser = &subtitle_parser[i];
			break;
		}
	}
	if ( cur_parser ) {
		struct subtitle_frame_struct* frame = (struct subtitle_frame_struct*)malloc_64( sizeof(struct subtitle_frame_struct) );
		if (frame==0) {
			return;
		}
		subtitle_frame_safe_constructor(frame);
		double tmp ;
		tmp = 1.0f*timecode;
		tmp *= p->file.video_rate;
		tmp /= p->file.video_scale;
		tmp /= 1000.0f;
		frame->p_start_frame = (unsigned int)tmp;
		tmp = 1.0f*(timecode+duration);
		tmp *= p->file.video_rate;
		tmp /= p->file.video_scale;
		tmp /= 1000.0f;
		frame->p_end_frame = (unsigned int)tmp;
		frame->p_num_lines = 1;
		
		memset(frame->p_string, 0, max_subtitle_string);
		strncpy(frame->p_string, block_buffer, ((max_subtitle_string - 1 > block_size) ? block_size : (max_subtitle_string - 1)) );
		
		int j = 0;
		while(j < max_subtitle_string ) {
			if ( frame->p_string[j] == 0 )
				break;
			else if ( frame->p_string[j] == '\n' || frame->p_string[j] == '\r' ) {
				frame->p_num_lines++;
				do {
					j++;
				}while( frame->p_string[j] == '\n' || frame->p_string[j] == '\r' );				
			}
			else
				j++; 
		}
		if ( type == 0x7478746C ) {
			if ( miniConvHaveDefaultSubtitleConv() ){
				char* temp_str = miniConvDefaultSubtitleConv(frame->p_string);
				if( temp_str != NULL ) {
					strncpy(frame->p_string, temp_str, max_subtitle_string-1);
				}
			}
		}
		
		subtitle_parse_add_frame( cur_parser, cur_parser->p_cur_sub_frame, frame);
	}
}

int mkv_handle_block(struct mkv_read_struct *p, uint8_t *block_buffer, uint64_t block_size, uint64_t block_duration, 
	int64_t block_bref, int64_t block_fref, uint8_t simpleblock, int track_id, int skip_to_keyframe) {
	int32_t tracknum;
	int32_t current_tracknum, video_tracknum, audio_tracknum;
	int32_t tmp = 0;
	int16_t time;
	uint8_t flags;
	uint64_t old_block_size;
	int32_t lace_size[16];
	uint8_t laces;
	int64_t timecode;
	uint64_t duration;
	int res;
	int use_this_block = 1;
	
	current_tracknum = p->file.info->tracks[track_id]->tracknum;
	video_tracknum = p->file.info->tracks[p->file.video_track_id]->tracknum;
	audio_tracknum = p->file.info->tracks[p->file.audio_track_ids[p->current_audio_track]]->tracknum;
	
	tracknum = mkv_ebml_read_vlen_uint(block_buffer, &tmp);
	
	block_buffer += tmp;
	time = block_buffer[0] << 8 | block_buffer[1];
	block_buffer += 2;
	block_size -= tmp + 2;
	old_block_size = block_size;
	flags = block_buffer[0];
	memset(lace_size, 0, 16*sizeof(int32_t));
	res = mkv_read_block_lacing(block_buffer, &block_size, &laces, lace_size);
	if (res != 1)
		return res;
	
	block_buffer += old_block_size - block_size;
	
	timecode = ((p->cluster_timecode + time - p->file.info->first_timecode) * p->file.info->timecode_scale /1000000.0);
	if (timecode < 0)
		timecode = 0;
	duration = block_duration * p->file.info->timecode_scale /1000000.0;
		
	res = 1;
	uint32_t subtitle_type;	
	if ( mkv_is_subtitle_track(p, tracknum, &subtitle_type) ) {
		mkv_handle_text_subtitle_block(p, block_buffer, block_size, timecode, duration, tracknum, subtitle_type);
		use_this_block = 0;
		res = 0;
	}
	else if ( tracknum == current_tracknum ) {
		if ( skip_to_keyframe ) {
			if ( simpleblock ) {
				if (!(flags&0x80)) {
					use_this_block = 0;
					res = 0;
				}
			}
			else if (block_bref != 0 || block_fref != 0) {
            			use_this_block = 0;
            			res = 0;
            		}
		}
	}
	else {
		use_this_block = 0;
		res = 0;
		if ( !skip_to_keyframe && (tracknum == video_tracknum || tracknum == audio_tracknum) )
			use_this_block = 1;
	}
	
	if ( use_this_block ) {
		int i;
		struct mkv_read_output_struct packet;
		for (i=0; i < laces; i++) {
			memset(&packet, 0, sizeof(struct mkv_read_output_struct));
			packet.size = lace_size[i];
			packet.data = malloc_64(packet.size);
			if (packet.data == 0) {
				return -2;
			}
			memcpy(packet.data, block_buffer, packet.size);
			packet.timestamp = (int)timecode;
			if ( tracknum == video_tracknum )
				tmp = in_mkv_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_rear, MKV_VIDEO_QUEUE_MAX, &packet);
			else
				tmp = in_mkv_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_rear, MKV_AUDIO_QUEUE_MAX, &packet);
			if ( tmp == 0 )  {
				return -1;
			}
			block_buffer+=packet.size;
			if ( tracknum == video_tracknum )
				timecode+=(1000LL*p->file.video_scale/p->file.video_rate);
			else
				timecode+=(1000LL*p->file.audio_resample_scale/p->file.audio_rate);	
		}
	}
	
	return(res);
}

char *mkv_read_fill_buffer(struct mkv_read_struct *p, int track_id, int skip_to_keyframe) {
	
	int32_t il, tmp;
	uint64_t l;
	int res;
	
	while (1) {
		while (p->cluster_size > 0) {
			uint64_t block_duration = 0;
			int64_t block_bref = 0, block_fref = 0;
			while (p->blockgroup_size > 0) {
				switch (mkv_ebml_read_id(p->reader, &il)) {
					case MATROSKA_ID_BLOCKDURATION: {
						block_duration = mkv_ebml_read_uint(p->reader, &l);
						if (block_duration == EBML_UINT_INVALID) {
							if( p->block_buffer ) {
								free_64(p->block_buffer);
								p->block_buffer = 0;
							}
							return("mkv_read_fill_buffer: error block duration");
						}
						break;
					}

					case MATROSKA_ID_BLOCK: {
						p->block_size = mkv_ebml_read_length(p->reader, &tmp);
						if (p->block_size > SIZE_MAX - MKV_INPUT_PADDING) 
							return("mkv_read_fill_buffer: error block size");
						if( p->block_buffer ) {
							free_64(p->block_buffer);
							p->block_buffer = 0;
						}
						p->block_buffer = malloc_64(p->block_size+ MKV_INPUT_PADDING);
						if ( !p->block_buffer )
							return("mkv_read_fill_buffer: can not malloc_64 block_buffer");
						if (buffered_reader_read(p->reader, p->block_buffer, p->block_size) != (uint32_t)(p->block_size)) {
							free_64(p->block_buffer);
							p->block_buffer = 0;
							return("mkv_read_fill_buffer: can not read block");
						}
						l = tmp + p->block_size;
						break;
					}

					case MATROSKA_ID_REFERENCEBLOCK: {
						int64_t num = mkv_ebml_read_int(p->reader, &l);
						if (num == EBML_INT_INVALID) {
							if( p->block_buffer ) {
								free_64(p->block_buffer);
								p->block_buffer = 0;
							}
							return("mkv_read_fill_buffer: can not find block ref");
						}
						if (num <= 0)
							block_bref = num;
						else
							block_fref = num;
						break;
					}

					case EBML_ID_INVALID:
						if( p->block_buffer ) {
							free_64(p->block_buffer);
							p->block_buffer = 0;
						}
						return("mkv_read_fill_buffer: read id invalid");

					default:
						mkv_ebml_read_skip(p->reader, &l);
						break;
				}
				p->blockgroup_size -= l + il;
				p->cluster_size -= l + il;
			}
			if (p->block_buffer) {
				res = mkv_handle_block(p, (uint8_t *)(p->block_buffer), p->block_size, block_duration, block_bref, block_fref, 0, track_id, skip_to_keyframe);
				free_64(p->block_buffer);
				p->block_buffer = 0;
				if (res < 0) {
					if ( res == -1 )
						return("mkv_read_fill_buffer: queue is full");
					else
						return("mkv_read_fill_buffer: can not malloc_64 data buffer");
				}
				if (res)
					return(0);
			}

			if (p->cluster_size > 0) {
				switch (mkv_ebml_read_id(p->reader, &il)) {
					case MATROSKA_ID_CLUSTERTIMECODE: {
						uint64_t num = mkv_ebml_read_uint(p->reader, &l);
						if (num == EBML_UINT_INVALID)
							return("mkv_read_fill_buffer: invalid cluster timecode");						
						p->cluster_timecode = num;
						break;
					}

					case MATROSKA_ID_BLOCKGROUP: {
						p->blockgroup_size = mkv_ebml_read_length(p->reader, &tmp);
						l = tmp;
						break;
					}

					case MATROSKA_ID_SIMPLEBLOCK: {
						int res;
						p->block_size = mkv_ebml_read_length(p->reader, &tmp);
						if (p->block_size > SIZE_MAX - MKV_INPUT_PADDING) 
							return("mkv_read_fill_buffer: error block size");
						if( p->block_buffer ) {
							free_64(p->block_buffer);
							p->block_buffer = 0;
						}
						p->block_buffer = malloc_64(p->block_size+ MKV_INPUT_PADDING);
						if ( !p->block_buffer )
							return("mkv_read_fill_buffer: can not malloc_64 block_buffer");
						if (buffered_reader_read(p->reader, p->block_buffer, p->block_size) != (uint32_t)(p->block_size)) {
							free_64(p->block_buffer);
							p->block_buffer = 0;
							return("mkv_read_fill_buffer: can not read block");
						}
						l = tmp + p->block_size;
						res = mkv_handle_block (p, (uint8_t *)(p->block_buffer), p->block_size, block_duration, block_bref, block_fref, 1, track_id, skip_to_keyframe);
						free_64(p->block_buffer);
						p->block_buffer = 0;
						p->cluster_size -= l + il;
						if (res < 0) {
							if ( res == -1 )
								return("mkv_read_fill_buffer: queue is full");
							else
								return("mkv_read_fill_buffer: can not malloc_64 data buffer");
						}
						else if (res)
							return(0);
						else 
							p->cluster_size += l + il;
						break;
					}
					
					case EBML_ID_INVALID:
						return("mkv_read_fill_buffer: can not find block");;

					default:
						mkv_ebml_read_skip(p->reader, &l);
						break;
				}
				p->cluster_size -= l + il;
			}
		}
		if (mkv_ebml_read_id(p->reader, &il) != MATROSKA_ID_CLUSTER)
			return("mkv_read_fill_buffer: eof");
		p->cluster_size = mkv_ebml_read_length(p->reader, 0);
	}
	return(0);
}

char *mkv_read_seek(struct mkv_read_struct *p, int timestamp) {
	int32_t video_tracknum = p->file.info->tracks[p->file.video_track_id]->tracknum;
	int32_t i;
	mkvinfo_index_t* index = 0;
	if (timestamp < 0)
		timestamp = 0;
	for(i=0; i < p->file.info->total_indexes-1; i++) {
		if ( p->file.info->indexes[i].tracknum == video_tracknum ) {
			int64_t timecode = ((p->file.info->indexes[i].timecode - p->file.info->first_timecode) * p->file.info->timecode_scale /1000000.0);
			int64_t next_timecode = ((p->file.info->indexes[i+1].timecode - p->file.info->first_timecode) * p->file.info->timecode_scale /1000000.0);
			if (timestamp >= (int)timecode && timestamp < (int)next_timecode) {
				index = p->file.info->indexes+i;
				break;
			}
		}
	}
	if ( index == 0 ) {
		index = p->file.info->indexes + (p->file.info->total_indexes-1);
	}
	buffered_reader_seek(p->reader, index->filepos);
	p->cluster_size = p->blockgroup_size = 0;
	
	clear_mkv_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, &p->audio_queue_rear, MKV_VIDEO_QUEUE_MAX);
	clear_mkv_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, &p->video_queue_rear, MKV_AUDIO_QUEUE_MAX);
	
	return(mkv_read_fill_buffer(p, p->file.video_track_id, 1));
}

char *mkv_read_get_video(struct mkv_read_struct *p, struct mkv_read_output_struct *output) {
	if ( p->video_queue_size > 0 )
		out_mkv_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, MKV_VIDEO_QUEUE_MAX, output);
	else {
		char* res = mkv_read_fill_buffer(p, p->file.video_track_id, 0);
		if (res)
			return res;
		if ( !(p->video_queue_size>0) )
			return "mkv_read_get_video: video queue is empty";
		out_mkv_read_queue(p->video_queue, &p->video_queue_size, &p->video_queue_front, MKV_VIDEO_QUEUE_MAX, output);
	}
	return(0);
}

char *mkv_read_get_audio(struct mkv_read_struct *p, unsigned int audio_stream, struct mkv_read_output_struct *output){
	p->current_audio_track = audio_stream;
	if ( p->audio_queue_size > 0 )
		out_mkv_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, MKV_AUDIO_QUEUE_MAX, output);
	else {
		char* res = mkv_read_fill_buffer(p, p->file.audio_track_ids[audio_stream], 0);
		if (res)
			return res;
		if ( !(p->audio_queue_size>0) )
			return "mkv_read_get_audio: audio queue is empty";
		out_mkv_read_queue(p->audio_queue, &p->audio_queue_size, &p->audio_queue_front, MKV_AUDIO_QUEUE_MAX, output);
	}
	return(0);
}
