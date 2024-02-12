// The Resource class is intended to contain details about a resource that we will consume.
// Attribures of our resource class are:
//  - path to file of resource
//  - type of resource (audio::Mix_Chunk, music::Mix_Music) #MORE TO COME LATER
//  - name - this can be derived from something passed in, or based on the filename.
//           ? need to normalize special characters?
//  - ? perhaps a hash of the name of the thing to make it easy to refer to?
//  - ? do we want to store the Mix_object in this object to play through the sound engine?

#pragma once
#if __APPLE__
#include <SDL2_mixer/SDL_mixer.h>
#else
#include "SDL2/SDL_mixer.h"
#endif
#include <string>

using std::string; // scope to include for easier access to 'string' type.

class Resource {
    public:
        string& GetName();
        string& GetHash();
    
    protected:
        bool FileExists(const string& filePath);
        void SetName(const string& filePath);
        string filePath;
        string name;
        string hash;
};

class Sound: public Resource {
    public:
        explicit Sound(const string& filePath);
        explicit Sound(const string& filePath, const string& name);
        ~Sound();
        Mix_Chunk* GetSound();
        Mix_Chunk* GetAudio();

    private:
        Mix_Chunk* sound;
};

class Music: public Resource {
    public:
        explicit Music(const string& filePath);
        explicit Music(const string& filePath, const string& name);
        ~Music();
        Mix_Music* GetMusic();
        Mix_Music* GetAudio();

    private:
        Mix_Music* music;
};