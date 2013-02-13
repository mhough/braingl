/*
 * colormapwidget.cpp
 *
 *  Created on: Jan 29, 2013
 *      Author: Ralph Schurade
 */
#include "colormapwidget.h"
#include "../../gl/glfunctions.h"
#include "../../../algos/colormapbase.h"

#include <QtGui/QtGui>

ColormapWidget::ColormapWidget( int width ) :
    m_colormap( 0 ),
    m_width( width ),
    m_lowerThreshold( 0.0 ),
    m_upperThreshold( 1.0 ),
    m_min( 0.0 ),
    m_max( 0.0 )
{
    QVBoxLayout* vLayout = new QVBoxLayout();

//    QHBoxLayout* hLayout = new QHBoxLayout();
//    m_nlabel = new QLabel( QString("colormap"), this );
//    hLayout->addWidget( m_nlabel );
//    hLayout->addStretch();

    QHBoxLayout* hLayout2 = new QHBoxLayout();
    m_nlabel = new QLabel( this );

    m_image = createImage( width );
    QPixmap pix( width, 20 );
    pix.convertFromImage( *m_image );
    m_nlabel->setPixmap( pix );
    hLayout2->addWidget( m_nlabel );

    //vLayout->addLayout( hLayout );
    vLayout->addLayout( hLayout2 );
    vLayout->setContentsMargins( 1,1,1,1 );

    setLayout( vLayout );

    setFrameStyle( QFrame::Panel | QFrame::Raised );

    setStyleSheet( "QLabel { font:  bold 12px; margin-bottom: -1px }"
                     "QLineEdit { font: 12px; max-height: 14px; margin-top: -1px }"
                   );
}

ColormapWidget::~ColormapWidget()
{
}

QImage* ColormapWidget::createImage( int width )
{
    QImage* image = new QImage( width, 20, QImage::Format_RGB32 );
    for ( int i = 0; i < width; ++i )
    {
        QColor c;
        int g = 0;

        ColormapBase cmap = GLFunctions::getColormap( m_colormap );

        float value = ( ( (float)i / (float)width ) - m_lowerThreshold ) / ( m_upperThreshold - m_lowerThreshold );
        c = cmap.getColor( qMax( 0.0f, qMin( 1.0f, value ) ) );

        for ( int k = 0; k < 20; ++k )
        {
            image->setPixel( i, k, c.rgba() );
        }
    }

    return image;
}

void ColormapWidget::setLowerThreshold( float value )
{
    m_lowerThreshold = qMax( 0.0f, qMin( 1.0f, value / m_max ) );
    m_image = createImage( m_width );
    QPixmap pix( m_width, 20 );
    pix.convertFromImage( *m_image );
    m_nlabel->setPixmap( pix );
}

void ColormapWidget::setUpperThreshold( float value )
{
    m_upperThreshold = qMax( 0.0f, qMin( 1.0f, value / m_max ) );
    m_image = createImage( m_width );
    QPixmap pix( m_width, 20 );
    pix.convertFromImage( *m_image );
    m_nlabel->setPixmap( pix );
}

void ColormapWidget::setMin( float value )
{
    m_min = value;
}

void ColormapWidget::setMax( float value )
{
    m_max = value;
}


void ColormapWidget::setColormap( int value )
{
    m_colormap = value;
    m_image = createImage( m_width );
    QPixmap pix( m_width, 20 );
    pix.convertFromImage( *m_image );
    m_nlabel->setPixmap( pix );
}
