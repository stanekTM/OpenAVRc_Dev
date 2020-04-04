 /*
 **************************************************************************
 *                                                                        *
 *                 ____                ___ _   _____                      *
 *                / __ \___  ___ ___  / _ | | / / _ \____                 *
 *               / /_/ / _ \/ -_) _ \/ __ | |/ / , _/ __/                 *
 *               \____/ .__/\__/_//_/_/ |_|___/_/|_|\__/                  *
 *                   /_/                                                  *
 *                                                                        *
 *              This file is part of the OpenAVRc project.                *
 *                                                                        *
 *                         Based on code(s) named :                       *
 *             OpenTx - https://github.com/opentx/opentx                  *
 *             Deviation - https://www.deviationtx.com/                   *
 *                                                                        *
 *                Only AVR code here for visibility ;-)                   *
 *                                                                        *
 *   OpenAVRc is free software: you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by *
 *   the Free Software Foundation, either version 2 of the License, or    *
 *   (at your option) any later version.                                  *
 *                                                                        *
 *   OpenAVRc is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *   GNU General Public License for more details.                         *
 *                                                                        *
 *       License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html          *
 *                                                                        *
 **************************************************************************
*/

#include <wx/msgdlg.h>

#include "BluetoothFrame.h"
#include "OpenAVRc_DesktopMain.h"

//(*InternalHeaders(BluetoothFrame)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(BluetoothFrame)
const long BluetoothFrame::ID_STATICBOX1 = wxNewId();
const long BluetoothFrame::ID_COMBOBOX1 = wxNewId();
const long BluetoothFrame::ID_STATICTEXT1 = wxNewId();
const long BluetoothFrame::ID_STATICTEXT2 = wxNewId();
const long BluetoothFrame::ID_STATICTEXT3 = wxNewId();
const long BluetoothFrame::ID_PANEL1 = wxNewId();
const long BluetoothFrame::ID_TIMERRX = wxNewId();
//*)

BEGIN_EVENT_TABLE(BluetoothFrame,wxFrame)
	//(*EventTable(BluetoothFrame)
	//*)
END_EVENT_TABLE()

BluetoothFrame::BluetoothFrame(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(BluetoothFrame)
	Create(parent, wxID_ANY, _("Bluetooth"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetClientSize(wxSize(645,409));
	Panel1 = new wxPanel(this, ID_PANEL1, wxPoint(392,176), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	StaticBoxCom = new wxStaticBox(Panel1, ID_STATICBOX1, _("Communication"), wxPoint(8,8), wxSize(392,88), 0, _T("ID_STATICBOX1"));
	ComboBoxCom = new wxComboBox(Panel1, ID_COMBOBOX1, wxEmptyString, wxPoint(64,32), wxSize(72,23), 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX1"));
	StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Port :"), wxPoint(8,32), wxSize(48,16), wxALIGN_RIGHT, _T("ID_STATICTEXT1"));
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("M�moire libre :"), wxPoint(160,32), wxSize(96,16), wxALIGN_RIGHT, _T("ID_STATICTEXT2"));
	StaticTextFreeMem = new wxStaticText(Panel1, ID_STATICTEXT3, _("------"), wxPoint(264,32), wxSize(80,16), wxALIGN_LEFT, _T("ID_STATICTEXT3"));
	TimerRX.SetOwner(this, ID_TIMERRX);
	TimerRX.Start(200, true);

	Connect(ID_COMBOBOX1,wxEVT_COMMAND_COMBOBOX_SELECTED,(wxObjectEventFunction)&BluetoothFrame::OnComboBoxComSelected);
	Connect(ID_COMBOBOX1,wxEVT_COMMAND_COMBOBOX_DROPDOWN,(wxObjectEventFunction)&BluetoothFrame::OnComboBoxComDropdown);
	Connect(ID_TIMERRX,wxEVT_TIMER,(wxObjectEventFunction)&BluetoothFrame::OnTimerRXTrigger);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&BluetoothFrame::OnClose);
	//*)

 {
  SetIcon(wxICON(oavrc_icon));
 }

 BTComPort = new Tserial();
 comIsValid = false;
 uCLI = "uCLI>";
}

BluetoothFrame::~BluetoothFrame()
{
	//(*Destroy(BluetoothFrame)
	//*)
}

void BluetoothFrame::OnClose(wxCloseEvent& event)
{
  if (BTComPort != NULL) delete BTComPort;
  OpenAVRc_DesktopFrame *parent = wxDynamicCast(this->GetParent(), OpenAVRc_DesktopFrame);
  if(parent)
    parent->EnableBluetoothSelectedMenu();
  Destroy();
}

void BluetoothFrame::DetectSerial()
{
  TCHAR Devices [5000];
  for(int i=0; i<255; i++) // checking ports from COM0 to COM255
  {
    wxString str;
    str = str.Format(wxT("%i"),i);
    wxString ComName = "COM"+str; // converting to COM0, COM1, COM2
    long test = QueryDosDevice(ComName.c_str(), Devices, 5000); //Win32(64) API only
    if (test!=0) //QueryDosDevice returns zero if it didn't find an object
    {
      ComboBoxCom->Insert(ComName,0); // add to the ComboBox
    }
  }
}

void BluetoothFrame::ConnectBTCom(wxString name)
{
  int error;
  char comMame[10];
  strncpy(comMame, (const char*)name.mb_str(wxConvUTF8), 10);
  assert(BTComPort);
  wxBusyCursor wait;
  error = BTComPort->connect(comMame, 115200, spNONE);
  if (error == 0) {
      comIsValid = true;
      wxString ram;
      sendCmdAndWaitForResp("ram", &ram);
      ram.BeforeFirst('\r'); // remove all after \r (\n)
      StaticTextFreeMem->SetLabel(ram);
      StaticTextFreeMem->Update();
  }
  else {
    wxString intString = wxString::Format(wxT("%i"), error);
    wxMessageBox("Erreur N�"+ intString + " port COM");
    }
}

void BluetoothFrame::OnComboBoxComDropdown(wxCommandEvent& event)
{
  BTComPort->disconnect();
  ComboBoxCom->Clear();
  DetectSerial();
}

void BluetoothFrame::OnComboBoxComSelected(wxCommandEvent& event)
{
  ConnectBTCom(ComboBoxCom->GetValue());
}

void BluetoothFrame::OnTimerRXTrigger(wxTimerEvent& event)
{
  timout = false;
}

void BluetoothFrame::sendCmdAndWaitForResp(wxString BTcommand, wxString* BTanwser)
{
 if (comIsValid)
  {
   int16_t l = BTcommand.length();
   if (l != 0)
    {
     char cstring[40];
     strncpy(cstring, (const char*)BTcommand.mb_str(wxConvUTF8), l);
     char CRLF[2] = {'\r','\n'};
     BTComPort->sendArray(cstring, l); // Send BTcommand
     BTComPort->sendArray(CRLF, 2); // Send EOL+CR
     timout = true;
     TimerRX.StartOnce();
     wxBusyCursor wait;
     do
      {
       wxYieldIfNeeded();
      }
     while (timout);

      int Num = BTComPort->getNbrOfBytes();
     if (Num)
      {
       char buffer[Num+1];
       BTComPort->getArray(buffer, Num);
       *BTanwser = wxString::FromUTF8(buffer);
       if(!(BTanwser->StartsWith(uCLI))) return;
       *BTanwser = BTanwser->Mid(uCLI.length()+BTcommand.length()+1); // remove uCLI> + command echo
       //*BTanwser = BTanwser->BeforeFirst('\r'); // remove all after \r (\n)
      }
    }
  }
}

