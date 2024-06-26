#include "pch.h"
#include "framework.h"
#include "main.h"
#include <cmath>


enum chessStatus {
	empty = 0,
	white = 1,
	black = 2,
	success = 3
};

void getBlockTableRecord(AcDbBlockTableRecord *& pBlockTableRecord) {
	AcDbBlockTable *pBlockTable;
	acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);

	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
	pBlockTable->close();
}

bool isPointInRectangle(const AcGePoint3d& clickPoint, const AcDbPolyline* pRectangle) {
	if (pRectangle->numVerts() != 4) {
		acutPrintf(_T("\n错误：该多段线不是一个矩形"));
		return false;
	}

	// 获取矩形的四个顶点
	AcGePoint3d vertices[4];
	for (int i = 0; i < 4; ++i) {
		AcGePoint2d vertex2d;
		pRectangle->getPointAt(i, vertex2d);
		vertices[i] = AcGePoint3d(vertex2d.x, vertex2d.y, 0.0); // 将2D点转换为3D点
	}

	// 定义向量
	AcGeVector3d v1 = vertices[1] - vertices[0];
	AcGeVector3d v2 = vertices[2] - vertices[1];
	AcGeVector3d v3 = vertices[3] - vertices[2];
	AcGeVector3d v4 = vertices[0] - vertices[3];

	// 点到顶点的向量
	AcGeVector3d vp1 = clickPoint - vertices[0];
	AcGeVector3d vp2 = clickPoint - vertices[1];
	AcGeVector3d vp3 = clickPoint - vertices[2];
	AcGeVector3d vp4 = clickPoint - vertices[3];

	// 计算叉积
	double c1 = v1.crossProduct(vp1).z;
	double c2 = v2.crossProduct(vp2).z;
	double c3 = v3.crossProduct(vp3).z;
	double c4 = v4.crossProduct(vp4).z;

	// 判断点是否在矩形内部
	if ((c1 >= 0 && c2 >= 0 && c3 >= 0 && c4 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0 && c4 <= 0))
		return true;
	else
		return false;
}



void createRegretButton(CchessBoard* pNewEntity, AcDbBlockTableRecord* pBlockTableRecord) {
	// 计算矩形的位置
	AcGePoint3d chessBoardCenter = pNewEntity->getCenter();
	double chessBoardWidth = pNewEntity->getWidth();
	double chessBoardHeight = pNewEntity->getHeight();

	double rectWidth = chessBoardWidth / 5.0;
	double rectHeight = chessBoardHeight / 6.0;
	double offsetX = chessBoardWidth / 4.0;
	double rectLeftX = chessBoardCenter.x + chessBoardWidth / 2.0 + offsetX;
	double rectBottomY = chessBoardCenter.y - rectHeight / 2.0;

	// 创建矩形对象
	AcDbPolyline *pRectangle = new AcDbPolyline(4);
	pRectangle->setDatabaseDefaults();

	// 添加矩形的四个顶点
	pRectangle->addVertexAt(0, AcGePoint2d(rectLeftX, rectBottomY));
	pRectangle->addVertexAt(1, AcGePoint2d(rectLeftX + rectWidth, rectBottomY));
	pRectangle->addVertexAt(2, AcGePoint2d(rectLeftX + rectWidth, rectBottomY + rectHeight));
	pRectangle->addVertexAt(3, AcGePoint2d(rectLeftX, rectBottomY + rectHeight));
	pRectangle->setClosed(true);

	// 将矩形放入块表
	AcDbObjectId rectId;
	pBlockTableRecord->appendAcDbEntity(rectId, pRectangle);

	// 关闭矩形
	pRectangle->close();
}

AcDbObjectId createChessBoard() {
	// 获取块表记录（模型空间）
	AcDbBlockTableRecord *pBlockTableRecord;
	getBlockTableRecord(pBlockTableRecord);

	// 创建棋盘对象，并链接Jig
	CchessBoard *pNewEntity = new CchessBoard();
	pNewEntity->setDatabaseDefaults();
	CchessBoardJig chessBoardJig=CchessBoardJig();

	// 将其放入块表
	if (chessBoardJig.startJig(pNewEntity) == AcEdJig::kNormal) {
		AcDbObjectId id;
		pBlockTableRecord->appendAcDbEntity(id, pNewEntity);

		// 创建悔棋按钮
		createRegretButton(pNewEntity, pBlockTableRecord);

		// 关闭对象
		pNewEntity->close();
		pBlockTableRecord->close();
		return id;
	}
	else {
		delete pNewEntity;
		return nullptr;
	}
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
	if (es != Acad::eOk) {
		acutPrintf(_T("\n绑定失败，错误为: %d"), es);
	}
}

void removeReactor(CchessBoard* pChessBoard, AcDbObjectId chessId) {
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbDictionary* pNameList = nullptr;

	// 获取全局字典
	Acad::ErrorStatus es = pDb->getNamedObjectsDictionary(pNameList, AcDb::kForWrite);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n无法获取全局字典"));
		return;
	}

	// 获取自定义字典
	AcDbDictionary* pReactDict = nullptr;
	es = pNameList->getAt(_T("ReactorDictionary"), (AcDbObject*&)pReactDict, AcDb::kForWrite);
	pNameList->close();
	if (es == Acad::eKeyNotFound) {
		acutPrintf(_T("\n自定义字典不存在"));
		return;
	}
	else if (es != Acad::eOk) {
		acutPrintf(_T("\n无法获取自定义字典，错误为: %d"), es);
		return;
	}

	// 设置反应器在字典中的名称
	AcString reactorName = _T("reactor_");
	strConcat(chessId, reactorName);

	// 获取反应器对象ID
	AcDbObjectId reactId;
	es = pReactDict->getAt(reactorName, reactId);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n无法找到反应器对象，错误为: %d"), es);
		pReactDict->close();
		return;
	}

	// 打开反应器对象
	CmyReactor* pReactor = nullptr;
	es = acdbOpenObject(pReactor, reactId, AcDb::kForWrite);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n无法打开反应器对象，错误为: %d"), es);
		pReactDict->close();
		return;
	}

	// 从ChessBoard对象上移除持久反应器
	es = pChessBoard->removePersistentReactor(reactId);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n从ChessBoard对象上移除持久反应器失败，错误为: %d"), es);
	}

	// 从自定义字典中删除反应器对象
	es = pReactDict->remove(reactorName);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n从自定义字典中删除反应器对象失败，错误为: %d"), es);
	}

	// 删除反应器对象
	es = pReactor->erase();
	if (es != Acad::eOk) {
		acutPrintf(_T("\n删除反应器对象失败，错误为: %d"), es);
	}

	pReactor->close();
	pReactDict->close();
}



void changeColor(CchessBoard* pChessBoard, int x, int y, int dx, int dy, int positiveStep,int negativeStep) {
	// 向该维度的正方向遍历
	std::vector<std::vector<AcDbObjectId>> chessIds = pChessBoard->getChessIds();
	for (int step = 0; step <= positiveStep; ++step) {
		// 获取棋子Id
		int newX = x + step * dx;
		int newY = y + step * dy;
		AcDbObjectId chessId = chessIds[newX][newY];

		// 通过Id重新打开这个棋子
		AcDbEntity* chessEnt;
		if (acdbOpenAcDbEntity(chessEnt, chessId, AcDb::kForWrite) != Acad::eOk) {
			acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessId.asOldId());
			return;
		}
		Cchess* chess = (Cchess*)chessEnt;

		// 修改棋子颜色
		chess->setColor(success);
		chess->recordGraphicsModified();
		chess->close();
	}

	// 向反方向遍历
	for (int step = 1; step <= negativeStep; ++step) {
		int newX = x - step * dx;
		int newY = y - step * dy;
		AcDbObjectId chessId = chessIds[newX][newY];

		// 通过Id重新打开这个棋子
		AcDbEntity* chessEnt;
		if (acdbOpenAcDbEntity(chessEnt, chessId, AcDb::kForWrite) != Acad::eOk) {
			acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessId.asOldId());
			return;
		}
		Cchess* chess = (Cchess*)chessEnt;

		// 修改棋子颜色
		chess->setColor(success);
		chess->recordGraphicsModified();
		chess->close();
	}
	acedUpdateDisplay();
}

// 思路：x,y用于判断当前下在哪个位置，遍历这个点的四周判断是否有五子连在一起
bool isWin(CchessBoard* pChessBoard, int color, int x, int y) {
	// 获取棋盘内信息和当前棋子的颜色
	std::vector<std::vector<int>> grids = pChessBoard->getGrids();
	if (color == empty) {
		return false;
	}

	// 在四个方向检查偏移量
	std::vector<std::pair<int, int>> directions = {
		{0, 1}, {1, 0}, {1, 1}, {1, -1}
	};

	// 获取行、列数
	int row = grids.size();
	int column = grids[0].size();

	// 开始检查
	for (auto dir : directions) {
		int count = 1;

		// 每次走的步幅
		int dx = dir.first;
		int dy = dir.second;

		// 在正负方向走的步数，用于实现动画效果的函数，可以简化传参量
		int positiveStep = 0;
		int negativeStep = 0;

		// 向该维度的正方向检查
		for (int step = 1; step < 5; ++step) {
			int newX = x + step * dx;
			int newY = y + step * dy;
			if (newX >= 0 && newX < row && newY >= 0 && newY < column && grids[newX][newY] == color) {
				positiveStep++;
				count++;
			}
			else {
				break;
			}
		}

		// 向反方向检查
		for (int step = 1; step < 5; ++step) {
			int newX = x - step * dx;
			int newY = y - step * dy;
			if (newX >= 0 && newX < row && newY >= 0 && newY < column && grids[newX][newY] == color) {
				negativeStep++;
				count++;
			}
			else {
				break;
			}
		}

		if (count >= 5) {
			// 实现动画效果
			changeColor(pChessBoard, x, y, dx, dy, positiveStep, negativeStep);
			return true;
		}
	}

	return false;
}

void printToScreen(const AcString& textString, AcGePoint3d position, double height = 20.0) {
	AcDbText* pText = new AcDbText();
	pText->setTextString(textString.kACharPtr());
	pText->setPosition(position);
	pText->setHeight(height);

	// 获取块表记录（模型空间）
	AcDbBlockTable* pBlockTable;
	acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);

	AcDbBlockTableRecord* pBlockTableRecord;
	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
	pBlockTable->close();

	// 将文本对象添加到模型空间
	AcDbObjectId textId;
	pBlockTableRecord->appendAcDbEntity(textId, pText);
	pText->close();
	pBlockTableRecord->close();
}

// 判断一个点是否在矩形内
bool isPointInPolygon(AcGePoint3d p, AcGePoint3d* vertices, int vertexCount) {
	if (vertexCount != 4) {
		return false;
	}

	// 定义向量
	AcGeVector3d v1 = vertices[1] - vertices[0];
	AcGeVector3d v2 = vertices[2] - vertices[1];
	AcGeVector3d v3 = vertices[3] - vertices[2];
	AcGeVector3d v4 = vertices[0] - vertices[3];

	// 点到顶点的向量
	AcGeVector3d vp1 = p - vertices[0];
	AcGeVector3d vp2 = p - vertices[1];
	AcGeVector3d vp3 = p - vertices[2];
	AcGeVector3d vp4 = p - vertices[3];

	// 计算叉积
	double c1 = v1.crossProduct(vp1).z;
	double c2 = v2.crossProduct(vp2).z;
	double c3 = v3.crossProduct(vp3).z;
	double c4 = v4.crossProduct(vp4).z;

	// 判断点是否在矩形内部
	if ((c1 >= 0 && c2 >= 0 && c3 >= 0 && c4 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0 && c4 <= 0))
		return true;
	else
		return false;
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
	AcGePoint3d chessBoardCenter = chessBoard->getCenter();
	AcGePoint3d pt1 = chessBoardCenter + AcGeVector3d(-width / 2, -height / 2, 0); // Bottom-left corner
	AcGePoint3d pt2 = chessBoardCenter + AcGeVector3d(width / 2, -height / 2, 0); // Bottom-right corner
	AcGePoint3d pt3 = chessBoardCenter + AcGeVector3d(width / 2, height / 2, 0); // Top-right corner
	AcGePoint3d pt4 = chessBoardCenter + AcGeVector3d(-width / 2, height / 2, 0); // Top-left corner
	AcGePoint3d backgroundPoints[4] = {
		pt1 + AcGeVector3d(-width / 20,-height / 20, 0),
		pt2 + AcGeVector3d(width / 20,-height / 20, 0),
		pt3 + AcGeVector3d(width / 20,height / 20, 0),
		pt4 + AcGeVector3d(-width / 20,height / 20, 0)
	};

	// 确定棋子半径
	double radius = min(cellWidth, cellHeight) / 2.5;
	int chessColor = black;

	// 开始游戏
	for (int i = 0; i<256; i++) {
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
			break;
		}
		Cchess* chess = (Cchess*)chessEnt;

		// 获取棋子中心点位置
		AcGePoint3d chessCenter = chess->getCenter();

		// 如果点击的点不在棋盘内，则撤销下棋
		if (!isPointInPolygon(chessCenter, backgroundPoints, 4)) {
			// 释放当前棋子实体
			chess->erase();
			chess->close();
			--i;

			// 禁止悔第一枚棋子
			if (i == 0) {
				acutPrintf(_T("第一枚棋子无法悔棋\n"));
				continue;
			}

			// 获取上枚安放的棋子
			AcDbObjectId chessToDeleteId = chessBoard->getCurrentChessId();
			AcDbEntity* chessToDeleteEnt;
			if (acdbOpenAcDbEntity(chessToDeleteEnt, chessToDeleteId, AcDb::kForWrite) != Acad::eOk) {
				acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessId.asOldId());
				break;
			}

			// 删除对应的反应器
			removeReactor(chessBoard, chessToDeleteId);

			// 从块表将棋子删除
			Cchess* chessToDelete = (Cchess*)chessToDeleteEnt;
			chessToDelete->erase();
			chessToDelete->close();

			// 恢复棋盘状况
			chessBoard->regretChess();

			// 更新棋子颜色，继续循环
			chessColor = chessColor % 2 + 1;
			--i;
			continue;
		}

		// 找到离棋子最近的棋盘位置
		int x = 0, y = 0;
		AcGePoint3d pt = findClosePoint(chessCenter, chessBoard, x, y);

		// 判断当前位置是否已存在棋子
		if (chessBoard->getStatus(x, y) != empty) {
			chess->erase();
			chess->close();
			--i;
			continue;
		}

		// 更改其中心点，并更新棋盘数据
		chess->setCenter(pt);
		chessBoard->setGrids(x, y, chessColor);
		chessBoard->setChessIds(x, y, chessId);

		// 关闭棋子
		chess->close();

		// 将棋子添加到ChessBoard的反应器当中
		addReactor(chessBoard, chessId);

		// 判断当前是否胜利
		if (isWin(chessBoard,chessColor, x, y)) {
			// 设置胜利文字的位置、内容
			AcGePoint3d textPoint = chessBoard->getCenter();
			textPoint.y =textPoint.y+ height / 2 + 20;
			AcString message;
			message.format(L"玩家 %d 获胜\n", chessColor);

			// 将胜利文字打到画布上
			printToScreen(message, textPoint,20);
			break;
		}

		// 更新棋子颜色，继续循环
		chessColor = chessColor % 2 + 1;			
	}

	// 关闭棋盘
	chessBoard->close();
}
