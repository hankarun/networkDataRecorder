#include <wx/wxprec.h>

#include <array>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/grid.h>
#include <wx/button.h>
#include <wx/socket.h>

#include <wx/filedlg.h>

DECLARE_EVENT_TYPE(wxEVT_SIZE_EVENT, -1)
DEFINE_EVENT_TYPE(wxEVT_SIZE_EVENT)

class PlayingThread :public wxThread
{
protected:
	wxEvtHandler* m_pParent;
public:
	PlayingThread(wxEvtHandler* pParent) : wxThread(), m_pParent(pParent) {}

	wxThread::ExitCode Entry() override
	{
		int iError = EXIT_SUCCESS;
		while (!TestDestroy())
		{
			wxCommandEvent evt(wxEVT_SIZE_EVENT, wxID_ANY);
			evt.SetInt(currentSize++);
			m_pParent->AddPendingEvent(evt);

			wxMilliSleep(timeout);
			if (iError != EXIT_SUCCESS) break;
		}

		return nullptr;
	}

private:
	int currentSize = 0;
	int timeout = 16;
};

class RecordingThread :public wxThread
{
protected:
	wxEvtHandler* m_pParent;
public:
	RecordingThread(wxEvtHandler* pParent) : wxThread(), m_pParent(pParent) {}

	wxThread::ExitCode Entry() override
	{
		int iError = EXIT_SUCCESS;
		while (!TestDestroy())
		{
			wxCommandEvent evt(wxEVT_SIZE_EVENT, wxID_ANY);
			evt.SetInt(currentSize++);
			m_pParent->AddPendingEvent(evt);

			wxMilliSleep(100);
			if (iError != EXIT_SUCCESS) break;
		}

		return nullptr;
	}

private:
	int currentSize = 0;
};

class MyApp : public wxApp
{
public:
	virtual bool OnInit();
};

class MyFrame : public wxFrame
{
	DECLARE_EVENT_TABLE()
public:
	MyFrame();

private:
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);

	void onRecordClick(wxCommandEvent& event);
	void onPlayClick(wxCommandEvent& event);
	void onStopClick(wxCommandEvent& event);

	void OnThread(wxCommandEvent& evt);
private:
	wxButton* recordButton = nullptr;
	wxButton* playButton = nullptr;
	wxButton* stopdButton = nullptr;

	wxMenuBar* m_pMenuBar;
	wxMenu* m_pFileMenu;
	wxMenu* m_pHelpMenu;

	wxStaticText* size;
	PlayingThread* m_pThread;
};

enum
{
	ID_Hello = 1
};

wxIMPLEMENT_APP(MyApp);

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_COMMAND(wxID_ANY, wxEVT_SIZE_EVENT, MyFrame::OnThread)
END_EVENT_TABLE()

bool MyApp::OnInit()
{
	MyFrame* frame = new MyFrame();
	frame->SetSize(500, 200);
	frame->Show(true);
	return true;
}

MyFrame::MyFrame()
	: wxFrame(nullptr, wxID_ANY, "Network Recroder", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
{
	CreateStatusBar();

	Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);

	m_pMenuBar = new wxMenuBar();
	// File Menu
	m_pFileMenu = new wxMenu();
	m_pFileMenu->Append(wxID_EXIT, _T("&Quit"));
	m_pMenuBar->Append(m_pFileMenu, _T("&File"));
	// About menu
	m_pHelpMenu = new wxMenu();
	m_pHelpMenu->Append(wxID_ABOUT, _T("&About"));
	m_pMenuBar->Append(m_pHelpMenu, _T("&Help"));

	SetMenuBar(m_pMenuBar);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxHORIZONTAL);
	recordButton = new wxButton(this, -1, "Record", wxPoint(10, 10), wxSize(50, 50));
	recordButton->Bind(wxEVT_BUTTON, &MyFrame::onRecordClick, this);

	playButton = new wxButton(this, -1, "Play", wxPoint(60, 10), wxSize(50, 50));
	playButton->Bind(wxEVT_BUTTON, &MyFrame::onPlayClick, this);

	stopdButton = new wxButton(this, -1, "Stop", wxPoint(110, 10), wxSize(50, 50));
	stopdButton->Bind(wxEVT_BUTTON, &MyFrame::onStopClick, this);
	stopdButton->Enable(false);

	boxSizer->Add(recordButton);
	boxSizer->Add(playButton);
	boxSizer->Add(stopdButton);

	wxTextCtrl* ipText = new wxTextCtrl(this, -1, "225.1.1.55");
	wxTextCtrl* inPort = new wxTextCtrl(this, -1, "8000");
	size = new wxStaticText(this, -1, "-");

	wxBoxSizer* connectionInfo = new wxBoxSizer(wxHORIZONTAL);
	connectionInfo->Add(new wxStaticText(this, -1, "IP"));
	connectionInfo->Add(ipText);
	connectionInfo->Add(new wxStaticText(this, -1, "Port"));
	connectionInfo->Add(inPort);

	wxBoxSizer* recorPlayInfo = new wxBoxSizer(wxHORIZONTAL);
	recorPlayInfo->Add(new wxStaticText(this, -1, "Data Size"));
	recorPlayInfo->Add(size);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(connectionInfo);
	mainSizer->Add(boxSizer);
	mainSizer->Add(recorPlayInfo);

	SetSizer(mainSizer);
}

void MyFrame::OnExit(wxCommandEvent& event)
{
	Close(true);
}

void MyFrame::OnAbout(wxCommandEvent& event)
{

}

void MyFrame::onRecordClick(wxCommandEvent& event)
{
	wxFileDialog saveFileDialog(this, _("Save Record File"), "", "", "Record (*.netRecord)|*.netRecord", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (saveFileDialog.ShowModal() == wxID_CANCEL)
		return;

	auto path = saveFileDialog.GetPath();

	recordButton->Enable(false);
	playButton->Enable(false);
	stopdButton->Enable(true);

	m_pThread = new PlayingThread(this);	
	if (wxTHREAD_NO_ERROR != m_pThread->Create()) { /* handle creation error here! */ }
	m_pThread->Run(); // run the thread
}

void MyFrame::onPlayClick(wxCommandEvent& event)
{
	wxFileDialog openFileDialog(this, _("Open Record file"), "", "", "Record (*.netRecord)|*.netRecord", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return;

	auto path = openFileDialog.GetPath();

	recordButton->Enable(false);
	playButton->Enable(false);
	stopdButton->Enable(true);

	m_pThread = new PlayingThread(this);
	if (wxTHREAD_NO_ERROR != m_pThread->Create()) { /* handle creation error here! */ }
	m_pThread->Run(); // run the thread
}

void MyFrame::onStopClick(wxCommandEvent& event)
{
	recordButton->Enable(true);
	playButton->Enable(true);
	stopdButton->Enable(false);

	m_pThread->Delete();
	m_pThread = nullptr;
}

void MyFrame::OnThread(wxCommandEvent& evt)
{
	if (evt.GetEventType() == wxEVT_SIZE_EVENT)
	{
		size->SetLabelText(std::to_string(evt.GetInt()));
	}
}
