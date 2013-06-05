#include "gitlmodual.h"
#include "gitleventbus.h"


GitlModual::GitlModual() :
    m_cDelegate(this)
{    
}

bool GitlModual::detonate( GitlEvent cEvt )
{
    return true;
}

void GitlModual::subscribeToEvtByName( const QString& strEvtName )
{
    return m_cDelegate.subscribeToEvtByName(strEvtName);
}

void GitlModual::unsubscribeToEvtByName( const QString& strEvtName )
{
    return m_cDelegate.unsubscribeToEvtByName(strEvtName);
}

void GitlModual::dispatchEvt(const GitlEvent& rcEvt )
{
    m_cDelegate.dispatchEvt(&rcEvt);
}

void GitlModual::setModualName(const QString& strModualName )
{
    m_cDelegate.setModualName(strModualName);
}
