///-----------------------------------------------------------------
///
/// @file      CILPForm.h
/// @author    Manoel
/// Created:   28/07/2011 19:47:16
/// @section   DESCRIPTION
///            CILPForm class declaration
///
///------------------------------------------------------------------

#ifndef __CILPFORM_H__
#define __CILPFORM_H__

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

//Do not add custom headers between 
//Header Include Start and Header Include End.
//wxDev-C++ designer will remove them. Add custom headers after the block.
////Header Include Start
#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/statbox.h>
#include <wx/panel.h>
////Header Include End

using namespace std;

#include <cstring>
#include <string>
#include <iostream>
#include <ctime>
#include <cstdio>

#include "Utilities.h"

////Dialog Style Start
#undef CILPForm_STYLE
#define CILPForm_STYLE wxCLIP_CHILDREN | wxCAPTION | wxSYSTEM_MENU | wxMINIMIZE_BOX | wxCLOSE_BOX
////Dialog Style End

class CILPForm : public wxFrame
{
	private:
		DECLARE_EVENT_TABLE();
		
	public:
		CILPForm(wxWindow *parent, wxWindowID id = 1, const wxString &title = wxT("CILP"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = CILPForm_STYLE);
		void CILPFormActivate(wxActivateEvent& event);
		virtual ~CILPForm();
		void SubMenuFecharClick(wxCommandEvent& event);
		void ButtonBuildClick(wxCommandEvent& event);
		void ButtonTrainClick(wxCommandEvent& event);
		void ButtonLoadProgramClick(wxCommandEvent& event);
		void BuildLoadTrainingClick(wxCommandEvent& event);
		
		// User-made methods       
        void ShowMessage(string message);
		
	private:
		//Do not add custom control declarations between
		//GUI Control Declaration Start and GUI Control Declaration End.
		//wxDev-C++ will remove them. Add custom code after the block.
		////GUI Control Declaration Start
		wxFileDialog *WxOpenFileDialog1;
		wxMenuBar *Menu;
		wxTextCtrl *WxMemo1;
		wxStaticText *WxStaticText1;
		wxPanel *WxPanel2;
		wxStaticText *WxStaticText8;
		wxTextCtrl *EditMomentum;
		wxTextCtrl *EditLearningRate;
		wxTextCtrl *EditDecayFactor;
		wxStaticText *WxStaticText7;
		wxStaticText *WxStaticText5;
		wxTextCtrl *EditEHN;
		wxTextCtrl *EditBatchSize;
		wxTextCtrl *EditEpochNumber;
		wxStaticText *WxStaticText3;
		wxStaticText *WxStaticText2;
		wxStaticText *WxStaticText6;
		wxStaticText *WxStaticText4;
		wxComboBox *ComboTrainingType;
		wxStaticBox *WxStaticBox3;
		wxTextCtrl *EditTrainingFile;
		wxButton *BuildLoadTraining;
		wxButton *ButtonTrain;
		wxStaticBox *WxStaticBox2;
		wxButton *ButtonBuild;
		wxButton *ButtonLoadProgram;
		wxTextCtrl *EditProgramFile;
		wxStaticBox *WxStaticBox1;
		wxPanel *WxPanel1;
		////GUI Control Declaration End
		
		wxMessageDialog *messageDialog;
		FILE *logFile;
		Utilities util;
		
	private:
		//Note: if you receive any error with these enum IDs, then you need to
		//change your old form code that are based on the #define control IDs.
		//#defines may replace a numeric value for the enum names.
		//Try copy and pasting the below block in your old form header files.
		enum
		{
			////GUI Enum Control ID Start
			ID_MNU_MENUITEM1_1009 = 1009,
			ID_MNU_FECHAR_1013 = 1013,
			
			ID_WXMEMO1 = 1099,
			ID_WXSTATICTEXT1 = 1098,
			ID_WXPANEL2 = 1102,
			ID_WXSTATICTEXT8 = 1104,
			ID_EDITMOMENTUM = 1103,
			ID_WXEDIT1 = 1095,
			ID_WXEDIT5 = 1090,
			ID_WXSTATICTEXT7 = 1088,
			ID_WXSTATICTEXT5 = 1087,
			ID_WXEDIT3 = 1086,
			ID_WXEDIT2 = 1085,
			ID_WXEDIT0 = 1084,
			ID_WXSTATICTEXT3 = 1082,
			ID_WXSTATICTEXT2 = 1081,
			ID_WXSTATICTEXT6 = 1078,
			ID_WXSTATICTEXT4 = 1069,
			ID_WXCOMBOBOX1 = 1068,
			ID_WXSTATICBOX3 = 1064,
			ID_EDITTRAININGFILE = 1063,
			ID_BUILDLOADTRAINING = 1062,
			ID_BUTTONTRAIN = 1060,
			ID_WXSTATICBOX2 = 1059,
			ID_BUTTONBUILD = 1058,
			ID_BUTTONLOADPROGRAM = 1057,
			ID_EDITPROGRAMFILE = 1056,
			ID_WXSTATICBOX1 = 1054,
			ID_WXPANEL1 = 1101,
			////GUI Enum Control ID End
			ID_DUMMY_VALUE_ //don't remove this value unless you have other enum values
		};
		
	private:
		void OnClose(wxCloseEvent& event);
		void CreateGUIControls();
};

#endif
