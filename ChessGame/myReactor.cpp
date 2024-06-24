#include "pch.h"
#include "myReactor.h"
#include "dbproxy.h"


//----------------------------------------------------------------------------
Adesk::UInt32 CmyReactor::kCurrentVersionNumber = 1 ;

//----------------------------------------------------------------------------
//---- runtime definition
ACRX_DXF_DEFINE_MEMBERS (
	CmyReactor, AcDbObject,
	AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent, 
	AcDbProxyEntity::kNoOperation, MYREACTOR, MYREACTORAPP
)

//----------------------------------------------------------------------------
//---- construct & destruct

CmyReactor::CmyReactor(){}

CmyReactor::~CmyReactor(){}

void CmyReactor::setChessBoardCenter(AcGePoint3d newPt) {
	chessBoardCenter_old = newPt;
}
void CmyReactor::setChessBoardWidth(double w) {
	chessBoardWidth_old = w;
}
void CmyReactor::setChessBoardHeight(double h) {
	chessBoardHeight_old = h;
}

//----------------------------------------------------------------------------
//----- AcDbObject protocols
//---- Dwg Filing protocol
Acad::ErrorStatus CmyReactor::dwgOutFields (AcDbDwgFiler *pFiler) const {
	assertReadEnabled ();
	Acad::ErrorStatus es = AcDbObject::dwgOutFields(pFiler);
	if (es != Acad::eOk)
		return (es);
	if ((es = pFiler->writeUInt32(CmyReactor::kCurrentVersionNumber)) != Acad::eOk)
		return (es);

	//----- Output params
	pFiler->writePoint3d(chessBoardCenter_old); // 棋盘中心点
	pFiler->writeDouble(chessBoardWidth_old); // 棋盘宽度
	pFiler->writeDouble(chessBoardHeight_old); // 棋盘高度
	pFiler->writeUInt32(chesses.size()); // 棋子数量

	// 棋子的ObjectId
	for (const auto& chessId : chesses) {
		pFiler->writeItem((AcDbSoftPointerId&)chessId);
	}

	return (pFiler->filerStatus());
}

Acad::ErrorStatus CmyReactor::dwgInFields(AcDbDwgFiler * pFiler) {
	assertWriteEnabled();
	Acad::ErrorStatus es = AcDbObject::dwgInFields(pFiler);
	if (es != Acad::eOk)
		return (es);
	Adesk::UInt32 version = 0;
	if ((es = pFiler->readUInt32(&version)) != Acad::eOk)
		return (es);
	if (version > CmyReactor::kCurrentVersionNumber)
		return (Acad::eMakeMeProxy);

	//----- Read params
	pFiler->readPoint3d(&chessBoardCenter_old);
	pFiler->readDouble(&chessBoardWidth_old); // 棋盘宽度
	pFiler->readDouble(&chessBoardHeight_old); // 棋盘高度

	// 读取棋子数量
	Adesk::UInt32 numChesses;
	pFiler->readUInt32(&numChesses); 

	// 读取棋子Id
	chesses.clear();
	for (Adesk::UInt32 i = 0; i < numChesses; ++i) {
		AcDbObjectId chessId;
		pFiler->readItem((AcDbSoftPointerId*)&chessId);
		chesses.push_back(chessId);
	}

	return (pFiler->filerStatus());
}

//----- Persistent reactor callbacks
void CmyReactor::openedForModify(const AcDbObject* pDbObj) {
	assertReadEnabled();
	AcDbObject::openedForModify(pDbObj);
}

void CmyReactor::cancelled(const AcDbObject* pDbObj) {
	assertReadEnabled();
	AcDbObject::cancelled(pDbObj);
}

void CmyReactor::objectClosed(const AcDbObjectId objId) {
	assertReadEnabled();
	AcDbObject::objectClosed(objId);
}

void CmyReactor::goodbye(const AcDbObject* pDbObj) {
	assertReadEnabled();
	AcDbObject::goodbye(pDbObj);
}

void CmyReactor::copied(const AcDbObject* pDbObj, const AcDbObject* pNewObj) {
	assertReadEnabled();
	AcDbObject::copied(pDbObj, pNewObj);
}

void CmyReactor::erased(const AcDbObject* pDbObj, Adesk::Boolean bErasing) {
	assertReadEnabled();
	AcDbObject::erased(pDbObj, bErasing);
}

void CmyReactor::modified(const AcDbObject* pDbObj) {
	assertReadEnabled();

	// 确保处理的是棋盘对象
	CchessBoard* pChessBoard = (CchessBoard*)AcDbEntity::cast(pDbObj);
	if (pChessBoard == nullptr) {
		return;
	}

	// 获取棋盘当前位置、宽高，以及其行列数
	AcGePoint3d currentCenter= pChessBoard->getCenter();
	double currentWidth = pChessBoard->getWidth();
	double currentHeight = pChessBoard->getHeight();
	double column = pChessBoard->getColumn();
	double row = pChessBoard->getRow();

	// 计算行高和列宽
	double oldCellWidth = chessBoardWidth_old / column;
	double oldCellHeight = chessBoardHeight_old / row;
	double newCellWidth = currentWidth / column;
	double newCellHeight = currentHeight / row;

	// 计算棋盘缩放比例
	double scaleX = newCellWidth / oldCellWidth;
	double scaleY = newCellHeight / oldCellHeight;

	// 计算平移向量，移动棋子
	AcDbEntity* pChessEnt = nullptr;
	for (int i = 0; i < chesses.size(); i++) {
		if (acdbOpenObject(pChessEnt, chesses[i], AcDb::kForWrite) == Acad::eOk) {
			// 获取当前棋子的位置
			Cchess* pChess = (Cchess*)pChessEnt; //毕竟C++是强类型语言，得强制类型转换一下
			AcGePoint3d chessPos;
			chessPos = pChess->getCenter();

			// 计算相对于旧中心点的位置
			AcGeVector3d relativePos = chessPos - chessBoardCenter_old;

			// 按缩放比例调整相对位置
			relativePos.set(relativePos.x * scaleX, relativePos.y * scaleY, relativePos.z);

			// 计算新的棋子位置
			AcGePoint3d newChessPos = currentCenter + relativePos;

			// 移动棋子到新位置
			AcGeVector3d translation = newChessPos - chessPos;
			pChess->transformBy(AcGeMatrix3d::translation(translation));
			pChess->setRadius(min(newCellHeight,newCellWidth)/2.5);
			pChess->close();
		}
		else {
			acutPrintf(_T("\n错误：无法打开对应的棋子"));
		}
	}

	// 更新反应器内记录
	chessBoardCenter_old = currentCenter;
	chessBoardWidth_old = currentWidth;
	chessBoardHeight_old = currentHeight;
}

void CmyReactor::modifiedGraphics(const AcDbEntity* pDbEnt) {
	assertReadEnabled();
	AcDbObject::modifiedGraphics(pDbEnt);
}

void CmyReactor::modifiedXData(const AcDbObject* pDbObj) {
	assertReadEnabled();
	AcDbObject::modifiedXData(pDbObj);
}

void CmyReactor::subObjModified(const AcDbObject* pMainbObj, const AcDbObject* pSubObj) {
	assertReadEnabled();
	AcDbObject::subObjModified(pMainbObj, pSubObj);
}

void CmyReactor::modifyUndone(const AcDbObject* pDbObj) {
	assertReadEnabled();
	AcDbObject::modifyUndone(pDbObj);
}

void CmyReactor::reappended(const AcDbObject* pDbObj) {
	assertReadEnabled();
	AcDbObject::reappended(pDbObj);
}

void CmyReactor::unappended(const AcDbObject* pDbObj) {
	assertReadEnabled();
	AcDbObject::unappended(pDbObj);
}

//----- SubXXX() methods (self notification)
Acad::ErrorStatus CmyReactor::subOpen(AcDb::OpenMode mode) {
	return (AcDbObject::subOpen(mode));
}

Acad::ErrorStatus CmyReactor::subErase(Adesk::Boolean erasing) {
	return (AcDbObject::subErase(erasing));
}

Acad::ErrorStatus CmyReactor::subCancel() {
	return (AcDbObject::subCancel());
}

Acad::ErrorStatus CmyReactor::subClose() {
	return (AcDbObject::subClose());
}

