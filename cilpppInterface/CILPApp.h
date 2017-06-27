//---------------------------------------------------------------------------
//
// Name:        CILPApp.h
// Author:      Manoel
// Created:     28/07/2011 19:47:16
// Description: 
//
//---------------------------------------------------------------------------

#ifndef __CILPFORMApp_h__
#define __CILPFORMApp_h__

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#else
	#include <wx/wxprec.h>
#endif

class CILPFormApp : public wxApp
{
	public:
		bool OnInit();
		int OnExit();
};

#endif
