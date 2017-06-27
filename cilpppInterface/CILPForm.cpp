///-----------------------------------------------------------------
///
/// @file      CILPForm.cpp
/// @author    Manoel
/// Created:   28/07/2011 19:47:16
/// @section   DESCRIPTION
///            CILPForm class implementation
///
///------------------------------------------------------------------

#include "CILPForm.h"

//Do not add custom headers between
//Header Include Start and Header Include End
//wxDev-C++ designer will remove them
////Header Include Start
#include "Images/Self_CILPForm_XPM.xpm"
////Header Include End

//----------------------------------------------------------------------------
// CILPForm
//----------------------------------------------------------------------------
//Add Custom Events only in the appropriate block.
//Code added in other places will be removed by wxDev-C++
////Event Table Start
BEGIN_EVENT_TABLE(CILPForm,wxFrame)
	////Manual Code Start
	////Manual Code End
	EVT_MENU(ID_MNU_FECHAR_1013, CILPForm::SubMenuFecharClick)
	EVT_BUTTON(ID_BUILDLOADTRAINING,CILPForm::BuildLoadTrainingClick)
	EVT_BUTTON(ID_BUTTONTRAIN,CILPForm::ButtonTrainClick)
	EVT_BUTTON(ID_BUTTONBUILD,CILPForm::ButtonBuildClick)
	EVT_BUTTON(ID_BUTTONLOADPROGRAM,CILPForm::ButtonLoadProgramClick)
END_EVENT_TABLE()
////Event Table End

CILPForm::CILPForm(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &position, const wxSize& size, long style)
: wxFrame(parent, id, title, position, size, style)
{
	CreateGUIControls();
}

CILPForm::~CILPForm()
{
}

void CILPForm::CreateGUIControls()
{
	//Do not add custom code between
	//GUI Items Creation Start and GUI Items Creation End
	//wxDev-C++ designer will remove them.
	//Add the custom code before or after the blocks
	////GUI Items Creation Start

	WxPanel1 = new wxPanel(this, ID_WXPANEL1, wxPoint(0, 0), wxSize(387, 337));

	WxStaticBox1 = new wxStaticBox(WxPanel1, ID_WXSTATICBOX1, wxT("Network Building"), wxPoint(16, 20), wxSize(169, 92));

	EditProgramFile = new wxTextCtrl(WxPanel1, ID_EDITPROGRAMFILE, wxT("Load a Program File..."), wxPoint(24, 44), wxSize(151, 22), wxTE_READONLY, wxDefaultValidator, wxT("EditProgramFile"));
	EditProgramFile->SetToolTip(wxT("Use the button 'Load' to load a program file. File format: each line must contain a clause like this: 'C1,C2,...,Cn->A'"));
	EditProgramFile->SetBackgroundColour(wxColour(wxT("LIGHT GREY")));

	ButtonLoadProgram = new wxButton(WxPanel1, ID_BUTTONLOADPROGRAM, wxT("Load"), wxPoint(24, 70), wxSize(75, 25), 0, wxDefaultValidator, wxT("ButtonLoadProgram"));

	ButtonBuild = new wxButton(WxPanel1, ID_BUTTONBUILD, wxT("Build"), wxPoint(102, 70), wxSize(75, 25), 0, wxDefaultValidator, wxT("ButtonBuild"));

	WxStaticBox2 = new wxStaticBox(WxPanel1, ID_WXSTATICBOX2, wxT("Network Training"), wxPoint(202, 20), wxSize(169, 91));

	ButtonTrain = new wxButton(WxPanel1, ID_BUTTONTRAIN, wxT("Train"), wxPoint(288, 70), wxSize(75, 25), 0, wxDefaultValidator, wxT("ButtonTrain"));

	BuildLoadTraining = new wxButton(WxPanel1, ID_BUILDLOADTRAINING, wxT("Load"), wxPoint(210, 70), wxSize(75, 25), 0, wxDefaultValidator, wxT("BuildLoadTraining"));

	EditTrainingFile = new wxTextCtrl(WxPanel1, ID_EDITTRAININGFILE, wxT("Load a Training File..."), wxPoint(210, 44), wxSize(151, 22), wxTE_READONLY, wxDefaultValidator, wxT("EditTrainingFile"));
	EditTrainingFile->SetToolTip(wxT("Use the button 'Load' to load a training file. File format: each line must contain a clause like this: 'C1,C2,...,Cn->A'"));
	EditTrainingFile->SetBackgroundColour(wxColour(wxT("LIGHT GREY")));

	WxStaticBox3 = new wxStaticBox(WxPanel1, ID_WXSTATICBOX3, wxT("Network Parameters"), wxPoint(16, 124), wxSize(355, 201));

	wxArrayString arrayStringFor_ComboTrainingType;
	arrayStringFor_ComboTrainingType.Add(wxT("Sequential"));
	arrayStringFor_ComboTrainingType.Add(wxT("Batch"));
	ComboTrainingType = new wxComboBox(WxPanel1, ID_WXCOMBOBOX1, wxT("Choose..."), wxPoint(100, 156), wxSize(85, 23), arrayStringFor_ComboTrainingType, wxCB_READONLY, wxDefaultValidator, wxT("ComboTrainingType"));

	WxStaticText4 = new wxStaticText(WxPanel1, ID_WXSTATICTEXT4, wxT("Training Set:"), wxPoint(30, 160), wxDefaultSize, 0, wxT("WxStaticText4"));

	WxStaticText6 = new wxStaticText(WxPanel1, ID_WXSTATICTEXT6, wxT("# of Epochs:"), wxPoint(30, 190), wxDefaultSize, 0, wxT("WxStaticText6"));

	WxStaticText2 = new wxStaticText(WxPanel1, ID_WXSTATICTEXT2, wxT("Batch Size:"), wxPoint(214, 160), wxDefaultSize, 0, wxT("WxStaticText2"));

	WxStaticText3 = new wxStaticText(WxPanel1, ID_WXSTATICTEXT3, wxT("# of EHN:"), wxPoint(218, 190), wxDefaultSize, 0, wxT("WxStaticText3"));
	WxStaticText3->SetToolTip(wxT("Number of Extra Hidden Neurons"));

	EditEpochNumber = new wxTextCtrl(WxPanel1, ID_WXEDIT0, wxT(""), wxPoint(100, 186), wxSize(85, 22), 0, wxDefaultValidator, wxT("EditEpochNumber"));

	EditBatchSize = new wxTextCtrl(WxPanel1, ID_WXEDIT2, wxT(""), wxPoint(276, 156), wxSize(85, 22), 0, wxDefaultValidator, wxT("EditBatchSize"));

	EditEHN = new wxTextCtrl(WxPanel1, ID_WXEDIT3, wxT(""), wxPoint(276, 186), wxSize(85, 22), 0, wxDefaultValidator, wxT("EditEHN"));

	WxStaticText5 = new wxStaticText(WxPanel1, ID_WXSTATICTEXT5, wxT("Decay Factor:"), wxPoint(198, 220), wxDefaultSize, 0, wxT("WxStaticText5"));

	WxStaticText7 = new wxStaticText(WxPanel1, ID_WXSTATICTEXT7, wxT("Learning Rate:"), wxPoint(20, 220), wxDefaultSize, 0, wxT("WxStaticText7"));

	EditDecayFactor = new wxTextCtrl(WxPanel1, ID_WXEDIT5, wxT(""), wxPoint(276, 216), wxSize(85, 22), 0, wxDefaultValidator, wxT("EditDecayFactor"));

	EditLearningRate = new wxTextCtrl(WxPanel1, ID_WXEDIT1, wxT(""), wxPoint(100, 216), wxSize(85, 22), 0, wxDefaultValidator, wxT("EditLearningRate"));

	EditMomentum = new wxTextCtrl(WxPanel1, ID_EDITMOMENTUM, wxT(""), wxPoint(100, 246), wxSize(85, 22), 0, wxDefaultValidator, wxT("EditMomentum"));

	WxStaticText8 = new wxStaticText(WxPanel1, ID_WXSTATICTEXT8, wxT("Momentum:"), wxPoint(30, 250), wxDefaultSize, 0, wxT("WxStaticText8"));

	WxPanel2 = new wxPanel(this, ID_WXPANEL2, wxPoint(0, 338), wxSize(403, 337), wxCLIP_CHILDREN);

	WxStaticText1 = new wxStaticText(WxPanel2, ID_WXSTATICTEXT1, wxT("System Log"), wxPoint(152, 14), wxDefaultSize, 0, wxT("WxStaticText1"));
	WxStaticText1->SetFont(wxFont(9, wxSWISS, wxNORMAL, wxBOLD, false, wxT("Segoe UI Semibold")));

	WxMemo1 = new wxTextCtrl(WxPanel2, ID_WXMEMO1, wxEmptyString, wxPoint(16, 38), wxSize(353, 253), wxTE_READONLY | wxTE_MULTILINE, wxDefaultValidator, wxT("WxMemo1"));
	WxMemo1->SetMaxLength(0);
	WxMemo1->AppendText(wxT("HELLO!!"));
	WxMemo1->SetFocus();
	WxMemo1->SetInsertionPointEnd();

	Menu = new wxMenuBar();
	wxMenu *ID_MNU_MENUITEM1_1009_Mnu_Obj = new wxMenu(0);
	ID_MNU_MENUITEM1_1009_Mnu_Obj->Append(ID_MNU_FECHAR_1013, wxT("Close"), wxT(""), wxITEM_NORMAL);
	Menu->Append(ID_MNU_MENUITEM1_1009_Mnu_Obj, wxT("File"));
	SetMenuBar(Menu);

	WxOpenFileDialog1 =  new wxFileDialog(this, wxT("Choose a file"), wxT(""), wxT(""), wxT("*.txt"), wxOPEN);

	SetTitle(wxT("CILP"));
	SetIcon(Self_CILPForm_XPM);
	SetSize(475,26,391,711);
	Center();
	
	////GUI Items Creation End
	
	// Initializations needs to be inserted here
    Utilities *utilPointer = new Utilities(WxMemo1);
	util = *utilPointer;
    ComboTrainingType->SetValue("Batch");
}

void CILPForm::OnClose(wxCloseEvent& event)
{
    fclose(logFile);
	Destroy();
}

void CILPForm::ShowMessage(string message)
{
    messageDialog = new wxMessageDialog(this, wxT(message), wxT("Message box"), wxOK | wxICON_INFORMATION | wxSTAY_ON_TOP);
    messageDialog->ShowModal();
}

/*
 * SubMenuFecharClick
 */
void CILPForm::SubMenuFecharClick(wxCommandEvent& event)
{
    fclose(logFile);
	Destroy();
}

/*
 * ButtonTrainClick
 */
void CILPForm::ButtonTrainClick(wxCommandEvent& event)
{
	 
}

/*
 * ButtonBuildClick
 */
void CILPForm::ButtonBuildClick(wxCommandEvent& event)
{
    // TODO2
}

/*
 * ButtonLoadProgramClick
 */
void CILPForm::ButtonLoadProgramClick(wxCommandEvent& event)
{
    if(WxOpenFileDialog1->ShowModal() == wxID_OK)
        EditProgramFile->SetValue(WxOpenFileDialog1->GetPath());
}

/*
 * BuildLoadTrainingClick
 */
void CILPForm::BuildLoadTrainingClick(wxCommandEvent& event)
{
    if(WxOpenFileDialog1->ShowModal() == wxID_OK)
        EditTrainingFile->SetValue(WxOpenFileDialog1->GetPath());
}
