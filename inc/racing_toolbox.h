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

#ifndef RACING_TOOLBOX_H
#define RACING_TOOLBOX_H

// wxWidgets Precompiled Headers
#include "wx/wxprec.h"

#ifndef  WX_PRECOMP
#include "wx/wx.h"
#endif

// The dialog base class from which we are derived
// Note wxFormBuilder was used to generate the UI
#include "racing_toolboxbase.h"

// Some configuration settings
extern bool showStartline;
extern bool showLayLines;
extern bool showWindAngles;
extern bool showMultiCanvas;
extern int tackingAngle;
extern int defaultTimerValue;

class RacingToolbox : public RacingToolboxBase {
	
public:
	RacingToolbox(wxWindow* parent);
	~RacingToolbox();
	
protected:
	// Overridden methods from the base class
	void OnCountdownTimerChanged(wxSpinEvent& event);
	void OnTackingAngleChanged(wxSpinEvent& event);
	void OnWindAngleChanged(wxCommandEvent& event);
	void OnStartLineChanged(wxCommandEvent& event);
	void OnLayLinesChanged(wxCommandEvent& event);
	void OnCanvasChanged(wxCommandEvent& event);
private:
	bool settingsDirty;
};

#endif
