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
		if ( args.ArgC() < 1 || Q_strlen(args.Arg(1)) == 0 )
		{
			steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage("https://www.youcaring.com/TE120andCarebox");
			return;
		}
		else if ( !Q_strncmp( args.Arg(1), "http://", 7 ) || !Q_strncmp( args.Arg(1), "https://", 8 ) )
		{
			steamapicontext->SteamFriends()->ActivateGameOverlayToWebPage(args.Arg(1));
			return;
		}
	}

	Msg("Missing Steam API context, failed to execute: s_cl_openurl\n");

}
ConCommand s_cl_openurl( "s_cl_openurl", OpenURL_f, "Open URL in Steam Overlay.", 0 );
