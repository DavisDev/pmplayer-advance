#include "mp4_read.h"


void mp4_read_safe_constructor(struct mp4_read_struct *p) {
	mp4_file_safe_constructor(&p->file);

	p->buffered_reader = 0;
	
	p->video_output_buffer = 0;
	p->audio_cache_buffer = 0;
}


void mp4_read_close(struct mp4_read_struct *p) {
	
	mp4_file_close(&p->file);

	if ( p->buffered_reader != 0)
		buffered_reader_close(p->buffered_reader);
		
	if (p->video_output_buffer != 0)
		free_64(p->video_output_buffer); 
	
	if (p->audio_cache_buffer != 0)
		free_64(p->audio_cache_buffer);
	
	mp4_read_safe_constructor(p);
}

static unsigned int get_sample_size(struct mp4_read_struct *reader, int track_id, unsigned int sample) {
	mp4info_track_t* track = reader->file.info->tracks[track_id];
	return (track->stsz_sample_size ? track->stsz_sample_size : track->stsz_sample_size_table[sample]);
}

static int get_sample_offset(struct mp4_read_struct *reader, int track_id, unsigned int sample) {
	mp4info_track_t* track = reader->file.info->tracks[track_id];
	int k;
	unsigned int first_sample, last_sample;
	for( k = 0; k < track->stco_entry_count; k++ ) {
		
		int i, j;
		for( i = 0; i < track->stsc_entry_count-1; i++ ) {
			if ( (k+1) >= track->stsc_first_chunk[i] && (k+1) < track->stsc_first_chunk[i+1] )
				break;
		}
		first_sample = 0;
		for( j = 0; j < i; j++ ) {
			first_sample += ( ( track->stsc_first_chunk[j+1] - track->stsc_first_chunk[j] ) * track->stsc_samples_per_chunk[j] );
		}
		first_sample += ( ( (k+1) - track->stsc_first_chunk[i] ) * track->stsc_samples_per_chunk[i] );
		last_sample = first_sample + track->stsc_samples_per_chunk[i] - 1;
		
		if ( sample >= first_sample && sample <= last_sample )
			break;
	}
	int pos = track->stco_chunk_offset[k];
	while( first_sample < sample ) {
		pos += get_sample_size(reader, track_id, first_sample);
		first_sample++;
	}
	return pos;
}


char *mp4_read_open(struct mp4_read_struct *p, char *s) {
	
	mp4_read_safe_constructor(p);

	char *result = mp4_file_open(&p->file, s);
	if (result != 0) {
		mp4_read_close(p);
		return(result);
	}


	p->buffered_reader = buffered_reader_open(s);
	if (p->buffered_reader == 0) {
		mp4_read_close(p);
		return("mp4_read_open: can't open file");
	}

	p->video_output_buffer = malloc_64(p->file.maximum_video_sample_size);
	if (p->video_output_buffer == 0) {
		mp4_read_close(p);
	}
	memset(p->video_output_buffer, 0, p->file.maximum_video_sample_size);
	
	p->audio_cache_buffer = malloc_64((sizeof(unsigned int)+p->file.maximum_audio_sample_size) * p->file.maximun_audio_sample_number);
	if (p->audio_cache_buffer == 0) {
		mp4_read_close(p);
	}
	memset(p->audio_cache_buffer, 0, (sizeof(unsigned int)+p->file.maximum_audio_sample_size) * p->file.maximun_audio_sample_number);
	p->audio_output_length = (unsigned int*)p->audio_cache_buffer;
	p->audio_output_buffer = p->audio_cache_buffer + sizeof(unsigned int)*p->file.maximun_audio_sample_number;

	
	time_math_interleaving_constructor(&p->interleaving, 
		p->file.video_rate, 
		p->file.video_scale, 
		p->file.audio_rate, 
		p->file.audio_scale);
	time_math_interleaving_get(&p->interleaving);
	
	buffered_reader_seek(p->buffered_reader, get_sample_offset(p, p->file.video_track_id, 0) );
	
	return(0);
}



char *mp4_read_get_video(struct mp4_read_struct *p, unsigned int packet, struct mp4_video_read_output_struct *output) {
	
	int pos = get_sample_offset(p, p->file.video_track_id, packet);
	output->video_length = get_sample_size(p, p->file.video_track_id, packet);
	buffered_reader_seek(p->buffered_reader, pos);
	memset(p->video_output_buffer, 0, p->file.maximum_video_sample_size);
	if ( buffered_reader_read(p->buffered_reader, p->video_output_buffer, output->video_length) < output->video_length ) {
		return "buffered_reader: read failed";
	}
	output->video_buffer = p->video_output_buffer;
	
	return(0);
}

char *mp4_read_get_audio(struct mp4_read_struct *p, unsigned int packet, unsigned int audio_stream, struct mp4_audio_read_output_struct *output){
	
	if (p->interleaving.output_video_frame_number > packet) {
		time_math_interleaving_constructor(&p->interleaving, 
			p->file.video_rate, 
			p->file.video_scale, 
			p->file.audio_rate, 
			p->file.audio_scale);
		time_math_interleaving_get(&p->interleaving);
	}

	while (p->interleaving.output_video_frame_number != packet) {
		time_math_interleaving_get(&p->interleaving);
	}
	mp4info_track_t* track;
	
	unsigned int first_audio_frame, last_audio_frame;
	if ( p->file.audio_double_sample ) {
		first_audio_frame = p->interleaving.output_audio_frame_number / 2;
		last_audio_frame = (p->interleaving.output_audio_frame_number + p->interleaving.output_number_of_audio_frames - 1) / 2;
		output->number_of_audio_frames = last_audio_frame - first_audio_frame + 1;
		output->number_of_skip_audio_parts = p->interleaving.output_audio_frame_number % 2;
		output->number_of_audio_parts = p->interleaving.output_number_of_audio_frames;
	}
	else {
		first_audio_frame = p->interleaving.output_audio_frame_number;
		last_audio_frame = p->interleaving.output_audio_frame_number + p->interleaving.output_number_of_audio_frames - 1;
		output->number_of_audio_frames = p->interleaving.output_number_of_audio_frames;
		output->number_of_skip_audio_parts = 0;
		output->number_of_audio_parts = 0;
	}
	track = p->file.info->tracks[p->file.audio_track_ids[audio_stream]];
	int i;
	void *audio_output_buffer = p->audio_output_buffer;
	memset(audio_output_buffer, 0, p->file.maximum_audio_sample_size * p->file.maximun_audio_sample_number);
	for( i = 0; i < output->number_of_audio_frames; i++ ) {
		if ( first_audio_frame+i >= track->stts_sample_count[0] )
			break;
		int pos =  get_sample_offset(p, p->file.audio_track_ids[audio_stream], first_audio_frame+i);
		p->audio_output_length[i] = get_sample_size(p, p->file.audio_track_ids[audio_stream], first_audio_frame+i);
		buffered_reader_seek(p->buffered_reader, pos);
		if ( buffered_reader_read(p->buffered_reader, audio_output_buffer, p->audio_output_length[i]) < p->audio_output_length[i] ) {
			return "buffered_reader: read failed";
		}
		audio_output_buffer += p->audio_output_length[i];
	}
	
	output->first_delay  = p->interleaving.output_first_delay;
	output->last_delay   = p->interleaving.output_last_delay;
	output->audio_length = p->audio_output_length;
	output->audio_buffer = p->audio_output_buffer;

	return(0);
}
