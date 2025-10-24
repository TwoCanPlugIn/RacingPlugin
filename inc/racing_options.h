// Copyright(C) 2018-2020 by Steven Adler
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

#ifndef RACING_OPTIONS_H
#define RACING_OPTIONS_H

// wxWidgets Precompiled Headers
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

// The dialog base class from which we are derived
// Note wxFormBuilder was used to generate the UI
#include "racing_optionsbase.h"

#include "racing_optionpages.h"

class RacingOptions : public RacingOptionsBase {
	
public:
	RacingOptions(wxWindow* parent, wxWindowID id,const wxString& title , const wxSize& size);
	~RacingOptions();
	
protected:
	// Overridden methods from the base class
	void OnInit(wxInitDialogEvent& event);
	void OnOK(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);
	void OnApply(wxCommandEvent& event);
	void OnTreeSelected(wxTreeEvent& event);
	void OnTreeChanging(wxTreeEvent& event);
	void OnSearch(wxCommandEvent& event);
	void OnChar(wxKeyEvent& event);

private:
	bool originalShowStartline;
	bool originalShowLayLines;
	bool originalShowWindAngles;
	bool originalShowMultiCanvas;
	int originalTackingAngle;
	int originalDefaultTimerValue;
	bool originalGeneratePGN130306;
	bool originalGenerateMWVSentence;

	// The different panels of options
	wxTreeItemId root;
	wxTreeItemId basic;
	wxTreeItemId network;
	wxTreeItemId misc;

	// Base class for the options panels
	SettingsTemplate* settingsTemplate;

	
};

#endif
