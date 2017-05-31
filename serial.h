//File: serial.h
//Description: Serial communication class for Windows and Linux
//WebSite: http://cool-emerald.blogspot.com
//MIT License (https://opensource.org/licenses/MIT)
//Copyright (c) 2017 Yan Naing Aye

// References
// https://en.wikibooks.org/wiki/Serial_Programming/termios
// http://www.silabs.com/documents/public/application-notes/an197.pdf
// https://msdn.microsoft.com/en-us/library/ff802693.aspx
// http://www.cplusplus.com/forum/unices/10491/

#ifndef SERIAL_H
#define SERIAL_H

//---------------------------------------------------------
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__)

#include <windows.h>
#include <stdio.h>
#define READ_TIMEOUT 10      // milliseconds

#define B4800 4800
#define B9600 9600
#define B19200 19200
#define B38400 38400
#define B57600 57600
#define B115200 115200

inline void delay(unsigned long ms)
{
	Sleep(ms);
}

//-----------------------------------------------------------------------------
class Serial {
	HANDLE hComm; //handle
	OVERLAPPED osReader;
	OVERLAPPED osWrite;
	long portnum;
	long baud;
	BOOL fWaitingOnRead;
	char rxchar;
	COMMTIMEOUTS timeouts_ori;
public:
	Serial();
	Serial(long PortNo,long BaudRate);
	~Serial();
	long Open(void);//return 0 if success
	void Close();
	char ReadChar(bool& success);//return read char if success
	bool WriteChar(char ch);////return success flag
	bool Write(char *data);//write null terminated string and return success flag
	bool SetRTS(bool value);//return success flag
	bool SetDTR(bool value);//return success flag
	bool GetCTS(bool& success);
	bool GetDSR(bool& success);
	bool GetRI(bool& success);
	bool GetCD(bool& success);
	bool IsOpened();
	void SetPortNumber(long PortNo);
	long GetPortNumber();
	void SetBaudRate(long baudrate);
	long GetBaudRate();
};
//-----------------------------------------------------------------------------
Serial::Serial()
{
	hComm = INVALID_HANDLE_VALUE;
	portnum = 3;
	baud = B9600;
}

Serial::Serial(long PortNo,long BaudRate)
{
	hComm = INVALID_HANDLE_VALUE;
	portnum = PortNo;
	baud = BaudRate;
}

Serial::~Serial()
{
	Close();
}

void Serial::SetPortNumber(long PortNo) {
	if(PortNo>0) portnum = PortNo;
}

long Serial::GetPortNumber() {
	return portnum;
}

void Serial::SetBaudRate(long baudrate) {
	if (baudrate < B9600) baud = B4800;
	else if (baudrate < B19200) baud = B9600;
	else if (baudrate < B38400) baud = B19200;
	else if (baudrate < B57600) baud = B38400;
	else if (baudrate < B115200) baud = B57600;
	else baud = B115200;
}

long Serial::GetBaudRate() {
	return baud;
}

long Serial::Open()
{
	
	if (!IsOpened())
	{
		#ifdef UNICODE
			wchar_t wtext[32];
			swprintf(wtext, L"\\\\.\\COM%d", portnum);
		#else
			char wtext[32];
			sprintf(wtext, "\\\\.\\COM%d", portnum);
		#endif
		hComm = CreateFile(wtext,
			GENERIC_READ | GENERIC_WRITE,
			0,
			0,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			0);
		if (hComm == INVALID_HANDLE_VALUE) {return 1;}

		if (PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0) {return 2;}//purge

		//get initial state
		DCB dcbOri;
		bool fSuccess;
		fSuccess = GetCommState(hComm, &dcbOri);
		if (!fSuccess) {return 3;}

		DCB dcb1 = dcbOri;
		dcb1.BaudRate = baud;
		dcb1.Parity = NOPARITY;
		dcb1.ByteSize = 8;
		dcb1.StopBits = ONESTOPBIT;
		dcb1.fOutxCtsFlow = false;
		dcb1.fOutxDsrFlow = false;
		dcb1.fOutX = false;
		dcb1.fDtrControl = DTR_CONTROL_DISABLE;
		dcb1.fRtsControl = RTS_CONTROL_DISABLE;
		fSuccess = SetCommState(hComm, &dcb1);
		delay(60);
		if (!fSuccess) {return 4;}

		fSuccess = GetCommState(hComm, &dcb1);
		if (!fSuccess) {return 5;}

		osReader = { 0 };// Create the overlapped event. Must be closed before exiting to avoid a handle leak.
		osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (osReader.hEvent == NULL) {return 6;}// Error creating overlapped event; abort.
		fWaitingOnRead = FALSE;

		osWrite = { 0 };
		osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (osWrite.hEvent == NULL) {return 7;}

		if (!GetCommTimeouts(hComm, &timeouts_ori)) { return 8; } // Error getting time-outs.
		COMMTIMEOUTS timeouts;
		timeouts.ReadIntervalTimeout = 20;
		timeouts.ReadTotalTimeoutMultiplier = 15;
		timeouts.ReadTotalTimeoutConstant = 100;
		timeouts.WriteTotalTimeoutMultiplier = 15;
		timeouts.WriteTotalTimeoutConstant = 100;
		if (!SetCommTimeouts(hComm, &timeouts)) { return 9;} // Error setting time-outs.
	}
	return 0;
}

void Serial::Close()
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

bool Serial::IsOpened()
{
	if(hComm == INVALID_HANDLE_VALUE) return false;
	else return true;
}

bool Serial::Write(char *data)
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
		if (GetLastError() != ERROR_IO_PENDING) {fRes = FALSE;}// WriteFile failed, but it isn't delayed. Report error and abort.
		else {// Write is pending.
			if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, TRUE)) fRes = FALSE;
			else fRes = TRUE;// Write operation completed successfully.
		}
	}
	else fRes = TRUE;// WriteFile completed immediately.
	return fRes;
}

bool Serial::WriteChar(char ch)
{
	char s[2];
	s[0]=ch;
	s[1]=0;//null terminated
	return Write(s);
}

char Serial::ReadChar(bool& success)
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
				fWaitingOnRead = FALSE;// Reset flag so that another opertion can be issued.
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

bool Serial::SetRTS(bool value)
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

bool Serial::SetDTR(bool value)
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

bool Serial::GetCTS(bool& success)
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

bool Serial::GetDSR(bool& success)
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

bool Serial::GetRI(bool& success)
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

bool Serial::GetCD(bool& success)
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

#else  // presume POSIX

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>//the terminal I/O interfaces
#include <string.h>  // needed for memset
#include <sys/ioctl.h> //ioctl() call defenitions

using namespace std;
inline void delay(unsigned long ms)
{
	usleep(ms * 1000);
}

class Serial {
	long fd;//serial_fd
	char rxchar;
	long portnum;
	long baud;
public:
	Serial();
	Serial(long PortNo,long BaudRate);
	~Serial();
	long Open(void);//return 0 if success
	void Close();
	char ReadChar(bool& success);//return read char if success
	bool WriteChar(char ch);////return success flag
	bool Write(char *data);//write null terminated string and return success flag
	bool SetRTS(bool value);//return success flag
	bool SetDTR(bool value);//return success flag
	bool GetCTS(bool& success);
	bool GetDSR(bool& success);
	bool GetRI(bool& success);
	bool GetCD(bool& success);
	bool IsOpened();
	void SetPortNumber(long PortNo);// 0 for ttyS0 , add 100 for USB devices e.g. 100 for ttyUSB0
	long GetPortNumber();
	void SetBaudRate(long baudrate);
	long GetBaudRate();
};
//-----------------------------------------------------------------------------
Serial::Serial()
{
	fd=-1;
	portnum = 2;
	baud = B9600;
}

Serial::Serial(long PortNo,long BaudRate)
{
	fd=-1;
	portnum = PortNo;
	baud = BaudRate;
}

Serial::~Serial()
{
	Close();
}

//-----------------------------------------------------------------------------
long Serial::Open(void) {

	struct termios settings;
	memset(&settings, 0, sizeof(settings));
	settings.c_iflag = 0;
	settings.c_oflag = 0;
	settings.c_cflag = CS8 | CREAD | CLOCAL;           // 8n1, see termios.h for more information
	settings.c_lflag = 0;
	settings.c_cc[VMIN] = 1;
	settings.c_cc[VTIME] = 0;

	long i=11;//index to port number part
	char pstr[]="/dev/ttyUSB99";
	if(portnum<100){
		pstr[8]='S';//to make it ttyS instead of ttyUSB
		i=9;
	}
	long pn=portnum%100;//limits portnum to 99

	char digit=char(pn/10);
	if(digit>0) {
		pstr[i++]=digit+0x30;
	}
	digit=char(pn%10);
	pstr[i++]=digit+0x30;
	pstr[i++]=0;//null termination
	fd = open(pstr, O_RDWR | O_NONBLOCK);
	if (fd == -1) {
		return 1;
	}
	cfsetospeed(&settings, baud);            // 115200 baud
	cfsetispeed(&settings, baud);            // 115200 baud

	tcsetattr(fd, TCSANOW, &settings);

	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	return 0;
}
//-----------------------------------------------------------------------------
void Serial::Close() {
	if(IsOpened()) close(fd);
	fd=-1;
}

bool Serial::IsOpened()
{
	if(fd== (-1)) return false;
	else return true;
}

void Serial::SetPortNumber(long PortNo) {
	if(PortNo>0) portnum = PortNo;
}

long Serial::GetPortNumber() {
	return portnum;
}

void Serial::SetBaudRate(long baudrate) {
	if (baudrate < 9600) baud = B4800;
	else if (baudrate < 19200) baud = B9600;
	else if (baudrate < 38400) baud = B19200;
	else if (baudrate < 57600) baud = B38400;
	else if (baudrate < 115200) baud = B57600;
	else baud = B115200;
}

long Serial::GetBaudRate() {
	long baudrate=9600;
	if (baud < B9600) baudrate =4800;
	else if (baud < B19200) baudrate =9600;
	else if (baud < B38400) baudrate = 19200;
	else if (baud < B57600) baudrate = 38400;
	else if (baud < B115200) baudrate = 57600;
	else baudrate = 115200;
	return baudrate;
}
char Serial::ReadChar(bool& success)
{
	success=false;
	if (!IsOpened()) {return 0;	}
	success=read(fd, &rxchar, 1)==1;
	return rxchar;
}

bool Serial::Write(char *data)
{
	if (!IsOpened()) {return false;	}
	long n = strlen(data);
	if (n < 0) n = 0;
	else if(n > 1024) n = 1024;
	return (write(fd, data, n)==n);
}

bool Serial::WriteChar(char ch)
{
	char s[2];
	s[0]=ch;
	s[1]=0;//null terminated
	return Write(s);
}
//------------------------------------------------------------------------------
bool Serial::SetRTS(bool value) {
	long RTS_flag = TIOCM_RTS;
	bool success=true;
	if (value) {
		if (ioctl(fd, TIOCMBIS, &RTS_flag) == -1) success=false;//Set RTS pin
	}
	else {
		if (ioctl(fd, TIOCMBIC, &RTS_flag) == -1) success=false;//Clear RTS pin
	}
	return success;
}
//------------------------------------------------------------------------------
bool Serial::SetDTR(bool value) {
	long DTR_flag = TIOCM_DTR;
	bool success=true;
	if (value) {
		if (ioctl(fd, TIOCMBIS, &DTR_flag) == -1) success=false;//Set RTS pin
	}
	else {
		if (ioctl(fd, TIOCMBIC, &DTR_flag) == -1) success=false;//Clear RTS pin
	}
	return success;
}
//------------------------------------------------------------------------------
bool Serial::GetCTS(bool& success) {
	success=true;
	long status;
	if(ioctl(fd, TIOCMGET, &status)== -1) success=false;
	return ((status & TIOCM_CTS) != 0);
}
//------------------------------------------------------------------------------
bool Serial::GetDSR(bool& success) {
	success=true;
	long status;
	if(ioctl(fd, TIOCMGET, &status)== -1) success=false;
	return ((status & TIOCM_DSR) != 0);
}
//------------------------------------------------------------------------------
bool Serial::GetRI(bool& success) {
	success=true;
	long status;
	if(ioctl(fd, TIOCMGET, &status)== -1) success=false;
	return ((status & TIOCM_RI) != 0);
}
//------------------------------------------------------------------------------
bool Serial::GetCD(bool& success) {
	success=true;
	long status;
	if(ioctl(fd, TIOCMGET, &status)== -1) success=false;
	return ((status & TIOCM_CD) != 0);
}
//------------------------------------------------------------------------------

#endif

//------------------------------------------------------------------------------
#endif // SERIAL_H
