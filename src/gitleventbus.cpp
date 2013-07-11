#include "gitleventbus.h"
#include <QDebug>

SINGLETON_PATTERN_IMPLIMENT(GitlEventBus)

GitlEventBus::GitlEventBus()
{
}

/*! connect a modual to the event bus
  */
bool GitlEventBus::registerModual(GitlModualDelegate* pcModual)
{
    QMutexLocker cModualLocker(&m_cModualQueMutex);
    qRegisterMetaType< QSharedPointer<GitlEvent> >("GitlEvent");
    connect(this,     SIGNAL(eventTriggered(QSharedPointer<GitlEvent>) ),
            pcModual, SLOT  (detonate      (QSharedPointer<GitlEvent>) ),
            Qt::AutoConnection );

    return true;
}



/*! send event to event bus
  */
void GitlEventBus::post(GitlEvent& rcEvt)
{
    QMutexLocker cModualLocker(&m_cModualDispMutex);
    QSharedPointer<GitlEvent> pcEvtCopy( rcEvt.clone() );


    /// notify moduals
    emit eventTriggered(pcEvtCopy);

}

