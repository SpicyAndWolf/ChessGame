#include "pch.h"
#include "chessBoard.h"


//----------------------------------------------------------------------------
Adesk::UInt32 CchessBoard::kCurrentVersionNumber = 1;

//----------------------------------------------------------------------------
//---- runtime definition
ACRX_DXF_DEFINE_MEMBERS(
	CchessBoard, AcDbEntity,
	AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
	AcDbProxyEntity::kNoOperation, KEYBOARD, KEYBOARDAPP
)

//----------------------------------------------------------------------------
//---- construct & destruct

CchessBoard::CchessBoard() {
	center = AcGePoint3d(0.0, 0.0, 0.0);
	width = 100;
	height = 100;
	row = 14;
	column = 14;

	// ����grids
	std::vector<int> arr;
	for (int i = 0; i <= column; i++) {
		arr.push_back(0);
	}
	for (int i = 0; i <= row; i++) {
		grids.push_back(arr);
	}

	// ����chessIds
	std::vector<AcDbObjectId> arrId;
	for (int i = 0; i <= column; i++) {
		arrId.push_back(0);
	}
	for (int i = 0; i <= row; i++) {
		chessIds.push_back(arrId);
	}
}

CchessBoard::~CchessBoard() {}

void CchessBoard::initializeGrid(int r, int c)
{
	row = r;
	column = c;
	grids.resize((row+1), std::vector<int>((column+1), 0)); // ��ʼ�����и���Ϊ0
	chessIds.resize((row + 1), std::vector<AcDbObjectId>((column + 1), 0)); // ��ʼ�����и���Ϊ0
}

void CchessBoard::setCenter(ZcGePoint3d pt) {
	center = pt;
}
void CchessBoard::setWidth(double w) {
	width = w;
}
void CchessBoard::setHeight(double h) {
	height = h;
}
void CchessBoard::setGrids(int x, int y, int color) {
	// �߽���
	if (x < 0 || x > row || y < 0 || y > column) {
		acutPrintf(_T("\n��Ч������λ��"));
		return; 
	}

	// ���浱ǰ״̬,���ڻ���
	if (stepCountA == stepCountB)
		saveState();

	// �޸�����ֵ
	++stepCountA;
	grids[x][y] = color;
}

void CchessBoard::setChessIds(int x,int y, AcDbObjectId id) {
	// �߽���
	if (x < 0 || x > row || y < 0 || y > column) {
		acutPrintf(_T("\n��Ч������λ��"));
		return;
	}

	// ���浱ǰ״̬,���ڻ���
	if (stepCountA == stepCountB)
		saveState();

	// �޸�����ֵ
	chessIds[x][y] = id;
	++stepCountB;
	currentChessId = id;
}

void CchessBoard::saveState() {
	// ������ǰ����״̬�ĸ���
	ChessBoardState state;
	state.grids = grids;
	state.chessIds = chessIds;
	state.chessId = currentChessId;

	// ���������浽��ʷ��¼��
	history.push(state);
}

void CchessBoard::regretChess() {
	// ����Ƿ�����ʷ��¼
	if (history.empty()) {
		acutPrintf(_T("\n�޷�ִ�л��������û����ʷ��¼��"));
		return;
	}

	// �ָ�����һ��״̬
	ChessBoardState state = history.top();
	grids = state.grids;
	chessIds = state.chessIds;
	currentChessId = state.chessId;

	// �Ƴ�ջ��Ԫ��
	history.pop();
	stepCountA = stepCountB;
}


ZcGePoint3d CchessBoard::getCenter() {
	return center;
}
double CchessBoard::getWidth() {
	return width;
}
double CchessBoard::getHeight() {
	return height;
}
int CchessBoard::getRow() {
	return row;
}int CchessBoard::getColumn() {
	return column;
}
int CchessBoard::getStatus(int x, int y) {
	return grids[x][y];
}
std::vector<std::vector<int>> CchessBoard::getGrids() {
	return grids;
}
std::vector<std::vector<AcDbObjectId>> CchessBoard::getChessIds() {
	return chessIds;
}
AcDbObjectId CchessBoard::getCurrentChessId() {
	return currentChessId;
}


//----------------------------------------------------------------------------
//----- AcDbObject protocols
//---- Dwg Filing protocol
Acad::ErrorStatus CchessBoard::dwgOutFields(AcDbDwgFiler *pFiler) const {
	assertReadEnabled();
	Acad::ErrorStatus es = AcDbEntity::dwgOutFields(pFiler);
	if (es != Acad::eOk)
		return (es);
	if ((es = pFiler->writeUInt32(CchessBoard::kCurrentVersionNumber)) != Acad::eOk)
		return (es);

	//----- Output params
	pFiler->writePoint3d(center);
	pFiler->writeDouble(height);
	pFiler->writeDouble(width);
	pFiler->writeInt32(row);
	pFiler->writeInt32(column);

	// ��� grids ֵ
	for (int i = 0; i <= row; ++i) {
		for (int j = 0; j <= column; ++j) {
			pFiler->writeInt32(grids[i][j]);
		}
	}

	// ���chessIdsֵ
	for (int i = 0; i <= row; ++i) {
		for (int j = 0; j <= column; ++j) {
			pFiler->writeItem((AcDbSoftPointerId&)chessIds[i][j]);
		}
	}

	return (pFiler->filerStatus());
}

Acad::ErrorStatus CchessBoard::dwgInFields(AcDbDwgFiler * pFiler) {
	assertWriteEnabled();
	Acad::ErrorStatus es = AcDbEntity::dwgInFields(pFiler);
	if (es != Acad::eOk)
		return (es);
	Adesk::UInt32 version = 0;
	if ((es = pFiler->readUInt32(&version)) != Acad::eOk)
		return (es);
	if (version > CchessBoard::kCurrentVersionNumber)
		return (Acad::eMakeMeProxy);

	//----- Read params
	pFiler->readPoint3d(&center);
	pFiler->readDouble(&height);
	pFiler->readDouble(&width);

	// ��ȡrow��column
	int r = 0, c = 0;
	pFiler->readInt32(&r);
	pFiler->readInt32(&c);

	// ��ʼ��row��column����0 grids��chessIds
	initializeGrid(r, c);

	// ��gridsֵ
	for (int i = 0; i <= row; ++i) {
		for (int j = 0; j <= column; ++j) {
			int value = 0;
			pFiler->readInt32(&value);
			grids[i][j] = value;
		}
	}

	// ��chessIds
	for (int i = 0; i <= row; ++i) {
		for (int j = 0; j <= column; ++j) {
			AcDbObjectId value;
			pFiler->readItem((AcDbSoftPointerId*)&value);
			chessIds[i][j] = value;
		}
	}

	return (pFiler->filerStatus());
}

//----- AcDbEntity protocols
//----- Graphics protocol
Acad::ErrorStatus CchessBoard::subTransformBy(const AcGeMatrix3d& xform) {
	assertWriteEnabled();
	center = center.transformBy(xform);
	return Acad::eOk;
}

// �е�
Acad::ErrorStatus CchessBoard::subGetGripPoints(AcGePoint3dArray& gripPoints, AcDbIntArray & osnapModes, AcDbIntArray & geomIds) const {
	assertReadEnabled();
	// �����ε��ĸ��Ǻ����ĵ���Ϊ�е�
	gripPoints.append(center + AcGeVector3d(-width / 2, -height / 2, 0));
	gripPoints.append(center + AcGeVector3d(width / 2, -height / 2, 0));
	gripPoints.append(center + AcGeVector3d(width / 2, height / 2, 0));
	gripPoints.append(center + AcGeVector3d(-width / 2, height / 2, 0));
	gripPoints.append(center);

	return Acad::eOk;
}
Acad::ErrorStatus CchessBoard::subMoveGripPointsAt(const AcDbIntArray & indices, const AcGeVector3d& offset) {
	assertWriteEnabled();
	for (int i = 0; i < indices.length(); i++) {
		int index = indices.at(i);

		switch (index) {
		case 0: // Bottom-left corner
			center += offset / 2;
			width -= offset.x;
			height -= offset.y;
			break;
		case 1: // Bottom-right corner
			center += AcGeVector3d(offset.x / 2, -offset.y / 2, 0);
			width += offset.x;
			height -= offset.y;
			break;
		case 2: // Top-right corner
			center += offset / 2;
			width += offset.x;
			height += offset.y;
			break;
		case 3: // Top-left corner
			center += AcGeVector3d(-offset.x / 2, offset.y / 2, 0);
			width -= offset.x;
			height += offset.y;
			break;
		case 4: // Center point
			center += offset;
			break;
		}
	}

	// ȷ����Ⱥ͸߶�Ϊ��ֵ
	if (width < 0) {
		width = -width;
		center.x += width;
	}
	if (height < 0) {
		height = -height;
		center.y += height;
	}

	return Acad::eOk;
}

// ��׽
Acad::ErrorStatus CchessBoard::subGetOsnapPoints(
	AcDb::OsnapMode osnapMode,
	Adesk::GsMarker gsSelectionMark,
	const AcGePoint3d& pickPoint,
	const AcGePoint3d& lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray& snapPoints,
	AcDbIntArray & geomIds
) const {
	assertReadEnabled();
	// ����ÿ�����ӵĿ�Ⱥ͸߶�
	double cellWidth = width / column;
	double cellHeight = height / row;

	// ������̸��������߶ν���
	switch (osnapMode) {
	case AcDb::kOsModeEnd:
		for (int i = 0; i <= row; ++i) {
			for (int j = 0; j <= column; ++j) {
				AcGePoint3d snapPoint = center + AcGeVector3d(
					-j * cellWidth + width / 2,
					-i * cellHeight + height / 2,
					0
				);
				snapPoints.append(snapPoint);
			}
		}
		break;
	default:
		break;
	}
	return Acad::eOk;
}

//----------------------------------------------------------------------------
//----- AcDbEntity protocols
Adesk::Boolean CchessBoard::subWorldDraw(AcGiWorldDraw * mode) {
	assertReadEnabled();

	// ����ĸ�����
	AcGePoint3d pt1 = center + AcGeVector3d(-width / 2, -height / 2, 0); // Bottom-left corner
	AcGePoint3d pt2 = center + AcGeVector3d(width / 2, -height / 2, 0); // Bottom-right corner
	AcGePoint3d pt3 = center + AcGeVector3d(width / 2, height / 2, 0); // Top-right corner
	AcGePoint3d pt4 = center + AcGeVector3d(-width / 2, height / 2, 0); // Top-left corner

	// �������̱�����ɫ
	mode->subEntityTraits().setFillType(kAcGiFillAlways);
	AcCmEntityColor backgroundColor;
	backgroundColor.setRGB(181, 135, 86);
	mode->subEntityTraits().setTrueColor(backgroundColor);

	// �������̱���
	AcGePoint3d backgroundPoints[4] = {
		pt1 + AcGeVector3d(-width / 20,-height / 20, 0),
		pt2 + AcGeVector3d(width / 20,-height / 20, 0),
		pt3 + AcGeVector3d(width / 20,height / 20, 0),
		pt4 + AcGeVector3d(-width / 20,height / 20, 0)
	};
	mode->geometry().polygon(4, backgroundPoints);

	// �������̱߽���ɫ
	mode->subEntityTraits().setFillType(kAcGiFillNever);
	AcCmEntityColor borderColor;
	borderColor.setRGB(0, 0, 0);
	mode->subEntityTraits().setTrueColor(borderColor);

	// �������̱߽�
	AcGePoint3d points[5] = { pt1, pt2, pt3, pt4, pt1 };
	mode->geometry().polyline(5, points);

	// ����ÿ�����ӵĿ�Ⱥ͸߶�
	double cellWidth = width / column;
	double cellHeight = height / row;

	// �����ڲ�����������ɫ
	AcCmEntityColor lineColor;
	lineColor.setRGB(0, 0, 0); // ��ɫ����
	mode->subEntityTraits().setTrueColor(lineColor);

	// �����ڲ�����
	for (int i = 1; i < row; ++i) {
		AcGePoint3d start = pt1 + AcGeVector3d(0, i * cellHeight, 0);
		AcGePoint3d end = pt2 + AcGeVector3d(0, i * cellHeight, 0);
		AcGePoint3d points_temp[2] = { start, end };
		mode->geometry().polyline(2, points_temp);
	}

	for (int j = 1; j < column; ++j) {
		AcGePoint3d start = pt1 + AcGeVector3d(j * cellWidth, 0, 0);
		AcGePoint3d end = pt4 + AcGeVector3d(j * cellWidth, 0, 0);
		AcGePoint3d points_temp[2] = { start, end };
		mode->geometry().polyline(2, points_temp);
	}

	// ���ĵ�Ӵ�
	for (int i = 10; i < 30; i++) {
		mode->geometry().circle(center, cellWidth / i, AcGeVector3d::kZAxis);
	}

	return (AcDbEntity::subWorldDraw(mode));
}

Adesk::UInt32 CchessBoard::subSetAttributes(AcGiDrawableTraits * traits) {
	assertReadEnabled();
	return (AcDbEntity::subSetAttributes(traits));
}


