#include "pch.h"
#include "main.h"

void playGame() {
	// 创建棋盘
	AcDbObjectId chessBoardId = createChessBoard();

	// 通过chessBoardId打开这个棋盘
	AcDbEntity* chessBoardEnt;
	if (acdbOpenAcDbEntity(chessBoardEnt, chessBoardId, AcDb::kForWrite) != Acad::eOk) {
		acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessBoardId.asOldId());
		return;
	}
	CchessBoard* chessBoard = (CchessBoard*)chessBoardEnt;

	// 创建悔棋按钮
	AcDbObjectId regretButtonId;
	createRegretButton(chessBoard, regretButtonId);

	// 通过regretButtonId打开这个按钮
	AcDbEntity* regretButtonEnt;
	if (acdbOpenAcDbEntity(regretButtonEnt, regretButtonId, AcDb::kForWrite) != Acad::eOk) {
		acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessBoardId.asOldId());
		return;
	}
	AcDbPolyline *regretButton = (AcDbPolyline *)regretButtonEnt;

	// 读取棋盘信息
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

		// 如果点击的点不在棋盘内，则不落子或者悔棋
		if (!isPointInPolygon(chessCenter, backgroundPoints, 4)) {
			// 释放当前棋子实体
			chess->erase();
			chess->close();
			--i;

			// 判断是不是点到了悔棋，如果不是则继续下一轮循环
			if (!isPointInRectangle(chessCenter, regretButton)) {
				continue;
			}

			// 悔棋并捕捉错误
			if (regret(i, chessBoard, chessColor)) {
				continue;
			}
			else {
				break;
			}

		}

		// 下棋
		//首先找到离棋子最近的棋盘位置
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
			textPoint.y =textPoint.y+ height / 2+height/20 + 20;
			textPoint.x = textPoint.x - 70;
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
	chessBoard->setStatus(0); //将棋盘设为不活动状态
	chessBoard->recordGraphicsModified(1);
	chessBoard->close();
	regretButton->close();
	acedRedraw(NULL, 1);
}