//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Opens TE120 donation page
//
//=============================================================================

#include "cbase.h"
#include "convar.h"
#include "steam/steam_api.h"

void OpenDonationPage_f( void )
{
  if ( steamapicontext && steamapicontext->SteamFriends() )
  {
    steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage( "http://www.youtube.com" );
  }
}

ConCommand te120_opendonationpage( "te120_opendonationpage", OpenDonationPage_f, "Opens the TE120 donation page.", 0 );
