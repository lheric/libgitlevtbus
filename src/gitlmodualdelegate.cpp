#include "gitlmodualdelegate.h"
#include "gitleventbus.h"
#include <QDebug>
#include <iostream>
#include <QReadLocker>
#include <QWriteLocker>

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
    QWriteLocker clocker(&m_cModualMutex);
    m_cListeningEvts.push_back(strEvtName);
    return;
}


void GitlModualDelegate::unsubscribeToEvtByName( const QString& strEvtName )
{
    QWriteLocker clocker(&m_cModualMutex);
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

bool GitlModualDelegate::detonate(GitlEvent cEvt )
{

    if( xIsListenToEvt(cEvt.getEvtName()) == true )
        this->m_pcDelegator->detonate(cEvt);
    return true;
}

bool GitlModualDelegate::xIsListenToEvt( const QString& strEvtName )
{
    QReadLocker clocker(&m_cModualMutex);

    for(int iEvtIdx = 0; iEvtIdx < m_cListeningEvts.size(); iEvtIdx++ )
    {
        if( strEvtName == m_cListeningEvts.at(iEvtIdx) )
        {
            return true;
        }
    }
    return false;
}

void GitlModualDelegate::dispatchEvt(const GitlEvent *pcEvt )
{
    m_pcGitlEvtBus->post(pcEvt);
}
