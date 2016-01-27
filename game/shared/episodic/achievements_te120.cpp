//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: handles all TE120 achievements
//
//=============================================================================

#include "cbase.h"

#ifdef CLIENT_DLL

#include "achievementmgr.h"
#include "baseachievement.h"
// for CalcPlayerAttacks
#include "baseplayer_shared.h"
#include "basehlcombatweapon_shared.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CAchievementMgr AchievementMgr;	//create achievement manager object

// resets the achievements, debug only, disable in release!
//#define RESETSTATS
int CalcPlayerAttacks( bool bBulletOnly );

class CAchievementE120MyFirstGravityJump : public CBaseAchievement
{
protected:

  void Init()
  {
  	//SetFlags(ACH_LISTEN_MAP_EVENTS | ACH_SAVE_WITH_GAME);
    //SetFlags(ACH_SAVE_WITH_GAME);
    SetFlags( ACH_SAVE_GLOBAL );
  	SetGoal( 1 );
    //SetMapNameFilter( "chapter_2" );
    m_bStoreProgressInSteam = true;
  #ifdef RESETSTATS
    DevMsg("Reset all stats!\n");
    steamapicontext->SteamUserStats()->ResetAllStats( true );
  #endif
  /*
    int iCurrentStatValue;
    steamapicontext->SteamUserStats()->GetStat( "stat_test", &iCurrentStatValue );
    Msg( "Current stat_test value: %i\n", iCurrentStatValue );
    steamapicontext->SteamUserStats()->SetStat( "stat_test", iCurrentStatValue + 1 );
    steamapicontext->SteamUserStats()->StoreStats();
    Msg( "New stat_test value: %i\n", iCurrentStatValue );

    int iBulletAttackCount = CalcPlayerAttacks( true );
  */
  }

  // Listen for this event (event must be defined in :/resource/ModEvents.res)
  virtual void ListenForEvents()
  {
    ListenForGameEvent( "first_gravity_jump" );
  }

  void FireGameEvent_Internal( IGameEvent *event )
  {
    if ( 0 == Q_strcmp( event->GetName(), "first_gravity_jump" ) )
    {
      IncrementCount();
    }
  }
};
DECLARE_ACHIEVEMENT( CAchievementE120MyFirstGravityJump, ACHIEVEMENT_E120_MY_FIRST_GRAVITY_JUMP, "E120_MY_FIRST_GRAVITY_JUMP", 1 );

class CAchievementE120Chapter4 : public CBaseAchievement
{
protected:

  void Init()
  {
  	SetFlags( ACH_SAVE_GLOBAL );
    m_bStoreProgressInSteam = true;
  	SetGoal( 1 );
  }

  // Listen for this event (event must be defined in :/resource/ModEvents.res)
  virtual void ListenForEvents()
  {
    ListenForGameEvent( "chapter_4_complete" );
  }

  void FireGameEvent_Internal( IGameEvent *event )
  {
    if ( 0 == Q_strcmp( event->GetName(), "chapter_4_complete" ) )
    {
      IncrementCount();
    }
  }
};
DECLARE_ACHIEVEMENT( CAchievementE120Chapter4, ACHIEVEMENT_E120_CHAPTER_4, "E120_CHAPTER_4", 1 );

class CAchievementE120Slicer : public CBaseAchievement
{
protected:

  void Init()
  {
  	SetFlags( ACH_SAVE_GLOBAL );
    m_bStoreProgressInSteam = true;
  	SetGoal( 1 );
  }

  // Listen for this event (event must be defined in :/resource/ModEvents.res)
  virtual void ListenForEvents()
  {
    ListenForGameEvent( "sliced_zombie" );
  }

  void FireGameEvent_Internal( IGameEvent *event )
  {
    if ( 0 == Q_strcmp( event->GetName(), "sliced_zombie" ) )
    {
      IncrementCount();
    }
  }
};
DECLARE_ACHIEVEMENT( CAchievementE120Slicer, ACHIEVEMENT_E120_SLICER, "E120_SLICER", 1 );

class CAchievementE120CatchEmAll : public CBaseAchievement
{
protected:

  void Init()
  {
  	SetFlags( ACH_LISTEN_MAP_EVENTS | ACH_SAVE_WITH_GAME );
    m_bStoreProgressInSteam = true;
    SetGoal( 8 );
  }
};
DECLARE_ACHIEVEMENT( CAchievementE120CatchEmAll, ACHIEVEMENT_E120_CATCH_EM_ALL, "E120_CATCH_EM_ALL", 1 );

class CAchievementE120Scholar : public CBaseAchievement
{
protected:

  void Init()
  {
    SetFlags( ACH_LISTEN_MAP_EVENTS | ACH_SAVE_WITH_GAME );
    m_bStoreProgressInSteam = true;
    SetGoal( 4 );
  }
};
DECLARE_ACHIEVEMENT( CAchievementE120Scholar, ACHIEVEMENT_E120_SCHOLAR, "E120_SCHOLAR", 1 );

// achievements which are won by a map event firing once (triggerd trough logic_achievements)
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_E120_REUNITED, "E120_REUNITED", 1 );
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_E120_METROIDVANIA, "E120_METROIDVANIA", 1 );
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_E120_CHAPTER_1, "E120_CHAPTER_1", 1 );
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_E120_CHAPTER_2, "E120_CHAPTER_2", 1 );
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_E120_CHAPTER_3, "E120_CHAPTER_3", 1 );

//TODO: add E120_STRIDER_SMASHER achievement

/*
//-----------------------------------------------------------------------------
// Purpose: Counts the accumulated # of primary and secondary attacks from all
//			weapons (except grav gun).  If bBulletOnly is true, only counts
//			attacks with ammo that does bullet damage.
//-----------------------------------------------------------------------------
int CalcPlayerAttacks( bool bBulletOnly )
{
  C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	CAmmoDef *pAmmoDef = GetAmmoDef();
	if ( !pPlayer || !pAmmoDef )
		return 0;

	int iTotalAttacks = 0;
	int iWeapons = pPlayer->WeaponCount();
	for ( int i = 0; i < iWeapons; i++ )
	{
		CBaseHLCombatWeapon *pWeapon = dynamic_cast<CBaseHLCombatWeapon *>( pPlayer->GetWeapon( i ) );
		if ( pWeapon )
		{
			// add primary attacks if we were asked for all attacks, or only if it uses bullet ammo if we were asked to count bullet attacks
			if ( !bBulletOnly || ( pAmmoDef->m_AmmoType[pWeapon->GetPrimaryAmmoType()].nDamageType == DMG_BULLET ) )
			{
				iTotalAttacks += pWeapon->m_iPrimaryAttacks;
			}
			// add secondary attacks if we were asked for all attacks, or only if it uses bullet ammo if we were asked to count bullet attacks
			if ( !bBulletOnly || ( pAmmoDef->m_AmmoType[pWeapon->GetSecondaryAmmoType()].nDamageType == DMG_BULLET ) )
			{
				iTotalAttacks += pWeapon->m_iSecondaryAttacks;
			}
		}
	}
	return iTotalAttacks;
}
*/
#endif // CLIENT_DLL
