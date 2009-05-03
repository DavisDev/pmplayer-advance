#include "mp4_decode.h"
#include "me_boot_start.h"
#include "audio_util.h"

void mp4_decode_safe_constructor(struct mp4_decode_struct *p) {
	mp4_read_safe_constructor(&p->reader);

	mp4_avc_safe_constructor(&p->avc);
	mp4v_safe_constructor(&p->mp4v);

	int i = 0;
	for (; i < mp4_maximum_frame_buffers; i++) {
		p->video_frame_buffers[i] = 0;
		p->audio_frame_buffers[i] = 0;
	}

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
		for (; i < mp4_maximum_frame_buffers; i++) {
			if (p->audio_frame_buffers[i] != 0) 
				free_64(p->audio_frame_buffers[i]);
		}
	}
	else {
		for (; i < mp4_maximum_frame_buffers; i++) {
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
	
	cpu_clock_set_maximum();

	char *result = mp4_read_open(&p->reader, s);
	if (result != 0) {
		mp4_decode_close(p, pspType);
		return(result);
	}

	p->audio_frame_size = (p->reader.file.audio_resample_scale << 1) << p->reader.file.audio_stereo;


	p->video_format = p->reader.file.video_type;
	
	if ( p->video_format == 0x61766331 /*avc1*/ ) {
		if ( p->reader.file.video_width > 480 || p->reader.file.video_height > 272 ) {
			me_boot_start(1);
		}
		else if ( p->reader.file.info->tracks[p->reader.file.video_track_id]->avc_profile == 0x42 ) {
			me_boot_start(4);
		}
		else {
			me_boot_start(3);
		}
	}
	else {
		me_boot_start(3);
	}

	if ( p->video_format == 0x61766331 /*avc1*/ )
		result = mp4_avc_open(&p->avc, 
			p->reader.file.info->tracks[p->reader.file.video_track_id]->avc_profile,
			((p->reader.file.video_width > 480 || p->reader.file.video_height > 272) ? 5 : 4), 
			p->reader.file.info->tracks[p->reader.file.video_track_id]->avc_sps, 
			p->reader.file.info->tracks[p->reader.file.video_track_id]->avc_sps_size,
			p->reader.file.info->tracks[p->reader.file.video_track_id]->avc_pps, 
			p->reader.file.info->tracks[p->reader.file.video_track_id]->avc_pps_size,
			p->reader.file.info->tracks[p->reader.file.video_track_id]->avc_nal_prefix_size);
	else
		result = mp4v_open_ex(&p->mp4v,
			p->reader.file.info->tracks[p->reader.file.video_track_id]->mp4v_decinfo,
			p->reader.file.info->tracks[p->reader.file.video_track_id]->mp4v_decinfo_size,
			p->reader.file.maximum_video_sample_size);
	if (result != 0) {
		mp4_decode_close(p, pspType);
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
			//p->video_frame_buffers[i] = (void*)(0x0a000000 + i*p->video_frame_size);
			memset(p->video_frame_buffers[i], 0, p->video_frame_size);
		}
	}
	else {
		p->output_texture_width = 512;
		p->number_of_frame_buffers = 0;
		p->video_frame_size = 557056;
		i = 0;
		for (; i < mp4_maximum_frame_buffers; i++) {
			p->video_frame_buffers[i] = malloc_64(p->video_frame_size);
	
			if (p->video_frame_buffers[i] == 0) {
				break;
			}
	
			memset(p->video_frame_buffers[i], 0, p->video_frame_size);
	
			p->number_of_frame_buffers ++;
		}
	
	
		if (p->number_of_frame_buffers < (mp4_number_of_free_video_frame_buffers + 4)) {
			mp4_decode_close(p, pspType);
			return("mp4_decode_open: number_of_frame_buffers < 4");
		}
	
	
		p->number_of_frame_buffers -= mp4_number_of_free_video_frame_buffers;
	
		i = 0;
		for (; i < mp4_number_of_free_video_frame_buffers; i++) {
			free_64(p->video_frame_buffers[p->number_of_frame_buffers + i]);
	
			p->video_frame_buffers[p->number_of_frame_buffers + i] = 0;
		}
	}
	i = 0;
	for (; i <= p->number_of_frame_buffers; i++) {
		// p->audio_frame_buffers[p->number_of_frame_buffers] is a null buffer
	
		p->audio_frame_buffers[i] = malloc_64(p->audio_frame_size);

		if (p->audio_frame_buffers[i] == 0) {
			mp4_decode_close(p, pspType);
			return("mp4_decode_open: malloc_64 failed on audio_frame_buffers");
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



char *mp4_decode_get_audio(struct mp4_decode_struct *p, unsigned int frame_number, unsigned int audio_stream, int audio_channel, int decode_audio, unsigned int volume_boost) {
	char *result;
	struct mp4_read_output_struct a_packet;
	
	result = mp4_read_get_audio(&p->reader, frame_number, audio_stream, &a_packet);
	if (result != 0)
		return(result);
	
	if (a_packet.data == 0 || decode_audio == 0) {
		p->output_audio_frame_buffers[p->current_audio_buffer_number].data = p->audio_frame_buffers[p->number_of_frame_buffers];
	}
	else {
		p->output_audio_frame_buffers[p->current_audio_buffer_number].data = p->audio_frame_buffers[p->current_audio_buffer_number];
		
		int audio_output_length;
			
		if (p->reader.file.audio_double_sample) {
			audio_decoder_decode(p->audio_frame_buffers[p->number_of_frame_buffers], &audio_output_length, a_packet.data, a_packet.size);
			pcm_double_sample(p->audio_frame_buffers[p->current_audio_buffer_number], 
				p->audio_frame_buffers[p->number_of_frame_buffers],
				p->reader.file.audio_resample_scale / 2);
			memset(p->audio_frame_buffers[p->number_of_frame_buffers], 0, p->audio_frame_size);
		}
		else
			audio_decoder_decode(p->audio_frame_buffers[p->current_audio_buffer_number], &audio_output_length, a_packet.data, a_packet.size);
		
		
		pcm_normalize(p->audio_frame_buffers[p->current_audio_buffer_number], p->reader.file.audio_resample_scale << p->reader.file.audio_stereo);
		pcm_select_channel(p->audio_frame_buffers[p->current_audio_buffer_number], p->reader.file.audio_resample_scale << p->reader.file.audio_stereo, audio_channel);	
	}
	uint64_t timestamp = 1000LL;
	timestamp *= frame_number;
	timestamp *= p->reader.file.audio_resample_scale;
	timestamp /= p->reader.file.audio_rate;
	
	p->output_audio_frame_buffers[p->current_audio_buffer_number].timestamp = timestamp;
	p->current_audio_buffer_number = (p->current_audio_buffer_number + 1) % p->number_of_frame_buffers;
	
	return(0);
}

char *mp4_decode_get_cached_video(struct mp4_decode_struct *p, unsigned int pic_num, unsigned int frame_number, unsigned int audio_stream, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop) {
	char *result;
	sceKernelDcacheWritebackInvalidateAll();
	result = mp4_avc_get_cache(&p->avc, pmp_gu_rgb_buffer, pic_num);
	if (result != 0) {
		return(result);
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
	
	pmp_gu_draw(aspect_ratio, zoom, luminosity_boost, show_interface, show_subtitle, subtitle_format, frame_number, p->video_frame_buffers[p->current_video_buffer_number]);
	
	p->output_video_frame_buffers[p->current_video_buffer_number].data            = p->video_frame_buffers[p->current_video_buffer_number];

	pmp_gu_wait();
	
	uint64_t timestamp = 1000LL;
	timestamp *= frame_number;
	timestamp *= p->reader.file.video_scale;
	timestamp /= p->reader.file.video_rate;
	p->output_video_frame_buffers[p->current_video_buffer_number].timestamp = timestamp;

	p->current_video_buffer_number = (p->current_video_buffer_number + 1) % p->number_of_frame_buffers;
	sceKernelDcacheWritebackInvalidateAll();


	return(0);
}

char *mp4_decode_get_video(struct mp4_decode_struct *p, unsigned int read_num, unsigned int frame_number, unsigned int audio_stream, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop, int* pic_num) {
	
	char *result;
	struct mp4_read_output_struct v_packet;
	
	if ( read_num >= p->reader.file.number_of_video_frames ) {
		*pic_num = 1;
	}
	else {
		result = mp4_read_get_video(&p->reader, read_num, &v_packet);
		if (result != 0)
			return result;
		sceKernelDcacheWritebackInvalidateAll();
			
		if (p->video_format == 0x61766331 /*avc1*/)
			result = mp4_avc_get(&p->avc, (read_num==0?3:0), v_packet.data, v_packet.size, pmp_gu_rgb_buffer, pic_num);
		else {
			result = mp4v_get_rgb(&p->mp4v, v_packet.data, v_packet.size, pmp_gu_rgb_buffer);
			*pic_num = 1;
		}
		if (result != 0)
			return(result);
	}
	*pic_num = *pic_num - 1;
	
	if ( *pic_num >= 0 ) {
	
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
		
		pmp_gu_draw(aspect_ratio, zoom, luminosity_boost, show_interface, show_subtitle, subtitle_format, frame_number, p->video_frame_buffers[p->current_video_buffer_number]);
		
		p->output_video_frame_buffers[p->current_video_buffer_number].data            = p->video_frame_buffers[p->current_video_buffer_number];
	
		pmp_gu_wait();
		
		uint64_t timestamp = 1000LL;
		timestamp *= frame_number;
		timestamp *= p->reader.file.video_scale;
		timestamp /= p->reader.file.video_rate;
		p->output_video_frame_buffers[p->current_video_buffer_number].timestamp = timestamp;
	
		p->current_video_buffer_number = (p->current_video_buffer_number + 1) % p->number_of_frame_buffers;
		sceKernelDcacheWritebackInvalidateAll();

	}

	return(0);
}