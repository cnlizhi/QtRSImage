//QRSImageWindow.cpp

#include "QRSImageWindow.h"
#include "ui_QRSImageWindow.h"
#include "QBayesWidget.h"
#include <QFormLayout>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QErrorMessage>
#include <QtCharts/QChartView>
#include <QString>

QT_CHARTS_USE_NAMESPACE

QRSImageWindow::QRSImageWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::QRSImageWindow)
{
    ui->setupUi(this);
    setWindowTitle("Remote Sensing Image");
    this->resize(1200, 742);
    CreateGroupBoxTool();
    CreateWidgetImage();
    CreateActions();
}

QRSImageWindow::~QRSImageWindow()
{
    RSImage.~CRSImage();
    delete ui;
}

void QRSImageWindow::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
    ResizeWidget();
}

void QRSImageWindow::wheelEvent(QWheelEvent* event)
{
    if(m_OpenImage_Action->isEnabled())
    {
        return;
    }
    QPoint pos;
    QPoint pos1;
    QPoint pos2;
    pos1 = mapToGlobal(QPoint(0,0));
    pos2 = event->globalPos();
    pos = pos2 - pos1;
    if (pos.x() > m_Image_Label->x() && pos.x() < m_Image_Label->x()+m_Image_Label->width()
                && pos.y() > m_Image_Label->y() && pos.y() < m_Image_Label->y()+m_Image_Label->height())
    {
        if(event->delta() > 0)
        {
            ImgProcess.SetZoom(1.05, true);
            m_Image = ImgProcess.Display();
            m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
            m_Image_Label->show();
        }
        else
        {
            ImgProcess.SetZoom(0.95, true);
            m_Image = ImgProcess.Display();
            m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
            m_Image_Label->show();
        }
    }
    ResizeWidget();
}

void QRSImageWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        Exit();
    }
}

void QRSImageWindow::mousePressEvent(QMouseEvent* event)
{
    int i = 0;
    if(m_Image_Label->geometry().contains (this->mapFromGlobal (QCursor::pos())))
    {
        if(event->button() == Qt::LeftButton)
        {
            num_1++;
            point =m_Image_Label->mapFromGlobal( event->globalPos()) ;
            s_1[i]=point;
            i++;
        }
        if(event->button() == Qt::RightButton)
        {
            num_2++;
            point =m_Image_Label->mapFromGlobal( event->globalPos()) ;
            s_2[i]=point;
            i++;
        }
    }
}

void QRSImageWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if(m_InitImage_Action->isEnabled())
    {
        InitImage();
    }
}

void QRSImageWindow::OpenImage()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Please Choose Image."), "D:", tr("图片文件(*hdr *img)"));
    if (!RSImage.OpenImage(path))
    {
        QErrorMessage* Dialog = new QErrorMessage(this);
        Dialog->setWindowTitle(tr("Error！"));
        Dialog->showMessage(tr("Open Image Failed."));
    }
    else
    {
        if (!ImgProcess.Set(RSImage))
        {
            QErrorMessage* Dialog = new QErrorMessage(this);
            Dialog->setWindowTitle(tr("Error！"));
            Dialog->showMessage(tr("Set Image Process Failed."));
        }
        m_Image = ImgProcess.Display();
        m_Image_Widget->resize(this->width() - 400, this->width() - 60);
        m_Image_Widget->move(400, 60);
        m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
        m_Image_Label->resize(m_Image_Widget->size());
        CreateComboBox();
        ResizeWidget();
        ActionControl(true);
    }
}

void QRSImageWindow::OpenPhoto()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Please Choose Photo."), "D:", tr("图片文件(*jpg)"));
    if (!RSImage.OpenPhoto(path))
    {
        QErrorMessage* Dialog = new QErrorMessage(this);
        Dialog->setWindowTitle(tr("Error！"));
        Dialog->showMessage(tr("Open Photo Failed."));
    }
    else
    {
        if (!ImgProcess.Set(RSImage))
        {
            QErrorMessage* Dialog = new QErrorMessage(this);
            Dialog->setWindowTitle(tr("Error！"));
            Dialog->showMessage(tr("Set Photo Process Failed."));
        }
        m_Image = ImgProcess.Display();
        m_Image_Widget->resize(this->width() - 400, this->width() - 60);
        m_Image_Widget->move(400, 60);
        m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
        m_Image_Label->resize(m_Image_Widget->size());
        CreateComboBox();
        ResizeWidget();
        ActionControl(true);
    }
}

void QRSImageWindow::CloseImage()
{
    RSImage.CloseImage();
    ImgProcess.ReSet();
    m_Image = QImage();
    m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
    ActionControl(false);
}

void QRSImageWindow::SaveAsFile()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Please Choose Image."), "D:", tr("图片文件(*hdr *img)"));
    if (!RSImage.SaveAsFile(path))
    {
        QErrorMessage* Dialog = new QErrorMessage(this);
        Dialog->setWindowTitle(tr("Error！"));
        Dialog->showMessage(tr("Save As File Failed."));
    }
}

void QRSImageWindow::Exit()
{
    this->close();
}

void QRSImageWindow::Information()
{
    RSImage.Information();
}

void QRSImageWindow::Statistics()
{
    RSImage.Statistics();
}

void QRSImageWindow::Covariance()
{
    RSImage.Covariance();
}

void QRSImageWindow::Chart()
{
    m_ChartBand_ComboBox = new QComboBox();
    for (int i = 0; i < RSImage.GetBands(); i++)
    {
        m_ChartBand_ComboBox->addItem("Band" + QString::number(i + 1));
    }
    connect(m_ChartBand_ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ChartChanged()));
    m_ChartType_ComboBox = new QComboBox();
    m_ChartType_ComboBox->addItem("Histogram");
    m_ChartType_ComboBox->addItem("Line");
    m_ChartType_ComboBox->addItem("Spline");
    connect(m_ChartType_ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ChartChanged()));
    m_Chart_ChartView = new QChartView(m_Image_Widget);
    m_Chart_ChartView->setRenderHint(QPainter::Antialiasing);
    m_Chart_ChartView->setWindowTitle("Chart");
    m_Chart_ChartView->resize(400, 300);
    QWidget* Chart_Widget = new QWidget();
    Chart_Widget->setWindowTitle("Chart");
    Chart_Widget->resize(1200, 742);
    QGridLayout* BaseLayout = new QGridLayout(Chart_Widget);
    QHBoxLayout* BoxLayout = new QHBoxLayout();
    BoxLayout->addWidget(new QLabel("Band:"));
    BoxLayout->addWidget(m_ChartBand_ComboBox);
    BoxLayout->addWidget(new QLabel("Type:"));
    BoxLayout->addWidget(m_ChartType_ComboBox);
    BoxLayout->addStretch();
    BaseLayout->addLayout(BoxLayout, 0, 0, 1, 3);
    BaseLayout->addWidget(m_Chart_ChartView);
    ChannalChanged();
    m_ChartBand_ComboBox->show();
    m_ChartType_ComboBox->show();
    m_Chart_ChartView->show();
    Chart_Widget->show();
}

void QRSImageWindow::InitImage()
{
    ImgProcess.SetAngle(0, false);
    ImgProcess.SetZoom(1, false);
    m_Image = ImgProcess.Display();
    m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
    m_Image_Label->show();
}

void QRSImageWindow::RotateImage()
{
    bool ok;
    int value = QInputDialog::getInt
            (this, tr("RotateImage"), tr("Please enter rotate angle"), 0, 0, 360, 0, &ok);
    ImgProcess.SetAngle(value, true);
    m_Image = ImgProcess.Display();
    m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
    m_Image_Label->show();
}

void QRSImageWindow::Zoom()
{
    bool ok;
    double value = QInputDialog::getDouble
            (this, tr("Zoom"), tr("Please enter zoom"), 1, 0.33, 3, 2, &ok);
    ImgProcess.SetZoom(value, true);
    m_Image = ImgProcess.Display();
    m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
    m_Image_Label->show();
}

void QRSImageWindow::Filter()
{
    bool ok;
    QStringList items;
    items << "Mean Filter";
    items << "Gauss Filter";
    items << "Sharpness Filter";
    items << "Edge Detection";
    items << "Embossing Filter";
    QString qstr = QInputDialog::getItem
            (this, "Filter", "Please choose Filter", items, 0, true, &ok);
    char core;
    if(qstr == "Edge Detection")
    {
        core = 'D';
    }
    else
    {
        core = qstr.toStdString().c_str()[0];
    }
    m_Image = ImgProcess.Filter(core);
    m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
    m_Image_Label->show();
}

void QRSImageWindow::Help()
{
    QMessageBox::about(this, tr("About Remote Sensing Image"),
            tr("<p><b>Remote Sensing Image Process Program</b></p>"
               "<p>Created By Class.113172 LiZhi</p>"
               "<p>Date 2018.6.9</p>"
               "<p>遥感图像处理控制台程序，其功能菜单如下：</p>"
               "<p>00   Exit         退出程序</p>"
               "<p>01   Open Image   输入图像文件路径，即可读入文件数据</p>"
               "<p>02   Information  输出当前图像的路径，行列值、波段数、数据类型、排列方式等</p>"
               "<p>03   Close Image  关闭当前图像</p>"
               "<p>04   Statistics   输出图像数据统计量</p>"
               "<p>05   Covariance   输出图像协方差</p>"
               "<p>06   Chart        输出图像统计图表</p>"
               "<p>07   Save as File 保存图像为二进制文件</p>"
               "<p>08   Init Image   图像初始化</p>"
               "<p>09   Rotate Image 图像旋转，输入旋转角度，逆时针旋转图像</p>"
               "<p>10   Zoom         图像缩放，输入缩放比例尺，输出缩放图像</p>"
               "<p>11   Filter       输入滤波核，执行滤波，输出滤波后图像</p>"));
}

void QRSImageWindow::ChannalChanged()
{
    ImgProcess.SetChannel(m_RedChannel_ComboBox->currentIndex(), m_GreenChannel_ComboBox->currentIndex(), m_BlueChannel_ComboBox->currentIndex());
    m_Image = ImgProcess.Display();
    m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
    m_Image_Label->show();
}

void QRSImageWindow::Bayes()
{
    //QBayesWidget* BayesWidget = new QBayesWidget;
    //BayesWidget->setImage(m_Image);
    m_Image = ImgProcess.Bayes(m_Image);
    m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
    m_Image_Label->show();
}

void QRSImageWindow::Fisher()
{
    m_Image = ImgProcess.Fisher(0);
    m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
    m_Image_Label->show();
}

void QRSImageWindow::ChartChanged()
{
    switch (m_ChartType_ComboBox->currentIndex())
    {
    case 0:
        m_Chart_ChartView->setChart(RSImage.Chart(m_ChartBand_ComboBox->currentIndex(), 'H'));
        break;
    case 1:
        m_Chart_ChartView->setChart(RSImage.Chart(m_ChartBand_ComboBox->currentIndex(), 'L'));
        break;
    case 2:
        m_Chart_ChartView->setChart(RSImage.Chart(m_ChartBand_ComboBox->currentIndex(), 'S'));
        break;
    default:
        break;
    }
    m_Chart_ChartView->show();
}

void QRSImageWindow::CreateActions()
{
    QMenu* Menu_File = menuBar()->addMenu(tr("&File"));
    m_OpenImage_Action = Menu_File->addAction(tr("&OpenImage"), this, &QRSImageWindow::OpenImage, Qt::Key_O);
    m_OpenPhoto_Action = Menu_File->addAction(tr("&OpenPhoto"), this, &QRSImageWindow::OpenPhoto, Qt::Key_P);
    m_CloseImage_Action = Menu_File->addAction(tr("&CloseImage"), this, &QRSImageWindow::CloseImage, Qt::Key_C);
    m_SaveAsFile_Action = Menu_File->addAction(tr("&SaveAsFile"), this, &QRSImageWindow::SaveAsFile, Qt::Key_A);
    m_Exit_Action = Menu_File->addAction(tr("&Exit"), this, &QRSImageWindow::Exit, Qt::Key_X);
    QMenu* Menu_Data = menuBar()->addMenu(tr("&Data"));
    m_Information_Action = Menu_Data->addAction(tr("&Information"), this, &QRSImageWindow::Information, Qt::Key_I);
    m_Statistics_Action = Menu_Data->addAction(tr("&Statistics"), this, &QRSImageWindow::Statistics, Qt::Key_S);
    m_Covariance_Action = Menu_Data->addAction(tr("&Covariance"), this, &QRSImageWindow::Covariance, Qt::Key_V);
    m_Chart_Action = Menu_Data->addAction(tr("&Chart"), this, &QRSImageWindow::Chart, Qt::Key_H);
    QMenu* Menu_Display = menuBar()->addMenu(tr("&Display"));
    m_InitImage_Action = Menu_Display->addAction(tr("&InitImage"), this, &QRSImageWindow::InitImage, Qt::Key_T);
    m_RotateImage_Action = Menu_Display->addAction(tr("&RotateImage"), this, &QRSImageWindow::RotateImage, Qt::Key_R);
    m_Zoom_Action = Menu_Display->addAction(tr("&Zoom"), this, &QRSImageWindow::Zoom, Qt::Key_Z);
    m_Filter_Action = Menu_Display->addAction(tr("&Filter"), this, &QRSImageWindow::Filter, Qt::Key_F);
    QMenu* Menu_Help = menuBar()->addMenu(tr("&Help"));
    m_Help_Action = Menu_Help->addAction(tr("&Help"), this, &QRSImageWindow::Help, tr("?"));
    ActionControl(false);
}

void QRSImageWindow::CreateGroupBoxTool()
{
    m_RedChannel_ComboBox = new QComboBox;
    m_GreenChannel_ComboBox = new QComboBox;
    m_BlueChannel_ComboBox = new QComboBox;
    m_Bayes_Btn = new QPushButton;
    m_Fisher_Btn = new QPushButton;
    m_Tool_GroupBox = new QGroupBox(tr("Tool"), this);
    QFormLayout* Layout = new QFormLayout;
    Layout->addRow(new QLabel(tr("Channel Red")), m_RedChannel_ComboBox);
    Layout->addRow(new QLabel(tr("Channel Green")), m_GreenChannel_ComboBox);
    Layout->addRow(new QLabel(tr("Channel Blue")), m_BlueChannel_ComboBox);
    Layout->addRow(new QLabel(tr("Bayes")), m_Bayes_Btn);
    Layout->addRow(new QLabel(tr("Fisher")), m_Fisher_Btn);
    connect(m_Bayes_Btn, SIGNAL(clicked()), this, SLOT(Bayes()));
    connect(m_Fisher_Btn, SIGNAL(clicked()), this, SLOT(Fisher()));
    m_Tool_GroupBox->setLayout(Layout);
    m_Tool_GroupBox->resize(400, this->height() - 60);
    m_Tool_GroupBox->move(0, 60);
    m_Tool_GroupBox->show();
}

void QRSImageWindow::CreateWidgetImage()
{
    m_Image_Widget = new QWidget(this);
    m_Image_Label = new QLabel(m_Image_Widget);
    m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
    m_Image.scaled(m_Image_Label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_Image_ScrollArea = new QScrollArea(m_Image_Widget);
    m_Image_ScrollArea->setBackgroundRole(QPalette::Dark);
    m_Image_ScrollArea->setWidget(m_Image_Label);
    m_Image_ScrollArea->setAlignment(Qt::AlignCenter);
    m_Image_Widget->resize(this->width() - 400, this->height() - 60);
    m_Image_Widget->move(400, 60);
    m_Image_Widget->show();
}

void QRSImageWindow::CreateComboBox()
{
    m_RedChannel_ComboBox->clear();
    m_GreenChannel_ComboBox->clear();
    m_BlueChannel_ComboBox->clear();
    for (int i = 0; i < RSImage.GetBands(); i++)
    {
        m_RedChannel_ComboBox->addItem(tr("Band ") + QString::number(i + 1));
        m_GreenChannel_ComboBox->addItem(tr("Band ") + QString::number(i + 1));
        m_BlueChannel_ComboBox->addItem(tr("Band ") + QString::number(i + 1));
    }
    if(RSImage.GetBands() > 2)
    {
        m_RedChannel_ComboBox->setCurrentIndex(2);
        m_GreenChannel_ComboBox->setCurrentIndex(1);
        m_BlueChannel_ComboBox->setCurrentIndex(0);
    }
    connect(m_RedChannel_ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannalChanged()));
    connect(m_GreenChannel_ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannalChanged()));
    connect(m_BlueChannel_ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannalChanged()));
}

void QRSImageWindow::ResizeWidget()
{
    m_Image_Label->setAlignment(Qt::AlignCenter);
    m_Tool_GroupBox->resize(400, this->height() - 60);
    m_Tool_GroupBox->move(0, 60);
    m_Image_ScrollArea->resize(this->width() - 400, this->height() - 60);
    if(m_Image.width() < this->width() || m_Image.height() < this->height())
    {
        m_Image_Widget->resize(this->width() - 400, this->height() - 60);
        m_Image_Widget->move(400, 60);
        m_Image_Label->resize(m_Image_Widget->size());
    }
    else
    {
        m_Image_Widget->resize(m_Image.size());
        m_Image_Widget->move(400, 60);
        m_Image_Label->resize(m_Image_Widget->size());
    }
    m_Tool_GroupBox->show();
    m_Image_Label->show();
    m_Image_Widget->show();
}

void QRSImageWindow::ActionControl(bool bFlag)
{
    m_OpenImage_Action->setEnabled(!bFlag);
    m_OpenPhoto_Action->setEnabled(!bFlag);
    m_CloseImage_Action->setEnabled(bFlag);
    m_SaveAsFile_Action->setEnabled(bFlag);
    m_Information_Action->setEnabled(bFlag);
    m_Statistics_Action->setEnabled(bFlag);
    m_Covariance_Action->setEnabled(bFlag);
    m_Chart_Action->setEnabled(bFlag);
    m_InitImage_Action->setEnabled(bFlag);
    m_RotateImage_Action->setEnabled(bFlag);
    m_Zoom_Action->setEnabled(bFlag);
    m_Filter_Action->setEnabled(bFlag);
    m_RedChannel_ComboBox->setEnabled(bFlag);
    m_GreenChannel_ComboBox->setEnabled(bFlag);
    m_BlueChannel_ComboBox->setEnabled(bFlag);
    m_Bayes_Btn->setEnabled(bFlag);
    m_Fisher_Btn->setEnabled(bFlag);
}
