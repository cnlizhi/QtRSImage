//QBayesWidget.h

#ifndef QBAYESWIDGET_H
#define QBAYESWIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>

class QBayesWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QBayesWidget(QWidget* parent = nullptr);
    void setImage(QImage Image);
    ~QBayesWidget();
protected:
    void resizeEvent(QResizeEvent* event);
    void wheelEvent(QWheelEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void mouseDoubleClickEvent(QMouseEvent* event);
private slots:

private:
    QWidget* m_Image_Widget;
    QWidget* m_Print_Widget;
    QScrollArea* m_Image_ScrollArea;
    QScrollArea* m_Print_ScrollArea;
    QHBoxLayout* m_BaseLayout;
    QLabel* m_Image_Label;
    QLabel* m_Print_Label;
    QImage m_Image;
};

#endif // QBAYESWIDGET_H
