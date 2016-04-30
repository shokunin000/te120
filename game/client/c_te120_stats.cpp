//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: handles all TE120 stats
//
//=============================================================================
#include "cbase.h"
#include "igamesystem.h"
#include "steam/steam_api.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_TE120Stats : public CAutoGameSystem
{
public:
	C_TE120Stats();
	virtual ~C_TE120Stats();
	void CountGameLaunches();
	void CountPlayTime();
	void ResetAllStats();

	virtual void PostInit();
	virtual void Shutdown();
	virtual void LevelInitPostEntity();
	virtual void LevelShutdownPostEntity();
private:
	 float m_flLevelStartTime;
};

C_TE120Stats::C_TE120Stats()
{
	m_flLevelStartTime = 0.0f;
}

C_TE120Stats::~C_TE120Stats()
{
}

void C_TE120Stats::PostInit()
{
  CountGameLaunches();
  // ResetAllStats();
}

void C_TE120Stats::Shutdown()
{
	// Reset all stats and achivements when developer mod is on (should be disabled in final release)
	static ConVarRef developer( "developer" );
	if ( developer.GetInt() != 0 )
	{
		ResetAllStats();// Debug
	}
}

void C_TE120Stats::LevelInitPostEntity()
{
	m_flLevelStartTime = gpGlobals->realtime;
}

void C_TE120Stats::LevelShutdownPostEntity()
{
	CountPlayTime();
}
/*
virtual bool Init() = 0;
virtual void PostInit() = 0;
virtual void Shutdown() = 0;

// Level init, shutdown
virtual void LevelInitPreEntity() = 0;
// entities are created / spawned / precached here
virtual void LevelInitPostEntity() = 0;

virtual void LevelShutdownPreClearSteamAPIContext() {};
virtual void LevelShutdownPreEntity() = 0;
// Entities are deleted / released here...
virtual void LevelShutdownPostEntity() = 0;
// end of level shutdown

// Called during game save
virtual void OnSave() = 0;

// Called during game restore, after the local player has connected and entities have been fully restored
virtual void OnRestore() = 0;
*/

// Count how often TE120 was launched
void C_TE120Stats::CountGameLaunches()
{
  int iCurrentStatValue;

  if (steamapicontext && steamapicontext->SteamUserStats())
  {
	  steamapicontext->SteamUserStats()->GetStat( "stat_num_games", &iCurrentStatValue );
	  DevMsg( "Current stat_num_games value: %i\n", iCurrentStatValue );
	  steamapicontext->SteamUserStats()->SetStat( "stat_num_games", iCurrentStatValue + 1 ); // increment var
	  steamapicontext->SteamUserStats()->StoreStats();
	  DevMsg( "New stat_num_games value: %i\n", iCurrentStatValue );
  }
}

// Count TE120 playtime (in levels)
void C_TE120Stats::CountPlayTime()
{
	float flElapsed = gpGlobals->realtime - m_flLevelStartTime;

	if ( flElapsed < 0.0f )
	{
		Assert( 0 );
		Warning( "EVENT_LEVELSHUTDOWN:  with negative elapsed time (rt %f starttime %f)\n", gpGlobals->realtime, m_flLevelStartTime );
		flElapsed = 0.0f;
	}
	DevMsg( "Playtime value: %f\n", flElapsed );
}

// Only for development, resets all stats an achivements
void C_TE120Stats::ResetAllStats()
{
  DevMsg("Reset all stats!\n");
  steamapicontext->SteamUserStats()->ResetAllStats( true );
  steamapicontext->SteamUserStats()->SetStat( "stat_num_games", 0 );
  steamapicontext->SteamUserStats()->SetStat( "stat_meters_traveled", 0.0f );
  //steamapicontext->SteamUserStats()->StoreStats();
}

static C_TE120Stats g_pTE120Stats;
C_TE120Stats *GetTE120StatsGameSystem()
{
	return &g_pTE120Stats;
}
