// 原与 QML 交互的数据部分，现在仍需要

#include <QDebug>
#include "mainmodel.h"
#include "candidateword.h"

MainModel* MainModel::mSelf = 0;

MainModel* MainModel::self()
{
    if (!mSelf) {
        mSelf = new MainModel;
    }
    return mSelf;
}

MainModel::MainModel()
{

}

MainModel::~MainModel()
{

}

void MainModel::resetData() {
    KimpanelLookupTable lookup_table;
    setCandidateWords(lookup_table);
    setInputString("");
    setHasPrev(false);
    setHasNext(false);
    setInputStringCursorPos(-1);
    setHighLight(-1);
    //这个不属于焦点切换时需要重置的值
    //setIsHorizontal(true);
    setPinyinMode(0); //全拼
    setShowTips(false);
    setShowPreedit(false);
    setShowLookupTable(false);
}

void MainModel::setInputString(const QString inputString) {
    mInputString = inputString;
    emit inputStringChanged();
}

QString MainModel::inputString() const {
    return mInputString;
}

void MainModel::setTipsString(const QString tipsString) {
    mTipsString = tipsString;
    emit tipsStringChanged();
    emit mainWindowSizeChanged();
}

QString MainModel::tipsString() const {
    return mTipsString;
}

void MainModel::setInputStringCursorPos(int pos) {
    setInputString(mInputString.insert(pos, QString("|")));
}

void MainModel::setCandidateWords(const KimpanelLookupTable &lookup_table) {
    CandidateWord *candidate;
    QList<KimpanelLookupTable::Entry>::iterator iter;
    QList<KimpanelLookupTable::Entry> entries = lookup_table.entries;

    foreach (candidate, mCandidateWords) {
        candidate->deleteLater();
   }
    mCandidateWords.clear();
    for (iter = entries.begin(); iter != entries.end(); ++ iter) {
        if ((candidate = new (std::nothrow)CandidateWord) == NULL)
            break;

        // '<'在qml中会当成html的字符，所以，在这里做一层转义，其他的常用html字符目前尝试，没有出现问题，所以这里目前仅处理'<'
        QString text = iter->text;
        text.replace("<", "&lt;");

        candidate->setCddLabel(iter->label);
        candidate->setCddText(text);
        mCandidateWords.append(candidate);
    }

    setHasPrev(lookup_table.has_prev);
    setHasNext(lookup_table.has_next);

    emit candidateWordsChanged();
    emit mainWindowSizeChanged();
}

void MainModel::setHasPrev(const bool hasPrev) {
    mHasPrev = hasPrev;
    emit hasPrevChanged();
}

bool MainModel::hasPrev() const {
    return mHasPrev;
}

void MainModel::setHasNext(const bool hasNext) {
    mHasNext = hasNext;
    emit hasNextChanged();
}

bool MainModel::hasNext() const {
    return mHasNext;
}

void MainModel::setHighLight(const int highLight) {
    mHighLight = highLight;
    emit highLightChanged();
}


int MainModel::highLight() const {
    return mHighLight;
}

void MainModel::setPinyinMode(const int pinyinMode)
{
    mPinyinMode = pinyinMode;
    emit pinyinModeChanged();
}

int MainModel::pinyinMode() const
{
    return mPinyinMode;
}

void MainModel::setIsHorizontal(const bool isHorizontal) {
    mIsHorizontal = isHorizontal;
    emit isHorizontalChanged();
}

bool MainModel::isHorizontal() const {
    return mIsHorizontal;
}

void MainModel::setShowTips(const bool showTips) {
    mShowTips = showTips;
    emit showTipsChanged();
    emit mainWindowSizeChanged();
}

bool MainModel::showTips() const {
    return mShowTips;
}

void MainModel::setShowPreedit(const bool showPreedit) {
    mShowPreedit = showPreedit;
    emit showPreeditChanged();
    emit mainWindowSizeChanged();
}

bool MainModel::showPreedit() const {
    return mShowPreedit;
}

void MainModel::setShowLookupTable(const bool showLookupTable) {
    mShowLookupTable = showLookupTable;
    emit showLookupTableChanged();
    emit mainWindowSizeChanged();
}

bool MainModel::showLookupTable() const {
    return mShowLookupTable;
}

void MainModel::setWb(const bool isWb)
{
    mIsWb = isWb;
    emit mWbChanged();
}

bool MainModel::wb()
{
    return mIsWb;
}

void MainModel::setumode(const bool isenable)
{
    mIsUmodeEnable = isenable;
    emit umodeChanged();
}

bool MainModel::umode()
{
    return mIsUmodeEnable;
}

int MainModel::candidatesExpandFlag() const
{
    return mCandidatesExpandFlag;
}

void MainModel::setCandidatesExpandFlag(int candidatesExpandFlag)
{
    mCandidatesExpandFlag = candidatesExpandFlag;
    emit candiatesExpandFlagChanged();
}
