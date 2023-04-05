#include "candidatewindow.h"
#include <QGraphicsDropShadowEffect>
#include <QSvgRenderer>
#include <QPainter>
#include <QSize>
#include <QColor>
#include <QBitmap>
#include <QPalette>
#include <QPushButton>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QDebug>

#include "stdio.h"

#include "windowconfig.h"
#include "kimpanelagent.h"
#include "mainmodel.h"
#include "svgutils.h"
#include "toolbarmenu.h"
#include "../public/config.h"
#include "../public/configmanager.h"
#include "maincontroller.h"

#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>

#define Y_OFFSET 20

CandidateWindow::CandidateWindow(KimpanelAgent* kimpanel_agent)
{
    m_kimpanel_agent = kimpanel_agent;
    Init();    
}

CandidateWindow::~CandidateWindow()
{
    if(m_person_name_tip_window)
    {
        delete m_person_name_tip_window;
        m_person_name_tip_window = NULL;
    }

    if(m_font_candidate)
    {
        delete m_font_candidate;
        m_font_candidate = NULL;
    }
    if(m_font_input)
    {
        delete m_font_input;
        m_font_input = NULL;
    }
    if(m_font_index)
    {
        delete m_font_index;
        m_font_index = NULL;
    }
#ifdef USE_UOS
    if(m_uos_mode_monitor)
    {
        delete m_uos_mode_monitor;
        m_uos_mode_monitor = NULL;
    }
#endif
}

void CandidateWindow::ToShow()
{
    if(!this->isVisible())
        this->show();
    this->SoftMove(GetX(), GetY() + Y_OFFSET);    
}

void CandidateWindow::Init()
{
    setAttribute(Qt::WA_TranslucentBackground, true);//设置窗口背景透明
    m_has_cal_done = false;

    WC = WindowConfig::Instance();
    m_main_model = MainModel::self();

    m_shadow_corner_maker = new QGraphicsDropShadowEffect(this);
    m_font_size = WC->font_height;
    m_font_input = new QFont();
    m_font_input->setFamily(WC->english_font_name);
    //m_font_input->setPointSize(m_font_size);
    m_font_input->setPixelSize(m_font_size);

    m_font_candidate = new QFont();
    m_font_candidate->setFamily(WC->chinese_font_name);
    //m_font_candidate->setPointSize(m_font_size);
    m_font_candidate->setPixelSize(m_font_size);

    m_font_index = new QFont();
    m_font_index->setFamily(WC->english_font_name);

    m_delete_tip_window = new DeleteTipWidget();
    m_person_name_tip_window = new DeleteTipWidget();
    connect(m_delete_tip_window,&DeleteTipWidget::remove_user_ci,this,&CandidateWindow::slot_remove_ci_by_index);

    int index_font_size = m_font_size * 0.9;
    //m_font_index->setPointSize(index_font_size);
    m_font_index->setPixelSize(index_font_size);

    m_top_margin = DEFAULT_TOP_MARGIN;
    m_left_margin = DEFAULT_LEFT_MARGIN;
    m_right_margin = DEFAULT_RIGHT_MARGIN;
    m_bottom_margin = DEFAULT_BOTTOM_MARGIN;
    m_center_margin = DEFAULT_CENTER_MARGIN;
    m_gap_of_candiadtes = DEFAULT_GAP_OF_CANDIDATES;

    m_view_mode = VIEW_MODE_HORIZONTAL;
    m_selected_index = 0;

    m_has_pre = false;
    m_has_next = false;

    LoadImage();    
    ClearMouseState();

    m_element_should_draw = 0;
    m_cart_index = 0;

    m_rect_selected = {0, 0, 0, 0};

#ifdef USE_UOS
    m_uos_mode_monitor = new UosModeMonitor(this);
    m_is_spcial_mode = m_uos_mode_monitor->IsSpecialMode();
#else
    m_is_spcial_mode = true;
#endif

    CalPersonNameIconSize();

}

void CandidateWindow::Paint()
{

}

#define ADD_MARGIN(data, value) m_window_rect.value + data.value

#define MAKE_RECT(data) QRect(QPoint(m_window_rect.x + data.x, m_window_rect.y + data.y), QSize(data.width, data.height))
#define MAKE_RECTF(data) QRectF(m_window_rect.x + data.x, m_window_rect.y + data.y, data.width, data.height)
void CandidateWindow::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    if(!m_has_cal_done)
    {
        this->hide();
        qDebug() << "没有计算完候选栏!";
        return;
    }

    //填充背景 并画出圆角和边框
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    if(IsSpecialMode())
    {
        QRect corner ;
        corner.setLeft(1 + DEFAULT_RADIUS);
        corner.setTop(1 + DEFAULT_RADIUS);
        corner.setWidth(this->rect().width() - 2 * (1 + DEFAULT_RADIUS));
        corner.setHeight(this->rect().height() - 2 * (1 + DEFAULT_RADIUS));
        p.setPen(ConvertIntToQColor(WC->main_line_color));

        QPainterPath path;
        path.addRoundedRect(corner, DEFAULT_RADIUS, DEFAULT_RADIUS);
        p.drawPath(path);
        p.fillPath(path, QBrush(ConvertIntToQColor(WC->main_background_color)));
    }
    else
    {
        p.fillRect(QRect(0, 0, m_rect_whole_window.width, 1), QBrush(ConvertIntToQColor(WC->main_line_color))); //top
        p.fillRect(QRect(0, m_rect_whole_window.height - 1, m_rect_whole_window.width, 1), QBrush(ConvertIntToQColor(WC->main_line_color))); //bottom
        p.fillRect(QRect(0, 1, 1, m_rect_whole_window.height - 2), QBrush(ConvertIntToQColor(WC->main_line_color)));//left
        p.fillRect(QRect(m_rect_whole_window.width - 1, 1, 1, m_rect_whole_window.height - 2), QBrush(ConvertIntToQColor(WC->main_line_color)));//right
        p.fillRect(QRect(1, 1, m_rect_whole_window.width -2, m_rect_whole_window.height - 2), QBrush(ConvertIntToQColor(WC->main_background_color))); //background
    }

    if((m_rects_candidate.size() > 5)&&(SwitchShowModel() == VIEW_MODE_EXPAND))
    {
        int pos_y = m_rects_candidate.at(m_selected_index).y;
        m_expand_mode_current_row_rect.y = pos_y;
        p.fillRect(MAKE_RECT(m_expand_mode_current_row_rect),QBrush(ConvertIntToQColor(WC->line_background_color)));
    }

    //画编辑文字
    int x = m_window_rect.x;
    int y = m_window_rect.y + m_rect_input.y;
    p.setPen(ConvertIntToQColor(WC->compose_string_color));

    int cart_x = x;
    for(int i = 0; i < m_rect_inputs.size(); i++)
    {
        if(m_rect_inputs[i].type == TYPE_CN)
        {            
            p.setFont(*m_font_candidate);
        }
        else
        {
            p.setPen(ConvertIntToQColor(WC->compose_string_color));
            p.setFont(*m_font_input);
        }
        QRect input(QPoint(x, y), QSize(m_rect_inputs[i].rect.width, m_rect_input.height));
        p.drawText(input, m_rect_inputs[i].data, Qt::AlignLeft | Qt::AlignVCenter);

        x += m_rect_inputs[i].rect.width;
        if(m_cart_index > i)
            cart_x += m_rect_inputs[i].rect.width;
    }

    p.setPen(ConvertIntToQColor(WC->compose_caret_color));
    int factor = WC->font_height * 0.2;
    p.drawLine(QPoint(cart_x, y + factor), QPoint(cart_x, y + m_rect_input.height - factor));

    //画logo
    m_logo_svg->render(&p, MAKE_RECTF(m_rect_logo));
    if((SwitchShowModel() == VIEW_MODE_HORIZONTAL) && m_rects_candidate.size() > 0)
    {
        m_tab_expand_button_svg->render(&p,MAKE_RECTF(m_rect_tab));
    }

    //画横线
    if(m_element_should_draw & DRAW_MID_LINE)
    {
        p.fillRect(MAKE_RECT(m_rect_mid_line), QBrush(ConvertIntToQColor(WC->main_line_color)));
    }
    int mouse_on_index = -1;
    if(m_mouse_state.type == Area_Candiate)
        mouse_on_index = m_mouse_state.index;
    //画候选词
    if(m_element_should_draw & DRAW_CANDIDATE)
    {
        for(int i = 0; i < m_rects_candidate.size(); i++)
        {
            if(m_selected_index == i)
            {
                MakeSelectedRect(m_rect_selected, i);
                p.fillRect(MAKE_RECT(m_rect_selected), QBrush(ConvertIntToQColor(WC->cell_background_color)));
                if(mouse_on_index == i)
                    p.setPen(ConvertIntToQColor(WC->candidate_selected_color, 128));
                else
                    p.setPen(ConvertIntToQColor(WC->candidate_selected_color));
            }
            else
            {
                if(mouse_on_index == i)
                    p.setPen(ConvertIntToQColor(WC->candidate_string_color, 128));
                else
                    p.setPen(ConvertIntToQColor(WC->candidate_string_color));
            }
            if(m_rects_candidate[i].use_index)
            {
               p.setFont(*m_font_index);
               p.drawText(MAKE_RECT(m_rects_candidate[i].index), Qt::AlignVCenter | Qt::AlignVCenter, m_candidates_indexs.at(i));
            }
            p.setFont(*m_font_candidate);
            QString data = m_candidates.at(i);
            p.drawText(MAKE_RECT(m_rects_candidate[i].candidate), Qt::AlignVCenter | Qt::AlignVCenter, data);
            if(m_rects_candidate.at(i).is_name_candidate)
            {
                Rect indexCandiateRect = m_rects_candidate.at(i).candidate;
                m_person_name_candidate_rect.x = indexCandiateRect.x + indexCandiateRect.width - m_person_name_candidate_rect.width;
                m_person_name_candidate_rect.y = indexCandiateRect.y + (indexCandiateRect.height - m_font_size)/2;

                QString high_color,low_color;
                if(m_selected_index == i)
                {
                    ConvertIntColorToString(WC->candidate_selected_color,low_color,high_color);
                }
                else
                {
                    ConvertIntColorToString(WC->candidate_string_color,low_color,high_color);
                }

                if(mouse_on_index == i)
                {
                    SvgUtils::SetSVGBackColor(m_person_name_dom->documentElement(), QString("g"), QString("fill"), low_color, 0);
                    m_person_name_candidate_svg->load(m_person_name_dom->toByteArray());
                    m_person_name_candidate_svg->render(&p,MAKE_RECTF(m_person_name_candidate_rect));
                }
                else
                {
                    SvgUtils::SetSVGBackColor(m_person_name_dom->documentElement(), QString("g"), QString("fill"), high_color, 0);
                    m_person_name_candidate_svg->load(m_person_name_dom->toByteArray());
                    m_person_name_candidate_svg->render(&p,MAKE_RECTF(m_person_name_candidate_rect));
                }

            }
        }

    }

    //画左右翻页
    if(m_element_should_draw & DRAW_LEFT_RIGHT)
    {
        if(m_has_next)
            m_right_button_svg_high->render(&p, MAKE_RECTF(m_rect_right));
        else
            m_right_button_svg_low->render(&p, MAKE_RECTF(m_rect_right));
        if(m_has_pre)
            m_left_button_svg_high->render(&p, MAKE_RECTF(m_rect_left));
        else
            m_left_button_svg_low->render(&p, MAKE_RECTF(m_rect_left));
    }

    if(IsSpecialMode())
        DrawShadow(DEFAULT_OFFSET, DEFAULT_RADIUS);

    if((m_person_name_candidate_index != -1)&& (mouse_on_index == m_person_name_candidate_index) && (mouse_on_index < m_rects_candidate.size()))
    {
        Rect hover_candidate_rect;
        MakeSelectedRect(hover_candidate_rect, mouse_on_index);
        QRect hover_real_rect = MAKE_RECT(hover_candidate_rect);
        QPoint locate_pos = QPoint(hover_real_rect.x(), hover_real_rect.y() + hover_real_rect.height());
        QPoint global_pos = mapToGlobal(locate_pos);
        ConfigManager::Instance()->ReloadUserConfig();
        if(m_candidates.at(mouse_on_index).length() > 2)
        {
            int configItemValue = 0;
            if(ConfigManager::Instance()->GetIntConfigItem("key_name_mode",configItemValue))
            {
                QString key_value = QString::fromLatin1((char*)&configItemValue);
                m_person_name_tip_window->set_content_text(QString("按Ctrl+Shift+%1进入人名模式").arg(key_value));
            }
            m_person_name_tip_window->setFixedWidth(180);
            m_person_name_tip_window->move(global_pos);

            if(ConfigManager::Instance()->GetIntConfigItem("use_key_name_mode",configItemValue))
            {
                m_person_name_tip_window->setVisible(configItemValue);
            }
            m_delete_tip_window->setVisible(false);
        }
    }
    else if((mouse_on_index >= 0) && (mouse_on_index < m_rects_candidate.size()))
    {
        Rect hover_candidate_rect;
        MakeSelectedRect(hover_candidate_rect, mouse_on_index);
        QRect hover_real_rect = MAKE_RECT(hover_candidate_rect);
        QPoint locate_pos = QPoint(hover_real_rect.x(), hover_real_rect.y() + hover_real_rect.height());
        QPoint global_pos = mapToGlobal(locate_pos);
        if(m_candidates.at(mouse_on_index).length() > 2)
        {
             m_delete_tip_window->setCi_index(mouse_on_index);
             m_delete_tip_window->move(global_pos);
             m_delete_tip_window->setVisible(true);
             m_person_name_tip_window->setVisible(false);
        }
    }
    else
    {
        if(!m_delete_tip_window->geometry().contains(QCursor::pos()))
        {
            m_delete_tip_window->setVisible(false);
        }
        m_person_name_tip_window->setVisible(false);
    }

}

void CandidateWindow::mousePressEvent(QMouseEvent *event)
{
    ClearMouseState();
    QPoint p = event->pos();
    if(!ProcessMouseEvent(&p, Left_Button_Down))
        CustomizeQWidget::mousePressEvent(event);
    if(event->button() == Qt::LeftButton)
    {
        if(m_toolbar_main_menu != nullptr)
        {
            bool is_visible = m_toolbar_main_menu->isVisible();
            if(is_visible)
            {
                m_toolbar_main_menu->setVisible(!is_visible);
            }
        }
    }
}

void CandidateWindow::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint p = event->pos();
    if(!ProcessMouseEvent(&p, Left_Button_Release))
        CustomizeQWidget::mouseReleaseEvent(event);
    if(event->button() == Qt::RightButton)
    {
        if(m_toolbar_main_menu == nullptr)
        {
            m_toolbar_main_menu = new ToolBarMainMenu(this);
            connect(m_toolbar_main_menu,&ToolBarMainMenu::simple_tra_state_changed,this,&CandidateWindow::slot_simple_tra_changed);
        }

        QPoint click_pos = this->mapToGlobal(event->pos());
        QPoint cursor_pos = click_pos;
        int bottom_border = cursor_pos.y() + m_toolbar_main_menu->height();
        int rightBorder = cursor_pos.x() + m_toolbar_main_menu->width();

        int desktop_width = QApplication::desktop()->width();
        int screen_height = QApplication::primaryScreen()->availableGeometry().height();

        if (rightBorder > desktop_width)
        {
            cursor_pos.setX(desktop_width - m_toolbar_main_menu->width());
        }

        QList<QScreen *> list_screen =  QGuiApplication::screens();  //多显示器
        int screen_size = list_screen.size();
        if(screen_size == 1)
        {
            if (bottom_border > screen_height)
            {
                cursor_pos.setY(screen_height - m_toolbar_main_menu->height());
            }
        }
        m_toolbar_main_menu->move(cursor_pos);
        bool is_visible = m_toolbar_main_menu->isVisible();
        m_toolbar_main_menu->setVisible(!is_visible);
    }
}

void CandidateWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        slot_hide_main_menu();
    }

    QPoint p = event->pos();
    if(!ProcessMouseEvent(&p))
        CustomizeQWidget::mouseMoveEvent(event);

    DrawInputWindow();
}

void CandidateWindow::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    //m_delete_ci_btn->hide();
    ClearMouseState();
    DrawInputWindow();
}

void CandidateWindow::slot_remove_ci_by_index(const int ci_index)
{
    Display* disp = XOpenDisplay(NULL);
    if(disp == NULL)
    {
        return;
    }

    if(ci_index <0 || ci_index >9)
    {
        return;
    }
    int ctrl_value = 0xffe3;
    int key_value = 0x0030 + ci_index + 1;
    KeyCode keycode = 0, modcode = 0;

    keycode = XKeysymToKeycode (disp, key_value);
    if (keycode == 0) return;

    XTestGrabControl (disp, True);

    /* Generate modkey press */
    if (ctrl_value != 0)
    {
        modcode = XKeysymToKeycode(disp, ctrl_value);
        XTestFakeKeyEvent (disp, modcode, True, 0);
    }

    /* Generate regular key press and release */
    XTestFakeKeyEvent (disp, keycode, True, 0);
    XTestFakeKeyEvent (disp, keycode, False, 0);

    /* Generate modkey release */
    if (ctrl_value != 0)
        XTestFakeKeyEvent (disp, modcode, False, 0);

    XSync (disp, False);
    XTestGrabControl (disp, False);

    XCloseDisplay(disp);
}

void CandidateWindow::slot_simulate_click_ci(const int ci_index)
{
    if(ci_index <=9)
    {
        return;
    }
    int row_index = (ci_index / 5) + 1;
    int column_index = (ci_index % 5) + 1;
    Display* disp = XOpenDisplay(NULL);
    if(disp == NULL)
    {
        return;
    }
    int row_value = 0x0030 + row_index;
    int column_value = 0x0030 + column_index;

    XTestFakeKeyEvent(disp, XKeysymToKeycode(disp, row_value), True, CurrentTime);
    XTestFakeKeyEvent(disp, XKeysymToKeycode(disp, row_value), False, CurrentTime);

    XTestFakeKeyEvent(disp, XKeysymToKeycode(disp, column_value), True, CurrentTime);
    XTestFakeKeyEvent(disp, XKeysymToKeycode(disp, column_value), False, CurrentTime);

    XCloseDisplay(disp);
}

void CandidateWindow::slot_expand_candidates()
{
    Display* disp = XOpenDisplay(NULL);
    if(disp == NULL)
    {
        return ;
    }
    XTestFakeKeyEvent(disp, XKeysymToKeycode(disp, 0x0060), True, CurrentTime);
    XTestFakeKeyEvent(disp, XKeysymToKeycode(disp, 0x0060), False, CurrentTime);
    XCloseDisplay(disp);
    return;
}

void CandidateWindow::slot_simple_tra_changed(int state)
{
    MainController::Instance()->changeTrad(state);
    MainController::Instance()->ReloadLoginState();
}

void CandidateWindow::showEvent(QShowEvent */*event*/)
{
    slot_hide_main_menu();
}

void CandidateWindow::hideEvent(QHideEvent */*event*/)
{
    slot_hide_main_menu();
}

void CandidateWindow::slot_hide_main_menu()
{
    if(m_toolbar_main_menu != nullptr)
    {
        if(m_toolbar_main_menu->isVisible())
        {
            m_toolbar_main_menu->hide();
        }
    }
}

void CandidateWindow::SetInputString(const QString& str)
{
    m_input_string = str;
}

void CandidateWindow::SetCandidates(const KimpanelLookupTable &table)
{
    QList<KimpanelLookupTable::Entry>::iterator iter;
    QList<KimpanelLookupTable::Entry> entries = table.entries;

    m_person_name_candidate_index = -1;
    m_kimpanel_agent->QueryPersonNameCandidatesIndex();

    m_candidates.clear();
    m_candidates_indexs.clear();


    if(SwitchShowModel() != VIEW_MODE_EXPAND)
    {
        m_candidates.clear();
        m_candidates_indexs.clear();

        for (iter = entries.begin(); iter != entries.end(); ++ iter) {
            m_candidates_indexs.push_back(iter->label);
            m_candidates.push_back(iter->text);
        }
    }
    else
    {
        //扩展模式下对序号进行重新排序处理
        const int max_expand_candidates = 20;
        const int expand_candidates_per_line = 5;
        int candiates_count = min(entries.count(),max_expand_candidates);
        Config::Instance()->SetCandidatesNum(expand_candidates_per_line);
        if(candiates_count <= expand_candidates_per_line)
        {
            for(int index=0; index<candiates_count; ++index)
            {
                int column = (index%expand_candidates_per_line) + 1;
                m_candidates_indexs.push_back(QString("%1.").arg(column));
                m_candidates.push_back(entries.at(index).text);
            }
        }
        else
        {
            for(int index=0; index<candiates_count; ++index)
            {
                int row = (index/expand_candidates_per_line) + 1;
                int column = (index%expand_candidates_per_line) + 1;
                m_candidates_indexs.push_back(QString("%1%2.").arg(row).arg(column));
                m_candidates.push_back(entries.at(index).text);
            }
        }

        if(candiates_count > 0)
        {
            m_candidates_lines = (candiates_count - 1)/expand_candidates_per_line + 1;
        }
        else
        {
            m_candidates_lines = 0;
        }
    }
    m_has_pre = table.has_prev;
    m_has_next = table.has_next;
}

void CandidateWindow::SetIsHorizontal(bool is_horizontal)
{
    m_view_mode = is_horizontal ? VIEW_MODE_HORIZONTAL : VIEW_MODE_VERTICAL;
}

void CandidateWindow::SetCandidatesViewMode(VIEW_MODE inputMode)
{
     m_view_mode = inputMode;
}

void CandidateWindow::SetHightIndex(int index)
{    
    m_selected_index = index;
}

void CandidateWindow::SetCartIndex(int pos)
{
    m_cart_index = pos;
}

void CandidateWindow::SetFontSize(const int font_size)
{
    m_font_input->setPixelSize(font_size);
    m_font_candidate->setPixelSize(font_size);
    m_font_index->setPixelSize(font_size * 0.9);
}

void CandidateWindow::SetChineseFont(const QString& font_name)
{
    m_font_candidate->setFamily(font_name);
}

void CandidateWindow::SetEnglishFont(const QString& font_name)
{
    m_font_input->setFamily(font_name);
    m_font_index->setFamily(font_name);
}

void CandidateWindow::ChangeLogoAndButtonColor(const int color)
{
    MakeColor(color);

    SvgUtils::SetSVGBackColor(m_left_button_dom->documentElement(), QString("g"), QString("fill"), m_low_color, 1);
    m_left_button_svg_low->load(m_left_button_dom->toByteArray());

    SvgUtils::SetSVGBackColor(m_left_button_dom->documentElement(), QString("g"), QString("fill"), m_high_color, 1);
    m_left_button_svg_high->load(m_left_button_dom->toByteArray());

    SvgUtils::SetSVGBackColor(m_right_button_dom->documentElement(), QString("g"), QString("fill"), m_high_color, 1);
    m_right_button_svg_high->load(m_right_button_dom->toByteArray());

    SvgUtils::SetSVGBackColor(m_right_button_dom->documentElement(), QString("g"), QString("fill"), m_low_color, 1);
    m_right_button_svg_low->load(m_right_button_dom->toByteArray());

    SvgUtils::SetSVGBackColor(m_logo_content_dom->documentElement(), QString("g"), QString("fill"), m_low_color, 1);
    m_logo_svg->load(m_logo_content_dom->toByteArray());
}

void CandidateWindow::DrawInputWindow()
{
    SwitchMode();

    MakeHightAdjusted();

    CalInputViewWidthAndHeight();

    m_has_cal_done = true;

    this->resize(QSize(m_rect_whole_window.width, m_rect_whole_window.height));

    this->update();
}

void CandidateWindow::RefreshWindow()
{

    WC = WindowConfig::Instance();
    WC->LoadConfig();

    m_font_size = WC->font_height;
    m_font_input = new QFont();
    m_font_input->setFamily(WC->english_font_name);
    m_font_input->setPixelSize(m_font_size);

    m_font_candidate = new QFont();
    m_font_candidate->setFamily(WC->chinese_font_name);
    m_font_candidate->setPixelSize(m_font_size);

    m_font_index = new QFont();
    m_font_index->setFamily(WC->english_font_name);

    int index_font_size = m_font_size * 0.9;
    m_font_index->setPixelSize(index_font_size);

}

void CandidateWindow::InsertTestData()
{
    m_input_string = QString("ni'hao|");
    m_candidates << QString::fromLocal8Bit("你好")
                 << QString::fromLocal8Bit("拟好")
                 << QString::fromLocal8Bit("你号")
                 << QString::fromLocal8Bit("你")
                 << QString::fromLocal8Bit("泥");
}


/*为将来适配高分屏留有空间*/
int CandidateWindow::GetScaleValue(int origin_value)
{    
    return origin_value;
}


void CandidateWindow::DrawShadow(const int shadow_offset, const int circle_radius)
{
    if(m_shadow_corner_maker)
    {
        m_shadow_corner_maker->setOffset(shadow_offset, shadow_offset);
        m_shadow_corner_maker->setColor(QColor(0,0,0,15));
        m_shadow_corner_maker->setBlurRadius(circle_radius);
        this->setGraphicsEffect(m_shadow_corner_maker);
    }
}

void CandidateWindow::CalStrWidthAndHeight(const QString& str, const QFont& font, Rect& rect)
{
    QFontMetrics font_metrics(font);    
    rect.width = font_metrics.width(str);
    rect.height = font_metrics.height();
}

void CandidateWindow::CalStrWidthAndHeight(InputRect& rect, const QFont& font)
{
    QFontMetrics font_metrics(font);
    QString str = rect.data;
    rect.rect.width = font_metrics.width(str);    
    rect.rect.height = font_metrics.height();
}

int CandidateWindow::GetDotRightSpace()
{
    static QFontMetrics font_metrics(*m_font_index);
    static QChar c('.');
    return font_metrics.rightBearing(c);
}

void CandidateWindow::CalInputViewAreaByHorizontalMode()
{
    int width = 0;
    int height = 0;
    m_candidate_line_width = 0;
    m_max_candidate_height = 0;

    m_window_rect = {0, 0, 0, 0};
    CalWindowRectPos();
    CalPersonNameIconSize();

    if(m_candidates.size() > 0)
    {
        int	main_center_gap_height = GetScaleValue(WC->main_text_top_margin + WC->main_text_bottom_margin + 1);		//中心高度，即：拼音串与候选串之间的高度
        int scale_font_height = GetScaleValue(m_font_size);
        int ci_gap = GetCiGap();

        CalInputRect();

        for(int i = 0; i < m_candidates.size(); i++)
        {
            CandiateRect rect = {{0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0};

            QString data = m_candidates.at(i);
            CalStrWidthAndHeight(data, *m_font_candidate, rect.candidate);
            if(i == m_person_name_candidate_index)
            {
                rect.candidate.width += m_person_name_candidate_rect.width;
                rect.is_name_candidate = 1;
            }

            QString index;
            if(m_candidates_indexs.size() > i)
                index = m_candidates_indexs.at(i);
            width = rect.candidate.width;
            rect.height = rect.candidate.height; //index的高度不可能高于candidate
            if(!index.isEmpty())
            {
                rect.use_index = 1;
                CalStrWidthAndHeight(index, *m_font_index, rect.index);
                rect.index.width -= GetDotRightSpace();
                width += rect.index.width;
            }
            height = rect.height;
            rect.width = width;

            m_rects_candidate.push_back(rect);

            if(m_max_candidate_width < width)
                m_max_candidate_width = width;

            if(m_max_candidate_height < height)
                m_max_candidate_height = height;

            m_candidate_line_width += width + ci_gap;
        }

        for(int i = 0; i < m_candidates.size(); i++)
        {
            m_rects_candidate[i].y = m_rect_input.height + main_center_gap_height ;
            m_rects_candidate[i].height = m_max_candidate_height;
            m_rects_candidate[i].x = 0;
            for(int col = 0; col < i; col ++)
            {
                m_rects_candidate[i].x = m_rects_candidate[i].x + m_rects_candidate.at(col).width + ci_gap;
            }
            if(m_rects_candidate[i].use_index)
            {
                m_rects_candidate[i].candidate.y = m_rects_candidate[i].y;
                m_rects_candidate[i].index.x = m_rects_candidate[i].x;
                m_rects_candidate[i].index.y = m_rects_candidate[i].candidate.y + ((m_rects_candidate[i].candidate.height - m_rects_candidate[i].index.height) / 2);
                m_rects_candidate[i].candidate.x = m_rects_candidate[i].index.x + m_rects_candidate[i].index.width;
            }
            else
            {
                m_rects_candidate[i].candidate.y = m_rects_candidate[i].y;
                m_rects_candidate[i].candidate.x = m_rects_candidate[i].x;
            }
        }
        m_window_rect.height = m_rect_input.height + main_center_gap_height + m_max_candidate_height;

        //中横线位置
        m_rect_mid_line = {0, 0, 0, 0};
        m_rect_mid_line.y = m_rect_input.height + GetScaleValue(WC->main_text_top_margin);

        int scale_value_with_font = CalScaleWithFont(10);
        //左翻页键位置
        m_rect_left = {0, 0, 0, 0};
        m_rect_left.x = m_candidate_line_width + scale_value_with_font / 8;
        m_rect_left.y = m_rects_candidate[0].y + m_rects_candidate[0].height / 4 ;
        m_rect_left.width = m_rects_candidate[0].height * 0.5;
        m_rect_left.height = m_rects_candidate[0].height * 0.5;

        //右翻页键位置
        m_rect_right = {0, 0, 0, 0};
        m_rect_right.x = m_rect_left.x + m_rect_left.width * 1.5;
        m_rect_right.y = m_rects_candidate[0].y + m_rects_candidate[0].height / 4;
        m_rect_right.width = m_rects_candidate[0].height * 0.5;
        m_rect_right.height = m_rects_candidate[0].height * 0.5;

        //Tab键位置
        m_rect_tab.width = m_rects_candidate[0].height * 0.7;
        m_rect_tab.height = m_rects_candidate[0].height * 0.2;
        m_rect_tab.x = m_rect_right.x + m_rect_right.width + ci_gap;
        m_rect_tab.y = m_rects_candidate[0].y + m_rects_candidate[0].height*4 / 9;

        // logo 大小
        m_rect_logo = {0, 0, 0, 0};
        m_rect_logo.height = m_rect_mid_line.y * 0.8;
        m_rect_logo.width = m_rect_logo.height;        

        m_rect_input.width =  m_rect_input.width + ci_gap + m_rect_logo.width;
        m_candidate_line_width = m_rect_tab.x + m_rect_tab.width;

        int window_width = m_rect_input.width > m_candidate_line_width ? m_rect_input.width : m_candidate_line_width;
        m_window_rect.width = window_width;

        if (m_window_rect.width < MIN_MAIN_WINDOW_WIDTH)
            m_window_rect.width = MIN_MAIN_WINDOW_WIDTH;

        // 上半部分的高度
        int top_half_height = m_rect_mid_line.y;

        // logo 位置
        m_rect_logo.x = m_window_rect.width - m_rect_logo.width;
        m_rect_logo.y = (top_half_height - m_rect_logo.height) / 2 ;

        // 修正 icon 位置
        int delta = 0;
        if(m_rect_input.width < m_candidate_line_width)
            delta = m_rect_logo.x +  m_rect_logo.width - m_rect_right.x - m_rect_right.width;

        // 中横线的宽度、高度
        m_rect_mid_line.width = m_window_rect.width;
        m_rect_mid_line.height = GetScaleValue(1);

        // 修正Composition的参数
        int input_top = ((top_half_height - m_rect_input.height) / 2) - GetScaleValue(WC->main_text_top_margin - 1) + (scale_font_height/16);
        m_rect_input.y = input_top;

    }
}

void CandidateWindow::CalInputViewAreaByVerticalMode()
{
    int width = 0;
    int height = 0;
    m_candidate_line_width = 0;
    m_max_candidate_height = 0;
    m_max_candidate_width = 0;
    WindowConfig *WC = WindowConfig::Instance();

    m_window_rect = {0, 0, 0, 0};
    CalWindowRectPos();

    int	main_center_gap_height = GetScaleValue(WC->main_text_top_margin + WC->main_text_bottom_margin + 1);		//中心高度，即：拼音串与候选串之间的高度

    int scale_font_height = GetScaleValue(m_font_size);
    int ci_gap = GetCiGap();

    CalInputRect();

    if(m_candidates.size() > 0)
    {
        for(int i = 0; i < m_candidates.size(); i++)
        {
            CandiateRect rect = {{0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0};

            CalStrWidthAndHeight(m_candidates[i], *m_font_candidate, rect.candidate);
            if(i == m_person_name_candidate_index)
            {
                rect.candidate.width += m_person_name_candidate_rect.width;
                rect.is_name_candidate = 1;
            }
            QString index;
            if(m_candidates_indexs.size() > i)
                index = m_candidates_indexs.at(i);
            if(!index.isEmpty())
            {
                rect.use_index = 1;
                CalStrWidthAndHeight(m_candidates_indexs[i], *m_font_index, rect.index);
                rect.index.width -= GetDotRightSpace();
            }
            rect.height = rect.candidate.height;
            width = rect.candidate.width + rect.index.width;
            height = rect.height;
            rect.width = width;

            m_rects_candidate.push_back(rect);

            if(m_max_candidate_width < width)
                m_max_candidate_width = width;

            if(m_max_candidate_height < height)
                m_max_candidate_height = height;
        }

        m_rects_candidate[0].x = 0;
        m_rects_candidate[0].y = m_rect_input.height + main_center_gap_height ;
        m_rects_candidate[0].height = m_max_candidate_height;
        if(m_rects_candidate[0].use_index)
        {
            m_rects_candidate[0].index.x = 0;
            m_rects_candidate[0].index.y = m_rects_candidate[0].y + (m_rects_candidate[0].height - m_rects_candidate[0].index.height) / 2;
            m_rects_candidate[0].candidate.x = m_rects_candidate[0].index.x + m_rects_candidate[0].index.width;
            m_rects_candidate[0].candidate.y = m_rects_candidate[0].y;
        }
        else
        {
            m_rects_candidate[0].candidate.x = 0;
            m_rects_candidate[0].candidate.y = m_rects_candidate[0].y;
        }

        m_candidate_line_width = m_max_candidate_width + ci_gap;
        for(int i = 1; i < m_candidates.size(); i++)
        {
            m_rects_candidate[i].x = 0;
            m_rects_candidate[i].y = m_rects_candidate[i - 1].y + m_rects_candidate[i - 1].height;
            m_rects_candidate[i].height = m_max_candidate_height;

            if(m_rects_candidate[i].use_index)
            {
                m_rects_candidate[i].index.x = 0;
                m_rects_candidate[i].index.y = m_rects_candidate[i].y + (m_rects_candidate[i].height - m_rects_candidate[i].index.height) / 2;
                m_rects_candidate[i].candidate.x = m_rects_candidate[i].index.x + m_rects_candidate[i].index.width;
                m_rects_candidate[i].candidate.y = m_rects_candidate[i].y;
            }
            else
            {
                m_rects_candidate[i].candidate.x = 0;
                m_rects_candidate[i].candidate.y = m_rects_candidate[i].y;
            }

        }

        m_window_rect.height = m_rect_input.height + main_center_gap_height + m_max_candidate_height * m_candidates.size();

        //中横线位置
        m_rect_mid_line = {0, 0, 0, 0};
        m_rect_mid_line.y = m_rect_input.height + GetScaleValue(WC->main_text_top_margin);

        int scale_value_with_font = CalScaleWithFont(10);
        //左翻页键位置
        m_rect_left = {0, 0, 0, 0};
        m_rect_left.x = m_candidate_line_width + scale_value_with_font / 8;
        m_rect_left.y = m_rects_candidate[0].y + m_rects_candidate[0].height / 4;
        m_rect_left.width = m_rects_candidate[0].height * 0.5;
        m_rect_left.height = m_rects_candidate[0].height * 0.5;

        //右翻页键位置
        m_rect_right = {0, 0, 0, 0};
        m_rect_right.x = m_rect_left.x + m_rect_left.width *1.5;
        m_rect_right.y = m_rects_candidate[0].y + m_rects_candidate[0].height / 4;
        m_rect_right.width = m_rects_candidate[0].height * 0.5;
        m_rect_right.height = m_rects_candidate[0].height * 0.5;

        //当输入字符串长度大于翻页键的长度时，调整左右翻页键
        if(m_rect_input.width > m_rect_right.x + m_rect_right.width)
        {
            m_rect_right.x = m_rect_input.width - m_rect_right.width;
            m_rect_left.x = m_rect_right.x - m_rect_right.width *1.5;
        }


//        //Tab键位置
//        icon_tab.Width = CalcScaleWithFont(16);
//        icon_tab.Height = CalcScaleWithFont(4);
//        icon_tab.X = icon_right.GetRight() + ciGap;
//        icon_tab.Y = candn[0].Y + (m_candidate_line_height - icon_tab.Height) / 2;

        // logo 大小
        m_rect_logo = {0, 0, 0, 0};
        m_rect_logo.height = m_rect_mid_line.y * 0.8;
        m_rect_logo.width = m_rect_logo.height;

        m_rect_input.width =  m_rect_input.width + ci_gap + m_rect_logo.width;
        m_candidate_line_width = m_rect_right.x + m_rect_right.width;

        int window_width = m_rect_input.width > m_candidate_line_width ? m_rect_input.width : m_candidate_line_width;
        m_window_rect.width = window_width;

        if (m_window_rect.width < MIN_MAIN_WINDOW_WIDTH)
            m_window_rect.width = MIN_MAIN_WINDOW_WIDTH;

        // 上半部分的高度
        int top_half_height = m_rect_mid_line.y ;

        // logo 位置
        m_rect_logo.x = m_window_rect.width - m_rect_logo.width;
        m_rect_logo.y = (top_half_height - m_rect_logo.height) / 2;

        // 修正 icon 位置
        int delta = 0;
        if(m_rect_input.width < m_candidate_line_width)
            delta = m_rect_logo.x +  m_rect_logo.width - m_rect_right.x - m_rect_right.width;

        m_rect_left.x = m_rect_left.x + delta;
        m_rect_right.x = m_rect_right.x + delta;

        // 中横线的宽度、高度
        m_rect_mid_line.width = m_window_rect.width;
        m_rect_mid_line.height = GetScaleValue(1);

        // 修正Composition的参数
        int input_top = ((top_half_height - m_rect_input.height) / 2) - GetScaleValue(WC->main_text_top_margin - 1) + (scale_font_height/16);
        m_rect_input.y = input_top;
    }
}

void CandidateWindow::CalInputViewAreaByExpandMode()
{
    int width = 0;
    int height = 0;

    m_candidate_line_width = 0;
    m_max_candidate_height = 0;
    const int expand_mode_candidates_per_line = 5;

    m_window_rect = {0, 0, 0, 0};
    CalWindowRectPos();


    if(m_candidates.size() > 0)
    {
        //中心高度，即：拼音串与候选串之间的高度
        int	main_center_gap_height = GetScaleValue(WC->main_text_top_margin + WC->main_text_bottom_margin + 1);
        int scale_font_height = GetScaleValue(m_font_size);
        int ci_gap = GetCiGap();

        CalInputRect();

        //计算候选的尺寸(宽度和高度)
        for(int i = 0; i < m_candidates.size(); i++)
        {
            CandiateRect rect = {{0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0};

            QString data = m_candidates.at(i);
            CalStrWidthAndHeight(data, *m_font_candidate, rect.candidate);

            if(i == m_person_name_candidate_index)
            {
                rect.candidate.width += m_person_name_candidate_rect.width;
                rect.is_name_candidate = 1;
            }

            QString index;
            if(m_candidates_indexs.size() > i)
                index = m_candidates_indexs.at(i);

            int column_index = i;
            int column_max_width = 0;
            while((column_index+expand_mode_candidates_per_line) < m_candidates.size())
            {
                CandiateRect next_column_rect = {{0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0};
                QString next_column_data = m_candidates.at(column_index+expand_mode_candidates_per_line);
                CalStrWidthAndHeight(next_column_data, *m_font_candidate, next_column_rect.candidate);
                int column_temp_max_width = max(next_column_rect.candidate.width,rect.candidate.width);
                if(column_temp_max_width > column_max_width)
                {
                    column_max_width = column_temp_max_width;
                }
                column_index += expand_mode_candidates_per_line;
            }

            if(column_max_width > 0) {rect.candidate.width = column_max_width;}
            width = rect.candidate.width;
            rect.height = rect.candidate.height; //index的高度不可能高于candidate

            if(!index.isEmpty())
            {
                rect.use_index = 1;
                CalStrWidthAndHeight(index, *m_font_index, rect.index);
                rect.index.width -= GetDotRightSpace();
                width += rect.index.width;
            }
            height = rect.height;
            rect.width = width;

            m_rects_candidate.push_back(rect);

            //统计候选的最长长度和宽度
            if(m_max_candidate_width < width)
                m_max_candidate_width = width;

            if(m_max_candidate_height < height)
                m_max_candidate_height = height;

            QString content = m_candidates_indexs.at(i);
            int row_index = QString(content.at(0)).toInt();
            //single line
            if(content.length() == 2)
            {
                row_index = 1;
            }
            if(row_index == 1)
            {
                m_candidate_line_width += width + ci_gap;
            }
        }

        //expande current row backgroud color rect
        m_expand_mode_current_row_rect.width = m_candidate_line_width;
        m_expand_mode_current_row_rect.x = 0;
        m_expand_mode_current_row_rect.height = m_max_candidate_height;

        //计算候选词的位置
        int candiates_num = Config::Instance()->GetCandidatesNum();

        for(int i = 0; i < m_candidates.size(); i++)
        {
            QString content = m_candidates_indexs.at(i);
            int row_index = QString(content.at(0)).toInt();
            //single line
            if(content.length() == 2)
            {
                row_index = 1;
            }
            m_rects_candidate[i].y = (row_index-1) * m_max_candidate_height + main_center_gap_height + m_rect_input.height;
            m_rects_candidate[i].height = m_max_candidate_height;
            m_rects_candidate[i].x = 0;


            //get candidates postion x
            int start_column = (row_index - 1)*candiates_num;
            for(int col = start_column; col < i; col ++)
            {
                if(row_index == 1)
                {
                    m_rects_candidate[i].x = m_rects_candidate[i].x + m_rects_candidate.at(col).width + ci_gap;
                }
                else
                {
                    m_rects_candidate[i].x = m_rects_candidate[i-candiates_num].x;
                }

            }

            if(m_rects_candidate[i].use_index)
            {
                m_rects_candidate[i].candidate.y = m_rects_candidate[i].y;
                m_rects_candidate[i].index.x = m_rects_candidate[i].x;
                m_rects_candidate[i].index.y = m_rects_candidate[i].candidate.y + ((m_rects_candidate[i].candidate.height - m_rects_candidate[i].index.height) / 2);
                m_rects_candidate[i].candidate.x = m_rects_candidate[i].index.x + m_rects_candidate[i].index.width;
            }
            else
            {
                m_rects_candidate[i].candidate.y = m_rects_candidate[i].y;
                m_rects_candidate[i].candidate.x = m_rects_candidate[i].x;
            }
        }
        m_window_rect.height = m_rect_input.height + main_center_gap_height + (m_max_candidate_height)*m_candidates_lines;

        //中横线位置
        m_rect_mid_line = {0, 0, 0, 0};
        m_rect_mid_line.y = m_rect_input.height + GetScaleValue(WC->main_text_top_margin);

        int scale_value_with_font = CalScaleWithFont(10);
        //左翻页键位置
        m_rect_left = {0, 0, 0, 0};
        m_rect_left.x = m_candidate_line_width + scale_value_with_font / 8;
        m_rect_left.y = m_rects_candidate[0].y + m_rects_candidate[0].height / 4 ;
        m_rect_left.width = m_rects_candidate[0].height * 0.5;
        m_rect_left.height = m_rects_candidate[0].height * 0.5;

        //右翻页键位置
        m_rect_right = {0, 0, 0, 0};
        m_rect_right.x = m_rect_left.x + m_rect_left.width * 1.5;
        m_rect_right.y = m_rects_candidate[0].y + m_rects_candidate[0].height / 4;
        m_rect_right.width = m_rects_candidate[0].height * 0.5;
        m_rect_right.height = m_rects_candidate[0].height * 0.5;

        // logo 大小
        m_rect_logo = {0, 0, 0, 0};
        m_rect_logo.height = m_rect_mid_line.y * 0.8;
        m_rect_logo.width = m_rect_logo.height;

        m_rect_input.width =  m_rect_input.width + ci_gap + m_rect_logo.width;
        m_candidate_line_width = m_rect_right.x + m_rect_right.width;

        int window_width = m_rect_input.width > m_candidate_line_width ? m_rect_input.width : m_candidate_line_width;
        m_window_rect.width = window_width;

        if (m_window_rect.width < MIN_MAIN_WINDOW_WIDTH)
            m_window_rect.width = MIN_MAIN_WINDOW_WIDTH;

        // 上半部分的高度
        int top_half_height = m_rect_mid_line.y;

        // logo 位置
        m_rect_logo.x = m_window_rect.width - m_rect_logo.width;
        m_rect_logo.y = (top_half_height - m_rect_logo.height) / 2 ;

        // 修正 icon 位置
        int delta = 0;
        if(m_rect_input.width < m_candidate_line_width)
            delta = m_rect_logo.x +  m_rect_logo.width - m_rect_right.x - m_rect_right.width;

        m_rect_left.x = m_rect_left.x + delta;
        m_rect_right.x = m_rect_right.x + delta;

        // 中横线的宽度、高度
        m_rect_mid_line.width = m_window_rect.width;
        m_rect_mid_line.height = GetScaleValue(1);

        // 修正Composition的参数
        int input_top = ((top_half_height - m_rect_input.height) / 2) - GetScaleValue(WC->main_text_top_margin - 1) + (scale_font_height/16);
        m_rect_input.y = input_top;

    }
}

void CandidateWindow::CalInputViewAreaByNoCandidate()
{
    WindowConfig *WC = WindowConfig::Instance();

    m_window_rect = {0, 0, 0, 0};
    CalWindowRectPos();

    CalInputRect();
    int ci_gap = GetCiGap();
    m_window_rect.height = m_rect_input.height + m_window_rect.y;

    int mid_line_y = m_rect_input.height + GetScaleValue(WC->main_text_left_margin);

    int top_half_height = mid_line_y ;
    // logo 大小
    m_rect_logo = {0, 0, 0, 0};
    m_rect_logo.height = mid_line_y * 0.8;
    m_rect_logo.width = m_rect_logo.height;

    //window_width
    m_window_rect.width = m_rect_input.width + ci_gap + m_rect_logo.width;
    if (m_window_rect.width < MIN_MAIN_WINDOW_WIDTH)
        m_window_rect.width = MIN_MAIN_WINDOW_WIDTH;

    // logo 位置
    m_rect_logo.x = m_window_rect.width - m_rect_logo.width;
    m_rect_logo.y = (top_half_height - m_rect_logo.height) / 2 ;

}

void CandidateWindow::CalInputViewAreaByTipsMode()
{
    int width = 0;
    m_candidate_line_width = 0;
    m_max_candidate_height = 0;
    m_max_candidate_width = 0;
    WindowConfig *WC = WindowConfig::Instance();

    m_window_rect = {0, 0, 0, 0};
    CalWindowRectPos();

    int	main_center_gap_height = GetScaleValue(WC->main_text_top_margin + WC->main_text_bottom_margin + 1);		//中心高度，即：拼音串与候选串之间的高度

    int scale_font_height = GetScaleValue(m_font_size);
    int ci_gap = GetCiGap();

    CalInputRect();

    CandiateRect rect = {{0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0};

    QString data = m_candidates.at(0);
    CalStrWidthAndHeight(data, *m_font_candidate, rect.candidate);
    rect.height = rect.candidate.height;
    width = rect.candidate.width + ci_gap;
    rect.width = width;

    //int tips_width = width;
    m_rects_candidate.push_back(rect);

    m_rects_candidate[0].candidate.x = 0;
    m_rects_candidate[0].candidate.y = m_rect_input.height + main_center_gap_height ;

    //中横线位置
    m_rect_mid_line = {0, 0, 0, 0};
    m_rect_mid_line.y = m_rect_input.height + GetScaleValue(WC->main_text_top_margin);

    m_candidate_line_width = rect.width;

    m_window_rect.height = m_rect_input.height + main_center_gap_height + rect.height;


    // logo 大小
    m_rect_logo = {0, 0, 0, 0};
    m_rect_logo.height = m_rect_mid_line.y * 0.8;
    m_rect_logo.width = m_rect_logo.height;

    m_rect_input.width =  m_rect_input.width + ci_gap + m_rect_logo.width;

    int window_width = m_rect_input.width > m_candidate_line_width ? m_rect_input.width : m_candidate_line_width;
    m_window_rect.width = window_width;

    if (m_window_rect.width < MIN_MAIN_WINDOW_WIDTH)
        m_window_rect.width = MIN_MAIN_WINDOW_WIDTH;

    // 上半部分的高度
    int top_half_height = m_rect_mid_line.y ;

    // logo 位置
    m_rect_logo.x = m_window_rect.width - m_rect_logo.width;
    m_rect_logo.y = (top_half_height - m_rect_logo.height) / 2 ;

    // 修正Composition的参数
    int input_top = ((top_half_height - m_rect_input.height) / 2) - GetScaleValue(WC->main_text_top_margin - 1) + (scale_font_height/16);
    m_rect_input.y = input_top;

}

int CandidateWindow::CalScaleWithFont(int value)
{
    int scale_font_height = GetScaleValue(m_font_size);
    double rate = (double)scale_font_height / 16;
    double dvalue = (double)value * rate;
    return (int)dvalue;
}

void CandidateWindow::LoadImage()
{

    MakeColor(WC->compose_string_color);
    m_left_button_dom = new QDomDocument;
    QFile file_left(":/image/left.svg");
    file_left.open(QIODevice::ReadOnly);
    m_left_button_dom->setContent(file_left.readAll());
    file_left.close();

    m_left_button_svg_low = new QSvgRenderer(this);
    SvgUtils::SetSVGBackColor(m_left_button_dom->documentElement(), QString("g"), QString("fill"), m_low_color, 1);
    m_left_button_svg_low->load(m_left_button_dom->toByteArray());

    m_left_button_svg_high = new QSvgRenderer(this);
    SvgUtils::SetSVGBackColor(m_left_button_dom->documentElement(), QString("g"), QString("fill"), m_high_color, 1);
    m_left_button_svg_high->load(m_left_button_dom->toByteArray());

    m_right_button_dom = new QDomDocument;
    QFile file_right(":/image/right.svg");
    file_right.open(QIODevice::ReadOnly);
    m_right_button_dom->setContent(file_right.readAll());
    file_right.close();

    SvgUtils::SetSVGBackColor(m_right_button_dom->documentElement(), QString("g"), QString("fill"), m_high_color, 1);
    m_right_button_svg_high = new QSvgRenderer(this);
    m_right_button_svg_high->load(m_right_button_dom->toByteArray());

    SvgUtils::SetSVGBackColor(m_right_button_dom->documentElement(), QString("g"), QString("fill"), m_low_color, 1);
    m_right_button_svg_low = new QSvgRenderer(this);
    m_right_button_svg_low->load(m_right_button_dom->toByteArray());

    m_logo_content_dom = new QDomDocument;
    QFile file_logo(":/image/logo_cand.svg");
    file_logo.open(QIODevice::ReadOnly);
    m_logo_content_dom->setContent(file_logo.readAll());
    file_logo.close();

    SvgUtils::SetSVGBackColor(m_logo_content_dom->documentElement(), QString("g"), QString("fill"), m_low_color, 1);
    m_logo_svg = new QSvgRenderer(this);
    m_logo_svg->load(m_logo_content_dom->toByteArray());

    m_tab_button_dom = new QDomDocument;
    QFile file_tab(":/image/tab.svg");
    file_tab.open(QIODevice::ReadOnly);
    m_tab_button_dom->setContent(file_tab.readAll());
    file_tab.close();

    SvgUtils::SetSVGBackColor(m_tab_button_dom->documentElement(), QString("g"), QString("fill"), m_low_color, 1);
    m_tab_expand_button_svg = new QSvgRenderer(this);
    m_tab_expand_button_svg->load(m_tab_button_dom->toByteArray());

    m_person_name_dom = new QDomDocument();
    QFile file_person_name(":/image/name.svg");
    file_person_name.open(QIODevice::ReadOnly);
    m_person_name_dom->setContent(file_person_name.readAll());
    file_person_name.close();

    SvgUtils::SetSVGBackColor(m_person_name_dom->documentElement(), QString("g"), QString("fill"), m_low_color, 0);
    m_person_name_candidate_svg = new QSvgRenderer(this);
    m_person_name_candidate_svg->load(m_person_name_dom->toByteArray());


}

void CandidateWindow::SwitchMode()
{
//    SHOW_BH_TIPS //笔画提示
    m_element_should_draw = 0;
    do
    {
        if(m_candidates.size() == 0)
        {
            m_show_mode = SHOW_WITHOUT_CANDIDATE;
            return;
        }
        else if(m_main_model->wb() || !m_main_model->umode() || m_main_model->pinyinMode()) ////0:全拼 1：双拼
        {
            break;
        }
        else if(m_input_string.compare("u") == 0)
        {
            m_show_mode = SHOW_U_TIPS;
            m_element_should_draw |= DRAW_CANDIDATE;
            return;
        }
        else if(m_input_string.compare("i") == 0)
        {
            m_show_mode = SHOW_I_TIPS;
            m_element_should_draw |= DRAW_CANDIDATE;
            return;
        }
        else
            break;
    }while(false);


    m_show_mode = SHOW_WITH_CANDIDATE;
    m_element_should_draw |= DRAW_CANDIDATE;
    m_element_should_draw |= DRAW_LEFT_RIGHT;
    m_element_should_draw |= DRAW_MID_LINE;

}

void CandidateWindow::MakeHightAdjusted()
{
    if(m_selected_index < 0 || m_selected_index >= m_candidates.size())
        m_selected_index = 0;
    if(m_show_mode == SHOW_U_TIPS || m_show_mode == SHOW_I_TIPS) //让u/i的提示不被选中
        m_selected_index = -1;
}

void CandidateWindow::CalInputRect()
{
    m_max_height = 0;
    m_rect_inputs.clear();
    m_rect_input = {0, 0, 0, 0};

    for(int i = 0; i < m_input_string.length(); i++)
    {
        InputRect rect;
        QChar c = m_input_string.at(i);
        rect.data = c;
        if(IsChineseChar(c))
        {
            rect.type = TYPE_CN;
            CalStrWidthAndHeight(rect, *m_font_candidate);
        }
        else
        {
            CalStrWidthAndHeight(rect, *m_font_input);
        }

        m_rect_input.width += rect.rect.width;
        m_max_height = m_max_height > rect.rect.height ? m_max_height : rect.rect.height;
        m_rect_inputs.push_back(rect);
    }
    m_rect_input.height = m_max_height;
}

int CandidateWindow::GetCiGap()
{
    int scale_font_height = GetScaleValue(m_font_size);
    int ci_gap = scale_font_height / 2;
    if(ci_gap < 10)
        ci_gap = 10;

    return ci_gap;
}

bool CandidateWindow::IsChineseChar(const QChar& c)
{
    ushort uni_code = c.unicode();
    if((uni_code >= 0x4E00 && uni_code <= 0x9FA5) ||
       (uni_code >= 0x9FA6 && uni_code <= 0x9FEF) || //基本汉字补充
       (uni_code >= 0x3400 && uni_code <= 0x4DB5)) //扩展A
       //(uni_code >= 0x20000 && uni_code <= 0x2A6D6)  //扩展B

    {
        return true;
    }
    return false;
}

void CandidateWindow::MakeSelectedRect(Rect& rect, const int index)
{
    rect = {0, 0, 0, 0};

    if((SwitchShowModel() == VIEW_MODE_HORIZONTAL))
    {
        rect.y = m_rect_mid_line.y + m_rect_mid_line.height;
        rect.height = m_window_rect.height - m_rect_mid_line.y - m_rect_mid_line.height + WC->main_text_bottom_margin - GetScaleValue(1);
        if(index == 0)
        {
            rect.x = m_rects_candidate.at(index).x;
            rect.width = m_rects_candidate.at(index).width + (GetCiGap() / 2);
        }
        else
        {
            rect.x = m_rects_candidate.at(index).x - (GetCiGap() / 2);
            rect.width = m_rects_candidate.at(index).width + GetCiGap();
        }

    }
    else if(SwitchShowModel() == VIEW_MODE_EXPAND)
    {
        if(index == 0)
        {
            rect.y = m_rects_candidate.at(index).y;
            rect.height = m_rects_candidate.at(index).height;
        }
        else if(index == (m_rects_candidate.size() - 1))
        {
            rect.height = m_rects_candidate.at(index).height;
            rect.y = m_rects_candidate.at(index).y;
        }
        else
        {
            rect.height = m_rects_candidate.at(index).height;
            rect.y = m_rects_candidate.at(index).y;
        }

        rect.x = m_rects_candidate.at(index).x;
        rect.width = m_rects_candidate.at(index).width + GetCiGap()/2;
    }
    else
    {
        if(index == 0)
        {
            rect.y = m_rect_mid_line.y + m_rect_mid_line.height;
            rect.height = m_rects_candidate.at(index).height + (m_rects_candidate.at(index).y - m_rect_mid_line.y - 1);
        }
        else if(index == (m_rects_candidate.size() - 1))
        {
            rect.height = m_window_rect.height - m_rects_candidate.at(index).y + WC->main_text_bottom_margin -GetScaleValue(1);
            rect.y = m_rects_candidate.at(index).y;
        }
        else
        {
            rect.height = m_rects_candidate.at(index).height;
            rect.y = m_rects_candidate.at(index).y;
        }

        rect.x = m_rects_candidate.at(index).x;
        rect.width = m_rects_candidate.at(index).width + GetCiGap() / 2;
    }
}


QColor CandidateWindow::ConvertIntToQColor(int color_value, const int transparent)
{
    unsigned int b = (color_value & 0xff0000) >> 16;
    unsigned int g = (color_value & 0xff00) >> 8;
    unsigned int r = (color_value & 0xff);
    return QColor(r, g, b, transparent);
}

void CandidateWindow::ClearMouseState()
{
    qDebug() << "m_mouse_state has called!";
    m_mouse_state.type = Area_None;
    m_mouse_state.index = -1;
}

bool CandidateWindow::ProcessMouseEvent(QPoint* p, MouseEvent event)
{
    MouseState state = {Area_None, -1};

    p->setX(p->x() - m_window_rect.x);
    p->setY(p->y() - m_window_rect.y);
    do {
        for(int i = 0; i < m_rects_candidate.size(); i++)
        {
            Rect index_rect;
            MakeSelectedRect(index_rect, i);
            QRect index_new_rect = QRect(index_rect.x,index_rect.y,index_rect.width,index_rect.height);

            if(IsPointInRect(index_new_rect.x(),index_new_rect.width(),index_new_rect.y(),index_new_rect.height(), p))
            {                
                state.type = Area_Candiate;
                state.index = i;
                break;
            }
        }
        if(state.type != Area_None)        
            break;

        if(IsPointInRect(m_rect_left, p))
        {
            state.type = Area_Button;
            state.index = 0;
            break;
        }

        if(IsPointInRect(m_rect_right, p))
        {
            state.type = Area_Button;
            state.index = 1;
            break;
        }
        if(IsPointInRect(m_rect_tab,p))
        {
            state.type = Area_Button;
            state.index = 2;
            break;
        }
        ClearMouseState();

    } while(false);

    if(state.type == Area_None)
    {
        this->unsetCursor();
        return false;
    }
    this->setCursor(QCursor(Qt::PointingHandCursor));
    if(event == Mouse_Move)
    {
        m_mouse_state.type = state.type;
        m_mouse_state.index = state.index;
        return true;
    }
    else if(event == Left_Button_Down)
    {
        m_mouse_state.type = state.type;
        m_mouse_state.index = state.index;
        return true;
    }
    else
    {
        if(state.type == m_mouse_state.type && state.index == m_mouse_state.index)
        {
            CallbackEngine(m_mouse_state);
            return true;
        }
        else
        {
            ClearMouseState();
            return false;
        }
    }

}

void CandidateWindow::CallbackEngine(const MouseState mouse_state)
{
    if(mouse_state.type == Area_Candiate)
    {
        int candiate_index = mouse_state.index;
        if(candiate_index >=0 && candiate_index <= 9)
        {
            m_kimpanel_agent->SelectCandidate(mouse_state.index);
        }
        else
        {
            slot_simulate_click_ci(mouse_state.index);
        }

    }
    else if(mouse_state.type == Area_Button)
    {
        if(mouse_state.index == 0)
        {
            if(m_has_pre)
                m_kimpanel_agent->LookupTablePageUp();
        }
        else if(mouse_state.index == 1)
        {
            if(m_has_next)
                m_kimpanel_agent->LookupTablePageDown();
        }
        else if(mouse_state.index == 2)
        {
            slot_expand_candidates();
        }

    }
    else
        qDebug() << "call engine error!";

}

VIEW_MODE CandidateWindow::SwitchShowModel()
{
    if(!WC->show_vertical_candidate)
        return m_view_mode;
    else
        return VIEW_MODE_VERTICAL;
}

void CandidateWindow::MakeColor(int color)
{
    unsigned int b_high = (color & 0xff0000);
    b_high = b_high >> 16;
    unsigned int g_high = (color & 0xff00);
    unsigned int r_high = (color & 0xff);
    r_high = (r_high << 16);
    int high_color = r_high + g_high + b_high;

    char color_str[7] = {0};
    char color_str_after_process[7] = {0};
    sprintf(color_str, "%X", high_color);
    int length = strlen(color_str);
    for (int i = 0; i < 6 - length; i ++)
    {
        color_str_after_process[i] = '0';
    }
    strcat(color_str_after_process, color_str);
    m_high_color = QString("#%1").arg(color_str_after_process);


    unsigned int b = (color & 0xff0000) >> 16;
    unsigned int g = (color & 0xff00) >> 8;
    unsigned int r = (color & 0xff);

    r = 204 + (r / 5);
    g = 204 + (g / 5);
    b = 204 + (b / 5);

    r = r > 255 ? 255 : r;
    g = g > 255 ? 255 : g;
    b = b > 255 ? 255 : b;

    int low_color = 0;
    low_color += ( r << 16) ;
    low_color += ( g << 8);
    low_color += b;

    memset(color_str, 0, 7);
    memset(color_str_after_process, 0, 7);
    sprintf(color_str, "%X", low_color);
    length = strlen(color_str);
    for (int i = 0; i < 6 - length; i ++)
    {
        color_str_after_process[i] = '0';
    }
    strcat(color_str_after_process, color_str);
    m_low_color = QString("#%1").arg(color_str_after_process);
}

void CandidateWindow::CalWindowRectPos()
{
    int radius = 0;
    if(IsSpecialMode())
        radius = DEFAULT_RADIUS;

    m_window_rect.x = GetScaleValue(WC->main_text_left_margin) + radius;
    m_window_rect.y = GetScaleValue(WC->main_text_top_margin) + radius;
}

void CandidateWindow::CalWholeWindowSize()
{
    int double_radius = 0;
    if(IsSpecialMode())
        double_radius = DEFAULT_RADIUS * 2;

    m_rect_whole_window.width = m_window_rect.width + WC->main_text_left_margin + WC->main_text_right_margin +  double_radius;
    m_rect_whole_window.height = m_window_rect.height + WC->main_text_top_margin + WC->main_text_bottom_margin +  double_radius;
}

void CandidateWindow::CalPersonNameIconSize()
{
    double  nb, nw, nh;
    nb = m_font_size * 3 / 5;
    nw = nb * 14 / 12;
    nh = nb;

    m_person_name_candidate_rect.width = (int)(nw + 0.5);
    m_person_name_candidate_rect.height = (int)(nh + 0.5);
}

int CandidateWindow::CalInputViewWidthAndHeight()
{
    Clear();

    if(m_show_mode == SHOW_WITH_CANDIDATE)
    {
        if(SwitchShowModel() == VIEW_MODE_HORIZONTAL)
        {
            CalInputViewAreaByHorizontalMode();
        }
        else if(SwitchShowModel() == VIEW_MODE_EXPAND)
        {
            CalInputViewAreaByExpandMode();
        }
        else
        {
            CalInputViewAreaByVerticalMode();
        }
    }
    else if(m_show_mode == SHOW_WITHOUT_CANDIDATE)
    {
        CalInputViewAreaByNoCandidate();
    }
    else
    {
        CalInputViewAreaByTipsMode();
    }

    CalWholeWindowSize();
    return 1;
}

void CandidateWindow::Clear()
{
    m_rects_candidate.clear();
    m_candidate_height.clear();
}

void CandidateWindow::SetSystemMode(const bool mode)
{
    m_is_spcial_mode = mode;
}

bool CandidateWindow::IsSpecialMode()
{
    return m_is_spcial_mode;
}

void CandidateWindow::SetPersonNameCandidatesIndex(const int index)
{
    if(m_person_name_candidate_index == index)
    {
        return;
    }
    m_person_name_candidate_index = index;
    DrawInputWindow();
}

void CandidateWindow::ConvertIntColorToString(int color,QString& low_color_str,QString& high_color_str)
{
    unsigned int b_high = (color & 0xff0000);
    b_high = b_high >> 16;
    unsigned int g_high = (color & 0xff00);
    unsigned int r_high = (color & 0xff);
    r_high = (r_high << 16);
    int high_color = r_high + g_high + b_high;

    char color_str[7] = {0};
    char color_str_after_process[7] = {0};
    sprintf(color_str, "%X", high_color);
    int length = strlen(color_str);
    for (int i = 0; i < 6 - length; i ++)
    {
        color_str_after_process[i] = '0';
    }
    strcat(color_str_after_process, color_str);
    high_color_str = QString("#%1").arg(color_str_after_process);


    unsigned int b = (color & 0xff0000) >> 16;
    unsigned int g = (color & 0xff00) >> 8;
    unsigned int r = (color & 0xff);

    r = 204 + (r / 5);
    g = 204 + (g / 5);
    b = 204 + (b / 5);

    r = r > 255 ? 255 : r;
    g = g > 255 ? 255 : g;
    b = b > 255 ? 255 : b;

    int low_color = 0;
    low_color += ( r << 16) ;
    low_color += ( g << 8);
    low_color += b;

    memset(color_str, 0, 7);
    memset(color_str_after_process, 0, 7);
    sprintf(color_str, "%X", low_color);
    length = strlen(color_str);
    for (int i = 0; i < 6 - length; i ++)
    {
        color_str_after_process[i] = '0';
    }
    strcat(color_str_after_process, color_str);
    low_color_str = QString("#%1").arg(color_str_after_process);
}

