#include "utils.h"

#include <QDateTime>

Utils::Utils()
{

}
QString Utils::getRandomNumber()
{
	QDateTime time = QDateTime::currentDateTime();
	int timeT = time.toTime_t();

	qsrand(10000);
	QString randomNum = QString::number(qrand());
	QString result = QString::number(timeT) + randomNum;

	return result;
}

bool Utils::getMatchStr(const QString &srcStr, const QString &pattern, QString &result)
{
	QRegExp rx(pattern);
	int pos = rx.indexIn(srcStr);

	if (pos > -1) {
		result = rx.cap(0);
		return true;
	}

	return false;
}

bool Utils::getValueByKey(const QString &srcStr, const QString &key, QString &value)
{
    std::string src = srcStr.toStdString();
    std::string match = key.toStdString();
    int strlength = src.length();

    if (src.empty() || match.empty())
        return false;

    std::string newMatch = match + "=";

    int pos = src.find(newMatch);
    if (pos < 0)
        return false;

    int start = pos + newMatch.length();
    const char ch = '"';
    if (start < strlength && ch == src[start]) {
        start += 1;
    }

    int end = src.find(';', pos);
    if (end < 0 && end >= strlength && end <= start)
        return false;

    if (src[end - 1] == ch) {
        end -= 1;
    }

    int length = end - start;

    value = QString::fromStdString(src.substr(start, length));
    return true;
}
