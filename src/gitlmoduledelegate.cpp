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
#include <iostream>
using namespace std;
GitlModuleDelegate::GitlModuleDelegate(GitlModule *pcDelegator, GitlEventBus* pcEventBus)
{
    m_pcDelegator = pcDelegator;
    if(pcEventBus == NULL)
        m_pcGitlEvtBus = GitlEventBus::getInstance();
    else
        m_pcGitlEvtBus = pcEventBus;
    m_pcGitlEvtBus->registerModule(this);
    m_strModuleName = "undefined_module_name";

}



void GitlModuleDelegate::subscribeToEvtByName(const QString& strEvtName, GitlCallBack pfListener )
{
    m_cListeningEvts.insert(strEvtName, pfListener);
    return;
}


void GitlModuleDelegate::unsubscribeToEvtByName( const QString& strEvtName )
{
    m_cListeningEvts.remove(strEvtName);
}

bool GitlModuleDelegate::detonate(QSharedPointer<GitlEvent> pcEvt )
{
    QMap<QString, std::function<bool (GitlEvent&)>>::iterator p =
            m_cListeningEvts.find(pcEvt->getEvtName());

    if( p != m_cListeningEvts.end() )
        (p.value())(*pcEvt.data());
    return true;
}

bool GitlModuleDelegate::xIsListenToEvt( const QString& strEvtName )
{
    return m_cListeningEvts.contains(strEvtName);
}

void GitlModuleDelegate::dispatchEvt( const GitlEvent& rcEvt ) const
{
    if(m_pcGitlEvtBus != NULL)
        m_pcGitlEvtBus->post(rcEvt);
}

void GitlModuleDelegate::detach()
{
    if(m_pcGitlEvtBus != NULL)
        m_pcGitlEvtBus->unregisterModule(this);
    m_pcGitlEvtBus = NULL;
}

void GitlModuleDelegate::attach(GitlEventBus *pcEventBus)
{
    if(pcEventBus == NULL)
        return;
    detach();
    m_pcGitlEvtBus = pcEventBus;
    m_pcGitlEvtBus->registerModule(this);
}
