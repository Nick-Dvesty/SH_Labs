#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <set>
#include <mutex>
#include <thread>
#include <list>
#include <chrono>

#define DEBUG
//#undef DEBUG

using namespace std;


class crawl{
private:
    static string readFile(const string& filePath);
    static vector<string> extractLinks(const string& html);
    static bool copyFile(const std::string& source, const std::string& destination);
    static void crawlPageRec(const std::string&, set<string>*, int*, int*, int*, int *, mutex*);
    static vector<string> findNotUsing(vector<string> &findLinks, set<string> &allLinks);
    static int availableThread(int &countSameTimeThread, int &usingCountThread, size_t notUsingSize);
public:
    static void crawlPage(const string& url, int countSameTimeThread);
};

string crawl::readFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) throw invalid_argument("Invalid path: " + filePath);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return content;
}
vector<string> crawl::extractLinks(const string& html) {
    vector<string> links;
    regex link_regex(R"(<a\s+href=["'](file://[^"']+)["'])");
    smatch matches;
    string::const_iterator search_start(html.cbegin());
    while (regex_search(search_start, html.cend(), matches, link_regex)) {
        links.push_back(matches[1].str());
        search_start = matches.suffix().first;
    }

    return links;
}
void crawl::crawlPageRec(const string& url, set<string> *allLinks, int *resultCountPage, int *resultCountThread, int *countSameTimeThread, int *usingCountThread, mutex *mtx) {
    if (url.substr(0, 7) != "file://") {
        throw invalid_argument("Invalid URL: " + url);
    }
    mtx->lock();
    (*resultCountPage)++;
    mtx->unlock();
    string file_path = url.substr(7);
    copyFile(file_path, "copy_dir/" + file_path);
    string html_content = readFile(file_path);
    vector<string> links = extractLinks(html_content);
    mtx->lock();
    vector<string> notUsing = findNotUsing(links, *allLinks);
    auto countFileBaseThread = availableThread(*countSameTimeThread, *usingCountThread, notUsing.size());
    mtx->unlock();
    thread l[notUsing.size()];
    if (notUsing.size() > countFileBaseThread) {
        for (int i = 0; i < notUsing.size() - countFileBaseThread; i++) {
            mtx->lock();
            #ifdef DEBUG
            string output = std::to_string(*resultCountThread) + " " + std::to_string(*usingCountThread) +
                            ": create thread: " + notUsing[i] + "\n";
            std::cout<<output;
            #endif
            (*resultCountThread)++;
            mtx->unlock();
            l[i] = thread(crawlPageRec, notUsing[i], allLinks, resultCountPage, resultCountThread,
                          countSameTimeThread, usingCountThread, mtx);
        }
    }
    if (notUsing.size() > 0) {
        int i = (int)notUsing.size() - countFileBaseThread > 0 ? notUsing.size() - countFileBaseThread : 0;
        for (; i < notUsing.size(); ++i) {
            crawlPageRec(notUsing[i], allLinks, resultCountPage,
                         resultCountThread, countSameTimeThread, usingCountThread, mtx);
        }
    }
    if (notUsing.size() > countFileBaseThread) {
        for (int i = 0; i < notUsing.size() - countFileBaseThread; ++i) {
            l[i].join();
            mtx->lock();
            (*usingCountThread)--;
            mtx->unlock();
        }
    }
}
void crawl::crawlPage(const string& url, int countSameTimeThread) {
    if (countSameTimeThread < 1) throw invalid_argument("How can there be zero streams, asshole");
    set<string> allLinks;
    int resultCountPage = 0;
    int resultCountThread = 1;
    int usingCountThread = 1;
    mutex mtx;
    allLinks.insert(url);
    auto start_time = chrono::high_resolution_clock::now();
    crawlPageRec(url, &allLinks, &resultCountPage, &resultCountThread, &countSameTimeThread, &usingCountThread,
                 &mtx);
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end_time - start_time;
    std::cout<<"Count pages: " << resultCountPage << endl
    << "Count all thread: " << resultCountThread << endl
    << "All time: " << duration.count();
}
bool crawl::copyFile(const std::string& source, const std::string& destination) {
    std::ifstream src(source, std::ios::binary);  // Открытие исходного файла в бинарном режиме
    if (!src) {
        std::cerr << "Ошибка открытия исходного файла: " << source << std::endl;
        return false;
    }

    std::ofstream dest(destination, std::ios::binary);  // Открытие файла назначения в бинарном режиме
    if (!dest) {
        std::cerr << "Ошибка открытия файла назначения: " << destination << std::endl;
        return false;
    }

    // Копирование содержимого исходного файла в файл назначения
    dest << src.rdbuf();
    return true;
}
vector<string> crawl::findNotUsing(vector<std::string> &findLinks, set<std::string> &allLinks) {
    vector<string> notUsing;
    for (auto & findPath : findLinks) {
        bool used = false;
        for (auto & path : allLinks) {
            if (path == findPath){
                used = true;
                break;
            }
        }
        if (!used) {
            notUsing.push_back(findPath);
            allLinks.insert(findPath);
        }
    }
    return notUsing;
}
int crawl::availableThread(int &countSameTimeThread, int &usingCountThread, size_t notUsingSize) {
    if (countSameTimeThread - usingCountThread > 0) {
        if (countSameTimeThread - usingCountThread > notUsingSize) {
            usingCountThread += notUsingSize;
            return 0;
        } else {
            usingCountThread = countSameTimeThread;
            return notUsingSize - (countSameTimeThread - usingCountThread);
        }
    } else {
        return notUsingSize;
    }
}


int main() {
    string start_url = "file://0.html";
    crawl::crawlPage(start_url, 100);
}


