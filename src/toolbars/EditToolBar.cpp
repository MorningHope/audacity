	/**********************************************************************

  Audacity: A Digital Audio Editor

  EditToolBar.cpp

  Dominic Mazzoni
  Shane T. Mueller
  Leland Lucius

  See EditToolBar.h for details

*******************************************************************//*!

\class EditToolBar
\brief A ToolBar that has the edit buttons on it.

  This class, which is a child of Toolbar, creates the
  window containing interfaces to commonly-used edit
  functions that are otherwise only available through
  menus. The window can be embedded within a normal project
  window, or within a ToolbarFrame that is managed by a
  global ToolBarStub called gControlToolBarStub.

  All of the controls in this window were custom-written for
  Audacity - they are not native controls on any platform -
  however, it is intended that the images could be easily
  replaced to allow "skinning" or just customization to
  match the look and feel of each platform.

*//*******************************************************************/


#include "../Audacity.h"
#include "EditToolBar.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/event.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/tooltip.h>
#endif

#include "../AllThemeResources.h"
#include "../AudioIO.h"
#include "../ImageManipulation.h"
#include "../Internat.h"
#include "../Prefs.h"
#include "../Project.h"
#include "../Theme.h"
#include "../Track.h"
#include "../UndoManager.h"
#include "../widgets/AButton.h"

#include "../Experimental.h"

IMPLEMENT_CLASS(EditToolBar, ToolBar);

const int BUTTON_WIDTH = 27;
const int SEPARATOR_WIDTH = 14;

////////////////////////////////////////////////////////////
/// Methods for EditToolBar
////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( EditToolBar, ToolBar )
   EVT_COMMAND_RANGE( ETBCutID,
                      ETBCutID + ETBNumButtons - 1,
                      wxEVT_COMMAND_BUTTON_CLICKED,
                      EditToolBar::OnButton )
END_EVENT_TABLE()

//Standard contructor
EditToolBar::EditToolBar()
: ToolBar(EditBarID, _("Edit"), wxT("Edit"))
{
}

EditToolBar::~EditToolBar()
{
}

void EditToolBar::Create(wxWindow * parent)
{
   ToolBar::Create(parent);
}

void EditToolBar::AddSeparator()
{
   AddSpacer();
}

/// This is a convenience function that allows for button creation in
/// MakeButtons() with fewer arguments
/// Very similar to code in ControlToolBar...
AButton *EditToolBar::AddButton(
   teBmps eEnabledUp, teBmps eEnabledDown, teBmps eDisabled,
   int id,
   const wxChar *label,
   bool toggle)
{
   AButton *&r = mButtons[id];

   r = ToolBar::MakeButton(this,
      bmpRecoloredUpSmall, bmpRecoloredDownSmall, bmpRecoloredHiliteSmall,
      eEnabledUp, eEnabledDown, eDisabled,
      wxWindowID(id),
      wxDefaultPosition,
      toggle,
      theTheme.ImageSize( bmpRecoloredUpSmall ));

   r->SetLabel(label);
// JKC: Unlike ControlToolBar, does not have a focus rect.  Shouldn't it?
// r->SetFocusRect( r->GetRect().Deflate( 4, 4 ) );

   Add( r, 0, wxALIGN_CENTER );

   return r;
}

void EditToolBar::Populate()
{
   MakeButtonBackgroundsSmall();

   /* Buttons */
   AddButton(bmpCut, bmpCut, bmpCutDisabled, ETBCutID,
      _("Cut selection"));
   AddButton(bmpCopy, bmpCopy, bmpCopyDisabled, ETBCopyID,
      _("Copy selection"));
   AddButton(bmpPaste, bmpPaste, bmpPasteDisabled, ETBPasteID,
      _("Paste"));
   AddButton(bmpTrim, bmpTrim, bmpTrimDisabled, ETBTrimID,
      _("Trim audio outside selection"));
   AddButton(bmpSilence, bmpSilence, bmpSilenceDisabled, ETBSilenceID,
      _("Silence audio selection"));

   AddSeparator();

   AddButton(bmpUndo, bmpUndo, bmpUndoDisabled, ETBUndoID,
      _("Undo"));
   AddButton(bmpRedo, bmpRedo, bmpRedoDisabled, ETBRedoID,
      _("Redo"));

   AddSeparator();

#ifdef EXPERIMENTAL_SYNC_LOCK
   AddButton(bmpSyncLockTracksUp, bmpSyncLockTracksDown, bmpSyncLockTracksDisabled, ETBSyncLockID,
               _("Sync-Lock Tracks"), true);

   AddSeparator();
#endif

   AddButton(bmpZoomIn, bmpZoomIn, bmpZoomInDisabled, ETBZoomInID,
      _("Zoom In"));
   AddButton(bmpZoomOut, bmpZoomOut, bmpZoomOutDisabled, ETBZoomOutID,
      _("Zoom Out"));

   AddButton(bmpZoomSel, bmpZoomSel, bmpZoomSelDisabled, ETBZoomSelID,
      _("Fit selection in window"));
   AddButton(bmpZoomFit, bmpZoomFit, bmpZoomFitDisabled, ETBZoomFitID,
      _("Fit project in window"));

   mButtons[ETBZoomInID]->SetEnabled(false);
   mButtons[ETBZoomOutID]->SetEnabled(false);

   mButtons[ETBZoomSelID]->SetEnabled(false);
   mButtons[ETBZoomFitID]->SetEnabled(false);
   mButtons[ETBPasteID]->SetEnabled(false);

#ifdef EXPERIMENTAL_SYNC_LOCK
   mButtons[ETBSyncLockID]->PushDown();
#endif

#if defined(EXPERIMENTAL_EFFECTS_RACK)
   AddSeparator();
   AddButton(bmpEditEffects, bmpEditEffects, bmpEditEffects, ETBEffectsID,
      _("Show Effects Rack"), true);
#endif

   RegenerateTooltips();
}

void EditToolBar::UpdatePrefs()
{
   RegenerateTooltips();

   // Set label to pull in language change
   SetLabel(_("Edit"));

   // Give base class a chance
   ToolBar::UpdatePrefs();
}

void EditToolBar::RegenerateTooltips()
{
#if wxUSE_TOOLTIPS
   mButtons[ETBCutID]->SetToolTip(_("Cut"));
   mButtons[ETBCopyID]->SetToolTip(_("Copy"));
   mButtons[ETBPasteID]->SetToolTip(_("Paste"));
   mButtons[ETBTrimID]->SetToolTip(_("Trim Audio"));
   mButtons[ETBSilenceID]->SetToolTip(_("Silence Audio"));
   mButtons[ETBUndoID]->SetToolTip(_("Undo"));
   mButtons[ETBRedoID]->SetToolTip(_("Redo"));
   #ifdef EXPERIMENTAL_SYNC_LOCK
      mButtons[ETBSyncLockID]->SetToolTip(_("Sync-Lock Tracks"));
   #endif
   mButtons[ETBZoomInID]->SetToolTip(_("Zoom In"));
   mButtons[ETBZoomOutID]->SetToolTip(_("Zoom Out"));
   mButtons[ETBZoomSelID]->SetToolTip(_("Fit Selection"));
   mButtons[ETBZoomFitID]->SetToolTip(_("Fit Project"));

#if defined(EXPERIMENTAL_EFFECTS_RACK)
   mButtons[ETBEffectsID]->SetToolTip(_("Open Effects Rack"));
#endif
#endif
}

void EditToolBar::OnButton(wxCommandEvent &event)
{
   AudacityProject *p = GetActiveProject();
   if (!p) return;

   bool busy = gAudioIO->IsBusy();
   int id = event.GetId();

   switch (id) {
      case ETBCutID:
         if (!busy) p->OnCut();
         break;
      case ETBCopyID:
         if (!busy) p->OnCopy();
         break;
      case ETBPasteID:
         if (!busy) p->OnPaste();
         break;
      case ETBTrimID:
         if (!busy) p->OnTrim();
         break;
      case ETBSilenceID:
         if (!busy) p->OnSilence();
         break;
      case ETBUndoID:
         if (!busy) p->OnUndo();
         break;
      case ETBRedoID:
         if (!busy) p->OnRedo();
         break;
#ifdef EXPERIMENTAL_SYNC_LOCK
      case ETBSyncLockID:
         p->OnSyncLock();
         return;//avoiding the call to SetButton()
#endif
      case ETBZoomInID:
         p->OnZoomIn();
         break;
      case ETBZoomOutID:
         p->OnZoomOut();
         break;

#if 0 // Disabled for version 1.2.0 since it doesn't work quite right...
      case ETBZoomToggleID:
         p->OnZoomToggle();
         break;
#endif

      case ETBZoomSelID:
         p->OnZoomSel();
         break;
      case ETBZoomFitID:
         p->OnZoomFit();
         break;
#if defined(EXPERIMENTAL_EFFECTS_RACK)
      case ETBEffectsID:
         EffectManager::Get().ShowRack();
         break;
#endif
   }

   SetButton(false, mButtons[id]);
}

void EditToolBar::EnableDisableButtons()
{
   AudacityProject *p = GetActiveProject();
   if (!p) return;

   // Is anything selected?
   bool selection = false;
   TrackListIterator iter(p->GetTracks());
   for (Track *t = iter.First(); t; t = iter.Next())
      if (t->GetSelected()) {
         selection = true;
         break;
      }
   selection &= (p->GetSel0() < p->GetSel1());

   mButtons[ETBCutID]->SetEnabled(selection);
   mButtons[ETBCopyID]->SetEnabled(selection);
   mButtons[ETBTrimID]->SetEnabled(selection);
   mButtons[ETBSilenceID]->SetEnabled(selection);

   mButtons[ETBUndoID]->SetEnabled(p->GetUndoManager()->UndoAvailable());
   mButtons[ETBRedoID]->SetEnabled(p->GetUndoManager()->RedoAvailable());

   bool tracks = (!p->GetTracks()->IsEmpty());

   mButtons[ETBZoomInID]->SetEnabled(tracks && (p->ZoomInAvailable()));
   mButtons[ETBZoomOutID]->SetEnabled(tracks && (p->ZoomOutAvailable()) );

   #if 0 // Disabled for version 1.2.0 since it doesn't work quite right...
   mButtons[ETBZoomToggleID]->SetEnabled(tracks);
   #endif

   mButtons[ETBZoomSelID]->SetEnabled(selection);
   mButtons[ETBZoomFitID]->SetEnabled(tracks);

   mButtons[ETBPasteID]->SetEnabled(p->Clipboard());

#ifdef EXPERIMENTAL_SYNC_LOCK
   bool bSyncLockTracks;
   gPrefs->Read(wxT("/GUI/SyncLockTracks"), &bSyncLockTracks, false);

   if (bSyncLockTracks)
      mButtons[ETBSyncLockID]->PushDown();
   else
      mButtons[ETBSyncLockID]->PopUp();
#endif
}


// PRL: to do: move the below to its own file
// Much of this is imitative of EditToolBar.  Should there be a common base
// class?
#include "../Audacity.h"

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/event.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/tooltip.h>
#endif

#include "../AllThemeResources.h"
#include "../AudioIO.h"
#include "../ImageManipulation.h"
#include "../Internat.h"
#include "../Prefs.h"
#include "../Project.h"
#include "../Theme.h"
#include "../Track.h"
#include "../UndoManager.h"
#include "../widgets/AButton.h"
#include "../tracks/ui/Scrubbing.h"

#include "../Experimental.h"

IMPLEMENT_CLASS(ScrubbingToolBar, ToolBar);

//const int BUTTON_WIDTH = 27;
//const int SEPARATOR_WIDTH = 14;

////////////////////////////////////////////////////////////
/// Methods for ScrubbingToolBar
////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE( ScrubbingToolBar, ToolBar )
EVT_COMMAND_RANGE( STBStartID,
                  STBStartID + STBNumButtons - 1,
                  wxEVT_COMMAND_BUTTON_CLICKED,
                  ScrubbingToolBar::OnButton )
END_EVENT_TABLE()

//Standard contructor
ScrubbingToolBar::ScrubbingToolBar()
: ToolBar(ScrubbingBarID, _("Scrub"), wxT("Scrub"))
{
}

ScrubbingToolBar::~ScrubbingToolBar()
{
}

void ScrubbingToolBar::Create(wxWindow * parent)
{
   ToolBar::Create(parent);
}

/// This is a convenience function that allows for button creation in
/// MakeButtons() with fewer arguments
/// Very similar to code in ControlToolBar...
AButton *ScrubbingToolBar::AddButton
   (teBmps eEnabledUp, teBmps eEnabledDown, teBmps eDisabled,
    int id,
    const wxChar *label,
    bool toggle)
{
   AButton *&r = mButtons[id];

   r = ToolBar::MakeButton
      (this,
       bmpRecoloredUpSmall, bmpRecoloredDownSmall, bmpRecoloredHiliteSmall,
       eEnabledUp, eEnabledDown, eDisabled,
       wxWindowID(id),
       wxDefaultPosition,
       toggle,
       theTheme.ImageSize( bmpRecoloredUpSmall ));

   r->SetLabel(label);
   // JKC: Unlike ControlToolBar, does not have a focus rect.  Shouldn't it?
   // r->SetFocusRect( r->GetRect().Deflate( 4, 4 ) );

   Add( r, 0, wxALIGN_CENTER );

   return r;
}

void ScrubbingToolBar::Populate()
{
   MakeButtonBackgroundsSmall();

   /* Buttons */
   AddButton(bmpPlay, bmpStop, bmpPlayDisabled, STBStartID,
             _("Start scrubbing"), true);
   AddButton(bmpScrub, bmpScrub, bmpScrubDisabled, STBScrubID,
             _("Scrub"), true);
   AddButton(bmpSeek, bmpSeek, bmpSeekDisabled, STBSeekID,
             _("Seek"), true);


   RegenerateTooltips();
}

void ScrubbingToolBar::UpdatePrefs()
{
   RegenerateTooltips();

   // Set label to pull in language change
   SetLabel(_("Scrubbing"));

   // Give base class a chance
   ToolBar::UpdatePrefs();
}

void ScrubbingToolBar::RegenerateTooltips()
{
#if wxUSE_TOOLTIPS
   /* i18n-hint: These commands assist the user in finding a sound by ear. ...
    "Scrubbing" is variable-speed playback, ...
    "Seeking" is normal speed playback but with skips
    */
   auto project = GetActiveProject();
   if (project) {
      auto startStop = mButtons[STBStartID];
      auto &scrubber = project->GetScrubber();
      if(scrubber.HasStartedScrubbing() || scrubber.IsScrubbing()) {
         if (scrubber.Seeks())
            startStop->SetToolTip(_("Stop seeking"));
         else
            startStop->SetToolTip(_("Stop scrubbing"));
      }
      else {
         if (scrubber.Seeks())
            startStop->SetToolTip(_("Start seeking"));
         else
            startStop->SetToolTip(_("Start scrubbing"));
      }
   }
   mButtons[STBScrubID]->SetToolTip(_("Scrub"));
   mButtons[STBSeekID]->SetToolTip(_("Seek"));
#endif
}

void ScrubbingToolBar::OnButton(wxCommandEvent &event)
{
   AudacityProject *p = GetActiveProject();
   if (!p) return;
   auto &scrubber = p->GetScrubber();

   int id = event.GetId();

   switch (id) {
      case STBStartID:
         scrubber.OnStartStop(event);
         break;
      case STBScrubID:
         scrubber.OnScrub(event);
         break;
      case STBSeekID:
         scrubber.OnSeek(event);
         break;
      default:
         wxASSERT(false);
   }

   EnableDisableButtons();
}

void ScrubbingToolBar::EnableDisableButtons()
{
   const auto scrubButton = mButtons[STBScrubID];
   scrubButton->SetEnabled(true);
   const auto seekButton = mButtons[STBSeekID];
   seekButton->SetEnabled(true);

   AudacityProject *p = GetActiveProject();
   if (!p) return;

   auto &scrubber = p->GetScrubber();
   if (scrubber.Scrubs())
      scrubButton->PushDown();
   else
      scrubButton->PopUp();

   if (scrubber.Seeks())
      seekButton->PushDown();
   else
      seekButton->PopUp();

   const auto startButton = mButtons[STBStartID];
   if (scrubber.CanScrub())
      startButton->Enable();
   else
      startButton->Disable();
}
