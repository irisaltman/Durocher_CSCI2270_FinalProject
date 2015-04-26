#include "LibraryEntry.h"

LibraryEntry::LibraryEntry(json_object *j)
{
    json_object *title_json;
    json_object_object_get_ex(j, "title", &title_json);
    title = std::string(json_object_get_string(title_json));

    json_object *synopsis_json;
    json_object_object_get_ex(j, "synopsis", &synopsis_json);
    synopsis = std::string(json_object_get_string(synopsis_json));

    json_object *airing_status_json;
    json_object_object_get_ex(j, "airing_status", &airing_status_json);
    airingStatus = std::string(json_object_get_string(airing_status_json));

    json_object *episode_count_json;
    json_object_object_get_ex(j, "episode_count", &episode_count_json);
    episodeCount = std::string(json_object_to_json_string(episode_count_json));

    json_object *episodes_watched_json;
    json_object_object_get_ex(j, "episodes_watched", &episodes_watched_json);
    episodesWatched = std::string(json_object_to_json_string(episodes_watched_json));

    json_object *library_status_json;
    json_object_object_get_ex(j, "library_status", &library_status_json);
    libraryStatus = std::string(json_object_get_string(library_status_json));

    json_object *rating_json;
    json_object_object_get_ex(j, "rating", &rating_json);
    rating = std::string(json_object_to_json_string(rating_json));

    json_object *community_rating_json;
    json_object_object_get_ex(j, "community_rating", &community_rating_json);
    communityRating = std::string(json_object_to_json_string(community_rating_json));

    json_object *type_json;
    json_object_object_get_ex(j, "show_type", &type_json);
    type = std::string(json_object_get_string(type_json));

    json_object *genres_json;
    json_object_object_get_ex(j, "genres", &genres_json);
    genres.assign(json_object_array_length(genres_json),"");
    for(int i=0; i<json_object_array_length(genres_json); i++) {
        json_object *genre_json = json_object_array_get_idx(genres_json, i);
        json_object *genre_name_json;
        json_object_object_get_ex(genre_json, "name", &genre_name_json);
        genres[i] = std::string(json_object_get_string(genre_name_json));
    }
}

LibraryEntry::LibraryEntry()
{
    title = "";
    synopsis = "";
    airingStatus = "";
    episodeCount = "";
    episodesWatched = "";
    libraryStatus = "";
    rating = "";
    communityRating = "";
    type = "";
}


LibraryEntry::~LibraryEntry()
{
    //dtor
}