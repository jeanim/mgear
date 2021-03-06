/*

    This file is part of MGEAR.

    MGEAR is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/lgpl.html>.

    Author:     Jeremie Passerin      geerem@hotmail.com
    Url:        http://www.jeremiepasserin.com
    Date:       2011 / 07 / 13

*/
/////////////////////////////////////////////////
// INCLUDE
/////////////////////////////////////////////////
#include "gear_solvers.h"

/////////////////////////////////////////////////
// GLOBAL
/////////////////////////////////////////////////
MTypeId gear_rollSplineKine::id( 0x27001 );

// Define the Node's attribute specifiers

MObject gear_rollSplineKine::ctlParent;
MObject gear_rollSplineKine::inputs;
MObject gear_rollSplineKine::inputsRoll;
MObject gear_rollSplineKine::outputParent;

MObject gear_rollSplineKine::u;
MObject gear_rollSplineKine::resample;
MObject gear_rollSplineKine::subdiv;
MObject gear_rollSplineKine::absolute;

MObject gear_rollSplineKine::output;

gear_rollSplineKine::gear_rollSplineKine() {} // constructor
gear_rollSplineKine::~gear_rollSplineKine() {} // destructor

/////////////////////////////////////////////////
// METHODS
/////////////////////////////////////////////////
// CREATOR ======================================
void* gear_rollSplineKine::creator()
{
   return new gear_rollSplineKine();
}

// INIT =========================================
MStatus gear_rollSplineKine::initialize()
{
	MFnMatrixAttribute mAttr;
	MFnNumericAttribute nAttr;
	MStatus stat;
	
    // Inputs Matrices
    ctlParent = mAttr.create( "ctlParent", "ctlp", MFnMatrixAttribute::kDouble );
    mAttr.setStorable(true);
    mAttr.setReadable(false);
    mAttr.setIndexMatters(false);
    mAttr.setArray(true);
    addAttribute( ctlParent );
	
    inputs = mAttr.create( "inputs", "in", MFnMatrixAttribute::kDouble );
    mAttr.setStorable(true);
    mAttr.setReadable(false);
    mAttr.setIndexMatters(false);
    mAttr.setArray(true);
    addAttribute( inputs );

    inputsRoll = nAttr.create ( "inputsRoll", "inr", MFnNumericData::kFloat, 0.0 );
    nAttr.setArray(true);
    nAttr.setStorable(true);
    addAttribute ( inputsRoll );
	
	outputParent = mAttr.create( "outputParent", "outp" );
	mAttr.setStorable(true);
	mAttr.setKeyable(true);
	mAttr.setConnectable(true);
	stat = addAttribute( outputParent );
		if (!stat) {stat.perror("addAttribute"); return stat;}

    // Inputs Sliders
	u = nAttr.create("u", "u", MFnNumericData::kFloat, 0.0);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setMin(0);
	nAttr.setMax(1);
    stat = addAttribute( u );
		if (!stat) {stat.perror("addAttribute"); return stat;}
		
	resample = nAttr.create("resample", "re", MFnNumericData::kBoolean, false);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
    stat = addAttribute( resample );
		if (!stat) {stat.perror("addAttribute"); return stat;}

	subdiv = nAttr.create("subdiv", "sd", MFnNumericData::kShort, 10);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
	nAttr.setMin(3);
    stat = addAttribute( subdiv );
		if (!stat) {stat.perror("addAttribute"); return stat;}

	absolute = nAttr.create("absolute", "abs", MFnNumericData::kBoolean, false);
	nAttr.setStorable(true);
	nAttr.setKeyable(true);
    stat = addAttribute( absolute );
		if (!stat) {stat.perror("addAttribute"); return stat;}

    // Outputs
	output = mAttr.create( "output", "out" );
	mAttr.setStorable(false);
	mAttr.setKeyable(false);
	mAttr.setConnectable(true);
	stat = addAttribute( output );
		if (!stat) {stat.perror("addAttribute"); return stat;}

	// Connections
    stat = attributeAffects ( ctlParent, output );
		if (!stat) {stat.perror("attributeAffects"); return stat;}
    stat = attributeAffects ( inputs, output );
		if (!stat) {stat.perror("attributeAffects"); return stat;}
    stat = attributeAffects ( inputsRoll, output );
		if (!stat) {stat.perror("attributeAffects"); return stat;}
    stat = attributeAffects ( outputParent, output );
		if (!stat) {stat.perror("attributeAffects"); return stat;}
		
    stat = attributeAffects ( u, output );
		if (!stat) {stat.perror("attributeAffects"); return stat;}
    stat = attributeAffects ( resample, output );
		if (!stat) {stat.perror("attributeAffects"); return stat;}
    stat = attributeAffects ( subdiv, output );
		if (!stat) {stat.perror("attributeAffects"); return stat;}
    stat = attributeAffects ( absolute, output );
		if (!stat) {stat.perror("attributeAffects"); return stat;}
		
   return MS::kSuccess;
}
// COMPUTE ======================================
MStatus gear_rollSplineKine::compute(const MPlug& plug, MDataBlock& data)
{

	MStatus returnStatus;
	// Error check
    if (plug != output)
        return MS::kUnknownParameter;


	// Get inputs matrices ------------------------------
	// Inputs Parent
	MArrayDataHandle adh = data.inputArrayValue( ctlParent );
	int count = adh.elementCount();
	if (count < 1)
		return MS::kFailure;
	MMatrixArray inputsP(count);
	for (int i = 0 ; i < count ; i++){
		adh.jumpToElement(i);
		inputsP[i] = adh.inputValue().asMatrix();
	}

	// Inputs
	adh = data.inputArrayValue( inputs );
	if (count != adh.elementCount())
		return MS::kFailure;
	MMatrixArray inputs(count);
	for (int i = 0 ; i < count ; i++){
		adh.jumpToElement(i);
		inputs[i] = adh.inputValue().asMatrix();
	}

	adh = data.inputArrayValue( inputsRoll );
	if (count != adh.elementCount())
		return MS::kFailure;
	MDoubleArray roll(adh.elementCount());
	for (int i = 0 ; i < count ; i++){
		adh.jumpToElement(i);
		roll[i] = degrees2radians((double)adh.inputValue().asFloat());
	}

	// Output Parent
	MDataHandle ha = data.inputValue( outputParent );
	MMatrix outputParent = ha.asMatrix();
	
    // Get inputs sliders -------------------------------
    double in_u = (double)data.inputValue( u ).asFloat();
    bool in_resample = data.inputValue( resample ).asBool();
    int in_subdiv = data.inputValue( subdiv ).asShort();
    bool in_absolute = data.inputValue( absolute ).asBool();
	
    // Process ------------------------------------------
    // Get roll, pos, tan, rot, scl
    MVectorArray pos(count);
    MVectorArray tan(count);
	MQuaternion *rot;
	rot = new MQuaternion[count];
    MVectorArray scl(count);
	double threeDoubles[3];
	for (int i = 0 ; i < count ; i++){
		MTransformationMatrix tp(inputsP[i]);
		MTransformationMatrix t(inputs[i]);
		pos[i] = t.getTranslation(MSpace::kWorld);
		rot[i] = tp.rotation();

		t.getScale(threeDoubles, MSpace::kWorld);
		scl[i] = MVector(threeDoubles[0], threeDoubles[1], threeDoubles[2]);
		tan[i] = MVector(threeDoubles[0] * 2.5, 0, 0).rotateBy(t.rotation());
	}
	
    // Get step and indexes
    // We define between wich controlers the object is to be able to
    // calculate the bezier 4 points front this 2 objects
	double step = 1.0 / max( 1, count-1.0 );
	int index1 = (int)min( count-2.0, in_u/step );
	int index2 = index1+1;
	int index1temp = index1;
	int index2temp = index2;
	double v = (in_u - step * double(index1)) / step;
	double vtemp = v;
	
   // calculate the bezier
   MVector bezierPos;
   MVector xAxis, yAxis, zAxis;
   if(!in_resample){
      // straight bezier solve
      MVectorArray results = bezier4point(pos[index1],tan[index1],pos[index2],tan[index2],v);
      bezierPos = results[0];
      xAxis = results[1];
   }
   else if(!in_absolute){
      MVectorArray presample(in_subdiv);
      MVectorArray presampletan(in_subdiv);
      MDoubleArray samplelen(in_subdiv);
      double samplestep = 1.0 / double(in_subdiv-1);
      double sampleu = samplestep;
      presample[0]  = pos[index1];
      presampletan[0]  = tan[index1];
      MVector prevsample(presample[0]);
      MVector diff;
      samplelen[0] = 0;
      double overalllen = 0;
      MVectorArray results(2);
      for(long i=1;i<in_subdiv;i++,sampleu+=samplestep){
         results = bezier4point(pos[index1],tan[index1],pos[index2],tan[index2],sampleu);
         presample[i] = results[0];
         presampletan[i] = results[1];
		 diff = presample[i] - prevsample;
		 overalllen += diff.length();
         samplelen[i] = overalllen;
         prevsample = presample[i];
      }
      // now as we have the
      sampleu = 0;
      for(long i=0;i<in_subdiv-1;i++,sampleu+=samplestep){
         samplelen[i+1] = samplelen[i+1] / overalllen;
         if(v>=samplelen[i] && v <=  samplelen[i+1]){
            v = (v - samplelen[i]) / (samplelen[i+1] - samplelen[i]);
			bezierPos = linearInterpolate(presample[i],presample[i+1],v);
			xAxis = linearInterpolate(presampletan[i],presampletan[i+1],v);
            break;
         }
      }
   }
   else{
      MVectorArray presample(in_subdiv);
      MVectorArray presampletan(in_subdiv);
      MDoubleArray samplelen(in_subdiv);
      double samplestep = 1.0 / double(in_subdiv-1);
      double sampleu = samplestep;
      presample[0]  = pos[0];
      presampletan[0]  = tan[0];
      MVector prevsample(presample[0]);
      MVector diff;
      samplelen[0] = 0;
      double overalllen = 0;
      MVectorArray results;
      for(long i=1;i<in_subdiv;i++,sampleu+=samplestep){
         index1 = (int)min(count-2,sampleu / step);
         index2 = index1+1;
         v = (sampleu - step * double(index1)) / step;
         results = bezier4point(pos[index1],tan[index1],pos[index2],tan[index2],v);
         presample[i] = results[0];
         presampletan[i] = results[1];
		 diff = presample[i] - prevsample;
		 overalllen += diff.length();
         samplelen[i] = overalllen;
         prevsample = presample[i];
      }
      // now as we have the
      sampleu = 0;
      for(long i=0;i<in_subdiv-1;i++,sampleu+=samplestep){
         samplelen[i+1] = samplelen[i+1] / overalllen;
         if(in_u>=samplelen[i] && in_u <= samplelen[i+1]){
            in_u = (in_u - samplelen[i]) / (samplelen[i+1] - samplelen[i]);
			bezierPos = linearInterpolate(presample[i],presample[i+1],in_u);
			xAxis = linearInterpolate(presampletan[i],presampletan[i+1],in_u);
            break;
         }
      }
   }

   
	// compute the scaling (straight interpolation!)
	MVector scl1 = linearInterpolate(scl[index1temp], scl[index2temp],vtemp);

	// compute the rotation!
	MQuaternion q = slerp(rot[index1temp], rot[index2temp], vtemp);
	yAxis = MVector(0,1,0);
	yAxis = yAxis.rotateBy(q);
	
	// use directly or project the roll values!
	// print roll
	double a = linearInterpolate(roll[index1temp], roll[index2temp], vtemp);
	yAxis = yAxis.rotateBy( MQuaternion(xAxis.x * sin(a/2.0), xAxis.y * sin(a/2.0), xAxis.z * sin(a/2.0), cos(a/2.0)));
	
	zAxis = xAxis ^ yAxis;
	zAxis.normalize();
	yAxis = zAxis ^ xAxis;
	yAxis.normalize();

	// Output -------------------------------------------
	MTransformationMatrix result;

	// translation
	result.setTranslation(bezierPos, MSpace::kWorld);
	// rotation
	q = getQuaternionFromAxes(xAxis,yAxis,zAxis);
	result.setRotationQuaternion(q.x, q.y, q.z, q.w);
	// scaling
	threeDoubles[0] = 1;
	threeDoubles[0] = scl1.y;
	threeDoubles[0] = scl1.z;
	result.setScale(threeDoubles, MSpace::kWorld);

	MDataHandle h = data.outputValue( output );
	h.setMMatrix( result.asMatrix() * outputParent.inverse() );

	data.setClean( plug );


	return MS::kSuccess;
}

