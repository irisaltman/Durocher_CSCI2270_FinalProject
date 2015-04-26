#ifndef LIBRARYENTRY_H
#define LIBRARYENTRY_H
#include <string>
#include <json/json.h>
#include <vector>

class LibraryEntry
{
    public:
        LibraryEntry(json_object *j);
        LibraryEntry();
        virtual ~LibraryEntry();
        std::string getTitle() { return title; }
        std::string getSynopsis() { return synopsis; }
        std::string getAiringStatus() { return airingStatus; }
        std::string getEpisodeCount() { return episodeCount; }
        std::string getType() { return type; }
        std::string getLibraryStatus() { return libraryStatus; }
        std::string getEpisodesWatched() { return episodesWatched; }
        std::string getRating() { return rating; }
        std::string getCommunityRating() { return communityRating; }
        std::vector<std::string> getGenres() { return genres; }
    protected:
    private:
        std::string title;
        std::string synopsis;
        std::vector<std::string> genres;
        std::string airingStatus;
        std::string episodeCount;
        std::string type;
        std::string libraryStatus;
        std::string episodesWatched;
        std::string rating;
        std::string communityRating;
};
#endif // LIBRARYENTRY_H
