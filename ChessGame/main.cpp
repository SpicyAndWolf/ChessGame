#include "pch.h"
#include "main.h"

void playGame() {
	// ��������
	AcDbObjectId chessBoardId = createChessBoard();

	// ͨ��chessBoardId���������
	AcDbEntity* chessBoardEnt;
	if (acdbOpenAcDbEntity(chessBoardEnt, chessBoardId, AcDb::kForWrite) != Acad::eOk) {
		acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessBoardId.asOldId());
		return;
	}
	CchessBoard* chessBoard = (CchessBoard*)chessBoardEnt;

	// �������尴ť
	AcDbObjectId regretButtonId;
	createRegretButton(chessBoard, regretButtonId);

	// ͨ��regretButtonId�������ť
	AcDbEntity* regretButtonEnt;
	if (acdbOpenAcDbEntity(regretButtonEnt, regretButtonId, AcDb::kForWrite) != Acad::eOk) {
		acutPrintf(_T("Failed to open entity with Object ID: %ld\n"), chessBoardId.asOldId());
		return;
	}
	AcDbPolyline *regretButton = (AcDbPolyline *)regretButtonEnt;

	// ��ȡ������Ϣ
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

		// �������ĵ㲻�������ڣ������ӻ��߻���
		if (!isPointInPolygon(chessCenter, backgroundPoints, 4)) {
			// �ͷŵ�ǰ����ʵ��
			chess->erase();
			chess->close();
			--i;

			// �ж��ǲ��ǵ㵽�˻��壬��������������һ��ѭ��
			if (!isPointInRectangle(chessCenter, regretButton)) {
				continue;
			}

			// ���岢��׽����
			if (regret(i, chessBoard, chessColor)) {
				continue;
			}
			else {
				break;
			}

		}

		// ����
		//�����ҵ����������������λ��
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
			textPoint.y =textPoint.y+ height / 2+height/20 + 20;
			textPoint.x = textPoint.x - 70;
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
	chessBoard->setStatus(0); //��������Ϊ���״̬
	chessBoard->recordGraphicsModified(1);
	chessBoard->close();
	regretButton->close();
	acedRedraw(NULL, 1);
}