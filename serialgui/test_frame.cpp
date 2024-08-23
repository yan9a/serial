#include "ceserial.h"
#include "test_gui.h"
using namespace std;
// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
MyFrame::MyFrame(const wxString& title)
       : wxFrame(NULL, wxID_ANY, title,wxDefaultPosition,wxSize(390, 280), wxDEFAULT_FRAME_STYLE ^ wxRESIZE_BORDER),m_timer(this, ID_TIMER)
{
    // set the frame icon
    SetIcon(wxICON(sample));

#if wxUSE_MENUS
    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

	//Edit menu
	wxMenu *editMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;

    helpMenu->Append(Minimal_About, wxT("&About\tF1"), wxT("Show about dialog"));
    fileMenu->Append(Serial_Open, wxT("&Open\tAlt-O"), wxT("Open serial port"));
	fileMenu->Append(Serial_Close, wxT("&Close\tAlt-C"), wxT("Close serial port"));
	editMenu->Append(Txt_Clear, wxT("Clea&r\tAlt-R"), wxT("Clear text"));
	fileMenu->Append(Serial_Port, wxT("&Serial Port\tAlt-S"), wxT("Select serial port"));
	fileMenu->Append(Serial_Baud, wxT("&Baud Rate\tAlt-B"), wxT("Set baud rate"));
	fileMenu->Append(Serial_DataSize, wxT("&Data Size\tAlt-D"), wxT("Set data size"));
	fileMenu->Append(Serial_Parity, wxT("&Parity\tAlt-P"), wxT("Set parity"));
	fileMenu->Append(Serial_StopBits, wxT("S&top Bits\tAlt-t"), wxT("Set stop bits"));
	fileMenu->Append(Minimal_Quit, wxT("E&xit\tAlt-X"), wxT("Quit this program"));


    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, wxT("&File"));
	menuBar->Append(editMenu, wxT("&Edit"));
    menuBar->Append(helpMenu, wxT("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
    SetStatusText(wxT("Serial Communication"));
#endif // wxUSE_STATUSBAR
	btnSend = new wxButton(this,Button_Send,wxT( "Send"), wxPoint(5, 5), wxSize(100, 25));
	txtSend = new wxTextCtrl(this,Txt_Send,wxT("Hello!"),wxPoint(120,5),wxSize(250,25));
	//lblRx = new wxStaticText(this, ID_LBLRX, wxT("Rx:"), wxPoint(5, 75), wxSize(35, 25));
	txtRx = new wxTextCtrl(this, Txt_Rx, wxT(""), wxPoint(5, 35), wxSize(365, 125), wxTE_MULTILINE);
	chkRTS = new wxCheckBox(this, Chk_RTS, wxT("RTS"), wxPoint(5, 170), wxDefaultSize);
	chkDTR = new wxCheckBox(this, Chk_DTR, wxT("DTR"), wxPoint(55, 170), wxDefaultSize);
	chkCTS = new wxCheckBox(this, ID_CHKCTS, wxT("CTS"), wxPoint(155, 170), wxDefaultSize);
	chkDSR = new wxCheckBox(this, ID_CHKDSR, wxT("DSR"), wxPoint(205, 170), wxDefaultSize);
	chkRI = new wxCheckBox(this, ID_CHKRI, wxT("RI"), wxPoint(255, 170), wxDefaultSize);
	chkCD = new wxCheckBox(this, ID_CHKCD, wxT("CD"), wxPoint(305, 170), wxDefaultSize);

	Connect(Button_Send, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(MyFrame::OnSend));
	Connect(Minimal_About,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(MyFrame::OnAbout));
	Connect(Minimal_Quit,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(MyFrame::OnQuit));
	Connect(Serial_Open,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(MyFrame::OnOpen));
	Connect(Serial_Close,wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(MyFrame::OnClose));
	Connect(Serial_Port, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::SelPort));
	Connect(Serial_Baud, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::SetBaud));
	Connect(Serial_DataSize, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::SetDataSize));
	Connect(Serial_Parity, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::SetParity));
	Connect(Serial_StopBits, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::SetStopBits));
	Connect(Timer1,wxEVT_TIMER, wxTimerEventHandler(MyFrame::OnTimer));
	Connect(Txt_Clear, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MyFrame::ClearText));
	Connect(Chk_RTS,wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyFrame::OnChkRTS));
	Connect(Chk_DTR,wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(MyFrame::OnChkDTR));
	//Bind(wxEVT_MENU, &MyFrame::OnClose, this, Serial_Close);
	m_timer.Start(250);
	chkCTS->Disable();
	chkDSR->Disable();
	chkRI->Disable();
	chkCD->Disable();
}


// event handlers
void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format(
                                  wxT("Serial Communication! \n ")
                                  wxT("Author: Yan Naing Aye \n ")
                                  wxT("Web: https://github.com/yan9a/serial")
                                  ),
        wxT("About Serial Comm"),
        wxOK | wxICON_INFORMATION,
        this);
}

void MyFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
	if(com.Open()) txtRx->AppendText(wxString::Format(wxT("Error opening port %s.\n"),com.GetPort()));
	else txtRx->AppendText(wxString::Format(wxT("Port %s is opened.\n"), com.GetPort()));
}

void MyFrame::OnClose(wxCommandEvent& WXUNUSED(event))
{
	com.Close();
	txtRx->AppendText(wxString::Format(wxT("Port %s is closed.\n"), com.GetPort()));
}

void MyFrame::SelPort(wxCommandEvent& WXUNUSED(event))
{
	if (com.IsOpened()) {
		txtRx->AppendText(wxString::Format(wxT("Close Port %s first.\n"), com.GetPort()));
	}
	else {
        wxString cdev=wxString::Format(wxT("%s"), com.GetPort());
		wxString device = wxGetTextFromUser(wxT("Enter the port"), wxT("Set Port"), cdev);
		string str = device.ToStdString();
		if (str.length() > 0) {
			com.SetPortName(str);
		}

        txtRx->AppendText(wxString::Format(wxT("Port: %s\n"), com.GetPort()));
	}
}

void MyFrame::SetParity(wxCommandEvent& WXUNUSED(event))
{
	if (com.IsOpened()) {
		txtRx->AppendText(wxString::Format(wxT("Close Port %s first.\n"), com.GetPort()));
	}
	else {
		wxString cdev = wxString::Format(wxT("%c"), com.GetParity());
#if defined(__WINDOWS__)
		wxString parity = wxGetTextFromUser(wxT("Enter the parity ( N, E, O, M, or S )"), wxT("Set Parity"), cdev);
#else
		wxString parity = wxGetTextFromUser(wxT("Enter the parity ( N, E, or O )"), wxT("Set Parity"), cdev);
#endif

		string pstr = parity.ToStdString();
		if (pstr.length() > 0) {
			com.SetParity(pstr.at(0));
		}
		txtRx->AppendText(wxString::Format(wxT("Parity: %c\n"), com.GetParity()));
	}
}

void MyFrame::SetBaud(wxCommandEvent& WXUNUSED(event))
{
	if (com.IsOpened()) {
		txtRx->AppendText(wxString::Format(wxT("Close port %s first.\n"), com.GetPort()));
	}
	else {
		long n = wxGetNumberFromUser(wxT("Enter the baud rate"), wxT("Baud rate"), wxT("Set Baud Rate"),com.GetBaudRate(),0, 1000000);
		if (n >= 0) {
			com.SetBaudRate(n);
		}
		txtRx->AppendText(wxString::Format(wxT("Baud rate: %ld\n"), com.GetBaudRate()));
	}
}

void MyFrame::SetDataSize(wxCommandEvent& WXUNUSED(event))
{
	if (com.IsOpened()) {
		txtRx->AppendText(wxString::Format(wxT("Close port %s first.\n"), com.GetPort()));
	}
	else {
		long n = wxGetNumberFromUser(wxT("Enter the data size"), wxT("Data Size"), wxT("Set Data Size"), com.GetDataSize(), 5, 8);
		if (n >= 0) {
			com.SetDataSize(n);
		}
		txtRx->AppendText(wxString::Format(wxT("Data size: %ld\n"), com.GetDataSize()));
	}
}

void MyFrame::SetStopBits(wxCommandEvent& WXUNUSED(event))
{
	if (com.IsOpened()) {
		txtRx->AppendText(wxString::Format(wxT("Close port %s first.\n"), com.GetPort()));
	}
	else {
		long n = wxGetNumberFromUser(wxT("Enter the number of stop bits"), wxT("Data Size"), wxT("Set stop bits"), long(com.GetStopBits()), 1, 2);
		if (n > 0) {
			com.SetStopBits(float(n));
		}
		txtRx->AppendText(wxString::Format(wxT("Stop bits: %ld\n"), long(com.GetStopBits())));
	}
}

void MyFrame::OnSend(wxCommandEvent& WXUNUSED(event))
{
	wxString str = txtSend->GetValue();
	wxCharBuffer buffer = str.ToUTF8();
	if (com.Write(buffer.data())) {
		txtRx->AppendText(str);
	}
	else {
		txtRx->AppendText(wxT("Write error.\n"));
	}
}

void MyFrame::OnTimer(wxTimerEvent& WXUNUSED(event))
{
	char ch; bool r;
	do {ch = com.ReadChar(r);if (r) ProcessChar(ch);} while (r);
	UpdateCommStatus();
}

void MyFrame::ProcessChar(char ch)
{
	txtRx->AppendText(wxString::Format(wxT("%c"), ch));
}

void MyFrame::ClearText(wxCommandEvent& WXUNUSED(event))
{
	txtRx->Clear();
}

void MyFrame::OnChkRTS(wxCommandEvent& WXUNUSED(event))
{
	if (!com.SetRTS(chkRTS->IsChecked())) {
		txtRx->AppendText(wxT("RTS error.\n"));
	}
}

void MyFrame::OnChkDTR(wxCommandEvent& WXUNUSED(event))
{
	if (!com.SetDTR(chkDTR->IsChecked())) {
		txtRx->AppendText(wxT("DTR error.\n"));
	}
}

void MyFrame::UpdateCommStatus()
{
	bool s;
	bool v;
	v = com.GetCTS(s);
	if (s) chkCTS->SetValue(v);
	v = com.GetDSR(s);
	if (s) chkDSR->SetValue(v);
	v = com.GetRI(s);
	if (s) chkRI->SetValue(v);
	v = com.GetCD(s);
	if (s) chkCD->SetValue(v);
}
