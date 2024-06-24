#ifndef CHESSBOARDJIG_H
#define CHESSBOARDJIG_H
#pragma once
#include "pch.h"
#include "dbjig.h"
#include "chessBoard.h"

class CchessBoardJig;
//-----------------------------------------------------------------------------
class CchessBoardJig : public AcEdJig {

private:
	int mCurrentInputLevel;
	AcDbDimDataPtrArray mDimData;

public:
	AcGePoint3dArray mInputPoints;
	CchessBoard *mpEntity;

public:
	CchessBoardJig();
	CchessBoardJig(CchessBoard*);
	~CchessBoardJig();

	AcEdJig::DragStatus startJig(CchessBoard *pEntityToJig);

protected:
	virtual DragStatus sampler();
	virtual Adesk::Boolean update();
	virtual AcDbEntity *entity() const;
	virtual AcDbDimDataPtrArray *dimData(const double dimScale);
	virtual Acad::ErrorStatus setDimValue(const AcDbDimData *pDimData, const double dimValue);

	virtual Adesk::Boolean updateDimData();

	AcEdJig::DragStatus GetStartPoint();
	AcEdJig::DragStatus GetNextPoint();
};

#endif