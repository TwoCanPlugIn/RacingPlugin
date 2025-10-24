// Copyright(C) 2018-2025 by Steven Adler
//
// This file is part of Racing plugin for OpenCPN.
//
// Racing plugin for OpenCPN is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Racing plugin for OpenCPN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with the Racing plugin for OpenCPN. If not, see <https://www.gnu.org/licenses/>.
//
// NMEA2000® is a registered trademark of the National Marine Electronics Association
// Actisense® is a registered trademark of Active Research Limited


#ifndef OPTIONS_PAGES_H
#define OPTIONS_PAGES_H

// Pre compiled headers 
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
      #include <wx/wx.h>
#endif

#include <wx/intl.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

#include "racing_globals.h"

//  A whole series of classes that populate the settings pages

class  SettingsTemplate  : public wxPanel {

public:
	// The constructor
	SettingsTemplate(wxScrolledWindow* parent, wxFlexGridSizer* sizer, wxString name);

	// and destructor
	~SettingsTemplate(void);

protected:
	
private:
	wxScrolledWindow* parentWindow;
	wxFlexGridSizer* panelSizer;
	
};

class  BasicSettings : public SettingsTemplate {

public:
	// The constructor
	BasicSettings(wxScrolledWindow* parent, wxFlexGridSizer* sizer, wxString name);
	
	// and destructor
	~BasicSettings(void);

protected:

	
private: 
	wxStaticText* labelCountdownTimer;
	wxSpinCtrl* spinCountdownTimer;
	wxStaticText* labelTackingAngle;
	wxSpinCtrl* spinTackingAngle;

	void OnCountdownTimerUpdated(wxSpinEvent& event);
	void OnTackingAngleUpdated(wxSpinEvent& event);
	
};

class  NetworkSettings : public SettingsTemplate {

public:
	// The constructor
	NetworkSettings(wxScrolledWindow* parent, wxFlexGridSizer* sizer, wxString name);

	// and destructor
	~NetworkSettings(void);

protected:
	wxCheckBox* checkNMEA2000;
	wxCheckBox* checkNMEA0183;

	void OnNMEA2000Changed(wxCommandEvent& event);
	void OnNMEA0183Changed(wxCommandEvent& event);


private:
	
};

class  MiscSettings : public SettingsTemplate {

public:
	// The constructor
	MiscSettings(wxScrolledWindow* parent, wxFlexGridSizer* sizer, wxString name);

	// and destructor
	~MiscSettings(void);

protected:


private:
	
	wxCheckBox* checkWindAngle;
	wxCheckBox* checkStartLine;
	wxCheckBox* checkLayline;
	wxCheckBox* checkMultiCanvas;
	wxCheckBox* check1;

	wxCheckBox* check2;
	wxCheckBox* check3;
	wxCheckBox* check4;
	wxCheckBox* check5;
	wxCheckBox* check6;



	void OnWindAngleChanged(wxCommandEvent& event);
	void OnStartlineChanged(wxCommandEvent& event);
	void OnLaylineChanged(wxCommandEvent& event);
	void OnCanvasChanged(wxCommandEvent& event);


};

#endif 

