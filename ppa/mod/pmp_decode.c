/*
PMP Mod
Copyright (C) 2006 jonny

Homepage: http://jonny.leffe.dnsalias.com
E-mail:   jonny@leffe.dnsalias.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
av decoding in a ring buffer
*/


#include "pmp_decode.h"


void pmp_decode_safe_constructor(struct pmp_decode_struct *p)
	{
	pmp_read_safe_constructor(&p->reader);

	avc_safe_constructor(&p->avc);
	mp4v_safe_constructor(&p->mp4v);

	int i = 0;
	for (; i < maximum_frame_buffers; i++)
		{
		p->video_frame_buffers[i] = 0;
		p->audio_frame_buffers[i] = 0;
		}
	}


void pmp_decode_close(struct pmp_decode_struct *p, int pspType)
	{
	pmp_read_close(&p->reader);
	
	if (p->video_format == 1)
		avc_close(&p->avc);
	else
		mp4v_close(&p->mp4v);

	if (p->audio_decoder == 0 )
		audio_decoder_close();

	int i = 0;
	if (m33IsTVOutSupported(pspType)) 
		{
		for (; i < maximum_frame_buffers; i++)
			{
			if (p->audio_frame_buffers[i] != 0) free_64(p->audio_frame_buffers[i]);
			}
		}
	else
		{
		for (; i < maximum_frame_buffers; i++)
			{
			if (p->video_frame_buffers[i] != 0) free_64(p->video_frame_buffers[i]);
			if (p->audio_frame_buffers[i] != 0) free_64(p->audio_frame_buffers[i]);
			}
		}

	pmp_decode_safe_constructor(p);
	}


char *pmp_decode_open(struct pmp_decode_struct *p, char *s, int pspType, int tvAspectRatio, int tvWidth, int tvHeight, int videoMode)
	{
	pmp_decode_safe_constructor(p);




	char *result = pmp_read_open(&p->reader, FF_INPUT_BUFFER_PADDING_SIZE, s);
	if (result != 0)
		{
		pmp_decode_close(p, pspType);
		return(result);
		}

	p->audio_frame_size = (p->reader.file.header.audio.scale << 1) << p->reader.file.header.audio.stereo;


	p->video_format = p->reader.file.header.video.format;

	if ( p->video_format == 1 )
		result = avc_open(&p->avc, p->reader.file.maximum_packet_size);
	else
		result = mp4v_open(&p->mp4v, p->reader.file.maximum_packet_size);
	if (result != 0)
		{
		pmp_decode_close(p, pspType);
		return(result);
		}




	aspect_ratio_struct_init(p->reader.file.header.video.width, p->reader.file.header.video.height, pspType, tvAspectRatio, tvWidth, tvHeight, videoMode);


	cpu_clock_set_maximum();
	
	//*/
	if ( p->reader.file.header.audio.format == 0 )
		p->audio_decoder = audio_decoder_open(0x1002, p->reader.file.header.audio.rate, p->reader.file.header.audio.scale, 0);
	else if ( p->reader.file.header.audio.format == 1 )
		p->audio_decoder = audio_decoder_open(0x1003, p->reader.file.header.audio.rate, p->reader.file.header.audio.scale, 0);
	else if ( p->reader.file.header.audio.format == 2 || p->reader.file.header.audio.format == 21 || p->reader.file.header.audio.format == 22 ) {
		if ( p->reader.file.header.audio.format == 2 )
			p->audio_decoder = audio_decoder_open(0x1001, p->reader.file.header.audio.rate, p->reader.file.header.audio.scale, 384);
		else if (p->reader.file.header.audio.format == 21)
			p->audio_decoder = audio_decoder_open(0x1001, p->reader.file.header.audio.rate, p->reader.file.header.audio.scale, 304);
		else if (p->reader.file.header.audio.format == 22)
			p->audio_decoder = audio_decoder_open(0x1001, p->reader.file.header.audio.rate, p->reader.file.header.audio.scale, 192);
	}
	else
		p->audio_decoder = -1;
	if (p->audio_decoder == -1)
		{
		pmp_decode_close(p, pspType);
		return("pmp_decode_open: avcodec_find_decoder failed on CODEC_ID_MP3");
		}
	//*/
	cpu_clock_set_minimum();

	int i = 0;
	if (m33IsTVOutSupported(pspType)) 
		{
		p->output_texture_width = 768;
		p->number_of_frame_buffers = 8;
		
		i = 0;
		for (; i < p->number_of_frame_buffers; i++)
			{
			p->video_frame_buffers[i] = (void*)(0x0a000000 + (i+1)*1572864);
			memset(p->video_frame_buffers[i], 0, 1572864);
			}
		}
	else 
		{
		p->output_texture_width = 512;
		p->number_of_frame_buffers = 0;
	
		i = 0;
		for (; i < maximum_frame_buffers; i++)
			{
			p->video_frame_buffers[i] = malloc_64(557056);
	
			if (p->video_frame_buffers[i] == 0)
				{
				break;
				}
	
			memset(p->video_frame_buffers[i], 0, 557056);
	
			p->number_of_frame_buffers ++;
			}
	
	
		if (p->number_of_frame_buffers < (number_of_free_video_frame_buffers + 4))
			{
			pmp_decode_close(p, pspType);
			return("pmp_decode_open: number_of_frame_buffers < 4");
			}
	
	
		p->number_of_frame_buffers -= number_of_free_video_frame_buffers;
	
		i = 0;
		for (; i < number_of_free_video_frame_buffers; i++)
			{
			free_64(p->video_frame_buffers[p->number_of_frame_buffers + i]);
	
			p->video_frame_buffers[p->number_of_frame_buffers + i] = 0;
			}
		}
	i = 0;
	for (; i <= p->number_of_frame_buffers; i++)
		{
		// p->audio_frame_buffers[p->number_of_frame_buffers] is a null buffer
	
		p->audio_frame_buffers[i] = malloc_64(p->audio_frame_size * p->reader.file.header.audio.maximum_number_of_frames);

		if (p->audio_frame_buffers[i] == 0)
			{
			pmp_decode_close(p, pspType);
			return("pmp_decode_open: malloc_64 failed on audio_frame_buffers");
			}

		memset(p->audio_frame_buffers[i], 0, p->audio_frame_size * p->reader.file.header.audio.maximum_number_of_frames);
		}

	sceDisplayWaitVblankStart();
	sceDisplaySetFrameBuf(p->video_frame_buffers[0], p->output_texture_width, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_IMMEDIATE);




	p->current_buffer_number = 0;
	
	pmp_gu_init_previous_values();
	
	return(0);
	}


static void boost_volume(short *audio_buffer, unsigned int number_of_samples, unsigned int volume_boost)
	{
	if (volume_boost != 0)
		{
		while (number_of_samples--)
			{
			int sample = *audio_buffer;
			sample <<= volume_boost;


			if (sample > 32767)
				{
				*audio_buffer++ = 32767;
				}
			else if (sample < -32768)
				{
				*audio_buffer++ = -32768;
				}
			else
				{
				*audio_buffer++ = sample;
				}
			}
		}
	}

//add by cooleyes 2007/02/01
static void select_audio_channel(short *audio_buffer, unsigned int number_of_samples, int audio_channel)
	{
	if (audio_channel != 0)
		{
		if(audio_channel == -1)
			{
			while(number_of_samples)
				{
				*(audio_buffer+1) = *audio_buffer;
				audio_buffer+=2;
				number_of_samples-=2;
				}
			}
		else
			{
			while(number_of_samples)
				{
				*audio_buffer = *(audio_buffer+1);
				audio_buffer+=2;
				number_of_samples-=2;
				}
			}
		}
	}
//add end

//modify by cooleyes 2007/02/01
char *pmp_decode_get(struct pmp_decode_struct *p, unsigned int frame_number, unsigned int audio_stream, int audio_channel, int decode_audio, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop)
//char *pmp_decode_get(struct pmp_decode_struct *p, unsigned int frame_number, unsigned int audio_stream, int decode_audio, unsigned int volume_boost, unsigned int aspect_ratio, unsigned int zoom, unsigned int luminosity_boost, unsigned int show_interface, unsigned int show_subtitle, unsigned int subtitle_format, unsigned int loop)
//modify end
	{
	struct pmp_read_output_struct packet;


	char *result = pmp_read_get(&p->reader, frame_number, audio_stream, &packet);
	if (result != 0)
		{
		return(result);
		}


	sceKernelDcacheWritebackInvalidateAll();
	int y_width, u_width, v_width, height;
	
	if (p->video_format==1)
		result = avc_get(&p->avc, packet.video_buffer, packet.video_length, pmp_gu_rgb_buffer);
	else
		result = mp4v_get_rgb(&p->mp4v, packet.video_buffer, packet.video_length, pmp_gu_rgb_buffer); 
	if (result != 0)
		{
		return(result);
		}


	if (show_interface == 1)
		{
		draw_interface
			(
			p->reader.file.header.video.scale,
			p->reader.file.header.video.rate,
			p->reader.file.header.video.number_of_frames,
			frame_number,
			aspect_ratio,
			zoom,
			luminosity_boost,
			audio_stream,
			volume_boost,
			loop
			);
		}


	sceKernelDcacheWritebackInvalidateAll();
	//if (p->video_format==1)
	//	pmp_gu_draw(aspect_ratio, zoom, luminosity_boost, show_interface, show_subtitle, subtitle_format, frame_number, p->video_frame_buffers[p->current_buffer_number]);
	//else
		pmp_gu_draw(aspect_ratio, zoom, luminosity_boost, show_interface, show_subtitle, subtitle_format, frame_number, p->video_frame_buffers[p->current_buffer_number]);



	p->output_frame_buffers[p->current_buffer_number].number_of_audio_frames = packet.number_of_audio_frames;
	p->output_frame_buffers[p->current_buffer_number].first_delay            = packet.first_delay;
	p->output_frame_buffers[p->current_buffer_number].last_delay             = packet.last_delay;
	p->output_frame_buffers[p->current_buffer_number].video_frame            = p->video_frame_buffers[p->current_buffer_number];




	char *audio_result = 0;


	if (packet.audio_buffer == 0 || decode_audio == 0)
		{
		p->output_frame_buffers[p->current_buffer_number].audio_frame = p->audio_frame_buffers[p->number_of_frame_buffers];
		}
	else
		{
		p->output_frame_buffers[p->current_buffer_number].audio_frame = p->audio_frame_buffers[p->current_buffer_number];


		void *audio_buffer = packet.audio_buffer;


		int i = 0;
		for (; i < packet.number_of_audio_frames; i++)
			{
			int audio_length = packet.audio_length[i];
			int audio_output_length;

			if (audio_decoder_decode(p->audio_frame_buffers[p->current_buffer_number] + p->audio_frame_size * i, &audio_output_length, audio_buffer, audio_length) != audio_length)
				{
				audio_result = "pmp_decode_get: avcodec_decode_audio failed";
				break;
				}

			audio_buffer += audio_length;


			if (audio_output_length != p->audio_frame_size)
				{
				if (audio_output_length == 0)
					{
					}
				else if (audio_output_length < p->audio_frame_size)
					{
					audio_result = "pmp_decode_get: audio_output_length < audio_frame_size";
					break;
					}
				else if (audio_output_length > p->audio_frame_size)
					{
					audio_result = "pmp_decode_get: audio_output_length > audio_frame_size (severe error)";
					break;
					}
				}
			else
				{
				boost_volume(p->audio_frame_buffers[p->current_buffer_number] + p->audio_frame_size * i, p->reader.file.header.audio.scale << p->reader.file.header.audio.stereo, volume_boost);
				select_audio_channel(p->audio_frame_buffers[p->current_buffer_number] + p->audio_frame_size * i, p->reader.file.header.audio.scale << p->reader.file.header.audio.stereo, audio_channel);
				}
			}
		}
	
	
	
	
	pmp_gu_wait();




	p->current_buffer_number = (p->current_buffer_number + 1) % p->number_of_frame_buffers;
	sceKernelDcacheWritebackInvalidateAll();


	return(audio_result);
	}
