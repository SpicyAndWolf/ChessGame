#ifndef CHESSJIG_H
#define CHESSJIG_H
#pragma once
#include "pch.h"
#include "dbjig.h"
#include "chess.h"

//--------------------------------------s---------------------------------------
class CchessJig : public AcEdJig {

private:
	int mCurrentInputLevel ;
	AcDbDimDataPtrArray mDimData ;
	AcGePoint3d mCenter; // 记录之前的圆心

public:
	AcGePoint3dArray mInputPoints ;
	Cchess *mpEntity ;

public:
	CchessJig () ;
	~CchessJig () ;
	AcEdJig::DragStatus startJig (Cchess *pEntityToJig) ;

protected:
	virtual DragStatus sampler () ;
	virtual Adesk::Boolean update () ;
	virtual AcDbEntity *entity () const ;
	virtual AcDbDimDataPtrArray *dimData (const double dimScale) ;
	virtual Acad::ErrorStatus setDimValue (const AcDbDimData *pDimData, const double dimValue) ;
	virtual Adesk::Boolean updateDimData () ;

	AcEdJig::DragStatus GetStartPoint () ;
	AcEdJig::DragStatus GetNextPoint () ;
} ;

#endif