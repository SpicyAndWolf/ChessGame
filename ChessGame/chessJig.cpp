//-----------------------------------------------------------------------------
#include "pch.h"
#include "chessJig.h"

//-----------------------------------------------------------------------------
CchessJig::CchessJig () : AcEdJig (),mCurrentInputLevel(0), mpEntity(nullptr) {
	mpEntity = new Cchess();
	mpEntity->setDatabaseDefaults();
}

CchessJig::~CchessJig () { }

//-----------------------------------------------------------------------------
AcEdJig::DragStatus CchessJig::startJig (Cchess *pEntity) {
	//- Store the new entity pointer
	mpEntity = pEntity ;
	//- Setup each input prompt
	AcString inputPrompts[1] ={
		"\nPick point"
	};
	//- Setup kwords for each input
	AcString kwords[1] ={
		""
	};

	bool appendOk =true ;
	AcEdJig::DragStatus status =AcEdJig::kNull ;
	//- Loop the number of inputs
	for ( mCurrentInputLevel = 0; mCurrentInputLevel < 1; mCurrentInputLevel++ ) {
		//- Add a new input point to the list of input points
		mInputPoints.append (AcGePoint3d ()) ;
		//- Set the input prompt
		setDispPrompt (inputPrompts [mCurrentInputLevel]) ;
		//- Setup the keywords required
		setKeywordList (kwords [mCurrentInputLevel]) ;

		bool quit =false ;
		//- Lets now do the input
		status =drag () ;
		if ( status != kNormal ) {
			//- If it's a keyword
			switch ( status ) {
				case kCancel: 
				case kNull:
					quit =true ;
					break ;

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
					break ;
		  }
		} else {
			appendOk =true ;
		}

		//- If to finish
		if ( quit )
			break ;
	}

	//- If the input went well
	if ( appendOk )
		//- Append to the database
		append () ;
	else
		//- Clean up
		delete mpEntity  ;

	return (status) ;
}

//-----------------------------------------------------------------------------
//- Input sampler
AcEdJig::DragStatus CchessJig::sampler () {
	AcGePoint3d oldPnt = mInputPoints[mCurrentInputLevel];
	AcGePoint3d newPnt;
	AcEdJig::DragStatus status = acquirePoint(newPnt, oldPnt);
	if (status == AcEdJig::kNormal) {
		if (newPnt.isEqualTo(mInputPoints[mCurrentInputLevel]))
			return (AcEdJig::kNoChange);//如果获取的点与当前（即上一次获取的点）一样，则返回kNoChange，这样就不会调用update
		mInputPoints[mCurrentInputLevel] = newPnt;//更新当前的点
	}
	return (status);

}

//-----------------------------------------------------------------------------
//- Jigged entity update
Adesk::Boolean CchessJig::update () {
	switch (mCurrentInputLevel + 1) {
		case 1:
			mpEntity->setCenter (mInputPoints [mCurrentInputLevel]) ;
			break;
		default:
			break;
		}

	return Adesk::kTrue;
}

//-----------------------------------------------------------------------------
//- Jigged entity pointer return
AcDbEntity * CchessJig::entity () const {
	return ((AcDbEntity *)mpEntity) ;
}

//-----------------------------------------------------------------------------
//- Dynamic dimension data setup
AcDbDimDataPtrArray * CchessJig::dimData (const double dimScale) {

	/* SAMPLE CODE:
	AcDbAlignedDimension *dim =new AcDbAlignedDimension () ;
	dim->setDatabaseDefaults () ;
	dim->setNormal (AcGeVector3d::kZAxis) ;
	dim->setElevation (0.0) ;
	dim->setHorizontalRotation (0.0) ;
	dim->setXLine1Point (m_originPoint) ;
	dim->setXLine2Point (m_lastPoint) ;
	//- Get the dimPoint, first the midpoint
	AcGePoint3d dimPoint =m_originPoint + ((m_lastPoint - m_originPoint) / 2.0) ;
	//- Then the offset
	dim->setDimLinePoint (dimPoint) ;
	dim->setDimtad (1) ;

	AcDbDimData *dimData = new AcDbDimData (dim) ;
	//AppData *appData =new AppData (1, dimScale) ;
	//dimData.setAppData (appData) ;
	dimData->setDimFocal (true) ;
	dimData->setDimHideIfValueIsZero (true) ;

	//- Check to see if it is required
	if ( getDynDimensionRequired (m_inputNumber) )
		dimData->setDimInvisible (false) ;
	else
		dimData->setDimInvisible (true) ;

	//- Make sure it is editable TODO: 
	dimData->setDimEditable (true) ;
	mDimData.append (dimData) ;

	return (&mDimData) ;
	*/
	return (NULL) ;
}

//-----------------------------------------------------------------------------
//- Dynamic dimension data update
Acad::ErrorStatus CchessJig::setDimValue (const AcDbDimData *pDimData, const double dimValue) {
	Acad::ErrorStatus es =Acad::eOk ;

	/* SAMPLE CODE:
	//- Convert the const pointer to non const
	AcDbDimData *dimDataNC =const_cast<AcDbDimData *>(pDimData) ;
	int inputNumber =-1 ;
	//- Find the dim data being passed so we can determine the input number
	if ( mDimData.find (dimDataNC, inputNumber) ) {
		//- Now get the dimension
		AcDbDimension *pDim =(AcDbDimension *)dimDataNC->dimension () ;
		//- Check it's the type of dimension we want
		AcDbAlignedDimension *pAlnDim =AcDbAlignedDimension::cast (pDim) ;
		//- If ok
		if ( pAlnDim ) {
			//- Extract the dimensions as they are now
			AcGePoint3d dimStart =pAlnDim->xLine1Point () ;
			AcGePoint3d dimEnd =pAlnDim->xLine2Point () ;
			//- Lets get the new point entered by the user 
			AcGePoint3d dimEndNew =dimStart + (dimEnd - dimStart).normalize () * dimValue ;
			//- Finally set the end dim point
			pAlnDim->setXLine2Point (dimEndNew) ;
			//- Now update the jig data to reflect the dynamic dimension input
			mInputPoints [mCurrentInputLevel] =dimEndNew ;
		}
	}*/
	return (es) ;
}

//-----------------------------------------------------------------------------
//- Various helper functions
//- Dynamic dimdata update function
Adesk::Boolean CchessJig::updateDimData () {
	//- Check the dim data store for validity
	if ( mDimData.length () <= 0 )
		return (true) ;

	/* SAMPLE CODE :
	//- Extract the individual dimData
	AcDbDimData *dimData =mDimData [m_inputNumber] ;
	//- Now get the dimension
	AcDbDimension *pDim =(AcDbDimension *)dimData->dimension () ;
	//- Check it's the type of dimension we want
	AcDbAlignedDimension *pAlnDim =AcDbAlignedDimension::cast (pDim) ;
	//- If ok
	if ( pAlnDim ) {
		//- Check to see if it is required
		if ( getDynDimensionRequired (m_inputNumber) )
			dimData->setDimInvisible (false) ;
		else
			dimData->setDimInvisible (true) ;
		pAlnDim->setXLine1Point (m_originPoint) ;
		pAlnDim->setXLine2Point (m_lastPoint) ;
		//- Get the dimPoint, first the midpoint
		AcGePoint3d dimPoint =m_originPoint + ((m_lastPoint - m_originPoint) / 2.0) ;
		//- Then the offset
		pAlnDim->setDimLinePoint (dimPoint) ;
	} */
	return (true) ;
}

//-----------------------------------------------------------------------------
//- Std input to get a point with no rubber band
AcEdJig::DragStatus CchessJig::GetStartPoint () {
	AcGePoint3d newPnt ;
	//- Get the point 
	AcEdJig::DragStatus status =acquirePoint (newPnt) ;
	//- If valid input
	if ( status == AcEdJig::kNormal ) {
		//- If there is no difference
		if ( newPnt.isEqualTo (mInputPoints [mCurrentInputLevel]) )
			return (AcEdJig::kNoChange) ;
		//- Otherwise update the point
		mInputPoints [mCurrentInputLevel] =newPnt ;
	}
	return (status) ;
}

//-----------------------------------------------------------------------------
//- Std input to get a point with rubber band from point
AcEdJig::DragStatus CchessJig::GetNextPoint () {
	AcGePoint3d oldPnt = mInputPoints[mCurrentInputLevel] ;
	AcGePoint3d newPnt ;
	//- Get the point 
	AcEdJig::DragStatus status = acquirePoint (newPnt, oldPnt) ;
	//- If valid input
	if ( status == AcEdJig::kNormal ) {
		//- If there is no difference
		if ( newPnt.isEqualTo (mInputPoints [mCurrentInputLevel]) )
			return (AcEdJig::kNoChange) ;
		//- Otherwise update the point
		mInputPoints [mCurrentInputLevel] =newPnt ;
	}
	return (status) ;
}
