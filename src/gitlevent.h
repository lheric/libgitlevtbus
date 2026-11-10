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
#ifndef GITLEVENT_H
#define GITLEVENT_H
#include <QString>
#include <QMap>
#include <QVariant>
#include <type_traits>
#include "gitldef.h"
#include "gitleventparam.h"

class GitlModule;
class GitlEventBus;

/*!
 * \brief The GitlEvent class represents an event.
 *
 *  Custom events: derive from GitlEvent and add GITL_EVENT(YourEvent) to the
 *  class. The event is then named after its class, and listeners can use the
 *  typed API: GitlModule::subscribeToEvt<YourEvent>().
 *
 *  Copying: dispatch(), dispatchAsync() and GitlEventBus::post()/postAsync()
 *  copy the event onto the bus with clone(), so a subclass sent that way MUST
 *  declare CLONABLE(subclassname) (GITL_EVENT includes it). Otherwise the event
 *  would be sliced; the bus detects this, logs an error and drops the event (and
 *  asserts in debug builds). dispatchSync() does not copy and needs neither.
 *
 *  Listeners receive the event as const GitlEvent&. One copy is shared by all
 *  listeners, possibly on different threads, so it must not be modified.
 */
class GitlEvent
{
    /// virtual copy pattern, please add this macro to all the subclass
    CLONABLE(GitlEvent)

public:
    GitlEvent( const QString& strEvtName );
    GitlEvent();
    virtual ~GitlEvent() {}

    /*!
     * \brief hasParameter if this event carries a specific parameter
     * \param strParam parameter name
     * \return
     */
    bool hasParameter(QString strParam) const;

    /*!
     * \brief getParameter get the value of a specific parameter
     * \param strParam parameter name
     * \return parameter value, if it does not exist, return a default-constructed QVariant
     */
    QVariant getParameter(const QString& strParam ) const;

    /*!
     * \brief setParameter set the value of a  specific parameter
     * \param strParam parameter name
     * \param rvValue parameter value
     * \return
     */
    bool setParameter(const QString& strParam, const QVariant& rvValue);


    /*!
     * \brief dispatch dispatch this event to event bus, all module subscribed to this event name will be notified.
     * \param pcEventBus If pcEventBus is NULL, it will find a global (default) event bus and post the event onto the bus.
     *                   Or you can specify another event bus.
     */
    void dispatch(GitlEventBus *pcEventBus = NULL) const;

    /*!
     * \brief dispatchSync Deliver this event without copying it and wait until every
     * listener (on every thread) has finished. Needs no CLONABLE; works with a local
     * variable. See GitlEventBus::postSync() for the blocking caveats.
     * \param pcEventBus the bus, or nullptr for the global one
     */
    void dispatchSync(GitlEventBus *pcEventBus = nullptr) const;

    /*!
     * \brief dispatchAsync Copy this event and return immediately; every listener,
     * including those on this thread, runs later. Needs CLONABLE or GITL_EVENT.
     * \param pcEventBus the bus, or nullptr for the global one
     */
    void dispatchAsync(GitlEventBus *pcEventBus = nullptr) const;

protected:    

    ADD_CLASS_FIELD(QString, strEvtName, getEvtName, setEvtName)            ///< event name

    ADD_CLASS_FIELD_NOSETTER(GitlEventParam, cParameters, getParameters)    ///< event parameters-value pair

};


/*!
 * \brief GITL_EVENT Declare a typed event class. Put it first in the class body.
 *
 *  - names the event after the class: staticEvtName() returns "classname", and
 *    every instance gets that name when it is constructed, so constructors do not
 *    need to pass one to GitlEvent (a subclass with its own GITL_EVENT gets its own
 *    name, since the most-derived class names the event last);
 *  - provides clone(), so the event also works with dispatch() and post();
 *  - marks the class for the typed API, which checks at compile time that the
 *    exact class declares GITL_EVENT (a subclass that forgets it is rejected).
 *
 *  Like CLONABLE, it leaves the class in a public: section.
 *
 *  \code
 *  class PlayerJoined : public GitlEvent
 *  {
 *      GITL_EVENT(PlayerJoined)
 *  public:
 *      explicit PlayerJoined(const QString& strName = QString()) { m_strName = strName; }
 *      ADD_CLASS_FIELD(QString, strName, getName, setName)
 *  };
 *  \endcode
 */
#define GITL_EVENT(classname) \
    private: \
        GitlDetail::EvtNamer m_cGitlEvtNamer{this, staticEvtName()}; \
    public: \
        typedef classname GitlEvtSelf; \
        static QString staticEvtName() { return QStringLiteral(#classname); } \
        classname* clone() const override { return new classname(*this); }

namespace GitlDetail
{
    /// used by GITL_EVENT: a member whose initializer sets the event's name. Members
    /// are initialized after the base classes, so the most-derived class wins.
    struct EvtNamer
    {
        EvtNamer(GitlEvent* pcEvt, const QString& strName) { pcEvt->setEvtName(strName); }
    };

    /// true if T declares GITL_EVENT(T) itself (not only inherited from a base)
    template <typename T, typename = void>
    struct IsTypedEvent : std::false_type {};

    template <typename T>
    struct IsTypedEvent<T, std::void_t<typename T::GitlEvtSelf>>
        : std::is_same<typename T::GitlEvtSelf, T> {};
}


#endif // GITLEVENT_H
