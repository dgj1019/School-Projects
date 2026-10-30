#include <string>
#include<iostream>
using namespace std;

void ReverseIt(char* o, char* r){
    int spaceCounter = 0;
    int charCounter = 0;
    while(o[charCounter] != '\0'){
        if(o[charCounter] == ' '){
            spaceCounter++;
        }
        charCounter++;
    }
    
    int i = 1;
    int j = 0;
    int currCount;
    for(int k = 0; k < spaceCounter + 1; k++){
        while(charCounter - i >= 0 && o[charCounter - i] != ' '){
            i++;
        }
        currCount = (charCounter - i) + 1;
        
        if (currCount < 0) currCount = 0;
        
        while(o[currCount] != ' ' && o[currCount] != '\0'){
                r[j] = o[currCount];
                currCount++;
                j++;
        }
        if(k < spaceCounter){
            r[j] = ' ';
            j++;
        }
        i++;
    }
    r[j] = '\0';
    
}

int main()
{
    char original[] = "Conan The Barbarian\0";
    char reversed[100];

    ReverseIt(original, reversed);

    cout << original << " reversed is " << reversed << endl;
}