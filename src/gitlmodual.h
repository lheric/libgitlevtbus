#ifndef GITLMODUAL_H
#define GITLMODUAL_H

#include <QVector>
#include <QString>

#include "gitldef.h"
#include "gitlevent.h"

#include "gitlmodualdelegate.h"

class GitlEventBus;

class GitlModual
{
public:
    GitlModual();
    /*!
     * \brief detonate recall function for the subscribed event
     * \param cEvt
     * \return
     */
    virtual bool detonate( GitlEvent cEvt );

    /*!
     * \brief subscribeToEvtByName listen to event by name
     * \param strEvtName event name
     */
    void subscribeToEvtByName( const QString& strEvtName );

    /*!
     * \brief unsubscribeToEvtByName unsubscribe to event by name
     * \param strEvtName event name
     */
    void unsubscribeToEvtByName( const QString& strEvtName );

    /*!
     * \brief dispatchEvt post an event to event bus
     * \param pcEvt event
     */
    void dispatchEvt(const GitlEvent &rcEvt );

    /*!
     * \brief setModualName set the modual name (not required, only for better debugging)
     * \param strModualName
     */
    void setModualName(const QString& strModualName );


    ADD_CLASS_FIELD_PRIVATE( GitlModualDelegate, cDelegate )    /// delegate
};

#endif // GITLMODUAL_H
