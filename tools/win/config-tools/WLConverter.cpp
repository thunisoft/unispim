#include "WLConverter.h"

#include "tinyxml2.h"

#ifdef WIN32
#include <Windows.h>
#else
#endif // WIN32

#include <sstream>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cctype>
#include <QRegExp>
#include <QTextStream>
#include <QFile>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QDataStream>


namespace wlconverter
{

namespace utils
{

#ifdef WIN32

std::string String::WC2UTF8(const std::wstring& wc)
{
    int lenUTF8 = WideCharToMultiByte(CP_UTF8, 0, wc.c_str(), wc.length(), NULL, 0, NULL, NULL);
    std::unique_ptr<char> utf8(new char[lenUTF8]());
    WideCharToMultiByte(CP_UTF8, 0, wc.c_str(), wc.length(), utf8.get(), lenUTF8, NULL, NULL);
    return std::string(utf8.get(), lenUTF8);
}

std::string String::UTF82MB(const std::string& utf8)
{
    int lenWC = ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), NULL, 0);
    std::unique_ptr<wchar_t> wc(new wchar_t[lenWC]());
    ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), wc.get(), lenWC);

    int lenMB = ::WideCharToMultiByte(CP_ACP, 0, wc.get(), lenWC, NULL, 0, NULL, NULL);
    std::unique_ptr<char> mb(new char[lenMB]());
    ::WideCharToMultiByte(CP_ACP, 0, wc.get(), lenWC, mb.get(), lenMB, NULL, NULL);

    return std::string(mb.get(), lenMB);
}

std::string String::MB2UTF8(const std::string& mb)
{
    int lenWC = ::MultiByteToWideChar(CP_ACP, 0, mb.c_str(), mb.length(), NULL, 0);
    std::unique_ptr<wchar_t>  wc(new wchar_t[lenWC]());
    ::MultiByteToWideChar(CP_ACP, 0, mb.c_str(), mb.length(), wc.get(), lenWC);

    int lenUTF8 = ::WideCharToMultiByte(CP_UTF8, 0, wc.get(), lenWC, NULL, 0, NULL, NULL);
    std::unique_ptr<char> utf8(new char[lenUTF8]());
    ::WideCharToMultiByte(CP_UTF8, 0, wc.get(), lenWC, utf8.get(), lenUTF8, NULL, NULL);

    return std::string(utf8.get(), lenUTF8);
}

std::wstring String::MB2WC(const std::string& mb)
{
    int lenWC = ::MultiByteToWideChar(CP_ACP, 0, mb.c_str(), mb.length(), NULL, 0);
    std::unique_ptr<wchar_t> wc(new wchar_t[lenWC]());
    ::MultiByteToWideChar(CP_ACP, 0, mb.c_str(), mb.length(), wc.get(), lenWC);
    return std::wstring(wc.get(), lenWC);
}

std::wstring String::MB2WC(const char* mb, int length)
{
    int lenWC = ::MultiByteToWideChar(CP_ACP, 0, mb, length, NULL, 0);
    std::unique_ptr<wchar_t> wc(new wchar_t[lenWC]());
    ::MultiByteToWideChar(CP_ACP, 0, mb, length, wc.get(), lenWC);
    return std::wstring(wc.get(), lenWC);
}

std::string String::WC2MB(const std::wstring& wc)
{
    int lenMB = ::WideCharToMultiByte(CP_ACP, 0, wc.c_str(), wc.length(), NULL, 0, NULL, NULL);
    std::unique_ptr<char> mb(new char[lenMB]());
    ::WideCharToMultiByte(CP_ACP, 0, wc.c_str(), wc.length(), mb.get(), lenMB, NULL, NULL);
    return std::string(mb.get(), lenMB);
}

std::wstring String::UTF82WC(const std::string& utf8)
{
    int lenWC = ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), NULL, 0);
    std::unique_ptr<wchar_t> wc(new wchar_t[lenWC]());
    ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.length(), wc.get(), lenWC);
    return std::wstring(wc.get(), lenWC);
}

#else
#endif


void File::writeWC(std::ofstream& fout, const std::wstring& str)
{
    fout.write(reinterpret_cast<const char*>(str.c_str()),
        static_cast<std::streamsize> (static_cast<unsigned long long>(str.length()) * sizeof(wchar_t)));
}

unsigned long long File::fileSize(std::ifstream& fin)
{
    fin.seekg(0, fin.end);
    unsigned long long size = fin.tellg();
    fin.seekg(0, fin.beg);
    return size;
}

std::string File::adaptPath(const std::string& filePath)
{
#ifdef WIN32
    std::string result = filePath;
    size_t offset = 0;
    size_t pos = result.find_first_of('/', offset);
    while (pos != std::string::npos)
    {
        result[pos] = '\\';
        offset = pos + 1;
        pos = result.find_first_of('/', offset);
    }
    return result;
#else
#endif
}

}  // namespace utils


WordItem::WordItem(const std::string& word, const std::string& pinyin, bool isEnglish)
    : m_word(word)
    , m_pinyin(pinyin)
    , m_isEnglish(isEnglish)
{

}

std::string WordItem::toString() const
{
    std::ostringstream oss;
    oss << m_word << "\t" << m_pinyin << "\t" << "\t" << m_isEnglish;
    return oss.str();
}


std::shared_ptr<WordLibrary> PinyinBaiduTxt::loadFromUtf8Text(const std::string& text)
{
    std::shared_ptr<WordLibrary> wordLibrary(new WordLibrary());

    std::vector<std::string> lines = utils::String::split<std::string>(text, "\r\n");
    for (const auto& line : lines)
    {
        std::istringstream iss(line);
        std::string pinyin;
        std::string word;
        iss >> word >> pinyin;

        if (pinyin.empty() || word.empty())
        {
            continue;
        }

        // 去掉拼音的末尾的分隔符
        if (pinyin.back() == '\'')
        {
            pinyin.pop_back();
            if (pinyin.empty())
            {
                continue;
            }
        }

        wordLibrary->push_back(std::shared_ptr<WordItem>(new WordItem(word, pinyin)));
    }

    return wordLibrary;
}

std::shared_ptr<WordLibrary> PinyinBaiduTxt::loadFromFile(const std::string& filePath)
{
    std::ifstream fin(filePath, std::ifstream::in | std::ifstream::binary);
    if (!fin)
    {
        throw std::runtime_error("can not open file!");
    }
    std::shared_ptr<void> close_fin_later(nullptr, [&fin](void*){
        fin.close();
    }); (void)close_fin_later;

    unsigned int fileSize = static_cast<unsigned int>(utils::File::fileSize(fin));

    std::unique_ptr<char[]> buffer(new char[fileSize] {});
    fin.read(buffer.get(), fileSize);

    if (fileSize < 2)
    {
        throw std::runtime_error("invalid file!");
    }

    if (fileSize >= 2)
    {
        if (static_cast<unsigned char>(buffer[0]) == 0xFF
            && static_cast<unsigned char>(buffer[1]) == 0xFE)  // UTF16-LE
        {
            unsigned int textSize = fileSize - 2;
            std::wstring textWC(reinterpret_cast<wchar_t*>(buffer.get() + 2), textSize / sizeof(wchar_t));
            std::string textUtf8 = utils::String::WC2UTF8(textWC);
            return loadFromUtf8Text(textUtf8);
        }
    }

    if (fileSize >= 3)
    {
        if (static_cast<unsigned char>(buffer[0]) == 0xEF
            && static_cast<unsigned char>(buffer[1]) == 0xBB
            && static_cast<unsigned char>(buffer[2]) == 0xBF)  // UTF8 BOM
        {
            unsigned int textLength = fileSize - 3;
            std::string textUtf8(buffer.get() + 3, textLength);
            return loadFromUtf8Text(textUtf8);
        }
    }

    throw std::runtime_error("invalid file!");
}

std::shared_ptr<WordLibrary> PinyinGoogleTxt::loadFromUtf8Text(const std::string& text)
{
    std::shared_ptr<WordLibrary> wordLibrary(new WordLibrary());

    std::vector<std::string> lines = utils::String::split<std::string>(text, "\r\n");
    for (const auto& line : lines)
    {
        std::istringstream iss(line);

        std::string word;
        iss >> word;

        if (word.empty())
        {
            continue;
        }

        std::string pinyin;
        std::string pinyinSeg;
        while (iss >> pinyinSeg)
        {
            pinyin += pinyinSeg + '\'';
        }
        if (pinyin.empty())
        {
            continue;
        }

        // 去掉拼音的最后一个分隔符
        if (pinyin.back() == '\'')
        {
            pinyin.pop_back();
            if (pinyin.empty())
            {
                continue;
            }
        }

        wordLibrary->push_back(std::shared_ptr<WordItem>(new WordItem(word, pinyin)));
    }

    return wordLibrary;
}

std::shared_ptr<WordLibrary> PinyinGoogleTxt::loadFromFile(const std::string& filePath)
{
    std::ifstream fin(filePath, std::ifstream::in | std::ifstream::binary);
    if (!fin)
    {
        throw std::runtime_error("can not open file!");
    }
    std::shared_ptr<void> close_fin_later(nullptr, [&fin](void*){
        fin.close();
    }); (void)close_fin_later;

    unsigned int fileSize = static_cast<unsigned int>(utils::File::fileSize(fin));

    std::unique_ptr<char[]> buffer(new char[fileSize] {});
    fin.read(buffer.get(), fileSize);

    if (fileSize < 2)
    {
        throw std::runtime_error("invalid file!");
    }

    if (fileSize >= 2)
    {
        if (static_cast<unsigned char>(buffer[0]) == 0xFF
            && static_cast<unsigned char>(buffer[1]) == 0xFE)  // UTF16-LE
        {
            unsigned int textSize = fileSize - 2;
            std::wstring textWC(reinterpret_cast<wchar_t*>(buffer.get() + 2), textSize / sizeof(wchar_t));
            std::string textUtf8 = utils::String::WC2UTF8(textWC);
            return loadFromUtf8Text(textUtf8);
        }
    }

    if (fileSize >= 3)
    {
        if (static_cast<unsigned char>(buffer[0]) == 0xEF
            && static_cast<unsigned char>(buffer[1]) == 0xBB
            && static_cast<unsigned char>(buffer[2]) == 0xBF)  // UTF8 BOM
        {
            unsigned int textLength = fileSize - 3;
            std::string textUtf8(buffer.get() + 3, textLength);
            return loadFromUtf8Text(textUtf8);
        }
    }

    // ASCII
    unsigned int textLength = fileSize;
    std::string textMB(buffer.get(), textLength);
    std::string textUtf8 = utils::String::MB2UTF8(textMB);
    return loadFromUtf8Text(textUtf8);
}

void PinyinHuayuTxt::saveToFile(std::shared_ptr<WordLibrary> wordLibrary, const std::string& filePath)
{
    if (!wordLibrary)
    {
        return;
    }

    std::ofstream fout(filePath, std::ofstream::out | std::ofstream::binary);
    if (!fout)
    {
        throw std::runtime_error("can not open file!");
    }
    std::shared_ptr<void> close_fout_later(nullptr, [&fout](void*) {
        fout.close();
        }); (void)close_fout_later;


    fout.write("\xFF\xFE", 2);

    utils::File::writeWC(fout, L"名称=用户词库\r\n");
    utils::File::writeWC(fout, L"作者=华宇输入法导库工具\r\n");
    utils::File::writeWC(fout, L"编辑=1\r\n");
    utils::File::writeWC(fout, L"\r\n");

    for (const auto& wordItem : *wordLibrary)
    {
        if (!wordItem)
        {
            throw std::runtime_error("an empty word item!");
        }
        utils::File::writeWC(fout, utils::String::UTF82WC(wordItem->word()));
        utils::File::writeWC(fout, L" ");
        utils::File::writeWC(fout, utils::String::UTF82WC(wordItem->pinyin()));
        utils::File::writeWC(fout, L" ");
        std::wostringstream oss;
        utils::File::writeWC(fout, oss.str());
        utils::File::writeWC(fout, L"\r\n");
    }
}

QString PinyinHuayuTxt::changeFileEncodingToUtf16LE(const QString &inputTxtFilePath)
{
    QFile inputTxtFile(inputTxtFilePath);
    if (!inputTxtFile.open(QIODevice::ReadOnly))
    {
        throw std::runtime_error("无法打开输入文本文件！");
    }
    std::shared_ptr<QFile> closeInputTxtFileLater(&inputTxtFile, [](QFile *inputTxtFile) {
        inputTxtFile->close();
    }); (void)closeInputTxtFileLater;

    QFileInfo inputTxtInfo(inputTxtFilePath);
    qint64 fileSize = inputTxtInfo.size();

    QDataStream inputStream(&inputTxtFile);
    if (fileSize >= 2)
    {
        inputTxtFile.reset();
        char header[2];
        memset(header,0x00,2);
        inputStream.readRawData(header,2);

        unsigned char header0 = (unsigned char)header[0];
        unsigned char header1 = (unsigned char)header[1];
        if ((header0 == 0xFF) && (header1 == 0xFE))  // UTF16-LE
        {
            return inputTxtFilePath;
        }
    }

    if (fileSize >= 3)
    {
        inputTxtFile.reset();
        char header[3] = { 0 };
        inputStream.readRawData(header, 3);
        if (static_cast<unsigned char>(header[0]) == 0xEF
                && static_cast<unsigned char>(header[1]) == 0xBB
                && static_cast<unsigned char>(header[2]) == 0xBF)  // UTF8-BOM
        {
            // 转换为UTF-16LE
            const unsigned int inBufferSize = static_cast<unsigned int>(fileSize - 3);
            std::unique_ptr<char[]> inBuffer(new char[inBufferSize]{});
            inputStream.readRawData(inBuffer.get(), static_cast<int>(inBufferSize));

            std::wstring textWC = utils::String::UTF82WC(std::string(inBuffer.get(), static_cast<int>(inBufferSize)));

            QByteArray outData;
            outData.append(static_cast<char>(0xFF));
            outData.append(static_cast<char>(0xFE));
            outData.append(reinterpret_cast<const char*>(textWC.c_str()), sizeof (wchar_t) * textWC.length());

            QString tempFilePath =  QString("%1\\%2").arg(QDir::tempPath()).arg(QFileInfo(inputTxtFilePath).fileName());

            QFile tempFile(tempFilePath);
            if (!tempFile.open(QFile::WriteOnly))
            {
                throw std::runtime_error("无法打开输出文件！");
            }
            std::shared_ptr<QFile> closeTempFileLater(&tempFile, [](QFile *tempFile) {
                tempFile->close();
            }); (void)closeTempFileLater;

            QDataStream outStream(&tempFile);
            outStream.writeRawData(outData.data(), outData.size());

            return tempFilePath;
        }
    }

    throw std::runtime_error("文件编码异常\n编码应为UTF-16LE或UTF8-BOM");
}

std::shared_ptr<WordLibrary> PinyinQQQpyd::loadFromFile(const std::string& filePath)
{
    std::ifstream fin(filePath, std::ifstream::in | std::ifstream::binary);

    return nullptr;
}

std::shared_ptr<WordLibrary> PinyinQQTxt::loadFromUtf8Text(const std::string& text)
{
    std::shared_ptr<WordLibrary> wordLibrary(new WordLibrary());

    std::vector<std::string> lines = utils::String::split<std::string>(text, "\r\n");
    for (const auto& line : lines)
    {
        std::istringstream iss(line);
        std::string pinyin;
        std::string word;
        iss >> pinyin >> word;

        if (pinyin.empty() || word.empty())
        {
            continue;
        }

        wordLibrary->push_back(std::shared_ptr<WordItem>(new WordItem(word, pinyin)));
    }

    return wordLibrary;
}

std::shared_ptr<WordLibrary> PinyinQQTxt::loadFromFile(const std::string& filePath)
{
    std::ifstream fin(filePath, std::ifstream::in | std::ifstream::binary);
    if (!fin)
    {
        throw std::runtime_error("can not open file!");
    }
    std::shared_ptr<void> close_fin_later(nullptr, [&fin](void*){
        fin.close();
    }); (void)close_fin_later;

    unsigned int fileSize = static_cast<unsigned int>(utils::File::fileSize(fin));

    std::unique_ptr<char[]> buffer(new char[fileSize] {});
    fin.read(buffer.get(), fileSize);

    if (fileSize < 2)
    {
        throw std::runtime_error("invalid file!");
    }

    if (fileSize >= 2)
    {
        if (static_cast<unsigned char>(buffer[0]) == 0xFF
            && static_cast<unsigned char>(buffer[1]) == 0xFE)  // UTF16-LE
        {
            unsigned int textSize = fileSize - 2;
            std::wstring textWC(reinterpret_cast<wchar_t*>(buffer.get() + 2), textSize / sizeof(wchar_t));
            std::string textUtf8 = utils::String::WC2UTF8(textWC);
            return loadFromUtf8Text(textUtf8);
        }
    }

    if (fileSize >= 3)
    {
        if (static_cast<unsigned char>(buffer[0]) == 0xEF
            && static_cast<unsigned char>(buffer[1]) == 0xBB
            && static_cast<unsigned char>(buffer[2]) == 0xBF)  // UTF8 BOM
        {
            unsigned int textLength = fileSize - 3;
            std::string textUtf8(buffer.get() + 3, textLength);
            return loadFromUtf8Text(textUtf8);
        }
    }

    throw std::runtime_error("invalid file!");
}

std::shared_ptr<WordLibrary> PinyinSougouScel::loadFromFile(const std::string& filePath)
{
    std::ifstream fin(filePath.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!fin)
    {
        throw std::runtime_error("can not open file!");
    }
    std::shared_ptr<void> close_fin_later(nullptr, [&fin](void*){
        fin.close();
    }); (void)close_fin_later;

    char str[128] = { 0 };
    char outstr[128] = { 0 };

    int hzPosition = 0;
    fin.read(str, 128);  //\x40\x15\x00\x00\x44\x43\x53\x01
    if (str[4] == 0x44)
    {
        hzPosition = 0x2628;
    }
    else if (str[4] == 0x45)
    {
        hzPosition = 0x26C4;
    }
    else
    {
        throw std::runtime_error("parse file failed!");
    }

    fin.seekg(0x124);
    long wordCount = -1;
    fin.read(reinterpret_cast<char*>(&wordCount), 4);

    fin.seekg(0x130);
    wchar_t libraryName[64] = { 0 };
    fin.read(reinterpret_cast<char*>(libraryName), 64);
    std::cout << "字库名称：" << utils::String::WC2MB(libraryName) << std::endl;

    fin.seekg(0x338);
    wchar_t libraryType[64] = { 0 };
    fin.read(reinterpret_cast<char*>(libraryType), 64);
    std::cout << "字库类别：" << utils::String::WC2MB(libraryType) << std::endl;

    fin.seekg(0x540);
    wchar_t libraryInfo[64] = { 0 };
    fin.read(reinterpret_cast<char*>(libraryInfo), 64);
    std::cout << "字库信息：" << utils::String::WC2MB(libraryInfo) << std::endl;

    fin.seekg(0xd40);
    wchar_t libraryHint[64] = { 0 };
    fin.read(reinterpret_cast<char*>(libraryHint), 64);
    std::cout << "字库示例：" << utils::String::WC2MB(libraryHint) << std::endl;

    fin.seekg(0x1540);
    char buf[4] = { 0 };
    fin.read(buf, 4);  // \x9D\x01\x00\x00

    m_pyDic = std::map<int, std::string>();

    while (true)
    {
        char num[4] = { 0 };
        fin.read(num, 4);
        int mark = num[0] + num[1] * 256;
        wchar_t str[128] = { 0 };
        fin.read(reinterpret_cast<char*>(str), num[2]);

        std::string pinyinUtf8 = utils::String::WC2UTF8(str);
        m_pyDic[mark] = pinyinUtf8;
        if (pinyinUtf8 == "zuo")  // 最后一个拼音
        {
            break;
        }
    }

    std::string line;
    for (auto iter = m_pyDic.begin(); iter != m_pyDic.end(); ++iter)
    {
        line += iter->second + "\",\"";
    }
    std::cout << line << std::endl;

    fin.seekg(hzPosition);

    std::shared_ptr<WordLibrary> wordLibrary(new WordLibrary());

    while (true)
    {
        try
        {
            auto items = readAPinyinWord(fin);
            std::copy(items.begin(), items.end(), std::back_inserter(*wordLibrary));
        }
        catch (...)
        {
            std::cerr << "error occur" << std::endl;
        }
        if (!fin)  // 判断文件结束
        {
            break;
        }
    }

    return wordLibrary;
}

std::vector<std::shared_ptr<WordItem>> PinyinSougouScel::readAPinyinWord(std::ifstream& fin)
{
    char num[4] = { 0 };
    fin.read(num, 4);
    int samePYcount = num[0] + num[1] * 256;
    int count = num[2] + num[3] * 256;
    // 接下来读拼音
    char str[256] = { 0 };
    for (int i = 0; i < count; i++)
    {
        char ch = '\0';
        fin.read(&ch, 1);
        str[i] = ch;
    }
    std::string wordPY;
    for (int i = 0; i < count / 2; i++)
    {
        int key = str[i * 2] + str[i * 2 + 1] * 256;
        if (i != 0)
        {
            wordPY += "'";
        }
        wordPY += m_pyDic[key];

    }
    // 接下来读词语
    std::vector<std::shared_ptr<WordItem>> items;
    for (int s = 0; s < samePYcount; s++) //同音词，使用前面相同的拼音
    {
        char num[2] = { 0 };
        fin.read(num, 2);
        int hzBytecount = num[0] + num[1] * 256;
        std::unique_ptr<wchar_t[]> str(new wchar_t[hzBytecount] {0});
        fin.read(reinterpret_cast<char*>(str.get()), hzBytecount);

        std::string wordUtf8 = utils::String::WC2UTF8(str.get());

        char dummy[2] = { 0 };
        fin.read(dummy, 2);  // 全部是10,肯定不是词频，具体是什么不知道

        char dummy2[4] = { 0 };
        fin.read(dummy2, 4);  // 每个字对应的数字不一样，不知道是不是词频

        std::shared_ptr<WordItem> item(new WordItem(wordUtf8, wordPY, 0));
        items.push_back(item);

        // 接下来10个字节什么意思呢？暂时先忽略了
        char temp[6] = { 0 };
        for (int i = 0; i < 6; i++)
        {
            char ch = '\0';
            fin.read(&ch, 1);
            temp[i] = ch;
        }
    }
    return items;
}

std::shared_ptr<WordLibrary> PinyinSougouTxt::loadFromUtf8Text(const std::string& text)
{
    std::shared_ptr<WordLibrary> wordLibrary(new WordLibrary());

    std::vector<std::string> lines = utils::String::split<std::string>(text, "\r\n");
    for (const auto& line : lines)
    {
        std::istringstream iss(line);
        std::string pinyin;
        std::string word;
        iss >> pinyin >> word;

        if (pinyin.empty() || word.empty())
        {
            continue;
        }

        // 去掉拼音的第一个分隔符
        if (pinyin[0] == '\'')
        {
            pinyin = pinyin.substr(1);
            if (pinyin.empty())
            {
                continue;
            }
        }

        wordLibrary->push_back(std::shared_ptr<WordItem>(new WordItem(word, pinyin)));
    }

    return wordLibrary;
}

std::shared_ptr<WordLibrary> PinyinSougouTxt::loadFromFile(const std::string& filePath)
{
    std::ifstream fin(filePath, std::ifstream::in | std::ifstream::binary);
    if (!fin)
    {
        throw std::runtime_error("can not open file!");
    }
    std::shared_ptr<void> close_fin_later(nullptr, [&fin](void*){
        fin.close();
    }); (void)close_fin_later;

    unsigned int fileSize = static_cast<unsigned int>(utils::File::fileSize(fin));

    std::unique_ptr<char[]> buffer(new char[fileSize] {});
    fin.read(buffer.get(), fileSize);

    if (fileSize < 2)
    {
        throw std::runtime_error("invalid file!");
    }

    if (fileSize >= 2)
    {
        if (static_cast<unsigned char>(buffer[0]) == 0xFF
            && static_cast<unsigned char>(buffer[1]) == 0xFE)  // UTF16-LE
        {
            unsigned int textSize = fileSize - 2;
            std::wstring textWC(reinterpret_cast<wchar_t*>(buffer.get() + 2), textSize / sizeof(wchar_t));
            std::string textUtf8 = utils::String::WC2UTF8(textWC);
            return loadFromUtf8Text(textUtf8);
        }
    }

    if (fileSize >= 3)
    {
        if (static_cast<unsigned char>(buffer[0]) == 0xEF
            && static_cast<unsigned char>(buffer[1]) == 0xBB
            && static_cast<unsigned char>(buffer[2]) == 0xBF)  // UTF8 BOM
        {
            unsigned int textLength = fileSize - 3;
            std::string textUtf8(buffer.get() + 3, textLength);
            return loadFromUtf8Text(textUtf8);
        }
    }

    // ASCII
    unsigned int textLength = fileSize;
    std::string textMB(buffer.get(), textLength);
    std::string textUtf8 = utils::String::MB2UTF8(textMB);
    return loadFromUtf8Text(textUtf8);
}

std::shared_ptr<WordLibrary> PinyinMicrosoftXML::loadFromFile(const std::string& filePath)
{
    tinyxml2::XMLDocument xmlDocument;
    if (tinyxml2::XML_SUCCESS != xmlDocument.LoadFile(filePath.c_str()))
    {
        throw std::runtime_error("can not open file!");
    }

    std::shared_ptr<WordLibrary> wordLibrary(new WordLibrary());

    tinyxml2::XMLElement* rootElement = xmlDocument.RootElement();
    if (!rootElement)
    {
        throw std::runtime_error("invalid file!");
    }

    tinyxml2::XMLElement *xmlEntry = rootElement->FirstChildElement("ns1:DictionaryEntry");
    while (xmlEntry)
    {
        std::shared_ptr<void> find_next_entry_at_next_iteration(nullptr, [&xmlEntry](void*) {
            xmlEntry = xmlEntry->NextSiblingElement("ns1:DictionaryEntry");
            }); (void)find_next_entry_at_next_iteration;

        // 获取拼音
        tinyxml2::XMLElement* xmlInputString = xmlEntry->FirstChildElement("ns1:InputString");
        if (!xmlInputString)
        {
            continue;
        }
        const char* xmlInputStringText = xmlInputString->GetText();
        if (!xmlInputStringText)
        {
            continue;
        }
        std::vector<std::string> pinyinSegs = utils::String::split<std::string>(xmlInputStringText, " ");
        // 去掉单字拼音后面的声调数字，比如 ai4 去掉声调后变为 ai
        for (auto& pinyin : pinyinSegs)
        {
            if (isdigit(pinyin.back()))
            {
                pinyin.pop_back();
            }
        }
        std::string pinyin = utils::String::join<std::string>(pinyinSegs, "'");
        if (pinyin.empty())
        {
            continue;
        }

        // 获取词
        tinyxml2::XMLElement* xmlOutputString = xmlEntry->FirstChildElement("ns1:OutputString");
        if (!xmlOutputString)
        {
            continue;
        }
        const char* xmlOutputStringText = xmlOutputString->GetText();
        if (!xmlOutputStringText)
        {
            continue;
        }
        std::string word(xmlOutputStringText);

        // 获取词频
        tinyxml2::XMLElement* xmlExist = xmlEntry->FirstChildElement("ns1:Exist");
        if (!xmlExist)
        {
            continue;
        }
        const char* xmlExistText = xmlExist->GetText();
        if (!xmlExistText)
        {
            continue;
        }
        std::istringstream iss(xmlExistText);

        std::shared_ptr<WordItem> wordItem(new WordItem(word, pinyin));
        wordLibrary->push_back(wordItem);
    }

    return wordLibrary;
}

std::shared_ptr<WordLibraryImporter> WordLibraryConverter::getWordLibraryImporter(const QString &firstLine)
{
    // 百度拼音
    // 僾见忾闻	ai'jian'kai'wen'	1
    QRegExp regexpBaidu("^[\u4e00-\u9fa5]+\\s+([a-z]+')+\\s+\\d+$");
    if (regexpBaidu.exactMatch(firstLine))
    {
        return std::shared_ptr<PinyinBaiduTxt>(new PinyinBaiduTxt());
    }

    // 谷歌拼音
    // 僾见忾闻	1	ai jian kai wen
    QRegExp regexpGoogle("^[\u4e00-\u9fa5]+\\s+\\d+\\s+([a-z]+\\s)+[a-z]+$");
    if (regexpGoogle.exactMatch(firstLine))
    {
        return std::shared_ptr<PinyinGoogleTxt>(new PinyinGoogleTxt());
    }

    // 搜狗拼音
    // 'ai'jian'kai'wen 僾见忾闻
    QRegExp regexpSougou("^('[a-z]+)+\\s+[\u4e00-\u9fa5]+$");
    if (regexpSougou.exactMatch(firstLine))
    {
        return std::shared_ptr<PinyinSougouTxt>(new PinyinSougouTxt());
    }

    // 微软拼音
    // <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
    QRegExp regexpMicrosoft("^\\<\\?xml.*\\?\\>$");
    if (regexpMicrosoft.exactMatch(firstLine))
    {
        return std::shared_ptr<PinyinMicrosoftXML>(new PinyinMicrosoftXML());
    }

    // QQ拼音
    // ai'jian'kai'wen 僾见忾闻 1
    QRegExp regexpQQ("^([a-z]+')*[a-z]+\\s+[\u4e00-\u9fa5]+\\s+\\d+$");
    if (regexpQQ.exactMatch(firstLine))
    {
        return std::shared_ptr<PinyinQQTxt>(new PinyinQQTxt());
    }

    return nullptr;
}

QString WordLibraryConverter::getWordLibraryFileFirstLine(const QString &filePath, const QString &encoding)
{
    QFile file(filePath);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream fin(&file);
    if (!encoding.isEmpty())
    {
        fin.setCodec(encoding.toStdString().c_str());
    }
    QString line = fin.readLine();
    file.close();
    return line;
}

QStringList WordLibraryConverter::getWordlibrarayFileThreeLine(const QString &filePath, const QString &encoding)
{
    QFile file(filePath);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream fin(&file);
    if (!encoding.isEmpty())
    {
        fin.setCodec(encoding.toStdString().c_str());
    }
    QStringList lines;
    int i = 0;
    QString line;
    do{
       line = fin.readLine();
       if(line.isEmpty())
           break;
       lines << line;
       i++;
    }while(i < 3);
    file.close();
    return lines;
}


void WordLibraryConverter::convert(std::shared_ptr<WordLibraryImporter> importer,
                    const QString &importFilePath,
                    std::shared_ptr<WordLibraryExporter> exporter,
                    const QString &exportFilePath)
{
#ifdef WIN32
    std::string importFilePathAdapted(importFilePath.toUtf8().data());
    importFilePathAdapted = utils::String::UTF82MB(importFilePathAdapted);
    importFilePathAdapted = utils::File::adaptPath(importFilePathAdapted);
    std::string exportFilePathAdpated(exportFilePath.toUtf8().data());
    exportFilePathAdpated = utils::String::UTF82MB(exportFilePathAdpated);
    exportFilePathAdpated = utils::File::adaptPath(exportFilePathAdpated);
#else
    std::string importFilePathAdapted = importFilePath;
    std::string exportFilePathAdpated = exportFilePath;
#endif

    auto wordLibrary = importer->loadFromFile(importFilePathAdapted);
    exporter->saveToFile(wordLibrary, exportFilePathAdpated);
}

bool WordLibraryConverter::isPinyinHuayu(const QString &firstLine)
{
    // 华宇拼音
    // 名称=用户词库
    QRegExp regexpHuayu("^\u540d\u79f0\\=.*$");
    return regexpHuayu.exactMatch(firstLine);
}

QString WordLibraryConverter::convert(const QString &importFilePath)
{
    std::shared_ptr<WordLibraryExporter> exporter(new PinyinHuayuTxt());
    QString exportFilePath = QString("%1/%2").arg(QDir::tempPath()).arg(QFileInfo(importFilePath).fileName());

    QStringList encodingListForTest;
    encodingListForTest << "" << "GBK" << "UTF8" << "UTF16";
    for (const QString &encoding : encodingListForTest)
    {
        //QString firstLine = getWordLibraryFileFirstLine(importFilePath, encoding);
        QStringList lines = getWordlibrarayFileThreeLine(importFilePath, encoding);
        for(const QString &line : lines)
        {
            if (isPinyinHuayu(line))
            {
                PinyinHuayuTxt converter;
                return converter.changeFileEncodingToUtf16LE(importFilePath);
            }

            auto importer = getWordLibraryImporter(line);
            if (importer)
            {
                convert(importer, importFilePath, exporter, exportFilePath);
                return exportFilePath;
            }
        }
    }

    throw std::runtime_error("convert failed!");
}

}  // namespace wlconverter
