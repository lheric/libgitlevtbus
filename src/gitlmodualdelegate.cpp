#include "gitlmodualdelegate.h"
#include "gitleventbus.h"
#include <QDebug>
#include <iostream>
using namespace std;
GitlModualDelegate::GitlModualDelegate(GitlModual *pcDelegator)
{
    m_pcDelegator = pcDelegator;
    m_pcGitlEvtBus = GitlEventBus::getInstance();
    m_pcGitlEvtBus->registerModual(this);
    m_strModualName = "undefined_modual_name";

}



void GitlModualDelegate::subscribeToEvtByName( const QString& strEvtName )
{
    m_cListeningEvts.push_back(strEvtName);
    return;
}


void GitlModualDelegate::unsubscribeToEvtByName( const QString& strEvtName )
{
    for(int i = 0; i < m_cListeningEvts.size(); i++)
    {
        if(m_cListeningEvts.at(i) == strEvtName)
        {
            m_cListeningEvts.remove(i);
            return;
        }
    }

    return;
}

bool GitlModualDelegate::detonate(QSharedPointer<GitlEvent> pcEvt )
{
    if( xIsListenToEvt(pcEvt->getEvtName()) == true )
        this->m_pcDelegator->detonate(*pcEvt.data());
    return true;
}

bool GitlModualDelegate::xIsListenToEvt( const QString& strEvtName )
{
    for(int iEvtIdx = 0; iEvtIdx < m_cListeningEvts.size(); iEvtIdx++ )
    {
        if( strEvtName == m_cListeningEvts.at(iEvtIdx) )
        {
            return true;
        }
    }
    return false;
}

void GitlModualDelegate::dispatchEvt( GitlEvent& rcEvt )
{
    m_pcGitlEvtBus->post(rcEvt);
}
