//-----------------------------------------------------------------------------
#include "pch.h"
#include "chessBoardJig.h"

//-----------------------------------------------------------------------------
// ���졢��������
CchessBoardJig::CchessBoardJig() : AcEdJig(),
mCurrentInputLevel(0), mpEntity(nullptr) { }

CchessBoardJig::CchessBoardJig(CchessBoard* ent) : AcEdJig(),
mCurrentInputLevel(0), mpEntity(ent) { }

CchessBoardJig::~CchessBoardJig() { }

//-----------------------------------------------------------------------------
AcEdJig::DragStatus CchessBoardJig::startJig(CchessBoard *pEntity) {
	//- Store the new entity pointer
	mpEntity = pEntity;
	//- Setup each input prompt
	AcString inputPrompts[2] = {
		"\nPick point",
		"\nPick point"
	};
	//- Setup kwords for each input
	AcString kwords[2] = {
		"",
		""
	};

	bool appendOk = true;
	AcEdJig::DragStatus status = AcEdJig::kNull;
	//- Loop the number of inputs
	for (mCurrentInputLevel = 0; mCurrentInputLevel < 2; mCurrentInputLevel++) {
		//- Add a new input point to the list of input points
		mInputPoints.append(AcGePoint3d());
		//- Set the input prompt
		setDispPrompt(inputPrompts[mCurrentInputLevel]);
		//- Setup the keywords required
		setKeywordList(kwords[mCurrentInputLevel]);

		bool quit = false;
		//- Lets now do the input
		status = drag();
		if (status != kNormal) {
			//- If it's a keyword
			switch (status) {
			case kCancel:
			case kNull:
				quit = true;
				break;

			case kKW1:
			case kKW2:
			case kKW3:
			case kKW4:
			case kKW5:
			case kKW6:
			case kKW7:
			case kKW8:
			case kKW9:
				//- Do something
				break;
			}
		}
		else {
			appendOk = true;
		}

		//- If to finish
		if (quit)
			break;
	}

	//- If the input went well
	if (appendOk)
		//- Append to the database
		append();
	else
		//- Clean up
		delete mpEntity;

	return (status);
}

//-----------------------------------------------------------------------------
//- Input sampler
AcEdJig::DragStatus CchessBoardJig::sampler() {
	AcGePoint3d oldPnt = mInputPoints[mCurrentInputLevel];
	AcGePoint3d newPnt;
	AcEdJig::DragStatus status = acquirePoint(newPnt, oldPnt);
	if (status == AcEdJig::kNormal) {
		if (newPnt.isEqualTo(mInputPoints[mCurrentInputLevel]))
			return (AcEdJig::kNoChange);//�����ȡ�ĵ��뵱ǰ������һ�λ�ȡ�ĵ㣩һ�����򷵻�kNoChange�������Ͳ������update
		mInputPoints[mCurrentInputLevel] = newPnt;//���µ�ǰ�ĵ�
	}
	return (status);

}

//-----------------------------------------------------------------------------
//- Jigged entity update
Adesk::Boolean CchessBoardJig::update() {
	switch (mCurrentInputLevel + 1) {
	case 1://��һ�������ȡ�������ĵ�
		mpEntity->setCenter(mInputPoints[mCurrentInputLevel]);
		break;
	case 2://�ڶ��������ȡ���ǿ�͸�
	{
		AcGePoint3d center = mpEntity->getCenter();
		AcGePoint3d secondPoint = mInputPoints[mCurrentInputLevel];

		// �����͸�
		double width = fabs(secondPoint.x - center.x) * 2;
		double height = fabs(secondPoint.y - center.y) * 2;

		mpEntity->setWidth(width);
		mpEntity->setHeight(height);
	}
	default:
		break;
	}

	return (updateDimData());
}

//-----------------------------------------------------------------------------
//- Jigged entity pointer return
AcDbEntity * CchessBoardJig::entity() const {
	return ((AcDbEntity *)mpEntity);
}

//-----------------------------------------------------------------------------
//- Dynamic dimension data setup
AcDbDimDataPtrArray * CchessBoardJig::dimData(const double dimScale) {
	AcDbAlignedDimension *dim = new AcDbAlignedDimension();
	dim->setDatabaseDefaults();
	dim->setNormal(AcGeVector3d::kZAxis);
	dim->setElevation(0.0);
	dim->setHorizontalRotation(0.0);
	dim->setXLine1Point(mInputPoints[0]);
	dim->setXLine2Point(mInputPoints[1]);
	AcGePoint3d dimPoint = mInputPoints[0] + ((mInputPoints[1] - mInputPoints[0]) / 2.0);
	dim->setDimLinePoint(dimPoint);
	dim->setDimtad(1);

	//AcDbDimData�������ڱ���new�����ı�ע
	AcDbDimData *dimData = new AcDbDimData(dim);
	dimData->setDimFocal(true);
	dimData->setDimHideIfValueIsZero(true);
	dimData->setDimInvisible(false);
	dimData->setDimEditable(true);
	mDimData.append(dimData);//��DimData�������������ظ�CAD

	return (NULL);
}

//-----------------------------------------------------------------------------
//- Dynamic dimension data update
Acad::ErrorStatus CchessBoardJig::setDimValue(const AcDbDimData *pDimData, const double dimValue) {
	Acad::ErrorStatus es = Acad::eOk;

	AcDbDimData *dimDataNC = const_cast<AcDbDimData *>(pDimData);//��constָ��ת����ָͨ��
	int inputNumber = -1;
	if (mDimData.find(dimDataNC, inputNumber)) {
		AcDbDimension *pDim = (AcDbDimension *)dimDataNC->dimension();//��DimData�л�ȡ��ע����
		AcDbAlignedDimension *pAlnDim = AcDbAlignedDimension::cast(pDim);
		if (pAlnDim) {
			AcGePoint3d dimStart = pAlnDim->xLine1Point();
			AcGePoint3d dimEnd = pAlnDim->xLine2Point();
			AcGePoint3d dimEndNew = dimStart + (dimEnd - dimStart).normalize() * dimValue;
			pAlnDim->setXLine2Point(dimEndNew);
			//��ͨ����עֵ��������ĵ�洢������Ϊ����Ľ��
			mInputPoints[mCurrentInputLevel] = dimEndNew;
			update();//�ֶ�����update��Ҳ���Է�װһ������ͬʱ��update��setDimValue����
		}
	}
	return (es);
}

//-----------------------------------------------------------------------------
//- Various helper functions
//- Dynamic dimdata update function
Adesk::Boolean CchessBoardJig::updateDimData() {
	//- Check the dim data store for validity
	if (mDimData.length() <= 0)
		return (true);

	AcDbDimData *dimData = mDimData[0];
	AcDbDimension *pDim = (AcDbDimension *)dimData->dimension();
	AcDbAlignedDimension *pAlnDim = AcDbAlignedDimension::cast(pDim);
	if (pAlnDim) {
		dimData->setDimInvisible(false);
		pAlnDim->setXLine1Point(mInputPoints[0]);
		pAlnDim->setXLine2Point(mInputPoints[1]);
		AcGePoint3d dimPoint = mInputPoints[0] + ((mInputPoints[1] - mInputPoints[0]) / 2.0);
		pAlnDim->setDimLinePoint(dimPoint);
	}

	return (true);;
}

//-----------------------------------------------------------------------------
//- Std input to get a point with no rubber band
AcEdJig::DragStatus CchessBoardJig::GetStartPoint() {
	AcGePoint3d newPnt;
	//- Get the point 
	AcEdJig::DragStatus status = acquirePoint(newPnt);
	//- If valid input
	if (status == AcEdJig::kNormal) {
		//- If there is no difference
		if (newPnt.isEqualTo(mInputPoints[mCurrentInputLevel]))
			return (AcEdJig::kNoChange);
		//- Otherwise update the point
		mInputPoints[mCurrentInputLevel] = newPnt;
	}
	return (status);
}

//-----------------------------------------------------------------------------
//- Std input to get a point with rubber band from point
AcEdJig::DragStatus CchessBoardJig::GetNextPoint() {
	AcGePoint3d oldPnt = mInputPoints[mCurrentInputLevel];
	AcGePoint3d newPnt;
	//- Get the point 
	AcEdJig::DragStatus status = acquirePoint(newPnt, oldPnt);
	//- If valid input
	if (status == AcEdJig::kNormal) {
		//- If there is no difference
		if (newPnt.isEqualTo(mInputPoints[mCurrentInputLevel]))
			return (AcEdJig::kNoChange);
		//- Otherwise update the point
		mInputPoints[mCurrentInputLevel] = newPnt;
	}
	return (status);
}
