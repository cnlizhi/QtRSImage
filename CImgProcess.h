//CImgProcess.h

#ifndef _CIMGPROCESS_H
#define _CIMGPROCESS_H

#include "CRSImage.h"
#include <QImage>

typedef unsigned char DN;

class CImgProcess
{
public:
    CImgProcess();
    CImgProcess(const CImgProcess& rIProcess);
    ~CImgProcess();
    bool Set(CRSImage& RSImage);
    void ReSet();
    QImage Display();
    QImage Display(DN*** pppDN);
    void SetChannel(int Red, int Grn, int Blu);
    void SetAngle(double angle, bool bFlag);
    void SetZoom(double zoom, bool bFlag);
    QImage Filter(char core);
    QImage Bayes(QImage img);
    QImage Fisher(int i);
protected:
    int GetMaximum(DN** band) const;
    int GetMinimum(DN** band) const;
private:
    DN*** m_pppDN;
    int m_Red;
    int m_Grn;
    int m_Blu;
    int m_columns;
    int m_rows;
    int m_bands;
    double m_angle;
    double m_zoom;
};

#endif //_CIMGPROCESS_H
