#ifndef TOOLBARMODEL_H
#define TOOLBARMODEL_H

#include <QObject>

class ToolBarModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int left READ left WRITE setLeft NOTIFY leftChanged)
    Q_PROPERTY(int top READ top WRITE setTop
        NOTIFY topChanged)
    Q_PROPERTY(bool cn READ cn WRITE setCn
        NOTIFY cnChanged)
    Q_PROPERTY(bool wb READ wb WRITE setWb
        NOTIFY wbChanged)
    Q_PROPERTY(bool trad READ trad WRITE setTrad
        NOTIFY tradChanged)
    Q_PROPERTY(bool fullwidth READ fullwidth WRITE setFullWidth
        NOTIFY fullwidthChanged)
    Q_PROPERTY(bool cnMark READ cnMark WRITE setCnMark
        NOTIFY cnMarkChanged)
    Q_PROPERTY(bool softKbd READ softKbd WRITE setSoftKbd
        NOTIFY softKbdChanged)
    Q_PROPERTY(int bottomLimit READ bottomLimit)
    Q_PROPERTY(int rightLimit READ rightLimit)

    Q_PROPERTY(int toolbarConfig READ toolbarConfig WRITE setToolbarConfig
        NOTIFY toolbarChanged)

public:
    explicit ToolBarModel(QObject *parent = 0);
    void Init();
    Q_INVOKABLE void setLeft(int left);
    int left();
    void setTop(int top);
    int top();
    void setCn(bool isCn);
    bool cn();
    void setWb(bool isWb);
    bool wb();
    void setTrad(bool isTrad);
    bool trad();
    void setFullWidth(bool isFullWidth);
    bool fullwidth();
    void setCnMark(bool isCnMark);
    bool cnMark();
    void setSoftKbd(bool show);
    bool softKbd();
    int bottomLimit();
    int rightLimit();
    void toresetWindow();
    void setToolbarConfig(int toolConfig);
    int toolbarConfig();

    void loadConfig();

    int using_english_punctuation_in_chinese_mode();
    void set_using_english_punctuation_in_chinese_mode(int using_english_punctuation_in_chinese_mode);

signals:
    void leftChanged();
    void topChanged();
    void cnChanged();
    void tradChanged();
    void fullwidthChanged();
    void cnMarkChanged();
    void softKbdChanged();
    void wbChanged();
    void resetWindow();
    void toolbarChanged();

private:
    int mLeft;
    int mTop;
    bool mIsCn;
    bool mIsTrad;
    bool mIsFullWidth;
    bool mIsCnMark;
    bool mIsSoftKbdShow;
    int mBottomLimit;
    int mRightLimit;
    bool mIsWb;
    int mToolbarConfig;
    int m_using_english_punctuation_in_chinese_mode;
};

#endif // TOOLBARMODEL_H
