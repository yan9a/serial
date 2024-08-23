//Description: Serial communication for wxWidgets
//WebSite: http://cool-emerald.blogspot.sg/2017/05/serial-port-programming-in-c-with.html
//MIT License (https://opensource.org/licenses/MIT)
//Copyright (c) 2017 Yan Naing Aye

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

// the application icon (under Windows and OS/2 it is in resources and even
// though we could still include the XPM here it would be unused)
#ifndef wxHAS_IMAGES_IN_RESOURCES
    #include "sample.xpm"
#endif

#include <wx/numdlg.h>
// ----------------------------------------------------------------------------
// private classes
#include "ceserial.h"
using namespace std;
// ----------------------------------------------------------------------------

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
};

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    MyFrame(const wxString& title);
	wxButton *btnSend;
	wxTextCtrl *txtSend;
	ceSerial com;
	wxTimer m_timer;
	wxTextCtrl *txtRx;
	wxCheckBox *chkRTS;
	wxCheckBox *chkDTR;
	wxCheckBox *chkCTS;
	wxCheckBox *chkDSR;
	wxCheckBox *chkRI;
	wxCheckBox *chkCD;
    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);
	void SelPort(wxCommandEvent& event);
	void SetDataSize(wxCommandEvent& event);
	void SetParity(wxCommandEvent& event);
	void SetStopBits(wxCommandEvent& event);
	void SetBaud(wxCommandEvent& event);
	void OnSend(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& event);
	void ProcessChar(char ch);
	void ClearText(wxCommandEvent& event);
	void OnChkRTS(wxCommandEvent& event);
	void OnChkDTR(wxCommandEvent& event);
	void UpdateCommStatus();
private:

};
// constants
// ----------------------------------------------------------------------------

// IDs for the controls and the menu commands
const int ID_BTNSEND = 101;
const int ID_TXTSEND = 102;
const int ID_CHKRTS = 103;
const int ID_BAUDRATE = 103;
const int ID_TIMER = 104;
const int ID_TXTRX = 105;
const int ID_CHKDTR = 106;
const int ID_SELPORT = 107;
const int ID_CHKCTS = 108;
const int ID_CHKDSR = 109;
const int ID_CHKRI = 110;
const int ID_CHKCD = 111;
const int ID_DATASIZE = 112;
const int ID_PARITY = 113;
const int ID_STOPBITS = 114;

enum
{
	Button_Send = ID_BTNSEND,
	Txt_Send = ID_TXTSEND,
	Chk_RTS = ID_CHKRTS,
	Serial_Baud = ID_BAUDRATE,
	Timer1 = ID_TIMER,
	Txt_Rx =ID_TXTRX,
	Chk_DTR = ID_CHKDTR,
	Serial_Port = ID_SELPORT,
	Serial_DataSize=ID_DATASIZE,
	Serial_Parity=ID_PARITY,
	Serial_StopBits=ID_STOPBITS,
	Txt_Clear = wxID_CLEAR,
	Serial_Open = wxID_OPEN,
	Serial_Close = wxID_CLOSE,
	Minimal_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Minimal_About = wxID_ABOUT

};
