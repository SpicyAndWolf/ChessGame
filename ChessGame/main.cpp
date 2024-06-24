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
	// ��ȡ����¼��ģ�Ϳռ䣩
	AcDbBlockTableRecord *pBlockTableRecord;
	getBlockTableRecord(pBlockTableRecord);

	// �������̶���
	CchessBoard *pNewEntity = new CchessBoard();
	pNewEntity->setDatabaseDefaults();

	// �������ġ�����
	ads_point pt;
	double width, height;
	acedGetPoint(NULL, _T("\n�������ģ�"), pt);
	acedGetReal(_T("\n�����ȣ�"), &width);
	acedGetReal(_T("\n����߶ȣ�"), &height);

	// ���ö�������
	AcGePoint3d center = AcGePoint3d(pt[0], pt[1], pt[2]);
	pNewEntity->setCenter(center);
	pNewEntity->setWidth(width);
	pNewEntity->setHeight(height);

	// �������̵�ģ�Ϳռ�
	AcDbObjectId objId;
	pBlockTableRecord->appendAcDbEntity(objId,pNewEntity);
	pBlockTableRecord->close();
	pNewEntity->close();
	
	return objId;
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
	if (es == Acad::eOk) {
		acutPrintf(_T("\n�󶨳ɹ�"));
	}
	else {
		acutPrintf(_T("\n��ʧ�ܣ�����Ϊ: %d"), es);
	}
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

	// ȷ�����Ӱ뾶
	double radius = min(cellWidth, cellHeight) / 2.5;
	int chessColor = black;

	for (int i = 0; i < 10; i++) {
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
			return;
		}
		Cchess* chess = (Cchess*)chessEnt;

		// ��ȡ�������ĵ�λ��
		AcGePoint3d center = chess->getCenter();

		// �ҵ����������������λ�ã����������ĵ�
		int x = 0, y = 0;
		AcGePoint3d pt = findClosePoint(center, chessBoard, x, y);
		chess->setCenter(pt);
		chessBoard->setGrids(x, y, chessColor);

		// �ر�����
		chess->close();

		// ��������ӵ�ChessBoard�ķ�Ӧ������
		addReactor(chessBoard, chessId);
		chessColor = chessColor % 2 + 1;
	}

	// �ر�����
	chessBoard->close();

}