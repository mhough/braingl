/*
 * memeshRenderer.cpp
 *
 *  Created on: Dec 4, 2012
 *      Author: Ralph Schurade
 */
#include "meshrenderer.h"

#include "glfunctions.h"

#include "../../data/mesh/trianglemesh2.h"

#include <QtOpenGL/QGLShaderProgram>
#include <QDebug>
#include <QVector3D>
#include <QMatrix4x4>

#include <limits>

MeshRenderer::MeshRenderer( TriangleMesh2* mesh ) :
    ObjectRenderer(),
    m_tris( 0 ),
    vboIds( new GLuint[ 2 ] ),
    m_mesh( mesh ),
    m_dirty( true )
{

}

MeshRenderer::~MeshRenderer()
{
    glDeleteBuffers(1, &( vboIds[ 0 ] ) );
    glDeleteBuffers(1, &( vboIds[ 1 ] ) );
}

void MeshRenderer::setMesh( TriangleMesh2* mesh )
{
    m_mesh = mesh;
    m_dirty = true;
}

void MeshRenderer::init()
{
    glGenBuffers( 2, vboIds );
}

void MeshRenderer::draw( QMatrix4x4 p_matrix, QMatrix4x4 mv_matrix )
{
    GLFunctions::getShader( "mesh" )->bind();
    // Set modelview-projection matrix
    GLFunctions::getShader( "mesh" )->setUniformValue( "mvp_matrix", p_matrix * mv_matrix );
    GLFunctions::getShader( "mesh" )->setUniformValue( "mv_matrixInvert", mv_matrix.inverted() );

    if ( m_dirty )
    {
        initGeometry();
    }

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vboIds[ 0 ] );
    glBindBuffer( GL_ARRAY_BUFFER, vboIds[ 1 ] );
    setShaderVars();
    glDrawElements( GL_TRIANGLES, m_mesh->numTris()*3, GL_UNSIGNED_INT, 0 );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void MeshRenderer::setupTextures()
{
}

void MeshRenderer::setShaderVars()
{
    QGLShaderProgram* program = GLFunctions::getShader( "mesh" );

    program->bind();

    long int offset = 0;
    // Tell OpenGL programmable pipeline how to locate vertex position data

    int vertexLocation = program->attributeLocation( "a_position" );
    program->enableAttributeArray( vertexLocation );
    glVertexAttribPointer( vertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (const void *) offset );

    offset += sizeof(float) * 3;
    int normalLocation = program->attributeLocation( "a_normal" );
    program->enableAttributeArray( normalLocation );
    glVertexAttribPointer( normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (const void *) offset );

}

void MeshRenderer::initGeometry()
{
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vboIds[ 0 ] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_mesh->numTris() * 3 * sizeof(GLuint), m_mesh->getIndexes(), GL_STATIC_DRAW );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vboIds[ 1 ] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_mesh->numVerts() * 6 * sizeof(GLfloat), m_mesh->getVertices(), GL_STATIC_DRAW );

    m_dirty = false;
}
