/*
 * newdatasetwidget.h
 *
 * Created on: 26.01.2013
 * @author Ralph Schurade
 */

#ifndef NEWDATASETWIDGET_H_
#define NEWDATASETWIDGET_H_

#include "../../../data/enums.h"

#include <QAbstractItemModel>
#include <QtGui>

class QProgressBar;
class QPushButton;
class Dataset;
class DatasetScalar;
class SelectWithLabel;
class SliderWithEdit;
class SliderWithEditInt;

class NewDatasetWidget : public QWidget
{
    Q_OBJECT

public:
    NewDatasetWidget( QWidget* parent = 0 );
    virtual ~NewDatasetWidget();


private:
    QModelIndex createIndex( int branch, int pos, int column );

    QVBoxLayout* m_layout;

    QPushButton* m_startButton;

    SelectWithLabel* m_modeSelect;
    SelectWithLabel* m_sourceSelect;

    SliderWithEditInt* m_nX;
    SliderWithEditInt* m_nY;
    SliderWithEditInt* m_nZ;
    SliderWithEdit* m_dX;
    SliderWithEdit* m_dY;
    SliderWithEdit* m_dZ;

private slots:
    void start();
    void modeChanged( int mode );

    void copyWithRois( DatasetScalar* source, QVector<float> &target );
    void copy( int branch, int pos, DatasetScalar* source, QVector<float> &target );

signals:
};

#endif /* TENSORTRACKWIDGET_H_ */
