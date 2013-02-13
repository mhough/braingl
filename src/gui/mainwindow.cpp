/*
 * mainwindow.cpp
 *
 *  Created on: May 03, 2012
 *      Author: Ralph Schurade
 */

#include "mainwindow.h"

#include "views/datasetinfoview.h"

#include "widgets/glwidget.h"
#include "widgets/fndockwidget.h"
#include "widgets/datasetpropertywidget.h"
#include "widgets/roipropertywidget.h"
#include "widgets/globalpropertywidget.h"
#include "widgets/docknavglwidget.h"
#include "widgets/combinednavglwidget.h"
#include "widgets/singleshwidget.h"
#include "widgets/roiwidget.h"
#include "widgets/datasetlistwidget.h"
#include "widgets/navglwidget.h"
#include "widgets/combinednavglwidget.h"
#include "widgets/toolbar.h"
#include "widgets/statusbar.h"
#include "widgets/shadereditwidget.h"
#include "widgets/colormapeditwidget.h"

#include "gl/glfunctions.h"

#include "../data/datastore.h"
#include "../data/globalpropertymodel.h"
#include "../data/roimodel.h"
#include "../data/loader.h"
#include "../data/writer.h"
#include "../data/vptr.h"
#include "../data/enums.h"

#include <QtGui/QtGui>

int MainWindow::screenshotNumber = 0;

MainWindow::MainWindow( DataStore* dataStore, GlobalPropertyModel* globalProps, ROIModel* roiModel, bool debug ) :
	QMainWindow(),
    m_dataStore( dataStore ),
    m_globalProps( globalProps ),
    m_roiModel( roiModel ),
    m_debug( debug )
{
	m_centralWidget = new QMainWindow();
	m_centralWidget->setObjectName( "central widget" );
	m_centralWidget->setDockOptions( QMainWindow::AnimatedDocks |  QMainWindow::AllowNestedDocks | QMainWindow::AllowTabbedDocks );
	m_centralWidget->setDocumentMode( true );
	setCentralWidget( m_centralWidget );

	ColormapBase colormap;
	GLFunctions::addColormap2( colormap );

	ColormapBase cmap( QColor( 0, 0, 255 ), QColor( 255, 0, 0 ) );
    cmap.insertValue( 0.25, QColor( 0, 255, 255 ) );
    cmap.insertValue( 0.5, QColor( 0, 255, 0 ) );
    cmap.insertValue( 0.75, QColor( 255, 255, 0 ) );
    GLFunctions::addColormap2( cmap );

    ColormapBase cmap2( QColor( 0, 0, 255 ), QColor( 255, 0, 255 ) );
    cmap2.insertValue( 0.2, QColor( 0, 255, 255 ) );
    cmap2.insertValue( 0.4, QColor( 0, 255, 0 ) );
    cmap2.insertValue( 0.6, QColor( 255, 255, 0 ) );
    cmap2.insertValue( 0.8, QColor( 255, 0, 0 ) );
    GLFunctions::addColormap2( cmap2 );

    ColormapBase cmap3( QColor( 0, 0, 255 ), QColor( 255, 0, 0 ) );
    cmap3.insertValue( 0.5, QColor( 255, 255, 255 ) );
    GLFunctions::addColormap2( cmap3 );

    createActions();
    createMenus();
    createToolBars();
    createDockWindows();


    // this needs to be done after the view is created
    m_toolsToolBar->setSelectionModel( m_datasetWidget->selectionModel() );

    createStatusBar();

    setWindowTitle( tr( "Fibernavigator 2" ) );

    setUnifiedTitleAndToolBarOnMac( true );

	loadSettings();
}

void MainWindow::closeEvent( QCloseEvent *event )
{
    saveSettings();
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue( "mainWindowGeometry", saveGeometry() );
    settings.setValue( "mainWindowState", saveState() );

    settings.setValue( "centralWidgetGeometry", m_centralWidget->saveGeometry() );
    settings.setValue( "centralWidgetState", m_centralWidget->saveState() );

    settings.setValue( "lockDockTitles", lockDockTitlesAct->isChecked() );
    settings.setValue( Fn::Global2String::s( Fn::Global::LAST_PATH ), m_globalProps->data( m_globalProps->index( (int)Fn::Global::LAST_PATH, 0 ) ) );
    settings.setValue( Fn::Global2String::s( Fn::Global::BACKGROUND_COLOR_MAIN ), m_globalProps->data( m_globalProps->index( (int)Fn::Global::BACKGROUND_COLOR_MAIN, 0 ) ) );
    settings.setValue( Fn::Global2String::s( Fn::Global::BACKGROUND_COLOR_COMBINED ), m_globalProps->data( m_globalProps->index( (int)Fn::Global::BACKGROUND_COLOR_COMBINED, 0 ) ) );
    settings.setValue( Fn::Global2String::s( Fn::Global::BACKGROUND_COLOR_NAV1 ), m_globalProps->data( m_globalProps->index( (int)Fn::Global::BACKGROUND_COLOR_NAV1, 0 ) ) );
    settings.setValue( Fn::Global2String::s( Fn::Global::BACKGROUND_COLOR_NAV2 ), m_globalProps->data( m_globalProps->index( (int)Fn::Global::BACKGROUND_COLOR_NAV2, 0 ) ) );
    settings.setValue( Fn::Global2String::s( Fn::Global::BACKGROUND_COLOR_NAV3 ), m_globalProps->data( m_globalProps->index( (int)Fn::Global::BACKGROUND_COLOR_NAV3, 0 ) ) );
    settings.setValue( Fn::Global2String::s( Fn::Global::CROSSHAIR_COLOR ), m_globalProps->data( m_globalProps->index( (int)Fn::Global::CROSSHAIR_COLOR, 0 ) ) );
    settings.setValue( Fn::Global2String::s( Fn::Global::SHOW_NAV_SLIDERS ), m_globalProps->data( m_globalProps->index( (int)Fn::Global::SHOW_NAV_SLIDERS, 0 ) ) );
}

void MainWindow::loadSettings()
{
    QSettings settings;
    restoreGeometry( settings.value( "mainWindowGeometry" ).toByteArray() );
    restoreState( settings.value( "mainWindowState" ).toByteArray() );

    m_centralWidget->restoreGeometry( settings.value( "centralWidgetGeometry" ).toByteArray() );
    m_centralWidget->restoreState( settings.value( "centralWidgetState" ).toByteArray() );

    if ( settings.contains( "lockDockTitles") )
    {
        if ( settings.value( "lockDockTitles" ).toBool() )
        {
            lockDockTitlesAct->activate( QAction::Trigger );
        }
    }
    loadSetting( settings, Fn::Global::LAST_PATH );
    loadSetting( settings, Fn::Global::BACKGROUND_COLOR_MAIN );
    loadSetting( settings, Fn::Global::BACKGROUND_COLOR_COMBINED );
    loadSetting( settings, Fn::Global::BACKGROUND_COLOR_NAV1 );
    loadSetting( settings, Fn::Global::BACKGROUND_COLOR_NAV2 );
    loadSetting( settings, Fn::Global::BACKGROUND_COLOR_NAV3 );
    loadSetting( settings, Fn::Global::CROSSHAIR_COLOR );
    loadSetting( settings, Fn::Global::SHOW_NAV_SLIDERS );
}

void MainWindow::loadSetting( QSettings &settings, Fn::Global setting )
{
    if ( settings.contains( Fn::Global2String::s( setting) ) )
    {
        QVariant s = settings.value( Fn::Global2String::s( setting ) );
        m_globalProps->setData( m_globalProps->index( (int)setting, 0 ), s );
    }
}

void MainWindow::print()
{
}

void MainWindow::open()
{
    QString fn = m_globalProps->data( m_globalProps->index( (int)Fn::Global::LAST_PATH, 0 ) ).toString();
    QStringList fileNames = QFileDialog::getOpenFileNames( this, "Open File", fn );
    for ( int i = 0; i < fileNames.size(); ++i )
    {
        load( fileNames[i] );
    }

}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast< QAction * >( sender() );
    if ( action )
    {
        load( action->data().toString() );
    }
}

void MainWindow::load( QString fileName )
{
    if ( !fileName.isEmpty() )
    {
        Loader loader;
        loader.setFilename( QDir( fileName ) );
        if ( loader.load() )
        {
            for ( int k = 0; k < loader.getNumDatasets(); ++k )
            {
                m_dataStore->setData( m_dataStore->index( m_dataStore->rowCount(), (int)Fn::Property::NEW_DATASET ), VPtr<Dataset>::asQVariant( loader.getDataset( k ) ), Qt::DisplayRole );
            }
            QFileInfo fi( fileName );
            QDir dir = fi.absoluteDir();
            QString lastPath = dir.absolutePath();
            m_globalProps->setData( m_globalProps->index( (int)Fn::Global::LAST_PATH, 0 ), lastPath );

            setCurrentFile(fileName);
        }
    }
}

void MainWindow::setCurrentFile( const QString &fileName )
{
    curFile = fileName;
    setWindowFilePath( curFile );

    QSettings settings;
    QStringList files = settings.value( "recentFileList" ).toStringList();
    files.removeAll( fileName );
    files.prepend( fileName );
    while ( files.size() > MaxRecentFiles )
        files.removeLast();

    settings.setValue( "recentFileList", files );

    foreach (QWidget *widget, QApplication::topLevelWidgets())
    {
        MainWindow *mainWin = qobject_cast< MainWindow * >( widget );
        if ( mainWin )
            mainWin->updateRecentFileActions();
    }
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value( "recentFileList" ).toStringList();

    int numRecentFiles = qMin( files.size(), (int) MaxRecentFiles );

    for ( int i = 0; i < numRecentFiles; ++i )
    {
        QString text = tr( "&%1 %2" ).arg( i + 1 ).arg( strippedName( files[ i ] ) );
        recentFileActs[ i ]->setText( text );
        recentFileActs[ i ]->setData( files[ i ] );
        recentFileActs[ i ]->setVisible( true );
    }
    for ( int j = numRecentFiles; j < MaxRecentFiles; ++j )
        recentFileActs[ j ]->setVisible( false );

    separatorAct->setVisible( numRecentFiles > 0 );
}

QString MainWindow::strippedName( const QString &fullFileName )
{
    return QFileInfo( fullFileName ).fileName();
}

void MainWindow::save()
{
    QString fn = m_globalProps->data( m_globalProps->index( (int)Fn::Global::LAST_PATH, 0 ) ).toString();
    QString fileName = QFileDialog::getSaveFileName( this, "Save File", fn );

    if ( !fileName.isEmpty() )
    {
        QDir dir;
        if ( dir.exists( fileName ) )
        {
            QMessageBox msgBox;
            msgBox.setText("File already exists.");
            msgBox.setInformativeText("Do you want to overwrite it?");
            msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Save);
            int ret = msgBox.exec();
            switch ( ret )
            {
                case QMessageBox::Save :
                {
                    Dataset* ds = VPtr<Dataset>::asPtr( m_dataStore->data( m_dataStore->index( m_datasetWidget->getSelected(), (int)Fn::Property::DATASET_POINTER) ) );
                    Writer writer( ds, fileName );
                    writer.save();
                    break;
                }
                case QMessageBox::Cancel :
                    break;
            }
        }
        else
        {
            Dataset* ds = VPtr<Dataset>::asPtr( m_dataStore->data( m_dataStore->index( m_datasetWidget->getSelected(), (int)Fn::Property::DATASET_POINTER) ) );
            Writer writer( ds, fileName );
            writer.save();
        }

        QFileInfo fi( fileName );
        dir = fi.absoluteDir();
        QString lastPath = dir.absolutePath();
        m_globalProps->setData( m_globalProps->index( (int)Fn::Global::LAST_PATH, 0 ), lastPath );
    }
}

void MainWindow::undo()
{
}


void MainWindow::about()
{
    QMessageBox::about( this, tr( "About Fibernavigator 2" ),
            tr( "The <b>Fibernavigator 2</b> will be the replacement "
                    "of Fibernavigator and so much more." ) );
}

void MainWindow::createActions()
{
    openAct = new QAction( QIcon( ":/icons/open.png" ), tr( "&Open..." ), this );
    openAct->setShortcuts( QKeySequence::Open );
    openAct->setStatusTip( tr( "Load Dataset" ) );
    connect( openAct, SIGNAL(triggered()), this, SLOT(open()) );

    saveAct = new QAction( QIcon( ":/icons/save.png" ), tr( "&Save..." ), this );
    saveAct->setShortcuts( QKeySequence::Save );
    saveAct->setStatusTip( tr( "Save the current form letter" ) );
    connect( saveAct, SIGNAL(triggered()), this, SLOT(save()) );

    printAct = new QAction( QIcon( ":/icons/print.png" ), tr( "&Print..." ), this );
    printAct->setShortcuts( QKeySequence::Print );
    printAct->setStatusTip( tr( "Print the current form letter" ) );
    connect( printAct, SIGNAL(triggered()), this, SLOT(print()) );

    quitAct = new QAction( tr( "&Quit" ), this );
    quitAct->setShortcuts( QKeySequence::Quit );
    quitAct->setStatusTip( tr( "Quit the application" ) );
    connect( quitAct, SIGNAL(triggered()), this, SLOT(close()) );

    screenshotAct = new QAction( tr( "Screenshot" ), this );
    screenshotAct->setStatusTip( tr( "Sreenshot" ) );
    connect( screenshotAct, SIGNAL(triggered()), this, SLOT(screenshot()) );

    aboutAct = new QAction( tr( "&About" ), this );
    aboutAct->setStatusTip( tr( "Show the application's About box" ) );
    connect( aboutAct, SIGNAL(triggered()), this, SLOT(about()) );

    aboutQtAct = new QAction( tr( "About &Qt" ), this );
    aboutQtAct->setStatusTip( tr( "Show the Qt library's About box" ) );
    connect( aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()) );

    showAxialAct = new QAction( QIcon( ":/icons/axial.png" ), tr( "Show Axial Slice" ), this );
    showAxialAct->setStatusTip( tr( "Toggle rendering of the axial slice." ) );
    showAxialAct->setCheckable( true );
    showAxialAct->setChecked( true );
    connect( showAxialAct, SIGNAL( toggled( bool ) ), this, SLOT( slotToggleAxialSlice() ) );

    showCoronalAct = new QAction( QIcon( ":/icons/coronal.png" ), tr( "Show Coronal Slice" ), this );
    showCoronalAct->setStatusTip( tr( "Toggle rendering of the coronal slice." ) );
    showCoronalAct->setCheckable( true );
    showCoronalAct->setChecked( true );
    connect( showCoronalAct, SIGNAL( toggled( bool ) ), this, SLOT( slotToggleCoronalSlice() ) );

    showSagittalAct = new QAction( QIcon( ":/icons/sagittal.png" ), tr( "Show Sagittal Slice" ), this );
    showSagittalAct->setStatusTip( tr( "Toggle rendering of the sagittal slice." ) );
    showSagittalAct->setCheckable( true );
    showSagittalAct->setChecked( true );
    connect( showSagittalAct, SIGNAL( toggled( bool ) ), this, SLOT( slotToggleSagittalSlice() ) );

    standardViewAxialAct = new QAction( QIcon( ":/icons/axial.png" ), tr( "Axial View" ), this );
    standardViewAxialAct->setStatusTip( tr( "Toggle rendering of the axial slice." ) );
    connect( standardViewAxialAct, SIGNAL( triggered() ), this, SLOT( slotStandardAxialView() ) );

    standardViewCoronalAct = new QAction( QIcon( ":/icons/coronal.png" ), tr( "Coronal View" ), this );
    standardViewCoronalAct->setStatusTip( tr( "Toggle rendering of the coronal slice." ) );
    connect( standardViewCoronalAct, SIGNAL( triggered() ), this, SLOT( slotStandardCoronalView() ) );

    standardViewSagittalAct = new QAction( QIcon( ":/icons/sagittal.png" ), tr( "Sagittal view" ), this );
    standardViewSagittalAct->setStatusTip( tr( "Toggle rendering of the sagittal slice." ) );
    connect( standardViewSagittalAct, SIGNAL( triggered() ), this, SLOT( slotStandardSagittalView() ) );

    toggleShaderEditAct = new QAction( QIcon( ":/icons/cat.png" ), tr( "Toggle shader edit" ), this );
    toggleShaderEditAct->setStatusTip( tr( "Toggle the shader edit widget." ) );
    connect( toggleShaderEditAct, SIGNAL( triggered() ), this, SLOT( slotToggleShaderEdit() ) );

    lockDockTitlesAct = new QAction( tr( "Lock Widgets" ), this );
    lockDockTitlesAct->setStatusTip( tr( "Lock all dock widgets in place" ) );
    lockDockTitlesAct->setCheckable( true );
    lockDockTitlesAct->setChecked( false );
    connect( lockDockTitlesAct, SIGNAL( toggled( bool ) ), this, SLOT( slotToggleDockTitles( bool ) ) );

    renderCrosshairsAct = new QAction( tr( "Render Crosshairs" ), this );
    renderCrosshairsAct->setStatusTip( tr( "render crosshairs in navigation widgets." ) );
    renderCrosshairsAct->setCheckable( true );
    renderCrosshairsAct->setChecked( true );
    connect( renderCrosshairsAct, SIGNAL( toggled( bool ) ), this, SLOT( slotRenderCrosshairs( bool ) ) );

    newSelectionBoxAct = new QAction( QIcon( ":/icons/box.png" ), tr( "New ROI" ), this );
    newSelectionBoxAct->setStatusTip( tr( "Add a new ROI." ) );
    connect( newSelectionBoxAct, SIGNAL( triggered() ), this, SLOT( slotNewSelectionBox() ) );

    for ( int i = 0; i < MaxRecentFiles; ++i )
    {
        recentFileActs[ i ] = new QAction( this );
        recentFileActs[ i ]->setVisible( false );
        connect( recentFileActs[ i ], SIGNAL(triggered()), this, SLOT(openRecentFile()) );
    }
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu( tr( "&File" ) );
    fileMenu->addAction( openAct );
    fileMenu->addAction( saveAct );
    fileMenu->addAction( printAct );
    separatorAct = fileMenu->addSeparator();

    for ( int i = 0; i < MaxRecentFiles; ++i )
    {
        fileMenu->addAction( recentFileActs[ i ] );
    }
    fileMenu->addSeparator();
    fileMenu->addAction( quitAct );
    updateRecentFileActions();

    optionMenu = menuBar()->addMenu( tr( "&Options" ) );
    optionMenu->addAction( lockDockTitlesAct );
    optionMenu->addAction( renderCrosshairsAct );

    viewMenu = menuBar()->addMenu( tr( "&View" ) );

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu( tr( "&Help" ) );
    helpMenu->addAction( screenshotAct );
    helpMenu->addAction( aboutAct );
    helpMenu->addAction( aboutQtAct );
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar( tr( "File" ) );
    fileToolBar->addAction( openAct );
    fileToolBar->addAction( saveAct );
    //fileToolBar->addAction( printAct );
    if ( m_debug )
    {
        fileToolBar->addAction( toggleShaderEditAct );
    }
    fileToolBar->setObjectName( "fileToolbar");

    editToolBar = addToolBar( tr( "Edit" ) );
    editToolBar->addAction( showAxialAct );
    editToolBar->addAction( showCoronalAct );
    editToolBar->addAction( showSagittalAct );
    editToolBar->addAction( standardViewAxialAct );
    editToolBar->addAction( standardViewCoronalAct );
    editToolBar->addAction( standardViewSagittalAct );
    editToolBar->addAction( newSelectionBoxAct );
    editToolBar->setObjectName( "editToolbar");

    m_toolsToolBar = new ToolBar( tr( "Tools" ), this );
    m_toolsToolBar->setModel( m_dataStore );
    addToolBar( m_toolsToolBar );
}

void MainWindow::createStatusBar()
{
    StatusBar* sbar = new StatusBar( m_dataStore, m_globalProps, this );
    sbar->setSelectionModel( m_datasetWidget->selectionModel() );
    this->setStatusBar( sbar );
}

void MainWindow::createDockWindows()
{
    m_datasetWidget = new DatasetListWidget();
	m_datasetWidget->setModel( m_dataStore );
	FNDockWidget* dockDSW = new FNDockWidget( QString("Dataset List"), m_datasetWidget, this );
	addDockWidget( Qt::LeftDockWidgetArea, dockDSW );
	viewMenu->addAction( dockDSW->toggleViewAction() );
	connect( lockDockTitlesAct, SIGNAL( triggered() ), dockDSW, SLOT( toggleTitleWidget() ) );

	connect( m_datasetWidget, SIGNAL( moveSelectedItemUp( int ) ), m_dataStore, SLOT( moveItemUp( int ) ) );
	connect( m_datasetWidget, SIGNAL( moveSelectedItemDown( int ) ), m_dataStore, SLOT( moveItemDown( int ) ) );
	connect( m_datasetWidget, SIGNAL( deleteSelectedItem( int ) ), m_dataStore, SLOT( deleteItem( int ) ) );

	ROIWidget* m_roiWidget = new ROIWidget( m_roiModel, this );
	FNDockWidget* dockSBW = new FNDockWidget( QString("ROIs"), m_roiWidget, this );
    addDockWidget( Qt::LeftDockWidgetArea, dockSBW );
    viewMenu->addAction( dockSBW->toggleViewAction() );
    connect( lockDockTitlesAct, SIGNAL( triggered() ), dockSBW, SLOT( toggleTitleWidget() ) );
    connect( newSelectionBoxAct, SIGNAL( triggered() ), m_roiWidget, SLOT( addBox() ) );

	DatasetPropertyWidget* dsProperties = new DatasetPropertyWidget( this );
	FNDockWidget* dockDSP = new FNDockWidget( QString("dataset properties"), dsProperties, this );
    addDockWidget( Qt::LeftDockWidgetArea, dockDSP );
    dsProperties->setModel( m_dataStore );
    dsProperties->setSelectionModel( m_datasetWidget->selectionModel() );
    viewMenu->addAction( dockDSP->toggleViewAction() );
    connect( lockDockTitlesAct, SIGNAL( triggered() ), dockDSP, SLOT( toggleTitleWidget() ) );

    ROIPropertyWidget* roiProperties = new ROIPropertyWidget( this );
    FNDockWidget* dockRP = new FNDockWidget( QString("roi properties"), roiProperties, this );
    addDockWidget( Qt::LeftDockWidgetArea, dockRP );
    roiProperties->setModel( m_roiModel );
    roiProperties->setSelectionModel( m_roiWidget->selectionModel() );
    viewMenu->addAction( dockRP->toggleViewAction() );
    connect( lockDockTitlesAct, SIGNAL( triggered() ), dockRP, SLOT( toggleTitleWidget() ) );


    GlobalPropertyWidget* globalProperties = new GlobalPropertyWidget( this );
    FNDockWidget* dockGP = new FNDockWidget( QString("Global Properties"), globalProperties, this );
    addDockWidget( Qt::LeftDockWidgetArea, dockGP );
    globalProperties->setModel( m_globalProps );
    viewMenu->addAction( dockGP->toggleViewAction() );
    connect( lockDockTitlesAct, SIGNAL( triggered() ), dockGP, SLOT( toggleTitleWidget() ) );

	DatasetInfoView *dsInfo = new DatasetInfoView( this );
	FNDockWidget* dockDSI = new FNDockWidget( QString("dataset info"), dsInfo, this );
	addDockWidget( Qt::BottomDockWidgetArea, dockDSI );
	dsInfo->setModel( m_dataStore );
    dsInfo->setSelectionModel( m_datasetWidget->selectionModel() );
    viewMenu->addAction( dockDSI->toggleViewAction() );
    connect( lockDockTitlesAct, SIGNAL( triggered() ), dockDSI, SLOT( toggleTitleWidget() ) );

    // GL Widgets

    mainGLWidget = new GLWidget( m_dataStore, m_globalProps, m_roiModel, m_roiWidget->selectionModel() );
    FNDockWidget* dockMainGL = new FNDockWidget( QString("main gl"), mainGLWidget, this );
    m_centralWidget->addDockWidget( Qt::LeftDockWidgetArea, dockMainGL );
    viewMenu->addAction( dockMainGL->toggleViewAction() );
    connect( lockDockTitlesAct, SIGNAL( triggered() ), dockMainGL, SLOT( toggleTitleWidget() ) );

    DockNavGLWidget* nav1 = new DockNavGLWidget( m_dataStore, m_globalProps, QString("axial"), 2, this, mainGLWidget );
    FNDockWidget* dockNav1 = new FNDockWidget( QString("axial"), nav1, this );
    m_centralWidget->addDockWidget( Qt::RightDockWidgetArea, dockNav1 );
    viewMenu->addAction( dockNav1->toggleViewAction() );
    connect( lockDockTitlesAct, SIGNAL( triggered() ), dockNav1, SLOT( toggleTitleWidget() ) );

    DockNavGLWidget* nav2 = new DockNavGLWidget( m_dataStore, m_globalProps, QString( "sagittal" ), 0, this, mainGLWidget );
    FNDockWidget* dockNav2 = new FNDockWidget( QString("sagittal"), nav2, this );
    m_centralWidget->addDockWidget( Qt::RightDockWidgetArea, dockNav2 );
    viewMenu->addAction( dockNav2->toggleViewAction() );
    connect( lockDockTitlesAct, SIGNAL( triggered() ), dockNav2, SLOT( toggleTitleWidget() ) );

    DockNavGLWidget* nav3 = new DockNavGLWidget( m_dataStore, m_globalProps, QString( "coronal" ), 1, this, mainGLWidget );
    FNDockWidget* dockNav3 = new FNDockWidget( QString("axial"), nav3, this );
    m_centralWidget->addDockWidget( Qt::RightDockWidgetArea, dockNav3 );
    viewMenu->addAction( dockNav3->toggleViewAction() );
    connect( lockDockTitlesAct, SIGNAL( triggered() ), dockNav3, SLOT( toggleTitleWidget() ) );

    CombinedNavGLWidget* nav4 = new CombinedNavGLWidget( m_dataStore, m_globalProps, QString( "combined" ), this, mainGLWidget );
    FNDockWidget* dockNav4 = new FNDockWidget( QString("Combined Nav"), nav4, this );
    m_centralWidget->addDockWidget( Qt::LeftDockWidgetArea, dockNav4 );
    viewMenu->addAction( dockNav4->toggleViewAction() );
    connect( lockDockTitlesAct, SIGNAL( triggered() ), dockNav4, SLOT( toggleTitleWidget() ) );

    SingleSHWidget* sshw = new SingleSHWidget( m_dataStore, m_globalProps, QString( "single sh" ), this, mainGLWidget );
    FNDockWidget* dockSSHW = new FNDockWidget( QString("single sh" ), sshw, this );
    m_centralWidget->addDockWidget( Qt::LeftDockWidgetArea, dockSSHW );
    viewMenu->addAction( dockSSHW->toggleViewAction() );
    connect( lockDockTitlesAct, SIGNAL( triggered() ), dockSSHW, SLOT( toggleTitleWidget() ) );

    ColormapEditWidget* colormapEditWidget = new ColormapEditWidget( this );
    FNDockWidget* dockCE = new FNDockWidget( QString("colormap edit"), colormapEditWidget, this );
    addDockWidget( Qt::LeftDockWidgetArea, dockCE );
    viewMenu->addAction( dockCE->toggleViewAction() );
    connect( lockDockTitlesAct, SIGNAL( triggered() ), dockCE, SLOT( toggleTitleWidget() ) );


    m_centralWidget->tabifyDockWidget( dockSSHW, dockNav4 );
    m_centralWidget->tabifyDockWidget( dockNav4, dockMainGL );
}

void MainWindow::slotToggleAxialSlice()
{
    m_globalProps->setData( m_globalProps->index( (int)Fn::Global::SHOW_AXIAL, 0 ), showAxialAct->isChecked() );
}

void MainWindow::slotToggleCoronalSlice()
{
    m_globalProps->setData( m_globalProps->index( (int)Fn::Global::SHOW_CORONAL, 0 ), showCoronalAct->isChecked() );
}

void MainWindow::slotToggleSagittalSlice()
{
    m_globalProps->setData( m_globalProps->index( (int)Fn::Global::SHOW_SAGITTAL, 0 ), showSagittalAct->isChecked() );
}

void  MainWindow::slotStandardAxialView()
{
    m_globalProps->setData( m_globalProps->index( (int)Fn::Global::VIEW, 0 ), (int)Fn::Orient::AXIAL );
    mainGLWidget->setView( Fn::Orient::AXIAL );
}

void  MainWindow::slotStandardCoronalView()
{
    m_globalProps->setData( m_globalProps->index( (int)Fn::Global::VIEW, 0 ), (int)Fn::Orient::CORONAL );
    mainGLWidget->setView( Fn::Orient::CORONAL );
}

void  MainWindow::slotStandardSagittalView()
{
    m_globalProps->setData( m_globalProps->index( (int)Fn::Global::VIEW, 0 ), (int)Fn::Orient::SAGITTAL );
    mainGLWidget->setView( Fn::Orient::SAGITTAL );
}

void MainWindow::slotToggleShaderEdit()
{
//    m_shaderEditWidget = new ShaderEditWidget( this );
//    m_centralTabWidget->addTab( m_shaderEditWidget, "shader edit" );
//    m_centralTabWidget->setCurrentWidget( m_shaderEditWidget );
}

void MainWindow::slotToggleDockTitles( bool value )
{
}

void MainWindow::slotRenderCrosshairs( bool value )
{
    m_globalProps->setData( m_globalProps->index( (int)Fn::Global::RENDER_CROSSHAIRS, 0 ), value );
}

void MainWindow::slotNewSelectionBox()
{
}

void MainWindow::screenshot()
{
    QImage* image = mainGLWidget->screenshot();
    image->save( m_globalProps->data( m_globalProps->index( (int)Fn::Global::LAST_PATH, 0 ) ).toString() +
                  QString("/screenshot_") +
                  QString::number( screenshotNumber++ ) +
                  QString( ".png" ), "PNG" );
}
