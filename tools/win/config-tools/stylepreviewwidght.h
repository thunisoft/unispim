#ifndef STYLEPREVIEWWIDGHT_H
#define STYLEPREVIEWWIDGHT_H

#include <QWidget>
#include <QObject>
#include <QFont>
#include <QImage>
#include <QMap>
#include <QPaintEvent>
#include <QtXml/QDomElement>
#include <QFontMetrics>


enum CANDIDATE_LAYOUT{
    Horizontal_SHOW = 0,
    Vertical_SHOW = 1
};

class StylePreviewModel;
class StylePreviewWidght : public QWidget
{
    Q_OBJECT
public:
    explicit StylePreviewWidght(QWidget *parent = nullptr);

    ~StylePreviewWidght();
private:
    void InitWidght();

public:
    void LoadConfig();


public:
    void UpdateTheWidght();

    //int/QColor互相转换
    QColor TranshFromIntToColor(int inputColorValue);
    int TransFormColorToInt(QColor inputColor);

    void DrawSvgPicTotheRect(QPainter*painter,const QString& filePath, const QRect& rect, const QColor&color);
    QByteArray GetSvgPicByteArray(const QString& filePath, const QColor& color);
    QColor TransAlpaColor(const QColor& compose_string_color);

    void DrawTabSvgPicTotheRect(QPainter*painter,const QString& filePath, const QRect& rect, const QColor&color);
private:
    void SetSVGBackColor(QDomElement elem, QString strtagname, QString strattr, QString strattrval);

private:
    void DrawBackGround(QPainter *p);
    void DrawCompositionFrame(QPainter *p);
    void DrawCandidateFrame(QPainter *p);
    void DrawToolBar(QPainter *p);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    StylePreviewModel* m_previewModel = nullptr;
    QRect m_mainRect;
    QRect m_visionRect;
    QRect m_workRect;
    QRect m_composeRect;
    QRect m_carect;
    QRect m_midline;
    QRect m_candidateRect[10];
    int candidateWith[10] = {0};
    QRect m_candidateFrameRect;
    int number_fix_height;
    int compose_fix_height;
    QRect m_caret;

    QMap<QString,int> m_colorMap;
    QMap<QString,int> m_locateMap;
    int m_candidatesNum = 5;
    QVector<QString> m_candiateVector;

    QFont m_chineseFont;
    QFont m_englishFont;
    QFont m_numberFont;
    int m_fontSize;

    QFontMetrics* m_chinesefm = nullptr;
    QFontMetrics* m_englishfm = nullptr;
    QFontMetrics* m_numberfm = nullptr;
    int ciGap;


};



class StylePreviewModel : public QObject
{
    Q_OBJECT
public:
    StylePreviewModel();
    ~StylePreviewModel();

    void SetColorProperty(const QString& key, int value);

    inline void SetChineseFont(const QFont& chineseFont)
    {
        m_chinese_font = chineseFont;
    }

    inline void SetEnglishFont(const QFont& englishFont)
    {
        m_english_font = englishFont;
    }

    inline void SetFontSize(const int fontSize)
    {
        m_font_size = fontSize;
    }

    void SetToolBarConfig(const int value)
    {
        m_tool_bar_config = value;
    }

    void SetKeepExpand(const int expandFlag)
    {
        m_is_keep_expanding = expandFlag;
    }

    inline QString ChineseInputString()
    {
        return QString("华宇拼音");
    }
    inline QString EnglishInputString()
    {
        return QString("shu'ru'fa");
    }

    inline QVector<QString> GetCandidatesContentList()
    {
        return m_candidates_content_list;
    }

    inline void SetCandidatesSize(const int candidateSize)
    {
        m_candidates_size = candidateSize;
    }
    inline int GetCandiateSize()
    {
        return m_candidates_size;
    }

    inline CANDIDATE_LAYOUT GetCandidateLayout()
    {
        return m_candiate_layout;
    }
    void SetCandidateLayout(const CANDIDATE_LAYOUT inputLayout)
    {
        m_candiate_layout = inputLayout;
    }

    inline QFont ChineseFont()
    {
        return m_chinese_font;
    }

    inline QFont EnglishFont()
    {
        return m_english_font;
    }
    inline int FontSize()
    {
        return m_font_size;
    }

    QMap<QString, QImage> image_property_map() const;
    void setImage_property_map(const QMap<QString, QImage> &image_property_map);

    QMap<QString, int> locate_property_map() const;
    void setLocate_property_map(const QMap<QString, int> &locate_property_map);

    QMap<QString, int> color_config_map() const;
    void setColor_config_map(const QMap<QString, int> &color_config_map);

private:

    QMap<QString,int> m_color_config_map;//颜色属性组

    QFont m_chinese_font;         //中文字体
    QFont m_english_font;         //英文字体
    int m_font_size = 16;         //字体大小

    QMap<QString, QImage> m_image_property_map;//图片属性
    QMap<QString, int> m_locate_property_map;//定位属性


    CANDIDATE_LAYOUT m_candiate_layout;//候选词的布局
    int m_candidates_size;//候选词的个数
    QVector<QString> m_candidates_content_list;//候选词内容的列表
    QString m_chinese_input_String;//输入的中文音节串
    QString m_english_input_string;//输入的英文音节串

    int m_tool_bar_config;      //工具栏的配置
    bool m_is_keep_expanding;   //是否保持扩展

};



#endif // STYLEPREVIEWWIDGHT_H
