#include "customtextedit.h"
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
#include <QList>

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
    m_imageIndex = 0;
    m_copyMenu = new QMenu(this);
    m_copyImageAction = new QAction(QIcon(":/image/copyImage.png"),"粘贴图片",this);
    connect(m_copyImageAction, SIGNAL(triggered()),this,SLOT(CopyImageToCursor()));
    //m_copyMenu->addAction(m_copyImageAction);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(ShowMenuSlot(QPoint)));
    connect(this, SIGNAL(textChanged()),this, SLOT(feedBackContentChanged()));

}

QList<QString> CustomTextEdit::GetImageNameList()
{

    QString htmlString = this->document()->toHtml();

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
    //添加了对拖入图片格式的校验
    QMap<QString,QString> file_type_header;
    file_type_header["jpg"] = "0xFFD8FF";
    file_type_header["jpeg"] = "0xFFDBEF";
    file_type_header["png"] = "0x89504E47";
    file_type_header["gif"] = "0x47494638";
    file_type_header["tif"] = "0x49492A";
    file_type_header["tiff"] = "0x49492A";
    file_type_header["bmp"] = "0x424D";

    QUrl Uri(QString("file://%1").arg(filePath));
    QString urlPath = Uri.path();

    if(!QFile::exists(filePath))
    {
        return;
    }

    QFileInfo file_info(filePath);
    QString file_suffix = file_info.suffix();
    if(file_type_header.contains(file_suffix))
    {
        QFile file(filePath);
        if(!file.open(QIODevice::ReadOnly))
        {
            return;
        }
        QDataStream dataStream(&file);
        int length = file_type_header[file_suffix].length();
        if(file.size()<length)
        {
            return;
        }
        char fileContent[64];
        memset(fileContent,0x00,64);
        dataStream.readRawData(fileContent,length/2 - 1);
        file.close();
        bool isOK;
        QString filetype = file_type_header[file_suffix];
        unsigned long type_header = filetype.toULong(&isOK,16);
        QString hex_str = QString(QByteArray(fileContent).toHex());
        unsigned long read_header = hex_str.toULong(&isOK,16);
        if(type_header !=  read_header)
        {
            emit ImageTypeInvalid(file_info.fileName());
            return;
        }
    }

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

void CustomTextEdit::feedBackContentChanged()
{
    QString htmlContent = this->toHtml();
    qDebug() << htmlContent;

    QList<QString>::iterator itor = m_imageList.begin();

    while(itor != m_imageList.end())
    {
        if(htmlContent.contains(*itor))
        {
            m_imageList.erase(itor);
        }
        else
        {
           ++itor;
        }
    }
}
