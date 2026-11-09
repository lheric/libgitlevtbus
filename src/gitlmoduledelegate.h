/*******************************************************************************************
 * Copyright (c) 2013, Huang Li <lihuang55555@gmail.com>, IIPL <gitl.sysu.edu.cn>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, this list
 *   of conditions and the following disclaimer in the documentation and/or other
 *   materials provided with the distribution.
 * * Neither the name of the IIPL nor the names of its contributors may be used
 *   to endorse or promote products derived from this software without specific prior
 *   written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************************/
#ifndef GITLMODULEDELEGATE_H
#define GITLMODULEDELEGATE_H

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>
#include <QPointer>
#include <QStringList>
#include <functional>
#include "gitldef.h"
#include "gitlevent.h"

class GitlModule;
class GitlEventBus;

///
/// \brief GitlCallBack gitl event callback function.
/// The event is shared by every listener (possibly on different threads),
/// so it is passed as const.
///
typedef std::function<bool (const GitlEvent&)> GitlCallBack;

class GitlModuleDelegate : public QObject
{
    Q_OBJECT
    friend class GitlModule;
    friend class GitlEventBus;
private:
    explicit GitlModuleDelegate(GitlModule *pcDelegator, GitlEventBus *pcEventBus = nullptr);

    /// remove every subscription; used when the owning module is destroyed
    void clearSubscriptions();

    /// names of the events this module is subscribed to
    QStringList xSubscribedEvtNames() const;

public:
    /// unregisters from the bus, so the bus never routes to a destroyed module
    ~GitlModuleDelegate() override;

    /*!
     * \brief subscribeToEvtByName listening to an event by name.
     *        A module holds at most one callback per event name: subscribing
     *        again to the same name replaces the previous callback.
     * \param strEvtName event name
     */
    void subscribeToEvtByName( const QString& strEvtName,
                               GitlCallBack pfListener );

    /*!
     * \brief subscribeToEvtByName not listening to an event by name
     * \param strEvtName event name
     */
    void unsubscribeToEvtByName( const QString& strEvtName );

    /*!
     * \brief dispatchEvt dispatch an event to subscribers
     * \param pcEvt event
     */
    void dispatchEvt(const GitlEvent &rcEvt  ) const;


    /*!
     * \brief detach Detach the module
     */
    void detach();


    /*!
     * \brief attach Attach the module to a new event bus
     * \param pcEventBus
     */
    void attach(GitlEventBus *pcEventBus);

    /*!
     * \brief getGitlEvtBus the bus this module is attached to, or nullptr if it
     *        is detached or the bus has been deleted
     */
    GitlEventBus* getGitlEvtBus() const;

public slots:
    /*!
     * \brief detonate notifyed by event bus
     * \param cEvt
     * \return
     */
    bool detonate( QSharedPointer<GitlEvent> pcEvt );

protected:
    bool xIsListenToEvt(const QString& strEvtName);

    ADD_CLASS_FIELD( QString, strModuleName, getModuleName, setModuleName )
    ADD_CLASS_FIELD_PRIVATE( CONCATE(QMap<QString, GitlCallBack>), cListeningEvts )
    ADD_CLASS_FIELD_PRIVATE(GitlModule*, pcDelegator)

private:
    /// QPointer becomes null automatically if the bus is deleted
    QPointer<GitlEventBus> m_pcGitlEvtBus;

    /// guards m_cListeningEvts and m_pcGitlEvtBus, which may be touched from
    /// a thread other than the one this object lives in. Lock order: this mutex,
    /// then the bus's route mutex; the bus never calls back into a module while
    /// holding its own lock.
    mutable QMutex m_cMutex;

};

#endif // GITLMODULEDELEGATE_H
