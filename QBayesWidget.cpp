//QBayesWidget.cpp

#include "QBayesWidget.h"
#include "QHBoxLayout"
#include <QKeyEvent>
#include <QImage>

QBayesWidget::QBayesWidget(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("Bayes");
    this->resize(1200, 742);
    m_Image_Widget = new QWidget(this);
    m_Image_Label = new QLabel(m_Image_Widget);
    m_Image_ScrollArea = new QScrollArea(m_Image_Widget);
    m_Image_ScrollArea->setBackgroundRole(QPalette::Dark);
    m_Image_ScrollArea->setWidget(m_Image_Label);
    m_Image_ScrollArea->setAlignment(Qt::AlignCenter);
    m_Image_Label->resize(this->width() / 2, this->height());
    m_Print_Widget = new QWidget(this);
    m_Print_Label = new QLabel(m_Print_Widget);
    m_Print_ScrollArea = new QScrollArea(m_Print_Widget);
    m_Print_ScrollArea->setBackgroundRole(QPalette::Light);
    m_Print_ScrollArea->setWidget(m_Print_Label);
    m_Print_ScrollArea->setAlignment(Qt::AlignCenter);
    m_Print_Label->resize(this->width() / 2, this->height());
    m_BaseLayout = new QHBoxLayout(this);
    m_BaseLayout->addWidget(m_Image_Widget);
    m_BaseLayout->addWidget(m_Print_Widget);
    m_BaseLayout->addStretch();
    m_Image_Widget->show();
    m_Print_Widget->show();
}

QBayesWidget::~QBayesWidget()
{
}

void QBayesWidget::setImage(QImage Image)
{
    m_Image = Image;
    m_Image_Label->setPixmap(QPixmap::fromImage(m_Image));
    m_Image_Widget->show();
    m_Print_Widget->show();
}

void QBayesWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
}

void QBayesWidget::wheelEvent(QWheelEvent* event)
{
    Q_UNUSED(event)
}

void QBayesWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape)
    {
        this->close();
    }
}

void QBayesWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}
