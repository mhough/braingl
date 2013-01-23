/*
 * propertyint.h
 *
 *  Created on: Jan 17, 2013
 *      Author: schurade
 */

#ifndef PROPERTYINT_H_
#define PROPERTYINT_H_

#include "property.h"

class SliderWithEditInt;
class QWidget;

class PropertyInt : public Property
{
    Q_OBJECT

public:
    PropertyInt( FN_PROPERTY name, int value = 0, int min = std::numeric_limits<int>::min(), int max = std::numeric_limits<int>::max() );
    virtual ~PropertyInt();

    QWidget* getWidget();

    QVariant getValue();

    void setValue( QVariant value );
    void setMin( int min );
    void setMax( int max );


private:
    int m_value;
    int m_min;
    int m_max;

    SliderWithEditInt* m_widget;

private slots:
    void widgetChanged( int value, int id );

signals:
    void valueChanged();

};

#endif /* PROPERTYINT_H_ */