/*
 *	Copyright (C) 2006 cooleyes
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pspkernel.h>
#include "fat.h"
#include "directory.h"
#include "libminiconv.h"

static const char * get_file_ext(const char * filename){
	int len = strlen(filename);
	const char * p = filename + len;
	while(p > filename && *p != '.' && *p != '/') p --;
	if(*p == '.')
		return p + 1;
	else
		return NULL;
}

static int compare_directory_item(directory_item_struct item1, directory_item_struct item2) {
	if ( (item1.filetype == FS_DIRECTORY) && (item2.filetype != FS_DIRECTORY) )
		return -1;
	if ( (item1.filetype != FS_DIRECTORY) && (item2.filetype == FS_DIRECTORY) )
		return 1;
	if ( stricmp(item1.longname, "..") == 0 )
		return -1;
	if ( stricmp(item2.longname, "..") == 0 )
		return 1;
	char* s1 = item1.longname;
	char* s2 = item2.longname;
	while (*s1 && *s2) {
		char c1 = *s1++;
		char c2 = *s2++;

		if ((c1 >= 'a') && (c1 <= 'z'))
			c1 -= 'a' - 'A';
		if ((c2 >= 'a') && (c2 <= 'z'))
			c2 -= 'a' - 'A';

		if (c1 > c2)
			return 1;
		if (c1 < c2)
			return -1;
	}
	if (*s1)
		return 1;
	if (*s2)
		return -1;
	return 0;
}

file_type_enum directory_get_filetype(const char* filename, file_type_ext_struct* file_type_ext_table) {

	const char * ext = get_file_ext(filename);
	if(ext == NULL)
		return FS_UNKNOWN_FILE;
	file_type_ext_struct *exts = file_type_ext_table;
	while(exts->ext != NULL){
		if(stricmp(ext, exts->ext) == 0)
			return exts->filetype;
		exts++;
	}
	return FS_UNKNOWN_FILE;
}

int open_ms_directory(const char* dir, char* sdir, int show_hidden, int show_unknown, file_type_ext_struct* file_type_ext_table, directory_item_struct** list) {
	int item_count;
	p_fat_info info;
	if(*list != NULL){
		free((void *)(*list));
		*list = NULL;
	}
	u32 count = fat_readdir(dir, sdir, &info);
	if(count == INVALID)
		return 0;
	u32 i, cur_count = 0;
	for(i = 0; i < count; i ++) {
		if(!show_hidden && (info[i].attr & FAT_FILEATTR_HIDDEN) > 0)
			continue;
		if(cur_count % 256 == 0){
			if(cur_count == 0)
				*list = (directory_item_struct*)malloc(sizeof(directory_item_struct) * 256);
			else
				*list = (directory_item_struct*)realloc(*list, sizeof(directory_item_struct) * (cur_count + 256));
			if(*list == NULL){
				free((void *)info);
				return 0;
			}
		}
		if(info[i].attr & FAT_FILEATTR_DIRECTORY){
			(*list)[cur_count].filetype = FS_DIRECTORY;
			strcpy((*list)[cur_count].shortname, info[i].filename);
			strcpy((*list)[cur_count].longname, info[i].longname);
			(*list)[cur_count].compname = (*list)[cur_count].longname;
		}
		else{
			if(info[i].filesize == 0)
				continue;
			file_type_enum ft = directory_get_filetype(info[i].longname, file_type_ext_table);
			if(!show_unknown && ft == FS_UNKNOWN_FILE)
				continue;
			(*list)[cur_count].filetype = ft;
			strcpy((*list)[cur_count].shortname, info[i].filename);
			strcpy((*list)[cur_count].longname, info[i].longname);
			(*list)[cur_count].compname = (*list)[cur_count].longname;
			(*list)[cur_count].filesize = info[i].filesize;
			(*list)[cur_count].cdate = info[i].cdate;
			(*list)[cur_count].ctime = info[i].ctime;
			(*list)[cur_count].mdate = info[i].mdate;
			(*list)[cur_count].mtime = info[i].mtime;
		}
		cur_count ++;
	}
	free((void *)info);
	if ( strcmp(sdir, "ms0:/") == 0 ) {
		if(cur_count % 256 == 0)
		{
			if(cur_count == 0)
				*list = (directory_item_struct*)malloc(sizeof(directory_item_struct) * 256);
			else
				*list = (directory_item_struct*)realloc(*list, sizeof(directory_item_struct) * (cur_count + 256));
			if(*list == NULL)
			{
				return 0;
			}
		}
		for( i = cur_count ; i > 0 ; i--) {
			(*list)[i].filetype = (*list)[i-1].filetype;
			strcpy((*list)[i].shortname, (*list)[i-1].shortname);
			strcpy((*list)[i].longname, (*list)[i-1].longname);
			(*list)[i].compname = (*list)[i].longname;
			(*list)[i].filesize = (*list)[i-1].filesize;
			(*list)[i].cdate = (*list)[i-1].cdate;
			(*list)[i].ctime = (*list)[i-1].ctime;
			(*list)[i].mdate = (*list)[i-1].mdate;
			(*list)[i].mtime = (*list)[i-1].mtime;
		}
		(*list)[0].filetype = FS_DIRECTORY;
		strcpy((*list)[0].shortname, "..");
		strcpy((*list)[0].longname, "..");
		(*list)[0].compname = (*list)[0].longname;
		cur_count ++ ;
	}
	item_count = cur_count;
	return item_count;
}

int open_usb_net_directory(const char* dir, char* sdir, int show_hidden, int show_unknown, file_type_ext_struct* file_type_ext_table, directory_item_struct** list) {
	int item_count;
	if(*list != NULL)
	{
		free((void *)(*list));
		*list = NULL;
	}
	strcpy(sdir, dir);
	int fd = sceIoDopen(dir);
	if ( fd < 0 )
		return 0;
	SceIoDirent temp_dir;
	memset(&temp_dir, 0, sizeof(SceIoDirent));
	u32 cur_count = 0;
	while ( sceIoDread(fd, &temp_dir) > 0 ) {
		if(cur_count % 256 == 0)
		{
			if(cur_count == 0)
				*list = (directory_item_struct*)malloc(sizeof(directory_item_struct) * 256);
			else
				*list = (directory_item_struct*)realloc(*list, sizeof(directory_item_struct) * (cur_count + 256));
			if(*list == NULL)
			{
				sceIoDclose(fd);
				return 0;
			}
		}
		if ( temp_dir.d_stat.st_attr & FIO_SO_IFDIR ) {
			if ( strcmp( temp_dir.d_name , "." ) == 0 )
				continue;
			(*list)[cur_count].filetype = FS_DIRECTORY;
			strcpy((*list)[cur_count].shortname, temp_dir.d_name);
			strcpy((*list)[cur_count].longname, temp_dir.d_name);

			if ( miniConvHaveFileSystemConv() ){
				char* temp_str = miniConvFileSystemConv(temp_dir.d_name);
				if( temp_str != NULL ) {
					strcpy((*list)[cur_count].longname, temp_str);
				}
			}

			(*list)[cur_count].compname = (*list)[cur_count].shortname;
		}
		else {
			file_type_enum ft = directory_get_filetype(temp_dir.d_name, file_type_ext_table);
			if(!show_unknown && ft == FS_UNKNOWN_FILE)
				continue;
			(*list)[cur_count].filetype = ft;
			strcpy((*list)[cur_count].shortname, temp_dir.d_name);
			strcpy((*list)[cur_count].longname, temp_dir.d_name);

			if ( miniConvHaveFileSystemConv() ){
				char* temp_str = miniConvFileSystemConv(temp_dir.d_name);
				if( temp_str != NULL ) {
					strcpy((*list)[cur_count].longname, temp_str);
				}
			}

			(*list)[cur_count].compname = (*list)[cur_count].shortname;
			(*list)[cur_count].filesize = temp_dir.d_stat.st_size;
		}

		cur_count ++;
	}
	item_count = cur_count;
	sceIoDclose(fd);
	return item_count;
}

int open_directory(const char* dir, char* sdir, int show_hidden, int show_unknown, file_type_ext_struct* file_type_ext_table, directory_item_struct** list) {
	int item_count;
	if ( strncmp(dir,"ms0:", 4) == 0 )
		item_count = open_ms_directory(dir, sdir, show_hidden, show_unknown, file_type_ext_table, list);
	else if ( strncmp(dir,"ef0:", 4) == 0 )
		item_count = open_usb_net_directory(dir, sdir, show_unknown, show_unknown, file_type_ext_table, list);
	else if ( strncmp(dir,"usbhost0:", 9) == 0 || strncmp(dir,"nethost0:", 9) == 0)
		item_count = open_usb_net_directory(dir, sdir, show_unknown, show_unknown, file_type_ext_table, list);
	else {
		if(*list != NULL) {
			free((void *)(*list));
			*list = NULL;
		}
		item_count = 4;
		*list = (directory_item_struct*)malloc(sizeof(directory_item_struct) * 4);
		if(*list == NULL) {
			item_count = 0;
			return item_count;
		}

		memset(*list, 0, sizeof(directory_item_struct) * 4 );

		(*list)[0].filetype = FS_DIRECTORY;
		strcpy((*list)[0].shortname, "ms0:");
		strcpy((*list)[0].longname, "ms0:");
		(*list)[0].compname = (*list)[0].longname;

		(*list)[1].filetype = FS_DIRECTORY;
		strcpy((*list)[1].shortname, "usbhost0:");
		strcpy((*list)[1].longname, "usbhost0:");
		(*list)[1].compname = (*list)[1].longname;

		(*list)[2].filetype = FS_DIRECTORY;
		strcpy((*list)[2].shortname, "nethost0:");
		strcpy((*list)[2].longname, "nethost0:");
		(*list)[2].compname = (*list)[2].longname;

		(*list)[3].filetype = FS_DIRECTORY;
		strcpy((*list)[3].shortname, "ef0:");
		strcpy((*list)[3].longname, "ef0:");
		(*list)[3].compname = (*list)[3].longname;
	}

	int swap = 1;
	while ( (swap == 1) && (item_count>0)){
		swap = 0;
		int i;
		for (i = 0; i < item_count - 1; i++) {
			if (compare_directory_item((*list)[i], (*list)[i+1]) > 0){
				swap = 1;
				directory_item_struct temp_item;
				temp_item.filesize = (*list)[i].filesize;
				temp_item.cdate = (*list)[i].cdate;
				temp_item.ctime = (*list)[i].ctime;
				temp_item.mdate = (*list)[i].mdate;
				temp_item.mtime = (*list)[i].mtime;
				temp_item.filetype = (*list)[i].filetype;
				strcpy(temp_item.shortname, (*list)[i].shortname);
				strcpy(temp_item.longname, (*list)[i].longname);

				(*list)[i].filesize = (*list)[i+1].filesize;
				(*list)[i].cdate = (*list)[i+1].cdate;
				(*list)[i].ctime = (*list)[i+1].ctime;
				(*list)[i].mdate = (*list)[i+1].mdate;
				(*list)[i].mtime = (*list)[i+1].mtime;
				(*list)[i].filetype = (*list)[i+1].filetype;
				strcpy((*list)[i].shortname, (*list)[i+1].shortname);
				strcpy((*list)[i].longname, (*list)[i+1].longname);

				(*list)[i+1].filesize = temp_item.filesize;
				(*list)[i+1].cdate = temp_item.cdate;
				(*list)[i+1].ctime = temp_item.ctime;
				(*list)[i+1].mdate = temp_item.mdate;
				(*list)[i+1].mtime = temp_item.mtime;
				(*list)[i+1].filetype = temp_item.filetype;
				strcpy((*list)[i+1].shortname, temp_item.shortname);
				strcpy((*list)[i+1].longname, temp_item.longname);
			}
		}
	}
	return item_count;
}

int is_next_movie(const char* prev, const char* next) {
	int prev_len = strlen(prev);
	int next_len = strlen(next);

	if ( (prev_len != next_len) && (prev_len != next_len-1) )
		return 0;

	char* s1 = prev;
	char* s2 = next;
	while (*s1 && *s2) {
		char c1 = *s1;
		char c2 = *s2;

		if ((c1 >= 'a') && (c1 <= 'z'))
			c1 -= 'a' - 'A';
		if ((c2 >= 'a') && (c2 <= 'z'))
			c2 -= 'a' - 'A';
		if ( c1 != c2 )
			break;
		else {
			s1++;
			s2++;
		}
	}
	if ( *s1 == 0)
		return 0;
	char* s3 = (char*)(prev+prev_len-1);
	char* s4 = (char*)(next+next_len-1);
	while( (s3>s1) && (s4>s2) ) {
		char c3 = *s3;
		char c4 = *s4;

		if ((c3 >= 'a') && (c3 <= 'z'))
			c3 -= 'a' - 'A';
		if ((c4 >= 'a') && (c4 <= 'z'))
			c4 -= 'a' - 'A';
		if ( c3 != c4 )
			break;
		else {
			s3--;
			s4--;
		}
	}

	char value1[256], value2[256];
	memset(value1, 0, 256);
	memset(value2, 0, 256);
	strncpy(value1, s1, (s3-s1)+1);
	strncpy(value2, s2, (s4-s2)+1);

	if ( (strlen(value1)==1) && (strlen(value2)==1) ) {
		if ((value1[0] >= 'a') && (value1[0] <= 'z'))
			value1[0] -= 'a' - 'A';
		if ((value2[0] >= 'a') && (value2[0] <= 'z'))
			value2[0] -= 'a' - 'A';
		if ( value2[0] - value1[0] == 1)
			return 1;
		else
			return 0;
	}
	else {
		int idx1,idx2;
		idx1=idx2=-1;
		sscanf(value1, "%d", &idx1);
		sscanf(value2, "%d", &idx2);
		if ( idx2-idx1==1 )
			return 1;
		else
			return 0;
	}

	return 1;
}


