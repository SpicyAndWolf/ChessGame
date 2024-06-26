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
AcGePoint3d findClosePoint(AcGePoint3d , CchessBoard* ,int&, int&); // 找到与当前点击的点离得最近的棋盘格
void addReactor(CchessBoard* , AcDbObjectId ); // 给棋盘的反应器中添加一个棋子
void changeColor(CchessBoard* pChessBoard, int, int, int, int, int, int);// 获胜时修改颜色
bool isWin(CchessBoard* , int , int , int ); //判断是否胜利
bool isPointInPolygon(AcGePoint3d p, AcGePoint3d* vertices, int vertexCount);
void printToScreen(const AcString&, AcGePoint3d); // 在画布上打印文字
void removeReactor(CchessBoard* pChessBoard, AcDbObjectId chessId);

// 主函数
void playGame();
