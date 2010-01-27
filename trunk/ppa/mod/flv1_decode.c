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
#include "flv1_decode.h"
#include "me_boot_start.h"
#include "audio_util.h"
#include "psp1k_frame_buffer.h"

static int in_flv1_timestamp_queue(int* queue, unsigned int* queue_size, unsigned int queue_max, int timestamp) {
	if ( *queue_size+1 > queue_max )
		return 0;
	int i,j;
	for(i=0; i<*queue_size; i++) {
		if ( timestamp < queue[i] ) {
			for(j=*queue_size-1; j>=i; j--) {
				queue[j+1] = queue[j];
			}		
			break;
		}
	}
	queue[i] = timestamp;
	*queue_size += 1;
	return 1;
}

static int out_flv1_timestamp_queue(int* queue, unsigned int* queue_size, unsigned int queue_max, int* timestamp) {
	if ( *queue_size == 0 )
		return 0;
	*timestamp  = queue[0];
	int i;
	for(i=1; i<*queue_size; i++)
		queue[i-1] = queue[i];
	queue[i-1] = -1;
	*queue_size -= 1;
	return 1;
}

static void clear_flv1_timestamp_queue(int* queue, unsigned int* queue_size, unsigned int queue_max) {
	int i;
	for(i=0; i<queue_max; i++) {
		queue[i] = -1;
	}
	*queue_size = 0;
}

void flv1_decode_safe_constructor(struct flv1_decode_struct *p) {
	flv1_read_safe_constructor(&p->reader);

	mp4_avc_safe_constructor(&p->avc);
#ifdef USE_FFMPEG_FLV1_DECODER
	flv1_safe_constructor(&p->flv1);
#endif

	int i = 0;
	for (; i < flv1_maximum_frame_buffers; i++) {
		p->video_frame_buffers[i] = 0;
		p->audio_frame_buffers[i] = 0;
	}
	
	clear_flv1_timestamp_queue(p->timestamp_queue, &p->timestamp_queue_size, flv1_timestamp_queue_max);
	p->last_audio_timestamp = 0;
	p->last_video_timestamp = 0;
	p->is_eof = 0;

}


void flv1_decode_close(struct flv1_decode_struct *p, int pspType) {
	flv1_read_close(&p->reader);
	
	if (p->video_format == 0x61766331 /*avc1*/)
		mp4_avc_close(&p->avc);
#ifdef USE_FFMPEG_FLV1_DECODER
	else
		flv1_close(&p->flv1);
#endif

	if (p->audio_decoder == 0 )
		audio_decoder_close();
		
	clear_reset_framebuffer();

	int i = 0;
	for (; i < flv1_maximum_frame_buffers; i++) {
		if (p->audio_frame_buffers[i] != 0) 
			free_64(p->audio_frame_buffers[i]);
	}

	flv1_decode_safe_constructor(p);
}


char *flv1_decode_open(struct flv1_decode_struct *p, char *s, int pspType, int tvAspectRatio, int tvWidth, int tvHeight, int videoMode) {
	
	flv1_decode_safe_constructor(p);
	
	cpu_clock_set_maximum();

	char *result = flv1_read_open(&p->reader, s);
	if (result != 0) {
		flv1_decode_close(p, pspType);
		return(result);
	}

	p->audio_frame_size = (p->reader.file.audio_resample_scale << 1) << p->reader.file.audio_stereo;


	p->video_format = p->reader.file.video_type;
	
	if ( p->video_format == 0x61766331 /*avc1*/ ) {
		if ( p->reader.file.video_width > 480 || p->reader.file.video_height > 272 ) {
			me_boot_start(1);
		}
		else if ( p->reader.file.info->avc_profile == 0x42 ) {
			me_boot_start(4);
		}
		else {
			me_boot_start(3);
		}
	}
	else {
		me_boot_start(3);
	}

	if ( p->video_format == 0x61766331 /*avc1*/ ) {
		
		result = mp4_avc_open(&p->avc, 
			p->reader.file.info->avc_profile,
			((p->reader.file.video_width > 480 || p->reader.file.video_height > 272) ? 5 : 4), 
			p->reader.file.info->avc_sps, 
			p->reader.file.info->avc_sps_size,
			p->reader.file.info->avc_pps, 
			p->reader.file.info->avc_pps_size,
			p->reader.file.info->avc_nal_prefix_size);
//		result = "mp4_avc_open: no supported";
	}
	else {
#ifdef USE_FFMPEG_FLV1_DECODER
		result = flv1_open(&p->flv1,
			p->reader.file.video_width,
			p->reader.file.video_height);
#else
		result = "flv1_open: no supported:";
#endif
	}
	if (result != 0) {
		flv1_decode_close(p, pspType);
		return(result);
	}
	unsigned int display_width = p->reader.file.video_width;
	unsigned int display_height = p->reader.file.video_height;
	if (display_width == 720 && display_height == 480 ) {
		display_width = 853; 
	}
	aspect_ratio_struct_init(p->reader.file.video_width, p->reader.file.video_height, display_width, display_height, pspType, tvAspectRatio, tvWidth, tvHeight, videoMode);
	
	//*/
	if ( p->reader.file.audio_type == 0x6D703461 /*mp4a*/ )
		p->audio_decoder = audio_decoder_open(PSP_CODEC_AUDIO_AAC, p->reader.file.audio_actual_rate, p->reader.file.audio_scale, 0);
	else
		p->audio_decoder = audio_decoder_open(PSP_CODEC_AUDIO_MP3, p->reader.file.audio_actual_rate, p->reader.file.audio_scale, 0);
	if (p->audio_decoder == -1) {
		flv1_decode_close(p, pspType);
		return("flv1_decode_open: can not find audio decoder");
	}
	//*/
	
	cpu_clock_set_minimum();

	int i = 0;
	if (m33IsTVOutSupported(pspType)) {
		p->output_texture_width = 768;
		p->number_of_frame_buffers = 8;
		p->video_frame_size = 1572864;
		
		i = 0;
		for (; i < p->number_of_frame_buffers; i++) {
			p->video_frame_buffers[i] = (void*)(0x0a000000 + (i+1)*p->video_frame_size);
			//p->video_frame_buffers[i] = (void*)(0x0a000000 + i*p->video_frame_size);
			memset(p->video_frame_buffers[i], 0, p->video_frame_size);
		}
	}
	else {
		p->output_texture_width = 512;
		p->number_of_frame_buffers = 4;
		p->video_frame_size = 557056;
		
		i = 0;
		for (; i < p->number_of_frame_buffers; i++) {
			p->video_frame_buffers[i] = psp1k_get_frame_buffer(i);
			memset(p->video_frame_buffers[i], 0, p->video_frame_size);
		}

	}
	i = 0;
	for (; i <= p->number_of_frame_buffers; i++) {
		// p->audio_frame_buffers[p->number_of_frame_buffers] is a null buffer
	
		p->audio_frame_buffers[i] = malloc_64(p->audio_frame_size);

		if (p->audio_frame_buffers[i] == 0) {
			flv1_decode_close(p, pspType);
			return("flv1_decode_open: malloc_64 failed on audio_frame_buffers");
		}

		memset(p->audio_frame_buffers[i], 0, p->audio_frame_size);
	}

	sceDisplayWaitVblankStart();
	sceDisplaySetFrameBuf(p->video_frame_buffers[0], p->output_texture_width, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_IMMEDIATE);


	p->current_video_buffer_number = 0;
	p->current_audio_buffer_number = 0;
	
	uint64_t duration = 1000LL;
	duration *= p->reader.file.video_scale;
	duration /= p->reader.file.video_rate;
	p->video_frame_duration = duration;
	
	duration = 1000LL;
	duration *= p->reader.file.audio_resample_scale;
	duration /= p->reader.file.audio_rate;
	p->audio_frame_duration = duration;
	
	pmp_gu_init_previous_values();
	
	return(0);
}

int   flv1_decode_is_eof(struct flv1_decode_struct *p) {
	return (p->is_eof);
}

void  flv1_decode_reset(struct flv1_decode_struct *p) {
	flv1_read_seek(&p->reader, 0, 0);
	p->is_eof = 0;
}

char *flv1_decode_seek(struct flv1_decode_struct *p, int timestamp, int last_timestamp) {
	return flv1_read_seek(&p->reader, timestamp, last_timestamp);
}

char *flv1_decode_keyframe_forward(struct flv1_decode_struct *p, int keyframes) {
	return flv1_read_keyframe_forward(&p->reader, keyframes);
}

char *flv1_decode_keyframe_backward(struct flv1_decode_struct *p, int keyframes) {
	return flv1_read_keyframe_backward(&p->reader, keyframes);
}

char *flv1_decode_get_audio(struct flv1_decode_struct *p, unsigned int audio_stream, int audio_channel, int decode_audio, unsigned int volume_boost) {
	char *result;
	struct flv1_read_output_struct a_packet;
	memset(&a_packet, 0, sizeof(struct flv1_read_output_struct));
	
	result = flv1_read_get_audio(&p->reader, audio_stream, &a_packet);
	if (result != 0) {
		a_packet.timestamp = p->last_audio_timestamp + p->audio_frame_duration;
	}
	
	if (a_packet.data == 0 || decode_audio == 0) {
		p->output_audio_frame_buffers[p->current_audio_buffer_number].data = p->audio_frame_buffers[p->number_of_frame_buffers];
	}
	else {
		p->output_audio_frame_buffers[p->current_audio_buffer_number].data = p->audio_frame_buffers[p->current_audio_buffer_number];
		
		int audio_output_length;
			
		if (p->reader.file.audio_up_sample) {
			audio_decoder_decode(p->audio_frame_buffers[p->number_of_frame_buffers], &audio_output_length, a_packet.data, a_packet.size);
			pcm_up_sample(p->audio_frame_buffers[p->current_audio_buffer_number], 
				p->audio_frame_buffers[p->number_of_frame_buffers],
				p->reader.file.audio_up_sample,
				p->reader.file.audio_resample_scale / (p->reader.file.audio_up_sample+1) );
			memset(p->audio_frame_buffers[p->number_of_frame_buffers], 0, p->audio_frame_size);
		}
		else
			audio_decoder_decode(p->audio_frame_buffers[p->current_audio_buffer_number], &audio_output_length, a_packet.data, a_packet.size);
		
		
		pcm_normalize(p->audio_frame_buffers[p->current_audio_buffer_number], p->reader.file.audio_resample_scale << p->reader.file.audio_stereo);
		pcm_select_channel(p->audio_frame_buffers[p->current_audio_buffer_number], p->reader.file.audio_resample_scale << p->reader.file.audio_stereo, audio_channel);	
		
		free_64( a_packet.data );
	}
	
	p->output_audio_frame_buffers[p->current_audio_buffer_number].timestamp = a_packet.timestamp;
	p->last_audio_timestamp = a_packet.timestamp;
	p->current_audio_buffer_number = (p->current_audio_buffer_number + 1) % p->number_of_frame_buffers;
	
	return(0);
}

char *flv1_decode_get_cached_video(struct flv1_decode_struct *p, unsigned int pic_num, unsigned int audio_stream, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop) {
	char *result;
	sceKernelDcacheWritebackInvalidateAll();
	result = mp4_avc_get_cache(&p->avc, pmp_gu_rgb_buffer, pic_num);
	if (result != 0) {
		return(result);
	}
	int timestamp;
	out_flv1_timestamp_queue(p->timestamp_queue, &p->timestamp_queue_size, flv1_timestamp_queue_max, &timestamp);
	double tmp ;
	tmp = 1.0f*timestamp;
	tmp *= p->reader.file.video_rate;
	tmp /= p->reader.file.video_scale;
	tmp /= 1000.0f;
	if (show_interface == 1){
		draw_interface(
			p->reader.file.video_scale,
			p->reader.file.video_rate,
			p->reader.file.number_of_video_frames,
			(unsigned int)tmp,
			aspect_ratio,
			zoom,
			luminosity_boost,
			audio_stream,
			volume_boost,
			loop);
	}


	sceKernelDcacheWritebackInvalidateAll();
	
	pmp_gu_draw(aspect_ratio, zoom, luminosity_boost, show_interface, show_subtitle, subtitle_format, (unsigned int)tmp, p->video_frame_buffers[p->current_video_buffer_number]);
	
	p->output_video_frame_buffers[p->current_video_buffer_number].data            = p->video_frame_buffers[p->current_video_buffer_number];

	pmp_gu_wait();
	
	p->output_video_frame_buffers[p->current_video_buffer_number].timestamp = timestamp;
	p->last_video_timestamp = timestamp;
	p->current_video_buffer_number = (p->current_video_buffer_number + 1) % p->number_of_frame_buffers;
	sceKernelDcacheWritebackInvalidateAll();


	return(0);
}

char *flv1_decode_get_video(struct flv1_decode_struct *p, unsigned int audio_stream, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop, int* pic_num) {
	
	char *result;
	struct flv1_read_output_struct v_packet;
	memset(&v_packet, 0, sizeof(struct flv1_read_output_struct));
	
	if (p->is_eof) {
		*pic_num = 1;
		in_flv1_timestamp_queue(p->timestamp_queue, &p->timestamp_queue_size, flv1_timestamp_queue_max, p->last_video_timestamp + p->video_frame_duration);
	}
	else {
		result = flv1_read_get_video(&p->reader, &v_packet);
		if (result != 0) {
			if ( strcmp(result, "flv1_read_fill_buffer: eof") == 0 ) {
				*pic_num = 1;
				p->is_eof = 1;
				in_flv1_timestamp_queue(p->timestamp_queue, &p->timestamp_queue_size, flv1_timestamp_queue_max, p->last_video_timestamp + p->video_frame_duration);
			}
			else
				return result;
		}
		else {
			sceKernelDcacheWritebackInvalidateAll();
			
			in_flv1_timestamp_queue(p->timestamp_queue, &p->timestamp_queue_size, flv1_timestamp_queue_max, v_packet.timestamp);
		
			if (p->video_format == 0x61766331 /*avc1*/)
				result = mp4_avc_get(&p->avc, (v_packet.timestamp==0?3:0), v_packet.data, v_packet.size, pmp_gu_rgb_buffer, pic_num);
			else {
#ifdef USE_FFMPEG_FLV1_DECODER
				result = flv1_get_rgb(&p->flv1, v_packet.data, v_packet.size, pmp_gu_rgb_buffer);
#else
				result = "flv1_get_rgb: decode error";
#endif
				*pic_num = 1;
			}
			if( v_packet.data )
				free_64(v_packet.data);
			if (result != 0)
				return(result);
		}
	}
	*pic_num = *pic_num - 1;
	
	if ( *pic_num >= 0 ) {
		
		int timestamp;
		out_flv1_timestamp_queue(p->timestamp_queue, &p->timestamp_queue_size, flv1_timestamp_queue_max, &timestamp);
		
		double tmp ;
		tmp = 1.0f*timestamp;
		tmp *= p->reader.file.video_rate;
		tmp /= p->reader.file.video_scale;
		tmp /= 1000.0f;
	
		if (show_interface == 1){
			draw_interface(
				p->reader.file.video_scale,
				p->reader.file.video_rate,
				p->reader.file.number_of_video_frames,
				(unsigned int)tmp,
				aspect_ratio,
				zoom,
				luminosity_boost,
				audio_stream,
				volume_boost,
				loop);
		}
	
	
		sceKernelDcacheWritebackInvalidateAll();
		
		pmp_gu_draw(aspect_ratio, zoom, luminosity_boost, show_interface, show_subtitle, subtitle_format, (unsigned int)tmp, p->video_frame_buffers[p->current_video_buffer_number]);
		
		p->output_video_frame_buffers[p->current_video_buffer_number].data            = p->video_frame_buffers[p->current_video_buffer_number];
	
		pmp_gu_wait();
		
		p->output_video_frame_buffers[p->current_video_buffer_number].timestamp = timestamp;
		p->last_video_timestamp = timestamp;
	
		p->current_video_buffer_number = (p->current_video_buffer_number + 1) % p->number_of_frame_buffers;
		sceKernelDcacheWritebackInvalidateAll();

	}

	return(0);
}
