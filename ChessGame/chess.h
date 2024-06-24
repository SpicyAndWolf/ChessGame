#ifndef CHESS_H
#define CHESS_H
#pragma once
#include "dbproxy.h"


class Cchess : public AcDbEntity
{
public:
	ACRX_DECLARE_MEMBERS(Cchess);
	void setRadius(double);
	void setCenter(AcGePoint3d);
	double getRadius();
	AcGePoint3d getCenter();

protected:
	static Adesk::UInt32 kCurrentVersionNumber;
	double radius;
	AcGePoint3d center;
	int color;
	enum chessColor {
		empty = 0,
		white = 1,
		black = 2
	};

public:
	Cchess();
	Cchess(double,int);
	Cchess(double, AcGePoint3d);
	virtual ~Cchess();
	void setColor(int);

	//---- Dwg Filing protocol
	virtual Acad::ErrorStatus dwgOutFields(AcDbDwgFiler *pFiler) const;
	virtual Acad::ErrorStatus dwgInFields(AcDbDwgFiler *pFiler);

	//----- Graphics protocol
	Acad::ErrorStatus subTransformBy(const AcGeMatrix3d&);
	Acad::ErrorStatus subGetGripPoints(AcGePoint3dArray& gripPoints, AcDbIntArray & osnapModes, AcDbIntArray & geomIds) const;
	Acad::ErrorStatus subMoveGripPointsAt(const AcDbIntArray & indices, const AcGeVector3d& offset);
	Acad::ErrorStatus subGetOsnapPoints(
		AcDb::OsnapMode,
		Adesk::GsMarker,
		const AcGePoint3d&,
		const AcGePoint3d&,
		const AcGeMatrix3d&,
		AcGePoint3dArray&,
		AcDbIntArray &
	) const;

protected:
	virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw *mode);
	virtual Adesk::UInt32 subSetAttributes(AcGiDrawableTraits *traits);

};

#endif
