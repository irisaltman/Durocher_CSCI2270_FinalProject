#ifndef LIBRARY_H
#define LIBRARY_H
#include "LibraryEntry.h"
#include <string>
#include <json-c/json.h>

struct LibraryEntryWrapper {
    LibraryEntry *entry;
    LibraryEntryWrapper *next;
    LibraryEntryWrapper *previous;

    // Constructor
    LibraryEntryWrapper(){
        entry = NULL;
        next = NULL;
        previous = NULL;
    }
};

class Library
{
    public:
        Library(std:: string username);
        virtual ~Library();
        LibraryEntry* getLibraryEntry(std::string title);
        int getLibrarySize();

    protected:
    private:
        int getLibrary(std::string username);
        void addEntry(LibraryEntry *x);
        int hashSum(std::string title);
        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
        bool curl_setup;
        json_object *library_json;
        int library_size;
        int hash_size;
        LibraryEntryWrapper *hashTable;
};

#endif // LIBRARY_H
