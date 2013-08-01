#include "gitlmodual.h"
#include "gitleventbus.h"
#include <QDebug>

/*!
 * This is a test modual
 */
class TestModual : public GitlModual
{
public:

    /// re-implement this method to do anything you want...
    virtual bool detonate( GitlEvent& rcEvt)
    {
        qDebug() << "I got notified!";
        return true;
    }

};



int main(int argc, char *argv[])
{
    TestModual cModual;
    cModual.subscribeToEvtByName("I am a test event");  ///< subscribe to an event

    GitlEvent cEvent("I am a test event");              ///< create an event
    cEvent.dispatch();                                  ///< dispatch

    ///< output: "I got notified!"
}
