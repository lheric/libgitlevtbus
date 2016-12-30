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

#include "gitldef.h"
#include "gitlevent.h"

#include "gitlmoduledelegate.h"

class GitlEventBus;

/*!
 * \brief The GitlModule class represents a module
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
    GitlModule(GitlEventBus* pcEventBus = NULL);

    /*!
     * \brief subscribeToEvtByName Subscribe to an event
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
    void dispatchEvt(GitlEvent &rcEvt );

    /*!
     * \brief setModuleName Set the name of this module. That's ok if you do not
     * give a name to this module. But for better debugging, we recommend you name it.
     * \param strModuleName name for this module
     */
    void setModuleName(QString strModuleName );

    /**
     * @brief getEventBus Get the event bus that this module is attached to
     * @return
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


    /// Delegate pattern
    /// Avoiding this class becoming a subclass of QObject
    /// (GUI class is based on QOBject, but QObject doesn't support virtual inheritance).
    ADD_CLASS_FIELD_PRIVATE( GitlModuleDelegate, cDelegate )
};

#endif // GITLMODULE_H
