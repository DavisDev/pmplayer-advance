#include "mp4_decode.h"


void mp4_decode_safe_constructor(struct mp4_decode_struct *p) {
	mp4_read_safe_constructor(&p->reader);

	mp4_avc_safe_constructor(&p->avc);
	mp4v_safe_constructor(&p->mp4v);

	int i = 0;
	for (; i < maximum_frame_buffers; i++) {
		p->video_frame_buffers[i] = 0;
		p->audio_frame_buffers[i] = 0;
	}
	p->next_video_frame = 0;
	p->next_video_read_frame = 0;
	p->number_of_decoded_frame = 0;
}


void mp4_decode_close(struct mp4_decode_struct *p, int pspType) {
	mp4_read_close(&p->reader);
	
	if (p->video_format == 0x61766331 /*avc1*/)
		mp4_avc_close(&p->avc);
	else
		mp4v_close(&p->mp4v);

	if (p->audio_decoder == 0 )
		audio_decoder_close();

	int i = 0;
	if (m33IsTVOutSupported(pspType)) {
		for (; i < maximum_frame_buffers; i++) {
			if (p->audio_frame_buffers[i] != 0) 
				free_64(p->audio_frame_buffers[i]);
		}
	}
	else {
		for (; i < maximum_frame_buffers; i++) {
			if (p->video_frame_buffers[i] != 0) 
				free_64(p->video_frame_buffers[i]);
			if (p->audio_frame_buffers[i] != 0) 
				free_64(p->audio_frame_buffers[i]);
		}
	}

	mp4_decode_safe_constructor(p);
}


char *mp4_decode_open(struct mp4_decode_struct *p, char *s, int pspType, int tvAspectRatio, int tvWidth, int tvHeight, int videoMode) {
	
	mp4_decode_safe_constructor(p);

	char *result = mp4_read_open(&p->reader, s);
	if (result != 0) {
		mp4_decode_close(p, pspType);
		return(result);
	}

	p->audio_frame_size = (p->reader.file.audio_resample_scale << 1) << p->reader.file.audio_stereo;


	p->video_format = p->reader.file.video_type;

	if ( p->video_format == 0x61766331 /*avc1*/ )
		result = mp4_avc_open(&p->avc, 
			4, 
			p->reader.file.info->tracks[p->reader.file.video_track_id]->avc_sps, 
			p->reader.file.info->tracks[p->reader.file.video_track_id]->avc_sps_size,
			p->reader.file.info->tracks[p->reader.file.video_track_id]->avc_pps, 
			p->reader.file.info->tracks[p->reader.file.video_track_id]->avc_pps_size);
	else
		result = mp4v_open(&p->mp4v);
	if (result != 0) {
		mp4_decode_close(p, pspType);
		return(result);
	}




	aspect_ratio_struct_init(p->reader.file.video_width, p->reader.file.video_height, pspType, tvAspectRatio, tvWidth, tvHeight, videoMode);


	cpu_clock_set_maximum();
	
	//*/
	if ( p->reader.file.audio_type == 0x6D703461 /*mp4a*/ )
		p->audio_decoder = audio_decoder_open(0x1003, p->reader.file.audio_actual_rate, p->reader.file.audio_scale, 0);
	else
		p->audio_decoder = -1;
	if (p->audio_decoder == -1) {
		mp4_decode_close(p, pspType);
		return("mp4_decode_open: can not find audio decoder");
	}
	//*/
	cpu_clock_set_minimum();

	int i = 0;
	if (m33IsTVOutSupported(pspType)) {
		p->output_texture_width = 768;
		p->number_of_frame_buffers = 8;
		
		i = 0;
		for (; i < p->number_of_frame_buffers; i++) {
			p->video_frame_size = 1572864;
			p->video_frame_buffers[i] = (void*)(0x0a000000 + (i+1)*p->video_frame_size);
			memset(p->video_frame_buffers[i], 0, p->video_frame_size);
		}
	}
	else {
		p->output_texture_width = 512;
		p->number_of_frame_buffers = 0;
		p->video_frame_size = 557056;
		i = 0;
		for (; i < maximum_frame_buffers; i++) {
			p->video_frame_buffers[i] = malloc_64(p->video_frame_size);
	
			if (p->video_frame_buffers[i] == 0) {
				break;
			}
	
			memset(p->video_frame_buffers[i], 0, p->video_frame_size);
	
			p->number_of_frame_buffers ++;
		}
	
	
		if (p->number_of_frame_buffers < (number_of_free_video_frame_buffers + 4)) {
			mp4_decode_close(p, pspType);
			return("mp4_decode_open: number_of_frame_buffers < 4");
		}
	
	
		p->number_of_frame_buffers -= number_of_free_video_frame_buffers;
	
		i = 0;
		for (; i < number_of_free_video_frame_buffers; i++) {
			free_64(p->video_frame_buffers[p->number_of_frame_buffers + i]);
	
			p->video_frame_buffers[p->number_of_frame_buffers + i] = 0;
		}
	}
	i = 0;
	for (; i <= p->number_of_frame_buffers; i++) {
		// p->audio_frame_buffers[p->number_of_frame_buffers] is a null buffer
	
		p->audio_frame_buffers[i] = malloc_64(p->audio_frame_size * p->reader.file.maximun_audio_sample_number);

		if (p->audio_frame_buffers[i] == 0) {
			mp4_decode_close(p, pspType);
			return("mp4_decode_open: malloc_64 failed on audio_frame_buffers");
		}

		memset(p->audio_frame_buffers[i], 0, p->audio_frame_size * p->reader.file.maximun_audio_sample_number);
	}

	sceDisplayWaitVblankStart();
	sceDisplaySetFrameBuf(p->video_frame_buffers[0], p->output_texture_width, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_IMMEDIATE);


	p->current_buffer_number = 0;
	
	pmp_gu_init_previous_values();
	
	return(0);
}


static void boost_volume(short *audio_buffer, unsigned int number_of_samples, unsigned int volume_boost) {
	if (volume_boost != 0) {
		while (number_of_samples--) {
			int sample = *audio_buffer;
			sample <<= volume_boost;


			if (sample > 32767) {
				*audio_buffer++ = 32767;
			}
			else if (sample < -32768) {
				*audio_buffer++ = -32768;
			}
			else {
				*audio_buffer++ = sample;
			}
		}
	}
}

static void select_audio_channel(short *audio_buffer, unsigned int number_of_samples, int audio_channel) {
	if (audio_channel != 0) {
		if(audio_channel == -1) {
			while(number_of_samples) {
				*(audio_buffer+1) = *audio_buffer;
				audio_buffer+=2;
				number_of_samples-=2;
			}
		}
		else {
			while(number_of_samples) {
				*audio_buffer = *(audio_buffer+1);
				audio_buffer+=2;
				number_of_samples-=2;
			}
		}
	}
}

static void double_audio_sample(short *dest, short *src, unsigned int number_of_samples) {
	int i;
	for(i = 0; i < number_of_samples; i++) {
		dest[i*4] = src[i*2];
		dest[i*4+1] = src[i*2+1];
		dest[i*4+2] = src[i*2];
		dest[i*4+3] = src[i*2+1];
	}
}

char *mp4_decode_get(struct mp4_decode_struct *p, unsigned int frame_number, unsigned int audio_stream, int audio_channel, int decode_audio, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop) {
	
	char *result;
	struct mp4_video_read_output_struct v_packet;
	struct mp4_audio_read_output_struct a_packet;
	
	result = mp4_read_get_audio(&p->reader, frame_number, audio_stream, &a_packet);
	if (result != 0)
		return(result);
	
	if ( frame_number == p->next_video_frame ) {
		p->next_video_frame = frame_number+1;
	}
	else {
		p->next_video_frame = frame_number+1;
		p->next_video_read_frame = frame_number;
		p->number_of_decoded_frame = 0;
	}
	if ( p->number_of_decoded_frame > 0 ) {
		//get decoded frame from decoder cache;
		sceKernelDcacheWritebackInvalidateAll();
		result = mp4_avc_get_cache(&p->avc, pmp_gu_rgb_buffer, p->number_of_decoded_frame);
		if (result != 0)
			return(result);
		p->number_of_decoded_frame--;
	}
	else {
		while(1) { 			
			if ( p->next_video_read_frame >= p->reader.file.number_of_video_frames )
				break;
			result = mp4_read_get_video(&p->reader, p->next_video_read_frame, &v_packet);
			if (result != 0)
				return result;
			sceKernelDcacheWritebackInvalidateAll();
			int pic_num = 0;
			result = mp4_avc_get(&p->avc, (p->next_video_read_frame==0?3:0), v_packet.video_buffer, v_packet.video_length, pmp_gu_rgb_buffer, &pic_num);
			if (result != 0)
				return(result);
			p->next_video_read_frame++;
			if (pic_num == 0) {
				continue;
			}
			else {
				p->number_of_decoded_frame = pic_num - 1;
				break;
			}
		}
	}
	
	if (show_interface == 1){
		draw_interface(
			p->reader.file.video_scale,
			p->reader.file.video_rate,
			p->reader.file.number_of_video_frames,
			frame_number,
			aspect_ratio,
			zoom,
			luminosity_boost,
			audio_stream,
			volume_boost,
			loop);
	}


	sceKernelDcacheWritebackInvalidateAll();
	
	pmp_gu_draw(aspect_ratio, zoom, luminosity_boost, show_interface, show_subtitle, subtitle_format, frame_number, p->video_frame_buffers[p->current_buffer_number]);
	

	p->output_frame_buffers[p->current_buffer_number].number_of_audio_frames = a_packet.number_of_audio_frames;
	p->output_frame_buffers[p->current_buffer_number].first_delay            = a_packet.first_delay;
	p->output_frame_buffers[p->current_buffer_number].last_delay             = a_packet.last_delay;
	p->output_frame_buffers[p->current_buffer_number].video_frame            = p->video_frame_buffers[p->current_buffer_number];


	char *audio_result = 0;


	if (a_packet.audio_buffer == 0 || decode_audio == 0) {
		p->output_frame_buffers[p->current_buffer_number].audio_frame = p->audio_frame_buffers[p->number_of_frame_buffers];
	}
	else {
		p->output_frame_buffers[p->current_buffer_number].audio_frame = p->audio_frame_buffers[p->current_buffer_number];


		void *audio_buffer = a_packet.audio_buffer;


		int i = 0;
		for (; i < a_packet.number_of_audio_frames; i++) {
			int audio_length = a_packet.audio_length[i];
			int audio_output_length;
			
			if (p->reader.file.audio_double_sample)
				audio_decoder_decode(p->audio_frame_buffers[p->number_of_frame_buffers], &audio_output_length, audio_buffer, audio_length);
			else
				audio_decoder_decode(p->audio_frame_buffers[p->current_buffer_number] + p->audio_frame_size * i, &audio_output_length, audio_buffer, audio_length);
			
			audio_buffer += audio_length;
			if (p->reader.file.audio_double_sample) {
				double_audio_sample(p->audio_frame_buffers[p->current_buffer_number] + p->audio_frame_size * i, 
							p->audio_frame_buffers[p->number_of_frame_buffers],
							p->reader.file.audio_resample_scale / 2);
				boost_volume(p->audio_frame_buffers[p->current_buffer_number] + p->audio_frame_size * i, p->reader.file.audio_resample_scale << p->reader.file.audio_stereo, volume_boost);
				select_audio_channel(p->audio_frame_buffers[p->current_buffer_number] + p->audio_frame_size * i, p->reader.file.audio_resample_scale << p->reader.file.audio_stereo, audio_channel);
				memset(p->audio_frame_buffers[p->number_of_frame_buffers], 0, p->audio_frame_size);
			}
			else {
				boost_volume(p->audio_frame_buffers[p->current_buffer_number] + p->audio_frame_size * i, p->reader.file.audio_resample_scale << p->reader.file.audio_stereo, volume_boost);
				select_audio_channel(p->audio_frame_buffers[p->current_buffer_number] + p->audio_frame_size * i, p->reader.file.audio_resample_scale << p->reader.file.audio_stereo, audio_channel);
			}
		}
	}
	
	pmp_gu_wait();

	p->current_buffer_number = (p->current_buffer_number + 1) % p->number_of_frame_buffers;
	sceKernelDcacheWritebackInvalidateAll();


	return(audio_result);
}
