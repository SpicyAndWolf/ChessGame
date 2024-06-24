#ifndef MYREACTOR_H
#define MYREACTOR_H
#pragma once
#include<vector>
#include "chessBoard.h"

class CmyReactor : public AcDbObject
{
public:
	ACRX_DECLARE_MEMBERS(CmyReactor);

protected:
	static Adesk::UInt32 kCurrentVersionNumber;
	std::vector<AcDbObjectId> chesses; // 关联的棋子
	AcGePoint3d chessBoardCenter_old; // 移动前棋盘的位置
	double chessBoardWidth_old; // 移动前棋盘的宽度
	double chessBoardHeight_old; // 移动前棋盘的高度

public:
	CmyReactor();
	virtual ~CmyReactor();
	void eLinkage(AcDbObjectId i) { chesses.push_back(i);};
	void setChessBoardCenter(AcGePoint3d);
	void setChessBoardWidth(double);
	void setChessBoardHeight(double);

	//----- AcDbObject protocols
	//---- Dwg Filing protocol
	virtual Acad::ErrorStatus dwgOutFields (AcDbDwgFiler *pFiler) const;
	virtual Acad::ErrorStatus dwgInFields (AcDbDwgFiler *pFiler);

	//----- Persistent reactor callbacks
	virtual void openedForModify(const AcDbObject* pDbObj);
	virtual void cancelled(const AcDbObject* pDbObj);
	virtual void objectClosed(const AcDbObjectId objId);
	virtual void goodbye(const AcDbObject* pDbObj);
	virtual void copied(const AcDbObject* pDbObj, const AcDbObject* pNewObj);
	virtual void erased(const AcDbObject* pDbObj, Adesk::Boolean bErasing = true);
	virtual void modified(const AcDbObject* pDbObj);
	virtual void modifiedGraphics(const AcDbEntity* pDbEnt);
	virtual void modifiedXData(const AcDbObject* pDbObj);
	virtual void subObjModified(const AcDbObject* pMainbObj, const AcDbObject* pSubObj);
	virtual void modifyUndone(const AcDbObject* pDbObj);
	virtual void reappended(const AcDbObject* pDbObj);
	virtual void unappended(const AcDbObject* pDbObj);

	//----- SubXXX() methods (self notification)
	virtual Acad::ErrorStatus subOpen(AcDb::OpenMode mode);
	virtual Acad::ErrorStatus subErase(Adesk::Boolean erasing);
	virtual Acad::ErrorStatus subCancel();
	virtual Acad::ErrorStatus subClose();

};

#endif
