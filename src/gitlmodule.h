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
#ifndef GITLMODULE_H
#define GITLMODULE_H

#include <QVector>
#include <QString>
#include <QSharedPointer>
#include <QtGlobal>
#include <type_traits>
#include <typeinfo>

#include "gitldef.h"
#include "gitlevent.h"

#include "gitlmoduledelegate.h"

class GitlEventBus;
class QThread;

/*!
 * \brief The GitlModule class represents a module.
 *
 * Threading: callbacks always run on the thread the module lives in, which is
 * the thread that created it unless it is moved with moveModuleToThread(). Events
 * posted from another thread are queued, so that thread must run a Qt event
 * loop (e.g. QThread::exec(), which QThread::run() calls by default).
 */

class GitlModule
{
public:
    /**
     * @brief GitlModule Represents a module in the event bus. It will keep listening to events
     *                   in the event bus and catch those it is interested in.
     * @param pcEventBus If pcEventBus it will find a gloabl event bus using singleton pattern.
     *                   Or you can specify an exsiting event bus.
     */
    GitlModule(GitlEventBus* pcEventBus = nullptr);

    /*!
     * \brief ~GitlModule Detaches from the bus.
     * Callbacks run on the thread the module lives in. If a subclass's
     * callbacks use its own members and the module can be destroyed from another
     * thread, call detach() at the start of the subclass destructor.
     */
    virtual ~GitlModule();

    /*!
     * \brief subscribeToEvtByName Subscribe to an event. A module holds at most
     * one callback per event name: subscribing again replaces the previous one.
     * \param strEvtName event name
     * \param pfListener listener callback function
     */
    void subscribeToEvtByName(const QString& strEvtName,
                              const GitlCallBack& pfListener  );

    /*!
     * \brief unsubscribeToEvtByName Unsubscribe to an event
     * \param strEvtName event name
     */
    void unsubscribeToEvtByName( const QString& strEvtName );

    /*!
     * \brief dispatchEvt Dispatch an event
     * \param rcEvt event
     */
    void dispatchEvt(const GitlEvent &rcEvt );

    /*!
     * \brief dispatchSync Deliver an event without copying it and wait until every
     * listener, on every thread, has finished. The event subclass needs no
     * CLONABLE, and the event may be a local variable:
     * \code
     * PlayerJoined cEvt("alice");
     * cModule.dispatchSync(cEvt);   // all listeners are done when this returns
     * \endcode
     * Listeners on this thread run immediately; listeners on other threads run on
     * their own threads, in parallel, while this call waits. It blocks forever if
     * a receiving thread runs no event loop, and deadlocks if two threads
     * dispatchSync() to each other at the same time.
     */
    void dispatchSync(const GitlEvent &rcEvt);

    /*!
     * \brief dispatchAsync Copy the event and return immediately. Every listener,
     * including those on this thread, runs later from its thread's event loop,
     * so this thread needs a running event loop too. The event is copied with
     * clone(), so an event subclass needs CLONABLE or GITL_EVENT.
     */
    void dispatchAsync(const GitlEvent &rcEvt);

    /*!
     * \brief subscribeToEvt Subscribe to a typed event (one declaring GITL_EVENT).
     * The listener receives the event as const EvtT&; its return value, if any, is ignored.
     * Shares the per-name slot with subscribeToEvtByName(EvtT::staticEvtName(), ...):
     * subscribing either way replaces the other. An event with this name that is not
     * an EvtT (e.g. a plain GitlEvent) is skipped with a warning.
     * \code
     * cModule.subscribeToEvt<PlayerJoined>([](const PlayerJoined& rcEvt)
     * {
     *     qDebug() << rcEvt.getName() << "joined";
     * });
     * \endcode
     */
    template <typename EvtT, typename F>
    void subscribeToEvt(F pfListener)
    {
        static_assert(GitlDetail::IsTypedEvent<EvtT>::value,
                      "EvtT must declare GITL_EVENT(EvtT) in its own class body");
        subscribeToEvtByName(EvtT::staticEvtName(),
            [pfListener](const GitlEvent& rcEvt) mutable -> bool
            {
                const EvtT* pcEvt = dynamic_cast<const EvtT*>(&rcEvt);
                if(pcEvt == nullptr)
                {
                    qWarning("GitlModule::subscribeToEvt: event '%s' of type '%s' is not a '%s'; skipped.",
                             qPrintable(rcEvt.getEvtName()), typeid(rcEvt).name(), typeid(EvtT).name());
                    return false;
                }
                pfListener(*pcEvt);   ///< any return value is ignored, as for all callbacks
                return true;
            });
    }

    /*!
     * \brief unsubscribeToEvt Unsubscribe from a typed event
     */
    template <typename EvtT>
    void unsubscribeToEvt()
    {
        static_assert(GitlDetail::IsTypedEvent<EvtT>::value,
                      "EvtT must declare GITL_EVENT(EvtT) in its own class body");
        unsubscribeToEvtByName(EvtT::staticEvtName());
    }

    /*!
     * \brief setModuleName Set the name of this module. That's ok if you do not
     * give a name to this module. But for better debugging, we recommend you name it.
     * \param strModuleName name for this module
     */
    void setModuleName(QString strModuleName );

    /**
     * @brief getEventBus Get the event bus that this module is attached to
     * @return the bus, or nullptr if the module is detached or the bus was deleted
     */
    GitlEventBus* getEventBus();

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
     * \brief moveModuleToThread Make this module's callbacks run on another thread.
     * GitlModule is not a QObject, so if a subclass is also a QObject, its own
     * QObject::moveToThread() does NOT move the module's callbacks; call this too.
     * Like QObject::moveToThread(), it must be called from the thread the module
     * currently lives in.
     * \param pcThread target thread; it needs a running event loop to receive events
     */
    void moveModuleToThread(QThread *pcThread);

    /*!
     * \brief moduleThread The thread this module's callbacks run on
     */
    QThread* moduleThread() const;


    /// Delegate pattern
    /// Avoiding this class becoming a subclass of QObject
    /// (GUI class is based on QOBject, but QObject doesn't support virtual inheritance).
    ADD_CLASS_FIELD_PRIVATE( GitlModuleDelegate, cDelegate )
};

#endif // GITLMODULE_H
