#ifndef GITLEVENT_H
#define GITLEVENT_H
#include <QString>
#include <QMap>
#include <QVariant>
#include <QSharedPointer>
#include "gitldef.h"


class GitlModual;

class GitlEvent
{
    /// virtual copy pattern, please add this macro to all the subclass
    VIRTUAL_COPY_PATTERN(GitlEvent)

public:
    GitlEvent( const QString& strEvtName );
    GitlEvent();
    virtual ~GitlEvent() {}

    bool hasParameter(QString strParam) const;
    QVariant getParameter(const QString& strParam ) const;
    bool setParameter(const QString& strParam, const QVariant& rvValue);
    void dispatch();

protected:
    QMap<QString,QVariant> m_cParameters;
    ADD_CLASS_FIELD(QString, strEvtName, getEvtName, setEvtName)



};


#endif // GITLEVENT_H
