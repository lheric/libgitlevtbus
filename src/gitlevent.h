#ifndef GITLEVENT_H
#define GITLEVENT_H
#include <QString>
#include <QMap>
#include <QVariant>
#include "gitldef.h"


class GitlModual;

class GitlEvent
{
public:
    GitlEvent( const QString& strEvtName );
    GitlEvent();

    bool hasParameter(QString strParam) const;
    QVariant getParameter(const QString& strParam ) const;
    bool setParameter(const QString& strParam, const QVariant& rvValue);



protected:
    QMap<QString,QVariant> m_cParameters;
    ADD_CLASS_FIELD(QString, strEvtName, getEvtName, setEvtName)

};


#endif // GITLEVENT_H
