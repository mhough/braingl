/*
 * glfunctions.h
 *
 *  Created on: 15.05.2012
 *      Author: Ralph
 */

#ifndef GLFUNCTIONS_H_
#define GLFUNCTIONS_H_

class QAbstractItemModel;
class QGLShaderProgram;

struct VertexData
{
    QVector3D position;
    QVector3D texCoord;
};


class GLFunctions
{
public:
    static void setupTextures( QAbstractItemModel* model );
    static QGLShaderProgram* initSliceShader();
    static void setSliceShaderVars( QGLShaderProgram* program, QAbstractItemModel* model );

private:
    GLFunctions();
    virtual ~GLFunctions();
};

#endif /* GLFUNCTIONS_H_ */