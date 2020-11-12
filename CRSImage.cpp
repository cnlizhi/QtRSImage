//CRSImage.cpp

#include "CRSImage.h"
#include <QLabel>
#include <QImage>
#include <QFileDialog>
#include <QTableWidget>
#include <QSplineSeries>
#include <fstream>
#include <sstream>
#include <QDebug>
using namespace std;

CRSImage::CRSImage()
    : m_pppDN(NULL), m_samples(0), m_lines(0), m_bands(0),
    m_interleave(BSQ), m_datatype(0)
{
}

CRSImage::CRSImage(const CRSImage& rImage)
    : m_pppDN(rImage.m_pppDN),
    m_samples(rImage.m_samples), m_lines(rImage.m_lines), m_bands(rImage.m_bands),
    m_interleave(rImage.m_interleave), m_datatype(rImage.m_datatype)
{
}

CRSImage::~CRSImage()
{
    if (m_pppDN != NULL)
    {
        delete[] m_pppDN;
    }
}

bool CRSImage::OpenImage(QString path)
{
    if (path == NULL)
    {
        return false;
    }
    string strpath = path.toStdString();
    int	pos = strpath.rfind('.');
    if (pos >= 0)
    {
        strpath = strpath.substr(0, pos);
    }
    strpath.append(".hdr");
    if (!ReadMetaData(strpath.c_str()))
    {
        return false;
    }
    if (!InitBuffer())
    {
        return false;
    }
    if (!ReadImgData(path))
    {
        return false;
    }
    return true;
}

bool CRSImage::OpenPhoto(QString path)
{
    if (path.isEmpty())
    {
        return false;
    }
    QImage* pImg = new QImage;
    if (!(pImg->load(path)))
    {
        delete pImg;
        return false;
    }
    QImage Img = *pImg;
    m_samples = Img.width();
    m_lines = Img.height();
    m_bands = 3;
    if (!InitBuffer())
    {
        return false;
    }
    for (int i = 0; i < m_lines; i++)
    {
        for (int j = 0; j < m_samples; j++)
        {
            QColor color = Img.pixelColor(j ,i);
            m_pppDN[0][i][j] = static_cast<DN>(color.red());
            m_pppDN[1][i][j] = static_cast<DN>(color.green());
            m_pppDN[2][i][j] = static_cast<DN>(color.blue());
        }
    }
    return true;
}

void CRSImage::Information()
{
    QTableWidget* Table = new QTableWidget(5, 2);
    Table->setWindowTitle("Information");
    QStringList ColLabs;
    ColLabs << "Type" << "Data";
    Table->setHorizontalHeaderLabels(ColLabs);
    Table->setItem(0, 0, new QTableWidgetItem("samples"));
    Table->setItem(1, 0, new QTableWidgetItem("lines"));
    Table->setItem(2, 0, new QTableWidgetItem("bands"));
    Table->setItem(3, 0, new QTableWidgetItem("interleave"));
    Table->setItem(4, 0, new QTableWidgetItem("datatype"));
    Table->setItem(0, 1, new QTableWidgetItem(QString::number(m_samples)));
    Table->setItem(1, 1, new QTableWidgetItem(QString::number(m_lines)));
    Table->setItem(2, 1, new QTableWidgetItem(QString::number(m_bands)));
    if (m_interleave == BSQ)
    {
        Table->setItem(3, 1, new QTableWidgetItem("BSQ"));
    }
    else if (m_interleave == BIP)
    {
        Table->setItem(3, 1, new QTableWidgetItem("BIP"));
    }
    else if (m_interleave == BIL)
    {
        Table->setItem(3, 1, new QTableWidgetItem("BIL"));
    }
    Table->setItem(4, 1, new QTableWidgetItem(QString::number(m_datatype)));
    Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Table->resize(380, 300);
    Table->show();
}

void CRSImage::CloseImage()
{
    delete[] m_pppDN;
}

void CRSImage::Statistics()
{
    QTableWidget* Table = new QTableWidget(m_bands, 5);
    Table->setWindowTitle("Statistics");
    QStringList ColLabs;
    ColLabs << "Bands" << "Average" << "Variance" << "Maximum" << "Minimum";
    Table->setHorizontalHeaderLabels(ColLabs);
    for (int band = 0; band < m_bands; band++)
    {
        Table->setItem(band, 0, new QTableWidgetItem(QString::number(band + 1)));
        Table->setItem(band, 1, new QTableWidgetItem(QString::number(GetAverage(m_pppDN[band]))));
        Table->setItem(band, 2, new QTableWidgetItem(QString::number(GetVariance(m_pppDN[band]))));
        Table->setItem(band, 3, new QTableWidgetItem(QString::number(GetMaximum(m_pppDN[band]))));
        Table->setItem(band, 4, new QTableWidgetItem(QString::number(GetMinimum(m_pppDN[band]))));
    }
    Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Table->show();
}

void CRSImage::Covariance()
{
    QTableWidget* Table = new QTableWidget(m_bands, m_bands);
    Table->setWindowTitle("Covariance");
    QStringList Collabs_x;
    QStringList Collabs_y;
    for (int i = 0; i < m_bands; i++)
    {
        Collabs_x << "Band " + QString::number(i + 1);
        Collabs_y << "Band " + QString::number(i + 1);
    }
    Table->setHorizontalHeaderLabels(Collabs_x);
    Table->setVerticalHeaderLabels(Collabs_y);
    for (int i = 0; i < m_bands; i++)
    {
        for (int j = 0; j < m_bands; j++)
        {
            if (i == j)
            {
                Table->setItem(i, i, new QTableWidgetItem(QString::number(GetVariance(m_pppDN[i]))));
            }
            else
            {
                Table->setItem(i, j, new QTableWidgetItem(QString::number(GetCovariance(m_pppDN[i], m_pppDN[j]))));
            }
        }
    }
    Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    Table->show();
}

QChart* CRSImage::Chart(int band, char type)
{
    int index = 0;
    int Array[256];
    int x_max = GetMaximum(m_pppDN[band]);
    int	y_max = 0;
    for (int x = 0; x < 256; x++)
    {
        Array[x] = 0;
    }
    for (int i = 0; i < m_lines; i++)
    {
        for (int j = 0; j < m_samples; j++)
        {
            index = m_pppDN[band][i][j];
            Array[index]++;
        }
    }
    for (int x = 0; x < x_max; x++)
    {
        y_max = (Array[x] > y_max) ? Array[x] : y_max;
    }
    if(type == 'H')
    {
        QBarSet* set = new QBarSet(" ");
        for (int i = 0; i < x_max; i++)
        {
            *set << Array[i];
        }
        QStackedBarSeries *series = new QStackedBarSeries();
        series->append(set);
        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Remote Sensing Histogram");
        chart->setAnimationOptions(QChart::SeriesAnimations);
        QStringList categories;
        for(int i = 0; i < x_max; i++)
        {
            categories << QString::number(i);
        }
        QBarCategoryAxis *axis = new QBarCategoryAxis();
        axis->append(categories);
        chart->createDefaultAxes();
        chart->setAxisX(axis, series);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        return chart;
    }
    else if (type == 'L')
    {
        QLineSeries *series = new QLineSeries();
        for (int i = 0; i < x_max; i++)
        {
            series->append(i, Array[i]);
        }
        QChart* chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Remote Sensing Line Chart");
        chart->setAnimationOptions(QChart::SeriesAnimations);
        QStringList categories;
        for(int i = 0; i < x_max; i++)
        {
            categories << QString::number(i);
        }
        QBarCategoryAxis *axis = new QBarCategoryAxis();
        axis->append(categories);
        chart->createDefaultAxes();
        chart->setAxisX(axis, series);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        return chart;
    }
    else if (type == 'S')
    {
        QSplineSeries *series = new QSplineSeries();
        for (int i = 0; i < x_max; i++)
        {
            series->append(i, Array[i]);
        }
        QChart* chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Remote Sensing Spline Chart");
        chart->setAnimationOptions(QChart::SeriesAnimations);
        QStringList categories;
        for(int i = 0; i < x_max; i++)
        {
            categories << QString::number(i);
        }
        QBarCategoryAxis *axis = new QBarCategoryAxis();
        axis->append(categories);
        chart->createDefaultAxes();
        chart->setAxisX(axis, series);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);
        return chart;
    }
    return NULL;
}

bool CRSImage::SaveAsFile(QString path)
{
    if (path == NULL)
    {
        return false;
    }
    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        int i, j;
        if (m_interleave == BSQ)
        {
            for (i = 0; i < m_bands && !file.atEnd(); i++)
            {
                for (j = 0; j < m_lines && !file.atEnd(); j++)
                {
                    file.write((char*)m_pppDN[i][j], sizeof(DN)*m_samples);
                }
            }
            if (i < m_bands || j < m_lines)
            {
                file.close();
                return false;
            }
        }
        else if (m_interleave == BIL)
        {
            for (i = 0; i < m_lines && !file.atEnd(); i++)
            {
                for (j = 0; j < m_bands && !file.atEnd(); j++)
                {
                    file.write((char*)m_pppDN[j][i], sizeof(DN)*m_samples);
                }
            }
            if (i < m_lines || j < m_bands)
            {
                file.close();
                return false;
            }
        }
        else if (m_interleave == BIP)
        {
            int* point = new int[m_bands];
            if (point == NULL)
            {
                file.close();
                return false;
            }
            for (i = 0; i < m_samples*m_lines && !file.atEnd(); i++)
            {
                file.write((char*)point, sizeof(DN)*m_bands);
                for (j = 0; j < m_bands; j++)
                {
                    m_pppDN[j][i / m_lines][i%m_lines] = point[j];
                }
            }
            if (i < m_samples*m_lines)
            {
                file.close();
                return false;
            }
        }
        file.close();
    }
    else
    {
        return false;
    }
    return true;
}

DN*** CRSImage::GetDN() const
{
    return m_pppDN;
}

int CRSImage::GetBands() const
{
    return m_bands;
}

int CRSImage::GetLines() const
{
    return m_lines;
}

int CRSImage::GetSamples() const
{
    return m_samples;
}

bool CRSImage::ReadMetaData(const char* cpath)
{
    ifstream ifile;
    char chgetline[_MAX_PATH];
    string strgetline;
    string strtext;
    stringstream ss;
    ifile.open(cpath, ios_base::in);
    if (!ifile.is_open())
    {
        return false;
    }
    while (!ifile.eof())
    {
        ifile.getline(chgetline, _MAX_PATH);
        strgetline = chgetline;
        ss.clear();
        ss.str(strgetline);
        ss >> strtext;
        if (strtext == "samples")
        {
            ss >> strtext >> m_samples;
        }
        else if (strtext == "lines")
        {
            ss >> strtext >> m_lines;
        }
        else if (strtext == "bands")
        {
            ss >> strtext >> m_bands;
        }
        else if (strtext == "interleave")
        {
            ss >> strtext >> strtext;
            if (strtext == "bsq")
            {
                m_interleave = BSQ;
            }
            else if (strtext == "bip")
            {
                m_interleave = BIP;
            }
            else if (strtext == "bil")
            {
                m_interleave = BIL;
            }
        }
        else if (strtext == "data")
        {
            ss >> strtext;
            if (strtext == "type")
            {
                ss >> strtext >> m_datatype;
            }
        }
    }
    ifile.close();
    return true;
}

bool CRSImage::InitBuffer()
{
    int i, j;
    m_pppDN = new DN**[m_bands];
    if (m_pppDN == NULL)
    {
        return false;
    }
    for (i = 0; i < m_bands; i++)
    {
        m_pppDN[i] = 0UL;
    }
    for (i = 0; i < m_bands; ++i)
    {
        m_pppDN[i] = new DN*[m_lines];
        if (m_pppDN[i] == NULL)
        {
            return false;
        }
        for (j = 0; j < m_lines; ++j)
        {
            m_pppDN[i][j] = 0UL;
        }
        for (j = 0; j < m_lines; ++j)
        {
            m_pppDN[i][j] = new DN[m_samples];
        }
    }
    return true;
}

bool CRSImage::ReadImgData(QString path)
{
    QFile file(path);
    if(file.open(QIODevice::ReadOnly))
    {
        int i, j;
        if (m_interleave == BSQ)
        {
            for (i = 0; i < m_bands && !file.atEnd(); i++)
            {
                for (j = 0; j < m_lines && !file.atEnd(); j++)
                {
                    file.read((char*)m_pppDN[i][j], sizeof(DN)*m_samples);
                }
            }
            if (i < m_bands || j < m_lines)
            {
                file.close();
                return false;
            }
        }
        else if (m_interleave == BIL)
        {
            for (i = 0; i < m_lines && !file.atEnd(); i++)
            {
                for (j = 0; j < m_bands && !file.atEnd(); j++)
                {
                    file.read((char*)m_pppDN[j][i], sizeof(DN)*m_samples);
                }
            }
            if (i < m_lines || j < m_bands)
            {
                file.close();
                return false;
            }
        }
        else if (m_interleave == BIP)
        {
            int* point = new int[m_bands];
            if (point == NULL)
            {
                file.close();
                return false;
            }
            for (i = 0; i < m_samples*m_lines && !file.atEnd(); i++)
            {
                file.read((char*)point, sizeof(DN)*m_bands);
                for (j = 0; j < m_bands; j++)
                {
                    m_pppDN[j][i / m_lines][i%m_lines] = point[j];
                }
            }
            if (i < m_samples*m_lines)
            {
                file.close();
                return false;
            }
        }
        file.close();
    }
    else
    {
        return false;
    }
    return true;
}

double CRSImage::GetAverage(DN** band) const
{
    double sum = 0;
    for (int i = 0; i < m_lines; i++)
    {
        for (int j = 0; j < m_samples; j++)
        {
            sum += band[i][j];
        }
    }
    return sum / static_cast<double>(m_lines*m_samples);
}

double CRSImage::GetVariance(DN** band) const
{
    double sum = 0;
    double Average = GetAverage(band);
    for (int i = 0; i < m_lines; i++)
    {
        for (int j = 0; j < m_samples; j++)
        {
            sum += pow((band[i][j] - Average), 2);
        }
    }
    return sum / static_cast<double>(m_lines*m_samples);
}

double CRSImage::GetCovariance(DN** band_x, DN** band_y) const
{
    double sum = 0;
    double Average_x = GetAverage(band_x);
    double Average_y = GetAverage(band_y);
    for (int i = 0; i < m_lines; i++)
    {
        for (int j = 0; j < m_samples; j++)
        {
            sum += (band_x[i][j] - Average_x)*(band_y[i][j] - Average_y);
        }
    }
    return sum / static_cast<double>(m_lines*m_samples);
}

int CRSImage::GetMaximum(DN** band) const
{
    int result = 0;
    for (int i = 0; i < m_lines; i++)
    {
        for (int j = 0; j < m_samples; j++)
        {
            result = (band[i][j] > result) ? band[i][j] : result;
        }
    }
    return result;
}

int CRSImage::GetMinimum(DN** band) const
{
    int result = 256;
    for (int i = 0; i < m_lines; i++)
    {
        for (int j = 0; j < m_samples; j++)
        {
            result = (band[i][j] < result) ? band[i][j] : result;
        }
    }
    return result;
}
