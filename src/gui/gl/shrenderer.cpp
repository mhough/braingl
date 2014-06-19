/*
 * shrenderer.cpp
 *
 * Created on: 03.07.2012
 * @author Ralph Schurade
 */
#include "shrenderer.h"
#include "shrendererthread.h"
#include "shrendererthread2.h"
#include "glfunctions.h"

#include "../../data/datasets/datasetsh.h"
#include "../../data/enums.h"
#include "../../data/models.h"
#include "../../data/vptr.h"
#include "../../algos/fmath.h"
#include "../../algos/qball.h"

#include "../../data/mesh/tesselation.h"
#include "../../data/mesh/trianglemesh2.h"

#include "../../thirdparty/newmat10/newmat.h"

#include <QGLShaderProgram>
#include <QDebug>

#include <limits>

SHRenderer::SHRenderer( std::vector<ColumnVector>* data ) :
    ObjectRenderer(),
    m_tris( 0 ),
    vboIds( new GLuint[ 3 ] ),
    m_mesh( 0 ),
    m_data( data ),
    m_scaling( 1.0 ),
    m_orient( 0 ),
    m_offset( 0 ),
    m_lod( 0 ),
    m_minMaxScaling( false ),
    m_order( 4 ),
    m_oldLoD( -1 ),
    m_pickId( GLFunctions::getPickIndex() ),
    m_masterThread( 0 ),
    m_meshUpdated( false ),
    m_updateWaiting( false )
{
}

SHRenderer::~SHRenderer()
{
    glDeleteBuffers(1, &( vboIds[ 0 ] ) );
    glDeleteBuffers(1, &( vboIds[ 1 ] ) );
    glDeleteBuffers(1, &( vboIds[ 2 ] ) );
}

void SHRenderer::init()
{
    initializeOpenGLFunctions();
    glGenBuffers( 3, vboIds );
}

void SHRenderer::draw( QMatrix4x4 p_matrix, QMatrix4x4 mv_matrix, int width, int height, int renderMode, PropertyGroup& props )
{
    float alpha = 1.0; //props.get( Fn::Property::D_ALPHA ).toFloat();
    m_renderMode = renderMode;

    m_pMatrix = p_matrix;
    m_mvMatrix = mv_matrix;

    switch ( renderMode )
    {
        case 0:
            break;
        case 1:
        {
            if ( alpha < 1.0 ) // obviously not opaque
            {
                return;
            }
            break;
        }
        default:
        {
            if ( alpha == 1.0  ) // not transparent
            {
                return;
            }
            break;
        }
    }

    setRenderParams( props );

    QGLShaderProgram* program = GLFunctions::getShader( "mesh" );

    program->bind();

    GLFunctions::setupTextures();
    GLFunctions::setTextureUniforms( GLFunctions::getShader( "mesh" ), "maingl" );
    // Set modelview-projection matrix
    program->setUniformValue( "mvp_matrix", p_matrix * mv_matrix );
    program->setUniformValue( "mv_matrixInvert", mv_matrix.inverted() );

    program->setUniformValue( "u_colorMode", 2 );
    program->setUniformValue( "u_colormap", m_colormap );
    program->setUniformValue( "u_color", m_color.redF(), m_color.greenF(), m_color.blueF(), 1.0 );
    program->setUniformValue( "u_selectedMin", m_selectedMin );
    program->setUniformValue( "u_selectedMax", m_selectedMax );
    program->setUniformValue( "u_lowerThreshold", m_lowerThreshold );
    program->setUniformValue( "u_upperThreshold", m_upperThreshold );

    float nx = props.get( Fn::Property::D_NX ).toFloat();
    float ny = props.get( Fn::Property::D_NY ).toFloat();
    float nz = props.get( Fn::Property::D_NZ ).toFloat();
    float sx = props.get( Fn::Property::G_SAGITTAL ).toFloat();
    float sy = props.get( Fn::Property::G_CORONAL ).toFloat();
    float sz = props.get( Fn::Property::G_AXIAL ).toFloat();
    float dx = props.get( Fn::Property::D_DX ).toFloat();
    float dy = props.get( Fn::Property::D_DY ).toFloat();
    float dz = props.get( Fn::Property::D_DZ ).toFloat();

    program->setUniformValue( "u_x", sx * dx + dx / 2.0f );
    program->setUniformValue( "u_y", sy * dy + dy / 2.0f );
    program->setUniformValue( "u_z", sz * dz + dz / 2.0f );
    program->setUniformValue( "u_cutLowerX", false );
    program->setUniformValue( "u_cutLowerY", false );
    program->setUniformValue( "u_cutLowerZ", false );
    program->setUniformValue( "u_cutHigherX", false );
    program->setUniformValue( "u_cutHigherY", false );
    program->setUniformValue( "u_cutHigherZ", false );

    program->setUniformValue( "u_adjustX",   0.0f );
    program->setUniformValue( "u_adjustY", 0.0f );
    program->setUniformValue( "u_adjustZ", 0.0f );

    program->setUniformValue( "u_alpha", alpha );
    program->setUniformValue( "u_renderMode", renderMode );
    program->setUniformValue( "u_canvasSize", width, height );
    program->setUniformValue( "D0", 9 );
    program->setUniformValue( "D1", 10 );
    program->setUniformValue( "D2", 11 );
    program->setUniformValue( "P0", 12 );

    program->setUniformValue( "u_lighting", props.get( Fn::Property::D_LIGHT_SWITCH ).toBool() );
    program->setUniformValue( "u_lightAmbient", props.get( Fn::Property::D_LIGHT_AMBIENT ).toFloat() );
    program->setUniformValue( "u_lightDiffuse", props.get( Fn::Property::D_LIGHT_DIFFUSE ).toFloat() );
    program->setUniformValue( "u_materialAmbient", props.get( Fn::Property::D_MATERIAL_AMBIENT ).toFloat() );
    program->setUniformValue( "u_materialDiffuse", props.get( Fn::Property::D_MATERIAL_DIFFUSE ).toFloat() );
    program->setUniformValue( "u_materialSpecular", props.get( Fn::Property::D_MATERIAL_SPECULAR ).toFloat() );
    program->setUniformValue( "u_materialShininess", props.get( Fn::Property::D_MATERIAL_SHININESS ).toFloat() );


    float pAlpha =  1.0;
    float blue = (float) ( ( m_pickId ) & 0xFF ) / 255.f;
    float green = (float) ( ( m_pickId >> 8 ) & 0xFF ) / 255.f;
    float red = (float) ( ( m_pickId >> 16 ) & 0xFF ) / 255.f;
    program->setUniformValue( "u_pickColor", red, green , blue, pAlpha );

    program->setUniformValue( "u_dims", nx * dx, ny * dy, nz * dz );

    initGeometry( props );

    if ( m_meshUpdated )
    {
        updateMesh();
    }

    if ( !m_mesh )
    {
        return;
    }

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vboIds[ 0 ] );
    glBindBuffer( GL_ARRAY_BUFFER, vboIds[ 1 ] );
    setShaderVars( props );

    glEnable(GL_CULL_FACE);
    glCullFace( GL_BACK );
    glFrontFace( GL_CW );

    glDrawElements( GL_TRIANGLES, m_tris, GL_UNSIGNED_INT, 0 );

    glDisable(GL_CULL_FACE);

    //glShadeModel( GL_SMOOTH );  // XXX not in CoreProfile; use shader
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void SHRenderer::setShaderVars( PropertyGroup& props )
{
    QGLShaderProgram* program = GLFunctions::getShader( "mesh" );

    program->bind();

    intptr_t offset = 0;
    // Tell OpenGL programmable pipeline how to locate vertex position data

    int bufferSize = m_mesh->bufferSize();

    int vertexLocation = program->attributeLocation( "a_position" );
    program->enableAttributeArray( vertexLocation );
    glVertexAttribPointer( vertexLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * bufferSize, (const void *) offset );
    offset += sizeof(float) * 3;

    int normalLocation = program->attributeLocation( "a_normal" );
    program->enableAttributeArray( normalLocation );
    glVertexAttribPointer( normalLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * bufferSize, (const void *) offset );
    offset += sizeof(float) * 3;

    int valueLocation = program->attributeLocation( "a_value" );
    program->enableAttributeArray( valueLocation );
    glVertexAttribPointer( valueLocation, 1, GL_FLOAT, GL_FALSE, sizeof(float) * bufferSize, (const void *) offset );
    offset += sizeof(float) * 1;

    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glBindBuffer( GL_ARRAY_BUFFER, vboIds[ 2 ] );
    int colorLocation = program->attributeLocation( "a_color" );
    program->enableAttributeArray( colorLocation );
    glVertexAttribPointer( colorLocation, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0 );
}

void SHRenderer::initGeometry( PropertyGroup& props )
{
    float x = Models::getGlobal( Fn::Property::G_SAGITTAL ).toFloat();
    float y = Models::getGlobal( Fn::Property::G_CORONAL ).toFloat();
    float z = Models::getGlobal( Fn::Property::G_AXIAL ).toFloat();

    float zoom = Models::getGlobal( Fn::Property::G_ZOOM ).toFloat();
    float moveX = Models::getGlobal( Fn::Property::G_MOVEX ).toFloat();
    float moveY = Models::getGlobal( Fn::Property::G_MOVEY ).toFloat();

    QString s = createSettingsString( { x, y, z, m_orient, zoom, m_minMaxScaling, m_scaling, m_hideNegativeLobes, moveX, moveY, m_lod, m_offset } );
    if ( !m_updateWaiting && ( s == m_previousSettings || m_orient == 0 ) )
    {
        return;
    }
    m_previousSettings = s;

    createMesh( props );
}

void SHRenderer::setRenderParams( PropertyGroup& props )
{
    m_scaling = props.get( Fn::Property::D_SCALING ).toFloat();
    m_offset = props.get( Fn::Property::D_OFFSET ).toInt();
    m_lod = props.get( Fn::Property::D_LOD ).toInt();
    m_minMaxScaling = props.get( Fn::Property::D_MINMAX_SCALING ).toBool();
    m_hideNegativeLobes = props.get( Fn::Property::D_HIDE_NEGATIVE_LOBES ).toBool();
    m_order = props.get( Fn::Property::D_ORDER ).toInt();

    m_orient = 0;
    if ( props.get( Fn::Property::D_RENDER_AXIAL ).toBool() )
    {
        m_orient = 1;
    }
    if ( props.get( Fn::Property::D_RENDER_CORONAL ).toBool() )
    {
        m_orient += 2;
    }
    if ( props.get( Fn::Property::D_RENDER_SAGITTAL ).toBool() )
    {
        m_orient += 4;
    }

    m_colorMode = props.get( Fn::Property::D_COLORMODE ).toInt();
    m_colormap = props.get( Fn::Property::D_COLORMAP ).toInt();
    m_selectedMin = props.get( Fn::Property::D_SELECTED_MIN ).toFloat();
    m_selectedMax = props.get( Fn::Property::D_SELECTED_MAX ).toFloat();
    m_lowerThreshold = props.get( Fn::Property::D_LOWER_THRESHOLD ).toFloat();
    m_upperThreshold = props.get( Fn::Property::D_UPPER_THRESHOLD ).toFloat();
    m_color = props.get( Fn::Property::D_COLOR ).value<QColor>();
}

void SHRenderer::createMesh( PropertyGroup& props )
{
    if ( m_masterThread && m_masterThread->isRunning() )
    {
        m_updateWaiting = true;
        return;
    }

    m_updateWaiting = false;

    m_masterThread = new SHRendererThread2( 0, m_data, m_pMatrix, m_mvMatrix, props );
    connect( m_masterThread, SIGNAL( finished( TriangleMesh2* ) ), this, SLOT( slotNewMeshCreated( TriangleMesh2* ) ) );

    m_masterThread->start();
}

TriangleMesh2* SHRenderer::getMesh()
{
    return m_mesh;
}

void SHRenderer::slotNewMeshCreated( TriangleMesh2* mesh )
{
    m_newMesh = mesh;
    m_meshUpdated = true;
    Models::g()->submit();
}

void SHRenderer::updateMesh()
{
    if ( m_mesh != 0 )
    {
        delete m_mesh;

        glDeleteBuffers( 3, vboIds );
        glGenBuffers( 3, vboIds );
    }

    if ( m_newMesh )
    {
        m_mesh = m_newMesh;
        int bufferSize = m_mesh->bufferSize();

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vboIds[ 0 ] );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_mesh->numTris() * 3 * sizeof(GLuint), m_mesh->getIndexes(), GL_STATIC_DRAW );

        glBindBuffer( GL_ARRAY_BUFFER, vboIds[ 1 ] );
        glBufferData( GL_ARRAY_BUFFER, m_mesh->numVerts() * bufferSize * sizeof(GLfloat), m_mesh->getVertices(), GL_STATIC_DRAW );

        glBindBuffer( GL_ARRAY_BUFFER, vboIds[ 2 ] );
        glBufferData( GL_ARRAY_BUFFER, m_mesh->numVerts() * 4 * sizeof(GLfloat), m_mesh->getVertexColors(), GL_DYNAMIC_DRAW );

        m_tris = m_mesh->numTris() * 3;

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );

        m_newMesh = 0;
    }
    m_meshUpdated = false;
}
