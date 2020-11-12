//QRSImageWindow.h

#ifndef QRSIMAGEWINDOW_H
#define QRSIMAGEWINDOW_H

#include "CRSImage.h"
#include "CImgProcess.h"
#include <QMainWindow>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QTableWidget>
#include <QScrollArea>
#include <QLabel>
#include <QImage>
#include <QMenu>
#include <QAction>
#include <QPushButton>

namespace Ui
{
    class QRSImageWindow;
}

class QRSImageWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit QRSImageWindow(QWidget* parent = 0);
    ~QRSImageWindow();
protected:
    void resizeEvent(QResizeEvent* event);
    void wheelEvent(QWheelEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
private slots:
    void OpenImage();
    void OpenPhoto();
    void CloseImage();
    void SaveAsFile();
    void Exit();
    void Information();
    void Statistics();
    void Covariance();
    void Chart();
    void InitImage();
    void RotateImage();
    void Zoom();
    void Filter();
    void Help();
    void ChannalChanged();
    void Bayes();
    void Fisher();
    void ChartChanged();
private:
    Ui::QRSImageWindow *ui;
    void CreateActions();
    void CreateGroupBoxTool();
    void CreateWidgetImage();
    void CreateComboBox();
    void ResizeWidget();
    void ActionControl(bool bFlag);
    QApplication* Application;
    CRSImage RSImage;
    CImgProcess ImgProcess;
    QGroupBox* m_Tool_GroupBox;
    QScrollArea* m_Image_ScrollArea;
    QWidget* m_Image_Widget;
    QLabel* m_Image_Label;
    QImage m_Image;
    QComboBox* m_RedChannel_ComboBox;
    QComboBox* m_GreenChannel_ComboBox;
    QComboBox* m_BlueChannel_ComboBox;
    QPushButton* m_Bayes_Btn;
    QPushButton* m_Fisher_Btn;
    QComboBox* m_ChartBand_ComboBox;
    QComboBox* m_ChartType_ComboBox;
    QChartView* m_Chart_ChartView;
    QAction* m_OpenImage_Action;
    QAction* m_OpenPhoto_Action;
    QAction* m_CloseImage_Action;
    QAction* m_SaveAsFile_Action;
    QAction* m_Exit_Action;
    QAction* m_Information_Action;
    QAction* m_Statistics_Action;
    QAction* m_Covariance_Action;
    QAction* m_Chart_Action;
    QAction* m_InitImage_Action;
    QAction* m_RotateImage_Action;
    QAction* m_Zoom_Action;
    QAction* m_Filter_Action;
    QAction* m_Help_Action;
    QPoint point;
    QPoint s_1[50];
    double num_1=0;
    QPoint s_2[50];
    double num_2=0;
};

#endif // QRSIMAGEWINDOW_H
