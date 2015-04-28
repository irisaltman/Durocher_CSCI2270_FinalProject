# A simple C++ library for Hummingbird.me

### Summary

This library allows you to interface with the Hummingbird.me API from within your C++ application. [Hummingbird.me](https://hummingbird.me) is an anime tracking site that lets users keep track of shows that they are currently watching, plan to watch, have completed, have "on hold," or have "dropped". At the moment this library only lets you download a certain user's "anime library," which contains all of the anime that they have on any of their lists. You can then search the anime library for specific entries and get more information about them, like a synopsis, how many episodes the user has watched, what the show's genres are, ratings, etc.

### How to run

Running the example program is super easy, simply do `./main [username]` to download and browse a certain user's anime library. For example, [Josh](https://hummingbird.me/users/Josh/library) is the username of the co-founder of Hummingbird. To download and browse his library, simple type `./main Josh`.

Documentation on how the library works can be found in the library implementation files Library.cpp and LibraryEntry.cpp and their associated header files.

### Dependencies

This library depends on the cURL and json-c shared libraries. To install them in Ubuntu:

    # apt get install libjson0 libjson0-dev libcurl3 libcurl3-gnutls libcurl4-openssl-dev
    
To install them in Fedora:

    # yum install json-c json-c-devel libcurl libcurl-devel
    
You can build the program by running `make`, which will output the example program to the folder `bin/Release/`. You can also load the Code Blocks project file if you want.

### System Requirements

This project only works on Linux right now. It requires an internet connection to download information from Hummingbird.

### Group Members

Hjalte Durocher

### Contributors

None, yet.

### Known Bugs

* Trying to download some users' libraries causes the following error while parsing the JSON objects returned by the Hummingbird API:

```
terminate called after throwing an instance of 'std::logic_error'
    what():  basic_string::_S_construct null not valid
```

* Sometimes cURL (or perhaps something else) seems to hang or get stuck while getting a user's library in getLibrary(). (This is probably the case if it takes more than a minute to download).
    
### Future Features

There is a bunch of information returned by the Hummingbird API that isn't being stored yet. See [the official Hummingbird API reference](https://github.com/hummingbird-me/hummingbird/wiki/API-v1-Structures) for a full list of all the fields in the Anime and Library entry objects. Some of the ones that aren't being used yet include the URL to the corresponding Hummingbird anime page, started/finished airing dates, age rating, user's notes, times rewatched, rewatching status, etc. Adding these would be as simple as adding the corresponding strings and getters in the LibraryEntry class and making sure we parse the JSON objects for the information when want in `getLibrary()` and the LibraryEntry constructor.

Similarly, the rating system is not fully supported right now (no simple/advanced modes).

In the future there should also be a class that contains information about the specific user, like a bio and their favorites.

Ultimately, I would like to add the ability to use this library to update anime library entries, like marking episodes watched or shows completed, and have the changes sync with the Hummingbird website.
