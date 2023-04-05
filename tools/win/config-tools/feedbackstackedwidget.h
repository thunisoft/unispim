#ifndef FEEDBACKSTACKEDWIDGET_H
#define FEEDBACKSTACKEDWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QDropEvent>
#include <QDragEnterEvent>

namespace Ui {
class FeedbackStackedWidget;
}

class FeedbackStackedWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FeedbackStackedWidget(QWidget *parent = nullptr);
    ~FeedbackStackedWidget();

    void ClearFeedbackMsg();

private:
    void InitWidget();
    void ConnectSignalToSlot();
    void DeleteTmpImage(int tmpImageNum);

private slots:
    void ImageFormatErrorMsg(QString fileName);

private slots:

    void SubmitFeedbackInfoSlot();
    void ChangeFeedBackType(int index, bool state);
    void UpdateInputCountSlot();
    void InsertImageToTheDocument();
    /**@brief 查看文件的内容是否和文件格式匹配
   * @param[in]  imagePath 图片的地址
   * @return  函数执行结果
   * - true  格式匹配
   * - false  格式不匹配
   */
    bool IsImageContentMatchType(QString imagePath);

    void SlotFeedbackOnTheWeb();

private:
    Ui::FeedbackStackedWidget *ui;
    int m_typeValue;
    QButtonGroup* m_feedbackTypeGroup;
};

#endif // FEEDBACKSTACKEDWIDGET_H
