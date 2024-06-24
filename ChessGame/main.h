#pragma once
#include "pch.h"
#include "chessBoard.h"
#include "chess.h"
#include "chessJig.h"
#include "aced.h"
#include "myReactor.h"
#include "chessBoardJig.h"


void getBlockTableRecord(AcDbBlockTableRecord *&);
AcDbObjectId createChessBoard();
AcDbObjectId createChess(double,int);
AcGePoint3d findClosePoint(AcGePoint3d , CchessBoard* ,int&, int&);
void addReactor(CchessBoard* , AcDbObjectId );
void playGame();
