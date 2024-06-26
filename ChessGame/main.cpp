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
		acutPrintf(_T("\n���󣺸ö���߲���һ������"));
		return false;
	}

	// ��ȡ���ε��ĸ�����
	AcGePoint3d vertices[4];
	for (int i = 0; i < 4; ++i) {
		AcGePoint2d vertex2d;
		pRectangle->getPointAt(i, vertex2d);
		vertices[i] = AcGePoint3d(vertex2d.x, vertex2d.y, 0.0); // ��2D��ת��Ϊ3D��
	}

	// ��������
	AcGeVector3d v1 = vertices[1] - vertices[0];
	AcGeVector3d v2 = vertices[2] - vertices[1];
	AcGeVector3d v3 = vertices[3] - vertices[2];
	AcGeVector3d v4 = vertices[0] - vertices[3];

	// �㵽���������
	AcGeVector3d vp1 = clickPoint - vertices[0];
	AcGeVector3d vp2 = clickPoint - vertices[1];
	AcGeVector3d vp3 = clickPoint - vertices[2];
	AcGeVector3d vp4 = clickPoint - vertices[3];

	// ������
	double c1 = v1.crossProduct(vp1).z;
	double c2 = v2.crossProduct(vp2).z;
	double c3 = v3.crossProduct(vp3).z;
	double c4 = v4.crossProduct(vp4).z;

	// �жϵ��Ƿ��ھ����ڲ�
	if ((c1 >= 0 && c2 >= 0 && c3 >= 0 && c4 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0 && c4 <= 0))
		return true;
	else
		return false;
}



void createRegretButton(CchessBoard* pNewEntity, AcDbBlockTableRecord* pBlockTableRecord) {
	// ������ε�λ��
	AcGePoint3d chessBoardCenter = pNewEntity->getCenter();
	double chessBoardWidth = pNewEntity->getWidth();
	double chessBoardHeight = pNewEntity->getHeight();

	double rectWidth = chessBoardWidth / 5.0;
	double rectHeight = chessBoardHeight / 6.0;
	double offsetX = chessBoardWidth / 4.0;
	double rectLeftX = chessBoardCenter.x + chessBoardWidth / 2.0 + offsetX;
	double rectBottomY = chessBoardCenter.y - rectHeight / 2.0;

	// �������ζ���
	AcDbPolyline *pRectangle = new AcDbPolyline(4);
	pRectangle->setDatabaseDefaults();

	// ��Ӿ��ε��ĸ�����
	pRectangle->addVertexAt(0, AcGePoint2d(rectLeftX, rectBottomY));
	pRectangle->addVertexAt(1, AcGePoint2d(rectLeftX + rectWidth, rectBottomY));
	pRectangle->addVertexAt(2, AcGePoint2d(rectLeftX + rectWidth, rectBottomY + rectHeight));
	pRectangle->addVertexAt(3, AcGePoint2d(rectLeftX, rectBottomY + rectHeight));
	pRectangle->setClosed(true);

	// �����η�����
	AcDbObjectId rectId;
	pBlockTableRecord->appendAcDbEntity(rectId, pRectangle);

	// �رվ���
	pRectangle->close();
}

AcDbObjectId createChessBoard() {
	// ��ȡ����¼��ģ�Ϳռ䣩
	AcDbBlockTableRecord *pBlockTableRecord;
	getBlockTableRecord(pBlockTableRecord);

	// �������̶��󣬲�����Jig
	CchessBoard *pNewEntity = new CchessBoard();
	pNewEntity->setDatabaseDefaults();
	CchessBoardJig chessBoardJig=CchessBoardJig();

	// ���������
	if (chessBoardJig.startJig(pNewEntity) == AcEdJig::kNormal) {
		AcDbObjectId id;
		pBlockTableRecord->appendAcDbEntity(id, pNewEntity);

		// �������尴ť
		createRegretButton(pNewEntity, pBlockTableRecord);

		// �رն���
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
	// ��ȡ����¼��ģ�Ϳռ䣩
	AcDbBlockTableRecord *pBlockTableRecord;
	getBlockTableRecord(pBlockTableRecord);

	// �����Զ���Բ����,������Jig
	Cchess *pNewEntity = new Cchess(r, color);
	pNewEntity->setDatabaseDefaults();
	CchessJig chessJig=CchessJig();

	// ���������
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

// x��y���ڻ�ȡ�Ǹ����ڶ�ά�����е�λ�á�
AcGePoint3d findClosePoint(AcGePoint3d point,CchessBoard* chessBoard,int& x, int& y) {
	// ��ȡ������Ϣ
	int row = chessBoard->getRow();
	int column = chessBoard->getColumn();
	double width = chessBoard->getWidth();
	double height = chessBoard->getHeight();
	AcGePoint3d center = chessBoard->getCenter();

	// ���� grid �Ĵ�С
	double cellWidth = width / column;
	double cellHeight = height / row;

	// ��ʼ����
	AcGePoint3d closestPoint;
	double minDistance = DBL_MAX;

	// ��ʼѭ��������ĵ�
	for (int i = 0; i <= row; ++i) {
		for (int j = 0; j <= column; ++j) {
			// ������һ�ַ��ʵĵ�
			AcGePoint3d gridPoint = center + AcGeVector3d((j - column / 2.0) * cellWidth, (i - row / 2.0) * cellHeight, 0);

			// ����������õ�ľ��룬�ж��Ƿ���Ҫ�޸�
			double distance = point.distanceTo(gridPoint);
			if (distance < minDistance) {
				closestPoint = gridPoint;
				minDistance = distance;
				x = i;
				y = j;
			}
		}
	}

	// ��������ĵ�
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

	// ��ȡȫ���ֵ�
	Acad::ErrorStatus es = pDb->getNamedObjectsDictionary(pNameList, AcDb::kForWrite);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n�޷���ȡȫ���ֵ�"));
		return;
	}

	// ��ȡ�򴴽��Զ����ֵ�
	AcDbDictionary* pReactDict = nullptr;
	es = pNameList->getAt(_T("ReactorDictionary"), (AcDbObject*&)pReactDict, AcDb::kForWrite);
	if (es == Acad::eKeyNotFound) {
		pReactDict = new AcDbDictionary();
		AcDbObjectId reactDictId;
		es = pNameList->setAt(_T("ReactorDictionary"), pReactDict, reactDictId);
		if (es != Acad::eOk) {
			acutPrintf(_T("\n�޷������Զ����ֵ�"));
			pReactDict->close();
			pNameList->close();
			return;
		}
	}
	else if (es != Acad::eOk) {
		acutPrintf(_T("\n�޷���ȡ�Զ����ֵ䣬����Ϊ: %d"), es);
		pNameList->close();
		return;
	}
	pNameList->close();

	// ��ȡChessBoard���ġ��������ڳ�ʼ����Ӧ��
	AcGePoint3d chessBoardCenter = pChessBoard->getCenter();
	double chessBoardWidth = pChessBoard->getWidth();
	double chessBoardHeight = pChessBoard->getHeight();

	// ������Ӧ�����󣬲���ʼ��
	CmyReactor* pReactor = new CmyReactor();
	if (!pReactor) {
		acutPrintf(_T("\n�޷�������Ӧ������"));
		return;
	}
	pReactor->eLinkage(chessId);
	pReactor->setChessBoardCenter(chessBoardCenter);
	pReactor->setChessBoardWidth(chessBoardWidth);
	pReactor->setChessBoardHeight(chessBoardHeight);

	// ���÷�Ӧ�����ֵ��е�����
	AcString reactorName = _T("reactor_");
	strConcat(chessId, reactorName);

	// �ѷ�Ӧ��������ӵ���Ӧ���ֵ䣬����ȡ�÷�Ӧ����Id
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
			acutPrintf(_T("\n�޷�����Ӧ������洢����Ӧ���ֵ��У�����Ϊ: %d"), es);
			return;
		}
	}
	else {
		delete pReactor;
		pReactDict->close();
	}

	// ����Ӧ���������ΪChessBoard�ĳ־��Է�Ӧ��
	es = pChessBoard->addPersistentReactor(reactId);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n��ʧ�ܣ�����Ϊ: %d"), es);
	}
}

void removeReactor(CchessBoard* pChessBoard, AcDbObjectId chessId) {
	AcDbDatabase* pDb = acdbHostApplicationServices()->workingDatabase();
	AcDbDictionary* pNameList = nullptr;

	// ��ȡȫ���ֵ�
	Acad::ErrorStatus es = pDb->getNamedObjectsDictionary(pNameList, AcDb::kForWrite);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n�޷���ȡȫ���ֵ�"));
		return;
	}

	// ��ȡ�Զ����ֵ�
	AcDbDictionary* pReactDict = nullptr;
	es = pNameList->getAt(_T("ReactorDictionary"), (AcDbObject*&)pReactDict, AcDb::kForWrite);
	pNameList->close();
	if (es == Acad::eKeyNotFound) {
		acutPrintf(_T("\n�Զ����ֵ䲻����"));
		return;
	}
	else if (es != Acad::eOk) {
		acutPrintf(_T("\n�޷���ȡ�Զ����ֵ䣬����Ϊ: %d"), es);
		return;
	}

	// ���÷�Ӧ�����ֵ��е�����
	AcString reactorName = _T("reactor_");
	strConcat(chessId, reactorName);

	// ��ȡ��Ӧ������ID
	AcDbObjectId reactId;
	es = pReactDict->getAt(reactorName, reactId);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n�޷��ҵ���Ӧ�����󣬴���Ϊ: %d"), es);
		pReactDict->close();
		return;
	}

	// �򿪷�Ӧ������
	CmyReactor* pReactor = nullptr;
	es = acdbOpenObject(pReactor, reactId, AcDb::kForWrite);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n�޷��򿪷�Ӧ�����󣬴���Ϊ: %d"), es);
		pReactDict->close();
		return;
	}

	// ��ChessBoard�������Ƴ��־÷�Ӧ��
	es = pChessBoard->removePersistentReactor(reactId);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n��ChessBoard�������Ƴ��־÷�Ӧ��ʧ�ܣ�����Ϊ: %d"), es);
	}

	// ���Զ����ֵ���ɾ����Ӧ������
	es = pReactDict->remove(reactorName);
	if (es != Acad::eOk) {
		acutPrintf(_T("\n���Զ����ֵ���ɾ����Ӧ������ʧ�ܣ�����Ϊ: %d"), es);
	}

	// ɾ����Ӧ������
	es = pReactor->erase();
	if (es != Acad::eOk) {
		acutPrintf(_T("\nɾ����Ӧ������ʧ�ܣ�����Ϊ: %d"), es);
	}

	pReactor->close();
	pReactDict->close();
}



void changeColor(CchessBoard* pChessBoard, int x, int y, int dx, int dy, int positiveStep,int negativeStep) {
	// ���ά�ȵ����������
	std::vector<std::vector<AcDbObjectId>> chessIds = pChessBoard->getChessIds();
	for (int step = 0; step <= positiveStep; ++step) {
		// ��ȡ����Id
		int newX = x + step * dx;
		int newY = y + step * dy;
		AcDbObjectId chessId = chessIds[newX][newY];

		// ͨ��Id���´��������
		AcDbEntity* chessEnt;
		if (acdbOpenAcDbEntity(chessEnt, chessId, AcDb::kForWrite) != Acad::eOk) {
			acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessId.asOldId());
			return;
		}
		Cchess* chess = (Cchess*)chessEnt;

		// �޸�������ɫ
		chess->setColor(success);
		chess->recordGraphicsModified();
		chess->close();
	}

	// �򷴷������
	for (int step = 1; step <= negativeStep; ++step) {
		int newX = x - step * dx;
		int newY = y - step * dy;
		AcDbObjectId chessId = chessIds[newX][newY];

		// ͨ��Id���´��������
		AcDbEntity* chessEnt;
		if (acdbOpenAcDbEntity(chessEnt, chessId, AcDb::kForWrite) != Acad::eOk) {
			acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessId.asOldId());
			return;
		}
		Cchess* chess = (Cchess*)chessEnt;

		// �޸�������ɫ
		chess->setColor(success);
		chess->recordGraphicsModified();
		chess->close();
	}
	acedUpdateDisplay();
}

// ˼·��x,y�����жϵ�ǰ�����ĸ�λ�ã����������������ж��Ƿ�����������һ��
bool isWin(CchessBoard* pChessBoard, int color, int x, int y) {
	// ��ȡ��������Ϣ�͵�ǰ���ӵ���ɫ
	std::vector<std::vector<int>> grids = pChessBoard->getGrids();
	if (color == empty) {
		return false;
	}

	// ���ĸ�������ƫ����
	std::vector<std::pair<int, int>> directions = {
		{0, 1}, {1, 0}, {1, 1}, {1, -1}
	};

	// ��ȡ�С�����
	int row = grids.size();
	int column = grids[0].size();

	// ��ʼ���
	for (auto dir : directions) {
		int count = 1;

		// ÿ���ߵĲ���
		int dx = dir.first;
		int dy = dir.second;

		// �����������ߵĲ���������ʵ�ֶ���Ч���ĺ��������Լ򻯴�����
		int positiveStep = 0;
		int negativeStep = 0;

		// ���ά�ȵ���������
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

		// �򷴷�����
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
			// ʵ�ֶ���Ч��
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

	// ��ȡ����¼��ģ�Ϳռ䣩
	AcDbBlockTable* pBlockTable;
	acdbHostApplicationServices()->workingDatabase()->getSymbolTable(pBlockTable, AcDb::kForRead);

	AcDbBlockTableRecord* pBlockTableRecord;
	pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord, AcDb::kForWrite);
	pBlockTable->close();

	// ���ı�������ӵ�ģ�Ϳռ�
	AcDbObjectId textId;
	pBlockTableRecord->appendAcDbEntity(textId, pText);
	pText->close();
	pBlockTableRecord->close();
}

// �ж�һ�����Ƿ��ھ�����
bool isPointInPolygon(AcGePoint3d p, AcGePoint3d* vertices, int vertexCount) {
	if (vertexCount != 4) {
		return false;
	}

	// ��������
	AcGeVector3d v1 = vertices[1] - vertices[0];
	AcGeVector3d v2 = vertices[2] - vertices[1];
	AcGeVector3d v3 = vertices[3] - vertices[2];
	AcGeVector3d v4 = vertices[0] - vertices[3];

	// �㵽���������
	AcGeVector3d vp1 = p - vertices[0];
	AcGeVector3d vp2 = p - vertices[1];
	AcGeVector3d vp3 = p - vertices[2];
	AcGeVector3d vp4 = p - vertices[3];

	// ������
	double c1 = v1.crossProduct(vp1).z;
	double c2 = v2.crossProduct(vp2).z;
	double c3 = v3.crossProduct(vp3).z;
	double c4 = v4.crossProduct(vp4).z;

	// �жϵ��Ƿ��ھ����ڲ�
	if ((c1 >= 0 && c2 >= 0 && c3 >= 0 && c4 >= 0) || (c1 <= 0 && c2 <= 0 && c3 <= 0 && c4 <= 0))
		return true;
	else
		return false;
}


void playGame() {
	// ��������
	AcDbObjectId chessBoardId = createChessBoard();

	// ͨ��Id���������
	AcDbEntity* chessBoardEnt;
	if (acdbOpenAcDbEntity(chessBoardEnt, chessBoardId, AcDb::kForWrite) != Acad::eOk) {
		acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessBoardId.asOldId());
		return;
	}

	// ��ȡ������Ϣ
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

	// ȷ�����Ӱ뾶
	double radius = min(cellWidth, cellHeight) / 2.5;
	int chessColor = black;

	// ��ʼ��Ϸ
	for (int i = 0; i<256; i++) {
		// ��������
		AcDbObjectId chessId = createChess(radius,chessColor);
		if (chessId == nullptr) {
			acutPrintf(_T("��������ʧ��"), chessId.asOldId());
			return;
		}

		// ͨ��Id���´��������
		AcDbEntity* chessEnt;
		if (acdbOpenAcDbEntity(chessEnt, chessId, AcDb::kForWrite) != Acad::eOk) {
			acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessId.asOldId());
			break;
		}
		Cchess* chess = (Cchess*)chessEnt;

		// ��ȡ�������ĵ�λ��
		AcGePoint3d chessCenter = chess->getCenter();

		// �������ĵ㲻�������ڣ���������
		if (!isPointInPolygon(chessCenter, backgroundPoints, 4)) {
			// �ͷŵ�ǰ����ʵ��
			chess->erase();
			chess->close();
			--i;

			// ��ֹ�ڵ�һö����
			if (i == 0) {
				acutPrintf(_T("��һö�����޷�����\n"));
				continue;
			}

			// ��ȡ��ö���ŵ�����
			AcDbObjectId chessToDeleteId = chessBoard->getCurrentChessId();
			AcDbEntity* chessToDeleteEnt;
			if (acdbOpenAcDbEntity(chessToDeleteEnt, chessToDeleteId, AcDb::kForWrite) != Acad::eOk) {
				acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessId.asOldId());
				break;
			}

			// ɾ����Ӧ�ķ�Ӧ��
			removeReactor(chessBoard, chessToDeleteId);

			// �ӿ������ɾ��
			Cchess* chessToDelete = (Cchess*)chessToDeleteEnt;
			chessToDelete->erase();
			chessToDelete->close();

			// �ָ�����״��
			chessBoard->regretChess();

			// ����������ɫ������ѭ��
			chessColor = chessColor % 2 + 1;
			--i;
			continue;
		}

		// �ҵ����������������λ��
		int x = 0, y = 0;
		AcGePoint3d pt = findClosePoint(chessCenter, chessBoard, x, y);

		// �жϵ�ǰλ���Ƿ��Ѵ�������
		if (chessBoard->getStatus(x, y) != empty) {
			chess->erase();
			chess->close();
			--i;
			continue;
		}

		// ���������ĵ㣬��������������
		chess->setCenter(pt);
		chessBoard->setGrids(x, y, chessColor);
		chessBoard->setChessIds(x, y, chessId);

		// �ر�����
		chess->close();

		// ��������ӵ�ChessBoard�ķ�Ӧ������
		addReactor(chessBoard, chessId);

		// �жϵ�ǰ�Ƿ�ʤ��
		if (isWin(chessBoard,chessColor, x, y)) {
			// ����ʤ�����ֵ�λ�á�����
			AcGePoint3d textPoint = chessBoard->getCenter();
			textPoint.y =textPoint.y+ height / 2 + 20;
			AcString message;
			message.format(L"��� %d ��ʤ\n", chessColor);

			// ��ʤ�����ִ򵽻�����
			printToScreen(message, textPoint,20);
			break;
		}

		// ����������ɫ������ѭ��
		chessColor = chessColor % 2 + 1;			
	}

	// �ر�����
	chessBoard->close();
}
