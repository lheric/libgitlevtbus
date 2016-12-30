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
#include "gitldef.h"
#include "gitleventparam.h"

class GitlModule;
class GitlEventBus;

/*!
 * \brief The GitlEvent class represents an event.
 *  If you want to create an custom event by inherit GitlEvent, you ***MUST***
 *  reimplement the 'clone' method in this class. This can be done by adding
 *  VIRTUAL_COPY_PATTERN(subclassname) in the subclass. Otherwise the application
 *  may crash
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

protected:    

    ADD_CLASS_FIELD(QString, strEvtName, getEvtName, setEvtName)            ///< event name

    ADD_CLASS_FIELD_NOSETTER(GitlEventParam, cParameters, getParameters)    ///< event parameters-value pair

};


#endif // GITLEVENT_H
