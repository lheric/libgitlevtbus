#include "gitlevent.h"
#include "gitlmodual.h"
#include <QDebug>

GitlEvent::GitlEvent( const QString& strEvtName )
{
    this->m_strEvtName = strEvtName;
}


GitlEvent::GitlEvent()
{
    this->m_strEvtName = "UNKNOWN";
}

bool GitlEvent::hasParameter(QString strParam) const
{
    return m_cParameters.contains(strParam);
}

QVariant GitlEvent::getParameter(const QString& strParam ) const
{
    QVariant rvValue;
    if( m_cParameters.contains(strParam) )
    {
        rvValue = m_cParameters[strParam];
    }
    else
    {
        qWarning() << QString("Event Parameter %1 NOT found.").arg(strParam);
    }
    return rvValue;
}

bool GitlEvent::setParameter(const QString& strParam, const QVariant& rvValue)
{
    m_cParameters[strParam] = rvValue;
    return true;
}
