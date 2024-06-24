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
	row = 15;
	column = 15;

	// 设置grids
	std::vector<int> arr;
	for (int i = 0; i <= column; i++) {
		arr.push_back(0);
	}
	for (int i = 0; i <= row; i++) {
		grids.push_back(arr);
	}

	// 设置chessIds
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
	grids.resize((row+1), std::vector<int>((column+1), 0)); // 初始化所有格子为0
	chessIds.resize((row + 1), std::vector<AcDbObjectId>((column + 1), 0)); // 初始化所有格子为0
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
void CchessBoard::setRow(int r) {
	row = r;
}
void CchessBoard::setColumn(int c) {
	column = c;
}
void CchessBoard::setGrids(int x, int y, int status) {
	// 边界检查
	if (x < 0 || x > row || y < 0 || y > column) {
		acutPrintf(_T("\n无效的棋子位置"));
		return; 
	}

	// 实现部分Undo
	assertWriteEnabled(false);
	AcDbDwgFiler *pFiler = NULL;
	if ((pFiler = undoFiler()) != NULL) {
		undoFiler()->writeAddress(CchessBoard::desc());//导出实体标记
		undoFiler()->writeItem((Adesk::Int16)kGrids);//导出属性标记

		// 导出数组内容
		undoFiler()->writeInt32(row);
		undoFiler()->writeInt32(column);
		for (int i = 0; i <= row; i++) {
			for (int j = 0; j <= column; j++) {
				undoFiler()->writeInt32(grids[x][y]);
			}
		}
	}

	// 修改数据值
	grids[x][y] = status;
}

void CchessBoard::setChessIds(int x,int y, AcDbObjectId id) {
	// 边界检查
	if (x < 0 || x > row || y < 0 || y > column) {
		acutPrintf(_T("\n无效的棋子位置"));
		return;
	}

	// 实现部分Undo
	assertWriteEnabled(false);
	AcDbDwgFiler *pFiler = NULL;
	if ((pFiler = undoFiler()) != NULL) {
		undoFiler()->writeAddress(CchessBoard::desc());//导出实体标记
		undoFiler()->writeItem((Adesk::Int16)kChessIds);//导出属性标记

		// 导出数组内容
		undoFiler()->writeInt32(row);
		undoFiler()->writeInt32(column);
		for (int i = 0; i <= row; i++) {
			for (int j = 0; j <= column; j++) {
				undoFiler()->writeItem((AcDbSoftPointerId&)chessIds[x][y]);
			}
		}
	}

	// 修改数据值
	chessIds[x][y] = id;
}

Acad::ErrorStatus CchessBoard::applyPartialUndo(AcDbDwgFiler* undoFiler, AcRxClass* classObj)
{
	if (classObj != CchessBoard::desc())
		return AcDbEntity::applyPartialUndo(undoFiler, classObj);
	Adesk::Int16 shortCode;
	undoFiler->readItem(&shortCode);
	PartialUndoCode code = (PartialUndoCode)shortCode;
	int num;
	switch (code) {
		case kGrids:
			undoFiler->readInt32(&row);
			undoFiler->readInt32(&column);
			for (int i = 0; i < row; i++) {
				for (int j = 0; j < column; j++) {
					undoFiler->readInt32(&num);
					grids[i][j] = num;
				}
			}
			break;
		case kChessIds:
			undoFiler->readInt32(&row);
			undoFiler->readInt32(&column);
			for (int i = 0; i < row; i++) {
				for (int j = 0; j < column; j++) {
					AcDbObjectId value;
					undoFiler->readItem((AcDbSoftPointerId*)&value);
					chessIds[i][j] = value;
				}
			}
			break;
		default:
			assert(Adesk::kFalse);
			break;
	}
	return Acad::eOk;
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

	// 输出 grids 值
	for (int i = 0; i <= row; ++i) {
		for (int j = 0; j <= column; ++j) {
			pFiler->writeInt32(grids[i][j]);
		}
	}

	// 输出chessIds值
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

	// 读取row和column
	int r = 0, c = 0;
	pFiler->readInt32(&r);
	pFiler->readInt32(&c);

	// 初始化row、column，清0 grids和chessIds
	initializeGrid(r, c);

	// 读grids值
	for (int i = 0; i <= row; ++i) {
		for (int j = 0; j <= column; ++j) {
			int value = 0;
			pFiler->readInt32(&value);
			grids[i][j] = value;
		}
	}

	// 读chessIds
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

// 夹点
Acad::ErrorStatus CchessBoard::subGetGripPoints(AcGePoint3dArray& gripPoints, AcDbIntArray & osnapModes, AcDbIntArray & geomIds) const {
	assertReadEnabled();
	// 将矩形的四个角和中心点作为夹点
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

	// 确保宽度和高度为正值
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

// 捕捉
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
	// 计算每个格子的宽度和高度
	double cellWidth = width / column;
	double cellHeight = height / row;

	// 添加棋盘格内所有线段交点
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

	// 求出四个顶点
	AcGePoint3d pt1 = center + AcGeVector3d(-width / 2, -height / 2, 0); // Bottom-left corner
	AcGePoint3d pt2 = center + AcGeVector3d(width / 2, -height / 2, 0); // Bottom-right corner
	AcGePoint3d pt3 = center + AcGeVector3d(width / 2, height / 2, 0); // Top-right corner
	AcGePoint3d pt4 = center + AcGeVector3d(-width / 2, height / 2, 0); // Top-left corner

	// 用折线绘制棋盘边界
	AcGePoint3d points[5] = { pt1, pt2, pt3, pt4, pt1 };
	mode->geometry().polyline(5, points);

	// 计算每个格子的宽度和高度
	double cellWidth = width / column;
	double cellHeight = height / row;

	// 绘制内部格子
	for (int i = 0; i <= row; ++i) {
		AcGePoint3d start = pt1 + AcGeVector3d(0, i * cellHeight, 0);
		AcGePoint3d end = pt2 + AcGeVector3d(0, i * cellHeight, 0);
		AcGePoint3d points_temp[2] = { start,end };
		mode->geometry().polyline(2, points_temp);
	}

	for (int j = 0; j <= column; ++j) {
		AcGePoint3d start = pt1 + AcGeVector3d(j * cellWidth, 0, 0);
		AcGePoint3d end = pt4 + AcGeVector3d(j * cellWidth, 0, 0);
		AcGePoint3d points_temp[2] = { start,end };
		mode->geometry().polyline(2, points_temp);
	}

	return (AcDbEntity::subWorldDraw(mode));
}

Adesk::UInt32 CchessBoard::subSetAttributes(AcGiDrawableTraits * traits) {
	assertReadEnabled();
	return (AcDbEntity::subSetAttributes(traits));
}


