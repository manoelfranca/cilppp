//---------------------------------------------------------------------------
//
// Name:        CILPApp.cpp
// Author:      Manoel
// Created:     28/07/2011 19:47:16
// Description: 
//
//---------------------------------------------------------------------------

#include "CILPApp.h"
#include "CILPForm.h"

IMPLEMENT_APP(CILPFormApp)

bool CILPFormApp::OnInit()
{
    CILPForm* frame = new CILPForm(NULL);
    SetTopWindow(frame);
    frame->Show();
    return true;
}
 
int CILPFormApp::OnExit()
{
	return 0;
}
