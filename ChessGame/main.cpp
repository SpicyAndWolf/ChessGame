#include "pch.h"
#include "framework.h"
#include "main.h"
#include <cmath>

enum chessStatus {
	empty = 0,
	white = 1,
	black = 2
};

void getBlockTableRecord(AcDbBlockTableRecord *& pBlockTableRecord) {
	AcDbBlockTable *pBlockTable;
	acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);

	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
	pBlockTable->close();
}

AcDbObjectId createChessBoard() {
	// 获取块表记录（模型空间）
	AcDbBlockTableRecord *pBlockTableRecord;
	getBlockTableRecord(pBlockTableRecord);

	// 创建棋盘对象
	CchessBoard *pNewEntity = new CchessBoard();
	pNewEntity->setDatabaseDefaults();

	// 输入中心、长宽
	ads_point pt;
	double width, height;
	acedGetPoint(NULL, _T("\n输入中心："), pt);
	acedGetReal(_T("\n输入宽度："), &width);
	acedGetReal(_T("\n输入高度："), &height);

	// 设置对象属性
	AcGePoint3d center = AcGePoint3d(pt[0], pt[1], pt[2]);
	pNewEntity->setCenter(center);
	pNewEntity->setWidth(width);
	pNewEntity->setHeight(height);

	// 加入棋盘到模型空间
	AcDbObjectId objId;
	pBlockTableRecord->appendAcDbEntity(objId,pNewEntity);
	pBlockTableRecord->close();
	pNewEntity->close();
	
	return objId;
}

AcDbObjectId createChess(double r,int color) {
	// 获取块表记录（模型空间）
	AcDbBlockTableRecord *pBlockTableRecord;
	getBlockTableRecord(pBlockTableRecord);

	// 创建自定义圆对象,并连接Jig
	Cchess *pNewEntity = new Cchess(r, color);
	pNewEntity->setDatabaseDefaults();
	CchessJig chessJig=CchessJig();

	// 将其放入块表
	if (chessJig.startJig(pNewEntity) == AcEdJig::kNormal) {
		AcDbObjectId id;
		pBlockTableRecord->appendAcDbEntity(id,pNewEntity);
		pNewEntity->close();
		pBlockTableRecord->close();
		return id;
	}
	else {
		delete pNewEntity;
		return nullptr;
	}
}

// x、y用于获取那个点在二维矩阵中的位置。
AcGePoint3d findClosePoint(AcGePoint3d point,CchessBoard* chessBoard,int& x, int& y) {
	// 读取棋盘信息
	int row = chessBoard->getRow();
	int column = chessBoard->getColumn();
	double width = chessBoard->getWidth();
	double height = chessBoard->getHeight();
	AcGePoint3d center = chessBoard->getCenter();

	// 计算 grid 的大小
	double cellWidth = width / column;
	double cellHeight = height / row;

	// 初始变量
	AcGePoint3d closestPoint;
	double minDistance = DBL_MAX;

	// 开始循环找最近的点
	for (int i = 0; i <= row; ++i) {
		for (int j = 0; j <= column; ++j) {
			// 计算这一轮访问的点
			AcGePoint3d gridPoint = center + AcGeVector3d((j - column / 2.0) * cellWidth, (i - row / 2.0) * cellHeight, 0);

			// 计算棋子与该点的距离，判断是否需要修改
			double distance = point.distanceTo(gridPoint);
			if (distance < minDistance) {
				closestPoint = gridPoint;
				minDistance = distance;
				x = i;
				y = j;
			}
		}
	}

	// 返回最近的点
	return closestPoint;
}

void strConcat(AcDbObjectId chessId, AcString& reactorName) {
	AcDbHandle objHandle = chessId.handle();
	TCHAR handleStr[17];
	objHandle.getIntoAsciiBuffer(handleStr);
	reactorName.format(_T("Reactor_%s"), handleStr);
}

void addReactor(CchessBoard* pChessBoard, AcDbObjectId chessId) {
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbDictionary* pNameList = nullptr;

	// 获取全局字典
	Acad::ErrorStatus es = pDb->getNamedObjectsDictionary(pNameList, AcDb::kForWrite);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n无法获取全局字典"));
		return;
	}

	// 获取或创建自定义字典
	AcDbDictionary* pReactDict = nullptr;
	es = pNameList->getAt(_T("ReactorDictionary"), (AcDbObject*&)pReactDict, AcDb::kForWrite);
	if (es == Acad::eKeyNotFound) {
		pReactDict = new AcDbDictionary();
		AcDbObjectId reactDictId;
		es = pNameList->setAt(_T("ReactorDictionary"), pReactDict, reactDictId);
		if (es != Acad::eOk) {
			acutPrintf(_T("\n无法创建自定义字典"));
			pReactDict->close();
			pNameList->close();
			return;
		}
	}
	else if (es != Acad::eOk) {
		acutPrintf(_T("\n无法获取自定义字典，错误为: %d"), es);
		pNameList->close();
		return;
	}
	pNameList->close();

	// 获取ChessBoard中心、长宽，用于初始化反应器
	AcGePoint3d chessBoardCenter = pChessBoard->getCenter();
	double chessBoardWidth = pChessBoard->getWidth();
	double chessBoardHeight = pChessBoard->getHeight();

	// 创建反应器对象，并初始化
	CmyReactor* pReactor = new CmyReactor();
	if (!pReactor) {
		acutPrintf(_T("\n无法创建反应器对象"));
		return;
	}
	pReactor->eLinkage(chessId);
	pReactor->setChessBoardCenter(chessBoardCenter);
	pReactor->setChessBoardWidth(chessBoardWidth);
	pReactor->setChessBoardHeight(chessBoardHeight);

	// 设置反应器在字典中的名称
	AcString reactorName = _T("reactor_");
	strConcat(chessId, reactorName);

	// 把反应器对象添加到反应器字典，并获取该反应器的Id
	AcDbObjectId reactId;
	if ((pReactDict->getAt(reactorName, reactId)) == Acad::eKeyNotFound) {
		es = pReactDict->setAt(reactorName, pReactor, reactId);
		if (es == Acad::eOk) {
			pReactor->close();
			pReactDict->close();
		}
		else {
			delete pReactor;
			pReactDict->close();
			acutPrintf(_T("\n无法将反应器对象存储到反应器字典中，错误为: %d"), es);
			return;
		}
	}
	else {
		delete pReactor;
		pReactDict->close();
	}

	// 将反应器对象添加为ChessBoard的持久性反应器
	es = pChessBoard->addPersistentReactor(reactId);
	if (es == Acad::eOk) {
		acutPrintf(_T("\n绑定成功"));
	}
	else {
		acutPrintf(_T("\n绑定失败，错误为: %d"), es);
	}
}



void playGame() {
	// 创建棋盘
	AcDbObjectId chessBoardId = createChessBoard();

	// 通过Id打开这个棋盘
	AcDbEntity* chessBoardEnt;
	if (acdbOpenAcDbEntity(chessBoardEnt, chessBoardId, AcDb::kForWrite) != Acad::eOk) {
		acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessBoardId.asOldId());
		return;
	}

	// 读取棋盘信息
	CchessBoard* chessBoard = (CchessBoard*)chessBoardEnt;
	int row = chessBoard->getRow();
	int column = chessBoard->getColumn();
	double width = chessBoard->getWidth();
	double height = chessBoard->getHeight();
	double cellWidth = width / column;
	double cellHeight = height / row;

	// 确定棋子半径
	double radius = min(cellWidth, cellHeight) / 2.5;
	int chessColor = black;

	for (int i = 0; i < 10; i++) {
		// 创建棋子
		AcDbObjectId chessId = createChess(radius,chessColor);
		if (chessId == nullptr) {
			acutPrintf(_T("创建棋子失败"), chessId.asOldId());
			return;
		}

		// 通过Id重新打开这个棋子
		AcDbEntity* chessEnt;
		if (acdbOpenAcDbEntity(chessEnt, chessId, AcDb::kForWrite) != Acad::eOk) {
			acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessId.asOldId());
			return;
		}
		Cchess* chess = (Cchess*)chessEnt;

		// 获取棋子中心点位置
		AcGePoint3d center = chess->getCenter();

		// 找到离棋子最近的棋盘位置，更改其中心点
		int x = 0, y = 0;
		AcGePoint3d pt = findClosePoint(center, chessBoard, x, y);
		chess->setCenter(pt);
		chessBoard->setGrids(x, y, chessColor);

		// 关闭棋子
		chess->close();

		// 将棋子添加到ChessBoard的反应器当中
		addReactor(chessBoard, chessId);
		chessColor = chessColor % 2 + 1;
	}

	// 关闭棋盘
	chessBoard->close();

}