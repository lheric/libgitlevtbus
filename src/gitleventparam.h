#ifndef GITLEVENTPARAM_H
#define GITLEVENTPARAM_H
#include <QString>
#include <QMap>
#include <QVariant>
#include "gitldef.h"

class GitlEventParam
{
public:
    GitlEventParam();

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


    ADD_CLASS_FIELD_PRIVATE( CONCATE(QMap<QString,QVariant>), cParameters)    ///< parameters name-value pair

};

#endif // GITLEVENTPARAM_H
