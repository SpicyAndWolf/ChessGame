#include "pch.h"
#include "chess.h"

//----------------------------------------------------------------------------
Adesk::UInt32 Cchess::kCurrentVersionNumber = 1;

//----------------------------------------------------------------------------
//---- runtime definition
ACRX_DXF_DEFINE_MEMBERS(
	Cchess, AcDbEntity,
	AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
	AcDbProxyEntity::kNoOperation, CHESS, CHESSAPP
)

//----------------------------------------------------------------------------
//---- construct & destruct

Cchess::Cchess() {
	radius = 5;
	center = AcGePoint3d(0, 0, 0);
	color = black;
}

Cchess::Cchess(double r,int c) {
	radius = r;
	center = AcGePoint3d(0, 0, 0);
	color = c;
}

Cchess::Cchess(double r, AcGePoint3d c) {
	radius = r;
	center = c;
	color = black;
}

Cchess::~Cchess() {}

//---- 基本get/set函数

void Cchess::setRadius(double r) {
	radius = r;
}
void Cchess::setCenter(AcGePoint3d c) {
	center = c;
	recordGraphicsModified();
}
double Cchess::getRadius() {
	return radius;
}
AcGePoint3d Cchess::getCenter() {
	return center;
}
void Cchess::setColor(int c) {
	color = c;
}

//----------------------------------------------------------------------------
//---- Dwg Filing protocol
Acad::ErrorStatus Cchess::dwgOutFields(AcDbDwgFiler *pFiler) const {
	assertReadEnabled();
	Acad::ErrorStatus es = AcDbEntity::dwgOutFields(pFiler);
	if (es != Acad::eOk)
		return (es);
	if ((es = pFiler->writeUInt32(Cchess::kCurrentVersionNumber)) != Acad::eOk)
		return (es);

	//----- Output params
	pFiler->writePoint3d(center);
	pFiler->writeDouble(radius);
	pFiler->writeInt32(color);

	return (pFiler->filerStatus());
}

Acad::ErrorStatus Cchess::dwgInFields(AcDbDwgFiler * pFiler) {
	assertWriteEnabled();
	Acad::ErrorStatus es = AcDbEntity::dwgInFields(pFiler);
	if (es != Acad::eOk)
		return (es);
	Adesk::UInt32 version = 0;
	if ((es = pFiler->readUInt32(&version)) != Acad::eOk)
		return (es);
	if (version > Cchess::kCurrentVersionNumber)
		return (Acad::eMakeMeProxy);

	//----- Read params
	pFiler->readPoint3d(&center);
	pFiler->readDouble(&radius);
	pFiler->readInt32(&color);

	return (pFiler->filerStatus());
}

//----- 绘制 protocols
Adesk::Boolean Cchess::subWorldDraw(AcGiWorldDraw * mode) {
	assertReadEnabled();

	// 保存当前的图形属性
	AcGiSubEntityTraits& traits = mode->subEntityTraits();

	// 设置颜色
	traits.setColor(AcCmEntityColor::kACIbyBlock);
	AcCmEntityColor colorEntity;
	if (color == black) 
		colorEntity.setRGB(21,255,255);
	else if (color == white) 
		colorEntity.setRGB(255, 255, 255);
	traits.setTrueColor(colorEntity);

	// 画圆
	for (int i = 1; i < 20; i++) {
		mode->geometry().circle(center, radius/i, AcGeVector3d::kZAxis);
	}
	return (AcDbEntity::subWorldDraw(mode));
}

Adesk::UInt32 Cchess::subSetAttributes(AcGiDrawableTraits * traits) {
	assertReadEnabled();
	return (AcDbEntity::subSetAttributes(traits));
}

//---- 图形变换 Protocols
Acad::ErrorStatus Cchess::subTransformBy(const AcGeMatrix3d& xform) {
	assertWriteEnabled();
	center = center.transformBy(xform);
	return Acad::eOk;
}

//----- 捕捉点 protocol
Acad::ErrorStatus Cchess::subGetOsnapPoints(
	AcDb::OsnapMode osnapMode,
	Adesk::GsMarker gsSelectionMark,
	const AcGePoint3d& pickPoint,
	const AcGePoint3d& lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray& snapPoints,
	AcDbIntArray & geomIds
) const {
	assertReadEnabled();
	AcDbCircle circle;
	circle.setCenter(center);
	circle.setRadius(radius);
	return circle.getOsnapPoints(osnapMode, gsSelectionMark, pickPoint, lastPoint, viewXform, snapPoints, geomIds);
}

// 夹点
Acad::ErrorStatus Cchess::subGetGripPoints(AcGePoint3dArray& gripPoints, AcDbIntArray & osnapModes, AcDbIntArray & geomIds) const {
	assertReadEnabled();
	gripPoints.append(center);
	return Acad::eOk;
}

Acad::ErrorStatus Cchess::subMoveGripPointsAt(const AcDbIntArray & indices, const AcGeVector3d& offset) {
	assertWriteEnabled();
	switch (indices[0]) {
	case(0):
		center += offset;
		break;
	}
	return Acad::eOk;
}

