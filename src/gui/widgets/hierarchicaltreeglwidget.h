/*
 * navglwidget.h
 *
 * Created on: Spetember 17, 2013
 * @author Ralph Schurade
 */
#ifndef HIERARCHICALTREEGLWIDGET_H
#define HIERARCHICALTREEGLWIDGET_H

#include "GL/glew.h"

#include <QtOpenGL/QGLWidget>

class TreeRenderer;

class HierarchicalTreeGLWidget: public QGLWidget
{
    Q_OBJECT

public:
    HierarchicalTreeGLWidget( QString name, QWidget *parent = 0, const QGLWidget *shareWidget = 0 );
    ~HierarchicalTreeGLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

public slots:
    void update();
    void setWidgetVisible( bool visible );

signals:

protected:
    void initializeGL();
    void paintGL();
    void resizeGL( int width, int height );
    void mousePressEvent( QMouseEvent *event );
    void mouseReleaseEvent( QMouseEvent *event );
    void mouseMoveEvent( QMouseEvent *event );
    void wheelEvent( QWheelEvent *event );

private:
    TreeRenderer* m_renderer;
    bool m_visible;
};

#endif