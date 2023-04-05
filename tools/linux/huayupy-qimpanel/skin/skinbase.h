#ifndef __SKIN_BASE_H__
#define __SKIN_BASE_H__

#include <QColor>
#include <QObject>

class SkinBase : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString inputBackImg READ inputBackImg WRITE setInputBackImg
        NOTIFY inputBackImgChanged)
    Q_PROPERTY(QString customImg0 READ customImg0 WRITE setCustomImg0
        NOTIFY customImg0Changed)
    Q_PROPERTY(QString customImg1 READ customImg1 WRITE setCustomImg1
        NOTIFY customImg1Changed)
    Q_PROPERTY(QString tipsImg READ tipsImg WRITE setTipsImg
        NOTIFY tipsImgChanged)
    Q_PROPERTY(int marginLeft READ marginLeft WRITE setMarginLeft
        NOTIFY marginLeftChanged)
    Q_PROPERTY(int marginRight READ marginRight WRITE setMarginRight
        NOTIFY marginRightChanged)
    Q_PROPERTY(int marginTop READ marginTop WRITE setMarginTop
        NOTIFY marginTopChanged)
    Q_PROPERTY(int marginBottom READ marginBottom WRITE setMarginBottom
        NOTIFY marginBottomChanged)
    Q_PROPERTY(QString backArrowImg READ backArrowImg WRITE setBackArrowImg
        NOTIFY backArrowImgChanged)
    Q_PROPERTY(QString forwardArrowImg READ forwardArrowImg WRITE setForwardArrowImg
        NOTIFY forwardArrowImgChanged)

    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize
        NOTIFY fontSizeChanged)
    Q_PROPERTY(int candFontSize READ candFontSize WRITE setCandFontSize
        NOTIFY candFontSizeChanged)
    Q_PROPERTY(QColor inputColor READ inputColor WRITE setInputColor
        NOTIFY inputColorChanged)
    Q_PROPERTY(QColor indexColor READ indexColor WRITE setIndexColor
        NOTIFY indexColorChanged)
    Q_PROPERTY(QColor firstCandColor READ firstCandColor WRITE setFirstCandColor
        NOTIFY firstCandColorChanged)
    Q_PROPERTY(QColor otherColor READ otherColor WRITE setOtherColor
        NOTIFY otherColorChanged)

    Q_PROPERTY(QString mainBackImg READ mainBackImg WRITE setMainBackImg
        NOTIFY mainBackImgChanged)
    Q_PROPERTY(QString mainLogoImg READ mainLogoImg WRITE setMainLogoImg
        NOTIFY mainLogoImgChanged)
    Q_PROPERTY(QString mainEngImg READ mainEngImg WRITE setMainEngImg
        NOTIFY mainEngImgChanged)
    Q_PROPERTY(QString mainCnImg READ mainCnImg WRITE setMainCnImg
        NOTIFY mainCnImgChanged)
    Q_PROPERTY(QString mainWbImg READ mainWbImg WRITE setMainWbImg
        NOTIFY mainWbImgChanged)
    Q_PROPERTY(QString mainSimplifiedImg READ mainSimplifiedImg WRITE setMainSimplifiedImg
        NOTIFY mainSimplifiedImgChanged)
    Q_PROPERTY(QString mainTraditionalImg READ mainTraditionalImg WRITE setMainTraditionalImg
        NOTIFY mainTraditionalImgChanged)
    Q_PROPERTY(QString mainFullWidthImg READ mainFullWidthImg WRITE setMainFullWidthImg
        NOTIFY mainFullWidthImgChanged)
    Q_PROPERTY(QString mainHalfWidthImg READ mainHalfWidthImg WRITE setMainHalfWidthImg
        NOTIFY mainHalfWidthImgChanged)
    Q_PROPERTY(QString mainEngMarkImg READ mainEngMarkImg WRITE setMainEngMarkImg
        NOTIFY mainEngMarkImgChanged)
    Q_PROPERTY(QString mainCnMarkImg READ mainCnMarkImg WRITE setMainCnMarkImg
        NOTIFY mainCnMarkImgChanged)
    Q_PROPERTY(QString mainSoftKbdImg READ mainSoftKbdImg WRITE setMainSoftKbdImg
        NOTIFY mainSoftKbdImgChanged)
    Q_PROPERTY(QString mainSettingImg READ mainSettingImg WRITE setMainSettingImg
        NOTIFY mainSettingImgChanged)
    Q_PROPERTY(QString mainSymbolsImg READ mainSymbolsImg WRITE setMainSymbolsImg
        NOTIFY mainSymbolsImgChanged)
    Q_PROPERTY(QString mainPinImg READ mainPinImg WRITE setMainPinImg
        NOTIFY mainPinImgChanged)

public:
    void setMainBackImg(const QString mainBackImg);
    QString mainBackImg() const;
    void setMainLogoImg(const QString mainLogoImg);
    QString mainLogoImg() const;
    void setMainEngImg(const QString mainEngImg);
    QString mainEngImg() const;
    void setMainCnImg(const QString mainCnImg);
    QString mainCnImg() const;

    void setMainWbImg(const QString mainWbImg);
    QString mainWbImg() const;

    void setMainSimplifiedImg(const QString mainSimplifiedImg);
    QString mainSimplifiedImg() const;
    void setMainTraditionalImg(const QString mainTraditionalImg);
    QString mainTraditionalImg() const;
    void setMainFullWidthImg(const QString mainFullWidthImg);
    QString mainFullWidthImg() const;
    void setMainHalfWidthImg(const QString mainHalfWidthImg);
    QString mainHalfWidthImg() const;
    void setMainEngMarkImg(const QString mainEngMarkImg);
    QString mainEngMarkImg() const;
    void setMainCnMarkImg(const QString mainCnMarkImg);
    QString mainCnMarkImg() const;
    void setMainSoftKbdImg(const QString mainSoftKbdImg);
    QString mainSoftKbdImg() const;
    void setMainSettingImg(const QString mainSettingImg);
    QString mainSettingImg() const;
    void setMainSymbolsImg(const QString mainSymbolsImg);
    QString mainSymbolsImg() const;
    void setMainPinImg(const QString mainPinImg);
    QString mainPinImg() const;

public:
    void setInputBackImg(const QString inputBackImg);
    QString inputBackImg() const;
    void setCustomImg0(const QString customImg);
    QString customImg0() const;
    void setCustomImg1(const QString customImg);
    QString customImg1() const;
    void setTipsImg(const QString tipsImg);
    QString tipsImg() const;
    void setMarginLeft(const int marginLeft);
    int marginLeft() const;
    void setMarginRight(const int marginRight);
    int marginRight() const;
    void setMarginTop(const int marginTop);
    int marginTop() const;
    void setMarginBottom(const int marginBottom);
    int marginBottom() const;
    void setBackArrowImg(const QString backArrowImg);
    QString backArrowImg() const;
    void setForwardArrowImg(const QString forwardArrowImg);
    QString forwardArrowImg() const;
    void setFontSize(const int fontSize);
    int fontSize() const;
    void setCandFontSize(const int candFontSize);
    int candFontSize() const;
    void setInputColor(const QColor inputColor);
    QColor inputColor() const;
    void setIndexColor(const QColor indexColor);
    QColor indexColor() const;
    void setFirstCandColor(const QColor firstCandColor);
    QColor firstCandColor() const;
    void setOtherColor(const QColor otherColor);
    QColor otherColor() const;

signals:
    void inputBackImgChanged();
    void customImg0Changed();
    void customImg1Changed();
    void tipsImgChanged();
    void marginLeftChanged();
    void marginRightChanged();
    void marginTopChanged();
    void marginBottomChanged();
    void backArrowImgChanged();
    void forwardArrowImgChanged();
    void fontSizeChanged();
    void candFontSizeChanged();
    void inputColorChanged();
    void indexColorChanged();
    void firstCandColorChanged();
    void otherColorChanged();
    void mainBackImgChanged();
    void mainLogoImgChanged();
    void mainEngImgChanged();
    void mainCnImgChanged();
    void mainWbImgChanged();
    void mainSimplifiedImgChanged();
    void mainTraditionalImgChanged();
    void mainFullWidthImgChanged();
    void mainHalfWidthImgChanged();
    void mainEngMarkImgChanged();
    void mainCnMarkImgChanged();
    void mainSoftKbdImgChanged();
    void mainSettingImgChanged();
    void mainSymbolsImgChanged();
    void mainPinImgChanged();

private:
    //Fcitx SkinInputBar
    QString mInputBackImg;
    QString mCustomImg0;
    QString mCustomImg1;
    QString mTipsImg;
    int mMarginLeft;
    int mMarginRight;
    int mMarginTop;
    int mMarginBottom;
    QString mBackArrowImg;
    QString mForwardArrowImg;

    //Fcitx SkinFont
    int mFontSize;
    int mCandFontSize;
    QColor mInputColor;
    QColor mIndexColor;
    QColor mFirstCandColor;
    QColor mOtherColor;

    //Fcitx SkinMainBar
    QString mMainBackImg;
    QString mMainLogo;
    QString mMainEngImg;
    QString mMainCnImg;
    QString mMainWbImg;
    QString mMainSimplifiedImg;
    QString mMainTraditionalImg;
    QString mMainFullWidthImg;
    QString mMainHalfWidthImg;
    QString mMainEngMarkImg;
    QString mMainCnMarkImg;
    QString mMainSoftKbdImg;
    QString mMainSettingImg;
    QString mMainSymbolsImg;
    QString mMainPinImg;

public:
    Q_PROPERTY(int inputStringPosX READ inputStringPosX WRITE setInputStringPosX
            NOTIFY inputStringPosXChanged)
    void setInputStringPosX(const int inputStringPosX);
    int inputStringPosX() const;
signals:
    void inputStringPosXChanged();
private:
    int mInputStringPosX;

public:
    Q_PROPERTY(int inputStringPosY READ inputStringPosY WRITE setInputStringPosY
            NOTIFY inputStringPosYChanged)
    void setInputStringPosY(const int inputStringPosY);
    int inputStringPosY() const;
signals:
    void inputStringPosYChanged();
private:
    int mInputStringPosY;

public:
    Q_PROPERTY(int outputCandPosX READ outputCandPosX WRITE setOutputCandPosX
            NOTIFY outputCandPosXChanged)
    void setOutputCandPosX(const int outputCandPosX);
    int outputCandPosX() const;
signals:
    void outputCandPosXChanged();
private:
    int mOutputCandPosX;

public:
    Q_PROPERTY(int outputCandPosY READ outputCandPosY WRITE setOutputCandPosY
            NOTIFY outputCandPosYChanged)
    void setOutputCandPosY(const int outputCandPosY);
    int outputCandPosY() const;
signals:
    void outputCandPosYChanged();
private:
    int mOutputCandPosY;

public:
    Q_PROPERTY(int backArrowPosX READ backArrowPosX WRITE setBackArrowPosX
            NOTIFY backArrowPosXChanged)
    void setBackArrowPosX(const int backArrowPosX);
    int backArrowPosX() const;
signals:
    void backArrowPosXChanged();
private:
    int mBackArrowPosX;

public:
    Q_PROPERTY(int backArrowPosY READ backArrowPosY WRITE setBackArrowPosY
            NOTIFY backArrowPosYChanged)
    void setBackArrowPosY(const int backArrowPosY);
    int backArrowPosY() const;
signals:
    void backArrowPosYChanged();
private:
    int mBackArrowPosY;

public:
    Q_PROPERTY(int forwardArrowPosX READ forwardArrowPosX WRITE setForwardArrowPosX
            NOTIFY forwardArrowPosXChanged)
    void setForwardArrowPosX(const int forwardArrowPosX);
    int forwardArrowPosX() const;
signals:
    void forwardArrowPosXChanged();
private:
    int mForwardArrowPosX;

public:
    Q_PROPERTY(int forwardArrowPosY READ forwardArrowPosY WRITE setForwardArrowPosY
            NOTIFY forwardArrowPosYChanged)
    void setForwardArrowPosY(const int forwardArrowPosY);
    int forwardArrowPosY() const;
signals:
    void forwardArrowPosYChanged();
private:
    int mForwardArrowPosY;

public:
    Q_PROPERTY(int adjustWidth READ adjustWidth WRITE setAdjustWidth
            NOTIFY adjustWidthChanged)
    void setAdjustWidth(const int adjustWidth);
    int adjustWidth() const;
signals:
    void adjustWidthChanged();
private:
    int mAdjustWidth;

public:
    Q_PROPERTY(int adjustHeight READ adjustHeight WRITE setAdjustHeight
            NOTIFY adjustHeightChanged)
    void setAdjustHeight(const int adjustHeight);
    int adjustHeight() const;
signals:
    void adjustHeightChanged();
private:
    int mAdjustHeight;

public:
    Q_PROPERTY(QString horizontalTileMode READ horizontalTileMode WRITE setHorizontalTileMode
            NOTIFY horizontalTileModeChanged)
    void setHorizontalTileMode(const QString horizontalTileMode);
    QString horizontalTileMode() const;
signals:
    void horizontalTileModeChanged();
private:
    QString mHorizontalTileMode;

public:
    Q_PROPERTY(QString verticalTileMode READ verticalTileMode WRITE setVerticalTileMode
            NOTIFY verticalTileModeChanged)
    void setVerticalTileMode(const QString verticalTileMode);
    QString verticalTileMode() const;
signals:
    void verticalTileModeChanged();
private:
    QString mVerticalTileMode;

//===================================================================================================
private:
    //Fcitx SkinInputBar
    QString mInputBackImgVertical;
    QString mCustomImgVertical0;
    QString mCustomImgVertical1;
    QString mTipsImgVertical;
    int mMarginLeftVertical;
    int mMarginRightVertical;
    int mMarginTopVertical;
    int mMarginBottomVertical;
    QString mBackArrowImgVertical;
    QString mForwardArrowImgVertical;

    //Fcitx SkinFont
    int mFontSizeVertical;
    int mCandFontSizeVertical;
    QColor mInputColorVertical;
    QColor mIndexColorVertical;
    QColor mFirstCandColorVertical;
    QColor mOtherColorVertical;

    int mInputStringPosXVertical;
    int mInputStringPosYVertical;
    int mOutputCandPosXVertical;
    int mOutputCandPosYVertical;
    int mBackArrowPosXVertical;
    int mBackArrowPosYVertical;
    int mForwardArrowPosXVertical;
    int mForwardArrowPosYVertical;

    int mAdjustWidthVertical;
    int mAdjustHeightVertical;
    QString mHorizontalTileModeVertical;
    QString mVerticalTileModeVertical;

public:
#define DEFINE_SET_PROPERTY(read, type, property) \
    void set##property(const type read);

    DEFINE_SET_PROPERTY(inputBackImgVertical, QString, InputBackImgVertical)
    DEFINE_SET_PROPERTY(tipsImgVertical, QString, TipsImgVertical)
    DEFINE_SET_PROPERTY(marginLeftVertical, int, MarginLeftVertical)

    DEFINE_SET_PROPERTY(marginRightVertical, int, MarginRightVertical)
    DEFINE_SET_PROPERTY(marginTopVertical, int, MarginTopVertical)
    DEFINE_SET_PROPERTY(marginBottomVertical, int, MarginBottomVertical)
    DEFINE_SET_PROPERTY(backArrowImgVertical, QString, BackArrowImgVertical)
    DEFINE_SET_PROPERTY(forwardArrowImgVertical, QString, ForwardArrowImgVertical)
    DEFINE_SET_PROPERTY(fontSizeVertical, int, FontSizeVertical)
    DEFINE_SET_PROPERTY(candFontSizeVertical, int, CandFontSizeVertical)
    DEFINE_SET_PROPERTY(inputColorVertical, QColor, InputColorVertical)
    DEFINE_SET_PROPERTY(indexColorVertical, QColor, IndexColorVertical)
    DEFINE_SET_PROPERTY(firstCandColorVertical, QColor, FirstCandColorVertical)
    DEFINE_SET_PROPERTY(otherColorVertical, QColor, OtherColorVertical)


    DEFINE_SET_PROPERTY(inputStringPosXVertical, int, InputStringPosXVertical)
    DEFINE_SET_PROPERTY(inputStringPosYVertical, int, InputStringPosYVertical)
    DEFINE_SET_PROPERTY(outputCandPosXVertical, int, OutputCandPosXVertical)
    DEFINE_SET_PROPERTY(outputCandPosYVertical, int, OutputCandPosYVertical)
    DEFINE_SET_PROPERTY(backArrowPosXVertical, int, BackArrowPosXVertical)
    DEFINE_SET_PROPERTY(backArrowPosYVertical, int, BackArrowPosYVertical)
    DEFINE_SET_PROPERTY(forwardArrowPosXVertical, int, ForwardArrowPosXVertical)
    DEFINE_SET_PROPERTY(forwardArrowPosYVertical, int, ForwardArrowPosYVertical)

    DEFINE_SET_PROPERTY(adjustWidthVertical, int, AdjustWidthVertical)
    DEFINE_SET_PROPERTY(adjustHeightVertical, int, AdjustHeightVertical)
    DEFINE_SET_PROPERTY(horizontalTileModeVertical, QString, HorizontalTileModeVertical)
    DEFINE_SET_PROPERTY(verticalTileModeVertical, QString, VerticalTileModeVertical)
#undef DEFINE_SET_PROPERTY
//===================================================================================================

public:
    SkinBase();
    virtual ~SkinBase();
    virtual bool loadSkin(const QString skinPath);
    void reloadSkin();
    void init();

protected:
    QColor value2color(const QString& value);
};

#endif //__SKIN_BASE_H__
