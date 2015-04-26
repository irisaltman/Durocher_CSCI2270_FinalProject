/* Hjalte Durocher
 * CSCI 2270
 * Final Project
 * Rhonda Hoenigman
*/
#include "Library.h"
#include <iostream>
#include <string>
#include <stdlib.h>

using namespace std;

string printMenu(string username)
{
    cout << "---------------Main Menu---------------" << endl;
    cout << "1. Print " << username << "'s Currently Watching list" << endl;
    cout << "2. Print " << username << "'s Plan to Watch list" << endl;
    cout << "3. Print " << username << "'s Completed list" << endl;
    cout << "4. Print " << username << "'s On Hold list" << endl;
    cout << "5. Print " << username << "'s Dropped list" << endl;
    cout << "6. Get more information about a show" << endl;
    cout << "7. Quit" << endl;

    string in;
    getline(cin, in);

    return in;
}

Library* downloadLibrary(string username) {
    int rc;
    cout << "Downloading " << username << "'s Hummingbird.me library..." << endl;
    cout << "(This can take a while)" << endl;
    Library *L = new Library(username);
    return L;
}

int main(int argc, char *argv[])
{
    int rc;
    string username;

    if(argc != 2) {
        cout << "Usage: main username" << endl;
        cout << "username is the name of a Hummingird.me account (ex: Josh)" << endl;
        rc = 1;
    } else {
        username = string(argv[1]);

        Library *L = downloadLibrary(username);

        if(L->getLibrarySize() != -1) {
            cout << "Done! (loaded " << L->getLibrarySize() << " entries)"<< endl << endl;
            rc = 0;
        }
        else {
            cout << "Failure! Try running the program again."  << endl;
            rc = 1;
        }

        if(rc == 0) {
            int s;
            while(1) {
                s = atoi(printMenu(username).c_str());
                if(s == 1) {
                    cout << "1" << endl;
                }
                else if(s == 2) {
                    cout << "2" << endl;
                }
                else if(s == 3) {
                    cout << "3" << endl;
                }
                else if(s == 4) {
                    cout << "4" << endl;
                }
                else if(s == 5) {
                    cout << "5" << endl;
                }
                else if(s == 6) {
                    cout << "5" << endl;
                }
                else if(s == 7) {
                    cout << "Goodbye!" << endl;
                    rc = 0;
                    break;
                } else {
                    cout << "Didn't understand that, try again" << endl << endl;
                }
            }
        }

        delete L;
    }

    return rc;
}


