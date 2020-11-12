//CImgProcess.cpp

#include "CImgProcess.h"
#include <QDebug>

#define _MAIN_FILTER 1,1,1,1,1,1,1,1,1
#define _GAUSS_FILTER 1,4,7,4,1,4,16,26,16,4,7,26,41,26,7,4,16,26,16,4,1,4,7,4,1
#define _SHARPNESS_FILTER -1,-1,-1,-1,9,-1,-1,-1,-1
#define _EDGE_DETECTION -1,-1,-1,-1,8,-1,-1,-1,-1
#define _EMBOSSING_FILTER -1,-1,0,-1,0,1,0,1,1

CImgProcess::CImgProcess()
    : m_pppDN(NULL), m_Red(0), m_Grn(0), m_Blu(0),
      m_columns(0), m_rows(0), m_bands(0), m_angle(0), m_zoom(1)
{
}

CImgProcess::CImgProcess(const CImgProcess& rIProcess)
    : m_pppDN(rIProcess.m_pppDN), m_Red(rIProcess.m_Red),
      m_Grn(rIProcess.m_Grn), m_Blu(rIProcess.m_Blu),
      m_columns(rIProcess.m_columns),
      m_rows(rIProcess.m_rows), m_bands(rIProcess.m_bands),
      m_angle(rIProcess.m_angle), m_zoom(rIProcess.m_zoom)
{
}

CImgProcess::~CImgProcess()
{
    if (m_pppDN != NULL)
    {
        delete[] m_pppDN;
    }
}

bool CImgProcess::Set(CRSImage& RSImage)
{
    m_bands = RSImage.GetBands();
    m_rows = RSImage.GetLines();
    m_columns = RSImage.GetSamples();
    m_pppDN = new DN**[m_bands];
    if (m_pppDN == NULL)
    {
        return false;
    }
    for (int i = 0; i < m_bands; ++i)
    {
        m_pppDN[i] = 0UL;
        m_pppDN[i] = new DN*[m_rows];
        if (m_pppDN[i] == NULL)
        {
            return false;
        }
        for (int j = 0; j < m_rows; ++j)
        {
            m_pppDN[i][j] = 0UL;
            m_pppDN[i][j] = new DN[m_columns];
        }
    }
    for (int i = 0; i < m_bands; i++)
    {
        for (int j = 0; j < m_rows; j++)
        {
            for (int k = 0; k < m_columns; k++)
            {
                m_pppDN[i][j][k] = RSImage.GetDN()[i][j][k];
            }
        }
    }
    if(m_bands > 2)
    {
        m_Blu = 0;
        m_Grn = 1;
        m_Red = 2;
    }
    else
    {
        m_Blu = 0;
        m_Grn = 0;
        m_Red = 0;
    }
    return true;
}

void CImgProcess::ReSet()
{
    m_pppDN = NULL;
    m_Red = 0;
    m_Grn = 0;
    m_Blu = 0;
    m_bands = 0;
    m_rows = 0;
    m_columns = 0;
    m_angle = 0;
    return;
}

QImage CImgProcess::Display()
{
    DN** Col[3] = { m_pppDN[m_Red],m_pppDN[m_Grn],m_pppDN[m_Blu] };
    int Maximum[3] = { GetMaximum(Col[0]),GetMaximum(Col[1]),GetMaximum(Col[2]) };
    int Minimum[3] = { GetMinimum(Col[0]),GetMinimum(Col[1]),GetMinimum(Col[2]) };
    DN* pBuffer = new DN[3*m_rows*m_columns];
    int i = 0;
    for (int k = 0; k< m_rows; k++)
    {
        for (int l = 0; l < m_columns; l++)
        {
            for (int j = 0; j < 3 ; j++)
            {
                pBuffer[i] = (Col[j][k][l] - Minimum[j]) * 255 / (Maximum[j] - Minimum[j]);
                i++;
            }
        }
    }
    QImage Image(pBuffer, m_columns, m_rows, QImage::Format_RGB888);
    QMatrix LeftMatrix;
    LeftMatrix.rotate(m_angle);
    Image = Image.transformed(LeftMatrix, Qt::SmoothTransformation);
    QSize picSize(Image.width()*m_zoom, Image.height()*m_zoom);
    Image = Image.scaled(picSize, Qt::KeepAspectRatio);
    return Image;
}

QImage CImgProcess::Display(DN*** pppDN)
{
    DN** Col[3] = { pppDN[m_Red],pppDN[m_Grn],pppDN[m_Blu] };
    int Maximum[3] = { GetMaximum(Col[0]),GetMaximum(Col[1]),GetMaximum(Col[2]) };
    int Minimum[3] = { GetMinimum(Col[0]),GetMinimum(Col[1]),GetMinimum(Col[2]) };
    DN* pBuffer = new DN[3*m_rows*m_columns];
    int i = 0;
    for (int k = 0; k< m_rows; k++)
    {
        for (int l = 0; l < m_columns; l++)
        {
            for (int j = 0; j < 3 ; j++)
            {
                pBuffer[i] = (Col[j][k][l] - Minimum[j]) * 255 / (Maximum[j] - Minimum[j]);
                i++;
            }
        }
    }
    QImage Image(pBuffer, m_columns, m_rows, QImage::Format_RGB888);
    QMatrix LeftMatrix;
    LeftMatrix.rotate(m_angle);
    Image = Image.transformed(LeftMatrix, Qt::SmoothTransformation);
    QSize picSize(Image.width()*m_zoom, Image.height()*m_zoom);
    Image = Image.scaled(picSize, Qt::KeepAspectRatio);
    return Image;
}

void CImgProcess::SetChannel(int Red, int Grn, int Blu)
{
    m_Red = Red;
    m_Grn = Grn;
    m_Blu = Blu;
    return;
}

void CImgProcess::SetAngle(double angle, bool bFlag)
{
    if(!bFlag)
    {
        m_angle = angle;
        return;
    }
    m_angle += angle;
    return;
}

void CImgProcess::SetZoom(double zoom, bool bFlag)
{
    if(!bFlag)
    {
        m_zoom = zoom;
        return;
    }
    m_zoom *= zoom;
    return;
}

QImage CImgProcess::Filter(char core)
{
    int size = 0;
    if (core == 'M' || core == 'S' || core == 'D' || core == 'E')
    {
        size = 3;
    }
    else if (core == 'G')
    {
        size = 5;
    }
    else
    {
        return Display();
    }
    if (size % 2 == 0)
    {
        return Display();
    }
    double* CoreTemp = NULL;
    switch (core)
    {
    case 'M':
    {
        double BuffCore[9] = { _MAIN_FILTER };
        CoreTemp = BuffCore;
        break;
    }
    case 'G':
    {
        double BuffCore[25] = { _GAUSS_FILTER };
        CoreTemp = BuffCore;
        break;
    }
    case 'S':
    {
        double BuffCore[9] = { _SHARPNESS_FILTER };
        CoreTemp = BuffCore;
        break;
    }
    case 'D':
    {
        double BuffCore[9] = { _EDGE_DETECTION };
        CoreTemp = BuffCore;
        break;
    }
    case 'E':
    {
        double BuffCore[9] = { _EMBOSSING_FILTER };
        CoreTemp = BuffCore;
        break;
    }
    default:
        return Display();
        break;
    }
    DN*** pppDNBuff = new DN**[m_bands];
    if (pppDNBuff == NULL)
    {
        return Display();
    }
    for (int i = 0; i < m_bands; ++i)
    {
        pppDNBuff[i] = 0UL;
        pppDNBuff[i] = new DN*[m_rows];
        if (pppDNBuff[i] == NULL)
        {
            return Display();
        }
        for (int j = 0; j < m_rows; ++j)
        {
            pppDNBuff[i][j] = 0UL;
            pppDNBuff[i][j] = new DN[m_columns];
        }
    }
    for (int i = 0; i < m_bands; i++)
    {
        for (int j = 0; j < m_rows; j++)
        {
            for (int k = 0; k < m_columns; k++)
            {
                pppDNBuff[i][j][k] = m_pppDN[i][j][k];
            }
        }
    }
    double total = 0;
    for (int i = 0; i < size*size; i++)
    {
        total += CoreTemp[i];
    }
    for (int i = 0; i < m_bands; i++)
    {
        for (int j = size / 2; j < m_rows - size / 2; j++)
        {
            for (int k = size / 2; k < m_columns - size / 2; k++)
            {
                double sum = 0;
                int index = 0;
                for (int l = j - size / 2; l < j + size / 2 + 1; l++)
                {
                    for (int m = k - size / 2; m < k + size / 2 + 1; m++)
                    {
                        sum += m_pppDN[i][l][m] * CoreTemp[index++];
                    }
                }
                if (total != 0)
                {
                    sum /= total;
                }
                if (sum > 255)
                {
                    sum = 255;
                }
                if (sum < 0)
                {
                    sum = 0;
                }
                pppDNBuff[i][j][k] = static_cast<DN>(sum);
            }

        }
    }
    return Display(pppDNBuff);
}

double** Inverse(double Array[3][3])//矩阵求逆
{
    int m = 3;
    int n = 3;
    /*m = Array.GetLength(0);
    n = Array.GetLength(1);*/
    double array[2 * m + 1][ 2 * n + 1] ;//= new double[2 * m + 1, 2 * n + 1];
    for (int k = 0; k < 2 * m + 1; k++)  //初始化数组
    {
        for (int t = 0; t < 2 * n + 1; t++)
        {
            array[k][ t] = 0.00000000;

        }
    }
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
        {
            array[i][j] = Array[i][ j];
        }
    }

    for (int k = 0; k < m; k++)
    {
        for (int t = n; t <= 2 * n; t++)
        {
            if ((t - k) == m)
            {
                array[k][ t] = 1.0;
            }
            else
            {
                array[k][ t] = 0;
            }
        }
    }
    //得到逆矩阵
    for (int k = 0; k < m; k++)
    {
        if (array[k][ k] != 1)
        {
            double bs = array[k][ k];
            array[k][ k] = 1;
            for (int p = k + 1; p < 2 * n; p++)
            {
                array[k][ p] /= bs;
            }
        }
        for (int q = 0; q < m; q++)
        {
            if (q != k)
            {
                double bs = array[q][ k];
                for (int p = 0; p < 2 * n; p++)
                {
                    array[q][ p] -= bs * array[k][ p];
                }
            }
            else
            {
                continue;
            }
        }
    }

    double** NI;// = new double;
    NI=new double*[3];
    for (int i = 0; i < 3; i++)
            {
                NI[i] = new double[3];
            }
    for (int x = 0; x < m; x++)
    {
        for (int y = n; y < 2 * n; y++)
        {
            NI[x][ y - n] = array[x][ y];
        }
    }

    return NI;
}

double MatrixSurplus(double a[3][3])//矩阵求行列式值
{
    int i, j, k, p, r, m, n;
    m = 3;
    n = 3;
    double X, temp = 1, temp1 = 1, s = 0, s1 = 0;

    if (n == 2)
    {
        for (i = 0; i < m; i++)
            for (j = 0; j < n; j++)
                if ((i + j) % 2 > 0) temp1 *= a[i][ j];
                else temp *= a[i][ j];
        X = temp - temp1;
    }
    else
    {
        for (k = 0; k < n; k++)
        {
            for (i = 0, j = k; i < m && j < n; i++, j++)
                temp *= a[i][ j];
            if (m - i > 0)
            {
                for (p = m - i, r = m - 1; p > 0; p--, r--)
                    temp *= a[r][ p - 1];
            }
            s += temp;
            temp = 1;
        }

        for (k = n - 1; k >= 0; k--)
        {
            for (i = 0, j = k; i < m && j >= 0; i++, j--)
                temp1 *= a[i][ j];
            if (m - i > 0)
            {
                for (p = m - 1, r = i; r < m; p--, r++)
                    temp1 *= a[r][ p];
            }
            s1 += temp1;
            temp1 = 1;
        }

        X = s - s1;
    }
    return X;
}

QImage CImgProcess::Bayes(QImage img)
{
    int p1r[10000]{0};
    int p1g[10000]{0};
    int p1b[10000]{0};
    int p2r[10000]{0};
    int p2g[10000]{0};
    int p2b[10000]{0};
    int k = 0;
    double p1_ra = 0;
    double p1_ga = 0;
    double p1_ba = 0;
    double p2_ra = 0;
    double p2_ga = 0;
    double p2_ba = 0;
    return this->Fisher(1);
    for (int i = 186; i < 286; i++)
    {
        for (int j = 528; j < 628; j++)
        {
            p1r[k] = m_pppDN[0][i][j];p1_ra += (double)m_pppDN[0][i][j];
            p1g[k] = m_pppDN[1][i][j]; p1_ga += (double)m_pppDN[1][i][j];
            p1b[k] = m_pppDN[2][i][j]; p1_ba += (double)m_pppDN[2][i][j];
            k++;
        }
    }
    k = 0;
    for (int i = 693; i < 793; i++)
    {
        for (int j = 707; j < 807; j++)
        {
            p2r[k] = m_pppDN[0][i][j]; p2_ra += (double)m_pppDN[0][i][j];
            p2g[k] = m_pppDN[1][i][j]; p2_ga += (double)m_pppDN[1][i][j];
            p2b[k] = m_pppDN[2][i][j]; p2_ba += (double)m_pppDN[2][i][j];
            k++;
        }
    }
    p1_ra = (double)p1_ra / (double)10000;
    p1_ga = (double)p1_ga / (double)10000;
    p1_ba = (double)p1_ba / (double)10000;
    p2_ra = (double)p2_ra / (double)10000;
    p2_ga = (double)p2_ga / (double)10000;
    p2_ba = (double)p2_ba / (double)10000;
    double mycovp1[3][3]{0};
    double mycovp2[3][3]{0};
    double** mycovp1_1;
    double** mycovp2_1;
    for (int i= 0; i < 10000;i++)
    {
        mycovp1[0][0] += ((double)p1r[i]-(double)p1_ra)* ((double)p1r[i] - (double)p1_ra);
        mycovp1[0][ 1] += ((double)p1r[i] - (double)p1_ra) * ((double)p1g[i] - (double)p1_ga);
        mycovp1[0][ 2] += ((double)p1r[i] - (double)p1_ra) * ((double)p1b[i] - (double)p1_ba);
        mycovp1[1][ 0] += ((double)p1g[i] - (double)p1_ga) * ((double)p1r[i] - (double)p1_ra);
        mycovp1[1][ 1] += ((double)p1g[i] - (double)p1_ga) * ((double)p1g[i] - (double)p1_ga);
        mycovp1[1][ 2] += ((double)p1g[i] - (double)p1_ga) * ((double)p1b[i] - (double)p1_ba);
        mycovp1[2][ 0] += ((double)p1b[i] - (double)p1_ba) * ((double)p1r[i] - (double)p1_ra);
        mycovp1[2][1] += ((double)p1b[i] - (double)p1_ba) * ((double)p1g[i] - (double)p1_ga);
        mycovp1[2][ 2] += ((double)p1b[i] - (double)p1_ba) * ((double)p1b[i] - (double)p1_ba);
        mycovp2[0][ 0] += ((double)p2r[i] - (double)p2_ra) * ((double)p2r[i] - (double)p2_ra);
        mycovp2[0][ 1] += ((double)p2r[i] - (double)p2_ra) * ((double)p2g[i] - (double)p2_ga);
        mycovp2[0][2] += ((double)p2r[i] - (double)p2_ra) * ((double)p2b[i] - (double)p2_ba);
        mycovp2[1][ 0] += ((double)p2g[i] - (double)p2_ga) * ((double)p2r[i] - (double)p2_ra);
        mycovp2[1][1] += ((double)p2g[i] - (double)p2_ga) * ((double)p2g[i] - (double)p2_ga);
        mycovp2[1][2] += ((double)p2g[i] - (double)p2_ga) * ((double)p2b[i] - (double)p2_ba);
        mycovp2[2][0] += ((double)p2b[i] - (double)p2_ba) * ((double)p2r[i] - (double)p2_ra);
        mycovp2[2][1] += ((double)p2b[i] - (double)p2_ba) * ((double)p2g[i] - (double)p2_ga);
        mycovp2[2][2] += ((double)p2b[i] - (double)p2_ba) * ((double)p2b[i] - (double)p2_ba);
    }
    mycovp1[0][ 0]/=(double)9999;
    mycovp1[0][1] /= (double)9999;
    mycovp1[0][2] /= (double)9999;
    mycovp1[1][0] /= (double)9999;
    mycovp1[1][1] /= (double)9999;
    mycovp1[1][2] /= (double)9999;
    mycovp1[2][0] /= (double)9999;
    mycovp1[2][1] /= (double)9999;
    mycovp1[2][2] /= (double)9999;
    mycovp2[0][0] /= (double)9999;
    mycovp2[0][1] /= (double)9999;
    mycovp2[0][2] /= (double)9999;
    mycovp2[1][0] /= (double)9999;
    mycovp2[1][1] /= (double)9999;
    mycovp2[1][2] /= (double)9999;
    mycovp2[2][0] /= (double)9999;
    mycovp2[2][1] /= (double)9999;
    mycovp2[2][2] /= (double)9999;
    mycovp1_1 = Inverse(mycovp1);
    mycovp2_1 = Inverse(mycovp2);
    QImage bayes_Img;
    bayes_Img = img;
    for (int i = 0; i < m_columns; i++)
    {
        for (int j = 0; j < m_rows; j++)
        {
            double* temp1 = new double[3];
            double temp2 = 0;
            double* temp3 = new double[3];
            double temp4 = 0;
            double point1[3];
            double point2[3];
            point1[0] = (double)m_pppDN[0][ i][j]-p1_ra;
            point1[1] = (double)m_pppDN[1][ i][j]-p1_ga;
            point1[2] = (double)m_pppDN[2][ i][ j]-p1_ba;
            point2[0]= (double)m_pppDN[0][ i][ j] - p2_ra;
            point2[1] = (double)m_pppDN[1][ i][ j] - p2_ga;
            point2[2] = (double)m_pppDN[2][ i][ j] - p2_ba;
            for (int a = 0; a < 2; a++)
            {
                for (int b = 0; b < 2; b++)
                {
                    temp1[a] += (double)point1[a] * (double)mycovp1_1[a][ b];
                    temp3[a] += (double)point2[a] * (double)mycovp2_1[a][b];
                }
            }
            temp2 = (double)temp1[0] * (double)point1[0]+ (double)temp1[1] * (double)point1[1] + (double)temp1[2] * (double)point1[2];
            temp4 = (double)temp3[0] * (double)point2[0] + (double)temp3[1] * (double)point2[1] + (double)temp3[2] * (double)point2[2];
            double p1 =0;
            double p2 = 0;
            p1 = (double)1 / (double)pow((double)2 * (double)M_PI, 0.5) / (double)pow(MatrixSurplus(mycovp1), 0.5) * (double)exp((double)-1/2*(double)temp2);
            p2 = (double)1 / (double)pow((double)2 * (double)M_PI, 0.5) / (double)pow(MatrixSurplus(mycovp2), 0.5) * (double)exp((double)-1/2*(double)temp4);
            if (p1 > p2) { bayes_Img.setPixel(i, j, qRgb(0, 255, 0)); }
            if (p1 <= p2) { bayes_Img.setPixel(i, j, qRgb(255, 0, 0)); }
        }
    }
    return bayes_Img;
}

QImage CImgProcess::Fisher(int abc)
{
    int pixel[256];
    int index = 0;
    int sum = m_columns*m_rows;
    for (int i = 0; i < 256; i++)
    {
        pixel[i] = 0;
    }
    for (int i = 0; i < m_rows; i++)
    {
        for (int j = 0; j < m_columns; j++)
        {
            pixel[m_pppDN[0][i][j]];
        }
    }
    int mid = 0;
    while (index < sum / 2)
    {
        index += pixel[mid];
        mid++;
    }
    double miu1 = 0;
    double miu2 = 0;
    double cov1 = 0;
    double cov2 = 0;
    double Jw1 = 0;
    double Jw2 = 0;
    for (int i = 0; i <= mid; i++)
    {
        if (i < mid)
        {
            miu1 += i*pixel[i];
        }
        else
        {
            miu1 += i*(pixel[i] - index + sum / 2);
        }
    }
    miu1 *= 2 / sum;
    for (int i = mid; i < 256; i++)
    {
        if (i == mid)
        {
            miu2 += i*(index - sum / 2);
        }
        else
        {
            miu2 += i*pixel[i];
        }
    }
    miu2 *= 2 / sum;
    for (int i = 0; i <= mid; i++)
    {
        if (i < mid)
        {
            cov1 += (i - miu1)*(i - miu1)*pixel[i];
        }
        else
        {
            cov1 += (i - miu1)*(i - miu1)*(pixel[i] - index + sum / 2);
        }
    }
    cov1 *= 2 / sum;
    for (int i = mid; i < 256; i++)
    {
        if (i == mid)
        {
            cov2 += (i - miu2)*(i - miu2)*(index - sum / 2);
        }
        else
        {
            cov2 += (i - miu2)*(i - miu2)*pixel[i];
        }
    }
    cov2 *= 2 / sum;
    Jw1 = (miu2 - miu1) / (cov1 + cov2);
    index = mid - 32;
    for (int i = index; i < index + 64; i++)
    {
        int _sum = 0;
        miu1 = 0;
        miu2 = 0;
        cov1 = 0;
        cov2 = 0;
        for (int i = 0; i <= index; i++)
        {
            miu1 += i*pixel[i];
            _sum += pixel[i];
        }
        miu1 /= _sum;
        for (int i = index; i < 256; i++)
        {
            miu2 += i*pixel[i];
        }
        miu2 /= (sum - _sum);
        for (int i = 0; i <= index; i++)
        {
            cov1 += (i - miu1)*(i - miu1)*pixel[i];
        }
        cov1 /= (_sum - 1);
        for (int i = index; i < 256; i++)
        {
            cov2 += (i - miu2)*(i - miu2)*pixel[i];
        }
        cov2 /= (sum - _sum - 1);
        Jw2 = (miu2 - miu1) / (cov1 + cov2);
        if (Jw2 > Jw1)
        {
            mid = index;
        }
    }
    if (abc == 0) { mid = 108; }
    if (abc == 1) { mid = 150; }
    DN** Col[3] = { m_pppDN[m_Red],m_pppDN[m_Grn],m_pppDN[m_Blu] };
    DN* pBuffer = new DN[3*m_rows*m_columns];
    int i = 0;
    for (int k = 0; k< m_rows; k++)
    {
        for (int l = 0; l < m_columns; l++)
        {
            for (int j = 0; j < 3 ; j++)
            {
                if ((Col[j][k][l] > mid) && (j == 0))
                {
                    pBuffer[i] = 255;
                }
                else
                {
                    pBuffer[i] = 0;
                }
                i++;
            }
        }
    }
    QImage Image(pBuffer, m_columns, m_rows, QImage::Format_RGB888);
    QMatrix LeftMatrix;
    LeftMatrix.rotate(m_angle);
    Image = Image.transformed(LeftMatrix, Qt::SmoothTransformation);
    QSize picSize(Image.width()*m_zoom, Image.height()*m_zoom);
    Image = Image.scaled(picSize, Qt::KeepAspectRatio);
    return Image;
}

int CImgProcess::GetMaximum(DN** band) const
{
    int result = 0;
    for (int i = 0; i < m_rows; i++)
    {
        for (int j = 0; j < m_columns; j++)
        {
            result = (band[i][j] > result) ? band[i][j] : result;
        }
    }
    return result;
}

int CImgProcess::GetMinimum(DN** band) const
{
    int result = 256;
    for (int i = 0; i < m_rows; i++)
    {
        for (int j = 0; j < m_columns; j++)
        {
            result = (band[i][j] < result) ? band[i][j] : result;
        }
    }
    return result;
}
