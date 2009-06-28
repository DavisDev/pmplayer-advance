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
#include "mp4_play.h"


void mp4_play_safe_constructor(struct mp4_play_struct *p) {
	p->audio_reserved = -1;

	p->semaphore_can_get_video   = -1;
	p->semaphore_can_put_video   = -1;
	p->semaphore_can_get_audio   = -1;
	p->semaphore_can_put_audio   = -1;

	p->output_thread = -1;
	p->show_thread   = -1;	
	p->demux_thread  = -1;
	
	mp4_decode_safe_constructor(&p->decoder);
}


void mp4_play_close(struct mp4_play_struct *p, int usePos, int pspType) {
	
	if (!(p->audio_reserved < 0)) {
		while(sceAudioGetChannelRestLen(0) > 0 );
		sceAudioChRelease(0);
	}
	cooleyesAudioSetFrequency(sceKernelDevkitVersion(), 44100);

	if (!(p->semaphore_can_get_video   < 0)) sceKernelDeleteSema(p->semaphore_can_get_video);
	if (!(p->semaphore_can_put_video   < 0)) sceKernelDeleteSema(p->semaphore_can_put_video);
	if (!(p->semaphore_can_get_audio   < 0)) sceKernelDeleteSema(p->semaphore_can_get_audio);
	if (!(p->semaphore_can_put_audio   < 0)) sceKernelDeleteSema(p->semaphore_can_put_audio);
	
	if (!(p->output_thread < 0)) sceKernelDeleteThread(p->output_thread);
	if (!(p->show_thread   < 0)) sceKernelDeleteThread(p->show_thread);
	if (!(p->demux_thread   < 0)) sceKernelDeleteThread(p->demux_thread);

	mp4_decode_close(&p->decoder, pspType);

	int i = 0;
	for (i=0; i<p->subtitle_count; i++)
		subtitle_parse_close( &subtitle_parser[i] );
	
	//if (usePos) mp4_stat_save( p );
	mp4_stat_save( p );
	
	mp4_play_safe_constructor(p);
}
	

static int mp4_wait(volatile struct mp4_play_struct *p, SceUID s, char *e) {
	SceUInt t = 1;
	
	int result = sceKernelWaitSema(s, 1, &t);

	if (result == SCE_KERNEL_ERROR_OK) {
		return(1);
	}
	else if (result == SCE_KERNEL_ERROR_WAIT_TIMEOUT) {
		return(0);
	}
	else {
		p->return_result  = e;
		p->return_request = 1;
		return(-1);
	}
	
}

static int mp4_avsync_status(int audio_timestamp, int video_timestamp, int video_frame_duration) {

	// if video ahead of audio, do nothing
	if(video_timestamp - audio_timestamp > 2 * video_frame_duration)
		return 0;

	// if audio ahead of video, skip frame
	if(audio_timestamp - video_timestamp > 2 * video_frame_duration)
		return 2;

	return 1;
}


static int mp4_show_thread(SceSize input_length, void *input) {
	volatile struct mp4_play_struct *p = *((void **) input);


	p->current_video_buffer_number = 0;
	int wait;
	int avsync_status = 1;
	int first = 1;
	
	while (p->return_request == 0) {
		
		wait = mp4_wait(p, p->semaphore_can_get_video, "mp4_show_thread: sceKernelWaitSema failed on semaphore_can_get_video");
		if ( wait == -1) {
			break;
		}
		else if ( wait == 1 ) {
			
			avsync_status = mp4_avsync_status( p->decoder.output_audio_frame_buffers[p->current_audio_buffer_number].timestamp,
				p->decoder.output_video_frame_buffers[p->current_video_buffer_number].timestamp,
				p->decoder.video_frame_duration);
			
			if (p->seek != 0)
				avsync_status = 1;
			if(avsync_status > 0) {
				if(avsync_status == 1) {
					sceDisplayWaitVblankStart();
					sceDisplaySetFrameBuf(p->decoder.output_video_frame_buffers[p->current_video_buffer_number].data, p->decoder.output_texture_width, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_IMMEDIATE);
				}
				p->current_video_buffer_number = (p->current_video_buffer_number + 1) % p->decoder.number_of_frame_buffers;
				if ( first == 1 ) {
					first = 0;
				}
				else {
					if (sceKernelSignalSema(p->semaphore_can_put_video, 1) < 0) {
						p->return_result  = "mp4_show_thread: sceKernelSignalSema failed on semaphore_can_put_video";
						p->return_request = 1;
						break;
					}		
				}		
			}
			else {
				if (sceKernelSignalSema(p->semaphore_can_get_video, 1) < 0) {
					p->return_result  = "mp4_show_thread: sceKernelSignalSema failed on semaphore_can_get_video1";
					p->return_request = 1;
					break;
				}
			}
		}
		else {
			sceKernelDelayThread(1);
		}
	
		while (p->return_request == 0 && p->paused == 1) {
			sceKernelDelayThread(100000);
		}
		
	}


	return(0);
}



static unsigned int FONTCOLORS[] = { 
	0xffffff,
	0xff0000,
	0x00ff00,
	0x0000ff,
	0xffff00,
	0x00ffff
};

static unsigned int BORDERCOLORS[] = {	
	0x000000,
	0x7f0000,
	0x007f00,
	0x00007f,
	0x7f7f00,
	0x007f7f
};

static void mp4_input(volatile struct mp4_play_struct *p, SceCtrlData *previous_controller) {
	scePowerTick(0);


	SceCtrlData controller;
	sceCtrlReadBufferPositive(&controller, 1);
	if(1)
	{
		if (((controller.Buttons & PSP_CTRL_TRIANGLE) == 0) && (previous_controller->Buttons & PSP_CTRL_TRIANGLE)) {
			p->return_request = 1;
			p->return_result = "exit: manual";
		}
		else {
			if (p->paused == 1) {
				p->seek = 0;

				if ((controller.Buttons & PSP_CTRL_SQUARE) && ((previous_controller->Buttons & PSP_CTRL_SQUARE) == 0)) {
					p->paused = 0;
				}
				else if ((controller.Buttons & PSP_CTRL_CIRCLE) && ((previous_controller->Buttons & PSP_CTRL_CIRCLE) == 0)) {
					make_screenshot();
				}
			}
			else {
				if (controller.Buttons & PSP_CTRL_CROSS) {
					if (controller.Buttons & PSP_CTRL_RIGHT) {
						p->seek = 2;
					}
					else if (controller.Buttons & PSP_CTRL_LEFT) {
						p->seek = -2;
					}
					else {
						p->seek = 0;

						if ((controller.Buttons & PSP_CTRL_UP) && ((previous_controller->Buttons & PSP_CTRL_UP) == 0)) {
							if (p->zoom != 200) {
								p->zoom += 5;
							}
						}
						else if ((controller.Buttons & PSP_CTRL_DOWN) && ((previous_controller->Buttons & PSP_CTRL_DOWN) == 0)) {
							if (p->zoom != 100) {
								p->zoom -= 5;
							}
						}
						else if ((controller.Buttons & PSP_CTRL_RTRIGGER) && ((previous_controller->Buttons & PSP_CTRL_RTRIGGER) == 0)) {
							if (p->loop == 0) {
								p->loop = 1;
							}
							else {
								p->loop = 0;
							}
						}
						else if ((controller.Buttons & PSP_CTRL_LTRIGGER) && ((previous_controller->Buttons & PSP_CTRL_LTRIGGER) == 0)) {
							if (p->subtitle_count) {
								p->subtitle = (p->subtitle + 1)%(p->subtitle_count+1);
								if (p->subtitle)
									snprintf(info_string,512,"[i]%s[/i]", subtitle_parser[p->subtitle-1].filename);
								else
									snprintf(info_string,512,"[i]no subtitle[/i]");
								info_count = 60;
							}
						}
						else if ((controller.Buttons & PSP_CTRL_SQUARE) && ((previous_controller->Buttons & PSP_CTRL_SQUARE) == 0)) {
							p->subtitle_fontcolor = (p->subtitle_fontcolor+1)%NUMBER_OF_FONTCOLORS;
							gu_font_color_set( FONTCOLORS[p->subtitle_fontcolor] );
						}
						else if ((controller.Buttons & PSP_CTRL_CIRCLE) && ((previous_controller->Buttons & PSP_CTRL_CIRCLE) == 0)) {
							p->subtitle_bordercolor = (p->subtitle_bordercolor+1)%NUMBER_OF_BORDERCOLORS;
							gu_font_border_color_set( BORDERCOLORS[p->subtitle_bordercolor] );
						}
						else if ((controller.Buttons & PSP_CTRL_SELECT) && ((previous_controller->Buttons & PSP_CTRL_SELECT) == 0)) {
							if(p->audio_channel > -1) {
								p->audio_channel--;
							}
							else {
								p->audio_channel = 1;
							}
						}
						else if ((controller.Buttons & PSP_CTRL_START) && ((previous_controller->Buttons & PSP_CTRL_START) == 0)) {
							gu_lcd_output_inversion_set();
						}
					}
				}
				else if ((controller.Buttons & PSP_CTRL_SQUARE) && ((previous_controller->Buttons & PSP_CTRL_SQUARE) == 0)) {
					p->paused = 1;
					p->seek   = 0;
				}
				else if (controller.Buttons & PSP_CTRL_RIGHT) {
					if (p->seek != 2) {
						p->seek ++;
					}
				}
				else if (controller.Buttons & PSP_CTRL_LEFT) {
					if (p->seek != -2) {
						p->seek --;
					}
				}
				else if ((controller.Buttons & PSP_CTRL_SELECT) && ((previous_controller->Buttons & PSP_CTRL_SELECT) == 0)) {
					if (p->audio_stream + 1 == p->decoder.reader.file.audio_tracks) {
						p->audio_stream = 0;
					}
					else {
						p->audio_stream ++;
					}
				}
				else if ((controller.Buttons & PSP_CTRL_UP) && ((previous_controller->Buttons & PSP_CTRL_UP) == 0)) {
					if (p->volume_boost != 6) {
						p->volume_boost ++;
					}
					pcm_set_normalize_ratio(p->volume_boost);
				}
				else if ((controller.Buttons & PSP_CTRL_DOWN) && ((previous_controller->Buttons & PSP_CTRL_DOWN) == 0)) {
					if (p->volume_boost != 0) {
						p->volume_boost --;
					}
					pcm_set_normalize_ratio(p->volume_boost);
				}
				else if ((controller.Buttons & PSP_CTRL_RTRIGGER) && ((previous_controller->Buttons & PSP_CTRL_RTRIGGER) == 0)) {
					if (p->luminosity_boost != (number_of_luminosity_boosts - 1)) {
						p->luminosity_boost ++;
					}
				}
				else if ((controller.Buttons & PSP_CTRL_LTRIGGER) && ((previous_controller->Buttons & PSP_CTRL_LTRIGGER) == 0)) {
					if (p->luminosity_boost != 0) {
						p->luminosity_boost --;
					}
				}
				else if ((controller.Buttons & PSP_CTRL_START) && ((previous_controller->Buttons & PSP_CTRL_START) == 0)) {
					if (p->aspect_ratio != (number_of_aspect_ratios - 1)) {
						p->aspect_ratio ++;
					}
					else {
						p->aspect_ratio = 0;
					}
				}
				else if ((controller.Buttons & PSP_CTRL_CIRCLE) && ((previous_controller->Buttons & PSP_CTRL_CIRCLE) == 0)) {
					if (p->show_interface == 0) {
						p->show_interface = 1;
					}
					else {
						p->show_interface = 0;
					}
				}
				else {
					//p->seek = 0;
				}
			}
		}
	}


	*previous_controller = controller;
}


static int mp4_output_thread(SceSize input_length, void *input) {
	volatile struct mp4_play_struct *p = *((void **) input);

	p->current_audio_buffer_number = 0;
	int wait;
	int first = 1;
	SceInt32 volume = 0;
	
	while (p->return_request == 0) {
		volatile struct mp4_decode_buffer_struct *current_buffer = &p->decoder.output_audio_frame_buffers[p->current_audio_buffer_number];
		
		wait = mp4_wait(p, p->semaphore_can_get_audio, "mp4_output_thread: sceKernelWaitSema failed on semaphore_can_get_audio");
		if ( wait == -1) {
			break;
		}
		else if ( wait == 1 ) {
			if ( volume < PSP_AUDIO_VOLUME_MAX ) {
				volume += PSP_AUDIO_VOLUME_MAX/5;
				if ( volume >= PSP_AUDIO_VOLUME_MAX ) 
					volume = PSP_AUDIO_VOLUME_MAX;
			}
			p->current_timestamp = current_buffer->timestamp;
			if ( p->seek == 0 )
				sceAudioOutputBlocking(0, volume, current_buffer->data);
			p->current_audio_buffer_number = (p->current_audio_buffer_number + 1) % p->decoder.number_of_frame_buffers;
			if (first == 1) {
				first = 0;
			}
			else {	
				if (sceKernelSignalSema(p->semaphore_can_put_audio, 1) < 0) {
					p->return_result  = "mp4_output_thread: sceKernelSignalSema failed on semaphore_can_put_audio";
					p->return_request = 1;
					break;
				}
			}
			
		}
		else {
			sceKernelDelayThread(1);
		}

		while (p->return_request == 0 && p->paused == 1) {
			sceKernelDelayThread(100000);
		}
	}


	return(0);
}

void mp4_play_do_seek(volatile struct mp4_play_struct *p) {
	
	p->last_keyframe_pos = p->current_timestamp;
		
	if (p->resume_pos>0) {
		int pos = p->resume_pos;
		p->resume_pos = 0;
		mp4_decode_seek((struct mp4_decode_struct *) &p->decoder, pos, pos);
		return;
	}
	
	int keyframes = 0;
	if (p->seek > 0) {
		keyframes++;
		
		if (p->seek == 2) {
			keyframes++;
		}
		
		mp4_decode_keyframe_forward((struct mp4_decode_struct *) &p->decoder, keyframes);

		return;
	}
	
	if (p->seek < 0) {
		keyframes++;

		if (p->seek == -2) {
			keyframes++;
		}
		
		int i;
		for(i = 0; i < p->decoder.number_of_frame_buffers; i++)
			p->decoder.output_video_frame_buffers[i].timestamp = -p->decoder.video_frame_duration;
		
		mp4_decode_keyframe_backward((struct mp4_decode_struct *) &p->decoder, keyframes);
		
		return;
	}
	
	return;
	
}

void mp4_play_reset(volatile struct mp4_play_struct *p) {
	int i;
	for(i = 0; i < p->decoder.number_of_frame_buffers; i++)
		p->decoder.output_video_frame_buffers[i].timestamp = -p->decoder.video_frame_duration;
	mp4_decode_reset((struct mp4_decode_struct *) &p->decoder);
}

static int mp4_demux_thread(SceSize input_length, void *input) {
	volatile struct mp4_play_struct *p = *((void **) input);
	int cached_video_frame = 0;
	
	while(p->return_request == 0 && !mp4_decode_is_eof((struct mp4_decode_struct *) &p->decoder)) {
		int wait;
		char* result;
		
		wait = mp4_wait(p, p->semaphore_can_put_audio, "mp4_play_start: sceKernelWaitSema failed on semaphore_can_put_audio");
		if ( wait == -1) {
			p->return_request = 1;
			break;
		}
		else if ( wait == 1) {
			result = mp4_decode_get_audio((struct mp4_decode_struct *) &p->decoder, p->audio_stream, p->audio_channel, 1, p->volume_boost);
			if (result != 0) {
				p->return_result  = result;
				p->return_request = 1;
				break;
			}

			if (sceKernelSignalSema(p->semaphore_can_get_audio, 1) < 0) {
				p->return_result  = "mp4_play_start: sceKernelSignalSema failed on semaphore_can_get_audio";
				p->return_request = 1;
				break;
			}
		}
		
		wait = mp4_wait(p, p->semaphore_can_put_video, "mp4_play_start: sceKernelWaitSema failed on semaphore_can_put_video");
		if ( wait == -1) {
			p->return_request = 1;
			break;
		}
		else if ( wait == 1) {
			if ( cached_video_frame > 0 ) {
				result = mp4_decode_get_cached_video((struct mp4_decode_struct *) &p->decoder, cached_video_frame, p->audio_stream, p->volume_boost, p->aspect_ratio, p->zoom, p->luminosity_boost, p->show_interface, p->subtitle, p->subtitle_format, p->loop);
				if (result != 0) {
					p->return_result  = result;
					p->return_request = 1;
					break;
				}
				cached_video_frame--;
				
				if (sceKernelSignalSema(p->semaphore_can_get_video, 1) < 0) {
					p->return_result  = "mp4_play_start: sceKernelSignalSema failed on semaphore_can_get_video";
					p->return_request = 1;
					break;
				}
			}
			else {
				result = mp4_decode_get_video((struct mp4_decode_struct *) &p->decoder, p->audio_stream, p->volume_boost, p->aspect_ratio, p->zoom, p->luminosity_boost, p->show_interface, p->subtitle, p->subtitle_format, p->loop, &cached_video_frame);
				if (result != 0) {
					p->return_result  = result;
					p->return_request = 1;
					break;
				}
				if ( cached_video_frame >= 0 ) {
				
					if (sceKernelSignalSema(p->semaphore_can_get_video, 1) < 0) {
						p->return_result  = "mp4_play_start: sceKernelSignalSema failed on semaphore_can_get_video";
						p->return_request = 1;
						break;
					}
				}
				else {
					if (sceKernelSignalSema(p->semaphore_can_put_video, 1) < 0) {
						p->return_result  = "mp4_play_start: sceKernelSignalSema failed on semaphore_can_put_video1";
						p->return_request = 1;
						break;
					}
				}
			}
			
		}	
		if ( p->decoder.reader.file.audio_double_sample == 0 ) {
			wait = mp4_wait(p, p->semaphore_can_put_audio, "mp4_play_start: sceKernelWaitSema failed on semaphore_can_put_audio");
			if ( wait == -1) {
				p->return_request = 1;
				break;
			}
			else if ( wait == 1) {
				result = mp4_decode_get_audio((struct mp4_decode_struct *) &p->decoder, p->audio_stream, p->audio_channel, 1, p->volume_boost);
				if (result != 0) {
					p->return_result  = result;
					p->return_request = 1;
					break;
				}
	
				if (sceKernelSignalSema(p->semaphore_can_get_audio, 1) < 0) {
					p->return_result  = "mp4_play_start: sceKernelSignalSema failed on semaphore_can_get_audio";
					p->return_request = 1;
					break;
				}
			}		
		}
		
		mp4_play_do_seek(p);
		
		if ( mp4_decode_is_eof((struct mp4_decode_struct *) &p->decoder) ) {

			if (p->loop == 1) {	
				cached_video_frame = 0;
				mp4_play_reset(p);
			}
			else {
				p->return_request = 1;
				break;
			}
		}
		sceKernelDelayThread(10);
	}
	
}
char *mp4_play_start(volatile struct mp4_play_struct *p) {
	sceKernelStartThread(p->output_thread, 4, &p);
	sceKernelStartThread(p->show_thread,   4, &p);
	sceKernelStartThread(p->demux_thread,   4, &p);

	
	
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	SceCtrlData previous_controller;
	sceCtrlPeekBufferPositive(&previous_controller, 1);


	while (p->return_request == 0 && !mp4_decode_is_eof((struct mp4_decode_struct *) &p->decoder)) {
		
		
		mp4_input(p, &previous_controller);
		
	}

	if (mp4_decode_is_eof((struct mp4_decode_struct *) &p->decoder)) {
		p->last_keyframe_pos = 0;
	}
	
	sceKernelDelayThread(1000000);
	p->return_request = 1;




	sceKernelWaitThreadEnd(p->output_thread, 0);
	sceKernelWaitThreadEnd(p->show_thread,   0);
	sceKernelWaitThreadEnd(p->demux_thread,   0);


	return(p->return_result);
}


char *mp4_play_open(struct mp4_play_struct *p, struct movie_file_struct *movie, int usePos, int pspType, int tvAspectRatio, int tvWidth, int tvHeight, int videoMode) {
	mp4_play_safe_constructor(p);
	p->subtitle = 0;
	p->subtitle_count = 0;

	
	char *result = mp4_decode_open(&p->decoder, movie->movie_file, pspType, tvAspectRatio, tvWidth, tvHeight, videoMode);
	if (result != 0) {
		mp4_play_close(p, 0, pspType);
		return(result);
	}
	
	if ( p->subtitle_count < MAX_SUBTITLES )
		subtitle_parse_search( movie, p->decoder.reader.file.video_rate, p->decoder.reader.file.video_scale, &p->subtitle_count);
	if ( p->subtitle_count > 0 )
		p->subtitle = 1;
	
	if ( cooleyesAudioSetFrequency(sceKernelDevkitVersion(), p->decoder.reader.file.audio_rate) != 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceAudioSetFrequency failed");
	}
	p->audio_reserved = sceAudioChReserve(0, p->decoder.reader.file.audio_resample_scale, PSP_AUDIO_FORMAT_STEREO);
	if (p->audio_reserved < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceAudioChReserve failed");
	}

	p->semaphore_can_get_video = sceKernelCreateSema("can_get_video", 0, 0, p->decoder.number_of_frame_buffers, 0);
	if (p->semaphore_can_get_video < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateSema failed on semaphore_can_get_video");
	}


	p->semaphore_can_put_video = sceKernelCreateSema("can_put_video", 0, p->decoder.number_of_frame_buffers, p->decoder.number_of_frame_buffers, 0);
	if (p->semaphore_can_put_video < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateSema failed on semaphore_can_put_video");
	}


	p->semaphore_can_get_audio = sceKernelCreateSema("can_get_audio", 0, 0, p->decoder.number_of_frame_buffers, 0);
	if (p->semaphore_can_get_audio < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateSema failed on semaphore_can_get_audio");
	}


	p->semaphore_can_put_audio = sceKernelCreateSema("can_put_audio", 0, p->decoder.number_of_frame_buffers, p->decoder.number_of_frame_buffers, 0);
	if (p->semaphore_can_put_audio < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateSema failed on semaphore_can_put_audio");
	}


	//p->output_thread = sceKernelCreateThread("output", mp4_output_thread, 0x8, 0x10000, 0, 0);
	p->output_thread = sceKernelCreateThread("output", mp4_output_thread, 0x3D, 0x10000, PSP_THREAD_ATTR_USER, 0);
	if (p->output_thread < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateThread failed on output_thread");
	}

	//p->show_thread = sceKernelCreateThread("show", mp4_show_thread, 0x8, 0x10000, 0, 0);
	p->show_thread = sceKernelCreateThread("show", mp4_show_thread, 0x3F, 0x10000, PSP_THREAD_ATTR_USER, 0);
	if (p->show_thread < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateThread failed on show_thread");
	}
	
	p->demux_thread = sceKernelCreateThread("demux", mp4_demux_thread, 0x30, 0x10000, PSP_THREAD_ATTR_USER, 0);
	if (p->demux_thread < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateThread failed on demux_thread");
	}

	p->return_request = 0;
	p->return_result  = 0;


	p->paused = 0;
	p->seek   = 0;
	
	p->current_timestamp = 0;


	p->audio_stream     = 0;
	p->audio_channel    = 0;
	p->volume_boost     = 3;
	p->aspect_ratio     = 0;
	p->zoom             = 100;
	p->luminosity_boost = 0;
	p->show_interface   = 0;
	p->loop             = 0;
	p->resume_pos		= 0;
	p->last_keyframe_pos= 0;
	p->subtitle_format  = (((gufont_haveflags&GU_FONT_HAS_UNICODE_CHARMAP))?1:0);
	p->subtitle_fontcolor = 0;
	p->subtitle_bordercolor = 0;
	
	memcpy(p->hash, movie->movie_hash, 16);
	
	if (usePos) mp4_stat_load( p );

	return(0);
}
