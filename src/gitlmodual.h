#ifndef GITLMODUAL_H
#define GITLMODUAL_H

#include <QVector>
#include <QString>
#include <QSharedPointer>

#include "gitldef.h"
#include "gitlevent.h"

#include "gitlmodualdelegate.h"

class GitlEventBus;

class GitlModual
{
public:
    GitlModual();
    /*! The virtual function to deal with specific event
      */
    virtual bool detonate( GitlEvent& ) {return true;}
    void subscribeToEvtByName( const QString& strEvtName );
    void unsubscribeToEvtByName( const QString& strEvtName );
    void dispatchEvt(GitlEvent &rcEvt );
    void setModualName(QString strModualName );

    ADD_CLASS_FIELD_PRIVATE( GitlModualDelegate, cDelegate )
};

#endif // GITLMODUAL_H
