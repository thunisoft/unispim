#include "stylepreviewwidght.h"
#include "config.h"
#include <QPainter>
#include <QPen>
#include <QColor>
#include <QtSvg/QSvgRenderer>
#include <QtXml/QtXml>
#include <QFontMetrics>
#include "utils.h"

StylePreviewWidght::StylePreviewWidght(QWidget *parent) : QWidget(parent)
{
    InitWidght();
}

StylePreviewWidght::~StylePreviewWidght()
{
    if(m_previewModel != nullptr)
    {
        delete m_previewModel;
        m_previewModel = nullptr;
    }
}

void StylePreviewWidght::InitWidght()
{
    if(m_previewModel == nullptr)
    {
        m_previewModel = new StylePreviewModel();
    }
    LoadConfig();
}



void StylePreviewWidght::LoadConfig()
{
    //加载对应的颜色配置
    ConfigItemStruct configItem;
    QStringList colorItemKeyList;
    colorItemKeyList   <<"compose_string_color"
                       <<"candidate_selected_color"
                       <<"candidate_string_color"
                       <<"main_background_color"
                       <<"main_line_color"
                       <<"compose_caret_color"
                       <<"line_background_color"
                       <<"cell_background_color";
    QMap<QString,int> colorMap;
    for(QString currentKey : colorItemKeyList)
    {
        if(Config::Instance()->GetConfigItemByJson(currentKey,configItem))
        {
            colorMap.insert(currentKey,configItem.itemCurrentIntValue);
        }
    }
    m_previewModel->setColor_config_map(colorMap);


    if(Config::Instance()->GetConfigItemByJson("font_height",configItem))
    {
        m_previewModel->SetFontSize(configItem.itemCurrentIntValue);
    }

    if(Config::Instance()->GetConfigItemByJson("chinese_font_name",configItem))
    {
        m_previewModel->SetChineseFont(QFont(configItem.itemCurrentStrValue));
    }
    if(Config::Instance()->GetConfigItemByJson("english_font_name",configItem))
    {
        m_previewModel->SetEnglishFont(QFont(configItem.itemCurrentStrValue));
    }

    if(Config::Instance()->GetConfigItemByJson("show_vertical_candidate",configItem))
    {
        m_previewModel->SetCandidateLayout((CANDIDATE_LAYOUT)configItem.itemCurrentIntValue);
    }

    if(Config::Instance()->GetConfigItemByJson("candidates_per_line",configItem))
    {
        m_previewModel->SetCandidatesSize(configItem.itemCurrentIntValue);
    }

    if(Config::Instance()->GetConfigItemByJson("tool_bar_config",configItem))
    {
        m_previewModel->SetToolBarConfig(configItem.itemCurrentIntValue);
    }
    if(Config::Instance()->GetConfigItemByJson("always_expand_candidates",configItem))
    {
        m_previewModel->SetKeepExpand(configItem.itemCurrentIntValue);
    }

    m_colorMap = m_previewModel->color_config_map();
    m_locateMap = m_previewModel->locate_property_map();
    m_candiateVector = m_previewModel->GetCandidatesContentList();
    m_candidatesNum = m_previewModel->GetCandiateSize();

    m_englishFont = m_previewModel->EnglishFont();
    m_chineseFont = m_previewModel->ChineseFont();
    m_numberFont  = QFont("Courier New");
    m_fontSize = m_previewModel->FontSize();
    m_englishFont.setPixelSize(m_fontSize);
    m_chineseFont.setPixelSize(m_fontSize);

    int numberFontSize = m_fontSize*7/8;
    numberFontSize = (numberFontSize <=0 )?1:numberFontSize;
    m_numberFont.setPixelSize(numberFontSize);
    m_numberFont.setLetterSpacing(QFont::PercentageSpacing,90);

    //析构对应的内容
    if(m_chinesefm != nullptr)
    {
        delete m_chinesefm;
        m_chinesefm = nullptr;
    }
    if(m_englishfm != nullptr)
    {
        delete m_englishfm;
        m_englishfm = nullptr;
    }
    if(m_numberfm != nullptr)
    {
        delete m_numberfm;
        m_numberfm = nullptr;
    }


    //中英文字符串的长度
    m_chinesefm = new QFontMetrics(m_chineseFont);
    m_englishfm = new QFontMetrics(m_englishFont);
    m_numberfm = new QFontMetrics(m_numberFont);
    int scale_font_height = m_fontSize;
    ciGap = scale_font_height / 2;

    if (ciGap < 14)
        ciGap = 14;

}

void StylePreviewWidght::UpdateTheWidght()
{
    LoadConfig();
    this->update();
}

void StylePreviewWidght::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter paint(this);
    //绘制背景
    DrawBackGround(&paint);
    //绘制组合编辑框
    DrawCompositionFrame(&paint);
    //绘制候选框
    DrawCandidateFrame(&paint);
    //绘制工具栏
    DrawToolBar(&paint);
}

QColor StylePreviewWidght::TranshFromIntToColor(int inputColorValue)
{
    int red = inputColorValue & 255;
    int green = inputColorValue >> 8 & 255;
    int blue = inputColorValue >> 16 & 255;
    return QColor(red,green,blue);
}

int StylePreviewWidght::TransFormColorToInt(QColor inputColor)
{
    int redValue = inputColor.red();
    int greenValue = inputColor.green();
    int blueValue = inputColor.blue();

    return (int)((blueValue << 16)|(greenValue << 8)|(redValue));
}

void StylePreviewWidght::DrawSvgPicTotheRect(QPainter*painter,const QString &filePath, const QRect &rect, const QColor &color)
{
    QSvgRenderer m_svgRender;
    QByteArray imageContent = GetSvgPicByteArray(filePath,color);
    m_svgRender.load(imageContent);
    m_svgRender.render(painter,rect);
}

QByteArray StylePreviewWidght::GetSvgPicByteArray(const QString &filePath, const QColor &color)
{
    QString mRgbStr = color.name();
    QFile file(filePath);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    QDomDocument doc;
    doc.setContent(data);
    file.close();

    SetSVGBackColor(doc.documentElement(), "g", "fill", mRgbStr);
    QByteArray svg_content = doc.toByteArray();
    return svg_content;
}

QColor StylePreviewWidght::TransAlpaColor(const QColor &compose_string_color)
{
    int r = (unsigned int)compose_string_color.red();
    int g = (unsigned int)compose_string_color.green();
    int b = (unsigned int)compose_string_color.blue();

    r = 204 + (r / 5);
    g = 204 + (g / 5);
    b = 204 + (b / 5);

    r = r > 255 ? 255 : r;
    g = g > 255 ? 255 : g;
    b = b > 255 ? 255 : b;

    return QColor(r,g,b);
}

void StylePreviewWidght::DrawTabSvgPicTotheRect(QPainter *painter, const QString &filePath, const QRect &rect, const QColor &color)
{
    QSvgRenderer m_svgRender;
    QByteArray imageContent = GetSvgPicByteArray(filePath,color);
    QString targetStr = QString(imageContent).replace("fill:#1173BB;",QString("fill:%1;").arg(color.name()));
    m_svgRender.load(targetStr.toUtf8());
    m_svgRender.render(painter,rect);
}

void StylePreviewWidght::SetSVGBackColor(QDomElement elem, QString strtagname, QString strattr, QString strattrval)
{
    if (elem.tagName().compare(strtagname) == 0)
    {
        QString before_color = elem.attribute(strattr);
        if((!before_color.isEmpty()) && (before_color.startsWith("#")))
        {
            elem.setAttribute(strattr, strattrval.toUpper());
            QString color = elem.attribute(strattr);
        }

    }
    for (int i = 0; i < elem.childNodes().count(); i++)
    {
        if (!elem.childNodes().at(i).isElement())
        {
            continue;
        }
        SetSVGBackColor(elem.childNodes().at(i).toElement(), strtagname, strattr, strattrval);
    }
}



void StylePreviewWidght::DrawBackGround(QPainter *p)
{
    try {

        CANDIDATE_LAYOUT layout = m_previewModel->GetCandidateLayout();
        Q_UNUSED(layout);
        //int rows = (layout == Horizontal_SHOW)? 1:m_candidatesNum;
        int rows = 1;

        QRect chineseRect = m_chinesefm->boundingRect(m_previewModel->ChineseInputString());
        QRect englishRect = m_englishfm->boundingRect(m_previewModel->EnglishInputString());

        int chineseAscent = m_chinesefm->ascent();
        int numberAscent = m_numberfm->ascent();
        int englishAscent = m_englishfm->ascent();
        number_fix_height = chineseAscent - numberAscent;
        compose_fix_height = chineseAscent - englishAscent;

        int compose_width = chineseRect.width() + englishRect.width();
        Q_UNUSED(compose_width);
        int main_center_gap_height = m_locateMap.value("main_center_gap_height",14);
        int candidate_row_gap = (rows-1)*m_locateMap.value("CAND_GAP_HEIGHT",2);
        int candidates_row_height = rows*chineseRect.height();
        int composing_height = chineseRect.height();
        m_workRect.setHeight(composing_height + main_center_gap_height + candidate_row_gap + candidates_row_height);

        //Q_ASSERT(m_candidatesNum > 0 && m_candidatesNum <= 9);
        int candidateLineWidth = 0;
        //候选词之间的间距

        for(int index=0; index<m_candidatesNum; ++index)
        {
            QRect indexRect = m_numberfm->boundingRect(QString("%1.").arg(index+1));
            QRect currentCandidateRect = m_chinesefm->boundingRect(m_candiateVector.at(index));
            candidateLineWidth += indexRect.width();
            candidateLineWidth += 3;
            candidateLineWidth += currentCandidateRect.width();
            candidateLineWidth += ciGap;
        }

        candidateLineWidth += (m_fontSize/8);//加上左箭头的左间距

        auto CalcScaleWithFont = [&](int value)->int {
            double rate = (double)m_fontSize / 16;
            double dvalue = (double)value * rate;
            return (int)dvalue;
        };

        candidateLineWidth += CalcScaleWithFont(10); //计算左箭头的宽度
        candidateLineWidth += CalcScaleWithFont(7); //右箭头的Xmin
        candidateLineWidth += CalcScaleWithFont(10); //右箭头的Xmax
        candidateLineWidth += ciGap;    //tab键的Xmin
        candidateLineWidth += CalcScaleWithFont(16); //tab键的Xmax

        m_workRect.setWidth(candidateLineWidth);
        m_workRect.setX(17);
        m_workRect.setY(11);

        QColor borderColor = TranshFromIntToColor(m_colorMap.value("main_line_color"));
        //绘制边框
        QPen borderPen(borderColor);
        borderPen.setStyle(Qt::SolidLine);
        borderPen.setWidth(1);
        int radius = 8;
        p->setPen(borderPen);

        m_visionRect = m_workRect;
        m_visionRect.setX(5);
        m_visionRect.setY(5);

        int main_left_margin = m_locateMap.value("main_text_left_margin");
        int main_right_margin = m_locateMap.value("main_text_right_margin");
        int main_top_margin = m_locateMap.value("main_text_top_margin");
        int main_bottom_margin = m_locateMap.value("main_text_bottom_margin");

        m_visionRect.setHeight(m_workRect.height() + main_top_margin + main_bottom_margin);
        m_visionRect.setWidth(m_workRect.width() + main_left_margin + main_right_margin);

        QPainterPath path;
        path.addRoundedRect(m_visionRect,radius,radius);

        m_midline.setX(0);
        m_midline.setY(composing_height + main_top_margin);
        m_midline.setWidth(m_workRect.width());
        m_midline.setHeight(1);

        m_composeRect = m_workRect;
        m_composeRect.setHeight(composing_height);


        QColor backgroundColor = TranshFromIntToColor(m_colorMap.value("main_background_color"));
        p->fillPath(path,backgroundColor);
        p->drawPath(path);
        p->drawLine(m_workRect.x() + m_midline.x(), m_workRect.y() + m_midline.y(),
                    m_workRect.x()+ m_midline.x() + m_midline.width(),
                    m_workRect.y()+m_midline.y());

        this->setFixedWidth(m_visionRect.width() + 30);
        this->setFixedHeight(m_visionRect.height() + 30);

    } catch (...) {
        Utils::WriteLogToFile("style preview draw background throw error");
    }

}

void StylePreviewWidght::DrawCompositionFrame(QPainter *p)
{
    try {
        //获取对应的属性   
        CANDIDATE_LAYOUT layout = m_previewModel->GetCandidateLayout();
        int rows = (layout == Horizontal_SHOW)? 1:m_candidatesNum;
        Q_UNUSED(rows);
        QVector<QString> candiateVector = m_previewModel->GetCandidatesContentList();

        //中英文字符串的长度
        QRect chineseRect = m_chinesefm->boundingRect(m_previewModel->ChineseInputString());
        QRect leftEnglishCompose = m_composeRect;
        leftEnglishCompose.setX(m_composeRect.x() + chineseRect.width());
        leftEnglishCompose.setY(leftEnglishCompose.y() + compose_fix_height);

        QString chineseCompose = m_previewModel->ChineseInputString();
        QString englishCompose = m_previewModel->EnglishInputString();

        QColor composeColor = TranshFromIntToColor(m_colorMap.value("compose_string_color"));
        QPen borderPen(composeColor);
        p->setPen(borderPen);
        p->setFont(m_chineseFont);
        p->drawText(m_composeRect,chineseCompose);
        p->setFont(m_englishFont);
        p->drawText(leftEnglishCompose,englishCompose);
        //绘制光标
        int leftEnglishWidth = m_englishfm->boundingRect(englishCompose).width();
        m_caret.setX(leftEnglishCompose.x() + leftEnglishWidth + 1);
        m_caret.setY(m_composeRect.y());
        m_caret.setHeight((int)(m_composeRect.height() + 0.5) - (3 * m_fontSize / 16));


        QColor cursorColor = TranshFromIntToColor(m_colorMap.value("compose_caret_color"));
        QPen cursorPen(cursorColor);
        cursorPen.setWidth(1);
        p->setPen(cursorPen);
        p->drawLine(m_caret.x(),m_caret.y(),m_caret.x(),m_caret.y() + m_caret.height());



    } catch (...) {
        Utils::WriteLogToFile("style preview draw background throw error");
    }
}

void StylePreviewWidght::DrawCandidateFrame(QPainter *p)
{
    try {
        //获取对应的属性

        //CANDIDATE_LAYOUT layout = m_previewModel->GetCandidateLayout();
        //int rows = (layout == Horizontal_SHOW)? 1:m_candidatesNum;
        int rows = 1;
        Q_UNUSED(rows);


        //中英文字符串的长度
        QRect chineseRect = m_chinesefm->boundingRect(m_previewModel->ChineseInputString());

        int main_top_margin = m_locateMap.value("main_text_top_margin");
        int main_bottom_margin = m_locateMap.value("main_text_bottom_margin");

        m_candidateFrameRect = m_workRect;
        int main_center_gap_height = m_locateMap.value("main_center_gap_height",14);
        m_candidateFrameRect.setY(m_workRect.y() + main_center_gap_height + m_composeRect.height());
        m_candidateFrameRect.setHeight(chineseRect.height());

        int scale_font_height = m_fontSize;

        int candidateSize = m_candiateVector.size();
        Q_ASSERT(m_candidatesNum <= candidateSize);
        QRect candidateIndexRect;
        candidateIndexRect = m_candidateFrameRect;
        QPen candiatePen;
        for (int index =0; index<m_candidatesNum; ++index) {
            QString currenIndexStr = QString("%1.").arg(index+1);
            int indexWidth = m_numberfm->boundingRect(currenIndexStr).width();
            QString currentIndexCandidate = m_candiateVector.at(index);
            int strWidth = m_chinesefm->boundingRect(currentIndexCandidate).width();

            QRect currentRecordRect;
            currentRecordRect.setX(m_workRect.x());
            currentRecordRect.setY(m_workRect.y() + m_midline.y()+1);
            currentRecordRect.setWidth(indexWidth + strWidth+5);
            currentRecordRect.setHeight(m_workRect.height() + main_bottom_margin - m_midline.y()-1);
            m_candidateRect[index] = currentRecordRect;
            QColor cellColor = TranshFromIntToColor(m_colorMap.value("cell_background_color"));

            if(index == 0)
            {
               QBrush brush(cellColor);
               p->fillRect(m_candidateRect[0],brush);
               QColor selectedColor = TranshFromIntToColor(m_colorMap.value("candidate_selected_color"));
               candiatePen.setColor(selectedColor);
               p->setPen(candiatePen);
               p->setFont(m_numberFont);

               candidateIndexRect.setY(candidateIndexRect.y() + number_fix_height);
               p->drawText(candidateIndexRect,currenIndexStr);
               candidateIndexRect.setX(candidateIndexRect.x()+indexWidth+3);
               p->setFont(m_chineseFont);
               candidateIndexRect.setY(candidateIndexRect.y() - number_fix_height);
               p->drawText(candidateIndexRect,currentIndexCandidate);
               candidateIndexRect.setX(candidateIndexRect.x()+strWidth + ciGap);

            }
            else
            {
                QColor candidateColor = TranshFromIntToColor(m_colorMap.value("candidate_string_color"));
                candiatePen.setColor(candidateColor);
                p->setPen(candiatePen);
                p->setFont(m_numberFont);
                candidateIndexRect.setY(candidateIndexRect.y() + number_fix_height);
                p->drawText(candidateIndexRect,currenIndexStr);
                candidateIndexRect.setX(candidateIndexRect.x()+indexWidth+3);
                p->setFont(m_chineseFont);
                candidateIndexRect.setY(candidateIndexRect.y() - number_fix_height);
                p->drawText(candidateIndexRect,currentIndexCandidate);
                candidateIndexRect.setX(candidateIndexRect.x()+strWidth + ciGap);             
            }

        }
        auto CalcScaleWithFont = [&](int value)->int {
            double rate = (double)scale_font_height / 16;
            double dvalue = (double)value * rate;

            int returnValue = static_cast<int>(dvalue);
            if(returnValue <=0)
                returnValue = 1;
            return returnValue;
        };

        QRect icon_left = candidateIndexRect;
        icon_left.setX(candidateIndexRect.x() + (scale_font_height/8));
        icon_left.setY(candidateIndexRect.y() + ((m_chinesefm->height() - icon_left.height()) / 2) + number_fix_height);
        int Value = CalcScaleWithFont(10);
        Q_UNUSED(Value);
        icon_left.setWidth(CalcScaleWithFont(10));
        icon_left.setHeight(CalcScaleWithFont(10));
        QColor composeColor = TranshFromIntToColor(m_colorMap.value("compose_string_color"));
        QColor icon_low_color = TransAlpaColor(composeColor);
        DrawSvgPicTotheRect(p,":/image/preview/left.svg",icon_left,icon_low_color);


        QRect icon_right = icon_left;
        icon_right.setX(icon_left.x() + CalcScaleWithFont(10) + CalcScaleWithFont(7));
        icon_right.setWidth(CalcScaleWithFont(10));
        DrawSvgPicTotheRect(p,":/image/preview/right.svg",icon_right,composeColor);

        QRect icon_tab = icon_right;
        icon_tab.setX(icon_right.x() + CalcScaleWithFont(10) + ciGap);
        icon_tab.setY(candidateIndexRect.y() + ((m_chinesefm->height() - icon_left.height()) / 2) + number_fix_height);
        icon_tab.setWidth(CalcScaleWithFont(16));
        icon_tab.setHeight(CalcScaleWithFont(4));
        DrawTabSvgPicTotheRect(p,":/image/preview/tab.svg",icon_tab,icon_low_color);


        QRect icon_logo(0,0,0,0);
        int logoWidth = m_midline.y() - 4;
        icon_logo.setX(m_visionRect.width() - logoWidth);
        int top_half_height = m_midline.y() + (main_top_margin - 1);
        icon_logo.setY(((top_half_height - logoWidth)/2) + main_top_margin -1);
        icon_logo.setHeight(logoWidth);
        icon_logo.setWidth(logoWidth);
        DrawSvgPicTotheRect(p,":/image/preview/logo_cand.svg",icon_logo,icon_low_color);

    } catch (...) {
        Utils::WriteLogToFile("style preview draw background throw error");
    }
}


void StylePreviewWidght::DrawToolBar(QPainter *p)
{
    Q_UNUSED(p);
}

StylePreviewModel::StylePreviewModel()
{
    m_candidates_content_list = {"输入法","输入","数","书","属","树","术","舒","输"};
    m_locate_property_map = {{"main_text_left_margin",12},
                            {"main_text_right_margin",8},
                            {"main_text_top_margin",6},
                            {"main_text_bottom_margin",6},
                            {"main_center_gap_height",14},
                            {"main_remain_number_margin",16},
                             {"CAND_GAP_HEIGHT",2}};
}

StylePreviewModel::~StylePreviewModel()
{

}

QMap<QString, int> StylePreviewModel::locate_property_map() const
{
    return m_locate_property_map;
}

void StylePreviewModel::setLocate_property_map(const QMap<QString, int> &locate_property_map)
{
    m_locate_property_map = locate_property_map;
}

QMap<QString, int> StylePreviewModel::color_config_map() const
{
    return m_color_config_map;
}

void StylePreviewModel::setColor_config_map(const QMap<QString, int> &color_config_map)
{
    m_color_config_map = color_config_map;
}






