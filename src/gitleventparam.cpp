#include "gitleventparam.h"
#include <QDebug>
GitlEventParam::GitlEventParam()
{
}

bool GitlEventParam::hasParameter(QString strParam) const
{
    return m_cParameters.contains(strParam);
}

QVariant GitlEventParam::getParameter(const QString& strParam ) const
{
    QVariant rvValue;
    if( m_cParameters.contains(strParam) )
    {
        rvValue = m_cParameters[strParam];
    }
    else
    {
        qWarning() << QString("Parameter %1 NOT found.").arg(strParam);
    }
    return rvValue;
}

bool GitlEventParam::setParameter(const QString& strParam, const QVariant& rvValue)
{
    m_cParameters[strParam] = rvValue;
    return true;
}
