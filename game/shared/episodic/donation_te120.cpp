//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Opens TE120 donation page
//
//=============================================================================

#include "cbase.h"
#include "convar.h"
#include "steam/steam_api.h"

void OpenURL_f( const CCommand &args )
{
	if ( steamapicontext && steamapicontext->SteamFriends() )
	{
		if ( args.ArgC() < 1 | args.Arg(1) == "" )
		{
			//Msg("Usage: s_cl_openurl <link>\n");
			steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage("https://www.youcaring.com/carebox-program-571865");
			return;
		}
		else if ( !Q_strncmp( args.Arg(1), "http://", 7 ) || !Q_strncmp( args.Arg(1), "https://", 8 ) )
		{
			steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage(args.Arg(1));
			return;
		}
	}

	Msg("Missing Steam API context, failed to execute: s_cl_openurl!\n");
}
ConCommand s_cl_openurl( "s_cl_openurl", OpenURL_f, "Open URL in Steam Overlay.", 0 );
