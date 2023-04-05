#ifndef __CANDIDATE_WORD_H__
#define __CANDIDATE_WORD_H__

#include <QObject>

class CandidateWord : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString cddLabel READ cddLabel WRITE setCddLabel
        NOTIFY cddLabelChanged)
    Q_PROPERTY(QString cddText READ cddText WRITE setCddText
        NOTIFY cddTextChanged)

public:
    void setCddLabel(const QString label);
    QString cddLabel() const;
    void setCddText(const QString text);
    QString cddText() const;

signals:
    void cddLabelChanged();
    void cddTextChanged();

private:
    QString mCddLabel;  //存放标号字符串，比如“1.”，“2.”，“3.”，“4.”，“5.”
    QString mCddText;   //存放候选词字符串，比如“啊”，“阿”，“吖”，“呵”，“腌”
};


#endif // __CANDIDATE_WORD_H__
