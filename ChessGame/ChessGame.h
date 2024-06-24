// ChessGame.h : main header file for the ChessGame DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CChessGameApp
// See ChessGame.cpp for the implementation of this class
//

class CChessGameApp : public CWinApp
{
public:
	CChessGameApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
