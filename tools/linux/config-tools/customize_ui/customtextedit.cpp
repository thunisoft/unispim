#include "customize_ui/customtextedit.h"
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QFile>
#include <QTextImageFormat>
#include <QTextCursor>
#include <QImageReader>
#include <QIcon>
#include <QClipboard>
#include <QApplication>
#include <QTextBlock>
#include <QDebug>

CustomTextEdit::CustomTextEdit(QWidget*parent):QTextEdit(parent)
{
    InitWidget();
}

CustomTextEdit::~CustomTextEdit()
{

}


void CustomTextEdit::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.isEmpty())
    {
        return;
    }
    foreach(QUrl url, urls)
    {
        //将图片或者文件内容插入到文档中
        QString filePath = url.toLocalFile();
        if(IsFileAImage(filePath))
        {
            InsertImageToCursor(filePath);
        }
    }
}

void CustomTextEdit::InitWidget()
{
    this->setStyleSheet("border: 1px solid #CCCCCC;font-size:15px;");
    m_imageIndex = 0;
    m_copyMenu = new QMenu(this);
    m_copyImageAction = new QAction(QIcon(":/image/copyImage.png"),"粘贴图片",this);
    connect(m_copyImageAction, SIGNAL(triggered()),this,SLOT(CopyImageToCursor()));
    //m_copyMenu->addAction(m_copyImageAction);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(ShowMenuSlot(QPoint)));

}

QList<QString> CustomTextEdit::GetImageNameList()
{

    QString htmlString = this->document()->toHtml();
    qDebug() << htmlString;

    //剔除可能的重复图片
    QList<QString>result;
    for(QString item : m_imageList)
    {
        if(!result.contains(item))
        {
            result << item;
        }
    }
    m_imageList = result;
    for(int index=0; index<m_imageList.size(); ++index)
    {
        QString image_file = m_imageList.at(index);
        QFileInfo info(image_file);
        QString file_name = info.fileName();

        if(htmlString.contains(file_name))
        {
           qDebug() << image_file;
        }
        else
        {
            m_imageList.removeAt(index);
        }

    }

  return m_imageList;
}

void CustomTextEdit::ShowMenuSlot(QPoint point)
{
    Q_UNUSED(point)
    m_copyMenu->exec(QCursor::pos());
}

void CustomTextEdit::CopyImageToCursor()
{
    QClipboard* clip = QApplication::clipboard();

    const QMimeData* mimeData = clip->mimeData();
    QImage copyImage;

    //动态拷贝文件
    if(mimeData->hasFormat("text/uri-list"))
    {
        QList<QUrl> urlList = mimeData->urls();
        foreach(QUrl url, urlList)
        {
            //将图片或者文件内容插入到文档中
            QString filePath = url.toLocalFile();
            if(IsFileAImage(filePath))
            {
                InsertImageToCursor(filePath);
            }
        }

    }

    //直接将文件放在剪切板中
    if(mimeData->hasImage())
    {
        copyImage = qvariant_cast<QImage>(mimeData->imageData());
        QTextDocument* textDocument = this->document();
        QUrl url(QString("drop_image_%1").arg(m_imageIndex++));
        textDocument->addResource(QTextDocument::ImageResource, url, QVariant(copyImage));
        QTextCursor cursor = this->textCursor();
        QTextImageFormat imageFormat;
        imageFormat.setWidth(copyImage.width());
        imageFormat.setHeight(copyImage.height());
        imageFormat.setName(url.toString());
        m_imageList.push_back(url.toString());
        cursor.insertImage(imageFormat);

    }


}


void CustomTextEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat("text/uri-list"))
            event->acceptProposedAction();

}

bool CustomTextEdit::IsFileAImage(QString filePath)
{
    if(filePath.isEmpty())
    {
        return false;
    }
    if(!QFile::exists(filePath))
    {
        return false;
    }
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filePath);

    return mime.name().startsWith("image/");
}

void CustomTextEdit::InsertImageToCursor(QString filePath)
{
    QUrl Uri(QString("file://%1").arg(filePath));
    QString urlPath = Uri.path();
    QImage image = QImageReader(filePath).read();

    double width = image.width();
    double height = image.height();
    if(width > 256)
    {
        height = 256 * (height / width);
        width = 256;
        image = image.scaled(width,height);
    }

    QTextDocument* textDocument = this->document();
    textDocument->addResource(QTextDocument::ImageResource, Uri, QVariant(image));
    QTextCursor cursor = this->textCursor();
    QTextImageFormat imageFormat;
    imageFormat.setWidth(width);
    imageFormat.setHeight(height);
    imageFormat.setName(Uri.toString());
    cursor.insertImage(imageFormat);
    m_imageList.push_back(filePath);
}

bool CustomTextEdit::InsertImageToTheDoc(QString filePath)
{
    if(IsFileAImage(filePath))
    {
        InsertImageToCursor(filePath);
        return true;
    }
    return false;
}
