/*
 * datasetglyphset.h
 *
 *  Created on: Apr 24, 2013
 *      Author: boettgerj
 */

#include "datasetsurfaceset.h"

#include "../../gui/gl/pointglyphrenderer.h"
#include "../../gui/gl/vectorglyphrenderer.h"
#include "../../gui/gl/pieglyphrenderer.h"

#include <qstring.h>

#ifndef DATASETGLYPHSET_H_
#define DATASETGLYPHSET_H_

class DatasetGlyphset: public DatasetSurfaceset
{
public:
    DatasetGlyphset( QDir filename, float mt );
    virtual ~DatasetGlyphset();
    void readConnectivity( QString filename );
    void setMinthresh( float mt );

    void draw( QMatrix4x4 pMatrix, QMatrix4x4 mvMatrix, int width, int height, int renderMode );

    void setProperties();

    void makeCons();
    int consNumber;

    void makeVecs();
    int vecsNumber;

    void makePies();

private:
    float minthresh;
    float** conn; //square connectivity matrix

    int n;  //number of vertices, has to match size of matrix

    float* consArray;
    float* vecsArray;

    QVector<float*>* pieArrays;
    QVector<int>* numbers;

    PointGlyphRenderer* m_prenderer;
    VectorGlyphRenderer* m_vrenderer;
    PieGlyphRenderer* m_pierenderer;

    int prevGeo, prevGlyph, prevCol, prevGlyphstyle;
    float prevThresh, prevMinlength;
};

#endif /* DATASETGLYPHSET_H_ */