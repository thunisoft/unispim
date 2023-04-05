#ifndef CUSTOMTEXTEDIT_H
#define CUSTOMTEXTEDIT_H

#include <QTextEdit>
#include <QWidget>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMenu>
#include <QAction>
#include  <QSet>


class CustomTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    CustomTextEdit(QWidget*parent = nullptr);
    ~CustomTextEdit();

    QList<QString> GetImageNameList();
    bool InsertImageToTheDoc(QString filePath);

public slots:
    void feedBackContentChanged();
protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);


private:
    bool IsFileAImage(QString filePath);
    void InsertImageToCursor(QString filePath);
    void InitWidget();

private slots:
     void CopyImageToCursor();
     void ShowMenuSlot(QPoint point);

signals:

     void ImageTypeInvalid(QString fileName);


private:
    QMenu* m_copyMenu;
    QAction* m_copyImageAction;
    int m_imageIndex;
    QList<QString> m_imageList;
};

#endif // CUSTOMTEXTEDIT_H
