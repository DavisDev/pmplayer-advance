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
#include <string.h>
#include <string>
#include "config.h"
using namespace std;

Config* Config::instance = NULL;

bool Config::loadConfig(const char* filename) {
	Config::freeConfig();
	Config::instance = new Config();
	if ( Config::instance->load(filename) )
		return true;
	else {
		delete Config::instance;
		Config::instance = NULL;
		return false;
	}
};

void Config::freeConfig() {
	if (Config::instance)
		delete Config::instance;
};

Config* Config::getInstance() {
	return Config::instance;
};

Config::Config() {};
Config::~Config() {};

bool Config::load(const char* filename) {
	xmldoc = TiXmlDocument(filename);
	return xmldoc.LoadFile();
};
	
void Config::save(const char* filename) {
	xmldoc.SaveFile(filename);
};

/********************************************************************************
 *                  get functions                                               *
 ********************************************************************************/
bool Config::getBooleanValue(const char* name, bool default_value) {
	bool ret_value = default_value;
	
	string name_str = string(name);
	TiXmlHandle element_handle(&xmldoc);
	string::size_type pos = name_str.find_first_of('/');
	
	while(pos != string::npos) {
		string element_name_str = name_str.substr(0, pos);
		name_str = name_str.substr(pos+1, name_str.length()- 1 - pos);  
		element_handle = element_handle.FirstChild(element_name_str.c_str());
		if (!element_handle.Element()) {
			return ret_value;
			break;
		}
		pos = name_str.find_first_of('/');
	}
	
	const char* value_str = element_handle.Element()->Attribute(name_str.c_str());
	if (value_str) {
		if ( stricmp(value_str, "TRUE") == 0)
			ret_value = true;
		else
			ret_value = false;
	}
	return ret_value;
};

int Config::getIntegerValue(const char* name, int default_value) {
	int ret_value = default_value;
	
	string name_str = string(name);
	TiXmlHandle element_handle(&xmldoc);
	string::size_type pos = name_str.find_first_of('/');
	
	while(pos != string::npos) {
		string element_name_str = name_str.substr(0, pos);
		name_str = name_str.substr(pos+1, name_str.length()- 1 - pos);  
		element_handle = element_handle.FirstChild(element_name_str.c_str());
		if (!element_handle.Element()) {
			return ret_value;
			break;
		}
		pos = name_str.find_first_of('/');
	}
	
	const char* value_str = element_handle.Element()->Attribute(name_str.c_str());
	if (value_str) {
		sscanf(value_str, "%d", &ret_value);
	}
	return ret_value;
};

float Config::getFloatValue(const char* name, float default_value) {
	float ret_value = default_value;
	
	string name_str = string(name);
	TiXmlHandle element_handle(&xmldoc);
	string::size_type pos = name_str.find_first_of('/');
	
	while(pos != string::npos) {
		string element_name_str = name_str.substr(0, pos);
		name_str = name_str.substr(pos+1, name_str.length()- 1 - pos);  
		element_handle = element_handle.FirstChild(element_name_str.c_str());
		if (!element_handle.Element()) {
			return ret_value;
			break;
		}
		pos = name_str.find_first_of('/');
	}
	
	const char* value_str = element_handle.Element()->Attribute(name_str.c_str());
	if (value_str) {
		sscanf(value_str, "%f", &ret_value);
	}
	return ret_value;
};

const char* Config::getStringValue(const char* name, const char* default_value) {
	const char* ret_value = default_value;
	
	string name_str = string(name);
	TiXmlHandle element_handle(&xmldoc);
	string::size_type pos = name_str.find_first_of('/');
	
	while(pos != string::npos) {
		string element_name_str = name_str.substr(0, pos);
		name_str = name_str.substr(pos+1, name_str.length()- 1 - pos);  
		element_handle = element_handle.FirstChild(element_name_str.c_str());
		if (!element_handle.Element()) {
			return ret_value;
			break;
		}
		pos = name_str.find_first_of('/');
	}
	
	const char* value_str = element_handle.Element()->Attribute(name_str.c_str());
	if (value_str) {
		ret_value = value_str;
	}
	return ret_value;
};

Color Config::getColorValue(const char* name, Color default_value) {
	Color ret_value = default_value;
	
	string name_str = string(name);
	TiXmlHandle element_handle(&xmldoc);
	string::size_type pos = name_str.find_first_of('/');
	
	while(pos != string::npos) {
		string element_name_str = name_str.substr(0, pos);
		name_str = name_str.substr(pos+1, name_str.length()- 1 - pos);  
		element_handle = element_handle.FirstChild(element_name_str.c_str());
		if (!element_handle.Element()) {
			return ret_value;
			break;
		}
		pos = name_str.find_first_of('/');
	}
	
	const char* value_str = element_handle.Element()->Attribute(name_str.c_str());
	if (value_str) {
		sscanf(value_str, "#%06x", &ret_value);
	}
	return ret_value;
};

/********************************************************************************
 *                  set functions                                               *
 ********************************************************************************/
bool Config::setBooleanValue(const char* name, bool default_value) {
	string name_str = string(name);
	TiXmlHandle element_handle(&xmldoc);
	string::size_type pos = name_str.find_first_of('/');
	
	while(pos != string::npos) {
		string element_name_str = name_str.substr(0, pos);
		name_str = name_str.substr(pos+1, name_str.length()- 1 - pos);  
		element_handle = element_handle.FirstChild(element_name_str.c_str());
		if (!element_handle.Element()) {
			return false;
			break;
		}
		pos = name_str.find_first_of('/');
	}
	char value_str[64];
	memset(value_str, 0, 64);
	if ( default_value )
		sprintf(value_str, "TRUE");
	else
		sprintf(value_str, "FALSE");
	element_handle.Element()->SetAttribute(name_str.c_str(), value_str);
	return true;
};

bool Config::setIntegerValue(const char* name, int default_value) {
	string name_str = string(name);
	TiXmlHandle element_handle(&xmldoc);
	string::size_type pos = name_str.find_first_of('/');
	
	while(pos != string::npos) {
		string element_name_str = name_str.substr(0, pos);
		name_str = name_str.substr(pos+1, name_str.length()- 1 - pos);  
		element_handle = element_handle.FirstChild(element_name_str.c_str());
		if (!element_handle.Element()) {
			return false;
			break;
		}
		pos = name_str.find_first_of('/');
	}
	element_handle.Element()->SetAttribute(name_str.c_str(), default_value);
	return true;
};

bool Config::setFloatValue(const char* name, float default_value) {
	string name_str = string(name);
	TiXmlHandle element_handle(&xmldoc);
	string::size_type pos = name_str.find_first_of('/');
	
	while(pos != string::npos) {
		string element_name_str = name_str.substr(0, pos);
		name_str = name_str.substr(pos+1, name_str.length()- 1 - pos);  
		element_handle = element_handle.FirstChild(element_name_str.c_str());
		if (!element_handle.Element()) {
			return false;
			break;
		}
		pos = name_str.find_first_of('/');
	}
	char value_str[64];
	memset(value_str, 0, 64);
	sprintf(value_str, "%f", default_value);
	element_handle.Element()->SetAttribute(name_str.c_str(), value_str);
	return true;
};

bool Config::setStringValue(const char* name, const char* default_value) {
	string name_str = string(name);
	TiXmlHandle element_handle(&xmldoc);
	string::size_type pos = name_str.find_first_of('/');
	
	while(pos != string::npos) {
		string element_name_str = name_str.substr(0, pos);
		name_str = name_str.substr(pos+1, name_str.length()- 1 - pos);  
		element_handle = element_handle.FirstChild(element_name_str.c_str());
		if (!element_handle.Element()) {
			return false;
			break;
		}
		pos = name_str.find_first_of('/');
	}
	element_handle.Element()->SetAttribute(name_str.c_str(), default_value);
	return true;
};

bool Config::setColorValue(const char* name, Color default_value) {
	string name_str = string(name);
	TiXmlHandle element_handle(&xmldoc);
	string::size_type pos = name_str.find_first_of('/');
	
	while(pos != string::npos) {
		string element_name_str = name_str.substr(0, pos);
		name_str = name_str.substr(pos+1, name_str.length()- 1 - pos);  
		element_handle = element_handle.FirstChild(element_name_str.c_str());
		if (!element_handle.Element()) {
			return false;
			break;
		}
		pos = name_str.find_first_of('/');
	}
	char value_str[64];
	memset(value_str, 0, 64);
	sprintf(value_str, "#%06X", default_value);
	element_handle.Element()->SetAttribute(name_str.c_str(), value_str); 
	return true;
};
