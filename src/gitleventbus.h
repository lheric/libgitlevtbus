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
#ifndef GITLEVENTBUS_H
#define GITLEVENTBUS_H

#include <QList>
#include <QHash>
#include <QVector>
#include <QStringList>
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QSharedPointer>
#include "gitldef.h"
#include "gitlevent.h"
#include "gitlmodule.h"


class GitlModuleDelegate;
/*!
 * \brief The GitlEventBus class represents the event bus.
 *
 * The bus keeps an index from event name to the modules subscribed to it, so
 * posting an event only reaches those modules. Modules on the posting thread
 * are called directly; modules on other threads get the event queued to their
 * own thread. Modules that did not subscribe are not touched at all.
 */
class GitlEventBus : public QObject
{
    Q_OBJECT
    friend class GitlModuleDelegate;
private:
    GitlEventBus();

public:
    /*!
     * \brief create The safe way to explictly create a new event bus
     * \return A new event bus owned by the caller, who must delete it.
     *         Deleting a bus is safe while modules are still attached:
     *         they are disconnected and their getEventBus() returns nullptr.
     */
    static GitlEventBus *create();

    /*!
     * \brief registerModule connect a module to the event bus.
     * Used internally by GitlModule; prefer GitlModule::attach().
     * \param pcModule
     * \return false if pcModule is null
     */
    bool registerModule(GitlModuleDelegate *pcModule);


    /*!
     * \brief unregisterModule disconncet a module from the event bus.
     * Used internally by GitlModule; prefer GitlModule::detach().
     * \param pcModule
     * \return true if the module was receiving any events from this bus
     */
    bool unregisterModule(GitlModuleDelegate *pcModule);

    /*!
     * \brief postSync Deliver an event without copying it, and wait until every
     * listener has finished. Listeners on this thread run immediately; listeners
     * on other threads run on their own threads (in parallel), and this call
     * blocks until they are done. The event subclass needs no CLONABLE, and the
     * event may be a local variable.
     *
     * Blocks forever if a receiving thread runs no event loop, and deadlocks if
     * two threads postSync() to each other at the same time. If a receiving
     * module is destroyed before handling the event, it is simply skipped.
     * eventTriggered() is not emitted (there is no copy that could outlive the call).
     */
    void postSync(const GitlEvent &rcEvt) const;

    /*!
     * \brief postAsync Copy the event and return immediately; every listener,
     * including those on this thread, is called later from its thread's event
     * loop. The event is copied with clone(), so an event subclass must declare
     * CLONABLE(...).
     */
    void postAsync(const GitlEvent &rcEvt) const;

public slots:
    /*! send event to event bus. Listeners on this thread run immediately, listeners
     *  on other threads later. The event is copied with clone(), so an event
     *  subclass must declare CLONABLE(...).
      */
    void post(const GitlEvent &rcEvt) const;

signals:
    /*! Emitted for every event posted with post() or postAsync(), whatever its
     *  name (not for postSync()). Modules do not use this signal; it is for
     *  logging or debugging.
     */
    void eventTriggered( QSharedPointer<GitlEvent> pcEvt ) const;


    ///SINGLETON design pattern
    SINGLETON_PATTERN_DECLARE(GitlEventBus)

private:
    /// how xRoute() delivers to modules
    enum class EventDeliveryMethod
    {
        Auto,   ///< this thread: call now; other threads: queue
        Sync,   ///< like Auto, then wait for the queued ones; no ownership taken
        Async   ///< queue for every module, including those on this thread
    };
    /// deliver an event to the modules subscribed to its name
    void xRoute(const QSharedPointer<GitlEvent> &pcEvt, EventDeliveryMethod eDeliveryMethod) const;
    /// copy an event with clone(), or log and return null if it would be sliced
    static QSharedPointer<GitlEvent> xClone(const GitlEvent &rcEvt, const char *szCaller);

    /// make pcModule's routes exactly rcEvtNames (empty: remove it from the bus),
    /// keeping its place in the routes it already had; true if it had any routes
    bool xSetRoutes(GitlModuleDelegate *pcModule, const QStringList &rcEvtNames);

    /// guards m_cRoutes; may be taken while a module's own mutex is held
    /// (module -> bus), never the other way round
    mutable QMutex m_cRouteMutex;
    /// event name -> subscribed modules, in subscription order
    QHash<QString, QVector<GitlModuleDelegate*>> m_cRoutes;

};

#endif // GITLEVTBUS_H
