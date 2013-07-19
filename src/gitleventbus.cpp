#include "gitleventbus.h"
#include <QDebug>

SINGLETON_PATTERN_IMPLIMENT(GitlEventBus)

GitlEventBus::GitlEventBus()
{
}

/*! connect a modual to the event bus
  */
Q_DECLARE_METATYPE( QSharedPointer<GitlEvent> )
bool GitlEventBus::registerModual(GitlModualDelegate* pcModual)
{    
    qRegisterMetaType< QSharedPointer<GitlEvent> >("QSharedPointer<GitlEvent>");
    connect(this,     SIGNAL(eventTriggered(QSharedPointer<GitlEvent>) ),
            pcModual, SLOT  (detonate      (QSharedPointer<GitlEvent>) ),
            Qt::AutoConnection );

    return true;
}



/*! send event to event bus
  */
void GitlEventBus::post(GitlEvent& rcEvt)
{    
    QSharedPointer<GitlEvent> pcEvtCopy( rcEvt.clone() );


    /// notify moduals
    emit eventTriggered(pcEvtCopy);

}

