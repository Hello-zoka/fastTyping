#include "database.h"
#include "dictionaryDB.h"
#include "dictionaries.h"

using namespace FastTyping::Server;
using namespace FastTyping::Logic;


int main() {
    DictionariesStorage flusher;
    flusher.dropDictionaries();

    DictionariesStorage storage;
    storage.addDictionary("Aboba", false, "const");
    storage.addDictionary("Boba", false, "const");
    storage.addDictionary("File", false, "file");
    storage.addDictionary("DLL", false, "dll");
    storage.addDictionary("English", true, "corpus");

    ConstDictionariesStorage constant;
    constant.addConst("Aboba", "This is sample don't judge me");
    constant.addConst("Boba", "Sztirlic nie podumal...");

    FileDictionariesStorage file;
    file.addFile("File", "test.txt");

    DLLDictionariesStorage dll;
    dll.addDLL("DLL", "test");

    CorpusDictionariesStorage corpus;
    corpus.addCorpus("English", "EnglishTop100");

    add_corpus_dictionary("test", {"name", "main", "moon"});

    return 0;
}