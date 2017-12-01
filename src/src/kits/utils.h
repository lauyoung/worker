#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QRegExp>
class Utils
{
public:
    Utils();

    static bool getValueByKey(const QString &srcStr, const QString &key, QString &value);
	static bool getMatchStr(const QString &srcStr, const QString &pattern, QString &result);
};

#endif // UTILS_H
