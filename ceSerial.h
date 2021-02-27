// File: ceSerial.h
// Description: ceSerial communication class for Windows and Linux
// WebSite: http://cool-emerald.blogspot.sg/2017/05/serial-port-programming-in-c-with.html
// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2018 Yan Naing Aye

// References
// https://en.wikibooks.org/wiki/Serial_Programming/termios
// http://www.silabs.com/documents/public/application-notes/an197.pdf
// https://msdn.microsoft.com/en-us/library/ff802693.aspx
// http://www.cplusplus.com/forum/unices/10491/

#ifndef CESERIAL_H
#define CESERIAL_H
#include <string>

#if defined(_WIN64) || defined(__WIN32__) || defined(_WIN32) || defined(WIN32) || defined(__WINDOWS__) || defined(__TOS_WIN__) || defined(__CYGWIN__)
    #define ceWINDOWS 
#elif defined(unix) || defined(__unix) || defined(__unix__)
    #define ceLINUX
#endif

#ifdef ceWINDOWS
	#include <windows.h>
#endif

namespace ce {

class ceSerial {
	char rxchar;
	std::string port;
	long baud;
	long dsize;
	char parity;
	float stopbits;
#ifdef ceWINDOWS
    HANDLE hComm; //handle
	OVERLAPPED osReader;
	OVERLAPPED osWrite;
	BOOL fWaitingOnRead;
	COMMTIMEOUTS timeouts_ori;
#else
	long fd;//serial_fd
#endif
public:
	static void Delay(unsigned long ms);
	ceSerial();
	ceSerial(std::string Device, long BaudRate, long DataSize, char ParityType, float NStopBits);
	~ceSerial();
	long Open(void);//return 0 if success
	void Close();
	char ReadChar(bool& success);//return read char if success
	bool WriteChar(char ch);////return success flag
	bool Write(char *data);//write null terminated string and return success flag
	bool Write(char *data,long n);
	bool SetRTS(bool value);//return success flag
	bool SetDTR(bool value);//return success flag
	bool GetCTS(bool& success);
	bool GetDSR(bool& success);
	bool GetRI(bool& success);
	bool GetCD(bool& success);
	bool IsOpened();
	void SetPort(std::string Port);
	std::string GetPort();
	void SetBaudRate(long baudrate);
	long GetBaudRate();
	void SetDataSize(long nbits);
	long GetDataSize();
	void SetParity(char p);
	char GetParity();
	void SetStopBits(float nbits);
	float GetStopBits();
};

} // namespace ce 

#endif // CESERIAL_H
