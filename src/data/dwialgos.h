/*
 * dwialgos.h
 *
 *  Created on: Jun 18, 2012
 *      Author: schurade
 */

#ifndef DWIALGOS_H_
#define DWIALGOS_H_

#include <QtCore/QList>

class DatasetDWI;
class DatasetNifti;
class Dataset3D;
class TriangleMesh;

class DWIAlgos
{
public:
    static DatasetDWI* qBall( DatasetDWI* ds );
    static DatasetDWI* qBallSharp( DatasetDWI* ds );
    static DatasetDWI* tensorFit( DatasetDWI* ds );
    static DatasetScalar* calcFA( DatasetDWI* ds );
    static QList<Dataset*> calcEV( DatasetDWI* ds );


private:
    DWIAlgos();
    virtual ~DWIAlgos();

    static double pow2( double v ) {return v*v;};
    static double pow3( double v ) {return v*v*v;};

};

#endif /* DWIALGOS_H_ */
