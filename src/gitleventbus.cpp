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
#include "gitleventbus.h"
#include <QDebug>
#include <QPointer>
#include <QSemaphore>
#include <QThread>
#include <memory>
#include <typeinfo>

/// needed for queued connections to eventTriggered() made by users
Q_DECLARE_METATYPE( QSharedPointer<GitlEvent> )

SINGLETON_PATTERN_IMPLIMENT(GitlEventBus)

GitlEventBus::GitlEventBus()
{
    qRegisterMetaType< QSharedPointer<GitlEvent> >("QSharedPointer<GitlEvent>");
}

GitlEventBus *GitlEventBus::create()
{
    return new GitlEventBus();
}

/*! connect a module to the event bus
  */
bool GitlEventBus::registerModule(GitlModuleDelegate* pcModule)
{
    if(pcModule == nullptr)
        return false;
    /// read the module's subscriptions before taking the bus lock (lock order: module -> bus)
    xSetRoutes(pcModule, pcModule->xSubscribedEvtNames());
    return true;
}

bool GitlEventBus::unregisterModule(GitlModuleDelegate *pcModule)
{
    return xSetRoutes(pcModule, QStringList());
}

bool GitlEventBus::xSetRoutes(GitlModuleDelegate *pcModule, const QStringList &rcEvtNames)
{
    QMutexLocker cLocker(&m_cRouteMutex);
    bool bHadRoutes = false;
    /// drop the routes for events no longer subscribed; the others keep their order
    for(auto it = m_cRoutes.begin(); it != m_cRoutes.end(); )
    {
        if(it.value().contains(pcModule))
        {
            bHadRoutes = true;
            if(!rcEvtNames.contains(it.key()))
                it.value().removeAll(pcModule);
        }
        it = it.value().isEmpty() ? m_cRoutes.erase(it) : std::next(it);
    }
    /// add the missing ones at the end
    for(const QString& strEvtName : rcEvtNames)
    {
        QVector<GitlModuleDelegate*>& vcModules = m_cRoutes[strEvtName];
        if(!vcModules.contains(pcModule))
            vcModules.append(pcModule);
    }
    return bHadRoutes;
}


/*! send event to event bus
  */
void GitlEventBus::post(const GitlEvent& rcEvt) const
{
    QSharedPointer<GitlEvent> pcEvtCopy = xClone(rcEvt, "GitlEventBus::post");
    if(pcEvtCopy.isNull())
        return;
    xRoute(pcEvtCopy, EventDeliveryMethod::Auto);
}

void GitlEventBus::postAsync(const GitlEvent& rcEvt) const
{
    QSharedPointer<GitlEvent> pcEvtCopy = xClone(rcEvt, "GitlEventBus::postAsync");
    if(pcEvtCopy.isNull())
        return;
    xRoute(pcEvtCopy, EventDeliveryMethod::Async);
}

void GitlEventBus::postSync(const GitlEvent& rcEvt) const
{
    /// No copy: a non-owning pointer to the caller's object. This is safe because
    /// xRoute() does not return until every listener is done with it, and listeners
    /// only get const access.
    QSharedPointer<GitlEvent> pcEvt(const_cast<GitlEvent*>(&rcEvt), [](GitlEvent*){});
    xRoute(pcEvt, EventDeliveryMethod::Sync);
}

QSharedPointer<GitlEvent> GitlEventBus::xClone(const GitlEvent &rcEvt, const char *szCaller)
{
    QSharedPointer<GitlEvent> pcEvtCopy( rcEvt.clone() );

    /// A subclass without CLONABLE(...) is sliced to its base class by clone(),
    /// and listeners that downcast it would hit undefined behaviour.
    const GitlEvent& rcEvtCopy = *pcEvtCopy;
    if( typeid(rcEvtCopy) != typeid(rcEvt) )
    {
        qCritical("%s: event '%s' of type '%s' was cloned as '%s'. "
                  "Add CLONABLE(...) to the event subclass, or use a "
                  "dispatch that does not copy (dispatchSync). The event was not dispatched.",
                  szCaller, qPrintable(rcEvt.getEvtName()), typeid(rcEvt).name(), typeid(rcEvtCopy).name());
        Q_ASSERT_X(false, szCaller, "event subclass is missing CLONABLE(...)");
        return QSharedPointer<GitlEvent>();
    }
    return pcEvtCopy;
}

void GitlEventBus::xRoute(const QSharedPointer<GitlEvent> &pcEvt, EventDeliveryMethod eDeliveryMethod) const
{
    /// Notify only the modules subscribed to this event name.
    /// Queued deliveries are posted while holding the lock: a module unregisters
    /// under the same lock before it is destroyed, so it cannot disappear while we
    /// queue to it, and a queued call is dropped by Qt if its receiver is destroyed
    /// before delivery.
    /// Modules called directly are called after the lock is released, so their
    /// callbacks may subscribe, dispatch or delete modules; QPointer guards
    /// against a callback deleting a module that is later in the list.
    QVector<QPointer<GitlModuleDelegate>> vcDirect;
    QSemaphore cSyncDone;
    int iSyncPending = 0;
    {
        QMutexLocker cLocker(&m_cRouteMutex);
        auto it = m_cRoutes.constFind(pcEvt->getEvtName());
        if(it != m_cRoutes.constEnd())
        {
            QThread* pcCurrent = QThread::currentThread();
            for(GitlModuleDelegate* pcModule : it.value())
            {
                if(eDeliveryMethod != EventDeliveryMethod::Async && pcModule->thread() == pcCurrent)
                    vcDirect.append(pcModule);
                else if(eDeliveryMethod == EventDeliveryMethod::Sync)
                {
                    /// the queued call owns a releaser: the waiting sender is released
                    /// after the listener ran, and also if Qt drops the call because
                    /// the receiving module was destroyed first
                    auto pcReleaser = std::make_shared<QSemaphoreReleaser>(cSyncDone);
                    ++iSyncPending;
                    QMetaObject::invokeMethod(pcModule, [pcModule, pcEvt, pcReleaser]()
                                              { pcModule->detonate(pcEvt); },
                                              Qt::QueuedConnection);
                }
                else
                    QMetaObject::invokeMethod(pcModule, [pcModule, pcEvt]()
                                              { pcModule->detonate(pcEvt); },
                                              Qt::QueuedConnection);
            }
        }
    }
    for(const QPointer<GitlModuleDelegate>& pcModule : vcDirect)
        if(!pcModule.isNull())
            pcModule->detonate(pcEvt);

    if(eDeliveryMethod == EventDeliveryMethod::Sync)
    {
        cSyncDone.acquire(iSyncPending);   ///< wait for the listeners on other threads
        return;                            ///< no eventTriggered: nothing may outlive this call
    }

    /// only for external listeners (logging, debugging); modules do not connect to it
    emit eventTriggered(pcEvt);
}

