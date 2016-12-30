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
#include "gitlmodule.h"
#include "gitleventbus.h"
#include <QDebug>

GitlModule::GitlModule(GitlEventBus *pcEventBus) :
    m_cDelegate(this, pcEventBus)
{    
}

void GitlModule::subscribeToEvtByName( const QString& strEvtName,
                                       const GitlCallBack& pfListener )
{
    return m_cDelegate.subscribeToEvtByName(strEvtName, pfListener);
}

void GitlModule::unsubscribeToEvtByName( const QString& strEvtName )
{
    return m_cDelegate.unsubscribeToEvtByName(strEvtName);
}

void GitlModule::dispatchEvt( GitlEvent& rcEvt )
{
    m_cDelegate.dispatchEvt(rcEvt);
}

void GitlModule::setModuleName( QString strModuleName )
{
    m_cDelegate.setModuleName(strModuleName);
}

GitlEventBus *GitlModule::getEventBus()
{
    return m_cDelegate.getGitlEvtBus();
}

void GitlModule::detach()
{
    m_cDelegate.detach();
}

void GitlModule::attach(GitlEventBus *pcEventBus)
{
    m_cDelegate.attach(pcEventBus);
}
