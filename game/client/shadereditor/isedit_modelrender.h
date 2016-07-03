#ifndef ISEDIT_MODELRENDER_H
#define ISEDIT_MODELRENDER_H
#ifdef _WIN32
#pragma once
#endif // _WIN32

#ifdef SHADER_EDITOR_DLL
#include "../public/tier1/interface.h"
#else
#include "interface.h"
#include "shadereditor/shadereditorsystem.h"
#endif // SHADER_EDITOR_DLL

class ISEditModelRender
{
public:
	virtual bool LoadModel( const char *localPath ) = 0;
	virtual void DestroyModel() = 0;
	virtual void GetModelCenter( float *pFl3_ViewOffset ) = 0;

	virtual int QuerySequences( char ***list ) = 0;
	virtual void SetSequence( const char *name ) = 0;

	virtual void ExecRender() = 0;
	virtual void DoPostProc( int x, int y, int w, int h ) = 0;
	virtual int MaterialPicker( char ***szMat ) = 0;

	virtual void DestroyCharPtrList( char ***szList ) = 0;
};

#ifdef SHADER_EDITOR_DLL
extern ISEditModelRender *sEditMRender;
#else
class SEditModelRender;
extern SEditModelRender *sEditMRender;
#endif // SHADER_EDITOR_DLL
#endif // ISEDIT_MODELRENDER_H
