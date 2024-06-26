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
AcGePoint3d findClosePoint(AcGePoint3d , CchessBoard* ,int&, int&); // �ҵ��뵱ǰ����ĵ������������̸�
void addReactor(CchessBoard* , AcDbObjectId ); // �����̵ķ�Ӧ�������һ������
void changeColor(CchessBoard* pChessBoard, int, int, int, int, int, int);// ��ʤʱ�޸���ɫ
bool isWin(CchessBoard* , int , int , int ); //�ж��Ƿ�ʤ��
bool isPointInPolygon(AcGePoint3d p, AcGePoint3d* vertices, int vertexCount);
void printToScreen(const AcString&, AcGePoint3d); // �ڻ����ϴ�ӡ����
void removeReactor(CchessBoard* pChessBoard, AcDbObjectId chessId);

// ������
void playGame();
