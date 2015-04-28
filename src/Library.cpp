#include "Library.h"
#include <iostream>
#include <vector>
#include <curl/curl.h>

/* Number of easy curls to bundle in a multi curl. ~50-100 seems to be optimum */
#define N 50

Library::Library(std::string username)
{
    /* Should be called only once for the entire program */
    curl_global_init(CURL_GLOBAL_SSL);

    hash_size = 100;
    hashTable = new LibraryEntryWrapper[hash_size];

    int failure = getLibrary(username);

    /* To indicate that the user's library could not be gotten.
       the library size is set to -1. */
    if(failure == 1)
        library_size = -1;
}

Library::~Library()
{

    /* Opposite of curl_global_init() */
    curl_global_cleanup();

    /* Delete hashtable */
    if(library_size > 0) {
        for(int i=0; i<hash_size; i++) {
            if(hashTable[i].entry != NULL) {
                LibraryEntryWrapper *x = &hashTable[i];
                while(x != NULL) {
                    x = x->next;
                    delete x;
                }
            }
        }
    }
}

/* Writes the data returned by each curl to the buffer string pointed to by *userp.
   Source: http://stackoverflow.com/questions/9786150/save-curl-content-result-into-a-string-in-c */
size_t Library::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int Library::getLibrary(std::string username) {

    /* Return code: 1 on failure, 0 on success */
    int rc;

    /* Hummingbird.me API URL for getting library */
	std::string baseurl = "https://hummingbird.me/api/v1";
	std::string endpoint = baseurl + "/users/" + username + "/library";

	/* Buffer string for storing the API response */
	std::string buffer;

    /* Set up curl for downloading library information from API.
       This will go to the API endpoint URL above and download
       the server's response in plain text (unparsed JSON) */
	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();

	/* Set curl options and perform curl downloads */
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);
		curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1);
		curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
    if (res != CURLE_OK) {
        /* Curl failed! */
        fprintf(stderr, "cURL failed: %s\n",
            curl_easy_strerror(res));
        rc = 1;
    } else {
        /* Curl succeeded! */

        /* Now we must download all of the metadata for the shows in the library */

        /* Parse the downloaded library from buffer to a JSON object */
        library_json = json_tokener_parse(buffer.c_str());

        /* Get the size of the library from JSON object */
        library_size = json_object_array_length(library_json);

        /* The final json objects that we want will be stored in this array*/
        json_object *library_entries_json[library_size];

        /* Set up N new curls, which will be reused multiple times
           to download all of the metadata for the shows in the library.
           Reusing a bunch of curls makes the transfer go faster because
           each curl maintains a connection to the same server even after
           being reset using curl_easy_reset(). */
        CURL *curls[N];
        std::string buffers[library_size];

        /* No. of entries that we have read from library_json so far */
        int counter = 0;

        /* Initialize curl multi interface. This performs a bunch of
           easy curls at once asynchronously, speeding things up a lot.
           In this case, we perform batches of N curls at a time,
           reusing both the multi_handle and the easy curls. */
        CURLM *multi_handle = curl_multi_init();

        /* Stores the number of easy curls still running in multi_curl */
        int still_running;

        /* Initialize our N easy curls (only has to be done once) */
        for(int i=0; i<N; i++)
            curls[i] = curl_easy_init();

        /* Main loop in which we perform all of the curls */
        while(counter < library_size) {

            /* How many easy curls to put in the multi curl */
            int multi_handle_size = N;

            if(library_size-counter < N)
                multi_handle_size = library_size - counter;

            /* Add easy curls to multi curl */
            for(int i=0; i<multi_handle_size; i++) {

                /* Get the library entry from library json array */
                json_object *entry = json_object_array_get_idx(library_json, counter);

                /* Initialize the final library entry */
                library_entries_json[counter] = json_object_new_object();

                /* Add library status field to final library entry */
                json_object *entry_library_status;
                json_object_object_get_ex(entry, "status", &entry_library_status);
                json_object_object_add(library_entries_json[counter], "library_status", entry_library_status);

                /* Add episodes watched field to final library entry */
                json_object *entry_episodes_watched;
                json_object_object_get_ex(entry, "episodes_watched", &entry_episodes_watched);
                json_object_object_add(library_entries_json[counter], "episodes_watched", entry_episodes_watched);

                /* Add rating field to final library entry */
                json_object *entry_rating;
                json_object_object_get_ex(entry, "rating", &entry_rating);
                json_object *entry_rating_value;
                json_object_object_get_ex(entry_rating, "value", &entry_rating_value);
                json_object_object_add(library_entries_json[counter], "rating", entry_rating_value);

                /* Get Hummingbird ID number of library entry */
                json_object *entry_anime;
                json_object_object_get_ex(entry, "anime", &entry_anime);
                json_object *entry_id;
                json_object_object_get_ex(entry_anime, "id", &entry_id);

                /* Use ID number to figure out URL for downloading more metadata */
                std::string id = json_object_to_json_string(entry_id);
                endpoint = baseurl + "/anime/" + id;

                /* Set up curl with the required options */
                if(curls[i]) {
                    curl_easy_setopt(curls[i], CURLOPT_URL, endpoint.c_str());
                    curl_easy_setopt(curls[i], CURLOPT_WRITEFUNCTION, WriteCallback);
                    curl_easy_setopt(curls[i], CURLOPT_WRITEDATA, &buffers[counter]);
                    curl_easy_setopt(curls[i], CURLOPT_FAILONERROR, 1);
                    curl_easy_setopt(curls[i], CURLOPT_HEADER, 0);
                } else {
                    rc = 1;
                    break;
                }

                /* Add ith curl to multi handle */
                curl_multi_add_handle(multi_handle, curls[i]);

                counter++;
            }

            /* Kickstart multi curl, as per https://moz.com/devblog/high-performance-libcurl-tips/*/
            if(counter == multi_handle_size)
                curl_multi_socket_action(multi_handle, CURL_SOCKET_TIMEOUT, 0, &still_running);

            /* Perform multi curl action (asynchronous) */
            curl_multi_perform(multi_handle, &still_running);

            /* libcurl mutli interface voodoo (waiting for multi curl to finish)
               Source: http://curl.haxx.se/libcurl/c/multi-app.html */
            do {
                struct timeval timeout;
                int rc; /* select() return code */
                CURLMcode mc; /* curl_multi_fdset() return code */

                fd_set fdread;
                fd_set fdwrite;
                fd_set fdexcep;
                int maxfd = -1;

                long curl_timeo = -1;

                FD_ZERO(&fdread);
                FD_ZERO(&fdwrite);
                FD_ZERO(&fdexcep);

                /* set a suitable timeout to play around with */
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

                curl_multi_timeout(multi_handle, &curl_timeo);
                if(curl_timeo >= 0) {
                    timeout.tv_sec = curl_timeo / 1000;
                    if(timeout.tv_sec > 1)
                        timeout.tv_sec = 1;
                    else
                        timeout.tv_usec = (curl_timeo % 1000) * 1000;
                }

                /* get file descriptors from the transfers */
                mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

                if(mc != CURLM_OK)
                {
                      fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
                      break;
                }

                /* On success the value of maxfd is guaranteed to be >= -1. We call
                   select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
                   no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
                   to sleep 100ms, which is the minimum suggested value in the
                   curl_multi_fdset() doc. */

                if(maxfd == -1) {
                      /* Portable sleep for platforms other than Windows. */
                      struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
                      rc = select(0, NULL, NULL, NULL, &wait);
                }
                else {
                      /* Note that on some platforms 'timeout' may be modified by select().
                         If you need access to the original value save a copy beforehand. */
                      rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
                }

                switch(rc) {
                case -1:
                      /* select error */
                      break;
                case 0: /* timeout */
                default: /* action */
                      curl_multi_perform(multi_handle, &still_running);
                      break;
                }
            } while(still_running);

            for(int i=0; i<N; i++) {
                curl_multi_remove_handle(multi_handle, curls[i]);
                curl_easy_reset(curls[i]);
            }
        }

        /* Clean up curls and multi curl */
        for(int i=0; i<N; i++)
            curl_easy_cleanup(curls[i]);

        curl_multi_cleanup(multi_handle);

        /* Parse all of the JSON respones from the API and add the fields
           we want to our own library_entries_json array */
        for(int i=0; i<library_size; i++) {

             /* Parse anime object from buffer*/
             json_object *anime_json = json_tokener_parse(buffers[i].c_str());

             /* Add title field to ith final library entry */
             json_object *entry_anime_title;
             json_object_object_get_ex(anime_json, "title", &entry_anime_title);
             json_object_object_add(library_entries_json[i], "title", entry_anime_title);

             /* Add synopsis field to ith final library entry */
             json_object *entry_anime_synopsis;
             json_object_object_get_ex(anime_json, "synopsis", &entry_anime_synopsis);
             json_object_object_add(library_entries_json[i], "synopsis", entry_anime_synopsis);

             /* Add airing status field to ith final library entry */
             json_object *entry_airing_status;
             json_object_object_get_ex(anime_json, "status", &entry_airing_status);
             json_object_object_add(library_entries_json[i], "airing_status", entry_airing_status);

             /* Add episode count field to ith final library entry */
             json_object *entry_anime_episode_count;
             json_object_object_get_ex(anime_json, "episode_count", &entry_anime_episode_count);
             json_object_object_add(library_entries_json[i], "episode_count", entry_anime_episode_count);

             /* Add show type field to ith final library entry */
             json_object *entry_anime_type;
             json_object_object_get_ex(anime_json, "show_type", &entry_anime_type);
             json_object_object_add(library_entries_json[i], "show_type", entry_anime_type);

             /* Add community rating field to ith final library entry */
             json_object *entry_anime_community_rating;
             json_object_object_get_ex(anime_json, "community_rating", &entry_anime_community_rating);
             json_object_object_add(library_entries_json[i], "community_rating", entry_anime_community_rating);

             /* Add genres field to ith final library entry */
             json_object *entry_anime_genres;
             json_object_object_get_ex(anime_json, "genres", &entry_anime_genres);
             json_object_object_add(library_entries_json[i], "genres", entry_anime_genres);

             /* Add final library entry to internal hash table */
             LibraryEntry *x = new LibraryEntry(library_entries_json[i]);
             addEntry(x);
        }
        /* Success */
        rc = 0;
    }

	return rc;
}

void Library::addEntry(LibraryEntry *x) {
    LibraryEntryWrapper *w = new LibraryEntryWrapper();
    w->entry = x;
    LibraryEntryWrapper *y = NULL;

    int h = hashSum(x->getTitle());

    y = &hashTable[h];

    if(y->entry == NULL) {
        hashTable[h] = *w;
        delete w;
    } else {
        while(y->next != NULL) {
            y = y->next;
        }
        y->next = w;
        w->previous = y;
    }
}

int Library::getLibrarySize() {
    return library_size;
}

int Library::hashSum(std::string title) {
    int sum = 0;
    for (unsigned i = 0; i < title.size(); i++)
        sum += title[i];  /* ascii value of ith character in the string */
    sum = sum % hash_size;
    return sum;
}

LibraryEntry* Library::getLibraryEntry(std::string title) {
    LibraryEntryWrapper *x;
    int h = hashSum(title);
    bool found = false;

    x = &hashTable[h];

    while(found == false && x->entry != NULL) {
        if(x->entry->getTitle().compare(title) == 0) {
            found = true;
        } else {
            x = x->next;
        }
    }

    if(found == true)
        return x->entry;
    else
        return NULL;
}

/* Returns true if the first LibraryEntry goes before the second, alphabetically by title */
bool Library::libraryEntryTitleSort(LibraryEntry* i, LibraryEntry* j) {

    bool goesBefore;

    if(i->getTitle().compare(j->getTitle()) < 0)
        goesBefore = true;
    else
        goesBefore = false;

    return goesBefore;
}

/* Returns a vector of LibraryEntry pointers sorted alphabetically by title */
std::vector<LibraryEntry*> Library::getLibraryEntries(library_status ls) {

    std::vector<LibraryEntry*> libraryEntries;

    for(int i=0; i<hash_size; i++) {
        if(hashTable[i].entry != NULL) {
            LibraryEntryWrapper *x = &hashTable[i];
            while(x != NULL) {
                if(x->entry->getLibraryStatus() == ls) {
                    libraryEntries.push_back(x->entry);
                }
                x = x->next;
            }
        }
    }

    std::sort(libraryEntries.begin(), libraryEntries.end(), libraryEntryTitleSort);
    return libraryEntries;

}
