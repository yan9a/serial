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

#include "ceSerial.h"

#define READ_TIMEOUT 10      // milliseconds

void ceSerial::Delay(unsigned long ms) {
	Sleep(ms);
}

ceSerial::ceSerial() :
	ceSerial("\\\\.\\COM1", 9600, 8, 'N', 1)
{
}

ceSerial::ceSerial(std::string Device, long BaudRate,long DataSize,char ParityType,float NStopBits):stdbaud(true) {
	hComm = INVALID_HANDLE_VALUE;
	SetBaudRate(BaudRate);
	SetDataSize(DataSize);
	SetParity(ParityType);
	SetStopBits(NStopBits);
	SetPortName(Device);
	SetFlowControl(FlowControl_None);
}

ceSerial::~ceSerial() {
	Close();
}

void ceSerial::SetPortName(std::string Device) {
	port = Device;
}

std::string ceSerial::GetPort() {
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
		if ((p != 'M') && (p != 'S')) p = 'N';
	}
	parity = p;
}

char ceSerial::GetParity() {
	return parity;
}

void ceSerial::SetStopBits(float nbits) {
	if (nbits >= 2) stopbits = 2;
	else if(nbits >= 1.5) stopbits = 1.5;
	else stopbits = 1;
}

float ceSerial::GetStopBits() {
	return stopbits;
}

void ceSerial::SetFlowControl(ceSerial::FlowControl flow)
{
	flowControl = flow;
}

ceSerial::FlowControl ceSerial::GetFlowControl()
{
	return flowControl;
}


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

long ceSerial::Open() {
	if (IsOpened()) return 0;
#ifdef UNICODE
	wstring wtext(port.begin(),port.end());
#else
	std::string wtext = port;
#endif
    hComm = CreateFile(wtext.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        0);
    if (hComm == INVALID_HANDLE_VALUE) {return GetLastError();}

    if (PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR) == 0) {return GetLastError();}//purge

    //get initial state
    DCB dcbOri;
    bool fSuccess;
    fSuccess = GetCommState(hComm, &dcbOri);
    if (!fSuccess) {return GetLastError();}

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

	switch (flowControl)
	{
		default:
			// fall through to 'None'
			__fallthrough;

		case FlowControl_None:
			dcb1.fOutxCtsFlow = false;
			dcb1.fOutxDsrFlow = false;
			dcb1.fDtrControl = DTR_CONTROL_DISABLE;
			dcb1.fRtsControl = RTS_CONTROL_DISABLE;
			dcb1.fInX = false;
			dcb1.fOutX = false;
			break;

		case FlowControl_Software:		// XON, XOFF
			dcb1.fOutxCtsFlow = false;
			dcb1.fOutxDsrFlow = false;
			dcb1.fDtrControl = DTR_CONTROL_DISABLE;
			dcb1.fRtsControl = RTS_CONTROL_DISABLE;
			dcb1.fInX = true;
			dcb1.fOutX = true;
			break;

		case FlowControl_Hardware:		// RTS, CTS
			dcb1.fOutxCtsFlow = true;
			dcb1.fOutxDsrFlow = false;
			dcb1.fDtrControl = DTR_CONTROL_DISABLE;
			dcb1.fRtsControl = RTS_CONTROL_HANDSHAKE;
			dcb1.fInX = false;
			dcb1.fOutX = false;
			break;
	}

	fSuccess = SetCommState(hComm, &dcb1);
    this->Delay(60);
    if (!fSuccess) {return GetLastError();}

    fSuccess = GetCommState(hComm, &dcb1);
    if (!fSuccess) {return GetLastError();}

    osReader = { 0 };// Create the overlapped event.
    // Must be closed before exiting to avoid a handle leak.
    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (osReader.hEvent == NULL) {return GetLastError();}// Error creating overlapped event; abort.
    fWaitingOnRead = FALSE;

    osWrite = { 0 };
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osWrite.hEvent == NULL) {return GetLastError();}

    if (!GetCommTimeouts(hComm, &timeouts_ori)) { return GetLastError(); } // Error getting time-outs.
    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout = 20;
    timeouts.ReadTotalTimeoutMultiplier = 15;
    timeouts.ReadTotalTimeoutConstant = 100;
    timeouts.WriteTotalTimeoutMultiplier = 15;
    timeouts.WriteTotalTimeoutConstant = 100;
    if (!SetCommTimeouts(hComm, &timeouts)) { return GetLastError();} // Error setting time-outs.
	return 0;
}

void ceSerial::Close() {
	if (IsOpened())
	{
		SetCommTimeouts(hComm, &timeouts_ori);
		CloseHandle(osReader.hEvent);
		CloseHandle(osWrite.hEvent);
		CloseHandle(hComm);//close comm port
		hComm = INVALID_HANDLE_VALUE;
	}
}

bool ceSerial::IsOpened() {
	if(hComm == INVALID_HANDLE_VALUE) return false;
	else return true;
}

bool ceSerial::Write(char *data) {
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

bool ceSerial::Write(char *data,long n) {
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

bool ceSerial::WriteChar(char ch) {
	char s[2];
	s[0]=ch;
	s[1]=0;//null terminated
	return Write(s);
}

char ceSerial::ReadChar(bool& success) {
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

bool ceSerial::SetRTS(bool value) {
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

bool ceSerial::SetDTR(bool value) {
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

bool ceSerial::GetCTS(bool& success) {
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

bool ceSerial::GetDSR(bool& success) {
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

bool ceSerial::GetRI(bool& success) {
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

bool ceSerial::GetCD(bool& success) {
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

