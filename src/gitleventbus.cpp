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
#include <QMetaMethod>
#include <QPointer>
#include <QThread>
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
    xRegister(pcModule, pcModule->xSubscribedEvtNames());
    return true;
}

bool GitlEventBus::unregisterModule(GitlModuleDelegate *pcModule)
{
    return xUnregister(pcModule);
}

void GitlEventBus::xRegister(GitlModuleDelegate *pcModule, const QStringList &rcEvtNames)
{
    QMutexLocker cLocker(&m_cRouteMutex);
    m_cModuleRoutes[pcModule];   ///< registered, even with no subscriptions yet
    cLocker.unlock();
    for(const QString& strEvtName : rcEvtNames)
        xAddRoute(pcModule, strEvtName);
}

bool GitlEventBus::xUnregister(GitlModuleDelegate *pcModule)
{
    QMutexLocker cLocker(&m_cRouteMutex);
    auto it = m_cModuleRoutes.find(pcModule);
    if(it == m_cModuleRoutes.end())
        return false;
    for(const QString& strEvtName : it.value())
    {
        auto r = m_cRoutes.find(strEvtName);
        if(r == m_cRoutes.end())
            continue;
        r.value().removeAll(pcModule);
        if(r.value().isEmpty())
            m_cRoutes.erase(r);
    }
    m_cModuleRoutes.erase(it);
    return true;
}

void GitlEventBus::xAddRoute(GitlModuleDelegate *pcModule, const QString &strEvtName)
{
    QMutexLocker cLocker(&m_cRouteMutex);
    auto it = m_cModuleRoutes.find(pcModule);
    if(it == m_cModuleRoutes.end() || it.value().contains(strEvtName))
        return;     ///< not registered, or already routed
    it.value().insert(strEvtName);
    m_cRoutes[strEvtName].append(pcModule);
}

void GitlEventBus::xRemoveRoute(GitlModuleDelegate *pcModule, const QString &strEvtName)
{
    QMutexLocker cLocker(&m_cRouteMutex);
    auto it = m_cModuleRoutes.find(pcModule);
    if(it == m_cModuleRoutes.end() || !it.value().remove(strEvtName))
        return;
    auto r = m_cRoutes.find(strEvtName);
    if(r == m_cRoutes.end())
        return;
    r.value().removeAll(pcModule);
    if(r.value().isEmpty())
        m_cRoutes.erase(r);
}


/*! send event to event bus
  */
void GitlEventBus::post(const GitlEvent& rcEvt) const
{    
    QSharedPointer<GitlEvent> pcEvtCopy( rcEvt.clone() );

    /// A subclass without CLONABLE(...) is sliced to its base class by clone(),
    /// and listeners that downcast it would hit undefined behaviour.
    const GitlEvent& rcEvtCopy = *pcEvtCopy;
    if( typeid(rcEvtCopy) != typeid(rcEvt) )
    {
        qCritical("GitlEventBus::post: event '%s' of type '%s' was cloned as '%s'. "
                  "Add CLONABLE(...) to the event subclass. The event was not dispatched.",
                  qPrintable(rcEvt.getEvtName()), typeid(rcEvt).name(), typeid(rcEvtCopy).name());
        Q_ASSERT_X(false, "GitlEventBus::post", "event subclass is missing CLONABLE(...)");
        return;
    }

    /// Notify only the modules subscribed to this event name.
    /// Modules on other threads get the event queued to their own thread. This is
    /// done while holding the lock: a module unregisters under the same lock before
    /// it is destroyed, so it cannot disappear while we queue to it, and a queued
    /// event is dropped by Qt if its receiver is destroyed before delivery.
    /// Modules on this thread are called after the lock is released, so their
    /// callbacks may subscribe, dispatch or delete modules; QPointer guards
    /// against a callback deleting a module that is later in the list.
    QVector<QPointer<GitlModuleDelegate>> vcDirect;
    {
        QMutexLocker cLocker(&m_cRouteMutex);
        auto it = m_cRoutes.constFind(pcEvtCopy->getEvtName());
        if(it != m_cRoutes.constEnd())
        {
            QThread* pcCurrent = QThread::currentThread();
            for(GitlModuleDelegate* pcModule : it.value())
            {
                if(pcModule->thread() == pcCurrent)
                    vcDirect.append(pcModule);
                else
                    QMetaObject::invokeMethod(pcModule, [pcModule, pcEvtCopy]()
                                              { pcModule->detonate(pcEvtCopy); },
                                              Qt::QueuedConnection);
            }
        }
    }
    for(const QPointer<GitlModuleDelegate>& pcModule : vcDirect)
        if(!pcModule.isNull())
            pcModule->detonate(pcEvtCopy);

    /// only for external listeners (logging, debugging); modules do not connect to it
    static const QMetaMethod cSignal = QMetaMethod::fromSignal(&GitlEventBus::eventTriggered);
    if(isSignalConnected(cSignal))
        emit eventTriggered(pcEvtCopy);
}

