#ifndef CHESSBOARD_H
#define CHESSBOARD_H
#pragma once
#include "pch.h"
#include "dbproxy.h"
#include <vector>
#include "chess.h"
#include <stack>

struct ChessBoardState {
	std::vector<std::vector<int>> grids;
	std::vector<std::vector<AcDbObjectId>> chessIds;
	AcDbObjectId chessId;
};

class CchessBoard : public AcDbEntity
{
public:
	ACRX_DECLARE_MEMBERS(CchessBoard);
	void initializeGrid(int, int);

	void setCenter(ZcGePoint3d);
	void setWidth(double);
	void setHeight(double);
	void setGrids(int, int, int);
	void setChessIds(int,int,AcDbObjectId);
	ZcGePoint3d getCenter();
	double getWidth();
	double getHeight();
	int getRow();
	int getColumn();
	int getStatus(int, int);
	std::vector<std::vector<int>> getGrids();
	std::vector<std::vector<AcDbObjectId>> getChessIds();
	AcDbObjectId getCurrentChessId();
	void saveState();
	void regretChess();

protected:
	static Adesk::UInt32 kCurrentVersionNumber;
	double width, height; // 棋盘长宽
	ZcGePoint3d center; //棋盘中心点
	int row, column; // 棋盘行列数
	std::vector<std::vector<int>> grids; // 棋盘格子数组,记录该格子的棋子颜色
	std::vector<std::vector<AcDbObjectId>> chessIds; // 记录棋盘Id，当前用于实现获胜时棋子变色。
	std::stack<ChessBoardState> history;
	int stepCountA = 0; // 这两个变量服务于悔棋功能，如果两者相等，则说明可以把当前状态压栈
	int stepCountB = 0;
	AcDbObjectId currentChessId; // 记录刚放下的棋子Id，也用于悔棋
	enum gridStatus {
		empty = 0,
		white = 1,
		black = 2
	};
	enum PartialUndoCode {
		kGrids = 101,
		kChessIds = 102,
		kCenter=103,
		kWidth=104,
		kHeight=105,
	};

public:
	CchessBoard();
	virtual ~CchessBoard();

	//---- Dwg Filing protocol
	virtual Acad::ErrorStatus dwgOutFields(AcDbDwgFiler *pFiler) const;
	virtual Acad::ErrorStatus dwgInFields(AcDbDwgFiler *pFiler);

	//----- Graphics protocol
	Acad::ErrorStatus subTransformBy(const AcGeMatrix3d&);
	Acad::ErrorStatus subGetGripPoints(AcGePoint3dArray& gripPoints, AcDbIntArray & osnapModes, AcDbIntArray & geomIds) const;
	Acad::ErrorStatus subMoveGripPointsAt(const AcDbIntArray & indices, const AcGeVector3d& offset);
	Acad::ErrorStatus subGetOsnapPoints(AcDb::OsnapMode, Adesk::GsMarker, const AcGePoint3d&, const AcGePoint3d&, const AcGeMatrix3d&, AcGePoint3dArray&, AcDbIntArray &) const;

protected:
	virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw *mode);
	virtual Adesk::UInt32 subSetAttributes(AcGiDrawableTraits *traits);

};

#endif
