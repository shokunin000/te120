//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: handles all TE120 achievements
//
//=============================================================================
#include "cbase.h"

#ifdef CLIENT_DLL
#include "achievementmgr.h"
#include "baseachievement.h"

// Create achievement manager object
CAchievementMgr AchievementMgr;

class CAchievementE120MyFirstGravityJump : public CBaseAchievement
{
protected:

  void Init()
  {
    SetFlags( ACH_SAVE_GLOBAL );
  	SetGoal( 1 );
    SetMapNameFilter( "chapter_2" );
    m_bStoreProgressInSteam = true;
  }

  // Listen for this event (event must be defined in :/resource/modevents.res)
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
DECLARE_ACHIEVEMENT( CAchievementE120MyFirstGravityJump, ACHIEVEMENT_E120_MY_FIRST_GRAVITY_JUMP, "E120_MY_FIRST_GRAVITY_JUMP", 5 );

class CAchievementE120Chapter4 : public CBaseAchievement
{
protected:

  void Init()
  {
  	SetFlags( ACH_SAVE_GLOBAL );
    m_bStoreProgressInSteam = true;
  	SetGoal( 1 );
    SetMapNameFilter( "chapter_4" );
  }

  // Listen for this event (event must be defined in :/resource/modevents.res)
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
DECLARE_ACHIEVEMENT( CAchievementE120Chapter4, ACHIEVEMENT_E120_CHAPTER_4, "E120_CHAPTER_4", 20 );

class CAchievementE120Slicer : public CBaseAchievement
{
protected:

  void Init()
  {
  	SetFlags( ACH_SAVE_GLOBAL );
    m_bStoreProgressInSteam = true;
  	SetGoal( 1 );
    SetMapNameFilter( "chapter_3" );
  }

  // Listen for this event (event must be defined in :/resource/modevents.res)
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
DECLARE_ACHIEVEMENT( CAchievementE120Slicer, ACHIEVEMENT_E120_SLICER, "E120_SLICER", 10 );

class CAchievementE120CatchEmAll : public CBaseAchievement
{
protected:

  void Init()
  {
  	SetFlags( ACH_LISTEN_MAP_EVENTS | ACH_SAVE_WITH_GAME );
    m_bStoreProgressInSteam = true;
    SetGoal( 8 );
  }

  // don't show progress notifications for this achievement, it's distracting
	virtual bool ShouldShowProgressNotification() { return false; }
};
DECLARE_ACHIEVEMENT( CAchievementE120CatchEmAll, ACHIEVEMENT_E120_CATCH_EM_ALL, "E120_CATCH_EM_ALL", 20 );

class CAchievementE120Scholar : public CBaseAchievement
{
protected:

  void Init()
  {
    SetFlags( ACH_LISTEN_MAP_EVENTS | ACH_SAVE_WITH_GAME );
    m_bStoreProgressInSteam = true;
    SetGoal( 4 );
  }

  virtual bool ShouldShowProgressNotification() { return true; }
};
DECLARE_ACHIEVEMENT( CAchievementE120Scholar, ACHIEVEMENT_E120_SCHOLAR, "E120_SCHOLAR", 15 );

class CAchievementE120StriderSmasher : public CBaseAchievement
{
protected:

  void Init()
  {
  	SetFlags( ACH_SAVE_GLOBAL );
    m_bStoreProgressInSteam = true;
  	SetGoal( 2 );
    SetMapNameFilter( "chapter_4" );
  }

  // Listen for this event (event must be defined in :/resource/modevents.res)
  virtual void ListenForEvents()
  {
    ListenForGameEvent( "strider_smasher" );
  }

  void FireGameEvent_Internal( IGameEvent *event )
  {
    if ( 0 == Q_strcmp( event->GetName(), "strider_smasher" ) )
    {
      IncrementCount();
    }
  }

  virtual bool ShouldShowProgressNotification() { return true; }
};
DECLARE_ACHIEVEMENT( CAchievementE120StriderSmasher, ACHIEVEMENT_E120_STRIDER_SMASHER, "E120_STRIDER_SMASHER", 25 );

// achievements which are won by a map event firing once (triggerd trough logic_achievements)
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_E120_REUNITED, "E120_REUNITED", 30 );
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_E120_METROIDVANIA, "E120_METROIDVANIA", 10 );
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_E120_CHAPTER_1, "E120_CHAPTER_1", 5 );
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_E120_CHAPTER_2, "E120_CHAPTER_2", 5 );
DECLARE_MAP_EVENT_ACHIEVEMENT( ACHIEVEMENT_E120_CHAPTER_3, "E120_CHAPTER_3", 10 );

#endif // CLIENT_DLL
