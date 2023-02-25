// File: ceSerial.cpp
// Description: ceSerial communication class implementation for Windows and Linux
// WebSite: http://cool-emerald.blogspot.sg/2017/05/serial-port-programming-in-c-with.html
// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2018 Yan Naing Aye

// References
// https://en.wikibooks.org/wiki/Serial_Programming/termios
// http://www.silabs.com/documents/public/application-notes/an197.pdf
// https://msdn.microsoft.com/en-us/library/ff802693.aspx
// http://www.cplusplus.com/forum/unices/10491/

#include "ceSerial.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
using namespace std;


#ifdef CE_WINDOWS
	#define READ_TIMEOUT 10      // milliseconds
#else
	#include <unistd.h>
	#include <fcntl.h>
	#include <termios.h>
	#include <sys/ioctl.h>
	#include <linux/serial.h>
#endif

namespace ce {

void ceSerial::Delay(unsigned long ms){
#ifdef CE_WINDOWS
	Sleep(ms);
#else
	usleep(ms*1000);
#endif
}

ceSerial::ceSerial() :
#ifdef CE_WINDOWS
	ceSerial("\\\\.\\COM1", 9600, 8, 'N', 1)
#else
	ceSerial("/dev/ttyS0", 9600, 8, 'N', 1)	
#endif
{

}

ceSerial::ceSerial(string Device, long BaudRate,long DataSize,char ParityType,float NStopBits):stdbaud(true)
{
#ifdef CE_WINDOWS
	hComm = INVALID_HANDLE_VALUE;
#else
	fd = -1;
#endif // defined
	SetBaudRate(BaudRate);
	SetDataSize(DataSize);
	SetParity(ParityType);
	SetStopBits(NStopBits);
	SetPortName(Device);
}

ceSerial::~ceSerial()
{
	Close();
}

void ceSerial::SetPortName(string Device) {
	port = Device;
}

string ceSerial::GetPort() {
	return port;
}

void ceSerial::SetDataSize(long nbits) {
	if ((nbits < 5) || (nbits > 8)) nbits = 8;
	dsize=nbits;
}

long ceSerial::GetDataSize() {
	return dsize;
}

void ceSerial::SetParity(char p) {
	if ((p != 'N') && (p != 'E') && (p != 'O')) {
#ifdef CE_WINDOWS
		if ((p != 'M') && (p != 'S')) p = 'N';
#else
		p = 'N';
#endif
	}
	parity = p;
}

char ceSerial::GetParity() {
	return parity;
}

void ceSerial::SetStopBits(float nbits) {
	if (nbits >= 2) stopbits = 2;
#ifdef CE_WINDOWS
	else if(nbits >= 1.5) stopbits = 1.5;
#endif
	else stopbits = 1;
}

float ceSerial::GetStopBits() {
	return stopbits;
}


#ifdef CE_WINDOWS

void ceSerial::SetBaudRate(long baudrate) {
	stdbaud = true;
	if (baudrate == 1100) baud = CBR_110;
	else if (baudrate == 300) baud = CBR_300;
	else if (baudrate == 600) baud = CBR_600;
	else if (baudrate == 1200) baud = CBR_1200;
	else if (baudrate == 2400) baud = CBR_2400;
	else if (baudrate == 4800) baud = CBR_4800;
	else if (baudrate == 9600) baud = CBR_9600;
	else if (baudrate == 14400) baud = CBR_14400;
	else if (baudrate == 19200) baud = CBR_19200;
	else if (baudrate == 38400) baud = CBR_38400;
	else if (baudrate == 57600) baud = CBR_57600;
	else if (baudrate == 115200) baud = CBR_115200;
	else if (baudrate == 128000) baud = CBR_128000;
	else if (baudrate == 256000) baud = CBR_256000;
	else {
		baud = baudrate;
		stdbaud = false;
	}
}

long ceSerial::GetBaudRate() {
	return baud;
}

long ceSerial::Open()
{
	if (IsOpened()) return 0;
#ifdef UNICODE
	wstring wtext(port.begin(),port.end());
#else
	string wtext = port;
#endif
    hComm = CreateFile(wtext.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        0);
    if (hComm == INVALID_HANDLE_VALUE) {return -1;}

    if (PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0) {return -1;}//purge

    //get initial state
    DCB dcbOri;
    bool fSuccess;
    fSuccess = GetCommState(hComm, &dcbOri);
    if (!fSuccess) {return -1;}

    DCB dcb1 = dcbOri;

    dcb1.BaudRate = baud;

	if (parity == 'E') dcb1.Parity = EVENPARITY;
	else if (parity == 'O') dcb1.Parity = ODDPARITY;
	else if (parity == 'M') dcb1.Parity = MARKPARITY;
	else if (parity == 'S') dcb1.Parity = SPACEPARITY;
    else dcb1.Parity = NOPARITY;

    dcb1.ByteSize = (BYTE)dsize;

	if(stopbits==2) dcb1.StopBits = TWOSTOPBITS;
	else if (stopbits == 1.5) dcb1.StopBits = ONE5STOPBITS;
    else dcb1.StopBits = ONESTOPBIT;

    dcb1.fOutxCtsFlow = false;
    dcb1.fOutxDsrFlow = false;
    dcb1.fOutX = false;
    dcb1.fDtrControl = DTR_CONTROL_DISABLE;
    dcb1.fRtsControl = RTS_CONTROL_DISABLE;
    fSuccess = SetCommState(hComm, &dcb1);
    this->Delay(60);
    if (!fSuccess) {return -1;}

    fSuccess = GetCommState(hComm, &dcb1);
    if (!fSuccess) {return -1;}

    osReader = { 0 };// Create the overlapped event.
    // Must be closed before exiting to avoid a handle leak.
    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (osReader.hEvent == NULL) {return -1;}// Error creating overlapped event; abort.
    fWaitingOnRead = FALSE;

    osWrite = { 0 };
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osWrite.hEvent == NULL) {return -1;}

    if (!GetCommTimeouts(hComm, &timeouts_ori)) { return -1; } // Error getting time-outs.
    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 20;
    timeouts.ReadTotalTimeoutMultiplier = 15;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.WriteTotalTimeoutMultiplier = 15;
    timeouts.WriteTotalTimeoutConstant = 100;
    if (!SetCommTimeouts(hComm, &timeouts)) { return -1;} // Error setting time-outs.
	return 0;
}

void ceSerial::Close()
{
	if (IsOpened())
	{
		SetCommTimeouts(hComm, &timeouts_ori);
		CloseHandle(osReader.hEvent);
		CloseHandle(osWrite.hEvent);
		CloseHandle(hComm);//close comm port
		hComm = INVALID_HANDLE_VALUE;
	}
}

bool ceSerial::IsOpened()
{
	if(hComm == INVALID_HANDLE_VALUE) return false;
	else return true;
}



bool ceSerial::Write(char *data)
{
	if (!IsOpened()) {
		return false;
	}
	BOOL fRes;
	DWORD dwWritten;
	long n = strlen(data);
	if (n < 0) n = 0;
	else if(n > 1024) n = 1024;

	// Issue write.
	if (!WriteFile(hComm, data, n, &dwWritten, &osWrite)) {
        // WriteFile failed, but it isn't delayed. Report error and abort.
		if (GetLastError() != ERROR_IO_PENDING) {fRes = FALSE;}
		else {// Write is pending.
			if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, TRUE)) fRes = FALSE;
			else fRes = TRUE;// Write operation completed successfully.
		}
	}
	else fRes = TRUE;// WriteFile completed immediately.
	return fRes;
}

bool ceSerial::Write(char *data,long n)
{
	if (!IsOpened()) {
		return false;
	}
	BOOL fRes;
	DWORD dwWritten;
	if (n < 0) n = 0;
	else if(n > 1024) n = 1024;

	// Issue write.
	if (!WriteFile(hComm, data, n, &dwWritten, &osWrite)) {
        // WriteFile failed, but it isn't delayed. Report error and abort.
		if (GetLastError() != ERROR_IO_PENDING) {fRes = FALSE;}
		else {// Write is pending.
			if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, TRUE)) fRes = FALSE;
			else fRes = TRUE;// Write operation completed successfully.
		}
	}
	else fRes = TRUE;// WriteFile completed immediately.
	return fRes;
}

bool ceSerial::WriteChar(char ch)
{
	char s[2];
	s[0]=ch;
	s[1]=0;//null terminated
	return Write(s);
}

char ceSerial::ReadChar(bool& success)
{
	success = false;
	if (!IsOpened()) {return 0;}

	DWORD dwRead;
	DWORD length=1;
	BYTE* data = (BYTE*)(&rxchar);
	//the creation of the overlapped read operation
	if (!fWaitingOnRead) {
		// Issue read operation.
		if (!ReadFile(hComm, data, length, &dwRead, &osReader)) {
			if (GetLastError() != ERROR_IO_PENDING) { /*Error*/}
			else { fWaitingOnRead = TRUE; /*Waiting*/}
		}
		else {if(dwRead==length) success = true;}//success
	}


	//detection of the completion of an overlapped read operation
	DWORD dwRes;
	if (fWaitingOnRead) {
		dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
		switch (dwRes)
		{
		// Read completed.
		case WAIT_OBJECT_0:
			if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE)) {/*Error*/ }
			else {
				if (dwRead == length) success = true;
				fWaitingOnRead = FALSE;
            // Reset flag so that another opertion can be issued.
			}// Read completed successfully.
			break;

		case WAIT_TIMEOUT:
			// Operation isn't complete yet.
			break;

		default:
			// Error in the WaitForSingleObject;
			break;
		}
	}
	return rxchar;
}

bool ceSerial::SetRTS(bool value)
{
	bool r = false;
	if (IsOpened()) {
		if (value) {
			if (EscapeCommFunction(hComm, SETRTS)) r = true;
		}
		else {
			if (EscapeCommFunction(hComm, CLRRTS)) r = true;
		}
	}
	return r;
}

bool ceSerial::SetDTR(bool value)
{
	bool r = false;
	if (IsOpened()) {
		if (value) {
			if (EscapeCommFunction(hComm, SETDTR)) r = true;
		}
		else {
			if (EscapeCommFunction(hComm, CLRDTR)) r = true;
		}
	}
	return r;
}

bool ceSerial::GetCTS(bool& success)
{
	success = false;
	bool r = false;
	if (IsOpened()) {
		DWORD dwModemStatus;
		if (GetCommModemStatus(hComm, &dwModemStatus)){
			r = MS_CTS_ON & dwModemStatus;
			success = true;
		}
	}
	return r;
}

bool ceSerial::GetDSR(bool& success)
{
	success = false;
	bool r = false;
	if (IsOpened()) {
		DWORD dwModemStatus;
		if (GetCommModemStatus(hComm, &dwModemStatus)) {
			r = MS_DSR_ON & dwModemStatus;
			success = true;
		}
	}
	return r;
}

bool ceSerial::GetRI(bool& success)
{
	success = false;
	bool r = false;
	if (IsOpened()) {
		DWORD dwModemStatus;
		if (GetCommModemStatus(hComm, &dwModemStatus)) {
			r = MS_RING_ON & dwModemStatus;
			success = true;
		}
	}
	return r;
}

bool ceSerial::GetCD(bool& success)
{
	success = false;
	bool r = false;
	if (IsOpened()) {
		DWORD dwModemStatus;
		if (GetCommModemStatus(hComm, &dwModemStatus)) {
			r = MS_RLSD_ON & dwModemStatus;
			success = true;
		}
	}
	return r;
}

#else  //for POSIX

long ceSerial::Open(void) {
	struct serial_struct serinfo;
	struct termios settings;
	memset(&settings, 0, sizeof(settings));
	settings.c_iflag = 0;
	settings.c_oflag = 0;

	settings.c_cflag = CREAD | CLOCAL;//see termios.h for more information
	if(dsize==5)  settings.c_cflag |= CS5;//no change
	else if (dsize == 6)  settings.c_cflag |= CS6;
	else if (dsize == 7)  settings.c_cflag |= CS7;
	else settings.c_cflag |= CS8;

	if(stopbits==2) settings.c_cflag |= CSTOPB;

	if(parity!='N') settings.c_cflag |= PARENB;

	if (parity == 'O') settings.c_cflag |= PARODD;

	settings.c_lflag = 0;
	settings.c_cc[VMIN] = 1;
	settings.c_cc[VTIME] = 0;

	fd = open(port.c_str(), O_RDWR | O_NONBLOCK);
	if (fd == -1) {
		return -1;
	}

	if (!stdbaud) {
		// serial driver to interpret the value B38400 differently		
		serinfo.reserved_char[0] = 0;
		if (ioctl(fd, TIOCGSERIAL, &serinfo) < 0) {	return -1;}
		serinfo.flags &= ~ASYNC_SPD_MASK;
		serinfo.flags |= ASYNC_SPD_CUST;
		serinfo.custom_divisor = (serinfo.baud_base + (baud / 2)) / baud;
		if (serinfo.custom_divisor < 1) serinfo.custom_divisor = 1;
		if (ioctl(fd, TIOCSSERIAL, &serinfo) < 0) { return -1; }
		if (ioctl(fd, TIOCGSERIAL, &serinfo) < 0) { return -1; }
		if (serinfo.custom_divisor * baud != serinfo.baud_base) {
			/*
			warnx("actual baudrate is %d / %d = %f\n",
				serinfo.baud_base, serinfo.custom_divisor,
				(float)serinfo.baud_base / serinfo.custom_divisor);
			*/
		}
		cfsetospeed(&settings, B38400);
		cfsetispeed(&settings, B38400);
	}
	else {
		cfsetospeed(&settings, baud);
		cfsetispeed(&settings, baud);
	}	
	tcsetattr(fd, TCSANOW, &settings);
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	if (!stdbaud) {
		// driver to interpret B38400 as 38400 baud again
		ioctl(fd, TIOCGSERIAL, &serinfo);
		serinfo.flags &= ~ASYNC_SPD_MASK;
		ioctl(fd, TIOCSSERIAL, &serinfo);
	}
	return 0;
}

void ceSerial::Close() {
	if(IsOpened()) close(fd);
	fd=-1;
}

bool ceSerial::IsOpened()
{
	if(fd== (-1)) return false;
	else return true;
}

void ceSerial::SetBaudRate(long baudrate) {
	stdbaud = true;
	if (baudrate == 0) baud = B0;
	else if (baudrate == 50) baud = B50;
	else if (baudrate == 75) baud = B75;
	else if (baudrate == 110) baud = B110;
	else if (baudrate == 134) baud = B134;
	else if (baudrate == 150) baud = B150;
	else if (baudrate == 200) baud = B200;
	else if (baudrate == 300) baud = B300;
	else if (baudrate == 600) baud = B600;
	else if (baudrate == 1200) baud = B1200;
	else if (baudrate == 2400) baud = B2400;
	else if (baudrate == 4800) baud = B4800;
	else if (baudrate == 9600) baud = B9600;
	else if (baudrate == 19200) baud = B19200;
	else if (baudrate == 38400) baud = B38400;
	else if (baudrate == 57600) baud = B57600;
	else if (baudrate == 115200) baud = B115200;
	else if (baudrate == 230400) baud = B230400;
	else {
		baud = baudrate;
		stdbaud = false;
	}
}

long ceSerial::GetBaudRate() {
	return baud;
}
char ceSerial::ReadChar(bool& success)
{
	success=false;
	if (!IsOpened()) {return 0;	}
	success=read(fd, &rxchar, 1)==1;
	return rxchar;
}

bool ceSerial::Write(char *data)
{
	if (!IsOpened()) {return false;	}
	long n = strlen(data);
	if (n < 0) n = 0;
	else if(n > 1024) n = 1024;
	return (write(fd, data, n)==n);
}

bool ceSerial::Write(char *data,long n)
{
	if (!IsOpened()) {return false;	}
	if (n < 0) n = 0;
	else if(n > 1024) n = 1024;
	return (write(fd, data, n)==n);
}

bool ceSerial::WriteChar(char ch)
{
	char s[2];
	s[0]=ch;
	s[1]=0;//null terminated
	return Write(s);
}

bool ceSerial::SetRTS(bool value) {
	long RTS_flag = TIOCM_RTS;
	bool success=true;
	if (value) {//Set RTS pin
		if (ioctl(fd, TIOCMBIS, &RTS_flag) == -1) success=false;
	}
	else {//Clear RTS pin
		if (ioctl(fd, TIOCMBIC, &RTS_flag) == -1) success=false;
	}
	return success;
}

bool ceSerial::SetDTR(bool value) {
	long DTR_flag = TIOCM_DTR;
	bool success=true;
	if (value) {//Set DTR pin
		if (ioctl(fd, TIOCMBIS, &DTR_flag) == -1) success=false;
	}
	else {//Clear DTR pin
		if (ioctl(fd, TIOCMBIC, &DTR_flag) == -1) success=false;
	}
	return success;
}

bool ceSerial::GetCTS(bool& success) {
	success=true;
	long status;
	if(ioctl(fd, TIOCMGET, &status)== -1) success=false;
	return ((status & TIOCM_CTS) != 0);
}

bool ceSerial::GetDSR(bool& success) {
	success=true;
	long status;
	if(ioctl(fd, TIOCMGET, &status)== -1) success=false;
	return ((status & TIOCM_DSR) != 0);
}

bool ceSerial::GetRI(bool& success) {
	success=true;
	long status;
	if(ioctl(fd, TIOCMGET, &status)== -1) success=false;
	return ((status & TIOCM_RI) != 0);
}

bool ceSerial::GetCD(bool& success) {
	success=true;
	long status;
	if(ioctl(fd, TIOCMGET, &status)== -1) success=false;
	return ((status & TIOCM_CD) != 0);
}
#endif

} // namespace ce 
