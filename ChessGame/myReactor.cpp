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
	pFiler->writePoint3d(chessBoardCenter_old); // �������ĵ�
	pFiler->writeUInt32(chesses.size()); // ��������

	// ���ӵ�ObjectId
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

	// ��ȡ��������
	Adesk::UInt32 numChesses;
	pFiler->readUInt32(&numChesses); 

	// ��ȡ����Id
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

	// ȷ������������̶���
	CchessBoard* pChessBoard = (CchessBoard*)AcDbEntity::cast(pDbObj);
	if (pChessBoard == nullptr) {
		return;
	}

	// ��ȡ���̵�ǰλ�ã����·�Ӧ������Ϣ
	AcGePoint3d currentCenter= pChessBoard->getCenter();
	AcGePoint3d oldCenter = chessBoardCenter_old;
	pChessBoard->setCenter(currentCenter);

	// ����ƽ���������ƶ�����
	AcGeVector3d translation = currentCenter - oldCenter;
	AcDbEntity* pChess = nullptr;
	for (int i = 0; i < chesses.size(); i++) {
		if (acdbOpenObject(pChess, chesses[i], AcDb::kForWrite) == Acad::eOk) {
			pChess->transformBy(AcGeMatrix3d::translation(translation));
			pChess->close();
		}
		else {
			acutPrintf(_T("\n�����޷��򿪶�Ӧ������"));
		}
	}
	chessBoardCenter_old = currentCenter;
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

