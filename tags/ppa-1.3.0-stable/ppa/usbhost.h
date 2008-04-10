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
 
#ifndef __PPA_USBHOST_H__
#define __PPA_USBHOST_H__

class UsbHost {
	static bool usbHostPrxLoaded;
	static bool usbHostState;
private:
	UsbHost() {};
public:
	static bool loadUsbHostPrx(const char* prxName);
	static bool startUsbHost();
	static bool stopUsbHost();
	static bool getUsbHostState() { return usbHostState; };	
};

#endif
