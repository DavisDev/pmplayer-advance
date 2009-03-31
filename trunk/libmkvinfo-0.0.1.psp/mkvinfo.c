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
 
#include "mkvinfo_type.h"
#include "bufferedio.h"
#include "ebml.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

mkvinfo_t* mkvinfo_open(const char* filename) {
	mkvinfo_t* info = (mkvinfo_t*)malloc(sizeof(mkvinfo_t));
	if (!info)
		return 0;
	memset(info, 0, sizeof(mkvinfo_t));
	
	buffered_io_t io;	
	int32_t result = io_open(filename, (void*)(&io));
	if ( result < 0 ) {
		free(info);
		return 0;
	}
	info->handle = &io;
	parse_ebml(info);
	io_close(&io);
	info->handle = 0;
	
	return info;
}

void mkvinfo_close(mkvinfo_t* info) {
	int32_t i;
	
	if (info->parsed_cues) {
		free(info->parsed_cues);
		info->parsed_cues = 0;
		info->parsed_cues_num = 0;
	}
	if (info->parsed_seekhead) {
		free(info->parsed_seekhead);
		info->parsed_seekhead = 0;
		info->parsed_seekhead_num = 0;
	}
	if (info->indexes) {
		free(info->indexes);
		info->indexes = 0;
		info->total_indexes = 0;
	}

	for (i = 0; i < info->total_tracks; i++) {
		if (info->tracks[i]) {
			
			if (info->tracks[i]->private_data)
				free(info->tracks[i]->private_data);
			free(info->tracks[i]);
			info->tracks[i] = 0;
		}
	}
	
	if (info) free(info);
}

void mkvinfo_dump(mkvinfo_t* info, const char* dumpfile) {
	
	FILE* fp = fopen(dumpfile, "w+");
	fprintf(fp, "time_scale : %d(0x%08X)\n", info->timecode_scale, info->timecode_scale);
	fprintf(fp, "duration : %d\n", info->duration);
	fprintf(fp, "total_tracks : %d\n", info->total_tracks);
	fprintf(fp, "\n");
	int32_t i;
	for(i = 0; i < info->total_tracks; i++) {
		fprintf(fp, "[track%d] : \n", i+1);
		fprintf(fp, "\ttype : %d\n", info->tracks[i]->type);
		
		fprintf(fp, "\ttime_scale : %d\n", info->tracks[i]->time_scale);
		fprintf(fp, "\tduration : %d\n", info->tracks[i]->duration);
		
		fprintf(fp, "\tvideo_type : 0x%08X\n", info->tracks[i]->video_type);
		fprintf(fp, "\tvideo_width : %d\n", info->tracks[i]->width);
		fprintf(fp, "\tvideo_height : %d\n", info->tracks[i]->height);
		fprintf(fp, "\tvideo_display_width : %d\n", info->tracks[i]->display_width);
		fprintf(fp, "\tvideo_display_height : %d\n", info->tracks[i]->display_height);
		
		fprintf(fp, "\taudio_type : 0x%08X\n", info->tracks[i]->audio_type);
		fprintf(fp, "\taudio_channels : %d\n", info->tracks[i]->channels);
		fprintf(fp, "\taudio_samplerate : %d\n", info->tracks[i]->samplerate);
		fprintf(fp, "\taudio_samplebits : %d\n", info->tracks[i]->samplebits);
		
		fprintf(fp, "\tprivate_data[%d] : { ", info->tracks[i]->private_size);
		int32_t j;
		for(j = 0; j < info->tracks[i]->private_size; j++)
			fprintf(fp, "%02X ", info->tracks[i]->private_data[j]);
		fprintf(fp, "}\n");
		
		if ( info->tracks[i]->video_type == 0x61766331) {
			int avc_profile = info->tracks[i]->private_data[1];
			int avc_nal_prefix_size = ( info->tracks[i]->private_data[4] & 0x03 ) + 1;
			int avc_sps_size = info->tracks[i]->private_data[6];
			avc_sps_size = (avc_sps_size << 8) | info->tracks[i]->private_data[7];
			int avc_pps_size = info->tracks[i]->private_data[9+avc_sps_size];
			avc_pps_size = (avc_pps_size << 8) | info->tracks[i]->private_data[10+avc_sps_size];
			
			fprintf(fp, "\tavc_profile=0x%02X\n", avc_profile);
			fprintf(fp, "\tavc_nal_prefix_size=%d\n", avc_nal_prefix_size);
			fprintf(fp, "\tavc_sps[%d] : { ", avc_sps_size);
			for(j = 0; j < avc_sps_size; j++)
				fprintf(fp, "%02X ", info->tracks[i]->private_data[j+8]);
			fprintf(fp, "}\n");
			fprintf(fp, "\tavc_pps[%d] : { ", avc_pps_size);
			for(j = 0; j < avc_pps_size; j++)
				fprintf(fp, "%02X ", info->tracks[i]->private_data[j+8+avc_sps_size+3]);
			fprintf(fp, "}\n");
			
		}
		
		fprintf(fp, "\n");
	}
	fclose(fp);	

}
