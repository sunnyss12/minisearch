#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <string>
#include <map>
#include <set>
#include <vector>
#include "Simhasher.h"

class InvertedIndex;

class Document //copyable
{
    friend bool operator==(const Document &d1, const Document &d2);
    friend bool operator!=(const Document &d1, const Document &d2);
public:
    typedef std::pair<std::string, double> WordItem;
    typedef std::map<std::string, int> WordFrequency;
    typedef std::map<std::string, double> WordHeight;

    Document() :docid_(-1) {};
    void setDocId(int docid);
    void setTitle(std::string title);
    void setContent(std::string content);

    int getDocId() const
    { return docid_; }
    const std::string &getTitle() const
    { return title_; }
    const std::string &getContent() const
    { return content_; }

    void setText(std::string text); //设置全文 包括docid title content
    const std::string getText() const
    { return text_; }

    void computeWordFrequency(); //计算词频
    void extractTopK(); //抽取topK


    void computeWordWeight(); //计算每个词的TF-IDF
    void normalizeWordWeight(); //文档内权重归一化
    void addWeightToInvertedIndex(InvertedIndex &index) const;  //将自己的权重插入倒排索引

    //clear 节约内存
    void clearContent();
    void clearWordFrequency();

    //计算向量与本文本的相似度 static函数
    static double computeSimilarity(int docid, const std::vector<std::pair<std::string, double>> &vec, const InvertedIndex &index);

    uint64_t computeSimhash();

private:
    int docid_;
    std::string title_;
    std::string content_;
    std::string text_;

    WordFrequency wordFrequency_; //单词词频
    std::vector<WordItem> topK_; //存放词频最高的K个单词及其权重

    WordHeight wordWeight_; //单词权重 && 归一化权重

    Simhash::Simhasher simhasher_;
    static const int kTopWord = 10;
};


#endif //DOCUMENT_H
