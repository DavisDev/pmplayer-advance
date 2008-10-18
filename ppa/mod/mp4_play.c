#include "mp4_play.h"


void mp4_play_safe_constructor(struct mp4_play_struct *p) {
	p->audio_reserved = -1;

	p->semaphore_can_get   = -1;
	p->semaphore_can_put   = -1;
	p->semaphore_can_show  = -1;
	p->semaphore_show_done = -1;

	p->output_thread = -1;
	p->show_thread   = -1;

	mp4_decode_safe_constructor(&p->decoder);
}


void mp4_play_close(struct mp4_play_struct *p, int usePos, int pspType) {
	
	if (!(p->audio_reserved < 0)) {
		while(sceAudioGetChannelRestLen(0) > 0 );
		sceAudioChRelease(0);
	}
	cooleyesAudioSetFrequency(sceKernelDevkitVersion(), 44100);

	if (!(p->semaphore_can_get   < 0)) sceKernelDeleteSema(p->semaphore_can_get);
	if (!(p->semaphore_can_put   < 0)) sceKernelDeleteSema(p->semaphore_can_put);
	if (!(p->semaphore_can_show  < 0)) sceKernelDeleteSema(p->semaphore_can_show);
	if (!(p->semaphore_show_done < 0)) sceKernelDeleteSema(p->semaphore_show_done);

	if (!(p->output_thread < 0)) sceKernelDeleteThread(p->output_thread);
	if (!(p->show_thread   < 0)) sceKernelDeleteThread(p->show_thread);

	mp4_decode_close(&p->decoder, pspType);

	int i = 0;
	for (i=0; i<p->subtitle_count; i++)
		subtitle_parse_close( &subtitle_parser[i] );
	
	//if (usePos) mp4_stat_save( p );
	mp4_stat_save( p );
	
	mp4_play_safe_constructor(p);
}
	

static int mp4_wait(volatile struct mp4_play_struct *p, SceUID s, char *e) {
	SceUInt t = 1000000;
	

	while (1) {
		int result = sceKernelWaitSema(s, 1, &t);

		if (result == SCE_KERNEL_ERROR_OK) {
			break;
		}
		else if (result == SCE_KERNEL_ERROR_WAIT_TIMEOUT) {
			sceKernelDelayThread(1);

			if (p->return_request == 1) {
				return(0);
			}
		}
		else {
			p->return_result  = e;
			p->return_request = 1;
			return(0);
		}
	}


	return(1);
}


static int mp4_show_thread(SceSize input_length, void *input) {
	volatile struct mp4_play_struct *p = *((void **) input);


	unsigned int current_buffer_number = 0;


	while (p->return_request == 0) {
		if (mp4_wait(p, p->semaphore_can_show, "mp4_show_thread: sceKernelWaitSema failed on semaphore_can_show") == 0) {
			break;
		}


		sceDisplayWaitVblankStart();
		sceDisplaySetFrameBuf(p->decoder.output_frame_buffers[current_buffer_number].video_frame, p->decoder.output_texture_width, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_IMMEDIATE);


		if (sceKernelSignalSema(p->semaphore_show_done, 1) < 0) {
			p->return_result  = "mp4_show_thread: sceKernelSignalSema failed on semaphore_show_done";
			p->return_request = 1;
			break;
		}


		current_buffer_number = (current_buffer_number + 1) % p->decoder.number_of_frame_buffers;
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
					p->seek = 1;
				}
				else if (controller.Buttons & PSP_CTRL_LEFT) {
					p->seek = -1;
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
					if (p->volume_boost != 3) {
						p->volume_boost ++;
					}
				}
				else if ((controller.Buttons & PSP_CTRL_DOWN) && ((previous_controller->Buttons & PSP_CTRL_DOWN) == 0)) {
					if (p->volume_boost != 0) {
						p->volume_boost --;
					}
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
					p->seek = 0;
				}
			}
		}
	}


	*previous_controller = controller;
}


static int mp4_output_thread(SceSize input_length, void *input) {
	volatile struct mp4_play_struct *p = *((void **) input);


	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	SceCtrlData previous_controller;
	sceCtrlPeekBufferPositive(&previous_controller, 1);


    	unsigned int first_video_frame     = 1;
	unsigned int current_buffer_number = 0;


	while (p->return_request == 0) {
		volatile struct mp4_decode_buffer_struct *current_buffer = &p->decoder.output_frame_buffers[current_buffer_number];


		if (mp4_wait(p, p->semaphore_can_get, "mp4_output_thread: sceKernelWaitSema failed on semaphore_can_get") == 0) {
			break;
		}




		if (sceKernelSignalSema(p->semaphore_can_show, 1) < 0) {
			p->return_result  = "mp4_output_thread: sceKernelSignalSema failed on semaphore_can_show";
			p->return_request = 1;
			break;
		}


		if (p->seek == 0) {
			current_buffer->first_delay -= 500;
			sceKernelDelayThread(current_buffer->first_delay < 1 ? 1 : current_buffer->first_delay);
			if (current_buffer->number_of_audio_frames)
				sceAudioOutputBlocking(0, PSP_AUDIO_VOLUME_MAX, current_buffer->audio_frame);
		}


		mp4_input(p, &previous_controller);


		if (p->seek == 0) {
			int i = 1;
			for (; i < current_buffer->number_of_audio_frames; i++) {
				sceAudioOutputBlocking(0, PSP_AUDIO_VOLUME_MAX, current_buffer->audio_frame + p->decoder.audio_frame_size * i);
			}

			current_buffer->last_delay -= 500;
			sceKernelDelayThread(current_buffer->last_delay < 1 ? 1 : current_buffer->last_delay);
		}


		current_buffer_number = (current_buffer_number + 1) % p->decoder.number_of_frame_buffers;


		if (mp4_wait(p, p->semaphore_show_done, "mp4_output_thread: sceKernelWaitSema failed on semaphore_show_done") == 0) {
			break;
		}


		if (first_video_frame == 1) {
			first_video_frame = 0;
		}
		else {
			if (sceKernelSignalSema(p->semaphore_can_put, 1) < 0) {
				p->return_result  = "mp4_output_thread: sceKernelSignalSema failed on semaphore_can_put";
				p->return_request = 1;
				break;
			}
		}


		while (p->return_request == 0 && p->paused == 1) {
			sceKernelDelayThread(100000);

			mp4_input(p, &previous_controller);
		}
	}


	return(0);
}

static int is_video_keyframe(volatile struct mp4_play_struct *p, int video_frame) {
	int i;
	mp4info_track_t* track = p->decoder.reader.file.info->tracks[p->decoder.reader.file.video_track_id];
	for(i = 0; i < track->stss_entry_count; i++ ) {
		if ( video_frame+1 == track->stss_sync_sample[i] )
			return 1;
	}
	return 0;
	
}

static int mp4_next_video_frame(volatile struct mp4_play_struct *p, int current_video_frame) {
	// save last keyframe pos
	if (is_video_keyframe(p, current_video_frame))
		p->last_keyframe_pos = current_video_frame;
		
	if (p->resume_pos>0) {
		int pos = p->resume_pos;
		p->resume_pos = 0;
		return(pos);
	}
	
	if (p->seek > 0) {
        	int number_of_skips = 0;

        	if (p->seek == 2) {
			number_of_skips = 20;
		}

		int new_video_frame = current_video_frame + 1;

		while (new_video_frame < p->decoder.reader.file.number_of_video_frames) {
			if (is_video_keyframe(p, new_video_frame)) {
				if (number_of_skips == 0) {
					return(new_video_frame);
				}
				else {
					number_of_skips--;
				}
			}

			new_video_frame++;
		}

		return(p->decoder.reader.file.number_of_video_frames);
	}


	if (p->seek < 0) {
        	int number_of_skips = 0;

		if (p->seek == -2) {
			number_of_skips = 20;
		}


		int new_video_frame = current_video_frame - 1;

		while (new_video_frame > 0) {
			if (is_video_keyframe(p, new_video_frame)) {
				if (number_of_skips == 0) {
					return(new_video_frame);
				}
				else {
					number_of_skips--;
				}
			}

			new_video_frame--;
		}

		return(0);
	}


	return(current_video_frame + 1);
}


char *mp4_play_start(volatile struct mp4_play_struct *p) {
	sceKernelStartThread(p->output_thread, 4, &p);
	sceKernelStartThread(p->show_thread,   4, &p);




	int current_video_frame = 0;


	while (p->return_request == 0 && current_video_frame != p->decoder.reader.file.number_of_video_frames) {
		if (mp4_wait(p, p->semaphore_can_put, "mp4_play_start: sceKernelWaitSema failed on semaphore_can_put") == 0) {
			break;
		}

		char *result = mp4_decode_get((struct mp4_decode_struct *) &p->decoder, current_video_frame, p->audio_stream, p->audio_channel, 1, p->volume_boost, p->aspect_ratio, p->zoom, p->luminosity_boost, p->show_interface, p->subtitle, p->subtitle_format, p->loop);
		
		if (result != 0) {
			p->return_result  = result;
			p->return_request = 1;
			break;
		}


		if (sceKernelSignalSema(p->semaphore_can_get, 1) < 0) {
			p->return_result  = "mp4_play_start: sceKernelSignalSema failed on semaphore_can_get";
			p->return_request = 1;
			break;
		}


		current_video_frame = mp4_next_video_frame(p, current_video_frame);


		if ((p->loop == 1) && (current_video_frame == p->decoder.reader.file.number_of_video_frames)) {
			current_video_frame = 0;
		}
	}

	if (current_video_frame == p->decoder.reader.file.number_of_video_frames) {
		p->last_keyframe_pos = 0;
	}
	
	sceKernelDelayThread(1000000);
	p->return_request = 1;




	sceKernelWaitThreadEnd(p->output_thread, 0);
	sceKernelWaitThreadEnd(p->show_thread,   0);


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

	if (subtitle_parse_search( movie, p->decoder.reader.file.video_rate, p->decoder.reader.file.video_scale, &p->subtitle_count)==0) p->subtitle = 1;
	
	if ( cooleyesAudioSetFrequency(sceKernelDevkitVersion(), p->decoder.reader.file.audio_rate) != 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceAudioSetFrequency failed");
	}
	p->audio_reserved = sceAudioChReserve(0, p->decoder.reader.file.audio_resample_scale, PSP_AUDIO_FORMAT_STEREO);
	if (p->audio_reserved < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceAudioChReserve failed");
	}

	p->semaphore_can_get = sceKernelCreateSema("can_get", 0, 0, p->decoder.number_of_frame_buffers, 0);
	if (p->semaphore_can_get < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateSema failed on semaphore_can_get");
	}


	p->semaphore_can_put = sceKernelCreateSema("can_put", 0, p->decoder.number_of_frame_buffers, p->decoder.number_of_frame_buffers, 0);
	if (p->semaphore_can_put < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateSema failed on semaphore_can_put");
	}


	p->semaphore_can_show = sceKernelCreateSema("can_show", 0, 0, 1, 0);
	if (p->semaphore_can_show < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateSema failed on semaphore_can_show");
	}


	p->semaphore_show_done = sceKernelCreateSema("show_done", 0, 0, 1, 0);
	if (p->semaphore_show_done < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateSema failed on semaphore_show_done");
	}




	p->output_thread = sceKernelCreateThread("output", mp4_output_thread, 0x8, 0x10000, 0, 0);
	if (p->output_thread < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateThread failed on output_thread");
	}


	p->show_thread = sceKernelCreateThread("show", mp4_show_thread, 0x8, 0x10000, 0, 0);
	if (p->show_thread < 0) {
		mp4_play_close(p, 0, pspType);
		return("mp4_play_open: sceKernelCreateThread failed on show_thread");
	}




	p->return_request = 0;
	p->return_result  = 0;


	p->paused = 0;
	p->seek   = 0;


	p->audio_stream     = 0;
	p->audio_channel    = 0;
	p->volume_boost     = 0;
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
