#ifndef LIBRARY_H
#define LIBRARY_H
#include "LibraryEntry.h"
#include <string>
#include <vector>
#include <algorithm>
#include <json-c/json.h>

class Library
{

    /* Structure that wraps LibraryEntries so that we can make chains
       (linked lists) to deal with collisions in the hash table.
       (Private to the Library class) */
    struct LibraryEntryWrapper {
        LibraryEntry *entry;
        LibraryEntryWrapper *next;
        LibraryEntryWrapper *previous;

        /* Constructor */
        LibraryEntryWrapper(){
            entry = NULL;
            next = NULL;
            previous = NULL;
        }
    };

    public:
        Library(std:: string username);
        virtual ~Library();
        LibraryEntry* getLibraryEntry(std::string title);
        std::vector<LibraryEntry*> getLibraryEntries(library_status ls);
        int getLibrarySize();

    protected:
    private:
        int getLibrary(std::string username);
        void addEntry(LibraryEntry *x);
        int hashSum(std::string title);
        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
        static bool libraryEntryTitleSort(LibraryEntry* i, LibraryEntry* j);
        bool curl_setup;
        json_object *library_json;
        int library_size;
        int hash_size;
        LibraryEntryWrapper *hashTable;
};

#endif // LIBRARY_H
