#ifndef SHADEREDITORSYSTEM_H
#define SHADEREDITORSYSTEM_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

#include "datacache/imdlcache.h"

#include "iviewrender.h"
#include "view_shared.h"
#include "viewrender.h"

class ShaderEditorHandler : public CAutoGameSystemPerFrame
{
public:
	ShaderEditorHandler( char const *name );
	~ShaderEditorHandler();

	virtual bool Init();
	virtual void Shutdown();

	virtual void Update( float frametime );
	virtual void InitialPreRender();
	virtual void PostRender();

#ifdef SOURCE_2006
	void CustomViewRender( int *viewId, const VisibleFogVolumeInfo_t &fogVolumeInfo );
#else
	void CustomViewRender( int *viewId, const VisibleFogVolumeInfo_t &fogVolumeInfo, const WaterRenderInfo_t &waterRenderInfo );
#endif // SOURCE_2006
	void CustomPostRender();
	void UpdateSkymask( bool bCombineMode, int x, int y, int w, int h );

	const bool IsReady();
	int &GetViewIdForModify();
	const VisibleFogVolumeInfo_t &GetFogVolumeInfo();
#ifndef SOURCE_2006
	const WaterRenderInfo_t &GetWaterRenderInfo();
#endif // SOURCE_2006

private:
	bool m_bReady;

	void RegisterCallbacks();
	void PrepareCallbackData();

	void RegisterViewRenderCallbacks();

	int *m_piCurrentViewId;
	VisibleFogVolumeInfo_t m_tFogVolumeInfo;
#ifndef SOURCE_2006
	WaterRenderInfo_t m_tWaterRenderInfo;
#endif // SOURCE_2006
};

extern ShaderEditorHandler *g_ShaderEditorSystem;

#endif // SHADEREDITORSYSTEM_H
