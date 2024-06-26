#pragma once
#ifndef UTILS_H
#define UTILS_H
#include "pch.h"
#include "chessBoard.h"
#include "chess.h"
#include "chessJig.h"
#include "aced.h"
#include "myReactor.h"
#include "chessBoardJig.h"
#include "framework.h"
#include <cmath>

enum chessStatus {
	empty = 0,
	white = 1,
	black = 2,
	success = 3
};


void getBlockTableRecord(AcDbBlockTableRecord *&);
AcDbObjectId createChessBoard();
AcDbObjectId createChess(double, int);
void strConcat(AcDbObjectId chessId, AcString& reactorName);
AcGePoint3d findClosePoint(AcGePoint3d, CchessBoard*, int&, int&); // �ҵ��뵱ǰ����ĵ������������̸�
void addReactor(CchessBoard*, AcDbObjectId); // �����̵ķ�Ӧ�������һ������
void changeColor(CchessBoard* pChessBoard, int, int, int, int, int, int);// ��ʤʱ�޸���ɫ
bool isWin(CchessBoard*, int, int, int); //�ж��Ƿ�ʤ��
bool isPointInPolygon(AcGePoint3d p, AcGePoint3d* vertices, int vertexCount);
bool isPointInRectangle(const AcGePoint3d& clickPoint, const AcDbPolyline* pRectangle);
void printToScreen(const AcString&, AcGePoint3d,double); // �ڻ����ϴ�ӡ����
void removeReactor(CchessBoard* pChessBoard, AcDbObjectId chessId);
void createRegretButton(CchessBoard* pNewEntity, AcDbObjectId& regretButtonId);
bool regret(int& i, CchessBoard* chessBoard, int& chessColor);

#endif