#ifndef UTILITARIA_H_
#define UTILITARIA_H_

#include <vector>
#include <string>
#include <cstring>
#include <cstdio>
#include <wx/textctrl.h>

using namespace std;

class Utilities
{
    private:
        wxTextCtrl *memoInput;
        FILE *logFile;

    public:
        Utilities(wxTextCtrl*);
        Utilities();
        virtual ~Utilities();
        
        int find(vector<string>, string);
        int find(vector<int>, int);
        bool fileExists(string);
        void printLog(string, bool);
};

#endif /* UTILITARIA_H_ */
