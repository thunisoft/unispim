#ifndef WL_CONVERTER_H
#define WL_CONVERTER_H

#include <string>
#include <memory>
#include <fstream>
#include <vector>
#include <map>
#include <QString>
#include <QStringList>

namespace wlconverter
{

namespace utils
{

class String
{
public:
    static std::string WC2UTF8(const std::wstring& str);
    static std::string UTF82MB(const std::string& str);
    static std::string MB2UTF8(const std::string& mb);
    static std::wstring MB2WC(const std::string& mb);
    static std::wstring MB2WC(const char* mb, int length);
    static std::string WC2MB(const std::wstring& wc);
    static std::wstring UTF82WC(const std::string& utf8);

    template <typename T, typename ContainerT = std::vector<T>>
    static ContainerT split(const T& text, const T& splitter)
    {
        ContainerT result;
        size_t offset = 0;
        size_t pos = text.find_first_of(splitter, offset);
        while (pos != T::npos)
        {
            T line = text.substr(offset, pos - offset);
            if (!line.empty())
            {
                result.push_back(line);
            }
            offset = pos + splitter.length();
            pos = text.find_first_of(splitter, offset);
        }

        if (offset != T::npos)
        {
            T line = text.substr(offset);
            if (!line.empty())
            {
                result.push_back(line);
            }
        }

        return result;
    }

    template <typename T, typename ContainerT = std::vector<T>>
    static T join(const ContainerT& segs, const T& splitter)
    {
        T result;
        for (auto iter = segs.begin(); iter != segs.end(); ++iter)
        {
            if (iter != segs.begin())
            {
                result += splitter;
            }
            result += *iter;
        }
        return result;
    }
};

class File
{
public:
    static void writeWC(std::ofstream& fout, const std::wstring& str);
    static unsigned long long fileSize(std::ifstream& fin);
    static std::string adaptPath(const std::string& filePath);
};

}  // namespace utils

class WordItem
{
public:
    explicit WordItem(const std::string& word = "",
        const std::string& pinyin = "", bool isEnglish = false);

    const std::string& word() const { return m_word; }
    void setWord(const std::string& word) { m_word = word; }

    const std::string& pinyin() const { return m_pinyin; }
    void setPinyin(const std::string& pinyin) { m_pinyin = pinyin; }

    bool isEnglish() const { return m_isEnglish; }
    void setIsEnglish(bool isEnglish) { m_isEnglish = isEnglish; }

    std::string toString() const;

private:
    std::string m_word;
    std::string m_pinyin;
    bool m_isEnglish = false;
};

typedef std::vector<std::shared_ptr<WordItem>> WordLibrary;

class WordLibraryImporter
{
public:
    virtual std::shared_ptr<WordLibrary> loadFromFile(const std::string& filePath) = 0;
};

class WordLibraryExporter
{
public:
    virtual void saveToFile(std::shared_ptr<WordLibrary> wordLibrary, const std::string& filePath) = 0;
};

class PinyinBaiduTxt : public WordLibraryImporter
{
public:
    virtual std::shared_ptr<WordLibrary> loadFromFile(const std::string& filePath) override;

private:
    std::shared_ptr<WordLibrary> loadFromUtf8Text(const std::string& text);
};

class PinyinGoogleTxt : public WordLibraryImporter
{
public:
    virtual std::shared_ptr<WordLibrary> loadFromFile(const std::string& filePath) override;

private:
    std::shared_ptr<WordLibrary> loadFromUtf8Text(const std::string& text);
};

class PinyinHuayuTxt : public WordLibraryExporter
{
public:
    virtual void saveToFile(std::shared_ptr<WordLibrary> wordLibrary, const std::string& filePath) override;
    QString changeFileEncodingToUtf16LE(const QString &filePath);
};

class PinyinQQQpyd : public WordLibraryImporter
{
public:
    virtual std::shared_ptr<WordLibrary> loadFromFile(const std::string& filePath) override;
};

class PinyinQQTxt : public WordLibraryImporter
{
public:
    virtual std::shared_ptr<WordLibrary> loadFromFile(const std::string& filePath) override;

private:
    std::shared_ptr<WordLibrary> loadFromUtf8Text(const std::string& text);
};

class PinyinSougouScel : public WordLibraryImporter
{
public:
    virtual std::shared_ptr<WordLibrary> loadFromFile(const std::string& filePath) override;

private:
    std::vector<std::shared_ptr<WordItem>> readAPinyinWord(std::ifstream& fin);

private:
    std::map<int, std::string> m_pyDic;
};

class PinyinSougouTxt : public WordLibraryImporter
{
public:
    virtual std::shared_ptr<WordLibrary> loadFromFile(const std::string& filePath) override;

private:
    std::shared_ptr<WordLibrary> loadFromUtf8Text(const std::string& text);
};

class PinyinMicrosoftXML : public WordLibraryImporter
{
public:
    virtual std::shared_ptr<WordLibrary> loadFromFile(const std::string& filePath) override;
};

class WordLibraryConverter
{
public:
    static QString convert(const QString &filePath);

private:
    static std::shared_ptr<WordLibraryImporter> getWordLibraryImporter(const QString &firstLine);
    static QString getWordLibraryFileFirstLine(const QString &filePath, const QString &encoding);
    static QStringList getWordlibrarayFileThreeLine(const QString &filePath, const QString &encoding);
    static void convert(std::shared_ptr<WordLibraryImporter> importer,
                        const QString &importFilePath,
                        std::shared_ptr<WordLibraryExporter> exporter,
                        const QString &exportFilePath);
    static bool isPinyinHuayu(const QString &firstLine);
};

}  // namespace wlconverter


#endif // !WL_CONVERTER_H
