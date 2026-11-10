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
#include "gitlmoduledelegate.h"
#include "gitleventbus.h"
#include <QDebug>

GitlModuleDelegate::GitlModuleDelegate(GitlModule *pcDelegator, GitlEventBus* pcEventBus)
{
    m_pcDelegator = pcDelegator;
    if(pcEventBus == nullptr)
        m_pcGitlEvtBus = GitlEventBus::getInstance();
    else
        m_pcGitlEvtBus = pcEventBus;
    m_strModuleName = "undefined_module_name";
}

GitlModuleDelegate::~GitlModuleDelegate()
{
    detach();
}

void GitlModuleDelegate::subscribeToEvtByName(const QString& strEvtName, GitlCallBack pfListener )
{
    QMutexLocker cLocker(&m_cMutex);
    m_cListeningEvts.insert(strEvtName, pfListener);   ///< replaces any existing callback
    if(!m_pcGitlEvtBus.isNull())
        m_pcGitlEvtBus->xSetRoutes(this, m_cListeningEvts.keys());
}


void GitlModuleDelegate::unsubscribeToEvtByName( const QString& strEvtName )
{
    QMutexLocker cLocker(&m_cMutex);
    if(m_cListeningEvts.remove(strEvtName) > 0 && !m_pcGitlEvtBus.isNull())
        m_pcGitlEvtBus->xSetRoutes(this, m_cListeningEvts.keys());
}

QStringList GitlModuleDelegate::xSubscribedEvtNames() const
{
    QMutexLocker cLocker(&m_cMutex);
    return m_cListeningEvts.keys();
}

bool GitlModuleDelegate::detonate(QSharedPointer<GitlEvent> pcEvt )
{
    /// copy the callback under the lock, then call it without holding the lock,
    /// so a callback may subscribe/unsubscribe/dispatch without deadlocking
    GitlCallBack pfListener;
    {
        QMutexLocker cLocker(&m_cMutex);
        if( m_pcGitlEvtBus.isNull() )    ///< detached: drop events still queued from the old bus
            return false;
        auto p = m_cListeningEvts.constFind(pcEvt->getEvtName());
        if( p == m_cListeningEvts.constEnd() )
            return true;
        pfListener = p.value();
    }
    const GitlEvent& rcEvt = *pcEvt;
    pfListener(rcEvt);
    return true;
}

bool GitlModuleDelegate::xIsListenToEvt( const QString& strEvtName )
{
    QMutexLocker cLocker(&m_cMutex);
    return m_cListeningEvts.contains(strEvtName);
}

void GitlModuleDelegate::dispatchEvt( const GitlEvent& rcEvt ) const
{
    GitlEventBus* pcBus = getGitlEvtBus();
    if(pcBus != nullptr)    ///< post without holding the lock: a direct connection may call back into detonate()
        pcBus->post(rcEvt);
}

void GitlModuleDelegate::dispatchSyncEvt( const GitlEvent& rcEvt ) const
{
    GitlEventBus* pcBus = getGitlEvtBus();
    if(pcBus != nullptr)    ///< never hold our lock while waiting for other threads
        pcBus->postSync(rcEvt);
}

void GitlModuleDelegate::dispatchAsyncEvt( const GitlEvent& rcEvt ) const
{
    GitlEventBus* pcBus = getGitlEvtBus();
    if(pcBus != nullptr)
        pcBus->postAsync(rcEvt);
}

GitlEventBus* GitlModuleDelegate::getGitlEvtBus() const
{
    QMutexLocker cLocker(&m_cMutex);
    return m_pcGitlEvtBus.data();
}

void GitlModuleDelegate::detach()
{
    QMutexLocker cLocker(&m_cMutex);
    if(!m_pcGitlEvtBus.isNull())
        m_pcGitlEvtBus->xSetRoutes(this, QStringList());
    m_pcGitlEvtBus.clear();
}

void GitlModuleDelegate::attach(GitlEventBus *pcEventBus)
{
    if(pcEventBus == nullptr)
        return;
    QMutexLocker cLocker(&m_cMutex);
    if(!m_pcGitlEvtBus.isNull())
        m_pcGitlEvtBus->xSetRoutes(this, QStringList());
    m_pcGitlEvtBus = pcEventBus;
    m_pcGitlEvtBus->xSetRoutes(this, m_cListeningEvts.keys());   ///< carry the subscriptions over
}
